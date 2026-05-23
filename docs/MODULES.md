# Module Reference

Quick API reference for each translation unit. Implementations are in the matching `.c` file.

---

## `constants.h`

Preprocessor configuration (no `.c` file).

| Macro | Default | Purpose |
|-------|---------|---------|
| `MAX_NAME_LEN` | 64 | Buffer sizing |
| `DEFAULT_MAX_TIME` | 1000.0 | Sim horizon |
| `DEFAULT_CAPACITY` | 10 | Passengers per elevator |
| `LOG_FILE_NAME` | `simulation_log.txt` | Log path |
| `CONFIG_FILE_NAME` | `config.txt` | Config path |
| `MIN_FLOORS` / `MAX_FLOORS` | 2 / 50 | Validation |
| `MIN_ELEVATORS` / `MAX_ELEVATORS` | 1 / 100 | Validation |
| `MIN_CAPACITY` / `MAX_CAPACITY` | 1 / 20 | Validation |
| `DEFAULT_NUM_FLOORS` | 5 | Interactive defaults |
| `DEFAULT_NUM_ELEVATORS` | 2 | Interactive defaults |

---

## `logger.h` / `logger.c`

| Function | Description |
|----------|-------------|
| `logger_init()` | Open log file (truncate/write) |
| `logger_close()` | Close log file |
| `log_message(simTime, level, msg)` | Console + file |
| `log_event_created(simTime, desc)` | Wrapper for scheduling |
| `log_event_handled(simTime, desc)` | Wrapper for dispatch |

Levels: `LOG_INFO`, `LOG_WARNING`, `LOG_ERROR`.

---

## `file_manager.h` / `file_manager.c`

### `SimulationConfig`

```c
int numFloors, numElevators, capacity;
double maxSimulationTime;
```

| Function | Returns |
|----------|---------|
| `config_set_defaults(config)` | void |
| `config_validate(config)` | 1 valid, 0 invalid |
| `config_save(config, filename)` | 1 success |
| `config_load(config, filename)` | 1 success |

---

## `passenger.h` / `passenger.c`

| Function | Description |
|----------|-------------|
| `passenger_create(id, source, dest, time)` | `malloc` new node |
| `passenger_destroy(p)` | `free` node |
| `passenger_print(p)` | Debug print |

---

## `floor.h` / `floor.c`

| Function | Description |
|----------|-------------|
| `floor_init(floor, number)` | Empty queue |
| `floor_destroy(floor)` | Free all queued passengers |
| `floor_enqueue_passenger(floor, p)` | Tail insert O(1) |
| `floor_dequeue_passenger(floor)` | Head remove O(1) |
| `floor_print_queue(floor)` | Debug |
| `floor_queue_size(floor)` | Count nodes |

---

## `elevator.h` / `elevator.c`

| Function | Description |
|----------|-------------|
| `elevator_init(e, id, capacity)` | Idle at floor 0 |
| `elevator_find_first_idle(array, count)` | Index or -1 |
| `elevator_assign_to_floor(e, floor)` | Set MOVING + target; position updates on arrival |
| `elevator_will_serve_call` | On-the-way pickup eligibility (direction + not passed floor) |
| `elevator_print(e)` | Debug |

---

## `event.h` / `event.c`

| Function | Description |
|----------|-------------|
| `event_list_init(list)` | Empty FEL |
| `event_list_destroy(list)` | Free all events |
| `event_create(...)` | `malloc` one event |
| `event_list_insert_sorted(list, e)` | Insert by time |
| `event_list_pop_earliest(list)` | Remove head |
| `event_list_print(list)` | Debug |
| `event_type_to_string(type)` | For logging |

---

## `simulation.h` / `simulation.c`

| Function | Description |
|----------|-------------|
| `simulation_init(sim, config)` | Allocate building + FEL |
| `simulation_destroy(sim)` | Free everything |
| `simulation_reset(sim)` | Destroy + re-init same config |
| `simulation_run(sim)` | **Main DES loop** |
| `simulation_add_passenger_request(sim, src, dst)` | Queue + schedule call |
| `simulation_print_state(sim)` | Full debug dump |
| `simulation_validate_floor(sim, floor)` | Bounds check |

### Event handlers (called from dispatch)

| Handler | Trigger |
|---------|---------|
| `handle_passenger_call` | `EVENT_PASSENGER_CALL` |
| `handle_elevator_arrival` | `EVENT_ELEVATOR_ARRIVAL` |
| `handle_doors_open` | `EVENT_DOORS_OPEN` |
| `handle_doors_close` | `EVENT_DOORS_CLOSE` |
| `handle_passenger_exit` | `EVENT_PASSENGER_EXIT` |

---

## `main.c`

No public API. Contains menu loop and input validation helpers:

- `read_int_in_range(prompt, min, max, &out)`
- `configure_interactively`, `start_simulation_interactive`, `add_passenger_interactive`

---

## Dependency graph (includes)

```text
main.c → simulation.h, logger.h, file_manager.h, constants.h

simulation.c → simulation.h, constants.h, logger.h
             → (via headers) elevator, floor, event, file_manager

floor.c → floor.h → passenger.h
event.c → event.h
elevator.c → elevator.h, logger.h
passenger.c → passenger.h
file_manager.c → file_manager.h, constants.h, logger.h
logger.c → logger.h, constants.h
```

Keep includes minimal; avoid circular dependencies.
