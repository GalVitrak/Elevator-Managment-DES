#ifndef FLOOR_H
#define FLOOR_H

/*
 * floor.h - One floor's hall buttons and FIFO waiting queue (linked list).
 */

#include "passenger.h"

/*
 * One building floor with a FIFO waiting queue and hall-call flags.
 * waitingQueueFront/Rear: doubly-ended queue via singly linked Passenger list.
 */
typedef struct {
    int floorNumber;
    int upButtonPressed;
    int downButtonPressed;
    Passenger* waitingQueueFront;
    Passenger* waitingQueueRear;
} Floor;

/* Initialize empty queue and clear hall buttons. */
void floor_init(Floor* floor, int floorNumber);

/* Free all passengers still waiting on this floor. */
void floor_destroy(Floor* floor);

/* Append passenger to tail of waiting queue - O(1). */
void floor_enqueue_passenger(Floor* floor, Passenger* passenger);

/* Remove and return front passenger, or NULL if queue empty - O(1). */
Passenger* floor_dequeue_passenger(Floor* floor);

/* Print floor number, queue size, buttons, and each waiting passenger. */
void floor_print_queue(const Floor* floor);

/* Count passengers in queue (walks the list). */
int floor_queue_size(const Floor* floor);

/*
 * Remove up to maxCount passengers assigned to elevatorId (FIFO within assignment).
 * Returns head of detached list; caller owns nodes.
 */
Passenger* floor_take_assigned_passengers(Floor* floor, int elevatorId, int maxCount);

#endif /* FLOOR_H */
