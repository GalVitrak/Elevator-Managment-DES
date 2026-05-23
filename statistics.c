/*
 * statistics.c - End-of-simulation performance report (bank-style summary)
 */
#include "statistics.h"
#include "simulation.h"
#include "constants.h"
#include "logger.h"
#include "floor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATS_WAIT_EPSILON 0.01

static int statistics_total_queue_length(const Simulation* sim)
{
    int total = 0;
    int i;

    if (sim == NULL || sim->floors == NULL) {
        return 0;
    }

    for (i = 0; i < sim->numFloors; i++) {
        total += floor_queue_size(&sim->floors[i]);
    }
    return total;
}

static void statistics_update_max_queue(SimulationStats* stats, const Simulation* sim)
{
    int total = statistics_total_queue_length(sim);

    if (total > stats->maxQueueLength) {
        stats->maxQueueLength = total;
    }
}

void statistics_init(SimulationStats* stats, int numElevators)
{
    memset(stats, 0, sizeof(*stats));
    stats->numElevators = numElevators;
    stats->maxWaitTime = 0.0;
    if (numElevators > 0) {
        stats->elevatorBusyTime = (double*)calloc((size_t)numElevators, sizeof(double));
    }
}

void statistics_destroy(SimulationStats* stats)
{
    if (stats == NULL) {
        return;
    }
    free(stats->elevatorBusyTime);
    stats->elevatorBusyTime = NULL;
    stats->numElevators = 0;
}

void statistics_reset(SimulationStats* stats, int numElevators)
{
    statistics_destroy(stats);
    statistics_init(stats, numElevators);
}

void statistics_advance_to_time(SimulationStats* stats, const Simulation* sim, double newTime)
{
    double delta;
    int totalQueue;
    int i;

    if (stats == NULL || sim == NULL) {
        return;
    }

    delta = newTime - stats->lastSampleTime;
    if (delta < 0.0) {
        return;
    }

    if (delta > 0.0 && sim->elevators != NULL) {
        totalQueue = statistics_total_queue_length(sim);
        stats->queueLengthIntegral += (double)totalQueue * delta;

        for (i = 0; i < stats->numElevators; i++) {
            if (sim->elevators[i].status == ELEVATOR_MOVING) {
                stats->elevatorBusyTime[i] += delta;
            }
        }
    }

    stats->lastSampleTime = newTime;
    statistics_update_max_queue(stats, sim);
}

void statistics_on_passenger_request(SimulationStats* stats, const Simulation* sim)
{
    if (stats == NULL) {
        return;
    }
    stats->totalRequests++;
    statistics_update_max_queue(stats, sim);
}

void statistics_on_passenger_boarded(SimulationStats* stats, const Passenger* passenger,
                                   double boardTime)
{
    double waitTime;

    if (stats == NULL || passenger == NULL) {
        return;
    }

    waitTime = boardTime - passenger->requestTime;
    if (waitTime < 0.0) {
        waitTime = 0.0;
    }

    stats->sumWaitTime += waitTime;
    if (waitTime > STATS_WAIT_EPSILON) {
        stats->passengersWhoWaited++;
    }
    if (waitTime > stats->maxWaitTime) {
        stats->maxWaitTime = waitTime;
    }
}

void statistics_on_passenger_served(SimulationStats* stats, const Passenger* passenger,
                                    double exitTime)
{
    double totalTrip;

    if (stats == NULL || passenger == NULL) {
        return;
    }

    totalTrip = exitTime - passenger->requestTime;
    if (totalTrip < 0.0) {
        totalTrip = 0.0;
    }

    stats->totalServed++;
    stats->sumTotalTripTime += totalTrip;
}

