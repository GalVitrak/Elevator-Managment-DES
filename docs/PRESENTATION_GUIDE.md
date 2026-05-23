# Presentation Guide

How to present the **Elevator Management System (DES)** project in class: structure, emphasis, and delivery tips.

**Companion files:** [PRESENTATION_SLIDES.md](PRESENTATION_SLIDES.md) | [DEMO_SCRIPT.md](DEMO_SCRIPT.md) | [FAQ.md](FAQ.md)

---

## 1. Presentation goals

By the end, the audience should understand:

1. **What problem** you solve (multi-elevator passenger service in a building).
2. **Why DES** is the right paradigm (events, not fixed time steps).
3. **How your code is organized** (modular C, FEL, queues).
4. **What works today** (foundation ~50%) vs **what comes next** (phase 2).
5. **That you can demonstrate** a working simulation live.

---

## 2. Recommended format

| Segment | Duration | Content |
|---------|----------|---------|
| Introduction | 2 min | Problem, DES one-liner |
| Theory | 5 min | DES, FEL, event types — use [DES_THEORY.md](DES_THEORY.md) |
| Architecture | 5 min | Modules, diagrams — [PROJECT_OVERVIEW_VISUAL.md](PROJECT_OVERVIEW_VISUAL.md) |
| Implementation highlights | 5 min | Structs, linked lists, dynamic memory |
| Live demo | 5–7 min | [DEMO_SCRIPT.md](DEMO_SCRIPT.md) |
| Limitations & future work | 3 min | Phase 2, `TODO.md` |
| Q&A | 5 min | [FAQ.md](FAQ.md) |

**Total:** ~25–30 minutes (adjust to course limit).

---

## 3. Storyline (narrative arc)

Use this story — it is easy to remember:

> **“A building has floors, elevators, and passengers. Passengers appear and press buttons. Instead of simulating every second of the day, we only simulate moments when something *happens* — a call, an arrival, doors opening. Those moments are *events* stored in a *Future Event List* sorted by time. The simulation clock jumps from event to event. Our C program implements that list, the building state, and simple dispatch: send the first free elevator. Phase 1 proves the engine works; phase 2 will add realistic movement and smart scheduling.”**

---

## 4. What to emphasize ( graders care )

| Topic | Why it matters | Where in code |
|-------|----------------|---------------|
| **DES paradigm** | Core course concept | `simulation_run()` |
| **Future Event List** | Classic DES data structure | `event.c` |
| **Linked lists** | Academic requirement | FEL + floor queues |
| **Dynamic memory** | Academic requirement | `calloc` elevators/floors |
| **Modularity** | Software engineering | 8 module pairs |
| **Logging** | Traceability | `logger.c`, `simulation_log.txt` |
| **File I/O** | Persistence | `file_manager.c` |

---

## 5. What to de-emphasize

- Do not claim realistic physics yet — say **“instant movement is a deliberate placeholder.”**
- Do not promise GUI or threading — out of scope.
- Do not read code line-by-line — show **one handler** (`handle_passenger_call`) at most.

---

## 6. Live demo tips

1. **Terminal font size** — large enough for back row.
2. **Pre-delete** `simulation_log.txt` for a clean log (optional).
3. **Prepare inputs:** 5 floors, 2 elevators, 1 request: floor 0 → 3.
4. After run, open **`simulation_log.txt`** in an editor — shows professionalism.
5. Menu option **5** — shows FEL empty at end (good closing visual).

If demo fails: have screenshots in [SAMPLE_RUNS.md](SAMPLE_RUNS.md) as backup.

---

## 7. Slide deck strategy

- Use [PRESENTATION_SLIDES.md](PRESENTATION_SLIDES.md) as copy-paste source.
- **1 idea per slide** — avoid walls of text.
- Include **2–3 Mermaid diagrams** from [PROJECT_OVERVIEW_VISUAL.md](PROJECT_OVERVIEW_VISUAL.md).
- Last slide: **“Thank you + GitHub link + phase 2 roadmap.”**

---

## 8. Anticipated hard questions

Prepare using [FAQ.md](FAQ.md):

- Why not time-step simulation?
- How do you avoid memory leaks?
- What if all elevators are busy?
- Why C and not Python/Java?
- How will phase 2 improve dispatch?

---

## 9. Division of labor (team presentation)

| Speaker | Suggested section |
|---------|-------------------|
| Speaker A | Intro + DES theory |
| Speaker B | Architecture + data structures |
| Speaker C | Live demo + log walkthrough |
| Speaker D | Limitations, TODO, Q&A backup |

Solo presenter: follow slide order in [PRESENTATION_SLIDES.md](PRESENTATION_SLIDES.md) alone.

---

## 10. Checklist before presenting

- [ ] Project compiles (`make` or Visual Studio)
- [ ] Practiced demo once end-to-end
- [ ] `config.txt` optional backup
- [ ] GitHub repo loads README on phone (link works)
- [ ] Read FAQ top 10 questions
- [ ] Know where `simulation_run` and `event_list_insert_sorted` live
- [ ] Phase 2 branch names ready (`feature/des-realistic-movement`, etc.)

---

## 11. One-minute elevator pitch (memorize)

*“We built a discrete event simulator for elevators in standard C. Passengers generate events; a sorted future event list drives simulation time. Floors keep waiting queues; elevators are assigned when idle. The foundation includes full logging, configuration files, and modular code — about half the final project. Next we add travel times, smarter dispatch, and statistics. The repo is on GitHub with full documentation and a handoff TODO for phase two.”*

Good luck.
