#ifndef ELEVATOR_H
#define ELEVATOR_H

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;

typedef enum {
    ELEVATOR_IDLE,
    ELEVATOR_MOVING,
    ELEVATOR_MAINTENANCE,       /* reserved for phase 2 */
    ELEVATOR_OUT_OF_SERVICE     /* reserved for phase 2 */
} ElevatorStatus;

typedef enum {
    DOOR_OPEN,
    DOOR_CLOSED
} DoorState;

/*
 * One elevator cab and its runtime state.
 * passengerCount must not exceed capacity (enforced in phase 2).
 */
typedef struct {
    int id;
    int currentFloor;
    int targetFloor;
    Direction direction;
    ElevatorStatus status;
    DoorState doorState;
    int capacity;
    int passengerCount;
} Elevator;

/* Set elevator to idle at floor 0 with closed doors and zero passengers. */
void elevator_init(Elevator* elevator, int id, int capacity);

/*
 * Find the first idle elevator with closed doors.
 * Returns elevator index, or -1 if every cab is busy or unavailable.
 */
int elevator_find_first_idle(Elevator* elevators, int count);

/*
 * Send elevator toward floor (phase 1: instant move to that floor).
 * Updates direction, targetFloor, and currentFloor immediately.
 * TODO phase 2: schedule arrival event after travel delay instead.
 */
void elevator_assign_to_floor(Elevator* elevator, int floor);

/* Print one line describing this elevator (debug / menu option 5). */
void elevator_print(const Elevator* elevator);

#endif /* ELEVATOR_H */
