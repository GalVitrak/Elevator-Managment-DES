/*
 * statistics.c - End-of-simulation performance report (bank-style summary)
 *
 * PRESENTATION: Show statistics_finalize_and_print() output -> simulation_results.txt
 * (service rate, max queue wait, Queue waits over SLA, per-passenger table, util %).
 */
#include "statistics.h"
#include "simulation.h"
#include "constants.h"
#include "file_manager.h"
#include "logger.h"
#include "floor.h"
#include "text_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATS_WAIT_EPSILON 0.01

static int statistics_compare_trip_by_id(const void* a, const void* b)
{
    const PassengerTripRecord* left = (const PassengerTripRecord*)a;
    const PassengerTripRecord* right = (const PassengerTripRecord*)b;
    return left->passengerId - right->passengerId;
}

static void statistics_sort_trip_records(SimulationStats* stats)
{
    if (stats == NULL || stats->tripRecordCount <= 1 || stats->tripRecords == NULL) {
        return;
    }
    qsort(stats->tripRecords, (size_t)stats->tripRecordCount,
          sizeof(PassengerTripRecord), statistics_compare_trip_by_id);
}

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

static int statistics_ensure_trip_capacity(SimulationStats* stats)
{
    PassengerTripRecord* grown;
    int newCapacity;

    if (stats->tripRecordCount < stats->tripRecordCapacity) {
        return 1;
    }

    newCapacity = (stats->tripRecordCapacity == 0) ? 16 : stats->tripRecordCapacity * 2;
    grown = (PassengerTripRecord*)realloc(stats->tripRecords,
                                          (size_t)newCapacity * sizeof(PassengerTripRecord));
    if (grown == NULL) {
        return 0;
    }

    stats->tripRecords = grown;
    stats->tripRecordCapacity = newCapacity;
    return 1;
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
    free(stats->tripRecords);
    stats->elevatorBusyTime = NULL;
    stats->tripRecords = NULL;
    stats->tripRecordCount = 0;
    stats->tripRecordCapacity = 0;
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

    stats->passengersBoarded++;
    stats->sumWaitTime += waitTime;
    if (waitTime > STATS_WAIT_EPSILON) {
        stats->passengersWhoWaited++;
    }
    if (waitTime > MAX_QUEUE_WAIT_SECONDS) {
        stats->slaQueueViolations++;
    }
    if (waitTime > stats->maxWaitTime) {
        stats->maxWaitTime = waitTime;
    }
}

void statistics_on_passenger_served(SimulationStats* stats, const Passenger* passenger,
                                    double exitTime)
{
    double totalTrip;
    double queueTime;
    double travelTime;
    PassengerTripRecord* record;

    if (stats == NULL || passenger == NULL) {
        return;
    }

    totalTrip = exitTime - passenger->requestTime;
    if (totalTrip < 0.0) {
        totalTrip = 0.0;
    }

    queueTime = 0.0;
    if (passenger->boardTime >= 0.0) {
        queueTime = passenger->boardTime - passenger->requestTime;
        if (queueTime < 0.0) {
            queueTime = 0.0;
        }
    }

    travelTime = 0.0;
    if (passenger->boardTime >= 0.0) {
        travelTime = exitTime - passenger->boardTime;
        if (travelTime < 0.0) {
            travelTime = 0.0;
        }
    }

    if (statistics_ensure_trip_capacity(stats)) {
        record = &stats->tripRecords[stats->tripRecordCount++];
        record->passengerId = passenger->id;
        record->sourceFloor = passenger->sourceFloor;
        record->destinationFloor = passenger->destinationFloor;
        record->queueSeconds = queueTime;
        record->travelSeconds = travelTime;
        record->totalSeconds = totalTrip;
    }

    stats->totalServed++;
    stats->sumTotalTripTime += totalTrip;
}

