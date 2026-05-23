/*
 * elevator.c - Elevator cab state, floor stop plan (SCAN), dispatch helpers
 *
 * PRESENTATION: Show elevator_will_serve_call() for "on the way" pickup rules
 * (same direction as passenger, cab has not passed call floor yet).
 */
#include "elevator.h"
#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void elevator_stops_destroy(Elevator* elevator)
{
    if (elevator == NULL) {
        return;
    }
    free(elevator->floorStops);
    elevator->floorStops = NULL;
    elevator->numFloors = 0;
}

void elevator_init(Elevator* elevator, int id, int capacity, int numFloors)
{
    if (elevator == NULL) {
        return;
    }

    elevator_stops_destroy(elevator);
    elevator->id = id;
    elevator->currentFloor = 0;
    elevator->targetFloor = 0;
    elevator->direction = DIR_NONE;
    elevator->status = ELEVATOR_IDLE;
    elevator->doorState = DOOR_CLOSED;
    elevator->capacity = capacity;
    elevator->passengerCount = 0;
    elevator->onboardHead = NULL;
    elevator->numFloors = numFloors;

    if (numFloors > 0) {
        elevator->floorStops = (unsigned char*)calloc((size_t)numFloors,
                                                      sizeof(unsigned char));
    } else {
        elevator->floorStops = NULL;
    }
}

void elevator_add_stop(Elevator* elevator, int floor)
{
    if (elevator == NULL || elevator->floorStops == NULL) {
        return;
    }
    if (floor < 0 || floor >= elevator->numFloors) {
        return;
    }
    elevator->floorStops[floor]++;
}

void elevator_clear_stop(Elevator* elevator, int floor)
{
    if (elevator == NULL || elevator->floorStops == NULL) {
        return;
    }
    if (floor < 0 || floor >= elevator->numFloors) {
        return;
    }
    elevator->floorStops[floor] = 0;
}

int elevator_has_stop(const Elevator* elevator, int floor)
{
    if (elevator == NULL || elevator->floorStops == NULL) {
        return 0;
    }
    if (floor < 0 || floor >= elevator->numFloors) {
        return 0;
    }
    return elevator->floorStops[floor] > 0;
}

