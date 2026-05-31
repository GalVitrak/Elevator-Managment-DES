/*
 * main.c - Program entry point and console menu
 *
 * PRESENTATION: No DES logic here — only I/O. Demo paths:
 *   1 = initialize manual sim   4 = print state (grid + FEL)
 *   5 = configure + write random_seed.txt   6 = load seed + simulation_run()
 *
 * DES engine lives in simulation.c.
 */
#include "simulation.h"
#include "constants.h"
#include "logger.h"
#include "random_seed.h"

#include <stdio.h>
#include <string.h>

/* print_menu - Display numbered options and prompt for selection. */
static void print_menu(void)
{
    printf("\n--- Elevator Management System (DES) ---\n");
    printf("1. Start new simulation (manual mode)\n");
    printf("2. Add passenger request manually\n");
    printf("3. Run current simulation\n");
    printf("4. Print system state\n");
    printf("5. Generate random seed file\n");
    printf("6. Load seed file and run simulation\n");
    printf("7. Exit\n");
    printf("Select option: ");
}

/*
 * read_int_in_range - Read one integer from stdin; retry until value is in [minVal, maxVal].
 */
static void read_int_in_range(const char* prompt, int minVal, int maxVal, int* out)
{
    int value;
    int readCount;

    for (;;) {
        printf("%s", prompt);
        readCount = scanf("%d", &value);
        while (getchar() != '\n') {
        }

        if (readCount != 1) {
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        if (value < minVal || value > maxVal) {
            printf("Value must be between %d and %d. Try again.\n", minVal, maxVal);
            continue;
        }

        *out = value;
        return;
    }
}

static void print_configuration_limits(void)
{
    printf("\n--- Allowed values ---\n");
    printf("  Floors above ground (ground=0 is extra): %d-%d\n",
           MIN_FLOORS_ABOVE_GROUND, MAX_FLOORS_ABOVE_GROUND);
    printf("  Underground floors (display -1 .. -N, 0=none): %d-%d\n",
           MIN_UNDERGROUND_FLOORS, MAX_UNDERGROUND_FLOORS);
    printf("  Max internal levels (above + underground): %d\n", MAX_TOTAL_FLOORS);
    printf("  Elevators: %d-%d\n", MIN_ELEVATORS, MAX_ELEVATORS);
    printf("  Capacity per elevator: %d-%d\n", MIN_CAPACITY, MAX_CAPACITY);
    printf("  Max simulation time (seconds): 1-100000\n");
    printf("  Random passenger requests (this menu): %d-%d\n",
           MIN_SEED_REQUESTS, MAX_SEED_REQUESTS);
}

/*
 * read_display_floor - Read a building floor number (ground=0, negative=basement).
 */
static int read_display_floor(const Simulation* sim, const char* prompt, int* outDisplay)
{
    int display;
    int minDisplay;
    int maxDisplay;

    if (sim == NULL || outDisplay == NULL) {
        return 0;
    }

    minDisplay = config_display_floor_min(&sim->config);
    maxDisplay = config_display_floor_max(&sim->config);

    read_int_in_range(prompt, minDisplay, maxDisplay, &display);

    if (!config_validate_display_floor(&sim->config, display)) {
        printf("Invalid floor for this building.\n");
        return 0;
    }

    *outDisplay = display;
    return 1;
}

/*
 * configure_interactively - Ask user for floors, elevators, capacity, max time.
 * Invalid entries are re-prompted (setup is not cancelled).
 */
static void configure_interactively(SimulationConfig* config)
{
    int floorsAboveGround;  /* user count excluding ground; ground (0) added below */
    int underground;
    int elevators;
    int capacity;
    int maxTimeInt;
    char prompt[96];

    config_set_defaults(config);

    printf("\n--- Building shape ---\n");
    printf("Ground is always floor 0. Enter how many floors are ABOVE ground (not counting ground).\n");
    printf("Example: 100 -> ground (0) plus floors 1..100 (101 levels total).\n");
    snprintf(prompt, sizeof(prompt),
             "Floors above ground (%d-%d): ",
             MIN_FLOORS_ABOVE_GROUND, MAX_FLOORS_ABOVE_GROUND);
    read_int_in_range(prompt, MIN_FLOORS_ABOVE_GROUND, MAX_FLOORS_ABOVE_GROUND,
                      &floorsAboveGround);

    printf("Basement levels use negative floor numbers (-1, -2, ...). Enter 0 for no basement.\n");
    snprintf(prompt, sizeof(prompt),
             "Underground floors below ground (%d-%d): ",
             MIN_UNDERGROUND_FLOORS, MAX_UNDERGROUND_FLOORS);
    read_int_in_range(prompt, MIN_UNDERGROUND_FLOORS, MAX_UNDERGROUND_FLOORS,
                      &underground);

    for (;;) {
        config->numFloors = floorsAboveGround + 1;
        config->numUndergroundFloors = underground;
        if (config_validate(config)) {
            break;
        }
        printf("Building too large (max %d internal levels). Adjust floors and try again.\n",
               MAX_TOTAL_FLOORS);
        snprintf(prompt, sizeof(prompt),
                 "Floors above ground (%d-%d): ",
                 MIN_FLOORS_ABOVE_GROUND, MAX_FLOORS_ABOVE_GROUND);
        read_int_in_range(prompt, MIN_FLOORS_ABOVE_GROUND, MAX_FLOORS_ABOVE_GROUND,
                          &floorsAboveGround);
        snprintf(prompt, sizeof(prompt),
                 "Underground floors below ground (%d-%d): ",
                 MIN_UNDERGROUND_FLOORS, MAX_UNDERGROUND_FLOORS);
        read_int_in_range(prompt, MIN_UNDERGROUND_FLOORS, MAX_UNDERGROUND_FLOORS,
                          &underground);
    }

    printf("Building: %d above ground + ground -> display floors %d .. %d",
           floorsAboveGround,
           config_display_floor_min(config),
           config_display_floor_max(config));
    if (underground > 0) {
        printf(" (plus basement %d .. -1)", -underground);
    }
    printf(" (%d internal levels).\n", config_total_internal_floors(config));

    snprintf(prompt, sizeof(prompt), "Number of elevators (%d-%d): ",
             MIN_ELEVATORS, MAX_ELEVATORS);
    read_int_in_range(prompt, MIN_ELEVATORS, MAX_ELEVATORS, &elevators);

    snprintf(prompt, sizeof(prompt), "Elevator capacity (%d-%d): ",
             MIN_CAPACITY, MAX_CAPACITY);
    read_int_in_range(prompt, MIN_CAPACITY, MAX_CAPACITY, &capacity);

    read_int_in_range("Max simulation time in seconds (1-100000): ", 1, 100000,
                      &maxTimeInt);

    config->numElevators = elevators;
    config->capacity = capacity;
    config->maxSimulationTime = (double)maxTimeInt;
}

/*
 * add_passenger_interactive - Menu option 2: one request if sim already initialized.
 * Requires prior option 1; does not run simulation_run by itself.
 */
static void add_passenger_interactive(Simulation* sim)
{
    int sourceDisplay;
    int destDisplay;
    int sourceIndex;
    int destIndex;

    if (sim == NULL || sim->numFloors == 0) {
        printf("Start a simulation first (option 1).\n");
        return;
    }

    if (!read_display_floor(sim, "Source floor (ground=0, negative=basement): ",
                            &sourceDisplay)) {
        return;
    }
    if (!read_display_floor(sim, "Destination floor: ", &destDisplay)) {
        return;
    }

    sourceIndex = config_display_to_index(&sim->config, sourceDisplay);
    destIndex = config_display_to_index(&sim->config, destDisplay);
    simulation_add_passenger_request(sim, sourceIndex, destIndex);
}

/*
 * start_simulation_interactive - Menu option 1: configure and initialize a fresh
 * manual simulation. Does not auto-run; user can add requests and run via option 3.
 */
static void start_simulation_interactive(Simulation* sim, SimulationConfig* config)
{
    configure_interactively(config);

    simulation_destroy(sim);
    if (!simulation_init(sim, config)) {
        printf("Failed to initialize simulation.\n");
        return;
    }

    printf("Simulation initialized. Add manual requests (option 2), then run (option 3).\n");
}

/*
 * generate_random_seed_interactive - Menu option 5: user picks building params and
 * request count; generates random trips and saves to random_seed.txt.
 */
static void generate_random_seed_interactive(Simulation* sim, SimulationConfig* config)
{
    SeedScenario scenario;
    int numRequests;
    double avgInterArrival;
    int runNow;

    memset(&scenario, 0, sizeof(scenario));

    printf("\n--- Generate random seed scenario ---\n");
    printf("This saves building settings + random passenger trips to %s\n", SEED_FILE_NAME);
    print_configuration_limits();
    configure_interactively(config);

    {
        char prompt[96];
        snprintf(prompt, sizeof(prompt),
                 "How many random passenger requests (%d-%d): ",
                 MIN_SEED_REQUESTS, MAX_SEED_REQUESTS);
        read_int_in_range(prompt, MIN_SEED_REQUESTS, MAX_SEED_REQUESTS, &numRequests);
    }

    avgInterArrival = seed_compute_auto_inter_arrival(config, numRequests);
    printf("Auto avg seconds between arrivals (from timing model): %.2f\n",
           avgInterArrival);

    /* randomSeed 0 => seed_generate_random uses clock; value is stored in the seed file */
    if (!seed_generate_random(&scenario, config, numRequests, 0)) {
        printf("Failed to generate random requests.\n");
        seed_scenario_free(&scenario);
        return;
    }

    if (!seed_save_to_file(&scenario, SEED_FILE_NAME)) {
        printf("Failed to save seed file.\n");
        seed_scenario_free(&scenario);
        return;
    }

    printf("Saved %d random requests to %s (clock seed %u — use option 6 to replay)\n",
           scenario.numRequests, SEED_FILE_NAME, scenario.randomSeed);

    read_int_in_range("Run simulation now? (1=yes, 0=no): ", 0, 1, &runNow);

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
 * load_seed_and_run_interactive - Menu option 6: load random_seed.txt and run DES.
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
 * run_current_simulation - Menu option 3: run initialized simulation.
 */
static void run_current_simulation(Simulation* sim)
{
    if (sim == NULL || sim->numFloors == 0) {
        printf("Simulation not initialized. Start a new simulation first (option 1).\n");
        return;
    }
    simulation_print_state(sim);
    simulation_run(sim);
    simulation_print_state(sim);
}

/*
 * main - Menu only; DES engine is simulation_run() in simulation.c.
 *
 * DES is NOT real-time: simulation_run() jumps currentTime from event to event
 * (lowest T in Future Event List). No sleep() between events. See docs/DES_COURSE_HE.md.
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
            add_passenger_interactive(&sim);
            break;
        case 3:
            run_current_simulation(&sim);
            break;
        case 4:
            if (sim.numFloors == 0) {
                printf("Simulation not initialized. Start a new simulation first.\n");
            } else {
                simulation_print_state(&sim);
            }
            break;
        case 5:
            generate_random_seed_interactive(&sim, &config);
            break;
        case 6:
            load_seed_and_run_interactive(&sim, &config);
            break;
        case 7:
            running = 0;
            break;
        default:
            printf("Invalid option. Choose 1-7.\n");
            break;
        }
    }

    simulation_destroy(&sim);
    logger_close();

    printf("Goodbye.\n");
    return 0;
}
