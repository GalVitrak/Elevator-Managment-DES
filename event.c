/*
 * event.c - Future Event List (FEL): sorted linked list of pending DES events
 *
 * PRESENTATION: Classic DES structure — event_list_insert_sorted keeps list
 * ordered by time; event_list_pop_earliest returns the head (next event to process).
 */
#include "event.h"

#include <stdio.h>
#include <stdlib.h>

/* event_list_init - Empty list, size zero. */
void event_list_init(EventList* list)
{
    list->head = NULL;
    list->size = 0;
}

/*
 * event_create - Allocate one event node with given fields.
 * next is NULL; caller inserts via event_list_insert_sorted.
 */
Event* event_create(double time, EventType type, int elevatorId,
                    int passengerId, int floor, int destinationFloor)
{
    Event* event = (Event*)malloc(sizeof(Event));
    if (event == NULL) {
        return NULL;
    }

    event->time = time;
    event->type = type;
    event->elevatorId = elevatorId;
    event->passengerId = passengerId;
    event->floor = floor;
    event->destinationFloor = destinationFloor;
    event->next = NULL;
    return event;
}

/*
 * === PRESENTATION: FEL insert (linked list by time) ===
 * event_list_insert_sorted - Insert so times are non-decreasing from head to tail.
 * Equal times are placed after existing events with the same time (stable-ish order).
 */
void event_list_insert_sorted(EventList* list, Event* event)
{
    Event* current;
    Event* previous;

    if (list == NULL || event == NULL) {
        return;
    }

    if (list->head == NULL || event->time < list->head->time) {
        event->next = list->head;
        list->head = event;
        list->size++;
        return;
    }

    previous = NULL;
    current = list->head;

    while (current != NULL && current->time <= event->time) {
        previous = current;
        current = current->next;
    }

    event->next = current;
    if (previous == NULL) {
        list->head = event;
    } else {
        previous->next = event;
    }

    list->size++;
}

/*
 * event_list_pop_earliest - Remove list head (event with lowest T).
 * DES: each loop iteration takes the minimum-time future event.
 * Caller owns returned pointer and must free() after handling.
 */
Event* event_list_pop_earliest(EventList* list)
{
    Event* earliest;

    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    earliest = list->head;
    list->head = earliest->next;
    earliest->next = NULL;
    list->size--;
    return earliest;
}

/* event_list_destroy - Free all nodes and clear the list. */
void event_list_destroy(EventList* list)
{
    Event* current;
    Event* next;

    if (list == NULL) {
        return;
    }

    current = list->head;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->size = 0;
}

/* event_type_to_string - Name for logging and debug print. */
const char* event_type_to_string(EventType type)
{
    switch (type) {
    case EVENT_PASSENGER_CALL:    return "PASSENGER_CALL";
    case EVENT_ELEVATOR_ARRIVAL: return "ELEVATOR_ARRIVAL";
    case EVENT_DOORS_OPEN:       return "DOORS_OPEN";
    case EVENT_DOORS_CLOSE:      return "DOORS_CLOSE";
    case EVENT_PASSENGER_EXIT:   return "PASSENGER_EXIT";
    default:                     return "UNKNOWN";
    }
}

/* event_list_print - Dump FEL contents to stdout (index, time, type, ids). */
void event_list_print(const EventList* list)
{
    const Event* current;
    int index = 0;

    printf("Future Event List (%d events):\n", list->size);
    for (current = list->head; current != NULL; current = current->next) {
        if (current->type == EVENT_PASSENGER_CALL && current->destinationFloor >= 0) {
            printf("  [%d] t=%.2f %s pass=%d %d->%d\n",
                   index++,
                   current->time,
                   event_type_to_string(current->type),
                   current->passengerId,
                   current->floor,
                   current->destinationFloor);
        } else {
            printf("  [%d] t=%.2f %s elev=%d pass=%d floor=%d\n",
                   index++,
                   current->time,
                   event_type_to_string(current->type),
                   current->elevatorId,
                   current->passengerId,
                   current->floor);
        }
    }
}
