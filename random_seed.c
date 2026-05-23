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
int seed_generate_random(SeedScenario* scenario, const SimulationConfig* config,
                         int numRequests, unsigned int randomSeed)
{
    int i;
    unsigned int seedUsed;

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

    scenario->requests = (PassengerRequest*)malloc((size_t)numRequests * sizeof(PassengerRequest));
    if (scenario->requests == NULL) {
        scenario->numRequests = 0;
        return 0;
    }

    for (i = 0; i < numRequests; i++) {
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
 *   request=<source>,<destination>  (one per line)
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

    for (i = 0; i < scenario->numRequests; i++) {
        fprintf(file, "request=%d,%d\n",
                scenario->requests[i].sourceFloor,
                scenario->requests[i].destinationFloor);
    }

    fclose(file);
    log_message(0.0, LOG_INFO, "Random seed scenario saved to file");
    return 1;
}

static int parse_request_line(const char* line, int* source, int* dest)
{
  int parsed;
  if (strncmp(line, "request=", 8) != 0) {
    return 0;
  }
  parsed = sscanf(line + 8, "%d,%d", source, dest);
  return parsed == 2;
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
            if (scenario->requests == NULL || requestCount >= requestCapacity) {
                continue;
            }
            if (parse_request_line(line, &src, &dst)) {
                scenario->requests[requestCount].sourceFloor = src;
                scenario->requests[requestCount].destinationFloor = dst;
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
        simulation_add_passenger_request(sim,
                                         scenario->requests[i].sourceFloor,
                                         scenario->requests[i].destinationFloor);
    }

    return 1;
}
