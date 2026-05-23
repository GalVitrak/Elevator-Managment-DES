/*
 * elevator.c - Elevator cab state and simple first-idle dispatch helpers
 */
#include "elevator.h"
#include "logger.h"

#include <stdio.h>

/* elevator_init - Start cab at floor 0, idle, doors closed, empty. */
void elevator_init(Elevator* elevator, int id, int capacity)
{
    elevator->id = id;
    elevator->currentFloor = 0;
    elevator->targetFloor = 0;
    elevator->direction = DIR_NONE;
    elevator->status = ELEVATOR_IDLE;
    elevator->doorState = DOOR_CLOSED;
    elevator->capacity = capacity;
    elevator->passengerCount = 0;
}

/*
 * elevator_find_first_idle - Linear search for an available cab.
 * Requires IDLE status and CLOSED doors (not boarding/alighting).
 */
int elevator_find_first_idle(Elevator* elevators, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        if (elevators[i].status == ELEVATOR_IDLE &&
            elevators[i].doorState == DOOR_CLOSED) {
            return i;
        }
    }
    return -1;
}

/*
 * elevator_assign_to_floor - Move cab to target floor (phase 1: instantaneous).
 * Sets direction from current vs target, then sets currentFloor = floor immediately.
 * TODO phase 2: keep MOVING until a later ELEVATOR_ARRIVAL event fires.
 */
void elevator_assign_to_floor(Elevator* elevator, int floor)
{
    elevator->targetFloor = floor;
    elevator->status = ELEVATOR_MOVING;

    if (floor > elevator->currentFloor) {
        elevator->direction = DIR_UP;
    } else if (floor < elevator->currentFloor) {
        elevator->direction = DIR_DOWN;
    } else {
        elevator->direction = DIR_NONE;
    }

    /* TODO: realistic elevator movement - travel time per floor, acceleration */
    elevator->currentFloor = floor;
    elevator->status = ELEVATOR_MOVING;
}

static const char* direction_to_string(Direction direction)
{
    switch (direction) {
    case DIR_UP:   return "UP";
    case DIR_DOWN: return "DOWN";
    case DIR_NONE: return "NONE";
    default:       return "UNKNOWN";
    }
}

static const char* status_to_string(ElevatorStatus status)
{
    switch (status) {
    case ELEVATOR_IDLE:          return "IDLE";
    case ELEVATOR_MOVING:        return "MOVING";
    case ELEVATOR_MAINTENANCE:   return "MAINTENANCE";
    case ELEVATOR_OUT_OF_SERVICE: return "OUT_OF_SERVICE";
    default:                     return "UNKNOWN";
    }
}

/* elevator_print - One-line status for debug output. */
void elevator_print(const Elevator* elevator)
{
    printf("  Elevator %d: floor=%d target=%d dir=%s status=%s door=%s passengers=%d/%d\n",
           elevator->id,
           elevator->currentFloor,
           elevator->targetFloor,
           direction_to_string(elevator->direction),
           status_to_string(elevator->status),
           elevator->doorState == DOOR_OPEN ? "OPEN" : "CLOSED",
           elevator->passengerCount,
           elevator->capacity);
}
