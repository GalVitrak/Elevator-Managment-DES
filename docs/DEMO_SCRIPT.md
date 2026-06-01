# Live Demo Script

**Duration:** 5–7 minutes  
**Prerequisites:** Built executable, terminal visible to audience  

**Paths:** Option **1** (quick logic) or **6** → **7** (full metrics + `simulation_results.txt`). This script follows option 1; for seed runs see [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md).

---

## Before class (5 min setup)

```bash
cd Elevator-Managment-DES
make
# optional: del simulation_log.txt   (Windows)
# optional: rm simulation_log.txt    (Linux)
```

Open in second window (optional): editor ready for `simulation_log.txt`.

---

## Minute 0:00 — Introduce program

> “This is our console-based DES simulator. No GUI — we focus on the simulation engine.”

Run:

```bash
des_elevator.exe
```

Show menu briefly — don’t read every line.

---

## Minute 0:30 — Start simulation (Option 1)

| Prompt | Type | Why |
|--------|------|-----|
| Option | `1` | New simulation |
| Floors | `5` | Small building, easy to explain |
| Elevators | `2` | Shows dispatch choice |
| Capacity | `10` | Standard |
| Max time | `100` | Won’t run forever |
| # requests | `1` | Keep demo short |
| Source | `0` | Ground floor |
| Destination | `3` | Visible movement |

**Say while typing:**

> “We define the building, seed one passenger from floor 0 to floor 3, then the engine runs automatically.”

---

## Minute 2:00 — Watch console output

Point at lines:

1. `Simulation started`
2. `Passenger … request queued`
3. `Assigning elevator 0 to floor 0`
4. `Passenger boarded`
5. `Doors closed` / `Doors opened`
6. `Simulation finished`

**Key phrase:**

> “Time stamps are **simulation time**, not wall clock. That’s DES.”

---

## Minute 3:00 — Print state (Option 5)

If simulation ended and menu returned, choose `5`.

Point out:

- Elevator 0 at floor 3 (or idle state)
- Floor queues empty
- **Future Event List: 0 events** — “nothing left scheduled”

---

## Minute 4:00 — Open log file

```bash
notepad simulation_log.txt
# or code simulation_log.txt
```

Scroll slowly — same messages as console.

> “Dual logging gives us reproducible evidence for debugging and grading.”

---

## Minute 4:30 — Results file (1 min)

Open **`simulation_results.txt`**:

- Service rate, max queue wait, **Queue waits over SLA**
- Per-passenger table (optional scroll)

> “We measure queue wait until boarding; SLA target is three minutes.”

---

## Minute 5:00 — Exit

Option `8`.

**Closing line:**

> “DES engine, realistic timing, wait-priority dispatch, and reproducible seeds — optional extensions like energy are in TODO.md.”

---

## Backup plan (if demo crashes)

1. Apologize briefly  
2. Open [SAMPLE_RUNS.md](SAMPLE_RUNS.md) — walk through expected log  
3. Show GitHub README in browser  
4. Show diagram from [PROJECT_OVERVIEW_VISUAL.md](PROJECT_OVERVIEW_VISUAL.md)

---

## Extended demo (if 10 min allowed)

Add second request before run:

- Request 2: floor 4 → 1  
Or menu **6** → **7** with your course parameters (20 elevators, 450 requests).

---

## Do NOT do during demo

- Don’t paste huge code blocks (use [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md) snippets)  
- Don’t debug compile errors live  
- Don’t run 2000 requests with verbose log unless you filter the file
