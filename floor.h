#ifndef FLOOR_H
#define FLOOR_H

#include "passenger.h"

typedef struct {
    int floorNumber;
    int upButtonPressed;
    int downButtonPressed;
    Passenger* waitingQueueFront;
    Passenger* waitingQueueRear;
} Floor;

void floor_init(Floor* floor, int floorNumber);
void floor_destroy(Floor* floor);
void floor_enqueue_passenger(Floor* floor, Passenger* passenger);
Passenger* floor_dequeue_passenger(Floor* floor);
void floor_print_queue(const Floor* floor);
int floor_queue_size(const Floor* floor);

#endif /* FLOOR_H */
