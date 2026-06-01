/*
 * random_seed.c - Generate, save, and load random passenger request scenarios
 *
 * PRESENTATION: Menu 6/7 — seed_spread_arrivals_across_horizon avoids bunching at
 * max_time; seed_compute_auto_inter_arrival respects fleet capacity.
 */
#include "random_seed.h"
#include "constants.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* seed_scenario_free - Free request array and clear scenario fields. */
void seed_scenario_free(SeedScenario* scenario)
{
    if (scenario == NULL) {
        return;
    }
    free(scenario->requests);
    scenario->requests = NULL;
    scenario->numRequests = 0;
}

/*
 * pick_random_floor_pair - Random display floors (ground=0, negative=basement), source != dest.
 */
static void pick_random_floor_pair(const SimulationConfig* config, int* source, int* dest)
{
    int minDisplay;
    int maxDisplay;
    int span;
    int s;
    int d;

    if (config == NULL || source == NULL || dest == NULL) {
        return;
    }

    minDisplay = config_display_floor_min(config);
    maxDisplay = config_display_floor_max(config);
    span = maxDisplay - minDisplay + 1;

    if (span < 2) {
        *source = minDisplay;
        *dest = minDisplay;
        return;
    }

    s = minDisplay + (rand() % span);
    do {
        d = minDisplay + (rand() % span);
    } while (d == s);

    *source = s;
    *dest = d;
}

/*
 * seed_generate_random - Build random requests for a validated config.
 */
/*
 * random_exponential_gap - Sample next inter-arrival time (exponential distribution).
 */
static double random_exponential_gap(double meanSeconds)
{
    double u;
    double gap;

    if (meanSeconds <= 0.0) {
        meanSeconds = 30.0;
    }

    u = (double)rand() / (double)RAND_MAX;
    if (u >= 1.0) {
        u = 0.999999;
    }
    gap = -log(1.0 - u) * meanSeconds;
    return gap;
}

static double timing_door_cycle_per_stop(void)
{
    return DOOR_OPEN_TIME_SECONDS + DOOR_DWELL_SECONDS + DOOR_CLOSE_TIME_SECONDS;
}

/*
 * seed_compute_auto_inter_arrival - Derive mean gap from travel + door timing and fleet size.
 * Uses average trip (pickup leg + drop-off leg, two door cycles) spread across elevators,
 * and never faster than evenly filling the simulation horizon.
 */
double seed_compute_auto_inter_arrival(const SimulationConfig* config, int numRequests)
{
    double avgFloorDistance;
    double oneLegTravel;
    double fullTrip;
    double capacityGap;
    double spreadGap;

    if (config == NULL || config->numElevators < 1) {
        return 30.0;
    }

    if (config_total_internal_floors(config) > 1) {
        avgFloorDistance = (double)(config_display_floor_max(config) -
                                      config_display_floor_min(config)) / 3.0;
        if (avgFloorDistance < 1.0) {
            avgFloorDistance = 1.0;
        }
    } else {
        avgFloorDistance = 0.0;
    }

    oneLegTravel = avgFloorDistance * SECONDS_PER_FLOOR;
    fullTrip = 2.0 * oneLegTravel + 2.0 * timing_door_cycle_per_stop();
    capacityGap = fullTrip / (double)config->numElevators;

    if (numRequests > 0 && config->maxSimulationTime > 0.0) {
        spreadGap = (config->maxSimulationTime * 0.95) / (double)numRequests;
    } else {
        spreadGap = capacityGap;
    }

    /*
     * Use the slower of horizon spread and fleet capacity so arrivals stay
     * achievable (max serve-rate). Slight safety margin on capacity.
     */
    capacityGap *= 1.05;
    if (spreadGap > capacityGap) {
        return spreadGap;
    }
    return capacityGap;
}

typedef struct {
    int requestIndex;
    double sortKey;
} SeedArrivalOrder;

