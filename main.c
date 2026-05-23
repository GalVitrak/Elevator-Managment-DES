/*
 * main.c - Program entry point and console menu for the elevator DES
 *
 * Reads user choices, drives configuration load/save, seeds passenger requests,
 * and calls simulation_run() for option 1. Does not contain DES logic itself.
 */
#include "simulation.h"
#include "constants.h"
#include "logger.h"
#include "file_manager.h"
#include "random_seed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* print_menu - Display numbered options and prompt for selection. */
static void print_menu(void)
{
    printf("\n--- Elevator Management System (DES) ---\n");
    printf("1. Start new simulation\n");
    printf("2. Load configuration\n");
    printf("3. Save configuration\n");
    printf("4. Add passenger request manually\n");
    printf("5. Print system state\n");
    printf("6. Generate random seed file\n");
    printf("7. Load seed file and run simulation\n");
    printf("8. Exit\n");
    printf("Select option: ");
}

/*
 * read_int_in_range - Read one integer from stdin after prompt.
 * Discards rest of line after input. Returns 1 and sets *out if value in [minVal,maxVal].
 * Returns 0 on scanf failure or out-of-range (prints error message).
 */
static int read_int_in_range(const char* prompt, int minVal, int maxVal, int* out)
{
    int value;
    int readCount;

    printf("%s", prompt);
    readCount = scanf("%d", &value);
    while (getchar() != '\n') {
        /* discard rest of line */
    }

    if (readCount != 1) {
        printf("Invalid input. Please enter an integer.\n");
        return 0;
    }
    if (value < minVal || value > maxVal) {
        printf("Value must be between %d and %d.\n", minVal, maxVal);
        return 0;
    }

    *out = value;
    return 1;
}

/*
 * configure_interactively - Ask user for floors, elevators, capacity, max time.
 * Fills config struct; leaves defaults if any prompt fails validation.
 */
static void configure_interactively(SimulationConfig* config)
{
    int floors;
    int elevators;
    int capacity;
    int maxTimeInt;

    config_set_defaults(config);

    if (!read_int_in_range("Number of floors: ", MIN_FLOORS, MAX_FLOORS, &floors)) {
        return;
    }
    if (!read_int_in_range("Number of elevators: ", MIN_ELEVATORS, MAX_ELEVATORS, &elevators)) {
        return;
    }
    if (!read_int_in_range("Elevator capacity: ", MIN_CAPACITY, MAX_CAPACITY, &capacity)) {
        return;
    }
    if (!read_int_in_range("Max simulation time (seconds): ", 1, 100000, &maxTimeInt)) {
        return;
    }

    config->numFloors = floors;
    config->numElevators = elevators;
    config->capacity = capacity;
    config->maxSimulationTime = (double)maxTimeInt;
}

/*
 * add_passenger_interactive - Menu option 4: one request if sim already initialized.
 * Requires prior option 1 or 2; does not run simulation_run by itself.
 */
static void add_passenger_interactive(Simulation* sim)
{
    int source;
    int destination;

    if (sim == NULL || sim->numFloors == 0) {
        printf("Initialize simulation configuration first (option 1 or 2).\n");
        return;
    }

    if (!read_int_in_range("Source floor: ", 0, sim->numFloors - 1, &source)) {
        return;
    }
    if (!read_int_in_range("Destination floor: ", 0, sim->numFloors - 1, &destination)) {
        return;
    }

    simulation_add_passenger_request(sim, source, destination);
}

/*
 * start_simulation_interactive - Menu option 1: configure, init, seed requests, run DES.
 * Prints state before and after simulation_run for demonstration.
 */
static void start_simulation_interactive(Simulation* sim, SimulationConfig* config)
{
    int requestCount;
    int i;
    int source;
    int destination;

    configure_interactively(config);
    if (!config_validate(config)) {
        printf("Invalid configuration.\n");
        return;
    }

    simulation_destroy(sim);
    if (!simulation_init(sim, config)) {
        printf("Failed to initialize simulation.\n");
        return;
    }

    if (!read_int_in_range("How many passenger requests to seed? (0-50): ",
                           0, 50, &requestCount)) {
        return;
    }

    for (i = 0; i < requestCount; i++) {
        printf("Request %d:\n", i + 1);
        if (!read_int_in_range("  Source floor: ", 0, sim->numFloors - 1, &source)) {
            return;
        }
        if (!read_int_in_range("  Destination floor: ", 0, sim->numFloors - 1, &destination)) {
            return;
        }
        simulation_add_passenger_request(sim, source, destination);
    }

    simulation_print_state(sim);
    simulation_run(sim);
    simulation_print_state(sim);
}

/*
 * generate_random_seed_interactive - Menu option 6: user picks building params and
 * request count; generates random trips, saves to random_seed.txt (and config.txt).
 */
