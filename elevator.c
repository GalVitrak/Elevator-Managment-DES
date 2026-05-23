/*
 * elevator.c - Elevator cab state and simple first-idle dispatch helpers
 */
#include "elevator.h"
#include "constants.h"
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
    int start = 0;
    return elevator_find_idle_round_robin(elevators, count, &start);
}

int elevator_find_idle_round_robin(Elevator* elevators, int count, int* nextStartIndex)
{
    int i;
    int start;

    if (elevators == NULL || count <= 0 || nextStartIndex == NULL) {
        return -1;
    }

    start = *nextStartIndex % count;
    for (i = 0; i < count; i++) {
        int idx = (start + i) % count;
        if (elevators[idx].status == ELEVATOR_IDLE &&
            elevators[idx].doorState == DOOR_CLOSED) {
            *nextStartIndex = (idx + 1) % count;
            return idx;
        }
    }
    return -1;
}

/*
 * elevator_travel_time_seconds - DES delay for moving between two floors.
 */
double elevator_travel_time_seconds(int fromFloor, int toFloor)
{
    int distance = fromFloor - toFloor;
    if (distance < 0) {
        distance = -distance;
    }
    return (double)distance * SECONDS_PER_FLOOR;
}

/*
 * elevator_assign_to_floor - Begin trip to floor; cab stays at currentFloor until arrival event.
 */
void elevator_assign_to_floor(Elevator* elevator, int floor)
{
    elevator->targetFloor = floor;
    elevator->status = ELEVATOR_MOVING;
    elevator->doorState = DOOR_CLOSED;

    if (floor > elevator->currentFloor) {
        elevator->direction = DIR_UP;
    } else if (floor < elevator->currentFloor) {
        elevator->direction = DIR_DOWN;
    } else {
        elevator->direction = DIR_NONE;
    }
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
