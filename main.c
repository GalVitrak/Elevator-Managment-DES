#include "simulation.h"
#include "constants.h"
#include "logger.h"
#include "file_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_menu(void)
{
    printf("\n--- Elevator Management System (DES) ---\n");
    printf("1. Start new simulation\n");
    printf("2. Load configuration\n");
    printf("3. Save configuration\n");
    printf("4. Add passenger request manually\n");
    printf("5. Print system state\n");
    printf("6. Exit\n");
    printf("Select option: ");
}

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
            running = 0;
            break;
        default:
            printf("Invalid option. Choose 1-6.\n");
            break;
        }
    }

    simulation_destroy(&sim);
    logger_close();

    printf("Goodbye.\n");
    return 0;
}
