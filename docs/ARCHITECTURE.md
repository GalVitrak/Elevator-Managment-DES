# Architecture — Elevator DES

## 1. Discrete Event Simulation (DES) model

This project uses **discrete event simulation**, not fixed time-step simulation.

- **Simulation clock** (`Simulation.currentTime`) jumps forward to the timestamp of the next event.
- **State** (elevator positions, queues, door state) changes only when an event is processed.
- **Future Event List (FEL)** holds all pending events, sorted by `Event.time`.

Core loop (`simulation_run` in `simulation.c`):

```text
while FEL not empty AND currentTime < maxSimulationTime:
    event = pop earliest from FEL
    currentTime = event.time
    dispatch to handler(event)
    free(event)
```

No event is processed “between” scheduled times — that is the defining property of DES.

---

## 2. High-level component diagram

```text
+------------------+     +------------------+
|     main.c       |     |  file_manager    |
|  (console menu)  |---->|  config.txt I/O  |
+--------+---------+     +------------------+
         |
         v
+--------+---------+     +------------------+
|   simulation     |<--->|     logger       |
|  DES + handlers  |     | console + file   |
+--------+---------+     +------------------+
         |
    +----+----+---------+----------+
    v         v         v          v
 elevator   floor    passenger    event
  .c/.h     .c/.h      .c/.h      .c/.h
```

---

## 3. Data structures

### 3.1 Simulation (`simulation.h`)

| Field | Type | Role |
|-------|------|------|
| `currentTime` | `double` | Simulation clock |
| `maxSimulationTime` | `double` | Stop condition |
| `numFloors`, `numElevators` | `int` | Building size |
| `elevatorCapacity` | `int` | Max passengers per cab |
| `elevators` | `Elevator*` | Dynamic array |
| `floors` | `Floor*` | Dynamic array |
| `eventList` | `EventList` | FEL |
| `nextPassengerId` | `int` | Auto-increment IDs |
| `config` | `SimulationConfig` | Copy of loaded settings |
| `activePassengersByElevator` | `Passenger**` | **Foundation only:** one in-cab passenger per elevator |

### 3.2 Elevator (`elevator.h`)

| Field | Description |
|-------|-------------|
| `id` | 0 .. numElevators-1 |
| `currentFloor`, `targetFloor` | Position (0-based floors) |
| `direction` | `DIR_UP`, `DIR_DOWN`, `DIR_NONE` |
| `status` | `ELEVATOR_IDLE`, `MOVING`, `MAINTENANCE`, `OUT_OF_SERVICE` |
| `doorState` | `DOOR_OPEN`, `DOOR_CLOSED` |
| `capacity`, `passengerCount` | Load tracking |

Enums `MAINTENANCE` and `OUT_OF_SERVICE` exist for phase 2.

### 3.3 Floor (`floor.h`)

| Field | Description |
|-------|-------------|
| `floorNumber` | Index 0 .. numFloors-1 |
| `upButtonPressed`, `downButtonPressed` | Hall call flags (set, not fully cleared in phase 1) |
| `waitingQueueFront`, `waitingQueueRear` | FIFO linked list of `Passenger` |

### 3.4 Passenger (`passenger.h`)

Linked-list node: `id`, `sourceFloor`, `destinationFloor`, `requestTime`, `status`, `next`.

Statuses: `PASSENGER_WAITING`, `PASSENGER_IN_ELEVATOR`, `PASSENGER_ARRIVED`.

### 3.5 Event (`event.h`)

| Field | Usage |
|-------|--------|
| `time` | When the event fires |
| `type` | See §4 |
| `elevatorId` | -1 if N/A |
| `passengerId` | -1 if N/A |
| `floor` | Context floor (source, destination, or current) |
| `next` | FEL linked list |

### 3.6 EventList — Future Event List

- **Structure:** singly linked list, sorted ascending by `time`.
- **Insert:** `event_list_insert_sorted` — O(n) scan, simple and correct for coursework scale.
- **Pop:** `event_list_pop_earliest` — removes head.

---

## 4. Event types and foundation flow

### Event types