int elevator_has_any_stop(const Elevator* elevator)
{
    int floor;

    if (elevator == NULL || elevator->floorStops == NULL) {
        return 0;
    }

    for (floor = 0; floor < elevator->numFloors; floor++) {
        if (elevator->floorStops[floor] > 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * === PRESENTATION: On-the-way pickup eligibility ===
 * Example: cab going UP 0->20, call floor 15, dest 19 -> allowed if currentFloor <= 15.
 * Passenger going DOWN while cab goes UP -> rejected. Already passed call floor -> rejected.
 */
int elevator_will_serve_call(const Elevator* elevator, int callFloor, int destFloor)
{
    if (elevator == NULL) {
        return 0;
    }

    if (destFloor > callFloor) {
        if (elevator->direction == DIR_DOWN) {
            return 0;
        }
        if (elevator->currentFloor > callFloor) {
            return 0;
        }
        return 1;
    }
    if (destFloor < callFloor) {
        if (elevator->direction == DIR_UP) {
            return 0;
        }
        if (elevator->currentFloor < callFloor) {
            return 0;
        }
        return 1;
    }
    return 0;
}

int elevator_find_moving_for_call(Elevator* elevators, int count, int callFloor,
                                  int destFloor, int* outSlots)
{
    int i;
    int bestIndex = -1;
    int bestDistance = -1;

    if (elevators == NULL || count <= 0) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        Elevator* elev = &elevators[i];
        int distance;
        int slots;

        if (elev->status != ELEVATOR_MOVING || elev->doorState != DOOR_CLOSED) {
            continue;
        }
        if (!elevator_will_serve_call(elev, callFloor, destFloor)) {
            continue;
        }

        slots = elev->capacity - elev->passengerCount;
        if (slots <= 0) {
            continue;
        }

        distance = callFloor - elev->currentFloor;
        if (distance < 0) {
            distance = -distance;
        }
        if (bestIndex < 0 || distance < bestDistance) {
            bestIndex = i;
            bestDistance = distance;
            if (outSlots != NULL) {
                *outSlots = slots;
            }
        }
    }

    return bestIndex;
}

void elevator_clear_onboard(Elevator* elevator)
{
    Passenger* current;
    Passenger* next;

    if (elevator == NULL) {
        return;
    }

    current = elevator->onboardHead;
    while (current != NULL) {
        next = current->onboardNext;
        current->onboardNext = NULL;
        current = next;
    }
    elevator->onboardHead = NULL;
    elevator->passengerCount = 0;
}

void elevator_add_onboard(Elevator* elevator, Passenger* passenger)
{
    if (elevator == NULL || passenger == NULL) {
        return;
    }

    passenger->onboardNext = elevator->onboardHead;
    elevator->onboardHead = passenger;
    elevator->passengerCount++;
}

int elevator_onboard_count(const Elevator* elevator)
{
    return elevator == NULL ? 0 : elevator->passengerCount;
}

int elevator_next_stop_floor(const Elevator* elevator)
{
    int floor;
    int currentFloor;
    int bestUp = -1;
    int bestDown = -1;

    if (elevator == NULL || elevator->floorStops == NULL) {
        return -1;
    }

    currentFloor = elevator->currentFloor;

    for (floor = 0; floor < elevator->numFloors; floor++) {
        if (!elevator_has_stop(elevator, floor)) {
            continue;
        }
        if (floor > currentFloor) {
            if (bestUp < 0 || floor < bestUp) {
                bestUp = floor;
            }
        } else if (floor < currentFloor) {
            if (bestDown < 0 || floor > bestDown) {
                bestDown = floor;
            }
        }
    }

    if (elevator->direction == DIR_UP && bestUp >= 0) {
        return bestUp;
    }
    if (elevator->direction == DIR_DOWN && bestDown >= 0) {
        return bestDown;
    }
    /* No stops ahead in current direction: reverse (SCAN). */
    if (elevator->direction == DIR_UP && bestDown >= 0) {
        return bestDown;
    }
    if (elevator->direction == DIR_DOWN && bestUp >= 0) {
        return bestUp;
    }
    /* Idle / no direction: visit nearest stop (pickup before distant dest). */
    if (bestUp >= 0 || bestDown >= 0) {
        int bestFloor = -1;
        int bestDistance = -1;

        for (floor = 0; floor < elevator->numFloors; floor++) {
            int distance;

            if (!elevator_has_stop(elevator, floor) || floor == currentFloor) {
                continue;
            }
            distance = floor - currentFloor;
            if (distance < 0) {
                distance = -distance;
            }
            if (bestFloor < 0 || distance < bestDistance) {
                bestFloor = floor;
                bestDistance = distance;
            }
        }
        if (bestFloor >= 0) {
            return bestFloor;
        }
    }
    if (elevator_has_stop(elevator, currentFloor)) {
        return currentFloor;
    }
    return -1;
}

int elevator_find_nearest_idle(Elevator* elevators, int count, int targetFloor)
{
    int i;
    int bestIndex = -1;
    int bestDistance = -1;

    if (elevators == NULL || count <= 0) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        int distance;
        if (elevators[i].status != ELEVATOR_IDLE ||
            elevators[i].doorState != DOOR_CLOSED) {
            continue;
        }
        distance = elevators[i].currentFloor - targetFloor;
        if (distance < 0) {
            distance = -distance;
        }
        if (bestIndex < 0 || distance < bestDistance) {
            bestIndex = i;
            bestDistance = distance;
        }
    }

    return bestIndex;
}

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

double elevator_travel_time_seconds(int fromFloor, int toFloor)
{
    int distance = fromFloor - toFloor;
    if (distance < 0) {
        distance = -distance;
    }
    return (double)distance * SECONDS_PER_FLOOR;
}

void elevator_assign_to_floor(Elevator* elevator, int floor)
{
    if (elevator == NULL) {
        return;
    }

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
    case ELEVATOR_IDLE:           return "IDLE";
    case ELEVATOR_MOVING:         return "MOVING";
    case ELEVATOR_MAINTENANCE:    return "MAINTENANCE";
    case ELEVATOR_OUT_OF_SERVICE: return "OUT_OF_SERVICE";
    default:                      return "UNKNOWN";
    }
}

void elevator_print(const Elevator* elevator)
{
    int stopCount = 0;
    int floor;

    if (elevator == NULL) {
        return;
    }

    if (elevator->floorStops != NULL) {
        for (floor = 0; floor < elevator->numFloors; floor++) {
            if (elevator->floorStops[floor] > 0) {
                stopCount++;
            }
        }
    }

    printf("  Elevator %d: floor=%d target=%d dir=%s status=%s door=%s passengers=%d/%d stops=%d\n",
           elevator->id,
           elevator->currentFloor,
           elevator->targetFloor,
           direction_to_string(elevator->direction),
           status_to_string(elevator->status),
           elevator->doorState == DOOR_OPEN ? "OPEN" : "CLOSED",
           elevator->passengerCount,
           elevator->capacity,
           stopCount);
}
