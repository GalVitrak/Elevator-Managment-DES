# User Guide — Operator Manual

How to use the Elevator DES program without reading source code.

---

## Installation

1. Clone: `git clone https://github.com/GalVitrak/Elevator-Managment-DES.git`
2. Build: `make` (or Visual Studio)
3. Run from repository root (where `config.txt` / `random_seed.txt` are read/written)

---

## Menu overview

| Option | Purpose |
|--------|---------|
| **1** | Small interactive sim: configure, add requests, **run** |
| **2** | Load `config.txt`, init sim (no run) |
| **3** | Save `config.txt` |
| **4** | Add one manual request (sim must exist) |
| **5** | Print elevators, queues, building grid, FEL |
| **6** | Configure building + generate **`random_seed.txt`** |
| **7** | Load seed + **run** full simulation |
| **8** | Exit |

**Recommended for presentation:** **6** → **7**, then open `simulation_results.txt`.

---

## Menu option 1 — Start new simulation

Configure building, enter passenger requests manually, run DES once.

- Floors above ground (0 = ground only, up to 150 above)
- Underground floors (optional, display -1 .. -N)
- Elevators (1–100), capacity (1–20), max time, request count

Produces `simulation_log.txt`; statistics appended at end of run.

---

## Menu option 6 — Generate random seed file

Prints allowed ranges, then prompts for full building + workload:

- Writes **`random_seed.txt`** (config + seed + passenger list)
- Does **not** run simulation

Use before option **7** for repeatable stress tests.

---

## Menu option 7 — Load seed and run

Loads `random_seed.txt`, runs `simulation_run()` to completion (or until `max_simulation_time`).

Outputs:

- **`simulation_log.txt`** — event trace  
- **`simulation_results.txt`** — service rate, waits, SLA, per-passenger table, utilization  

---

## Menu option 5 — Print system state

Shows simulation time, each elevator, floor queues, **building grid**, and pending FEL events.

---

## Floor numbering

| Display | Meaning |
|---------|---------|
| 0 | Ground |
| 1 … N | Above ground |
| -1 … -K | Basement levels (if configured) |

Destination should differ from source (same-floor trips handled separately).

---

## Results file (key lines)

| Line | Meaning |
|------|---------|
| Service rate | % of requests completed |
| Maximum queue wait | Longest call-to-boarding time |
| Queue waits over SLA | Count over 180 s (`MAX_QUEUE_WAIT_SECONDS`) |
| Per-passenger table | Queue, travel, total trip per ID |

---

## Error messages

| Message | Cause | Fix |
|---------|-------|-----|
| Invalid input | Non-numeric | Enter integers |
| Value must be between X and Y | Out of range | See menu **6** limits |
| No idle elevator available | All cabs busy at dispatch instant | Normal under load; later events may assign |
| Failed to load random_seed.txt | Missing file | Run menu **6** first |

---

## Tips for demonstrations

- **Quick logic demo:** option **1**, 5 floors, 2 elevators, 1–2 requests  
- **Impressive metrics:** option **6** → **7** (e.g. 20 elevators, 450 requests)  
- See [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md) and [DEMO_SCRIPT.md](DEMO_SCRIPT.md)

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Program closes immediately | Run from terminal |
| Linker permission denied | Close `des_elevator.exe`, rebuild |
| Config/seed not found | Run from repo root |
| Non-ASCII path issues | Use ASCII path on Windows |

---

## Further reading

- [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md) — presentation build guide  
- [FAQ.md](FAQ.md) — common questions  
- [CONFIGURATION.md](CONFIGURATION.md) — config keys and constants
