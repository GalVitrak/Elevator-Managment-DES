#ifndef RANDOM_SEED_H
#define RANDOM_SEED_H

#include "file_manager.h"
#include "simulation.h"

/*
 * One passenger trip: source and destination floors (0-based).
 */
typedef struct {
    int sourceFloor;
    int destinationFloor;
    double arrivalTime;     /* simulation time when passenger appears (DES event) */
} PassengerRequest;

/*
 * Full random scenario: building config, RNG seed, and request list.
 * requests is heap-allocated when numRequests > 0 (see seed_free).
 */
typedef struct {
    SimulationConfig config;
    unsigned int randomSeed;
    int numRequests;
    PassengerRequest* requests;
} SeedScenario;

/* Release requests array and reset counts. */
void seed_scenario_free(SeedScenario* scenario);

/*
 * Generate random trips with staggered arrivalTime (exponential inter-arrival).
 * Mean gap is computed from travel/door timing and building config (see random_seed.c).
 * Uses srand(randomSeed); if randomSeed is 0, uses current time.
 */
int seed_generate_random(SeedScenario* scenario, const SimulationConfig* config,
                         int numRequests, unsigned int randomSeed);

/* Mean seconds between arrivals from timing model (for logging / seed file). */
double seed_compute_auto_inter_arrival(const SimulationConfig* config, int numRequests);

/* Write config, seed, and all requests to a text file. Returns 1 on success. */
int seed_save_to_file(const SeedScenario* scenario, const char* filename);

/*
 * Read scenario from file (config + requests). Caller must seed_scenario_free when done.
 * Returns 1 on success.
 */
int seed_load_from_file(SeedScenario* scenario, const char* filename);

/*
 * Init simulation and schedule PASSENGER_CALL events at each request's arrivalTime.
 * Returns 1 on success.
 */
int seed_apply_to_simulation(Simulation* sim, const SeedScenario* scenario);

#endif /* RANDOM_SEED_H */
