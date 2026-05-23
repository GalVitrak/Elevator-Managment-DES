#include "floor.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

void floor_init(Floor* floor, int floorNumber)
{
    floor->floorNumber = floorNumber;
    floor->upButtonPressed = 0;
    floor->downButtonPressed = 0;
    floor->waitingQueueFront = NULL;
    floor->waitingQueueRear = NULL;
}

static void floor_free_queue(Passenger* front)
{
    Passenger* current = front;
    while (current != NULL) {
        Passenger* next = current->next;
        passenger_destroy(current);
        current = next;
    }
}

void floor_destroy(Floor* floor)
{
    floor_free_queue(floor->waitingQueueFront);
    floor->waitingQueueFront = NULL;
    floor->waitingQueueRear = NULL;
}

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
