# Data Structures — Complete Reference

Every struct and enum in the project, field by field, for presentations and code review.

---

## 1. Enumerations

### 1.1 `Direction` (`elevator.h`)

| Value | Meaning |
|-------|---------|
| `DIR_UP` | Cab traveling or assigned upward |
| `DIR_DOWN` | Cab traveling or assigned downward |
| `DIR_NONE` | No movement direction |

### 1.2 `ElevatorStatus` (`elevator.h`)

| Value | Phase 1 usage |
|-------|----------------|
| `ELEVATOR_IDLE` | Available at floor |
| `ELEVATOR_MOVING` | Assigned / traveling |
| `ELEVATOR_MAINTENANCE` | Reserved — phase 2 |
| `ELEVATOR_OUT_OF_SERVICE` | Reserved — phase 2 |

### 1.3 `DoorState` (`elevator.h`)

| Value | Meaning |
|-------|---------|
| `DOOR_OPEN` | Passengers may board/alight |
| `DOOR_CLOSED` | Cab may move |

### 1.4 `PassengerStatus` (`passenger.h`)

| Value | Meaning |
|-------|---------|
| `PASSENGER_WAITING` | In floor queue |
| `PASSENGER_IN_ELEVATOR` | On cab |
| `PASSENGER_ARRIVED` | Completed trip (briefly before destroy) |

### 1.5 `EventType` (`event.h`)

| Value | Handler |
|-------|---------|
| `EVENT_PASSENGER_CALL` | `handle_passenger_call` |
| `EVENT_ELEVATOR_ARRIVAL` | `handle_elevator_arrival` |
| `EVENT_DOORS_OPEN` | `handle_doors_open` |
| `EVENT_DOORS_CLOSE` | `handle_doors_close` |
| `EVENT_PASSENGER_EXIT` | `handle_passenger_exit` |

### 1.6 `LogLevel` (`logger.h`)

`LOG_INFO`, `LOG_WARNING`, `LOG_ERROR`

---

## 2. Struct `Elevator`

**File:** `elevator.h`  
**Storage:** dynamic array `Simulation.elevators`

| Field | Type | Description |
|-------|------|-------------|
| `id` | `int` | Unique index 0..n−1 |
| `currentFloor` | `int` | Present floor (0-based) |
| `targetFloor` | `int` | Intended floor |
| `direction` | `Direction` | UP / DOWN / NONE |
| `status` | `ElevatorStatus` | Operational state |
| `doorState` | `DoorState` | Open or closed |
| `capacity` | `int` | Max passengers |
| `passengerCount` | `int` | Current load |

**Invariants (phase 2):** `0 <= passengerCount <= capacity`

---

## 3. Struct `Passenger`

**File:** `passenger.h`  
**Storage:** floor queue nodes OR `activePassengersByElevator`

| Field | Type | Description |
|-------|------|-------------|
| `id` | `int` | Unique passenger id |
| `sourceFloor` | `int` | Where request started |
| `destinationFloor` | `int` | Target floor |
| `requestTime` | `double` | Time of request |
| `status` | `PassengerStatus` | Lifecycle state |
| `next` | `Passenger*` | Next in floor queue |

**Allocation:** `passenger_create()` → `malloc`  
**Free:** `passenger_destroy()` or `floor_destroy`

---

## 4. Struct `Floor`

**File:** `floor.h`  
**Storage:** dynamic array `Simulation.floors`

| Field | Type | Description |
|-------|------|-------------|
| `floorNumber` | `int` | Index (same as array index) |
| `upButtonPressed` | `int` | 0/1 flag |
| `downButtonPressed` | `int` | 0/1 flag |
| `waitingQueueFront` | `Passenger*` | Head of FIFO |
| `waitingQueueRear` | `Passenger*` | Tail of FIFO |

**Queue operations:** O(1) enqueue/dequeue via front/rear pointers.

---

## 5. Struct `Event`

**File:** `event.h`  
**Storage:** nodes in FEL linked list

| Field | Type | Description |
|-------|------|-------------|
| `time` | `double` | When event fires |
| `type` | `EventType` | Kind of event |
| `elevatorId` | `int` | Related cab, or −1 |
| `passengerId` | `int` | Related passenger, or −1 |
| `floor` | `int` | Context floor (see below) |
| `next` | `Event*` | Next in FEL |

### `floor` field semantics (overload)

| Event type | Typical meaning of `floor` |
|------------|---------------------------|
| PASSENGER_CALL | Source / call floor |
| ELEVATOR_ARRIVAL | Floor reached |
| DOORS_OPEN | Floor where doors open |
| DOORS_CLOSE | Often **destination** when moving |
| PASSENGER_EXIT | Floor where passenger leaves |

Phase 2 may add explicit `destinationFloor` if needed.

---

## 6. Struct `EventList`

| Field | Type | Description |
|-------|------|-------------|
| `head` | `Event*` | Earliest event |
| `size` | `int` | Node count |

---

## 7. Struct `Simulation`

**File:** `simulation.h` — central state container

| Field | Type | Description |
|-------|------|-------------|
| `currentTime` | `double` | Simulation clock |
| `maxSimulationTime` | `double` | Stop horizon |
| `numFloors` | `int` | Building height count |
| `numElevators` | `int` | Cab count |
| `elevatorCapacity` | `int` | Per-cab capacity |
| `elevators` | `Elevator*` | Dynamic array |
| `floors` | `Floor*` | Dynamic array |
| `eventList` | `EventList` | FEL |
| `nextPassengerId` | `int` | ID generator |
| `config` | `SimulationConfig` | Saved settings |
| `activePassengersByElevator` | `Passenger**` | In-cab tracking |

---

## 8. Struct `SimulationConfig`

**File:** `file_manager.h`

| Field | Type | Config key |
|-------|------|------------|
| `numFloors` | `int` | `num_floors` |
| `numElevators` | `int` | `num_elevators` |
| `capacity` | `int` | `capacity` |
| `maxSimulationTime` | `double` | `max_simulation_time` |

---

## 9. Memory layout diagram

```text
Simulation
├── elevators[]     ──► [Elevator][Elevator]...
├── floors[]        ──► [Floor]──► Passenger ↔ Passenger ↔ ...
├── eventList.head  ──► Event ↔ Event ↔ Event (sorted by time)
└── activePassengersByElevator[] ──► Passenger* (or NULL)
```

---

## 10. Presentation talking points

1. **Three linked lists:** FEL, each floor queue, passenger `next` chains.  
2. **Two dynamic arrays:** elevators, floors.  
3. **One pointer array:** active passengers per elevator.  
4. All satisfy typical “data structures course” checklist.

See also [ALGORITHMS.md](ALGORITHMS.md) for operations on these structures.
