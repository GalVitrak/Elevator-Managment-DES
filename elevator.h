#ifndef ELEVATOR_H
#define ELEVATOR_H

/*
 * elevator.h - Cab state: floor, direction, door, capacity, stop mask, onboard list.
 * PRESENTATION: elevator_will_serve_call() declared in elevator.c (on-the-way rules).
 */

#include "passenger.h"

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;

typedef enum {
    ELEVATOR_IDLE,
    ELEVATOR_MOVING,
    ELEVATOR_MAINTENANCE,
    ELEVATOR_OUT_OF_SERVICE
} ElevatorStatus;

typedef enum {
    DOOR_OPEN,
    DOOR_CLOSED
} DoorState;

typedef struct {
    int id;
    int currentFloor;
    int targetFloor;
    Direction direction;
    ElevatorStatus status;
    DoorState doorState;
    int capacity;
    int passengerCount;
    int numFloors;
    unsigned char* floorStops;  /* dynamic [numFloors]: non-zero => must stop */
    Passenger* onboardHead;
} Elevator;

void elevator_init(Elevator* elevator, int id, int capacity, int numFloors);
void elevator_stops_destroy(Elevator* elevator);

int elevator_find_first_idle(Elevator* elevators, int count);
int elevator_find_idle_round_robin(Elevator* elevators, int count, int* nextStartIndex);
int elevator_find_nearest_idle(Elevator* elevators, int count, int targetFloor);

/*
 * Moving cab that will pass callFloor in the same direction (SCAN pickup on the way).
 * Returns -1 if none; optional outSlots = remaining capacity for new waiters.
 */
int elevator_find_moving_for_call(Elevator* elevators, int count, int callFloor,
                                  int destFloor, int* outSlots);

void elevator_clear_onboard(Elevator* elevator);
void elevator_add_onboard(Elevator* elevator, Passenger* passenger);
int elevator_onboard_count(const Elevator* elevator);

void elevator_add_stop(Elevator* elevator, int floor);
void elevator_clear_stop(Elevator* elevator, int floor);
int elevator_has_stop(const Elevator* elevator, int floor);
int elevator_has_any_stop(const Elevator* elevator);

/* Nearest stop ahead in current direction (SCAN); -1 if none. */
int elevator_next_stop_floor(const Elevator* elevator);

int elevator_will_serve_call(const Elevator* elevator, int callFloor, int destFloor);

void elevator_assign_to_floor(Elevator* elevator, int floor);
double elevator_travel_time_seconds(int fromFloor, int toFloor);
void elevator_print(const Elevator* elevator);

#endif /* ELEVATOR_H */
