/*
 * simulation.c - Discrete Event Simulation engine for the elevator system
 *
 * Owns the main DES loop (simulation_run), event scheduling, and all handle_*()
 * functions that change building state when events fire.
 */
#include "simulation.h"
#include "constants.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * simulation_schedule_event - Create an event and insert it into the FEL.
 * Logs creation at current sim time (not necessarily at event->time).
 * Internal: only scheduling entry point for new events.
 */
static void simulation_schedule_event(Simulation* sim, double time, EventType type,
                                      int elevatorId, int passengerId, int floor)
{
    Event* event;
    char desc[MAX_NAME_LEN * 4];

    event = event_create(time, type, elevatorId, passengerId, floor);
    if (event == NULL) {
        log_message(sim->currentTime, LOG_ERROR, "Failed to allocate event");
        return;
    }

    event_list_insert_sorted(&sim->eventList, event);

    snprintf(desc, sizeof(desc), "%s (elev=%d pass=%d floor=%d)",
             event_type_to_string(type), elevatorId, passengerId, floor);
    log_event_created(sim->currentTime, desc);
}

/*
 * simulation_dispatch_event - Route one event to the correct handler by type.
 * Called after currentTime has been advanced to event->time.
 */
static void simulation_dispatch_event(Simulation* sim, Event* event)
{
    char desc[MAX_NAME_LEN * 4];

    snprintf(desc, sizeof(desc), "%s", event_type_to_string(event->type));
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
}