static void statistics_print_per_passenger_table(FILE* out, const Simulation* sim,
                                               const SimulationStats* stats)
{
    static const int widths[6] = { 6, 6, 6, 11, 11, 11 };
    static const char* headers[6] = {
        "ID", "From", "To", "Queue (s)", "Travel (s)", "Total (s)"
    };
    TextTable table;
    int i;
    double sumQueue = 0.0;
    double sumTravel = 0.0;
    double sumTotal = 0.0;
    double maxTotal = 0.0;
    int slowestId = -1;
    double maxQueue = 0.0;
    int longestQueueId = -1;
    char idBuf[16];
    char fromBuf[16];
    char toBuf[16];
    char queueBuf[16];
    char travelBuf[16];
    char totalBuf[16];
    const char* cells[6];

    text_table_print_section(out,
                           "PER-PASSENGER TRIP DETAILS (display floors, sorted by ID)");

    if (stats->tripRecordCount == 0) {
        fprintf(out, "  (no completed trips)\n\n");
        return;
    }

    text_table_begin(&table, out, widths, 6);
    text_table_header(&table, headers);

    for (i = 0; i < stats->tripRecordCount; i++) {
        const PassengerTripRecord* r = &stats->tripRecords[i];

        snprintf(idBuf, sizeof(idBuf), "%d", r->passengerId);
        if (sim != NULL) {
            snprintf(fromBuf, sizeof(fromBuf), "%d",
                     config_index_to_display(&sim->config, r->sourceFloor));
            snprintf(toBuf, sizeof(toBuf), "%d",
                     config_index_to_display(&sim->config, r->destinationFloor));
        } else {
            snprintf(fromBuf, sizeof(fromBuf), "%d", r->sourceFloor);
            snprintf(toBuf, sizeof(toBuf), "%d", r->destinationFloor);
        }
        snprintf(queueBuf, sizeof(queueBuf), "%.2f", r->queueSeconds);
        snprintf(travelBuf, sizeof(travelBuf), "%.2f", r->travelSeconds);
        snprintf(totalBuf, sizeof(totalBuf), "%.2f", r->totalSeconds);

        cells[0] = idBuf;
        cells[1] = fromBuf;
        cells[2] = toBuf;
        cells[3] = queueBuf;
        cells[4] = travelBuf;
        cells[5] = totalBuf;
        text_table_row(&table, cells);

        sumQueue += r->queueSeconds;
        sumTravel += r->travelSeconds;
        sumTotal += r->totalSeconds;

        if (r->totalSeconds >= maxTotal) {
            maxTotal = r->totalSeconds;
            slowestId = r->passengerId;
        }
        if (r->queueSeconds >= maxQueue) {
            maxQueue = r->queueSeconds;
            longestQueueId = r->passengerId;
        }
    }

    text_table_separator(&table);
    snprintf(idBuf, sizeof(idBuf), "AVG");
    fromBuf[0] = '\0';
    toBuf[0] = '\0';
    snprintf(queueBuf, sizeof(queueBuf), "%.2f",
             sumQueue / (double)stats->tripRecordCount);
    snprintf(travelBuf, sizeof(travelBuf), "%.2f",
             sumTravel / (double)stats->tripRecordCount);
    snprintf(totalBuf, sizeof(totalBuf), "%.2f",
             sumTotal / (double)stats->tripRecordCount);
    cells[0] = idBuf;
    cells[1] = fromBuf;
    cells[2] = toBuf;
    cells[3] = queueBuf;
    cells[4] = travelBuf;
    cells[5] = totalBuf;
    text_table_row(&table, cells);
    text_table_end(&table);

    fprintf(out, "  Longest queue wait: passenger %d (%.2f s)\n",
            longestQueueId, maxQueue);
    fprintf(out, "  Longest total trip:  passenger %d (%.2f s)\n\n",
            slowestId, maxTotal);
}

