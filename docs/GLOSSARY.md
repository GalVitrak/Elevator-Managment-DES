# Glossary

Alphabetical reference of terms used in this project and presentation.

| Term | Definition |
|------|------------|
| **Arrival event** | `EVENT_ELEVATOR_ARRIVAL` — elevator reached a floor. |
| **Boarding** | Passenger moves from floor queue into elevator (`handle_doors_open`). |
| **Building** | Collection of floors and elevators; size set in config. |
| **C99** | ISO C standard (1999) used for compilation. |
| **Capacity** | Max passengers per elevator (`Elevator.capacity`). |
| **Config** | `SimulationConfig` / `config.txt` — static parameters. |
| **Console menu** | Interactive UI in `main.c` (options 1–6). |
| **DES** | Discrete Event Simulation. |
| **Dispatch** | Choosing which elevator serves a request. |
| **Door state** | `DOOR_OPEN` or `DOOR_CLOSED`. |
| **Dynamic array** | Heap array (`calloc`) for elevators and floors. |
| **Entity** | Passenger or elevator in simulation terms. |
| **Event** | Typed occurrence at a specific simulation time. |
| **Event handler** | Function `handle_*` that updates state. |
| **Event list** | See FEL. |
| **FEL** | Future Event List — pending events sorted by time. |
| **FIFO** | First In First Out — floor waiting queue discipline. |
| **Floor queue** | Linked list of waiting passengers on one floor. |
| **Foundation phase** | Phase 1 (~50%) — engine and skeleton logic. |
| **Future Event List** | Same as FEL. |
| **Ground floor** | Floor index `0`. |
| **Hall call** | Passenger presses up/down — modeled as `PASSENGER_CALL`. |
| **Handler** | See event handler. |
| **Idle** | Elevator available (`ELEVATOR_IDLE`). |
| **Instant movement** | Phase 1: no travel delay between floors. |
| **Linked list** | Chain of nodes via `next` pointers (events, passengers). |
| **Log level** | INFO, WARNING, ERROR. |
| **Passenger** | Person with source, destination, status. |
| **Phase 2** | Second half — realism, stats, dispatch. |
| **Pop** | Remove earliest event from FEL head. |
| **Preprocessor** | `#define` constants in `constants.h`. |
| **Queue** | Waiting passengers on a floor. |
| **Schedule** | Insert new event into FEL (`simulation_schedule_event`). |
| **Simulation clock** | `Simulation.currentTime`. |
| **Simulation time** | Model time, not real-world time. |
| **State** | Snapshot of all elevators, floors, passengers, FEL. |
| **Status (elevator)** | IDLE, MOVING, MAINTENANCE, OUT_OF_SERVICE. |
| **Status (passenger)** | WAITING, IN_ELEVATOR, ARRIVED. |
| **Teleport** | Informal term for instant movement in phase 1. |
| **Time-step simulation** | Alternative to DES — not used here. |
| **TODO marker** | Comment in code for phase 2 work. |
| **Trace log** | `simulation_log.txt` audit trail. |
| **Validation** | Checking menu/config inputs within min/max bounds. |
