/*
 * simulation.c - Discrete Event Simulation (DES) engine
 *
 * PRESENTATION: This is the most important file. Show graders:
 *   1. simulation_run()           - DES loop (pop FEL -> jump clock -> dispatch)
 *   2. simulation_dispatch_event() - routes PASSENGER_CALL, ARRIVAL, DOORS_*
 *   3. simulation_find_elevator_for_pickup() + batch dispatch - assignment policy
 *   4. simulation_pick_next_stop_floor() - pickups before other stops (SLA)
 *   5. handle_doors_open()          - board / alight at a floor
 *
 * Does NOT use fixed time steps: sim->currentTime jumps to each event's time.
 */
#include "simulation.h"
#include "constants.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void simulation_refresh_elevator_stops(Simulation* sim, int elevatorIndex);
static void simulation_start_elevator_if_needed(Simulation* sim, Elevator* elevator);
static int simulation_count_assigned_waiters(const Simulation* sim, int elevatorIndex);
static int simulation_elevator_free_slots(const Simulation* sim, int elevatorIndex);
static int simulation_find_elevator_for_pickup(const Simulation* sim, int callFloor,
                                               int destFloor, double waiterWaitSeconds);
static void simulation_record_floor_demand(Simulation* sim, int floorIndex);
static void simulation_idle_reposition_if_needed(Simulation* sim, Elevator* elevator);
static void simulation_assign_passenger_to_elevator(Simulation* sim, Passenger* passenger,
                                                    int elevatorIndex);
static void simulation_release_hall_buttons_if_clear(Simulation* sim, int floor);
static int simulation_elevator_needs_service_at_floor(const Simulation* sim,
                                                      const Elevator* elevator,
                                                      int floor);
static void simulation_set_direction_toward_floor(Elevator* elevator, int targetFloor);
static int simulation_pick_next_stop_floor(const Simulation* sim,
                                           const Elevator* elevator);
static void simulation_service_waiting_queues(Simulation* sim);
static void simulation_enforce_queue_wait_policy(Simulation* sim);

/*
 * simulation_format_onboard_list - Comma-separated passenger IDs in cab, or "none".
 */
static void simulation_format_onboard_list(const Simulation* sim, int elevatorId,
                                           char* buf, size_t bufSize)
{
    const Elevator* elevator;
    const Passenger* passenger;
    size_t len;

    if (buf == NULL || bufSize == 0) {
        return;
    }
    buf[0] = '\0';

    if (sim == NULL || elevatorId < 0 || elevatorId >= sim->numElevators) {
        snprintf(buf, bufSize, "none");
        return;
    }

    elevator = &sim->elevators[elevatorId];
    len = 0;
    for (passenger = elevator->onboardHead; passenger != NULL;
         passenger = passenger->onboardNext) {
        if (len > 0) {
            len += (size_t)snprintf(buf + len, bufSize - len, ",");
            if (len >= bufSize) {
                return;
            }
        }
        len += (size_t)snprintf(buf + len, bufSize - len, "%d", passenger->id);
        if (len >= bufSize) {
            return;
        }
    }

    if (len == 0) {
        snprintf(buf, bufSize, "none");
    }
}

static void simulation_format_door_event_desc(const Simulation* sim, EventType type,
                                              int elevatorId, int floor,
                                              char* desc, size_t descSize)
{
    char onboard[MAX_NAME_LEN * 4];

    simulation_format_onboard_list(sim, elevatorId, onboard, sizeof(onboard));
    snprintf(desc, descSize, "%s (elevator %d, floor %d, onboard: %s)",
             event_type_to_string(type), elevatorId, floor, onboard);
}

/*
 * simulation_schedule_event - Create an event and insert it into the FEL.
 * Logs creation at current sim time (not necessarily at event->time).
 * Internal: only scheduling entry point for new events.
 */
static void simulation_schedule_event(Simulation* sim, double time, EventType type,
                                      int elevatorId, int passengerId, int floor,
                                      int destinationFloor)
{
    Event* event;
    char desc[MAX_NAME_LEN * 4];

    event = event_create(time, type, elevatorId, passengerId, floor, destinationFloor);
    if (event == NULL) {
        log_message(sim->currentTime, LOG_ERROR, "Failed to allocate event");
        return;
    }

    event_list_insert_sorted(&sim->eventList, event);

    if (type == EVENT_DOORS_OPEN || type == EVENT_DOORS_CLOSE) {
        simulation_format_door_event_desc(sim, type, elevatorId, floor,
                                          desc, sizeof(desc));
    } else {
        snprintf(desc, sizeof(desc), "%s (elev=%d pass=%d floor=%d)",
                 event_type_to_string(type), elevatorId, passengerId, floor);
    }
    log_event_created(sim->currentTime, desc);
}

/*
 * simulation_schedule_elevator_travel - Start movement and schedule arrival after travel time.
 * Uses elevator_travel_time_seconds (SECONDS_PER_FLOOR per floor difference).
 */
static void simulation_schedule_elevator_travel(Simulation* sim, Elevator* elevator,
                                                int targetFloor)
{
    double travelTime;
    double arrivalTime;
    char msg[MAX_NAME_LEN * 4];
    int fromFloor = elevator->currentFloor;

    travelTime = elevator_travel_time_seconds(fromFloor, targetFloor);
    arrivalTime = sim->currentTime + DOOR_CLOSE_TIME_SECONDS + travelTime;

    elevator_assign_to_floor(elevator, targetFloor);

    snprintf(msg, sizeof(msg),
             "Elevator %d traveling %d -> %d (%.1f s, arrives t=%.2f)",
             elevator->id, fromFloor, targetFloor, travelTime, arrivalTime);
    log_message(sim->currentTime, LOG_INFO, msg);

    simulation_schedule_event(sim, arrivalTime, EVENT_ELEVATOR_ARRIVAL,
                              elevator->id, -1, targetFloor, -1);
}

/*
 * === PRESENTATION: Event router (after clock jump) ===
 * simulation_dispatch_event - Route one event to the correct handler by type.
 * Called after currentTime has been advanced to event->time.
 */
static void simulation_dispatch_event(Simulation* sim, Event* event)
{
    char desc[MAX_NAME_LEN * 4];

    if (event->type == EVENT_DOORS_OPEN || event->type == EVENT_DOORS_CLOSE) {
        simulation_format_door_event_desc(sim, event->type, event->elevatorId,
                                          event->floor, desc, sizeof(desc));
    } else {
        snprintf(desc, sizeof(desc), "%s", event_type_to_string(event->type));
    }
    log_event_handled(sim->currentTime, desc);

    switch (event->type) {
    case EVENT_PASSENGER_CALL:
        handle_passenger_call(sim, event);
        break;
    case EVENT_ELEVATOR_ARRIVAL:
        handle_elevator_arrival(sim, event);
        break;
    case EVENT_DOORS_OPEN:
        handle_doors_open(sim, event);
        break;
    case EVENT_DOORS_CLOSE:
        handle_doors_close(sim, event);
        break;
    case EVENT_PASSENGER_EXIT:
        handle_passenger_exit(sim, event);
        break;
    default:
        log_message(sim->currentTime, LOG_WARNING, "Unknown event type");
        break;
    }

    simulation_enforce_queue_wait_policy(sim);
}

/* simulation_validate_floor - Return 1 if floor index is valid for this building. */
int simulation_validate_floor(const Simulation* sim, int floor)
{
    return floor >= 0 && floor < sim->numFloors;
}

