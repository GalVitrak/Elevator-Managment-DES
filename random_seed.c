/*
 * random_seed.c - Generate, save, and load random passenger request scenarios
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
 * pick_random_floor_pair - Set source and dest in [0, numFloors-1], source != dest.
 */
static void pick_random_floor_pair(int numFloors, int* source, int* dest)
{
    int s;
    int d;

    if (numFloors < 2) {
        *source = 0;
        *dest = 0;
        return;
    }

    s = rand() % numFloors;
    do {
        d = rand() % numFloors;
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

    if (config->numFloors > 1) {
        avgFloorDistance = (double)(config->numFloors - 1) / 3.0;
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

    if (spreadGap > capacityGap) {
        return spreadGap;
    }
    return capacityGap;
}

int seed_generate_random(SeedScenario* scenario, const SimulationConfig* config,
                         int numRequests, unsigned int randomSeed)
{
    int i;
    unsigned int seedUsed;
    double arrivalTime;
    double maxArrival;
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

    maxArrival = config->maxSimulationTime * 0.95;

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
        if (arrivalTime > maxArrival) {
            arrivalTime = maxArrival;
        }

        scenario->requests[i].arrivalTime = arrivalTime;
        pick_random_floor_pair(config->numFloors,
                               &scenario->requests[i].sourceFloor,
                               &scenario->requests[i].destinationFloor);
    }

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

static int parse_request_line(const char* line, int* source, int* dest, double* arrivalTime)
{
    int parsed2;
    int parsed3;

    if (strncmp(line, "request=", 8) != 0) {
        return 0;
    }

    parsed3 = sscanf(line + 8, "%d,%d,%lf", source, dest, arrivalTime);
    if (parsed3 == 3) {
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

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "num_floors=", 11) == 0) {
            sscanf(line + 11, "%d", &scenario->config.numFloors);
            configOk = 1;
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
            if (scenario->requests == NULL || requestCount >= requestCapacity) {
                continue;
            }
            if (parse_request_line(line, &src, &dst, &arrival)) {
                scenario->requests[requestCount].sourceFloor = src;
                scenario->requests[requestCount].destinationFloor = dst;
                scenario->requests[requestCount].arrivalTime = arrival;
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

    if (requestCount != scenario->numRequests) {
        log_message(0.0, LOG_WARNING, "Seed file request count mismatch; using parsed count");
        scenario->numRequests = requestCount;
    }

    seed_stagger_arrivals_if_needed(scenario);

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
        simulation_schedule_passenger_arrival(sim,
                                              scenario->requests[i].arrivalTime,
                                              scenario->requests[i].sourceFloor,
                                              scenario->requests[i].destinationFloor);
    }

    return 1;
}
