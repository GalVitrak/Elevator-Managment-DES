# Frequently Asked Questions (FAQ)

For presentations, oral exams, and team Q&A.  
**Current codebase:** full DES with travel time, multi-passenger cabs, wait-priority dispatch, and `simulation_results.txt`.

---

## General

### What does this project do?

Simulates a multi-floor building with elevators and passengers using **Discrete Event Simulation (DES)** in C. Users configure the building (menu **6**), optionally generate `random_seed.txt`, run the simulator (menu **7**), and review `simulation_log.txt` plus `simulation_results.txt`.

### Why elevators?

Classic DES domain — queues, resources, discrete state changes — easy to explain visually.

### Is the project finished?

**Core coursework scope: yes.** DES engine, realistic timing, ride-sharing, dispatch (ETA + batch matching + on-the-way rules), statistics, and SLA reporting are implemented. **Optional** extensions: energy model, emergency events — see [TODO.md](../TODO.md).

---

## DES theory

### What is Discrete Event Simulation?

Simulation where time jumps between **events**; system state is constant between events.

### What is the Future Event List?

A collection of pending events sorted by time. Implemented as a **sorted linked list** (`event.c`).

### Why not simulate every second?

Wasteful when nothing happens. DES scales with **event count**, not simulated seconds.

### What advances the clock?

`simulation_run()` sets `currentTime = event->time` when popping the earliest event from the FEL.

---

## Implementation

### Which language and standard?

**C99**, compiled with GCC or MSVC. No C++, no external libraries.

### How many source files?

11 module pairs (`*.c` / `*.h`) + `main.c` + `constants.h` (e.g. `simulation`, `event`, `elevator`, `passenger`, `floor`, `logger`, `file_manager`, `random_seed`, `statistics`, `building_grid`, `text_table`).

### Where is the main loop?

`simulation.c` → `simulation_run()`.

### How are passengers stored?

- **Waiting:** FIFO linked list per floor (`floor.c`)
- **In elevator:** linked list `Elevator.onboardHead` (ride-sharing, many per cab)
- **Assigned but not boarded:** still in floor queue with `assignedElevatorId` set

### How is memory freed?

`simulation_destroy()` frees arrays, FEL, floor queues, onboard passengers, trip records. Each handled `Event` is freed after dispatch.

### Are there memory leaks?

Designed for paired alloc/free. Recommended: Valgrind on Linux for a full 2000-request run before final submission.

---

## Behavior

### How are elevators assigned?

**Wait-priority dispatch** (`simulation.c`):

1. **Batch greedy round** — best (passenger, elevator) pair by ETA + load − wait bonus
2. **Per-floor clustering** — nearby destinations share a cab (dynamic span 3 / 5 / 10 floors by wait)
3. **Idle cabs** — scored by estimated time to pickup, not only nearest floor index
4. **Moving cabs** (fleets &lt; 30 only) — if `elevator_will_serve_call()` and `ETA + wait ≤ 120 s`

Not “first idle elevator wins” anymore.

### How long does travel take?

`SECONDS_PER_FLOOR` (default **1 s** per floor). `simulation_schedule_elevator_travel()` schedules `EVENT_ELEVATOR_ARRIVAL` at `currentTime + travel + door close delay`.

### What is on-the-way pickup?

If a cab is **moving up** and has not passed floor 15, a passenger on 15 going **up** (e.g. to 19) may be assigned to that cab. **Opposite direction** or **already passed** the floor → rejected (`elevator_will_serve_call` in `elevator.c`).

With **≥ 30 elevators**, new calls use **idle** cabs only (moving pickup disabled to avoid overload).

### What is the 3-minute SLA?

`MAX_QUEUE_WAIT_SECONDS` = **180 s** — design target for **queue wait** (call until boarding). Results file lists **Queue waits over SLA** if anyone boarded after waiting longer.

### What if all elevators are busy?

Passenger stays in floor queue; dispatch retries on later events (`simulation_service_waiting_queues`). Warning may log if still unassigned after a call batch.

### Are floors 0-based or 1-based?

**Display:** ground = **0**, basements **-1 … -N**, up to **+150** above ground (when configured). Internally mapped to contiguous indices (`file_manager.c`).

### What do hall buttons do?

Set when passengers call; cleared when the floor queue is empty (`simulation_release_hall_buttons_if_clear`).

---

## Files & configuration

### What is `config.txt`?

Building parameters: floors, underground levels, elevators, capacity, max simulation time.

### What is `random_seed.txt`?

Saved scenario: config + RNG seed + passenger list. Menu **6** generates; menu **7** loads and runs.

### What is `simulation_log.txt`?

Event trace (tabular). Summary from `simulation_results.txt` is **appended** at end of run.

### What is `simulation_results.txt`?

Bank-style report: service rate, queue waits, SLA violations, per-passenger table, elevator utilization.

### Can I run without the menu?

Entry point is `main.c` only; menu **7** is the standard batch run path after seed generation.

---

## Git & team

### Where does a teammate start?

[TODO.md](../TODO.md) (optional items) + [ARCHITECTURE.md](ARCHITECTURE.md) + [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md).

---

## Presentation-specific

### What should we show live?

See [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md): menu **6** → **7**, then `simulation_results.txt`; or small menu **1** demo. Code slides: `simulation_run`, `elevator_will_serve_call`.

### What if the demo fails?

Use [SAMPLE_RUNS.md](SAMPLE_RUNS.md) and [PROJECT_OVERVIEW_VISUAL.md](PROJECT_OVERVIEW_VISUAL.md).

### How long is the talk?

25–30 minutes — [PRESENTATION_GUIDE.md](PRESENTATION_GUIDE.md).

---

## Grading / academic

### Where is each course requirement?

[ACADEMIC_REQUIREMENTS.md](ACADEMIC_REQUIREMENTS.md) and [GRADING_MAP.md](GRADING_MAP.md).

### Did you use threads / GUI / database?

**No** — out of scope.

### Is there a 2D dynamic structure?

Yes — `building_grid.c` (elevators × floors matrix), shown in menu **5**.

---

## Optional future work

### What is not implemented?

- Energy consumption model  
- Emergency / maintenance / `OUT_OF_SERVICE` flows  
- Mid-flight cancel and retarget of a moving cab  

See `TODO.md` and `/* TODO: emergency ... */` in `simulation.c`.

### Will you add a GUI?

Not in current scope; console + text reports are sufficient for DES coursework.

---

## Still stuck?

1. [DOCUMENTATION_INDEX.md](../DOCUMENTATION_INDEX.md)  
2. [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md)  
3. Search codebase: `grep -r "symbol_name"`  
4. GitHub issues on the repo