| Enum | Meaning |
|------|---------|
| `EVENT_PASSENGER_CALL` | Process hall call; assign elevator |
| `EVENT_ELEVATOR_ARRIVAL` | Cab reached a floor |
| `EVENT_DOORS_OPEN` | Doors open; board or exit |
| `EVENT_DOORS_CLOSE` | Doors close; may depart |
| `EVENT_PASSENGER_EXIT` | Passenger leaves cab at destination |

### Typical happy-path (single passenger)

```text
[User / menu] simulation_add_passenger_request
    -> create Passenger, enqueue on source floor
    -> schedule PASSENGER_CALL @ currentTime

PASSENGER_CALL (handle_passenger_call)
    -> find first idle elevator
    -> elevator_assign_to_floor (INSTANT in phase 1)
    -> schedule ELEVATOR_ARRIVAL @ currentTime

ELEVATOR_ARRIVAL
    -> schedule DOORS_OPEN @ currentTime

DOORS_OPEN
    -> dequeue passenger from floor queue
    -> board -> activePassengersByElevator[id]
    -> schedule DOORS_CLOSE @ currentTime + 0.5 (destination in event.floor)

DOORS_CLOSE
    -> instant move to destination (phase 1)
    -> schedule ELEVATOR_ARRIVAL @ destination

ELEVATOR_ARRIVAL (at destination)
    -> DOORS_OPEN

DOORS_OPEN (passenger on board, floor == destination)
    -> schedule PASSENGER_EXIT @ currentTime + 0.2

PASSENGER_EXIT
    -> decrement count, destroy passenger, schedule DOORS_CLOSE
```

Phase 2 should replace **instant** segments with delayed `EVENT_ELEVATOR_ARRIVAL` at computed future times.

---

## 5. Dispatch policy (phase 1)

Function: `elevator_find_first_idle` in `elevator.c`.

- Scans elevators `0 .. n-1` in order.
- Returns first where `status == ELEVATOR_IDLE` and `doorState == DOOR_CLOSED`.
- If none: request **stays in floor queue**; warning logged.

No distance-based or load-balancing logic yet.

---

## 6. Memory management

| Allocation | Freed in |
|------------|----------|
| `elevators`, `floors`, `activePassengersByElevator` | `simulation_destroy` |
| Each `Event` from `event_create` | After handling in `simulation_run`, or `event_list_destroy` |
| Passengers in floor queues | `floor_destroy` |
| Passenger on elevator | `handle_passenger_exit` or `simulation_destroy` |
| Passengers never dequeued | Must not leak — destroy on sim end via floor_destroy |

Always call `simulation_destroy` before exit (`main.c` does on shutdown).

---

## 7. Configuration and logging

- **Config file:** `config.txt` (see `file_manager.c`). Keys: `num_floors`, `num_elevators`, `capacity`, `max_simulation_time`.
- **Log file:** `simulation_log.txt` — mirrors console with `[t=...][LEVEL] message`.
- **Constants:** `constants.h` — limits (`MIN_FLOORS`, `MAX_FLOORS`, etc.).

---

## 8. Extension points (phase 2)

1. **`simulation_schedule_event`** — only place new events should be enqueued (private static in `simulation.c`).
2. **Handlers** — `handle_*` in `simulation.c` — primary behavior changes.
3. **`elevator_assign_to_floor`** — movement model.
4. **New module `statistics.c`** — recommended instead of bloating `simulation.c`.

---

## 9. Academic requirements mapping

| Requirement | Implementation |
|-------------|----------------|
| structs | `Elevator`, `Floor`, `Passenger`, `Event`, `Simulation` |
| strings | `snprintf`, log messages, config parsing |
| linked lists | FEL, floor queues, passenger `next` |
| dynamic arrays | `calloc` for elevators/floors |
| file save/load | `file_manager.c` |
| preprocessor | `constants.h` |
| modular code | One pair .h/.c per domain |
| logs | `logger.c` |

---

## 10. Known architectural limitations

Documented intentionally for phase 2:

1. **One passenger per elevator** in `activePassengersByElevator`.
2. **Instant travel** — breaks realism but keeps FEL logic testable early.
3. **Hall buttons** set but not systematically cleared.
4. **`event.floor` overload** — used for source, destination, or current context depending on event type; phase 2 may add `destinationFloor` to `Event` if needed.

See `TODO.md` for the remediation plan.
