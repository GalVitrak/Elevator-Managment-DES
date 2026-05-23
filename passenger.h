#ifndef PASSENGER_H
#define PASSENGER_H

/*
 * Passenger status in the DES lifecycle.
 * WAITING: in a floor queue; IN_ELEVATOR: on a cab; ARRIVED: trip complete.
 */
typedef enum {
    PASSENGER_WAITING,
    PASSENGER_IN_ELEVATOR,
    PASSENGER_ARRIVED
} PassengerStatus;

/*
 * A single passenger (linked-list node for floor waiting queues).
 * next: used when passenger is waiting on a floor; NULL when not in a queue.
 */
typedef struct Passenger {
    int id;
    int sourceFloor;
    int destinationFloor;
    double requestTime;
    PassengerStatus status;
    struct Passenger* next;
} Passenger;

/* Allocate a new passenger; returns NULL on malloc failure. */
Passenger* passenger_create(int id, int source, int destination, double requestTime);

/* Free one passenger node (must not be in a queue unless caller removed it). */
void passenger_destroy(Passenger* passenger);

/* Print passenger fields to stdout (debug / menu option 5). */
void passenger_print(const Passenger* passenger);

#endif /* PASSENGER_H */