static Passenger* simulation_find_passenger_in_queue(Floor* floor, int passengerId);

/* Defer dispatch until all PASSENGER_CALL events at this time/floor have run (batch clustering). */
static int simulation_more_passenger_calls_same_time_floor(const Simulation* sim,
                                                           double time, int floor)
{
    const Event* event = sim->eventList.head;

    while (event != NULL) {
        if (event->type == EVENT_PASSENGER_CALL &&
            event->time == time &&
            event->floor == floor) {
            return 1;
        }
        event = event->next;
    }
    return 0;
}

/*
 * simulation_init - Allocate elevators, floors, passenger tracking, and init FEL.
 * Zeros sim, copies config, initializes each elevator and floor.
 * Returns 0 on bad config or allocation failure (calls destroy on partial alloc).
 */
int simulation_init(Simulation* sim, const SimulationConfig* config)
{
    int i;

    if (sim == NULL || config == NULL || !config_validate(config)) {
        return 0;
    }

    memset(sim, 0, sizeof(*sim));
    sim->config = *config;
    sim->currentTime = 0.0;
    sim->maxSimulationTime = config->maxSimulationTime;
    sim->numFloors = config_total_internal_floors(config);
    sim->groundFloorIndex = config->numUndergroundFloors;
    sim->numElevators = config->numElevators;
    sim->elevatorCapacity = config->capacity;
    sim->nextPassengerId = 1;

    sim->elevators = (Elevator*)calloc((size_t)sim->numElevators, sizeof(Elevator));
    sim->floors = (Floor*)calloc((size_t)sim->numFloors, sizeof(Floor));

    if (sim->elevators == NULL || sim->floors == NULL) {
        simulation_destroy(sim);
        return 0;
    }

    for (i = 0; i < sim->numElevators; i++) {
        elevator_init(&sim->elevators[i], i, sim->elevatorCapacity, sim->numFloors);
        if (sim->elevators[i].floorStops == NULL) {
            simulation_destroy(sim);
            return 0;
        }
        sim->elevators[i].currentFloor = sim->groundFloorIndex;
    }

    for (i = 0; i < sim->numFloors; i++) {
        floor_init(&sim->floors[i], i);
    }

    if (!building_grid_init(&sim->buildingView, sim->numFloors, sim->numElevators,
                            config->numUndergroundFloors)) {
        simulation_destroy(sim);
        return 0;
    }

    event_list_init(&sim->eventList);
    sim->nextDispatchFloor = 0;
    statistics_reset(&sim->stats, sim->numElevators);
    sim->stats.lastSampleTime = 0.0;

    sim->floorDemand = (int*)calloc((size_t)sim->numFloors, sizeof(int));
    if (sim->floorDemand == NULL) {
        simulation_destroy(sim);
        return 0;
    }

    if (sim->numFloors >= DISPATCH_MIN_FLOORS_FOR_ZONES && sim->numElevators > 1) {
        sim->numZones = sim->numFloors / 20;
        if (sim->numZones < 4) {
            sim->numZones = 4;
        }
        if (sim->numZones > 20) {
            sim->numZones = 20;
        }
    } else {
        sim->numZones = 1;
    }

    return 1;
}

/*
 * simulation_destroy - Free FEL, floor queues, in-cab passengers, and arrays.
 * Safe on NULL sim or partially initialized sim.
 */
void simulation_destroy(Simulation* sim)
{
    int i;

    if (sim == NULL) {
        return;
    }

    event_list_destroy(&sim->eventList);

    building_grid_destroy(&sim->buildingView);

    if (sim->elevators != NULL) {
        for (i = 0; i < sim->numElevators; i++) {
            Passenger* onboard = sim->elevators[i].onboardHead;
            while (onboard != NULL) {
                Passenger* next = onboard->onboardNext;
                passenger_destroy(onboard);
                onboard = next;
            }
            sim->elevators[i].onboardHead = NULL;
            sim->elevators[i].passengerCount = 0;
            elevator_stops_destroy(&sim->elevators[i]);
        }
    }

    if (sim->floors != NULL) {
        for (i = 0; i < sim->numFloors; i++) {
            floor_destroy(&sim->floors[i]);
        }
        free(sim->floors);
        sim->floors = NULL;
    }

    free(sim->elevators);
    sim->elevators = NULL;

    free(sim->floorDemand);
    sim->floorDemand = NULL;
    sim->numZones = 0;

    statistics_destroy(&sim->stats);
}

/* simulation_reset - Destroy and re-initialize with the same config snapshot. */
void simulation_reset(Simulation* sim)
{
    SimulationConfig config;

    if (sim == NULL) {
        return;
    }

    config = sim->config;
    simulation_destroy(sim);
    simulation_init(sim, &config);
}

/*
 * simulation_add_passenger_request - Public API to add one ride request.
 * Creates passenger, enqueues on source floor, schedules PASSENGER_CALL at now.
 * Increments nextPassengerId after scheduling.
 */
void simulation_add_passenger_request(Simulation* sim, int sourceFloor,
                                      int destinationFloor)
{
    Passenger* passenger;
    char msg[MAX_NAME_LEN * 4];

    if (!simulation_validate_floor(sim, sourceFloor) ||
        !simulation_validate_floor(sim, destinationFloor)) {
        log_message(sim->currentTime, LOG_ERROR, "Invalid floor in passenger request");
        return;
    }

    if (sourceFloor == destinationFloor) {
        log_message(sim->currentTime, LOG_WARNING,
                    "Passenger source and destination are the same floor");
        return;
    }

    passenger = passenger_create(sim->nextPassengerId, sourceFloor,
                                 destinationFloor, sim->currentTime);
    if (passenger == NULL) {
        log_message(sim->currentTime, LOG_ERROR, "Failed to create passenger");
        return;
    }

    floor_enqueue_passenger(&sim->floors[sourceFloor], passenger);

    snprintf(msg, sizeof(msg),
             "Passenger %d request queued: floor %d -> %d",
             passenger->id, sourceFloor, destinationFloor);
    log_message(sim->currentTime, LOG_INFO, msg);

    simulation_schedule_event(sim, sim->currentTime, EVENT_PASSENGER_CALL,
                              -1, passenger->id, sourceFloor, destinationFloor);
    sim->nextPassengerId++;
}

/*
 * simulation_schedule_passenger_arrival - Schedule future PASSENGER_CALL (staggered arrivals).
 * Passenger appears in queue only when the event runs at arrivalTime.
 */
void simulation_schedule_passenger_arrival(Simulation* sim, double arrivalTime,
                                           int sourceFloor, int destinationFloor)
{
    char msg[MAX_NAME_LEN * 4];
    int passengerId;

    if (!simulation_validate_floor(sim, sourceFloor) ||
        !simulation_validate_floor(sim, destinationFloor)) {
        log_message(sim->currentTime, LOG_ERROR, "Invalid floor in scheduled arrival");
        return;
    }

    if (sourceFloor == destinationFloor) {
        return;
    }

    if (arrivalTime < 0.0) {
        arrivalTime = 0.0;
    }
    if (arrivalTime > sim->maxSimulationTime) {
        log_message(sim->currentTime, LOG_WARNING, "Skipping arrival past simulation end");
        return;
    }

    passengerId = sim->nextPassengerId;
    simulation_schedule_event(sim, arrivalTime, EVENT_PASSENGER_CALL,
                              -1, passengerId, sourceFloor, destinationFloor);
    sim->nextPassengerId++;

    snprintf(msg, sizeof(msg),
             "Scheduled passenger %d arrival at t=%.2f: floor %d -> %d",
             passengerId, arrivalTime, sourceFloor, destinationFloor);
    log_message(sim->currentTime, LOG_INFO, msg);
}