static void statistics_print_to_file(FILE* out, SimulationStats* stats,
                                     const Simulation* sim, double simDuration)
{
    int i;
    double avgWait = 0.0;
    double avgTrip = 0.0;
    double avgQueue = 0.0;
    double overallBusy = 0.0;
    double waitedPct = 0.0;
    double serviceRate = 0.0;
    char valueBuf[8][64];
    const char* configKeys[7];
    const char* configValues[7];
    const char* perfKeys[5];
    const char* perfValues[5];
    const char* timeKeys[4];
    const char* timeValues[4];
    const char* queueKeys[2];
    const char* queueValues[2];
    static const int elevWidths[3] = { 10, 14, 14 };
    static const char* elevHeaders[3] = { "Elevator", "Busy (s)", "Util (%)" };
    TextTable elevTable;
    char elevIdBuf[16];
    char busyBuf[16];
    char utilBuf[16];
    const char* elevCells[3];
    int perfRows = 4;

    if (stats->totalServed > 0) {
        avgWait = stats->sumWaitTime / (double)stats->totalServed;
        avgTrip = stats->sumTotalTripTime / (double)stats->totalServed;
    }
    if (simDuration > 0.0) {
        avgQueue = stats->queueLengthIntegral / simDuration;
    }
    if (stats->passengersBoarded > 0) {
        waitedPct = 100.0 * (double)stats->passengersWhoWaited /
                    (double)stats->passengersBoarded;
        if (waitedPct > 100.0) {
            waitedPct = 100.0;
        }
    }
    if (stats->totalRequests > 0) {
        serviceRate = 100.0 * (double)stats->totalServed / (double)stats->totalRequests;
    }

    statistics_sort_trip_records(stats);

    text_table_print_project_ascii_art(out);
    text_table_print_banner(out, "ELEVATOR SIMULATION RESULTS");

    configKeys[0] = "Simulation end time";
    snprintf(valueBuf[0], sizeof(valueBuf[0]), "%.2f s", simDuration);
    configValues[0] = valueBuf[0];

    configKeys[1] = "Floors / elevators / capacity";
    snprintf(valueBuf[1], sizeof(valueBuf[1]), "%d / %d / %d",
             sim->numFloors, sim->numElevators, sim->elevatorCapacity);
    configValues[1] = valueBuf[1];

    configKeys[2] = "Travel per floor";
    snprintf(valueBuf[2], sizeof(valueBuf[2]), "%.2f s", SECONDS_PER_FLOOR);
    configValues[2] = valueBuf[2];

    configKeys[3] = "Door open / dwell / close";
    snprintf(valueBuf[3], sizeof(valueBuf[3]), "%.2f / %.2f / %.2f s",
             DOOR_OPEN_TIME_SECONDS, DOOR_DWELL_SECONDS, DOOR_CLOSE_TIME_SECONDS);
    configValues[3] = valueBuf[3];

    configKeys[4] = "Door cycle per stop";
    snprintf(valueBuf[4], sizeof(valueBuf[4]), "%.2f s",
             DOOR_OPEN_TIME_SECONDS + DOOR_DWELL_SECONDS + DOOR_CLOSE_TIME_SECONDS);
    configValues[4] = valueBuf[4];

    text_table_print_section(out, "CONFIGURATION");
    text_table_print_key_values(out, configKeys, configValues, 5);

    perfKeys[0] = "Passenger requests";
    snprintf(valueBuf[0], sizeof(valueBuf[0]), "%d", stats->totalRequests);
    perfValues[0] = valueBuf[0];

    perfKeys[1] = "Passengers served";
    snprintf(valueBuf[1], sizeof(valueBuf[1]), "%d", stats->totalServed);
    perfValues[1] = valueBuf[1];

    perfKeys[2] = "Service rate";
    snprintf(valueBuf[2], sizeof(valueBuf[2]), "%.1f%%", serviceRate);
    perfValues[2] = valueBuf[2];

    perfKeys[3] = "Waited in queue (boarded)";
    snprintf(valueBuf[3], sizeof(valueBuf[3]), "%d / %d (%.1f%%)",
             stats->passengersWhoWaited, stats->passengersBoarded, waitedPct);
    perfValues[3] = valueBuf[3];

    if (stats->totalRequests > stats->totalServed) {
        perfKeys[4] = "Not served (still waiting)";
        snprintf(valueBuf[4], sizeof(valueBuf[4]), "%d",
                 stats->totalRequests - stats->totalServed);
        perfValues[4] = valueBuf[4];
        perfRows = 5;
    }

    text_table_print_section(out, "OVERALL PERFORMANCE");
    text_table_print_key_values(out, perfKeys, perfValues, perfRows);

    timeKeys[0] = "Average queue wait";
    snprintf(valueBuf[0], sizeof(valueBuf[0]), "%.2f s", avgWait);
    timeValues[0] = valueBuf[0];

    timeKeys[1] = "Maximum queue wait";
    snprintf(valueBuf[1], sizeof(valueBuf[1]), "%.2f s", stats->maxWaitTime);
    timeValues[1] = valueBuf[1];

    timeKeys[2] = "Queue waits over SLA";
    snprintf(valueBuf[2], sizeof(valueBuf[2]), "%d (limit %.0f s)",
             stats->slaQueueViolations, MAX_QUEUE_WAIT_SECONDS);
    timeValues[2] = valueBuf[2];

    timeKeys[3] = "Average total trip";
    snprintf(valueBuf[3], sizeof(valueBuf[3]), "%.2f s", avgTrip);
    timeValues[3] = valueBuf[3];

    text_table_print_section(out, "PASSENGER TIME SUMMARY");
    text_table_print_key_values(out, timeKeys, timeValues, 4);

    statistics_print_per_passenger_table(out, sim, stats);

    queueKeys[0] = "Peak queue length";
    snprintf(valueBuf[0], sizeof(valueBuf[0]), "%d", stats->maxQueueLength);
    queueValues[0] = valueBuf[0];

    queueKeys[1] = "Time-weighted average queue";
    snprintf(valueBuf[1], sizeof(valueBuf[1]), "%.2f passengers", avgQueue);
    queueValues[1] = valueBuf[1];

    text_table_print_section(out, "QUEUE STATISTICS");
    text_table_print_key_values(out, queueKeys, queueValues, 2);

    text_table_print_section(out, "ELEVATOR UTILIZATION");
    text_table_begin(&elevTable, out, elevWidths, 3);
    text_table_header(&elevTable, elevHeaders);

    for (i = 0; i < stats->numElevators; i++) {
        double util = 0.0;
        if (simDuration > 0.0) {
            util = 100.0 * stats->elevatorBusyTime[i] / simDuration;
            overallBusy += stats->elevatorBusyTime[i];
        }
        snprintf(elevIdBuf, sizeof(elevIdBuf), "%d", i);
        snprintf(busyBuf, sizeof(busyBuf), "%.2f", stats->elevatorBusyTime[i]);
        snprintf(utilBuf, sizeof(utilBuf), "%.1f", util);
        elevCells[0] = elevIdBuf;
        elevCells[1] = busyBuf;
        elevCells[2] = utilBuf;
        text_table_row(&elevTable, elevCells);
    }

    if (stats->numElevators > 0 && simDuration > 0.0) {
        overallBusy = 100.0 * overallBusy / ((double)stats->numElevators * simDuration);
    } else {
        overallBusy = 0.0;
    }

    text_table_separator(&elevTable);
    snprintf(elevIdBuf, sizeof(elevIdBuf), "FLEET");
    snprintf(busyBuf, sizeof(busyBuf), "-");
    snprintf(utilBuf, sizeof(utilBuf), "%.1f", overallBusy);
    elevCells[0] = elevIdBuf;
    elevCells[1] = busyBuf;
    elevCells[2] = utilBuf;
    text_table_row(&elevTable, elevCells);
    text_table_end(&elevTable);
    fputc('\n', out);
}

