# Elevator Management System — Discrete Event Simulation (DES)

[![Course](https://img.shields.io/badge/course-DES-blue)]()
[![Language](https://img.shields.io/badge/language-C99-green)]()
[![Status](https://img.shields.io/badge/status-advanced%20DES-green)]()

Academic C project simulating a multi-elevator building using **Discrete Event Simulation (DES)**.  
Repository: **https://github.com/GalVitrak/Elevator-Managment-DES**

---

## Table of contents

1. [Overview](#overview)
2. [What is implemented](#what-is-implemented)
3. [What is not implemented](#what-is-not-implemented)
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

### Current completion: advanced DES (ride-sharing + wait-priority dispatch)

| Area | Status |
|------|--------|
| DES loop, FEL, config, seeds, statistics | Done |
| Travel time, doors, multi-passenger cabs | Done |
| Destination clustering + **wait-priority** dispatch | Done |
| **SCAN floor stops**; pickups served before other stops | Done |
| **3-minute queue-wait target** (`MAX_QUEUE_WAIT_SECONDS` = 180 s) | Done |
| **Building grid** (elevators × floors matrix) | Done |
| Underground floors + display floor mapping | Done |
| Energy / emergency events | Not implemented |

**Presenting in class?** Start with **[docs/HOW_TO_PRESENT.md](docs/HOW_TO_PRESENT.md)** (slides, code snippets, demo script). Timing: [docs/PRESENTATION_GUIDE.md](docs/PRESENTATION_GUIDE.md).

**Reading the code?** Module headers and `/* === presentation: ... === */` blocks in `*.c` mark what to show in class; functions have short purpose comments.

**DES course concept (Hebrew):** [docs/DES_COURSE_HE.md](docs/DES_COURSE_HE.md) — clock jumps event-to-event (no real-time wait); loop picks lowest `T` from the Future Event List (`simulation_run` + `event_list_pop_earliest`).

---

## What is implemented

- Modular C architecture (`*.h` / `*.c` per domain)
- **DES** with sorted **Future Event List** and event-by-event clock
- **Realistic timing**: 1 s/floor, door open/dwell/close (`constants.h`)
- **Ride-sharing**: cluster nearby destinations (span ≤ `DEST_CLUSTER_MAX_SPAN_FLOORS`), fill cabs to capacity
- **SCAN-style stops**: per-floor **stop mask**; prefer longest-wait floors when choosing the next stop
- **Optimized dispatch** (`simulation.c`):
  - **ETA + load scoring** — pick the cab that can reach the call soonest, not just the nearest floor index
  - **Batch greedy matching** — each round assigns the best (passenger, elevator) pair globally
  - **Controlled on-the-way pickup** — moving cabs accept calls only if `ETA + wait ≤ 120 s`
  - **Dynamic clustering** — destination span 10 / 5 / 3 floors by queue wait (loose → tight near SLA)
  - **Zone bias** (large buildings) — soft penalty when a cab serves far outside its floor band
  - **Idle reposition** — empty cabs drift toward high-demand floors
  - Assigned pickups are visited **before** other stops; **SLA** target **180 s** (`MAX_QUEUE_WAIT_SECONDS`)
- **Random seed**: requests spread evenly across the simulation horizon (no arrival-time pile-up at `max_time`)
- **Dynamic 2D matrix**: `BuildingGrid` = elevators × floors (ASCII view in menu option **5**)
- **Statistics** + tabular `simulation_results.txt` and `simulation_log.txt` (per-passenger trip table, SLA violations)
- **Config** + **random seed** save/load (menu **6** configure + generate seed, **7** load seed and run)
- **Interactive setup**: invalid menu input is re-prompted (option **6** shows allowed ranges)
- **Door logs** include elevator id and onboard passenger list

---

## What is not implemented

- Energy model
- Emergency / maintenance / `OUT_OF_SERVICE` flows
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
git checkout main
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

**Small manual demo**

1. Menu **1** — start simulation; enter e.g. 5 floors above ground, 2 elevators, capacity 10, max time 100.
2. Add passenger requests (e.g. floor 0 → 3).
3. Review console, `simulation_log.txt`, and `simulation_results.txt`.
4. Menu **4** — print elevators, queues, building grid, and pending events.

**Large random scenario (recommended)**

1. Menu **5** — configure building (up to **150** floors above ground, **20** underground, **100** elevators, **2000** requests, etc.) and generate `random_seed.txt`.
2. Menu **6** — load seed and run DES; check `simulation_results.txt` for max/average queue wait and SLA line.

### Visual Studio

Open `DES-Elevator.slnx`, build **Debug|x64**, run from **repository root** as working directory.

---

## Documentation (20+ guides)

**Master index:** **[DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)** — complete list of every document.

### For your presentation (start here)

| Document | Use for |
|----------|---------|
| **[docs/HOW_TO_PRESENT.md](docs/HOW_TO_PRESENT.md)** | **Main guide:** slide outline, code snippets to show, demo, Q&A |
| **[docs/PRESENTATION_GUIDE.md](docs/PRESENTATION_GUIDE.md)** | Timing and what to emphasize |
| **[docs/PRESENTATION_SLIDES.md](docs/PRESENTATION_SLIDES.md)** | 27 ready-made slides (copy to PowerPoint) |
| **[docs/DEMO_SCRIPT.md](docs/DEMO_SCRIPT.md)** | Minute-by-minute live demo |
| **[docs/DES_THEORY.md](docs/DES_THEORY.md)** | DES / FEL theory for oral exam |
| **[docs/PROJECT_OVERVIEW_VISUAL.md](docs/PROJECT_OVERVIEW_VISUAL.md)** | Mermaid diagrams for slides |
| **[docs/FAQ.md](docs/FAQ.md)** | Q&A preparation |
| **[docs/SAMPLE_RUNS.md](docs/SAMPLE_RUNS.md)** | Expected log output (demo backup) |

### For users and graders

| Document | Use for |
|----------|---------|
| **[docs/USER_GUIDE.md](docs/USER_GUIDE.md)** | Full menu / operator manual |
| **[docs/GRADING_MAP.md](docs/GRADING_MAP.md)** | Where each requirement lives in code |
| **[docs/ACADEMIC_REQUIREMENTS.md](docs/ACADEMIC_REQUIREMENTS.md)** | Course requirements mapping |

### Technical deep dives

| Document | Use for |
|----------|---------|
| **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** | System design, DES loop |
| **[docs/DATA_STRUCTURES.md](docs/DATA_STRUCTURES.md)** | Every struct & enum |
| **[docs/EVENT_CATALOG.md](docs/EVENT_CATALOG.md)** | All event types & flows |
| **[docs/ALGORITHMS.md](docs/ALGORITHMS.md)** | FEL, queues, dispatch complexity |
| **[docs/MODULES.md](docs/MODULES.md)** | API per file |
| **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)** | Build, Git, PR workflow |
| **[docs/LOGGING_AND_DEBUGGING.md](docs/LOGGING_AND_DEBUGGING.md)** | Logs & debugging |
| **[docs/CONFIGURATION.md](docs/CONFIGURATION.md)** | `config.txt` reference |
| **[docs/GLOSSARY.md](docs/GLOSSARY.md)** | Terminology |
| **[docs/PHASE1_PHASE2.md](docs/PHASE1_PHASE2.md)** | What is done vs planned |

### Project management

| Document | Use for |
|----------|---------|
| **[TODO.md](TODO.md)** | Phase 2 tasks for teammate |
| **[docs/TEAM_AND_CONTRIBUTIONS.md](docs/TEAM_AND_CONTRIBUTIONS.md)** | Roles template |
| **[config.txt.example](config.txt.example)** | Sample configuration |

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
├── TODO.md                # Optional extensions + presentation checklist
├── docs/
│   ├── HOW_TO_PRESENT.md  # Slides, code snippets, demo (start here)
│   ├── ARCHITECTURE.md
│   ├── PRESENTATION_GUIDE.md
│   └── ...
└── DES-Elevator/          # Visual Studio project files
```

---

## Simulation concepts

### Future Event List (FEL)

All future happenings are `Event` nodes in a linked list sorted by `time`. The simulation always processes the **earliest** event next.

### Event types

| Type | Role |
|------|------|
| `PASSENGER_CALL` | Passenger arrives; batch dispatch to idle cabs |
| `ELEVATOR_ARRIVAL` | Cab reached a floor after travel time |
| `DOORS_OPEN` | Alight at destination, board assigned waiters |
| `DOORS_CLOSE` | Pick next stop (pickups prioritized by wait), schedule travel |
| `PASSENGER_EXIT` | Reserved; alighting handled in `DOORS_OPEN` |

Detailed sequence diagrams: **[docs/ARCHITECTURE.md §4](docs/ARCHITECTURE.md#4-event-types-and-foundation-flow)**.

### Floor numbering

- **Display floors**: ground = **0**, above-ground up to **+150**, basements **-1 … -20** (when configured).
- Internally, floors are stored as a contiguous index array (`config_index_to_display` / `config_display_to_index`).

### Dispatch and queue-wait SLA

1. Passengers wait in per-floor FIFO queues until a cab collects them.
2. **Dispatch** picks the floor with the longest queue wait, clusters destinations, and assigns to the **nearest idle** cab with free capacity.
3. Each moving cab visits **assigned pickup floors** (longest wait first) before other stops.
4. **SLA**: design target is **≤ 180 s** from call until boarding (`MAX_QUEUE_WAIT_SECONDS` in `constants.h`). The results file lists how many passengers boarded after exceeding that limit.

Under very high load (many requests, few elevators), some SLA violations may still occur; use menu **6** limits and request count to match your building scenario.

---

## Configuration

File: `config.txt` (or copy from `config.txt.example`). Interactive limits are in `constants.h`; menu **6** prints them before setup.

| Setting (menu **6**) | Valid range |
|----------------------|-------------|
| Floors above ground | 0 – 150 (display **0 … 150**) |
| Underground floors | 0 – 20 (display **-1 … -N**) |
| Elevators | 1 – 100 |
| Capacity per cab | 1 – 20 |
| Max simulation time | > 0 (seconds) |
| Random requests per seed | 0 – 2000 |

| Key / constant | Meaning |
|----------------|---------|
| `MAX_QUEUE_WAIT_SECONDS` | Target max queue wait (default **180** s) |
| `DEST_CLUSTER_MAX_SPAN_FLOORS` | Max destination spread in one cab cluster |
| `SECONDS_PER_FLOOR` | Travel time per floor (default **1** s) |

Menu **1** = start manual simulation setup, **2** = add manual request, **3** = run current simulation, **5** = configure + write `random_seed.txt`, **6** = load seed and run.

---

## Logging

- **Console:** real-time messages during run.
- **File:** `simulation_log.txt` (same format).

Format:

```text
[t=12.50][INFO] Assigning elevator 0 to floor 2
```

Levels: `INFO`, `WARN`, `ERROR`. Timestamps use **simulation time**, not wall clock.

### Results summary file

After every run (`simulation_run`), a **bank-style report** is written to:

- **`simulation_results.txt`** — full summary (overwrite each run)
- **`simulation_log.txt`** — same summary **appended** at the end

The console prints: `>>> Simulation summary saved to: simulation_results.txt`

Key result lines:

- **Maximum queue wait** — longest time from call to boarding (aim for ≤ 180 s under normal load).
- **Queue waits over SLA** — passengers who boarded after waiting more than `MAX_QUEUE_WAIT_SECONDS`.
- **Per-passenger table** — source/destination (display floors), queue time, travel time, total trip.

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
| linked lists | FEL, floor queues, onboard passengers |
| dynamic arrays | `calloc` elevators/floors, `realloc` trip records |
| **2D dynamic matrix** | `BuildingGrid` in `building_grid.c` |
| file save/load | `file_manager.c`, `random_seed.c` |
| preprocessor directives | `constants.h` |
| sort / search | Destination sort, `qsort` on trips, idle/moving cab search |
| modular code | Separate `.c`/`.h` modules |
| logs / reports | `logger.c`, `statistics.c`, `text_table.c` |

---

## Known limitations

1. **SCAN** is per-cab (not a centralized group controller for the whole bank).
2. **No mid-flight retargeting** — a cab already moving cannot be rerouted to a new floor without finishing its scheduled arrival event.
3. **Energy** and **emergency / maintenance** flows not implemented (`simulation.c` has a placeholder TODO).
4. Run from the **project directory** so `config.txt`, `random_seed.txt`, and output logs resolve correctly.
5. Extreme overload (too many requests for the configured fleet) can still produce SLA violations despite wait-priority dispatch.

---

## Team & handoff

| Role | Responsibility |
|------|----------------|
| Core DES | FEL, handlers, movement, doors, ride-sharing |
| Dispatch / SLA | Wait-priority assignment, statistics, seed spreading |
| Remaining | Energy, emergencies — see **`TODO.md`** |

Before extending the project:

1. Read `docs/ARCHITECTURE.md` and `docs/ALGORITHMS.md`
2. Follow the open items in `TODO.md`
3. Search `TODO` in `*.c` for insertion points

---

## License / course

Academic project for a **Discrete Event Simulation** course. Use and extend per your institution's collaboration rules.

---

**Questions?** Open a GitHub issue or contact the repository owner.