/* simulation_print_state - Debug dump: time, elevators, floors, FEL; grid optional. */
void simulation_print_state(const Simulation* sim, int showGrid)
{
    int i;

    if (sim == NULL) {
        return;
    }

    printf("\n=== Simulation State (t=%.2f / max=%.2f) ===\n",
           sim->currentTime, sim->maxSimulationTime);
    printf("Floors: %d | Elevators: %d | Capacity: %d\n",
           sim->numFloors, sim->numElevators, sim->elevatorCapacity);

    for (i = 0; i < sim->numElevators; i++) {
        elevator_print(&sim->elevators[i]);
    }

    for (i = 0; i < sim->numFloors; i++) {
        floor_print_queue(&sim->floors[i]);
    }

    if (showGrid) {
        building_grid_sync((BuildingGrid*)&sim->buildingView, sim);
        building_grid_print(&sim->buildingView);
    }

    event_list_print(&sim->eventList);
    printf("==========================================\n\n");
}

static void simulation_refresh_elevator_stops(Simulation* sim, int elevatorIndex)
{
    Elevator* elevator;
    int floorIndex;
    const Passenger* walker;

    if (sim == NULL || elevatorIndex < 0 || elevatorIndex >= sim->numElevators) {
        return;
    }

    elevator = &sim->elevators[elevatorIndex];
    if (elevator->floorStops == NULL) {
        return;
    }

    memset(elevator->floorStops, 0, (size_t)sim->numFloors);

    for (walker = elevator->onboardHead; walker != NULL; walker = walker->onboardNext) {
        elevator_add_stop(elevator, walker->destinationFloor);
    }

    for (floorIndex = 0; floorIndex < sim->numFloors; floorIndex++) {
        for (walker = sim->floors[floorIndex].waitingQueueFront; walker != NULL;
             walker = walker->next) {
            if (walker->assignedElevatorId == elevatorIndex) {
                /* Pickup only; destination added after boarding. */
                elevator_add_stop(elevator, floorIndex);
            }
        }
    }
}

static int simulation_elevator_needs_service_at_floor(const Simulation* sim,
                                                      const Elevator* elevator,
                                                      int floor)
{
    const Passenger* walker;

    if (sim == NULL || elevator == NULL || !simulation_validate_floor(sim, floor)) {
        return 0;
    }

    for (walker = elevator->onboardHead; walker != NULL; walker = walker->onboardNext) {
        if (walker->destinationFloor == floor) {
            return 1;
        }
    }

    for (walker = sim->floors[floor].waitingQueueFront; walker != NULL;
         walker = walker->next) {
        if (walker->assignedElevatorId == elevator->id) {
            return 1;
        }
    }

    return 0;
}

static void simulation_set_direction_toward_floor(Elevator* elevator, int targetFloor)
{
    if (elevator == NULL) {
        return;
    }
    if (targetFloor > elevator->currentFloor) {
        elevator->direction = DIR_UP;
    } else if (targetFloor < elevator->currentFloor) {
        elevator->direction = DIR_DOWN;
    }
}

static int simulation_count_assigned_waiters(const Simulation* sim, int elevatorIndex)
{
    int floorIndex;
    int count = 0;
    const Passenger* walker;

    for (floorIndex = 0; floorIndex < sim->numFloors; floorIndex++) {
        for (walker = sim->floors[floorIndex].waitingQueueFront; walker != NULL;
             walker = walker->next) {
            if (walker->assignedElevatorId == elevatorIndex) {
                count++;
            }
        }
    }
    return count;
}

static int simulation_elevator_free_slots(const Simulation* sim, int elevatorIndex)
{
    const Elevator* elevator;
    int reserved;

    if (sim == NULL || elevatorIndex < 0 || elevatorIndex >= sim->numElevators) {
        return 0;
    }

    elevator = &sim->elevators[elevatorIndex];
    reserved = simulation_count_assigned_waiters(sim, elevatorIndex);
    return elevator->capacity - elevator->passengerCount - reserved;
}

static int simulation_elevator_load(const Simulation* sim, int elevatorIndex)
{
    const Elevator* elev;

    if (sim == NULL || elevatorIndex < 0 || elevatorIndex >= sim->numElevators) {
        return 0;
    }
    elev = &sim->elevators[elevatorIndex];
    return elev->passengerCount + simulation_count_assigned_waiters(sim, elevatorIndex);
}

static int simulation_count_elevator_stops(const Elevator* elevator)
{
    int floor;
    int count = 0;

    if (elevator == NULL || elevator->floorStops == NULL) {
        return 0;
    }
    for (floor = 0; floor < elevator->numFloors; floor++) {
        count += (int)elevator->floorStops[floor];
    }
    return count;
}

static int simulation_zone_for_floor(const Simulation* sim, int floor)
{
    if (sim == NULL || sim->numZones <= 1 || sim->numFloors <= 0) {
        return 0;
    }
    if (floor < 0) {
        floor = 0;
    }
    if (floor >= sim->numFloors) {
        floor = sim->numFloors - 1;
    }
    return (floor * sim->numZones) / sim->numFloors;
}

static int simulation_zone_for_elevator(const Simulation* sim, int elevatorIndex)
{
    if (sim == NULL || sim->numZones <= 1 || sim->numElevators <= 0) {
        return 0;
    }
    return (elevatorIndex * sim->numZones) / sim->numElevators;
}

static double simulation_zone_penalty_seconds(const Simulation* sim, int elevatorIndex,
                                              int callFloor)
{
    if (sim == NULL || sim->numZones <= 1) {
        return 0.0;
    }
    if (simulation_zone_for_floor(sim, callFloor) ==
        simulation_zone_for_elevator(sim, elevatorIndex)) {
        return 0.0;
    }
    return DISPATCH_ZONE_PENALTY_SEC;
}

static double simulation_estimate_pickup_eta(const Simulation* sim, int elevatorIndex,
                                             int callFloor, int destFloor)
{
    const Elevator* elev;
    double travel;
    double stopDelay;
    int stopCount;

    if (sim == NULL || elevatorIndex < 0 || elevatorIndex >= sim->numElevators) {
        return 1e9;
    }

    elev = &sim->elevators[elevatorIndex];
    if (elev->status == ELEVATOR_MOVING) {
        if (!elevator_will_serve_call(elev, callFloor, destFloor)) {
            return 1e9;
        }
    } else if (elev->status != ELEVATOR_IDLE || elev->doorState != DOOR_CLOSED) {
        return 1e9;
    }

    travel = elevator_travel_time_seconds(elev->currentFloor, callFloor);
    stopCount = simulation_count_elevator_stops(elev);
    stopDelay = (double)stopCount *
                (DOOR_OPEN_TIME_SECONDS + DOOR_DWELL_SECONDS + DOOR_CLOSE_TIME_SECONDS);
    return travel + stopDelay +
           simulation_zone_penalty_seconds(sim, elevatorIndex, callFloor);
}

static double simulation_dispatch_score(double etaSeconds, int load,
                                        double waiterWaitSeconds)
{
    return etaSeconds + (double)load * DISPATCH_ETA_LOAD_WEIGHT -
           waiterWaitSeconds * DISPATCH_WAIT_BONUS;
}

