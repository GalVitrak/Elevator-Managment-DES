# Elevator Management System (DES)

Academic C project for a **Discrete Event Simulation (DES)** course. This repository simulates a simplified elevator management system using a future event list, modular architecture, and console-driven workflow.

## Project Purpose

Model elevators, floors, passengers, and timed events to study how an elevator system behaves under passenger requests. The simulation advances time by processing the earliest scheduled event from a **Future Event List (FEL)**.

## What This Phase Implements (~50%)

Foundation systems only:

- Modular project layout with headers and source files
- Core structs and enums (`Direction`, `ElevatorStatus`, `DoorState`, `PassengerStatus`, `EventType`)
- Dynamic allocation for elevators, floors, and linked-list nodes
- Sorted Future Event List (insert by time, pop earliest)
- Per-floor passenger waiting queues (enqueue / dequeue / print)
- Basic DES simulation loop (pop → advance time → handle → log)
- Skeleton event handlers with simple “instant movement” behavior
- First-idle elevator assignment
- Logging to console and `simulation_log.txt`
- Config save/load (`config.txt`)
- Console menu with input validation
- Explicit `TODO` markers for advanced features

## Intentionally Left Unfinished (Second Half)

The following are **not** implemented yet and are marked with `TODO` in code:

- Realistic elevator movement (travel time per floor, acceleration)
- Advanced dispatch / smart scheduling algorithms
- Overload detection
- Statistics engine (wait times, throughput)
- Energy consumption tracking
- Emergency events and maintenance flows
- Full passenger lifecycle timing
- Utilization and performance analytics reports
- GUI, networking, database, threads

## Project Structure

```
main.c              Entry point and console menu
constants.h         Preprocessor limits and default file names
simulation.h/.c     DES loop and event handlers
elevator.h/.c       Elevator model and simple assignment
passenger.h/.c      Passenger nodes
floor.h/.c          Floor queues
event.h/.c          Future Event List
logger.h/.c         Console + file logging
file_manager.h/.c   Config save/load
README.md           This file
Makefile            GCC build (optional)
DES-Elevator/       Visual Studio project files
```

## Compile Instructions

### GCC (recommended for CI / cross-platform)

```bash
make
```

Or manually:

```bash
gcc -Wall -Wextra -std=c99 -pedantic -o des_elevator main.c simulation.c elevator.c passenger.c floor.c event.c logger.c file_manager.c
```

### Microsoft Visual Studio

Open `DES-Elevator.slnx` (or the `.vcxproj` under `DES-Elevator/`), ensure all root `.c` files are included in the project, and build as a **Console Application** using the C compiler.

## Run Instructions

```bash
./des_elevator        # Linux / macOS / Git Bash
des_elevator.exe      # Windows
```

### Menu Options

1. **Start new simulation** – configure floors/elevators, seed requests, run DES
2. **Load configuration** – read `config.txt`
3. **Save configuration** – write `config.txt`
4. **Add passenger request** – enqueue a manual request (before or after init)
5. **Print system state** – elevators, floor queues, FEL
6. **Exit**

### Example `config.txt`

```
num_floors=5
num_elevators=2
capacity=10
max_simulation_time=1000.00
```

## Git / Branch Workflow

Work on feature branches only (never commit directly to `main`):

```bash
git checkout feature/des-foundation-half
```

Suggested follow-up branches:

- `feature/des-realistic-movement`
- `feature/des-dispatch-algorithms`
- `feature/des-statistics-engine`
- `feature/des-emergency-events`

## Known Limitations

- Elevators “teleport” to floors (no travel delay yet)
- Only the first idle elevator is assigned
- One tracked passenger per elevator in the foundation layer
- No statistics, energy metrics, or emergency handling
- Menu-driven only (no GUI)

## Authors / Course

Discrete Event Simulation – foundation milestone for team-based completion.
