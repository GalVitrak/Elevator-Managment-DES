#ifndef EVENT_H
#define EVENT_H

typedef enum {
    EVENT_PASSENGER_CALL,
    EVENT_ELEVATOR_ARRIVAL,
    EVENT_DOORS_OPEN,
    EVENT_DOORS_CLOSE,
    EVENT_PASSENGER_EXIT
} EventType;

typedef struct Event {
    double time;
    EventType type;
    int elevatorId;
    int passengerId;
    int floor;
    struct Event* next;
} Event;

typedef struct {
    Event* head;
    int size;
} EventList;

void event_list_init(EventList* list);
void event_list_destroy(EventList* list);
Event* event_create(double time, EventType type, int elevatorId,
                    int passengerId, int floor);
void event_list_insert_sorted(EventList* list, Event* event);
Event* event_list_pop_earliest(EventList* list);
void event_list_print(const EventList* list);
const char* event_type_to_string(EventType type);

#endif /* EVENT_H */