static int simulation_cluster_max_span_for_wait(double waitSeconds)
{
    if (waitSeconds >= MAX_QUEUE_WAIT_SECONDS * 0.5) {
        return DEST_CLUSTER_SPAN_TIGHT;
    }
    if (waitSeconds >= MAX_QUEUE_WAIT_SECONDS * 0.25) {
        return DEST_CLUSTER_MAX_SPAN_FLOORS;
    }
    return DEST_CLUSTER_SPAN_LOOSE;
}

static void simulation_record_floor_demand(Simulation* sim, int floorIndex)
{
    if (sim == NULL || sim->floorDemand == NULL ||
        !simulation_validate_floor(sim, floorIndex)) {
        return;
    }
    sim->floorDemand[floorIndex]++;
}

/*
 * === PRESENTATION: Dispatch — pick cab for one call ===
 * Scores idle cabs by ETA + load - wait bonus. Moving cabs only if fleet < 30
 * and elevator_will_serve_call() (same direction, not past pickup floor).
 */
static int simulation_find_elevator_for_pickup(const Simulation* sim, int callFloor,
                                               int destFloor, double waiterWaitSeconds)
{
    int i;
    int bestIndex;
    double bestScore;
    int movingIndex;

    bestIndex = -1;
    bestScore = 1e12;

    for (i = 0; i < sim->numElevators; i++) {
        double eta;
        double score;
        int load;

        if (simulation_elevator_free_slots(sim, i) <= 0) {
            continue;
        }

        if (sim->elevators[i].status == ELEVATOR_IDLE &&
            sim->elevators[i].doorState == DOOR_CLOSED) {
            eta = simulation_estimate_pickup_eta(sim, i, callFloor, destFloor);
            load = simulation_elevator_load(sim, i);
            score = simulation_dispatch_score(eta, load, waiterWaitSeconds);
            if (bestIndex < 0 || score < bestScore) {
                bestIndex = i;
                bestScore = score;
            }
        }
    }

    if (sim->numElevators < 30) {
        movingIndex = elevator_find_moving_for_call(sim->elevators, sim->numElevators,
                                                    callFloor, destFloor, NULL);
        if (movingIndex >= 0 && simulation_elevator_free_slots(sim, movingIndex) > 0) {
            double eta = simulation_estimate_pickup_eta(sim, movingIndex, callFloor, destFloor);
            int load = simulation_elevator_load(sim, movingIndex);

            if (eta + waiterWaitSeconds <= MOVING_PICKUP_MAX_SUM_WAIT) {
                double score = simulation_dispatch_score(eta, load, waiterWaitSeconds);
                if (bestIndex < 0 || score < bestScore) {
                    bestIndex = movingIndex;
                    bestScore = score;
                }
            }
        }
    }

    return bestIndex;
}

static int simulation_find_highest_demand_floor(const Simulation* sim)
{
    int floor;
    int bestFloor;
    int bestDemand;
    int bestQueue;

    if (sim == NULL) {
        return -1;
    }

    bestFloor = -1;
    bestDemand = -1;
    bestQueue = -1;

    for (floor = 0; floor < sim->numFloors; floor++) {
        int demand = (sim->floorDemand != NULL) ? sim->floorDemand[floor] : 0;
        int queueLen = floor_queue_size(&sim->floors[floor]);

        if (queueLen <= 0 && demand <= 0) {
            continue;
        }
        if (bestFloor < 0 || demand > bestDemand ||
            (demand == bestDemand && queueLen > bestQueue)) {
            bestFloor = floor;
            bestDemand = demand;
            bestQueue = queueLen;
        }
    }
    return bestFloor;
}

static void simulation_idle_reposition_if_needed(Simulation* sim, Elevator* elevator)
{
    int targetFloor;

    if (sim == NULL || elevator == NULL) {
        return;
    }
    if (elevator->status != ELEVATOR_IDLE || elevator->doorState != DOOR_CLOSED) {
        return;
    }
    if (elevator_onboard_count(elevator) > 0 || elevator_has_any_stop(elevator)) {
        return;
    }
    if (simulation_elevator_needs_service_at_floor(sim, elevator,
                                                   elevator->currentFloor)) {
        return;
    }

    {
        int totalWaiting = 0;
        int fi;
        for (fi = 0; fi < sim->numFloors; fi++) {
            totalWaiting += floor_queue_size(&sim->floors[fi]);
        }
        if (totalWaiting > sim->numElevators * 2) {
            return;
        }
    }

    targetFloor = simulation_find_highest_demand_floor(sim);
    if (targetFloor < 0 || targetFloor == elevator->currentFloor) {
        return;
    }
    if (floor_queue_size(&sim->floors[targetFloor]) < IDLE_REPOSITION_MIN_QUEUE) {
        return;
    }

    simulation_set_direction_toward_floor(elevator, targetFloor);
    simulation_schedule_elevator_travel(sim, elevator, targetFloor);
}

static void simulation_assign_passenger_to_elevator(Simulation* sim, Passenger* passenger,
                                                    int elevatorIndex)
{
    if (sim == NULL || passenger == NULL || elevatorIndex < 0 ||
        elevatorIndex >= sim->numElevators) {
        return;
    }

    passenger->assignedElevatorId = elevatorIndex;
    elevator_add_stop(&sim->elevators[elevatorIndex], passenger->sourceFloor);
}

static void simulation_start_elevator_if_needed(Simulation* sim, Elevator* elevator)
{
    int nextStop;

    if (sim == NULL || elevator == NULL) {
        return;
    }

    if (elevator->doorState != DOOR_CLOSED) {
        return;
    }

    simulation_refresh_elevator_stops(sim, elevator->id);

    if (elevator->status != ELEVATOR_IDLE || elevator->doorState != DOOR_CLOSED) {
        return;
    }

    if (simulation_elevator_needs_service_at_floor(sim, elevator,
                                                   elevator->currentFloor)) {
        simulation_schedule_event(sim, sim->currentTime + DOOR_OPEN_TIME_SECONDS,
                                  EVENT_DOORS_OPEN,
                                  elevator->id, -1, elevator->currentFloor, -1);
        return;
    }

    nextStop = simulation_pick_next_stop_floor(sim, elevator);
    if (nextStop >= 0 && nextStop != elevator->currentFloor) {
        simulation_set_direction_toward_floor(elevator, nextStop);
        simulation_schedule_elevator_travel(sim, elevator, nextStop);
    }
}

static void simulation_release_hall_buttons_if_clear(Simulation* sim, int floor)
{
    Floor* f;

    if (!simulation_validate_floor(sim, floor)) {
        return;
    }

    f = &sim->floors[floor];
    if (f->waitingQueueFront == NULL) {
        f->upButtonPressed = 0;
        f->downButtonPressed = 0;
    }
}

/*
 * === PRESENTATION: DES main loop (put this on a slide) ===
 *
 * simulation_run - Discrete-Event Simulation driver.
 *
 * Course model / מודל הקורס:
 *   - Jump from event to event; NO real-time wait between events (no sleep).
 *   - If event 1 is at T=1 and event 2 at T=2, the clock jumps 1->2 instantly.
 *   - Loop: find event with lowest T in the Future Event List, execute it, repeat.
 *
 * Steps each iteration:
 *   1) event_list_pop_earliest  - lowest T from sorted FEL
 *   2) currentTime = event.time - simulation clock JUMP (not wall-clock delay)
 *   3) simulation_dispatch_event - run handler
 *   4) free(event)
 */
