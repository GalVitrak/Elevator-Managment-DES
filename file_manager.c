/*
 * file_manager.c - Load/save simulation parameters (config.txt)
 */
#include "file_manager.h"
#include "constants.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>

/* config_set_defaults - Apply DEFAULT_* and DEFAULT_MAX_TIME from constants.h. */
void config_set_defaults(SimulationConfig* config)
{
    config->numFloors = DEFAULT_NUM_FLOORS;
    config->numElevators = DEFAULT_NUM_ELEVATORS;
    config->capacity = DEFAULT_CAPACITY;
    config->maxSimulationTime = DEFAULT_MAX_TIME;
}

/*
 * config_validate - Check all fields against MIN_* / MAX_* limits.
 * Returns 1 if valid, 0 if config is NULL or any field out of range.
 */
int config_validate(const SimulationConfig* config)
{
    if (config == NULL) {
        return 0;
    }
    if (config->numFloors < MIN_FLOORS || config->numFloors > MAX_FLOORS) {
        return 0;
    }
    if (config->numElevators < MIN_ELEVATORS || config->numElevators > MAX_ELEVATORS) {
        return 0;
    }
    if (config->capacity < MIN_CAPACITY || config->capacity > MAX_CAPACITY) {
        return 0;
    }
    if (config->maxSimulationTime <= 0.0) {
        return 0;
    }
    return 1;
}

/*
 * config_save - Write four key=value lines to filename.
 * Returns 0 if config invalid or file cannot be opened.
 */
int config_save(const SimulationConfig* config, const char* filename)
{
    FILE* file;

    if (config == NULL || filename == NULL) {
        return 0;
    }
    if (!config_validate(config)) {
        log_message(0.0, LOG_ERROR, "Cannot save invalid configuration");
        return 0;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        log_message(0.0, LOG_ERROR, "Failed to open config file for writing");
        return 0;
    }

    fprintf(file, "num_floors=%d\n", config->numFloors);
    fprintf(file, "num_elevators=%d\n", config->numElevators);
    fprintf(file, "capacity=%d\n", config->capacity);
    fprintf(file, "max_simulation_time=%.2f\n", config->maxSimulationTime);

    fclose(file);
    log_message(0.0, LOG_INFO, "Configuration saved successfully");
    return 1;
}

/* Return 1 if line starts with key and sscanf reads one integer after it. */
static int parse_int_line(const char* line, const char* key, int* value)
{
    int parsed;
    if (strncmp(line, key, strlen(key)) != 0) {
        return 0;
    }
    parsed = sscanf(line + strlen(key), "%d", value);
    return parsed == 1;
}

/* Return 1 if line starts with key and sscanf reads one double after it. */
static int parse_double_line(const char* line, const char* key, double* value)
{
    int parsed;
    if (strncmp(line, key, strlen(key)) != 0) {
        return 0;
    }
    parsed = sscanf(line + strlen(key), "%lf", value);
    return parsed == 1;
}

/*
 * config_load - Parse config file line by line; require all four keys.
 * Validates loaded values before returning success.
 */
int config_load(SimulationConfig* config, const char* filename)
{
    FILE* file;
    char line[MAX_NAME_LEN * 2];
    int floorsSet = 0;
    int elevatorsSet = 0;
    int capacitySet = 0;
    int timeSet = 0;

    if (config == NULL || filename == NULL) {
        return 0;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        log_message(0.0, LOG_ERROR, "Failed to open config file for reading");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (parse_int_line(line, "num_floors=", &config->numFloors)) {
            floorsSet = 1;
        } else if (parse_int_line(line, "num_elevators=", &config->numElevators)) {
            elevatorsSet = 1;
        } else if (parse_int_line(line, "capacity=", &config->capacity)) {
            capacitySet = 1;
        } else if (parse_double_line(line, "max_simulation_time=", &config->maxSimulationTime)) {
            timeSet = 1;
        }
    }

    fclose(file);

    if (!floorsSet || !elevatorsSet || !capacitySet || !timeSet) {
        log_message(0.0, LOG_ERROR, "Config file is missing required fields");
        return 0;
    }

    if (!config_validate(config)) {
        log_message(0.0, LOG_ERROR, "Loaded configuration values are invalid");
        return 0;
    }

    log_message(0.0, LOG_INFO, "Configuration loaded successfully");
    return 1;
}
