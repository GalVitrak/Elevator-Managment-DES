#include "event.h"

#include <stdio.h>
#include <stdlib.h>

void event_list_init(EventList* list)
{
    list->head = NULL;
    list->size = 0;
}

Event* event_create(double time, EventType type, int elevatorId,
                    int passengerId, int floor)
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
    event->next = NULL;
    return event;
}

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

void event_list_print(const EventList* list)
{
    const Event* current;
    int index = 0;

    printf("Future Event List (%d events):\n", list->size);
    for (current = list->head; current != NULL; current = current->next) {
        printf("  [%d] t=%.2f %s elev=%d pass=%d floor=%d\n",
               index++,
               current->time,
               event_type_to_string(current->type),
               current->elevatorId,
               current->passengerId,
               current->floor);
    }
}
