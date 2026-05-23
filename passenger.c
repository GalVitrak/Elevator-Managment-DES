#include "passenger.h"

#include <stdio.h>
#include <stdlib.h>

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
    passenger->status = PASSENGER_WAITING;
    passenger->next = NULL;
    return passenger;
}

void passenger_destroy(Passenger* passenger)
{
    free(passenger);
}

static const char* status_to_string(PassengerStatus status)
{
    switch (status) {
    case PASSENGER_WAITING:       return "WAITING";
    case PASSENGER_IN_ELEVATOR:   return "IN_ELEVATOR";
    case PASSENGER_ARRIVED:       return "ARRIVED";
    default:                      return "UNKNOWN";
    }
}

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