static int seed_arrival_order_compare(const void* a, const void* b)
{
    const SeedArrivalOrder* left = (const SeedArrivalOrder*)a;
    const SeedArrivalOrder* right = (const SeedArrivalOrder*)b;

    if (left->sortKey < right->sortKey) {
        return -1;
    }
    if (left->sortKey > right->sortKey) {
        return 1;
    }
    if (left->requestIndex < right->requestIndex) {
        return -1;
    }
    if (left->requestIndex > right->requestIndex) {
        return 1;
    }
    return 0;
}

/*
 * seed_spread_arrivals_across_horizon - Evenly space arrivals in [0, maxArrival].
 * Exponential draws set random order only; each rank gets an equal time slot so
 * %.2f file output does not pile hundreds of requests at the horizon end.
 */
static void seed_spread_arrivals_across_horizon(SeedScenario* scenario)
{
    int i;
    int n;
    double maxArrival;
    SeedArrivalOrder* order;
    double* newTimes;

    if (scenario == NULL || scenario->requests == NULL || scenario->numRequests <= 0) {
        return;
    }

    n = scenario->numRequests;
    maxArrival = scenario->config.maxSimulationTime * 0.95;

    if (n == 1) {
        scenario->requests[0].arrivalTime = maxArrival * 0.5;
        return;
    }

    order = (SeedArrivalOrder*)malloc((size_t)n * sizeof(SeedArrivalOrder));
    newTimes = (double*)malloc((size_t)n * sizeof(double));
    if (order == NULL || newTimes == NULL) {
        free(order);
        free(newTimes);
        for (i = 0; i < n; i++) {
            scenario->requests[i].arrivalTime =
                maxArrival * (double)i / (double)(n - 1);
        }
        return;
    }

    for (i = 0; i < n; i++) {
        order[i].requestIndex = i;
        order[i].sortKey = scenario->requests[i].arrivalTime;
    }

    qsort(order, (size_t)n, sizeof(SeedArrivalOrder), seed_arrival_order_compare);

    for (i = 0; i < n; i++) {
        newTimes[order[i].requestIndex] =
            maxArrival * (double)i / (double)(n - 1);
    }

    for (i = 0; i < n; i++) {
        scenario->requests[i].arrivalTime = newTimes[i];
    }

    free(order);
    free(newTimes);
}

int seed_generate_random(SeedScenario* scenario, const SimulationConfig* config,
                         int numRequests, unsigned int randomSeed)
{
    int i;
    unsigned int seedUsed;
    double arrivalTime;
    double meanGap;

    if (scenario == NULL || config == NULL || !config_validate(config)) {
        return 0;
    }
    if (numRequests < MIN_SEED_REQUESTS || numRequests > MAX_SEED_REQUESTS) {
        return 0;
    }

    seed_scenario_free(scenario);

    scenario->config = *config;
    scenario->numRequests = numRequests;

    seedUsed = randomSeed;
    if (seedUsed == 0) {
        seedUsed = (unsigned int)time(NULL);
    }
    scenario->randomSeed = seedUsed;
    srand(seedUsed);

    if (numRequests == 0) {
        scenario->requests = NULL;
        return 1;
    }

    meanGap = seed_compute_auto_inter_arrival(config, numRequests);

    scenario->requests = (PassengerRequest*)malloc((size_t)numRequests * sizeof(PassengerRequest));
    if (scenario->requests == NULL) {
        scenario->numRequests = 0;
        return 0;
    }

    arrivalTime = random_exponential_gap(meanGap) * 0.5;

    for (i = 0; i < numRequests; i++) {
        if (i > 0) {
            arrivalTime += random_exponential_gap(meanGap);
        }

        scenario->requests[i].arrivalTime = arrivalTime;
        pick_random_floor_pair(config,
                               &scenario->requests[i].sourceFloor,
                               &scenario->requests[i].destinationFloor);
    }

    seed_spread_arrivals_across_horizon(scenario);

    return 1;
}

/*
 * seed_save_to_file - Write human-readable scenario for replay and reports.
 *
 * Format:
 *   num_floors, num_elevators, capacity, max_simulation_time (same as config.txt)
 *   random_seed=<unsigned>
 *   num_requests=<count>
 *   avg_inter_arrival=<seconds>
 *   request=<source>,<destination>,<arrivalTime>  (one per line)
 */
