#ifndef SIMULATION_H
#define SIMULATION_H

#include "elevator.h"
#include "floor.h"
#include "event.h"
#include "file_manager.h"
#include "statistics.h"

/*
 * Central simulation state: clock, building, Future Event List, and tracking.
 * activePassengersByElevator[i]: passenger currently on elevator i (phase 1: at most one).
 */
typedef struct Simulation {
    double currentTime;
    double maxSimulationTime;
    int numFloors;
    int numElevators;
    int elevatorCapacity;
    Elevator* elevators;
    Floor* floors;
    EventList eventList;
    int nextPassengerId;
    SimulationConfig config;
    Passenger** activePassengersByElevator;
    SimulationStats stats;
    int nextDispatchElevator;   /* round-robin index for fair cab assignment */
    int nextDispatchFloor;      /* round-robin start when picking a waiting queue */
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

/* Print elevators, all floor queues, and the FEL (menu option 5). */
void simulation_print_state(const Simulation* sim);

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
