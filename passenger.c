/*
 * passenger.c - Passenger entity (linked-list node for floor queues)
 */
#include "passenger.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * passenger_create - Allocate and initialize a new passenger.
 * Sets status to WAITING and next to NULL.
 * Returns NULL if malloc fails.
 */
Passenger* passenger_create(int id, int source, int destination, double requestTime)
{
    Passenger* passenger = (Passenger*)malloc(sizeof(Passenger));
    if (passenger == NULL) {
        return NULL;
    }

    passenger->id = id;
    passenger->sourceFloor = source;
    passenger->destinationFloor = destination;
    passenger->requestTime = requestTime;
    passenger->boardTime = -1.0;
    passenger->status = PASSENGER_WAITING;
    passenger->next = NULL;
    return passenger;
}

/* passenger_destroy - Free one passenger struct. */
void passenger_destroy(Passenger* passenger)
{
    free(passenger);
}

/* Convert passenger status enum to string for printing. */
static const char* status_to_string(PassengerStatus status)
{
    switch (status) {
    case PASSENGER_WAITING:       return "WAITING";
    case PASSENGER_IN_ELEVATOR:   return "IN_ELEVATOR";
    case PASSENGER_ARRIVED:       return "ARRIVED";
    default:                      return "UNKNOWN";
    }
}

/* passenger_print - Print id, route, status, and request time to stdout. */
void passenger_print(const Passenger* passenger)
{
    if (passenger == NULL) {
        printf("  (null passenger)\n");
        return;
    }

    printf("  Passenger %d: floor %d -> %d, status=%s, requested at t=%.2f\n",
           passenger->id,
           passenger->sourceFloor,
           passenger->destinationFloor,
           status_to_string(passenger->status),
           passenger->requestTime);
}
