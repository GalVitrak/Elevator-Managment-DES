/*
 * constants.h - Global preprocessor limits and default values
 *
 * PRESENTATION: Show MAX_QUEUE_WAIT_SECONDS (180 s SLA), SECONDS_PER_FLOOR,
 * door timings, and DEST_CLUSTER_* spans — tunables for dispatch behavior.
 * All modules include this file for shared bounds and output file names.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

/* String buffer size used in log messages and config line parsing */
#define MAX_NAME_LEN        64

/* Default simulation horizon in simulation time units (seconds) */
#define DEFAULT_MAX_TIME    1000.0

/* Default maximum passengers per elevator cab */
#define DEFAULT_CAPACITY    10

/* Simulated seconds to travel one floor (DES movement model) */
#define SECONDS_PER_FLOOR           1.0

/* Door timing at each floor stop (seconds) */
#define DOOR_OPEN_TIME_SECONDS      0.5
#define DOOR_DWELL_SECONDS          3.0   /* doors stay open for boarding / alighting */
#define DOOR_CLOSE_TIME_SECONDS     0.5

/* Destination clustering span (floors); dynamic dispatch picks by queue wait */
#define DEST_CLUSTER_MAX_SPAN_FLOORS  5   /* default / medium wait */
#define DEST_CLUSTER_SPAN_LOOSE       10  /* low wait — higher throughput */
#define DEST_CLUSTER_SPAN_TIGHT       3   /* high wait — lower max wait */

/* Target max time from call until boarding (seconds); dispatch escalates beyond this */
#define MAX_QUEUE_WAIT_SECONDS        180.0

/* On-the-way pickup only if ETA + current wait stays below this (seconds) */
#define MOVING_PICKUP_MAX_SUM_WAIT    120.0

/* Dispatch scoring: minimize ETA + load penalty - wait bonus */
#define DISPATCH_ETA_LOAD_WEIGHT      3.0
#define DISPATCH_WAIT_BONUS           0.15  /* subtract per second waited (lower score) */
#define DISPATCH_ZONE_PENALTY_SEC     40.0  /* soft penalty if cab outside its zone */
#define DISPATCH_MIN_FLOORS_FOR_ZONES 40    /* building must have this many levels */

/* Idle cabs with no work drift toward a high-demand floor after doors close */
#define IDLE_REPOSITION_MIN_QUEUE     1

/* End-of-run summary report (bank-style statistics) */
#define STATS_REPORT_FILE   "simulation_results.txt"

/* Output file for simulation trace logs */
#define LOG_FILE_NAME       "simulation_log.txt"

/* Input/output file for building and simulation parameters */
#define CONFIG_FILE_NAME    "config.txt"

/* Saved random scenario: config + RNG seed + passenger request list */
#define SEED_FILE_NAME      "random_seed.txt"

/* Max passenger requests in one random seed file / generation (stress tests) */
#define MAX_SEED_REQUESTS   2000
#define MIN_SEED_REQUESTS   0

/*
 * Internal/config num_floors = ground (0) + floors above ground.
 * Interactive input asks for floors above ground only; ground is added automatically.
 * Example: user enters 100 -> num_floors=101, display floors 0..100.
 */
#define MIN_FLOORS              1   /* at least ground only (internal) */
#define MAX_FLOORS              151 /* ground + up to 150 above = display 0..150 */
#define MIN_FLOORS_ABOVE_GROUND 0
#define MAX_FLOORS_ABOVE_GROUND 150 /* user enters 150 -> floors 0..150 (151 levels) */

/* Basement levels below ground: display floors -1 .. -numUnderground */
#define MIN_UNDERGROUND_FLOORS  0
#define MAX_UNDERGROUND_FLOORS  20

/* Hard cap on internal floor array size (above-ground count + underground) */
#define MAX_TOTAL_FLOORS        171
#define MIN_ELEVATORS       1
#define MAX_ELEVATORS       100
#define MIN_CAPACITY        1
#define MAX_CAPACITY        20

/* Values used when config_set_defaults() is called */
#define DEFAULT_NUM_FLOORS      5
#define DEFAULT_NUM_ELEVATORS   2

#endif /* CONSTANTS_H */
