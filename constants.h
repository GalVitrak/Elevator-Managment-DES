/*
 * constants.h - Global preprocessor limits and default values
 *
 * All modules include this file for shared bounds (floors, elevators, etc.)
 * and default file names for logging and configuration.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

/* String buffer size used in log messages and config line parsing */
#define MAX_NAME_LEN        64

/* Default simulation horizon in simulation time units (seconds) */
#define DEFAULT_MAX_TIME    1000.0

/* Default maximum passengers per elevator cab */
#define DEFAULT_CAPACITY    10

/* Output file for simulation trace logs */
#define LOG_FILE_NAME       "simulation_log.txt"

/* Input/output file for building and simulation parameters */
#define CONFIG_FILE_NAME    "config.txt"

/* Saved random scenario: config + RNG seed + passenger request list */
#define SEED_FILE_NAME      "random_seed.txt"

/* Max passenger requests in one random seed file / generation */
#define MAX_SEED_REQUESTS   100
#define MIN_SEED_REQUESTS   0

/* Validation bounds for interactive input and config_load */
#define MIN_FLOORS          2
#define MAX_FLOORS          50
#define MIN_ELEVATORS       1
#define MAX_ELEVATORS       10
#define MIN_CAPACITY        1
#define MAX_CAPACITY        20

/* Values used when config_set_defaults() is called */
#define DEFAULT_NUM_FLOORS      5
#define DEFAULT_NUM_ELEVATORS   2

#endif /* CONSTANTS_H */
