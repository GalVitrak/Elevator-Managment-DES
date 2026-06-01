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
| `stats` | `SimulationStats` | Metrics + trip records for results file |
| `floorDemand` | `int*` | Per-floor call counts (idle reposition) |
| `numZones` | `int` | Zone-biased dispatch bands |
| `buildingView` | `BuildingGrid` | 2D elevators × floors display |

### 3.2 Elevator (`elevator.h`)

| Field | Description |
|-------|-------------|
| `id` | 0 .. numElevators-1 |
| `currentFloor`, `targetFloor` | Position (0-based floors) |
| `direction` | `DIR_UP`, `DIR_DOWN`, `DIR_NONE` |
| `status` | `ELEVATOR_IDLE`, `MOVING`, `MAINTENANCE`, `OUT_OF_SERVICE` |
| `doorState` | `DOOR_OPEN`, `DOOR_CLOSED` |
| `capacity`, `passengerCount` | Load tracking |
| `onboardHead` | Linked list of passengers in cab |
| `floorStops[]` | Per-floor stop count (SCAN mask) |

Enums `MAINTENANCE` and `OUT_OF_SERVICE` exist for optional emergency extension.

### 3.3 Floor (`floor.h`)

| Field | Description |
|-------|-------------|
| `floorNumber` | Index 0 .. numFloors-1 |
| `upButtonPressed`, `downButtonPressed` | Hall call flags (cleared when queue empty) |
| `waitingQueueFront`, `waitingQueueRear` | FIFO linked list of `Passenger` |

### 3.4 Passenger (`passenger.h`)

Linked-list node: `id`, `sourceFloor`, `destinationFloor`, `requestTime`, `boardTime`, `status`, `assignedElevatorId`, `next`, `onboardNext`.

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

## 4. Event types and flow

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
[Seed / menu] schedule PASSENGER_CALL @ arrivalTime
    -> handle_passenger_call: enqueue, simulation_service_waiting_queues

Dispatch assigns cab (ETA / batch / cluster)
    -> assignedElevatorId set; stop added at source floor
    -> simulation_schedule_elevator_travel when cab departs

ELEVATOR_ARRIVAL @ pickup floor
    -> DOORS_OPEN -> board assigned waiters, alight at destination
    -> DOORS_CLOSE -> pick next stop (pickups first by wait)

ELEVATOR_ARRIVAL @ destination
    -> DOORS_OPEN -> alight passengers; destroy if trip complete
    -> continue until FEL empty or maxSimulationTime
```

Travel uses `simulation_schedule_elevator_travel` (non-zero delay). Alighting is in `handle_doors_open`; `EVENT_PASSENGER_EXIT` is reserved.

---

## 5. Dispatch policy (current)

| Step | Function / area |
|------|-----------------|
| Global greedy match | `simulation_batch_dispatch_round` |
| Per-floor clusters | `simulation_dispatch_floor_clustered` |
| Cab choice | `simulation_find_elevator_for_pickup` (ETA + load − wait) |
| On-the-way | `elevator_will_serve_call` if fleet &lt; 30 |
| Routing | `simulation_pick_next_stop_floor` — assigned pickups before SCAN stops |
| SLA nudge | `simulation_enforce_queue_wait_policy` |

If no cab available, passenger remains queued until a later dispatch pass.

---

## 6. Memory management

| Allocation | Freed in |
|------------|----------|
| `elevators`, `floors`, `floorDemand` | `simulation_destroy` |
| Each `Event` from `event_create` | After handling in `simulation_run`, or `event_list_destroy` |
| Passengers in floor queues | `floor_destroy` |
| Passengers onboard | `handle_doors_open` alight + destroy, or `simulation_destroy` |
| Trip records | `statistics_destroy` |
| Passengers never dequeued | Must not leak — destroy on sim end via floor_destroy |

Always call `simulation_destroy` before exit (`main.c` does on shutdown).

---

## 7. Configuration and logging

- **Config file:** `config.txt` (see `file_manager.c`). Keys: `num_floors`, `num_elevators`, `capacity`, `max_simulation_time`.
- **Log file:** `simulation_log.txt` — mirrors console with `[t=...][LEVEL] message`.
- **Constants:** `constants.h` — limits (`MIN_FLOORS`, `MAX_FLOORS`, etc.).

---

## 8. Extension points (optional)

1. **`simulation_schedule_event`** — central scheduling for new events (`simulation.c`).
2. **Handlers** — `handle_*` in `simulation.c`.
3. **`statistics.c`** — add energy metrics, export formats.
4. **Emergency** — new `EventType`s; skip out-of-service cabs in dispatch.

---

## 9. Academic requirements mapping

| Requirement | Implementation |
|-------------|----------------|
| structs | `Elevator`, `Floor`, `Passenger`, `Event`, `Simulation` |
| strings | `snprintf`, log messages, config parsing |
| linked lists | FEL, floor queues, passenger `next` |
| dynamic arrays | `calloc` elevators/floors; `realloc` trip records |
| 2D matrix | `building_grid.c` |
| sort | `qsort` trip table in `statistics.c` |
| file save/load | `file_manager.c` |
| preprocessor | `constants.h` |
| modular code | One pair .h/.c per domain |
| logs | `logger.c` |

---

## 10. Known architectural limitations

1. **No mid-flight retarget** — moving cab cannot cancel an in-flight `ELEVATOR_ARRIVAL` for SLA recovery.
2. **Overload** — too many requests for too few elevators → long waits or unfinished queue at horizon (capacity issue).
3. **`event.floor` context** — meaning depends on event type (documented in `EVENT_CATALOG.md`).
4. **Energy / emergency** — not modeled.

See `TODO.md` for optional extensions.