static void statistics_print_to_file(FILE* out, const SimulationStats* stats,
                                     const Simulation* sim, double simDuration)
{
    int i;
    double avgWait = 0.0;
    double avgTrip = 0.0;
    double avgQueue = 0.0;
    double overallBusy = 0.0;
    double waitedPct = 0.0;

    if (stats->totalServed > 0) {
        avgWait = stats->sumWaitTime / (double)stats->totalServed;
        avgTrip = stats->sumTotalTripTime / (double)stats->totalServed;
    }
    if (simDuration > 0.0) {
        avgQueue = stats->queueLengthIntegral / simDuration;
    }
    if (stats->totalServed > 0) {
        waitedPct = 100.0 * (double)stats->passengersWhoWaited / (double)stats->totalServed;
    }

    fprintf(out, "=================================\n");
    fprintf(out, "       SIMULATION RESULTS       \n");
    fprintf(out, "=================================\n\n");

    fprintf(out, "--- Configuration ---\n");
    fprintf(out, "Simulation Time: %.2f seconds\n", simDuration);
    fprintf(out, "Number of Floors: %d\n", sim->numFloors);
    fprintf(out, "Number of Elevators: %d\n", sim->numElevators);
    fprintf(out, "Elevator Capacity: %d\n", sim->elevatorCapacity);
    fprintf(out, "Travel Time per Floor: %.2f seconds\n\n", SECONDS_PER_FLOOR);

    fprintf(out, "--- Overall Performance ---\n");
    fprintf(out, "Total Passenger Requests: %d\n", stats->totalRequests);
    fprintf(out, "Total Passengers Served: %d\n", stats->totalServed);
    if (stats->totalRequests > stats->totalServed) {
        fprintf(out, "Passengers Not Served (still waiting): %d\n",
                stats->totalRequests - stats->totalServed);
    }
    fprintf(out, "Passengers Who Waited in Queue: %d (%.2f%%)\n\n",
            stats->passengersWhoWaited, waitedPct);

    fprintf(out, "--- Passenger Statistics ---\n");
    fprintf(out, "Average Wait Time in Queue: %.2f seconds\n", avgWait);
    fprintf(out, "Maximum Wait Time in Queue: %.2f seconds\n", stats->maxWaitTime);
    fprintf(out, "Average Total Trip Time (Wait + Travel + Doors): %.2f seconds\n\n",
            avgTrip);

    fprintf(out, "--- Queue Statistics ---\n");
    fprintf(out, "Maximum Queue Length Reached: %d\n", stats->maxQueueLength);
    fprintf(out, "Average Queue Length: %.2f passengers\n\n", avgQueue);

    fprintf(out, "--- Elevator Utilization ---\n");
    for (i = 0; i < stats->numElevators; i++) {
        double util = 0.0;
        if (simDuration > 0.0) {
            util = 100.0 * stats->elevatorBusyTime[i] / simDuration;
            overallBusy += stats->elevatorBusyTime[i];
        }
        fprintf(out, "Elevator %d Busy Time: %.2f sec (Utilization: %.2f%%)\n",
                i, stats->elevatorBusyTime[i], util);
    }
    if (stats->numElevators > 0 && simDuration > 0.0) {
        overallBusy = 100.0 * overallBusy / ((double)stats->numElevators * simDuration);
    } else {
        overallBusy = 0.0;
    }
    fprintf(out, "Overall Elevator Utilization: %.2f%%\n", overallBusy);
    fprintf(out, "\n=================================\n");
}

void statistics_finalize_and_print(SimulationStats* stats, const Simulation* sim)
{
    FILE* reportFile;
    double simDuration;

    if (stats == NULL || sim == NULL) {
        return;
    }

    simDuration = sim->currentTime;
    statistics_advance_to_time(stats, sim, simDuration);

    reportFile = fopen(STATS_REPORT_FILE, "w");
    if (reportFile != NULL) {
        statistics_print_to_file(reportFile, stats, sim, simDuration);
        fclose(reportFile);
        log_message(sim->currentTime, LOG_INFO, "Simulation results written to file");
    } else {
        log_message(sim->currentTime, LOG_WARNING, "Could not write simulation results file");
    }

    statistics_print_to_file(stdout, stats, sim, simDuration);
}
