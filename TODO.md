# TODO — Project status & optional extensions

**Repository:** [Elevator-Managment-DES](https://github.com/GalVitrak/Elevator-Managment-DES)  
**Status:** **Submission-ready** for DES coursework — core engine, dispatch, statistics, and presentation docs are complete.

**Presentation:** [docs/HOW_TO_PRESENT.md](docs/HOW_TO_PRESENT.md) · [docs/PRESENTATION_GUIDE.md](docs/PRESENTATION_GUIDE.md)

---

## Completed (core + presentation)

- [x] DES loop + sorted Future Event List (`simulation_run`, `event.c`)
- [x] Realistic travel + door cycle (`simulation_schedule_elevator_travel`)
- [x] Multi-passenger cabs, capacity, onboard linked list
- [x] Wait-priority dispatch: ETA scoring, batch matching, dynamic clustering
- [x] On-the-way pickup (`elevator_will_serve_call`, fleets &lt; 30 elevators)
- [x] Zone bias, idle reposition, same-floor waiters
- [x] 180 s queue-wait SLA + `simulation_results.txt` reporting
- [x] Random seed spread, up to 2000 requests, 100 elevators
- [x] Building grid, door/onboard logging
- [x] README + **HOW_TO_PRESENT** + presentation-oriented inline comments in source

---

## Optional (only if course or you want extra credit)

### Features

- [ ] **Energy model** — kWh per floor, idle cost; add to results file (`simulation.c` `handle_doors_close`).
- [ ] **Emergency / maintenance** — `simulation.c` TODO ~684; new `EventType`s; `ELEVATOR_OUT_OF_SERVICE`.
- [ ] **Mid-flight SLA retarget** — cancel/reschedule arrival when wait &gt; 180 s and cab is moving.
- [ ] **Valgrind** — leak check on full 2000-request run (Linux).

---

## In-code TODO

```bash
grep -r "TODO" --include="*.c" --include="*.h"
```

| Location | Topic |
|----------|--------|
| `simulation.c` | Emergency / maintenance (optional extension) |

---

## Testing before presentation

1. Build Release; run from repo root.
2. Menu **5** → generate `random_seed.txt` (your building size).
3. Menu **6** → run; open `simulation_results.txt` (service %, max wait, SLA line).
4. Menu **4** — optional: show grid + FEL.
5. Follow [docs/HOW_TO_PRESENT.md](docs/HOW_TO_PRESENT.md) demo section.

---

## Definition of done (course project)

- [x] DES with FEL and event handlers
- [x] Non-zero movement and doors
- [x] Dispatch + capacity + statistics file
- [x] SLA-oriented queue-wait policy
- [x] README + presentation guide
- [ ] Energy / emergency (optional only)
