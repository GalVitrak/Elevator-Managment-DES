# TODO — Phase 2 (Second Half)

**Repository:** [Elevator-Managment-DES](https://github.com/GalVitrak/Elevator-Managment-DES)  
**Foundation branch:** `feature/des-foundation-half`  
**Status:** ~50% complete — foundation is done; advanced behavior is yours.

This document is the **handoff guide** for the teammate implementing the second half. Read `README.md` and `docs/ARCHITECTURE.md` first.

---

## Quick orientation

| What works today | What does not |
|------------------|---------------|
| DES loop + Future Event List (FEL) | Real travel time between floors |
| Floor waiting queues (linked list) | Smart / optimal dispatch |
| First-idle elevator assignment | Multiple passengers per elevator (proper model) |
| Instant “teleport” movement | Statistics / reports |
| Config save/load | Emergency / maintenance flows |
| Console menu + logging | Energy model |

---

## Priority roadmap (recommended order)

### P0 — Must-have for a complete academic submission

- [ ] **1. Realistic elevator movement**  
  - **Files:** `elevator.c`, `simulation.c` (`handle_elevator_arrival`, `elevator_assign_to_floor`)  
  - **Code markers:** `elevator.c:43`, `simulation.c:322`  
  - **Goal:** Do not set `currentFloor = targetFloor` instantly. Schedule `EVENT_ELEVATOR_ARRIVAL` at `currentTime + travelTime`.  
  - **Suggestion:** `travelTime = abs(target - current) * SECONDS_PER_FLOOR` (add to `constants.h`).  
  - **Keep:** elevator `status = ELEVATOR_MOVING` while traveling; ignore new assignments unless you add preemption rules.

- [ ] **2. Advanced dispatch when no idle elevator**  
  - **Files:** `elevator.c`, `simulation.c` (`handle_passenger_call`)  
  - **Code marker:** `simulation.c:287`  
  - **Goal:** When `elevator_find_first_idle` returns `-1`, assign nearest / best elevator (SCAN, LOOK, or simple “minimum distance”).  
  - **Do not break:** passengers already enqueued on floor queues.

- [ ] **3. Overload / capacity enforcement**  
  - **Files:** `simulation.c` (`handle_doors_open`)  
  - **Code marker:** `simulation.c:355`  
  - **Goal:** If `passengerCount >= capacity`, do not board; reschedule or wait. Log warning via `log_message`.

- [ ] **4. Statistics engine (end of simulation)**  
  - **Files:** new `statistics.h` / `statistics.c` (recommended), hook in `simulation_run`  
  - **Code markers:** `simulation.c:237-239`, `426`  
  - **Metrics to consider:**  
    - Average waiting time (request → board)  
    - Average travel time (board → exit)  
    - Total passengers served  
    - Per-elevator trip count  
  - **Store:** accumulate on passenger struct or parallel stats array during handlers.

### P1 — Strong polish

- [ ] **5. Full passenger lifecycle timing**  
  - **File:** `simulation.c`  
  - **Code marker:** `simulation.c:270`  
  - **Goal:** Separate door-open duration, boarding time, travel, alighting; use consistent delays (not hard-coded `0.5` only).

- [ ] **6. Multiple passengers per elevator**  
  - **Today:** `activePassengersByElevator[]` tracks **one** pointer per elevator.  
  - **Goal:** Linked list or small dynamic array on elevator (may extend `Elevator` struct or side structure).  
  - **Update:** `handle_doors_open`, `handle_passenger_exit`, destroy logic in `simulation_destroy`.

- [ ] **7. Energy consumption model**  
  - **Files:** `elevator.c` or new module, `simulation.c` (`handle_doors_close`)  
  - **Code marker:** `simulation.c:394`  
  - **Goal:** Add energy units per floor traveled + idle cost; print summary at end.

- [ ] **8. Utilization & performance reports**  
  - **Code marker:** `simulation.c:238`  
  - **Goal:** % time idle/moving, per-elevator utilization, optional export to text file.

### P2 — Advanced / if time permits

- [ ] **9. Emergency & maintenance events**  
  - **Code marker:** `simulation.c:239`  
  - **New event types:** extend `EventType` in `event.h` (e.g. `EVENT_EMERGENCY_STOP`, `EVENT_MAINTENANCE_START`).  
  - **Use:** `ELEVATOR_MAINTENANCE`, `ELEVATOR_OUT_OF_SERVICE` already exist in `elevator.h` but are unused.  
  - **Behavior:** Remove elevator from dispatch pool; reschedule affected passengers.

- [ ] **10. Smarter scheduling**  
  - **Code marker:** `simulation.c:287` (overlap with dispatch)  
  - Batch requests by direction; visit floors in SCAN order.

- [ ] **11. Final polish**  
  - **Code marker:** `simulation.c:426`  
  - Clean exit flow, reset floor buttons, consistent logging, edge cases (empty queue, invalid IDs).

---

## In-code TODO index

Search the repo: `grep -r "TODO" --include="*.c" --include="*.h"`

| Location | Topic |
|----------|--------|
| `elevator.c:43` | Travel time per floor |
| `simulation.c:237-239` | Statistics, utilization, emergency |
| `simulation.c:270` | Passenger lifecycle timing |
| `simulation.c:287` | Advanced dispatch |
| `simulation.c:322` | Movement between floors |
| `simulation.c:355` | Overload detection |
| `simulation.c:394` | Energy per trip |
| `simulation.c:426` | Exit flow + stats |

---

## Suggested Git branches (do not commit to `main`)

```text
feature/des-realistic-movement      ← start here (P0 #1)
feature/des-dispatch-algorithms     ← P0 #2, P2 #10
feature/des-statistics-engine       ← P0 #4, P1 #8
feature/des-capacity-and-multi-pax  ← P0 #3, P1 #6
feature/des-emergency-events        ← P2 #9
```

Workflow: branch → implement → `make` / VS build → manual test → commit → PR to `main`.

---

## Files you will touch most

| File | Why |
|------|-----|
| `simulation.c` | Event handlers, DES loop, scheduling |
| `elevator.c` | Movement, dispatch helpers |
| `event.h` / `event.c` | New event types, FEL helpers |
| `constants.h` | Timing constants, limits |
| `statistics.c` *(new)* | Aggregated metrics |
| `main.c` | Optional menu items (run stats, export) |

**Avoid rewriting:** `event_list_insert_sorted`, `floor_enqueue_passenger`, `logger_*`, `config_*` unless necessary.

---

## Testing checklist (manual)

After each feature:

1. Build: `make` or Visual Studio **Release|x64**.
2. Run `des_elevator.exe`, menu **1**, use 5 floors, 2 elevators, capacity 10, max time 100.
3. Seed 2–3 requests (different source/destination).
4. Menu **5** — inspect elevators, queues, FEL.
5. Open `simulation_log.txt` — confirm timestamps and event order.
6. Menu **3** → verify `config.txt`; **2** → reload.

**Edge cases to add:**

- All elevators busy → passenger stays in queue  
- Request to same floor as source (should warn — already in `simulation_add_passenger_request`)  
- Capacity full at boarding  
- Simulation stops at `max_simulation_time` with events still in FEL  

---

## Design constraints (please keep)

- **Standard C99 only** — no C++, no threads, no external libs.
- **DES pattern:** state changes only on events; advance `currentTime` to event time in `simulation_run`.
- **FEL:** always insert with `event_list_insert_sorted`; never assume FIFO without time order.
- **Memory:** every `malloc`/`calloc` must have a matching free in `simulation_destroy`, `floor_destroy`, or handler completion.
- **Logging:** use `log_message(sim->currentTime, ...)` for anything graders should see.

---

## Questions for the team lead

If blocked, confirm with course staff / team lead:

1. Required dispatch algorithm (FCFS vs SCAN vs custom)?  
2. Required statistics output format (console only vs file)?  
3. Floor numbering: 0-based (current) vs 1-based UI?  
4. Maximum project scope for final demo?

---

## Definition of done (phase 2)

- [ ] Elevators take non-zero time to move between floors  
- [ ] Dispatch works when no elevator is idle  
- [ ] Capacity respected  
- [ ] End-of-run statistics printed (and documented in README)  
- [ ] No memory leaks in a full run (optional: Valgrind on Linux)  
- [ ] README updated with new behavior  
- [ ] All in-code `TODO` markers resolved or replaced with issue links  

Good luck — the foundation is intentionally boring and stable so you can focus on simulation logic.
