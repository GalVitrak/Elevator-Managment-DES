#ifndef SIMULATION_H
#define SIMULATION_H

#include "elevator.h"
#include "floor.h"
#include "event.h"
#include "file_manager.h"

typedef struct {
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
    Passenger** activePassengersByElevator; /* one tracked passenger per elevator (foundation) */
} Simulation;

int simulation_init(Simulation* sim, const SimulationConfig* config);
void simulation_destroy(Simulation* sim);
void simulation_reset(Simulation* sim);
int simulation_run(Simulation* sim);
void simulation_add_passenger_request(Simulation* sim, int sourceFloor,
                                      int destinationFloor);
void simulation_print_state(const Simulation* sim);
int simulation_validate_floor(const Simulation* sim, int floor);

void handle_passenger_call(Simulation* sim, Event* event);
void handle_elevator_arrival(Simulation* sim, Event* event);
void handle_doors_open(Simulation* sim, Event* event);
void handle_doors_close(Simulation* sim, Event* event);
void handle_passenger_exit(Simulation* sim, Event* event);

#endif /* SIMULATION_H */