int simulation_run(Simulation* sim)
{
    Event* event;
    char msg[MAX_NAME_LEN * 4];

    if (sim == NULL) {
        return 0;
    }

    logger_begin_simulation_run();
    log_message(sim->currentTime, LOG_INFO, "Simulation started");

    /* DES loop: process Future Event List until empty or past maxSimulationTime */
    while (sim->eventList.head != NULL && sim->currentTime < sim->maxSimulationTime) {
        /* Lowest-T event in the list (ראש הרשימה הממוינת) */
        event = event_list_pop_earliest(&sim->eventList);
        if (event == NULL) {
            break;
        }

        if (event->time > sim->maxSimulationTime) {
            free(event);
            break;
        }

        statistics_advance_to_time(&sim->stats, sim, event->time);

        /* Jump simulation clock to this event's T — no waiting between events */
        sim->currentTime = event->time;

        simulation_dispatch_event(sim, event);
        free(event);
    }

    snprintf(msg, sizeof(msg), "Simulation finished at t=%.2f", sim->currentTime);
    log_message(sim->currentTime, LOG_INFO, msg);

    statistics_finalize_and_print(&sim->stats, sim);

  /* TODO: emergency events and OUT_OF_SERVICE / MAINTENANCE handling */

    return 1;
}

/*
 * simulation_press_hall_buttons - Mark up/down call for a floor.
 */
static void simulation_press_hall_buttons(Simulation* sim, int floor)
{
    if (floor > 0) {
        sim->floors[floor].upButtonPressed = 1;
    }
    if (floor < sim->numFloors - 1) {
        sim->floors[floor].downButtonPressed = 1;
    }
}

static void simulation_sort_passengers_by_request_time(Passenger** list, int count)
{
    int i;
    int j;

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (list[j]->requestTime > list[j + 1]->requestTime) {
                Passenger* tmp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = tmp;
            }
        }
    }
}

static double simulation_passenger_wait_seconds(const Simulation* sim,
                                                const Passenger* passenger)
{
    if (sim == NULL || passenger == NULL) {
        return 0.0;
    }
    return sim->currentTime - passenger->requestTime;
}

static double simulation_dispatch_priority_score(double waitSeconds)
{
    if (waitSeconds >= MAX_QUEUE_WAIT_SECONDS) {
        return waitSeconds + 1000000.0;
    }
    return waitSeconds;
}

/*
 * Pickup floor for this cab: longest-wait passenger already assigned here.
 * Returns -1 if none; optional outWait receives that wait (seconds).
 */
static int simulation_pick_urgent_pickup_floor(const Simulation* sim,
                                             const Elevator* elevator,
                                             double* outWait)
{
    int floor;
    int bestFloor;
    double bestWait;
    const Passenger* walker;

    if (sim == NULL || elevator == NULL) {
        return -1;
    }

    bestFloor = -1;
    bestWait = -1.0;

    for (floor = 0; floor < sim->numFloors; floor++) {
        for (walker = sim->floors[floor].waitingQueueFront; walker != NULL;
             walker = walker->next) {
            double wait;

            if (walker->assignedElevatorId != elevator->id) {
                continue;
            }

            wait = simulation_passenger_wait_seconds(sim, walker);
            if (bestFloor < 0 || wait > bestWait) {
                bestWait = wait;
                bestFloor = floor;
            }
        }
    }

    if (outWait != NULL) {
        if (bestFloor >= 0) {
            *outWait = bestWait;
        } else {
            *outWait = 0.0;
        }
    }

    return bestFloor;
}

/*
 * Priority for visiting a floor: longest wait among assigned waiters and
 * time since boarding for passengers who exit here.
 */
static double simulation_floor_stop_priority(const Simulation* sim,
                                             const Elevator* elevator,
                                             int floor)
{
    double maxWait;
    const Passenger* walker;

    if (sim == NULL || elevator == NULL ||
        !simulation_validate_floor(sim, floor)) {
        return 0.0;
    }

    maxWait = 0.0;

    for (walker = sim->floors[floor].waitingQueueFront; walker != NULL;
         walker = walker->next) {
        if (walker->assignedElevatorId == elevator->id) {
            double wait = simulation_passenger_wait_seconds(sim, walker);
            double score = simulation_dispatch_priority_score(wait);
            if (score > maxWait) {
                maxWait = score;
            }
        }
    }

    for (walker = elevator->onboardHead; walker != NULL; walker = walker->onboardNext) {
        if (walker->destinationFloor == floor) {
            double ride;
            if (walker->boardTime >= 0.0) {
                ride = sim->currentTime - walker->boardTime;
            } else {
                ride = simulation_passenger_wait_seconds(sim, walker);
            }
            if (ride > maxWait) {
                maxWait = ride;
            }
        }
    }

    return maxWait;
}

static int simulation_stop_ahead_in_direction(const Elevator* elevator, int floor)
{
    if (elevator == NULL) {
        return 0;
    }
    if (elevator->direction == DIR_NONE) {
        return floor != elevator->currentFloor;
    }
    if (elevator->direction == DIR_UP) {
        return floor > elevator->currentFloor;
    }
    if (elevator->direction == DIR_DOWN) {
        return floor < elevator->currentFloor;
    }
    return floor != elevator->currentFloor;
}

/*
 * Among SCAN-feasible stops, pick the floor with the longest waiting passengers;
 * break ties by shorter travel distance.
 */
static int simulation_pick_stop_from_candidates(const Simulation* sim,
                                                const Elevator* elevator,
                                                int requireAhead)
{
    int floor;
    int bestFloor;
    double bestWait;
    int bestDistance;

    if (sim == NULL || elevator == NULL) {
        return -1;
    }

    bestFloor = -1;
    bestWait = -1.0;
    bestDistance = -1;

    for (floor = 0; floor < elevator->numFloors; floor++) {
        int distance;
        double wait;

        if (!elevator_has_stop(elevator, floor)) {
            continue;
        }
        if (requireAhead && !simulation_stop_ahead_in_direction(elevator, floor)) {
            continue;
        }
        if (!requireAhead && simulation_stop_ahead_in_direction(elevator, floor)) {
            continue;
        }

        wait = simulation_floor_stop_priority(sim, elevator, floor);
        wait = simulation_dispatch_priority_score(wait);
        distance = elevator->currentFloor - floor;
        if (distance < 0) {
            distance = -distance;
        }

        if (bestFloor < 0 || wait > bestWait + 0.001 ||
            (wait >= bestWait - 0.001 && distance < bestDistance)) {
            bestFloor = floor;
            bestWait = wait;
            bestDistance = distance;
        }
    }

    return bestFloor;
}

/*
 * === PRESENTATION: Cab routing (after doors close) ===
 * Priority: service current floor -> urgent assigned pickups -> SCAN stops.
 */
static int simulation_pick_next_stop_floor(const Simulation* sim,
                                           const Elevator* elevator)
{
    int nextStop;
    int urgentPickup;
    double pickupWait;

    if (sim == NULL || elevator == NULL) {
        return -1;
    }

    if (simulation_elevator_needs_service_at_floor(sim, elevator,
                                                   elevator->currentFloor)) {
        return elevator->currentFloor;
    }

    /* Serve assigned pickups before any other stop (prevents multi-minute queue waits). */
    urgentPickup = simulation_pick_urgent_pickup_floor(sim, elevator, &pickupWait);
    if (urgentPickup >= 0 && pickupWait > 0.0 &&
        urgentPickup != elevator->currentFloor) {
        return urgentPickup;
    }

    nextStop = simulation_pick_stop_from_candidates(sim, elevator, 1);
    if (nextStop >= 0) {
        return nextStop;
    }

    nextStop = simulation_pick_stop_from_candidates(sim, elevator, 0);
    if (nextStop >= 0) {
        return nextStop;
    }

    return elevator_next_stop_floor(elevator);
}

