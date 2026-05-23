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
} Elevator;

void elevator_init(Elevator* elevator, int id, int capacity);
int elevator_find_first_idle(Elevator* elevators, int count);
void elevator_assign_to_floor(Elevator* elevator, int floor);
void elevator_print(const Elevator* elevator);

#endif /* ELEVATOR_H */
