#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

/*
 * Simulation parameters loaded from / saved to config.txt.
 * All fields are validated by config_validate() before use.
 */
typedef struct {
    int numFloors;              /* total at/above ground incl. 0 (e.g. 101 => floors 0..100) */
    int numUndergroundFloors;   /* basement levels: display -1 .. -numUnderground */
    int numElevators;
    int capacity;
    double maxSimulationTime;
} SimulationConfig;

/* Internal shaft indices: 0 .. config_total_internal_floors()-1 */
int config_total_internal_floors(const SimulationConfig* config);
int config_display_floor_min(const SimulationConfig* config);
int config_display_floor_max(const SimulationConfig* config);
int config_display_to_index(const SimulationConfig* config, int displayFloor);
int config_index_to_display(const SimulationConfig* config, int index);
int config_validate_display_floor(const SimulationConfig* config, int displayFloor);

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
