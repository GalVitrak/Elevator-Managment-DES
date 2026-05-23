# Frequently Asked Questions (FAQ)

For presentations, oral exams, and team Q&A.

---

## General

### What does this project do?

Simulates a multi-floor building with elevators and passengers using **Discrete Event Simulation** in C. Users configure the building, add ride requests, and watch the simulator process events in time order.

### Why elevators?

Classic DES domain — queues, resources, discrete state changes — easy to explain visually.

### Is the project finished?

About **50%**. Foundation (FEL, handlers, logging, config) is done. Realism and analytics are phase 2 (`TODO.md`).

---

## DES theory

### What is Discrete Event Simulation?

Simulation where time jumps between **events**; system state is constant between events.

### What is the Future Event List?

A collection of pending events sorted by time. We use a **sorted linked list** (`event.c`).

### Why not simulate every second?

Wasteful when nothing happens. DES scales with **event count**, not simulated seconds.

### What advances the clock?

`simulation_run` sets `currentTime = event.time` when popping the earliest event.

---

## Implementation

### Which language and standard?

**C99**, compiled with GCC or MSVC. No C++, no external libraries.

### How many source files?

8 modules × 2 (.c/.h) + `main.c` + `constants.h` ≈ 17 source/header files.

### Where is the main loop?

`simulation.c` → `simulation_run()`.

### How are passengers stored?

- **Waiting:** linked list per floor (`floor.c`)  
- **In elevator:** pointer in `activePassengersByElevator[]` (one per cab in phase 1)

### How is memory freed?

`simulation_destroy()` frees arrays, FEL, floor queues, active passengers. Events freed after handling. Passengers freed on exit or destroy.

### Are there memory leaks?

Foundation path is designed for paired alloc/free. Formal proof: run Valgrind on Linux before final submission (recommended for phase 2).

---

## Behavior

### How are elevators assigned?

**First idle** elevator (lowest id) with closed doors (`elevator_find_first_idle`).

### Why do elevators “teleport”?

**Phase 1 placeholder** to test event logic before adding travel delays (phase 2).

### What if all elevators are busy?

Passenger stays in floor queue; warning logged. No automatic retry until phase 2 dispatch.

### Are floors 0-based or 1-based?

**0-based** in code and menu (0 = ground).

### What do hall buttons (`upButtonPressed`) do?

Set during call; full clear logic is phase 2 polish.

---

## Files & configuration

### What is `config.txt`?

Text file with floors, elevators, capacity, max simulation time.

### What is `simulation_log.txt`?

Append-style log of all `log_message` output with simulation timestamps.

### Can I run without the menu?

Not in phase 1 — `main.c` is the only entry. Phase 2 could add batch mode.

---

## Git & team

### Why two phases / branches?

So foundation stays stable on `main` while features develop on `feature/*` branches.

### Where does teammate start?

`TODO.md` + `docs/ARCHITECTURE.md`.

---

## Presentation-specific

### What should we show live?

Menu option 1, one passenger request, log file, empty FEL at end. See [DEMO_SCRIPT.md](DEMO_SCRIPT.md).

### What if the demo fails?

Use [SAMPLE_RUNS.md](SAMPLE_RUNS.md) and diagrams from [PROJECT_OVERVIEW_VISUAL.md](PROJECT_OVERVIEW_VISUAL.md).

### How long is the talk?

25–30 minutes recommended; see [PRESENTATION_GUIDE.md](PRESENTATION_GUIDE.md).

---

## Grading / academic

### Where is each course requirement?

See [ACADEMIC_REQUIREMENTS.md](ACADEMIC_REQUIREMENTS.md) and [GRADING_MAP.md](GRADING_MAP.md).

### Did you use threads / GUI / database?

**No** — explicitly out of scope per assignment.

---

## Future (phase 2)

### What comes next?

Real movement, smart dispatch, capacity checks, statistics, energy, emergencies. Full list in `TODO.md`.

### Will you add a GUI?

Not planned in current scope; console is sufficient for DES course.

---

## Still stuck?

1. Read [DOCUMENTATION_INDEX.md](../DOCUMENTATION_INDEX.md)  
2. Search codebase: `grep -r "function_name"`  
3. Open GitHub issue on the repo
