# Elevator Management System — Discrete Event Simulation (DES)

[![Course](https://img.shields.io/badge/course-DES-blue)]()
[![Language](https://img.shields.io/badge/language-C99-green)]()
[![Status](https://img.shields.io/badge/status-foundation%20~50%25-yellow)]()

Academic C project simulating a multi-elevator building using **Discrete Event Simulation (DES)**.  
Repository: **https://github.com/GalVitrak/Elevator-Managment-DES**

---

## Table of contents

1. [Overview](#overview)
2. [What is implemented (Phase 1)](#what-is-implemented-phase-1)
3. [What is not implemented (Phase 2)](#what-is-not-implemented-phase-2)
4. [Quick start](#quick-start)
5. [Documentation index](#documentation-index)
6. [Project structure](#project-structure)
7. [Simulation concepts](#simulation-concepts)
8. [Configuration](#configuration)
9. [Logging](#logging)
10. [Git workflow](#git-workflow)
11. [Academic requirements checklist](#academic-requirements-checklist)
12. [Known limitations](#known-limitations)
13. [Team & handoff](#team--handoff)

---

## Overview

### Purpose

Model a simplified **elevator management system** for a multi-floor building:

- Passengers request rides from floor A to floor B.
- Elevators respond to hall calls using a **Future Event List (FEL)**.
- Simulation time advances event-by-event (not in fixed Δt steps).

### Why DES?

In DES, the clock jumps to the next scheduled event. Between events, nothing changes — this matches real elevator systems where state updates occur at arrivals, door openings, etc.

### Current completion: ~50%

| Phase | Owner | Branch (example) | Status |
|-------|--------|------------------|--------|
| Foundation | Phase 1 | `feature/des-foundation-half` | Done |
| Advanced behavior | Phase 2 (teammate) | See `TODO.md` | Planned |

---

## What is implemented (Phase 1)

- Modular C architecture (`*.h` / `*.c` per domain)
- Core **structs** and **enums** (direction, status, doors, events, passengers)
- **Dynamic memory**: elevator/floor arrays, linked-list nodes
- **Future Event List**: sorted linked list, insert by time, pop earliest
- **Floor waiting queues**: enqueue / dequeue / print (FIFO)
- **DES main loop**: pop → advance time → handle → log
- **Event handlers** (basic): passenger call, arrival, doors, exit
- **Dispatch**: assign **first idle** elevator
- **Movement**: **instant** (teleport) — placeholder for realism
- **Logging**: console + `simulation_log.txt` with simulation timestamps
- **Config**: save/load `config.txt`
- **Console menu** with input validation
- **TODO markers** in source for phase 2

---

## What is not implemented (Phase 2)

See **[TODO.md](TODO.md)** for the full teammate roadmap. Summary:

- Realistic travel time between floors
- Advanced dispatch (nearest cab, SCAN, load balancing)
- Capacity / overload handling
- Statistics and utilization reports
- Energy consumption model
- Emergency / maintenance / `OUT_OF_SERVICE` flows
- Multiple passengers per elevator (proper model)
- GUI, networking, database, threads

---

## Quick start

### Prerequisites

- GCC (MinGW) **or** Visual Studio 2022+
- Git

### Clone

```bash
git clone https://github.com/GalVitrak/Elevator-Managment-DES.git
cd Elevator-Managment-DES
git checkout feature/des-foundation-half
```

### Build (GCC)

```bash
make
```

### Run

```bash
./des_elevator          # Linux / macOS / Git Bash
des_elevator.exe        # Windows
```

### First run

1. Choose menu option **1** (Start new simulation).
2. Enter: 5 floors, 2 elevators, capacity 10, max time 100.
3. Add 1+ passenger requests (e.g. floor 0 → 3).
4. Simulation runs automatically; review console and `simulation_log.txt`.
5. Option **5** prints elevators, queues, and pending events.

### Visual Studio

Open `DES-Elevator.slnx`, build **Debug|x64**, run from **repository root** as working directory.

---

## Documentation index

| Document | Audience | Contents |
|----------|----------|----------|
| **[README.md](README.md)** | Everyone | Overview, quick start (this file) |
| **[TODO.md](TODO.md)** | Phase 2 developer | Prioritized tasks, file map, testing checklist |
| **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** | Developers | DES model, data structures, event flows |
| **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)** | Developers | Build, run, debug, PR workflow |
| **[docs/MODULES.md](docs/MODULES.md)** | Developers | Per-file API reference |
| **[config.txt.example](config.txt.example)** | Users | Sample configuration |

---

## Project structure

```text
Elevator-Managment-DES/
├── main.c                 # Entry point, console menu
├── constants.h            # Preprocessor limits and file names
├── simulation.c / .h      # DES engine and event handlers
├── elevator.c / .h        # Elevator state and assignment
├── passenger.c / .h       # Passenger linked-list nodes
├── floor.c / .h           # Per-floor waiting queues
├── event.c / .h           # Future Event List
├── logger.c / .h          # Logging (console + file)
├── file_manager.c / .h    # Configuration I/O
├── Makefile               # GCC build
├── config.txt.example     # Example config
├── README.md              # Project overview
├── TODO.md                # Phase 2 handoff
├── docs/
│   ├── ARCHITECTURE.md
│   ├── DEVELOPMENT.md
│   └── MODULES.md
└── DES-Elevator/          # Visual Studio project files
```

---

## Simulation concepts

### Future Event List (FEL)

All future happenings are `Event` nodes in a linked list sorted by `time`. The simulation always processes the **earliest** event next.

### Event types (phase 1)

| Type | Role |
|------|------|
| `PASSENGER_CALL` | Assign elevator to waiting passenger |
| `ELEVATOR_ARRIVAL` | Cab reached a floor |
| `DOORS_OPEN` | Boarding or alighting |
| `DOORS_CLOSE` | Prepare to move |
| `PASSENGER_EXIT` | Leave cab at destination |

Detailed sequence diagrams: **[docs/ARCHITECTURE.md §4](docs/ARCHITECTURE.md#4-event-types-and-foundation-flow)**.

### Floor numbering

Floors are **0-based**: `0` = ground, `numFloors - 1` = top.

### Dispatch (phase 1)

First elevator that is `IDLE` with doors `CLOSED` (lowest index wins). No distance optimization yet.

---

## Configuration

File: `config.txt` (or copy from `config.txt.example`).

```ini
num_floors=5
num_elevators=2
capacity=10
max_simulation_time=1000.00
```

| Key | Valid range |
|-----|-------------|
| `num_floors` | 2 – 50 |
| `num_elevators` | 1 – 10 |
| `capacity` | 1 – 20 |
| `max_simulation_time` | > 0 |

Menu **2** = load, **3** = save.

---

## Logging

- **Console:** real-time messages during run.
- **File:** `simulation_log.txt` (same format).

Format:

```text
[t=12.50][INFO] Assigning elevator 0 to floor 2
```

Levels: `INFO`, `WARN`, `ERROR`. Timestamps use **simulation time**, not wall clock.

---

## Git workflow

**Rules for this repo:**

1. **Never** push or commit directly to `main`.
2. Always use a feature branch: `feature/des-<topic>`.
3. Build and test locally before commit.
4. Open a **Pull Request** to merge into `main`.

```bash
git checkout -b feature/des-my-feature
# ... edit, make, test ...
git add .
git commit -m "Describe change"
git push -u origin feature/des-my-feature
```

Suggested phase 2 branches (see `TODO.md`):

- `feature/des-realistic-movement`
- `feature/des-dispatch-algorithms`
- `feature/des-statistics-engine`
- `feature/des-emergency-events`

---

## Academic requirements checklist

| Requirement | Where |
|-------------|--------|
| structs | `Elevator`, `Floor`, `Passenger`, `Event`, `Simulation` |
| strings | Logging, `snprintf`, config parsing |
| linked lists | FEL, floor queues |
| dynamic arrays | `calloc` elevators/floors |
| file save/load | `file_manager.c` |
| preprocessor directives | `constants.h` |
| modular code | Separate `.c`/`.h` modules |
| readable functions | Short handlers, named helpers |
| logs | `logger.c` |

---

## Known limitations

1. **Instant movement** — no travel delay between floors.
2. **Single passenger tracking** per elevator (`activePassengersByElevator`).
3. **No statistics** at end of run.
4. **No overload check** when boarding.
5. **Hall call flags** may remain set after service.
6. **Interactive menu only** — not suitable for automated piping without a test harness.
7. Enums `MAINTENANCE` / `OUT_OF_SERVICE` exist but are unused.

These are intentional; phase 2 addresses them per `TODO.md`.

---

## Team & handoff

| Role | Responsibility |
|------|----------------|
| Phase 1 | Foundation, FEL, queues, skeleton handlers, docs |
| Phase 2 | Realism, dispatch, stats, emergencies — start with **`TODO.md`** |

Before coding phase 2:

1. Read `docs/ARCHITECTURE.md`
2. Follow priority list in `TODO.md`
3. Search `TODO` in `*.c` for exact insertion points

---

## License / course

Academic project for a **Discrete Event Simulation** course. Use and extend per your institution's collaboration rules.

---

**Questions?** Open a GitHub issue or contact the repository owner.
