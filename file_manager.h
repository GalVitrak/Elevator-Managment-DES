#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

typedef struct {
    int numFloors;
    int numElevators;
    int capacity;
    double maxSimulationTime;
} SimulationConfig;

void config_set_defaults(SimulationConfig* config);
int config_validate(const SimulationConfig* config);
int config_save(const SimulationConfig* config, const char* filename);
int config_load(SimulationConfig* config, const char* filename);

#endif /* FILE_MANAGER_H */
