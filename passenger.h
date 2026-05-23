#ifndef PASSENGER_H
#define PASSENGER_H

typedef enum {
    PASSENGER_WAITING,
    PASSENGER_IN_ELEVATOR,
    PASSENGER_ARRIVED
} PassengerStatus;

typedef struct Passenger {
    int id;
    int sourceFloor;
    int destinationFloor;
    double requestTime;
    PassengerStatus status;
    struct Passenger* next;
} Passenger;

Passenger* passenger_create(int id, int source, int destination, double requestTime);
void passenger_destroy(Passenger* passenger);
void passenger_print(const Passenger* passenger);

#endif /* PASSENGER_H */
