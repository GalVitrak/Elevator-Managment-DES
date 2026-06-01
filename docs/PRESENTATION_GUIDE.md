# Presentation Guide (timing & emphasis)

Quick companion to **[HOW_TO_PRESENT.md](HOW_TO_PRESENT.md)** — use that file for **slides, code snippets, and demo steps**.

---

## Duration (~25–30 min)

| Segment | Min | Focus |
|---------|-----|--------|
| Intro + problem | 2 | Building, passengers, elevators |
| DES theory | 5 | FEL, clock jump — [DES_THEORY.md](DES_THEORY.md) |
| Architecture | 4 | Modules — [ARCHITECTURE.md](ARCHITECTURE.md) |
| Code highlights | 5 | `simulation_run`, `elevator_will_serve_call`, dispatch |
| Live demo | 6 | Menu **6**/**7** or small manual run |
| Results + limits | 3 | `simulation_results.txt`, SLA 180 s |
| Q&A | 5 | [FAQ.md](FAQ.md) |

---

## Emphasize (graders)

- **DES** — `simulation_run()` processes earliest event only.
- **FEL** — sorted linked list in `event.c`.
- **Dispatch** — ETA, clustering, wait priority, on-the-way rules.
- **SLA** — `MAX_QUEUE_WAIT_SECONDS`, results line “Queue waits over SLA”.
- **Structures** — lists, dynamic allocation, `BuildingGrid`.

## De-emphasize

- Line-by-line reading of `simulation.c` (too long).
- GUI, threads, energy (not built).
- Development environment details.

---

## Storyline (one paragraph)

A multi-elevator building is modeled with **discrete events**. Passengers arrive over time; dispatch assigns **idle or compatible moving** cabs using **ETA and queue wait**. Elevators travel floor-by-floor, open doors, board and alight. The clock advances only when events fire. A final report proves **service rate** and **queue-wait SLA**.

---

## Slide / demo assets

| Asset | File |
|-------|------|
| Full presentation build guide | [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md) |
| Slide bullets | [PRESENTATION_SLIDES.md](PRESENTATION_SLIDES.md) |
| Minute-by-minute demo | [DEMO_SCRIPT.md](DEMO_SCRIPT.md) |
| Diagrams | [PROJECT_OVERVIEW_VISUAL.md](PROJECT_OVERVIEW_VISUAL.md) |

---

## Current project status (for “future work” slide)

**Done:** travel time, doors, ride-sharing, wait-priority dispatch, statistics, seeds, 180 s SLA target.  
**Open:** energy model, emergency events — see [TODO.md](../TODO.md).