static int simulation_find_highest_priority_floor(const Simulation* sim)
{
    int floorIndex;
    int bestFloor;
    double bestScore;
    const Passenger* walker;

    if (sim == NULL) {
        return -1;
    }

    bestFloor = -1;
    bestScore = -1.0;

    for (floorIndex = 0; floorIndex < sim->numFloors; floorIndex++) {
        for (walker = sim->floors[floorIndex].waitingQueueFront; walker != NULL;
             walker = walker->next) {
            double wait;
            double score;

            wait = simulation_passenger_wait_seconds(sim, walker);
            score = simulation_dispatch_priority_score(wait);

            if (bestFloor < 0 || score > bestScore) {
                bestScore = score;
                bestFloor = floorIndex;
            }
        }
    }

    return bestFloor;
}

/* Nudge cabs toward their longest-waiting assigned pickup (3-minute SLA target). */
static void simulation_enforce_queue_wait_policy(Simulation* sim)
{
    int elevatorIndex;

    if (sim == NULL) {
        return;
    }

    for (elevatorIndex = 0; elevatorIndex < sim->numElevators; elevatorIndex++) {
        Elevator* elevator = &sim->elevators[elevatorIndex];
        double pickupWait;

        if (simulation_pick_urgent_pickup_floor(sim, elevator, &pickupWait) < 0) {
            continue;
        }
        if (pickupWait < MAX_QUEUE_WAIT_SECONDS * 0.5) {
            continue;
        }

        if (pickupWait >= MAX_QUEUE_WAIT_SECONDS &&
            elevator->doorState == DOOR_CLOSED &&
            elevator->status == ELEVATOR_IDLE) {
            int urgentFloor = simulation_pick_urgent_pickup_floor(sim, elevator, NULL);
            if (urgentFloor >= 0 && urgentFloor != elevator->currentFloor) {
                simulation_set_direction_toward_floor(elevator, urgentFloor);
                simulation_schedule_elevator_travel(sim, elevator, urgentFloor);
                continue;
            }
        }

        simulation_start_elevator_if_needed(sim, elevator);
    }
}

static void simulation_set_direction_from_onboard(Elevator* elevator)
{
    const Passenger* current;
    int maxDest;
    int minDest;
    int hasUp;
    int hasDown;

    maxDest = elevator->currentFloor;
    minDest = elevator->currentFloor;
    hasUp = 0;
    hasDown = 0;

    for (current = elevator->onboardHead; current != NULL;
         current = current->onboardNext) {
        if (current->destinationFloor > elevator->currentFloor) {
            hasUp = 1;
            if (current->destinationFloor > maxDest) {
                maxDest = current->destinationFloor;
            }
        } else if (current->destinationFloor < elevator->currentFloor) {
            hasDown = 1;
            if (current->destinationFloor < minDest) {
                minDest = current->destinationFloor;
            }
        }
    }

    if (hasUp && !hasDown) {
        elevator->direction = DIR_UP;
    } else if (hasDown && !hasUp) {
        elevator->direction = DIR_DOWN;
    } else if (hasUp) {
        elevator->direction = DIR_UP;
    } else {
        elevator->direction = DIR_NONE;
    }
}

/*
 * Cluster unassigned waiters by destination gap; assign nearest idle cab per cluster.
 * Example: floor 0 -> dest 3,5 | 10,15 | 50 on three cabs; fourth stays idle.
 */
static int simulation_dispatch_direction_group(Simulation* sim, int floorNum,
                                               Passenger** group, int groupCount,
                                               int goingUp)
{
    int index;
    int assigned;
    char msg[MAX_NAME_LEN * 4];

    if (groupCount <= 0) {
        return 0;
    }

    assigned = 0;
    simulation_sort_passengers_by_request_time(group, groupCount);

    index = 0;
    while (index < groupCount) {
        int elevatorIndex;
        Elevator* elevator;
        int clusterStart;
        int clusterCount;
        int clusterMin;
        int clusterMax;
        int i;

        {
            double headWait = simulation_passenger_wait_seconds(sim, group[index]);

            elevatorIndex = simulation_find_elevator_for_pickup(
                sim, floorNum, group[index]->destinationFloor, headWait);
        }
        if (elevatorIndex < 0) {
            break;
        }
        if (simulation_elevator_free_slots(sim, elevatorIndex) <= 0) {
            break;
        }

        clusterStart = index;
        clusterCount = 1;
        clusterMin = group[index]->destinationFloor;
        clusterMax = clusterMin;
        index++;

        while (index < groupCount &&
               clusterCount < simulation_elevator_free_slots(sim, elevatorIndex)) {
            int newDest = group[index]->destinationFloor;
            int newMin;
            int newMax;
            int span;
            int maxSpan;
            double clusterWait;

            if (goingUp) {
                if (newDest <= clusterMax) {
                    break;
                }
            } else if (newDest >= clusterMin) {
                break;
            }

            newMin = clusterMin;
            newMax = clusterMax;
            if (newDest < newMin) {
                newMin = newDest;
            }
            if (newDest > newMax) {
                newMax = newDest;
            }
            span = newMax - newMin;
            clusterWait = simulation_passenger_wait_seconds(sim, group[clusterStart]);
            maxSpan = simulation_cluster_max_span_for_wait(clusterWait);
            if (span > maxSpan) {
                break;
            }
            clusterCount++;
            clusterMin = newMin;
            clusterMax = newMax;
            index++;
        }

        elevator = &sim->elevators[elevatorIndex];
        for (i = clusterStart; i < clusterStart + clusterCount; i++) {
            simulation_assign_passenger_to_elevator(sim, group[i], elevatorIndex);
            assigned++;
        }

        simulation_press_hall_buttons(sim, floorNum);
        simulation_refresh_elevator_stops(sim, elevatorIndex);

        snprintf(msg, sizeof(msg),
                 "Elevator %d (%s, floor %d) assigned %d passenger(s) at floor %d (dest %d-%d)",
                 elevator->id,
                 elevator->status == ELEVATOR_MOVING ? "on the way" : "idle",
                 elevator->currentFloor, clusterCount, floorNum, clusterMin, clusterMax);
        log_message(sim->currentTime, LOG_INFO, msg);

        simulation_start_elevator_if_needed(sim, elevator);
    }

    return assigned;
}

static int simulation_dispatch_floor_clustered(Simulation* sim, int floorNum)
{
    Floor* floor;
    Passenger* upGroup[MAX_SEED_REQUESTS];
    Passenger* downGroup[MAX_SEED_REQUESTS];
    int upCount;
    int downCount;
    int assigned;
    const Passenger* current;

    if (!simulation_validate_floor(sim, floorNum)) {
        return 0;
    }

    floor = &sim->floors[floorNum];
    upCount = 0;
    downCount = 0;
    assigned = 0;

    for (current = floor->waitingQueueFront; current != NULL; current = current->next) {
        if (current->assignedElevatorId >= 0) {
            continue;
        }
        if (current->destinationFloor > floorNum) {
            if (upCount < MAX_SEED_REQUESTS) {
                upGroup[upCount++] = (Passenger*)current;
            }
        } else if (current->destinationFloor < floorNum) {
            if (downCount < MAX_SEED_REQUESTS) {
                downGroup[downCount++] = (Passenger*)current;
            }
        }
    }

    assigned += simulation_dispatch_direction_group(sim, floorNum, upGroup, upCount, 1);
    assigned += simulation_dispatch_direction_group(sim, floorNum, downGroup, downCount, 0);
    return assigned;
}