int seed_save_to_file(const SeedScenario* scenario, const char* filename)
{
    FILE* file;
    int i;

    if (scenario == NULL || filename == NULL || !config_validate(&scenario->config)) {
        return 0;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        log_message(0.0, LOG_ERROR, "Failed to open seed file for writing");
        return 0;
    }

    fprintf(file, "num_floors=%d\n", scenario->config.numFloors);
    fprintf(file, "num_underground=%d\n", scenario->config.numUndergroundFloors);
    fprintf(file, "num_elevators=%d\n", scenario->config.numElevators);
    fprintf(file, "capacity=%d\n", scenario->config.capacity);
    fprintf(file, "max_simulation_time=%.2f\n", scenario->config.maxSimulationTime);
    fprintf(file, "random_seed=%u\n", scenario->randomSeed);
    fprintf(file, "num_requests=%d\n", scenario->numRequests);
    fprintf(file, "avg_inter_arrival=%.2f\n",
            seed_compute_auto_inter_arrival(&scenario->config, scenario->numRequests));

    for (i = 0; i < scenario->numRequests; i++) {
        fprintf(file, "request=%d,%d,%.2f\n",
                scenario->requests[i].sourceFloor,
                scenario->requests[i].destinationFloor,
                scenario->requests[i].arrivalTime);
    }

    fclose(file);
    log_message(0.0, LOG_INFO, "Random seed scenario saved to file");
    return 1;
}

static int parse_request_line(const char* line, int* source, int* dest, double* arrivalTime,
                              int* explicitArrivalTime)
{
    int parsed2;
    int parsed3;

    if (explicitArrivalTime != NULL) {
        *explicitArrivalTime = 0;
    }

    if (strncmp(line, "request=", 8) != 0) {
        return 0;
    }

    parsed3 = sscanf(line + 8, "%d,%d,%lf", source, dest, arrivalTime);
    if (parsed3 == 3) {
        if (explicitArrivalTime != NULL) {
            *explicitArrivalTime = 1;
        }
        return 1;
    }

    parsed2 = sscanf(line + 8, "%d,%d", source, dest);
    if (parsed2 == 2) {
        *arrivalTime = 0.0;
        return 1;
    }

    return 0;
}

/*
 * seed_stagger_arrivals_if_needed - Old seed files had all arrivals at t=0; spread them out.
 */
static void seed_stagger_arrivals_if_needed(SeedScenario* scenario)
{
    int i;
    int allAtZero;
    double span;
    double step;

    if (scenario == NULL || scenario->requests == NULL || scenario->numRequests <= 1) {
        return;
    }

    allAtZero = 1;
    for (i = 0; i < scenario->numRequests; i++) {
        if (scenario->requests[i].arrivalTime > 0.01) {
            allAtZero = 0;
            break;
        }
    }
    if (!allAtZero) {
        return;
    }

    span = scenario->config.maxSimulationTime * 0.95;
    step = span / (double)scenario->numRequests;
    for (i = 0; i < scenario->numRequests; i++) {
        scenario->requests[i].arrivalTime = step * (double)i;
    }
}

/*
 * seed_load_from_file - Parse seed file; allocate requests array.
 */