/*
 * statistics_finalize_and_print - End of every simulation_run(): show summary and save to files.
 *   - stdout (console)
 *   - STATS_REPORT_FILE (simulation_results.txt) — full report, overwrite each run
 *   - LOG_FILE_NAME (simulation_log.txt) — append same report at end of run
 */
int statistics_finalize_and_print(SimulationStats* stats, const Simulation* sim)
{
    FILE* reportFile;
    FILE* logAppend;
    double simDuration;
    int fileOk = 0;

    if (stats == NULL || sim == NULL) {
        return 0;
    }

    simDuration = sim->currentTime;
    statistics_advance_to_time(stats, sim, simDuration);

    /* Dedicated results file (for submission / reports) */
    reportFile = fopen(STATS_REPORT_FILE, "w");
    if (reportFile != NULL) {
        statistics_print_to_file(reportFile, stats, sim, simDuration);
        fclose(reportFile);
        fileOk = 1;
    } else {
        log_message(sim->currentTime, LOG_ERROR, "Could not write simulation results file");
    }

    /* Append summary tables after the event log (close event table border first). */
    logger_close_table_for_append();

    logAppend = fopen(LOG_FILE_NAME, "a");
    if (logAppend != NULL) {
        fprintf(logAppend, "\n");
        statistics_print_to_file(logAppend, stats, sim, simDuration);
        fclose(logAppend);
    } else {
        log_message(sim->currentTime, LOG_WARNING, "Could not append summary to simulation log");
    }

    statistics_print_to_file(stdout, stats, sim, simDuration);

    if (fileOk) {
        printf("\n>>> Simulation summary saved to: %s", STATS_REPORT_FILE);
        printf(" (also appended to %s)\n\n", LOG_FILE_NAME);
    }

    return fileOk;
}
