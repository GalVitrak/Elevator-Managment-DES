#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

/*
 * Simulation parameters loaded from / saved to config.txt.
 * All fields are validated by config_validate() before use.
 */
typedef struct {
    int numFloors;
    int numElevators;
    int capacity;
    double maxSimulationTime;
} SimulationConfig;

/* Fill config with DEFAULT_* values from constants.h. */
void config_set_defaults(SimulationConfig* config);

/* Return 1 if all fields are within MIN/MAX bounds; 0 otherwise. */
int config_validate(const SimulationConfig* config);

/*
 * Write config to a text file (key=value lines).
 * Returns 1 on success, 0 if invalid config or file cannot be opened.
 */
int config_save(const SimulationConfig* config, const char* filename);

/*
 * Read config from file; all four keys must be present.
 * Returns 1 on success, 0 on I/O error, missing keys, or invalid values.
 */
int config_load(SimulationConfig* config, const char* filename);

#endif /* FILE_MANAGER_H */
