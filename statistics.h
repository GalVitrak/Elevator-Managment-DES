#ifndef STATISTICS_H
#define STATISTICS_H

#include "passenger.h"

struct Simulation; /* defined in simulation.h */

/*
 * Accumulated metrics during DES (time-weighted queue and elevator busy times).
 */
typedef struct {
    int numElevators;
    double lastSampleTime;
    double queueLengthIntegral;
    int maxQueueLength;
    int totalRequests;
    int totalServed;
    int passengersWhoWaited;
    double sumWaitTime;
    double maxWaitTime;
    double sumTotalTripTime;
    double* elevatorBusyTime;
} SimulationStats;

void statistics_init(SimulationStats* stats, int numElevators);
void statistics_destroy(SimulationStats* stats);
void statistics_reset(SimulationStats* stats, int numElevators);

/* Integrate queue/elevator busy metrics from lastSampleTime to newTime. */
void statistics_advance_to_time(SimulationStats* stats, const struct Simulation* sim,
                                double newTime);

void statistics_on_passenger_request(SimulationStats* stats, const struct Simulation* sim);
void statistics_on_passenger_boarded(SimulationStats* stats, const Passenger* passenger,
                                   double boardTime);
void statistics_on_passenger_served(SimulationStats* stats, const Passenger* passenger,
                                    double exitTime);

/* Close integrals to sim end time and print formatted report (console + log file). */
void statistics_finalize_and_print(SimulationStats* stats, const struct Simulation* sim);

#endif /* STATISTICS_H */