static int simulation_dispatch_same_floor_waiters(Simulation* sim, int floorNum)
{
    Floor* floor;
    Passenger* walker;
    int assigned;

    if (!simulation_validate_floor(sim, floorNum)) {
        return 0;
    }

    floor = &sim->floors[floorNum];
    assigned = 0;

    for (walker = floor->waitingQueueFront; walker != NULL; walker = walker->next) {
        double wait;
        int elevatorIndex;
        char msg[MAX_NAME_LEN * 4];

        if (walker->assignedElevatorId >= 0) {
            continue;
        }
        if (walker->destinationFloor != floorNum) {
            continue;
        }

        wait = simulation_passenger_wait_seconds(sim, walker);
        elevatorIndex = simulation_find_elevator_for_pickup(sim, floorNum, floorNum, wait);
        if (elevatorIndex < 0 || simulation_elevator_free_slots(sim, elevatorIndex) <= 0) {
            continue;
        }

        simulation_assign_passenger_to_elevator(sim, walker, elevatorIndex);
        simulation_press_hall_buttons(sim, floorNum);
        simulation_refresh_elevator_stops(sim, elevatorIndex);
        simulation_start_elevator_if_needed(sim, &sim->elevators[elevatorIndex]);
        assigned++;

        snprintf(msg, sizeof(msg),
                 "Elevator %d assigned same-floor passenger %d at floor %d",
                 elevatorIndex, walker->id, floorNum);
        log_message(sim->currentTime, LOG_INFO, msg);
    }

    return assigned;
}

/*
 * === PRESENTATION: Global greedy assignment ===
 * One pass: best (unassigned passenger, elevator) pair by dispatch score.
 */
static int simulation_batch_dispatch_round(Simulation* sim)
{
    int floorIndex;
    const Passenger* walker;
    const Passenger* bestPassenger;
    int bestFloor;
    int bestElevator;
    double bestScore;
    double score;
    double wait;
    double eta;
    int e;

    if (sim == NULL) {
        return 0;
    }

    bestPassenger = NULL;
    bestFloor = -1;
    bestElevator = -1;
    bestScore = 1e12;

    for (floorIndex = 0; floorIndex < sim->numFloors; floorIndex++) {
        for (walker = sim->floors[floorIndex].waitingQueueFront; walker != NULL;
             walker = walker->next) {
            if (walker->assignedElevatorId >= 0) {
                continue;
            }

            wait = simulation_passenger_wait_seconds(sim, walker);
            for (e = 0; e < sim->numElevators; e++) {
                if (simulation_elevator_free_slots(sim, e) <= 0) {
                    continue;
                }
                eta = simulation_estimate_pickup_eta(sim, e, floorIndex,
                                                     walker->destinationFloor);
                if (eta >= 1e8) {
                    continue;
                }
                if (sim->numElevators < 30 && sim->elevators[e].status == ELEVATOR_MOVING) {
                    if (eta + wait > MOVING_PICKUP_MAX_SUM_WAIT) {
                        continue;
                    }
                } else if (sim->elevators[e].status == ELEVATOR_MOVING) {
                    continue;
                }
                score = simulation_dispatch_score(eta, simulation_elevator_load(sim, e),
                                                  wait);
                if (bestPassenger == NULL || score < bestScore) {
                    bestScore = score;
                    bestPassenger = walker;
                    bestFloor = floorIndex;
                    bestElevator = e;
                }
            }
        }
    }

    if (bestPassenger == NULL || bestElevator < 0) {
        return 0;
    }

    {
        char msg[MAX_NAME_LEN * 4];

        simulation_assign_passenger_to_elevator(sim, (Passenger*)bestPassenger,
                                                bestElevator);
        simulation_press_hall_buttons(sim, bestFloor);
        simulation_refresh_elevator_stops(sim, bestElevator);
        simulation_start_elevator_if_needed(sim, &sim->elevators[bestElevator]);

        snprintf(msg, sizeof(msg),
                 "Batch: elevator %d <- passenger %d floor %d (wait %.1f s, score %.1f)",
                 bestElevator, bestPassenger->id, bestFloor,
                 simulation_passenger_wait_seconds(sim, bestPassenger), bestScore);
        log_message(sim->currentTime, LOG_INFO, msg);
    }

    return 1;
}

static void simulation_service_waiting_queues(Simulation* sim)
{
    int floorIndex;
    int assigned;
    int pass;
    int maxPasses;

    if (sim == NULL) {
        return;
    }

    maxPasses = MAX_SEED_REQUESTS + sim->numElevators * 4;
    for (pass = 0; pass < maxPasses; pass++) {
        if (simulation_batch_dispatch_round(sim)) {
            continue;
        }

        floorIndex = simulation_find_highest_priority_floor(sim);
        if (floorIndex < 0) {
            break;
        }

        assigned = simulation_dispatch_same_floor_waiters(sim, floorIndex);
        assigned += simulation_dispatch_floor_clustered(sim, floorIndex);
        if (assigned <= 0) {
            break;
        }
    }
}

static void simulation_alight_all_at_floor(Simulation* sim, Elevator* elevator, int floor)
{
    Passenger* current;
    Passenger* next;
    Passenger** previousPtr;

    previousPtr = &elevator->onboardHead;
    current = elevator->onboardHead;

    while (current != NULL) {
        next = current->onboardNext;
        if (current->destinationFloor == floor) {
            char msg[MAX_NAME_LEN * 4];
            *previousPtr = next;
            elevator->passengerCount--;
            current->onboardNext = NULL;
            current->status = PASSENGER_ARRIVED;
            statistics_on_passenger_served(&sim->stats, current, sim->currentTime);
            snprintf(msg, sizeof(msg), "Passenger %d exited elevator %d at floor %d",
                     current->id, elevator->id, floor);
            log_message(sim->currentTime, LOG_INFO, msg);
            passenger_destroy(current);
            current = next;
        } else {
            previousPtr = &current->onboardNext;
            current = next;
        }
    }
}

static void simulation_board_assigned_at_floor(Simulation* sim, Elevator* elevator, int floor)
{
    Passenger* chain;
    Passenger* current;
    char msg[MAX_NAME_LEN * 4];
    int boarded;

    chain = floor_take_assigned_passengers(&sim->floors[floor], elevator->id,
                                           sim->elevatorCapacity -
                                               elevator->passengerCount);

    boarded = 0;
    current = chain;
    while (current != NULL) {
        Passenger* nextInChain = current->next;
        current->next = NULL;
        current->status = PASSENGER_IN_ELEVATOR;
        current->boardTime = sim->currentTime;
        statistics_on_passenger_boarded(&sim->stats, current, sim->currentTime);
        elevator_add_onboard(elevator, current);
        boarded++;
        current = nextInChain;
    }

    if (boarded > 0) {
        snprintf(msg, sizeof(msg), "%d passenger(s) boarded elevator %d at floor %d",
                 boarded, elevator->id, floor);
        log_message(sim->currentTime, LOG_INFO, msg);
        simulation_set_direction_from_onboard(elevator);
        simulation_refresh_elevator_stops(sim, elevator->id);
    }
}

/*
 * simulation_find_passenger_in_queue - Walk floor queue to find passenger by id.
 * Used to verify PASSENGER_CALL matches a queued passenger. Returns NULL if missing.
 */