static void generate_random_seed_interactive(Simulation* sim, SimulationConfig* config)
{
    SeedScenario scenario;
    int numRequests;
    int seedInput;
    int avgIntervalInput;
    unsigned int randomSeed;
    double avgInterArrival;
    int runNow;
    int suggestedInterval;

    memset(&scenario, 0, sizeof(scenario));

    printf("\n--- Generate random seed scenario ---\n");
    configure_interactively(config);
    if (!config_validate(config)) {
        printf("Invalid configuration.\n");
        return;
    }

    if (!read_int_in_range("How many random passenger requests? (0-100): ",
                           MIN_SEED_REQUESTS, MAX_SEED_REQUESTS, &numRequests)) {
        return;
    }

    if (!read_int_in_range("Random seed (0 = use current time): ", 0, 2147483647, &seedInput)) {
        return;
    }
    randomSeed = (unsigned int)seedInput;

    suggestedInterval = (int)(config->maxSimulationTime / (double)(numRequests + 1));
    if (suggestedInterval < 1) {
        suggestedInterval = (int)DEFAULT_AVG_INTER_ARRIVAL;
    }
    printf("Suggested avg seconds between arrivals: %d (spread over simulation)\n",
           suggestedInterval);
    if (!read_int_in_range(
            "Avg seconds between passenger arrivals (0 = use suggestion): ",
            0, 3600, &avgIntervalInput)) {
        return;
    }
    avgInterArrival = (avgIntervalInput > 0) ?
        (double)avgIntervalInput : (double)suggestedInterval;

    if (!seed_generate_random(&scenario, config, numRequests, randomSeed,
                              avgInterArrival)) {
        printf("Failed to generate random requests.\n");
        seed_scenario_free(&scenario);
        return;
    }

    if (!seed_save_to_file(&scenario, SEED_FILE_NAME)) {
        printf("Failed to save seed file.\n");
        seed_scenario_free(&scenario);
        return;
    }

    if (config_save(config, CONFIG_FILE_NAME)) {
        printf("Configuration also saved to %s\n", CONFIG_FILE_NAME);
    }

    printf("Saved %d random requests to %s (random_seed=%u)\n",
           scenario.numRequests, SEED_FILE_NAME, scenario.randomSeed);

    if (!read_int_in_range("Run simulation now? (1=yes, 0=no): ", 0, 1, &runNow)) {
        seed_scenario_free(&scenario);
        return;
    }

    if (runNow == 1) {
        if (!seed_apply_to_simulation(sim, &scenario)) {
            printf("Failed to apply seed scenario to simulation.\n");
        } else {
            *config = scenario.config;
            simulation_print_state(sim);
            simulation_run(sim);
            simulation_print_state(sim);
        }
    }

    seed_scenario_free(&scenario);
}

/*
 * load_seed_and_run_interactive - Menu option 7: load random_seed.txt and run DES.
 */
static void load_seed_and_run_interactive(Simulation* sim, SimulationConfig* config)
{
    SeedScenario scenario;

    memset(&scenario, 0, sizeof(scenario));

    printf("\n--- Load seed file and run ---\n");
    if (!seed_load_from_file(&scenario, SEED_FILE_NAME)) {
        printf("Failed to load %s\n", SEED_FILE_NAME);
        seed_scenario_free(&scenario);
        return;
    }

    *config = scenario.config;

    if (!seed_apply_to_simulation(sim, &scenario)) {
        printf("Failed to initialize simulation from seed file.\n");
        seed_scenario_free(&scenario);
        return;
    }

    printf("Loaded %d requests from %s\n", scenario.numRequests, SEED_FILE_NAME);
    simulation_print_state(sim);
    simulation_run(sim);
    simulation_print_state(sim);

    seed_scenario_free(&scenario);
}

/*
 * main - Menu only; DES engine is simulation_run() in simulation.c.
 *
 * DES is NOT real-time: simulation_run() jumps currentTime from event to event
 * (lowest T in Future Event List). No sleep() between events. See docs/DES_COURSE_HE.md.
 *
 * Option 2 loads config and inits sim without running.
 * Option 3 saves config from current sim or defaults.
 */
int main(void)
{
    Simulation sim;
    SimulationConfig config;
    int choice = 0;
    int running = 1;

    config_set_defaults(&config);
    memset(&sim, 0, sizeof(sim));

    logger_init();
    log_message(0.0, LOG_INFO, "Elevator DES foundation started");

    while (running) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid menu input.\n");
            while (getchar() != '\n') {
            }
            continue;
        }
        while (getchar() != '\n') {
        }

        switch (choice) {
        case 1:
            start_simulation_interactive(&sim, &config);
            break;
        case 2:
            if (config_load(&config, CONFIG_FILE_NAME)) {
                printf("Configuration loaded from %s\n", CONFIG_FILE_NAME);
                simulation_destroy(&sim);
                if (!simulation_init(&sim, &config)) {
                    printf("Failed to initialize simulation from config.\n");
                }
            }
            break;
        case 3:
            if (sim.numFloors > 0) {
                config = sim.config;
            }
            if (config_save(&config, CONFIG_FILE_NAME)) {
                printf("Configuration saved to %s\n", CONFIG_FILE_NAME);
            }
            break;
        case 4:
            add_passenger_interactive(&sim);
            break;
        case 5:
            if (sim.numFloors == 0) {
                printf("Simulation not initialized. Load config or start simulation.\n");
            } else {
                simulation_print_state(&sim);
            }
            break;
        case 6:
            generate_random_seed_interactive(&sim, &config);
            break;
        case 7:
            load_seed_and_run_interactive(&sim, &config);
            break;
        case 8:
            running = 0;
            break;
        default:
            printf("Invalid option. Choose 1-8.\n");
            break;
        }
    }

    simulation_destroy(&sim);
    logger_close();

    printf("Goodbye.\n");
    return 0;
}