/* simulation_validate_floor - Return 1 if floor index is valid for this building. */
int simulation_validate_floor(const Simulation* sim, int floor)
{
    return floor >= 0 && floor < sim->numFloors;
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
    sim->numFloors = config->numFloors;
    sim->numElevators = config->numElevators;
    sim->elevatorCapacity = config->capacity;
    sim->nextPassengerId = 1;

    sim->elevators = (Elevator*)calloc((size_t)sim->numElevators, sizeof(Elevator));
    sim->floors = (Floor*)calloc((size_t)sim->numFloors, sizeof(Floor));
    sim->activePassengersByElevator =
        (Passenger**)calloc((size_t)sim->numElevators, sizeof(Passenger*));

    if (sim->elevators == NULL || sim->floors == NULL ||
        sim->activePassengersByElevator == NULL) {
        simulation_destroy(sim);
        return 0;
    }

    for (i = 0; i < sim->numElevators; i++) {
        elevator_init(&sim->elevators[i], i, sim->elevatorCapacity);
    }

    for (i = 0; i < sim->numFloors; i++) {
        floor_init(&sim->floors[i], i);
    }

    event_list_init(&sim->eventList);
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

    if (sim->activePassengersByElevator != NULL && sim->elevators != NULL) {
        for (i = 0; i < sim->numElevators; i++) {
            if (sim->activePassengersByElevator[i] != NULL) {
                passenger_destroy(sim->activePassengersByElevator[i]);
                sim->activePassengersByElevator[i] = NULL;
            }
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

    free(sim->activePassengersByElevator);
    sim->activePassengersByElevator = NULL;
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
                              -1, passenger->id, sourceFloor);
    sim->nextPassengerId++;
}

/* simulation_print_state - Debug dump: time, elevators, floors, FEL. */
void simulation_print_state(const Simulation* sim)
{
    int i;

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

    event_list_print(&sim->eventList);
    printf("==========================================\n\n");
}

/*
 * simulation_run - DES main loop: process events in time order until done or max time.
 * 1) Pop earliest event  2) Set currentTime  3) Dispatch handler  4) Free event node.
 * Stops if next event time exceeds maxSimulationTime (event discarded).
 */
int simulation_run(Simulation* sim)
{
    Event* event;
    char msg[MAX_NAME_LEN * 4];

    if (sim == NULL) {
        return 0;
    }

    log_message(sim->currentTime, LOG_INFO, "Simulation started");

    while (sim->eventList.head != NULL && sim->currentTime < sim->maxSimulationTime) {
        event = event_list_pop_earliest(&sim->eventList);
        if (event == NULL) {
            break;
        }

        if (event->time > sim->maxSimulationTime) {
            free(event);
            break;
        }

        sim->currentTime = event->time;
        simulation_dispatch_event(sim, event);
        free(event);
    }

    snprintf(msg, sizeof(msg), "Simulation finished at t=%.2f", sim->currentTime);
    log_message(sim->currentTime, LOG_INFO, msg);

  /* TODO: statistics calculations - wait times, throughput */
  /* TODO: utilization reports and performance analytics */
  /* TODO: emergency events and OUT_OF_SERVICE / MAINTENANCE handling */

    return 1;
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
 * handle_passenger_call - Try to assign first idle elevator to waiting passenger.
 * Sets hall buttons, moves elevator (instant in phase 1), schedules ELEVATOR_ARRIVAL.
 * If no idle cab, passenger stays in queue (phase 2: smarter dispatch).
 */
void handle_passenger_call(Simulation* sim, Event* event)
{
    int elevatorIndex;
    Elevator* elevator;
    char msg[MAX_NAME_LEN * 4];
    int sourceFloor = event->floor;
    int passengerId = event->passengerId;

    if (!simulation_find_passenger_in_queue(&sim->floors[sourceFloor], passengerId)) {
        log_message(sim->currentTime, LOG_WARNING,
                    "Passenger not found in floor queue for call event");
        return;
    }

    /* TODO: passenger travel timing and full lifecycle simulation */

    snprintf(msg, sizeof(msg), "Processing call for passenger %d on floor %d",
             passengerId, sourceFloor);
    log_message(sim->currentTime, LOG_INFO, msg);

    if (sourceFloor > 0) {
        sim->floors[sourceFloor].upButtonPressed = 1;
    }
    if (sourceFloor < sim->numFloors - 1) {
        sim->floors[sourceFloor].downButtonPressed = 1;
    }

    elevatorIndex = elevator_find_first_idle(sim->elevators, sim->numElevators);
    if (elevatorIndex < 0) {
        log_message(sim->currentTime, LOG_WARNING,
                    "No idle elevator available - request queued");
      /* TODO: advanced dispatch algorithms and smarter scheduling */
        return;
    }

    elevator = &sim->elevators[elevatorIndex];
    snprintf(msg, sizeof(msg), "Assigning elevator %d to floor %d",
             elevator->id, sourceFloor);
    log_message(sim->currentTime, LOG_INFO, msg);

    elevator_assign_to_floor(elevator, sourceFloor);

    /* Instant arrival for foundation phase */
    simulation_schedule_event(sim, sim->currentTime, EVENT_ELEVATOR_ARRIVAL,
                              elevator->id, passengerId, sourceFloor);
}

/*
 * handle_elevator_arrival - Cab reached event->floor; update position, open doors next.
 * TODO phase 2: position updated only here after travel delay, not in assign.
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
    elevator->status = ELEVATOR_MOVING;

    snprintf(msg, sizeof(msg), "Elevator %d arrived at floor %d",
             elevator->id, event->floor);
    log_message(sim->currentTime, LOG_INFO, msg);

  /* TODO: realistic elevator movement between floors */

    simulation_schedule_event(sim, sim->currentTime, EVENT_DOORS_OPEN,
                              elevator->id, event->passengerId, event->floor);
}

/*
 * handle_doors_open - Open doors: either drop off at destination or board from queue.
 * If passenger on board and floor == destination -> schedule PASSENGER_EXIT.
 * Else dequeue one waiter from this floor and schedule DOORS_CLOSE toward their dest.
 */
void handle_doors_open(Simulation* sim, Event* event)
{
    Elevator* elevator;
    Passenger* passengerOnBoard;
    Passenger* waitingPassenger;

    if (event->elevatorId < 0 || event->elevatorId >= sim->numElevators) {
        return;
    }

    elevator = &sim->elevators[event->elevatorId];
    elevator->doorState = DOOR_OPEN;
    elevator->status = ELEVATOR_IDLE;

    log_message(sim->currentTime, LOG_INFO, "Doors opened");

    passengerOnBoard = sim->activePassengersByElevator[elevator->id];
    if (passengerOnBoard != NULL &&
        passengerOnBoard->destinationFloor == event->floor) {
      /* Arrived at destination - passenger exits */
        simulation_schedule_event(sim, sim->currentTime + 0.2, EVENT_PASSENGER_EXIT,
                                  elevator->id, passengerOnBoard->id, event->floor);
        return;
    }

    waitingPassenger = floor_dequeue_passenger(&sim->floors[event->floor]);
    if (waitingPassenger != NULL) {
      /* TODO: overload detection when passengerCount >= capacity */
        waitingPassenger->status = PASSENGER_IN_ELEVATOR;
        elevator->passengerCount++;
        sim->activePassengersByElevator[elevator->id] = waitingPassenger;
        log_message(sim->currentTime, LOG_INFO, "Passenger boarded elevator");

        simulation_schedule_event(sim, sim->currentTime + 0.5, EVENT_DOORS_CLOSE,
                                  elevator->id, waitingPassenger->id,
                                  waitingPassenger->destinationFloor);
    } else {
        simulation_schedule_event(sim, sim->currentTime + 0.5, EVENT_DOORS_CLOSE,
                                  elevator->id, -1, event->floor);
    }
}

/*
 * handle_doors_close - Close doors; if carrying passenger, move to event->floor (dest)
 * and schedule another ELEVATOR_ARRIVAL. Otherwise set elevator IDLE.
 */
void handle_doors_close(Simulation* sim, Event* event)
{
    Elevator* elevator;

    if (event->elevatorId < 0 || event->elevatorId >= sim->numElevators) {
        return;
    }

    elevator = &sim->elevators[event->elevatorId];
    elevator->doorState = DOOR_CLOSED;

    log_message(sim->currentTime, LOG_INFO, "Doors closed");

    if (event->passengerId >= 0 && event->floor >= 0 &&
        simulation_validate_floor(sim, event->floor) &&
        sim->activePassengersByElevator[elevator->id] != NULL) {
        elevator_assign_to_floor(elevator, event->floor);
        simulation_schedule_event(sim, sim->currentTime, EVENT_ELEVATOR_ARRIVAL,
                                  elevator->id, event->passengerId, event->floor);
    } else {
        elevator->status = ELEVATOR_IDLE;
        elevator->direction = DIR_NONE;
    }

  /* TODO: energy consumption tracking per trip */
}

/*
 * handle_passenger_exit - Remove passenger from cab, free memory, schedule doors close.
 * event->floor is where exit happens (destination).
 */
void handle_passenger_exit(Simulation* sim, Event* event)
{
    Elevator* elevator;
    Passenger* passenger;

    if (event->elevatorId < 0 || event->elevatorId >= sim->numElevators) {
        return;
    }

    elevator = &sim->elevators[event->elevatorId];
    passenger = sim->activePassengersByElevator[elevator->id];

    if (elevator->passengerCount > 0) {
        elevator->passengerCount--;
    }

    if (passenger != NULL) {
        passenger->status = PASSENGER_ARRIVED;
        passenger_destroy(passenger);
        sim->activePassengersByElevator[elevator->id] = NULL;
    }

    log_message(sim->currentTime, LOG_INFO, "Passenger exited elevator");
    elevator->status = ELEVATOR_IDLE;
    elevator->direction = DIR_NONE;

    simulation_schedule_event(sim, sim->currentTime + 0.5, EVENT_DOORS_CLOSE,
                              elevator->id, -1, event->floor);

  /* TODO: final polish for passenger exit flow and statistics */
}