int seed_load_from_file(SeedScenario* scenario, const char* filename)
{
    FILE* file;
    char line[MAX_NAME_LEN * 4];
    int requestCapacity;
    int requestCount;
    int configOk;
    int sawExplicitArrivalTimes;

    if (scenario == NULL || filename == NULL) {
        return 0;
    }

    seed_scenario_free(scenario);
    config_set_defaults(&scenario->config);
    scenario->randomSeed = 0;

    file = fopen(filename, "r");
    if (file == NULL) {
        log_message(0.0, LOG_ERROR, "Failed to open seed file for reading");
        return 0;
    }

    requestCapacity = 0;
    requestCount = 0;
    configOk = 0;
    sawExplicitArrivalTimes = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "num_floors=", 11) == 0) {
            sscanf(line + 11, "%d", &scenario->config.numFloors);
            configOk = 1;
        } else if (strncmp(line, "num_underground=", 16) == 0) {
            sscanf(line + 16, "%d", &scenario->config.numUndergroundFloors);
        } else if (strncmp(line, "num_elevators=", 14) == 0) {
            sscanf(line + 14, "%d", &scenario->config.numElevators);
        } else if (strncmp(line, "capacity=", 9) == 0) {
            sscanf(line + 9, "%d", &scenario->config.capacity);
        } else if (strncmp(line, "max_simulation_time=", 20) == 0) {
            sscanf(line + 20, "%lf", &scenario->config.maxSimulationTime);
        } else if (strncmp(line, "random_seed=", 12) == 0) {
            sscanf(line + 12, "%u", &scenario->randomSeed);
        } else if (strncmp(line, "num_requests=", 13) == 0) {
            sscanf(line + 13, "%d", &scenario->numRequests);
            requestCapacity = scenario->numRequests;
            if (requestCapacity > 0) {
                scenario->requests = (PassengerRequest*)calloc(
                    (size_t)requestCapacity, sizeof(PassengerRequest));
                if (scenario->requests == NULL) {
                    fclose(file);
                    seed_scenario_free(scenario);
                    return 0;
                }
            }
        } else if (strncmp(line, "request=", 8) == 0) {
            int src;
            int dst;
            double arrival;
            int explicitArrival;
            if (scenario->requests == NULL || requestCount >= requestCapacity) {
                continue;
            }
            if (parse_request_line(line, &src, &dst, &arrival, &explicitArrival)) {
                scenario->requests[requestCount].sourceFloor = src;
                scenario->requests[requestCount].destinationFloor = dst;
                scenario->requests[requestCount].arrivalTime = arrival;
                if (explicitArrival) {
                    sawExplicitArrivalTimes = 1;
                }
                requestCount++;
            }
        }
    }

    fclose(file);

    if (!configOk || !config_validate(&scenario->config)) {
        log_message(0.0, LOG_ERROR, "Seed file has invalid or missing configuration");
        seed_scenario_free(scenario);
        return 0;
    }

    if (scenario->numRequests < MIN_SEED_REQUESTS ||
        scenario->numRequests > MAX_SEED_REQUESTS) {
        log_message(0.0, LOG_ERROR, "Seed file request count out of range");
        seed_scenario_free(scenario);
        return 0;
    }

    if (requestCount != scenario->numRequests) {
        log_message(0.0, LOG_WARNING, "Seed file request count mismatch; using parsed count");
        scenario->numRequests = requestCount;
    }

  /* Only spread arrivals for legacy seeds without per-request times (all implicit t=0). */
    if (!sawExplicitArrivalTimes) {
        seed_stagger_arrivals_if_needed(scenario);
    }

    log_message(0.0, LOG_INFO, "Random seed scenario loaded from file");
    return 1;
}

/*
 * seed_apply_to_simulation - Re-init sim from scenario config and queue all requests.
 */
int seed_apply_to_simulation(Simulation* sim, const SeedScenario* scenario)
{
    int i;

    if (sim == NULL || scenario == NULL || !config_validate(&scenario->config)) {
        return 0;
    }

    simulation_destroy(sim);
    if (!simulation_init(sim, &scenario->config)) {
        return 0;
    }

    for (i = 0; i < scenario->numRequests; i++) {
        int srcDisplay = scenario->requests[i].sourceFloor;
        int dstDisplay = scenario->requests[i].destinationFloor;
        int srcIndex;
        int dstIndex;

        if (!config_validate_display_floor(&scenario->config, srcDisplay) ||
            !config_validate_display_floor(&scenario->config, dstDisplay)) {
            log_message(0.0, LOG_ERROR, "Seed request has invalid floor numbers");
            return 0;
        }

        srcIndex = config_display_to_index(&scenario->config, srcDisplay);
        dstIndex = config_display_to_index(&scenario->config, dstDisplay);

        simulation_schedule_passenger_arrival(sim,
                                              scenario->requests[i].arrivalTime,
                                              srcIndex, dstIndex);
    }

    return 1;
}
