#ifndef SIMULATION_H
#define SIMULATION_H

#include "building_grid.h"
#include "elevator.h"
#include "floor.h"
#include "event.h"
#include "file_manager.h"
#include "statistics.h"

/*
 * PRESENTATION: Root struct — ties together FEL, floors, elevators, stats, grid.
 * Passengers on each cab: Elevator.onboardHead (ride-sharing linked list).
 */
typedef struct Simulation {
    double currentTime;
    double maxSimulationTime;
    int numFloors;          /* internal indices: 0 .. numFloors-1 */
    int groundFloorIndex;   /* internal index for display floor 0 */
    int numElevators;
    int elevatorCapacity;
    Elevator* elevators;
    Floor* floors;
    EventList eventList;
    int nextPassengerId;
    SimulationConfig config;
    SimulationStats stats;
    int nextDispatchFloor;
    BuildingGrid buildingView;  /* dynamic elevators x floors matrix (display) */
    int numZones;               /* floor bands for zone-biased dispatch (1 = off) */
    int* floorDemand;           /* call count per floor index (idle reposition) */
} Simulation;

/*
 * Allocate elevators, floors, and FEL; apply config.
 * Returns 1 on success, 0 if config invalid or allocation fails.
 */
int simulation_init(Simulation* sim, const SimulationConfig* config);

/* Free all dynamic memory held by sim (safe if partially initialized). */
void simulation_destroy(Simulation* sim);

/* Re-run init with the same config after destroy. */
void simulation_reset(Simulation* sim);

/*
 * DES main loop: pop earliest events until FEL empty or time >= maxSimulationTime.
 * Returns 1 if sim pointer valid, 0 if NULL.
 */
int simulation_run(Simulation* sim);

/*
 * Create passenger, enqueue on source floor, schedule PASSENGER_CALL at currentTime.
 * Ignores invalid floors or same source/destination.
 */
void simulation_add_passenger_request(Simulation* sim, int sourceFloor,
                                      int destinationFloor);

/*
 * Schedule a passenger to arrive at arrivalTime (DES event only — not all at t=0).
 * Passenger is created when the call event fires.
 */
void simulation_schedule_passenger_arrival(Simulation* sim, double arrivalTime,
                                           int sourceFloor, int destinationFloor);

/* Print elevators, floor queues, FEL; optional building grid (menu option 4). */
void simulation_print_state(const Simulation* sim, int showGrid);

/* Return 1 if floor is in [0, numFloors); 0 otherwise. */
int simulation_validate_floor(const Simulation* sim, int floor);

/* --- Event handlers (called from simulation_dispatch_event) --- */

/* Assign idle elevator to passenger waiting on event->floor. */
void handle_passenger_call(Simulation* sim, Event* event);

/* Elevator reached event->floor; schedule doors open. */
void handle_elevator_arrival(Simulation* sim, Event* event);

/* Open doors: board waiting passenger or schedule exit at destination. */
void handle_doors_open(Simulation* sim, Event* event);

/* Close doors; if passenger on board, move toward destination floor. */
void handle_doors_close(Simulation* sim, Event* event);

/* Passenger leaves cab; free passenger and close doors again. */
void handle_passenger_exit(Simulation* sim, Event* event);

#endif /* SIMULATION_H */
