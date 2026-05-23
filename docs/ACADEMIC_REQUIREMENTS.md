# Academic Requirements — Full Mapping

How this project satisfies typical **Discrete Event Simulation / Advanced C** course requirements.

---

## Requirement 1: Structures (`struct`)

| Struct | File | Purpose |
|--------|------|---------|
| `Elevator` | elevator.h | Cab state |
| `Passenger` | passenger.h | Person + queue node |
| `Floor` | floor.h | Hall + waiting queue |
| `Event` | event.h | FEL node |
| `EventList` | event.h | FEL container |
| `Simulation` | simulation.h | Global model |
| `SimulationConfig` | file_manager.h | Parameters |

**Presentation:** [DATA_STRUCTURES.md](DATA_STRUCTURES.md)

---

## Requirement 2: Strings

| Usage | Location |
|-------|----------|
| Log messages | `logger.c`, handlers |
| `snprintf` formatting | `simulation.c`, `logger.c` |
| Config line parsing | `file_manager.c` |
| Event type names | `event_type_to_string` |

Standard C strings (`char*`, `char[]` buffers), no custom string class.

---

## Requirement 3: Linked lists

| List | Link field | Module |
|------|------------|--------|
| Future Event List | `Event.next` | event.c |
| Floor waiting queue | `Passenger.next` | floor.c |

Operations: insert sorted, pop head, enqueue, dequeue.

**Presentation:** [ALGORITHMS.md](ALGORITHMS.md)

---

## Requirement 4: Dynamic arrays

| Array | Allocation | Freed in |
|-------|------------|----------|
| `Simulation.elevators` | `calloc(numElevators, sizeof(Elevator))` | `simulation_destroy` |
| `Simulation.floors` | `calloc(numFloors, sizeof(Floor))` | `simulation_destroy` |
| `Elevator.floorStops` | per-cab `calloc(numFloors, …)` | `elevator_stops_destroy` / `simulation_destroy` |
| `Simulation.floorDemand` | `calloc(numFloors, sizeof(int))` | `simulation_destroy` |

---

## Requirement 5: File save / load

- **Save:** `config_save` → `config.txt`  
- **Load:** `config_load` ← `config.txt`  
- Menu options 2 and 3  

**Presentation:** [CONFIGURATION.md](CONFIGURATION.md)

---

## Requirement 6: Preprocessor directives

**File:** `constants.h`

Examples:

```c
#define MAX_FLOORS 50
#define LOG_FILE_NAME "simulation_log.txt"
#define DEFAULT_CAPACITY 10
```

Used for limits, defaults, and file paths across modules.

---

## Requirement 7: Modular code

| Module | Coupling |
|--------|----------|
| 8 domain pairs + main | Headers declare API; .c implement |
| No circular includes | passenger ← floor ← simulation |

**Presentation:** [MODULES.md](MODULES.md), [ARCHITECTURE.md](ARCHITECTURE.md)

---

## Requirement 8: Readable functions

- Short handlers per event type  
- Named helpers: `simulation_schedule_event`, `simulation_find_elevator_for_pickup`  
- `=== PRESENTATION ===` blocks in source for demo walkthrough  

---

## Requirement 9: Logs

- Dual sink logging with simulation timestamps  
- Event create/handle tracing  
- Warnings for exceptional paths  

**Presentation:** [LOGGING_AND_DEBUGGING.md](LOGGING_AND_DEBUGGING.md)

---

## DES-specific (course topic)

| Concept | Implementation |
|---------|----------------|
| Future Event List | event.c |
| Event scheduling | simulation_schedule_event |
| Clock advance | simulation_run |
| Event handlers | handle_* functions |

**Presentation:** [DES_THEORY.md](DES_THEORY.md)

---

## Explicitly excluded (by design)

Document these if grader asks “why not X?”:

| Excluded | Reason |
|----------|--------|
| GUI | Out of scope |
| Threads | Out of scope |
| Database | Out of scope |
| Networking | Out of scope |
| C++ | Standard C only |

---

## Evidence checklist for submission

- [ ] Source compiles without errors  
- [ ] `simulation_log.txt` sample attached  
- [ ] `config.txt` sample attached  
- [ ] README + DOCUMENTATION_INDEX in repo  
- [ ] This file referenced in cover slide  

See also [GRADING_MAP.md](GRADING_MAP.md) for file paths.
