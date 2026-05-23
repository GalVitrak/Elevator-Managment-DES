#ifndef EVENT_H
#define EVENT_H

/*
 * event.h - Future Event List (FEL) types
 * PRESENTATION: Five event types drive all state changes; Event is a linked-list node.
 */

/* Types of discrete events processed by the simulation engine. */
typedef enum {
    EVENT_PASSENGER_CALL,
    EVENT_ELEVATOR_ARRIVAL,
    EVENT_DOORS_OPEN,
    EVENT_DOORS_CLOSE,
    EVENT_PASSENGER_EXIT
} EventType;

/*
 * One node in the Future Event List (FEL).
 * elevatorId / passengerId: use -1 when not applicable (e.g. door events
 * serve the whole cab; logs list onboard passengers instead of pass=).
 * floor: meaning depends on event type (source, destination, or current floor).
 */
typedef struct Event {
    double time;
    EventType type;
    int elevatorId;
    int passengerId;
    int floor;
    int destinationFloor;   /* for PASSENGER_CALL: target floor; else -1 */
    struct Event* next;
} Event;

/* Head pointer and count for the sorted Future Event List. */
typedef struct {
    Event* head;
    int size;
} EventList;

/* Set list to empty (does not free existing nodes - use destroy for that). */
void event_list_init(EventList* list);

/* Free every event node and reset list to empty. */
void event_list_destroy(EventList* list);

/* Allocate one event; caller must insert into list or free on failure path. */
Event* event_create(double time, EventType type, int elevatorId,
                    int passengerId, int floor, int destinationFloor);

/* Insert event so list stays sorted by non-decreasing time. */
void event_list_insert_sorted(EventList* list, Event* event);

/* Remove and return earliest event (list head); NULL if empty. */
Event* event_list_pop_earliest(EventList* list);

/* Print all pending events to stdout (debug / menu option 5). */
void event_list_print(const EventList* list);

/* Human-readable event type name for logging. */
const char* event_type_to_string(EventType type);

#endif /* EVENT_H */
