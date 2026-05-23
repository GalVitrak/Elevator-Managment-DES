/*
 * floor.c - Per-floor FIFO waiting queue (linked list of passengers)
 * PRESENTATION: floor_enqueue_passenger / floor_take_assigned_passengers — course linked list.
 */
#include "floor.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

/* floor_init - Empty queue, clear hall buttons, store floor index. */
void floor_init(Floor* floor, int floorNumber)
{
    floor->floorNumber = floorNumber;
    floor->upButtonPressed = 0;
    floor->downButtonPressed = 0;
    floor->waitingQueueFront = NULL;
    floor->waitingQueueRear = NULL;
}

/* Free every passenger node in a queue chain (used by floor_destroy). */
static void floor_free_queue(Passenger* front)
{
    Passenger* current = front;
    while (current != NULL) {
        Passenger* next = current->next;
        passenger_destroy(current);
        current = next;
    }
}

/* floor_destroy - Destroy all waiting passengers and reset queue pointers. */
void floor_destroy(Floor* floor)
{
    floor_free_queue(floor->waitingQueueFront);
    floor->waitingQueueFront = NULL;
    floor->waitingQueueRear = NULL;
}

/*
 * floor_enqueue_passenger - Add passenger at tail of FIFO queue.
 * Does nothing if floor or passenger is NULL.
 */
void floor_enqueue_passenger(Floor* floor, Passenger* passenger)
{
    if (floor == NULL || passenger == NULL) {
        return;
    }

    passenger->next = NULL;

    if (floor->waitingQueueRear == NULL) {
        floor->waitingQueueFront = passenger;
        floor->waitingQueueRear = passenger;
    } else {
        floor->waitingQueueRear->next = passenger;
        floor->waitingQueueRear = passenger;
    }
}

/*
 * floor_dequeue_passenger - Remove front of queue.
 * Returns NULL if empty; detached node has next = NULL.
 */
Passenger* floor_dequeue_passenger(Floor* floor)
{
    Passenger* passenger;

    if (floor == NULL || floor->waitingQueueFront == NULL) {
        return NULL;
    }

    passenger = floor->waitingQueueFront;
    floor->waitingQueueFront = passenger->next;
    passenger->next = NULL;

    if (floor->waitingQueueFront == NULL) {
        floor->waitingQueueRear = NULL;
    }

    return passenger;
}

Passenger* floor_take_assigned_passengers(Floor* floor, int elevatorId, int maxCount)
{
    Passenger* takenHead;
    Passenger* takenTail;
    Passenger* current;
    Passenger* previous;

    if (floor == NULL || elevatorId < 0 || maxCount <= 0) {
        return NULL;
    }

    takenHead = NULL;
    takenTail = NULL;
    previous = NULL;
    current = floor->waitingQueueFront;

    while (current != NULL && maxCount > 0) {
        Passenger* next = current->next;

        if (current->assignedElevatorId == elevatorId) {
            current->next = NULL;
            current->assignedElevatorId = -1;

            if (takenHead == NULL) {
                takenHead = current;
                takenTail = current;
            } else {
                takenTail->next = current;
                takenTail = current;
            }

            if (previous == NULL) {
                floor->waitingQueueFront = next;
            } else {
                previous->next = next;
            }
            if (next == NULL) {
                floor->waitingQueueRear = previous;
            }

            maxCount--;
        } else {
            previous = current;
        }

        current = next;
    }

    return takenHead;
}

/* floor_queue_size - Count nodes by walking the list. */
int floor_queue_size(const Floor* floor)
{
    int count = 0;
    const Passenger* current;

    if (floor == NULL) {
        return 0;
    }

    for (current = floor->waitingQueueFront; current != NULL; current = current->next) {
        count++;
    }
    return count;
}

/* floor_print_queue - Header line plus one line per waiting passenger. */
void floor_print_queue(const Floor* floor)
{
    const Passenger* current;

    printf("  Floor %d queue (%d waiting): up=%d down=%d\n",
           floor->floorNumber,
           floor_queue_size(floor),
           floor->upButtonPressed,
           floor->downButtonPressed);

    for (current = floor->waitingQueueFront; current != NULL; current = current->next) {
        passenger_print(current);
    }
}