static Passenger* simulation_find_passenger_in_queue(Floor* floor, int passengerId)
{
    Passenger* current = floor->waitingQueueFront;
    while (current != NULL) {
        if (current->id == passengerId) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/*
 * === PRESENTATION: Passenger arrives (PASSENGER_CALL event) ===
 * Enqueue on floor, record demand, then simulation_service_waiting_queues()
 * assigns cabs (batch + clustering). May warn if no cab available yet.
 */
void handle_passenger_call(Simulation* sim, Event* event)
{
    Passenger* passenger;
    char msg[MAX_NAME_LEN * 4];
    int sourceFloor = event->floor;
    int passengerId = event->passengerId;

    if (!simulation_find_passenger_in_queue(&sim->floors[sourceFloor], passengerId)) {
        if (event->destinationFloor < 0 ||
            !simulation_validate_floor(sim, event->destinationFloor)) {
            log_message(sim->currentTime, LOG_WARNING,
                        "Passenger not found and no destination in call event");
            return;
        }

        passenger = passenger_create(passengerId, sourceFloor, event->destinationFloor,
                                     sim->currentTime);
        if (passenger == NULL) {
            log_message(sim->currentTime, LOG_ERROR, "Failed to create passenger at arrival");
            return;
        }
        floor_enqueue_passenger(&sim->floors[sourceFloor], passenger);
    }

    statistics_on_passenger_request(&sim->stats, sim);
    simulation_record_floor_demand(sim, sourceFloor);

    snprintf(msg, sizeof(msg), "Processing call for passenger %d on floor %d",
             passengerId, sourceFloor);
    log_message(sim->currentTime, LOG_INFO, msg);

    if (simulation_more_passenger_calls_same_time_floor(sim, sim->currentTime,
                                                        sourceFloor)) {
        return;
    }

    simulation_service_waiting_queues(sim);

    if (simulation_find_passenger_in_queue(&sim->floors[sourceFloor], passengerId) !=
            NULL &&
        simulation_find_passenger_in_queue(&sim->floors[sourceFloor], passengerId)
                ->assignedElevatorId < 0) {
        log_message(sim->currentTime, LOG_WARNING,
                    "No idle elevator available - will retry when a cab is free");
    }
}

/*
 * handle_elevator_arrival - Cab reached event->floor after travel time; update position.
 */
void handle_elevator_arrival(Simulation* sim, Event* event)
{
    Elevator* elevator;
    char msg[MAX_NAME_LEN * 4];

    if (event->elevatorId < 0 || event->elevatorId >= sim->numElevators) {
        log_message(sim->currentTime, LOG_ERROR, "Invalid elevator in arrival event");
        return;
    }

    elevator = &sim->elevators[event->elevatorId];
    elevator->currentFloor = event->floor;
    elevator->targetFloor = event->floor;
    elevator->status = ELEVATOR_IDLE;
    elevator->doorState = DOOR_CLOSED;

    snprintf(msg, sizeof(msg), "Elevator %d arrived at floor %d",
             elevator->id, event->floor);
    log_message(sim->currentTime, LOG_INFO, msg);

    simulation_schedule_event(sim, sim->currentTime + DOOR_OPEN_TIME_SECONDS,
                              EVENT_DOORS_OPEN,
                              elevator->id, event->passengerId, event->floor, -1);
}

/*
 * === PRESENTATION: Boarding / alighting (DOORS_OPEN event) ===
 * Alight anyone whose destination is this floor; board waiters assigned to this cab.
 * Logs elevator id + onboard passenger ids for the trace file.
 */
void handle_doors_open(Simulation* sim, Event* event)
{
    Elevator* elevator;
    char msg[MAX_NAME_LEN * 4];
    char onboard[MAX_NAME_LEN * 4];

    if (event->elevatorId < 0 || event->elevatorId >= sim->numElevators) {
        return;
    }

    elevator = &sim->elevators[event->elevatorId];

    if (elevator->doorState == DOOR_OPEN) {
        return;
    }

    elevator->doorState = DOOR_OPEN;
    elevator->status = ELEVATOR_IDLE;

    simulation_format_onboard_list(sim, elevator->id, onboard, sizeof(onboard));
    snprintf(msg, sizeof(msg),
             "Elevator %d doors opened at floor %d (onboard: %s)",
             elevator->id, event->floor, onboard);
    log_message(sim->currentTime, LOG_INFO, msg);

    simulation_alight_all_at_floor(sim, elevator, event->floor);

    simulation_board_assigned_at_floor(sim, elevator, event->floor);

    simulation_refresh_elevator_stops(sim, elevator->id);
    simulation_release_hall_buttons_if_clear(sim, event->floor);

    simulation_schedule_event(sim, sim->currentTime + DOOR_DWELL_SECONDS,
                              EVENT_DOORS_CLOSE,
                              elevator->id, -1, event->floor, -1);
}

/*
 * handle_doors_close - Close doors; if carrying passenger, move to event->floor (dest)
 * and schedule another ELEVATOR_ARRIVAL. Otherwise set elevator IDLE.
 */
void handle_doors_close(Simulation* sim, Event* event)
{
    Elevator* elevator;
    char msg[MAX_NAME_LEN * 4];
    char onboard[MAX_NAME_LEN * 4];
    int nextStop;

    if (event->elevatorId < 0 || event->elevatorId >= sim->numElevators) {
        return;
    }

    elevator = &sim->elevators[event->elevatorId];
    elevator->doorState = DOOR_CLOSED;

    simulation_format_onboard_list(sim, elevator->id, onboard, sizeof(onboard));
    snprintf(msg, sizeof(msg),
             "Elevator %d doors closed at floor %d (onboard: %s)",
             elevator->id, event->floor, onboard);
    log_message(sim->currentTime, LOG_INFO, msg);

    simulation_refresh_elevator_stops(sim, elevator->id);

    if (elevator_onboard_count(elevator) > 0) {
        simulation_set_direction_from_onboard(elevator);
    }

    if (simulation_elevator_needs_service_at_floor(sim, elevator,
                                                   elevator->currentFloor)) {
        simulation_schedule_event(sim, sim->currentTime + DOOR_OPEN_TIME_SECONDS,
                                  EVENT_DOORS_OPEN,
                                  elevator->id, -1, elevator->currentFloor, -1);
        return;
    }

    if (elevator_has_any_stop(elevator) || elevator_onboard_count(elevator) > 0) {
        nextStop = simulation_pick_next_stop_floor(sim, elevator);
        if (nextStop >= 0 && nextStop != elevator->currentFloor) {
            simulation_set_direction_toward_floor(elevator, nextStop);
            simulation_schedule_elevator_travel(sim, elevator, nextStop);
        } else if (elevator_onboard_count(elevator) == 0 &&
                   !elevator_has_any_stop(elevator)) {
            elevator->status = ELEVATOR_IDLE;
            elevator->direction = DIR_NONE;
            simulation_service_waiting_queues(sim);
            simulation_idle_reposition_if_needed(sim, elevator);
        }
    } else {
        elevator->status = ELEVATOR_IDLE;
        elevator->direction = DIR_NONE;
        simulation_service_waiting_queues(sim);
        simulation_idle_reposition_if_needed(sim, elevator);
    }
}

/*
 * handle_passenger_exit - Remove passenger from cab, free memory, schedule doors close.
 * event->floor is where exit happens (destination).
 */
void handle_passenger_exit(Simulation* sim, Event* event)
{
    (void)sim;
    (void)event;
    /* Alighting handled in handle_doors_open (ride-sharing). */
}
