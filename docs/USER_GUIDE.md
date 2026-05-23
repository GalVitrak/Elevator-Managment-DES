# User Guide — Operator Manual

How to use the Elevator DES program without reading source code.

---

## Installation

1. Clone: `git clone https://github.com/GalVitrak/Elevator-Managment-DES.git`
2. Build: `make` (or Visual Studio)
3. Run from repository root (where `config.txt` is created)

---

## Starting the program

```bash
des_elevator.exe    # Windows
./des_elevator      # Linux / macOS
```

You see:

```text
--- Elevator Management System (DES) ---
1. Start new simulation
...
```

---

## Menu option 1 — Start new simulation

**Use when:** you want to configure, add requests, and **run** the simulation in one flow.

### Steps

1. Enter number of **floors** (2–50)  
2. Enter number of **elevators** (1–10)  
3. Enter **capacity** per elevator (1–20)  
4. Enter **max simulation time** in seconds (positive integer)  
5. Enter how many **passenger requests** to create (0–50)  
6. For each request: **source floor** and **destination floor** (0-based)

### After completion

- Simulation runs automatically  
- Log written to `simulation_log.txt`  
- Menu returns — use option 5 to inspect state  

### Example

| Step | Input |
|------|-------|
| Floors | 5 |
| Elevators | 2 |
| Capacity | 10 |
| Max time | 100 |
| Requests | 2 |
| Request 1 | 0 → 2 |
| Request 2 | 4 → 1 |

---

## Menu option 2 — Load configuration

Loads `config.txt` from current directory and initializes simulation **without running**.

### Required file format

```ini
num_floors=5
num_elevators=2
capacity=10
max_simulation_time=1000.00
```

Copy from `config.txt.example` if needed.

**Note:** After load, use option 1 or add requests (option 4); option 1 re-initializes if you start fresh.

---

## Menu option 3 — Save configuration

Writes current settings to `config.txt`.

- If simulation was initialized, saves active sim settings  
- Otherwise saves default/template values  

---

## Menu option 4 — Add passenger request manually

**Requires:** simulation already initialized (option 1 or 2 first).

Enter source and destination floors. Passenger is queued and `PASSENGER_CALL` event scheduled.

**Does not auto-run** full DES unless you trigger `simulation_run` via option 1. For class demo, prefer **option 1**.

---

## Menu option 5 — Print system state

Displays:

- Current simulation time and max time  
- Each elevator: floor, direction, status, doors, load  
- Each floor: queue size, waiting passengers  
- **Future Event List** — pending events  

Use before/after runs to explain DES to audience.

---

## Menu option 6 — Exit

Clean shutdown: frees memory, closes log file.

---

## Floor numbering

| Display | Meaning |
|---------|---------|
| 0 | Ground / entrance |
| 1 | First floor above ground |
| … | … |
| n−1 | Top floor |

**Destination must differ from source.**

---

## Log file

**Path:** `simulation_log.txt` (same folder as executable)

**Format:**

```text
[t=0.00][INFO] message here
```

Open with any text editor. Useful for reports and debugging.

---

## Error messages

| Message | Cause | Fix |
|---------|-------|-----|
| Invalid input. Please enter an integer. | Non-numeric menu input | Enter whole numbers |
| Value must be between X and Y. | Out of range | Use allowed range |
| Invalid floor in passenger request | Floor < 0 or ≥ numFloors | Check floor count |
| Failed to open config file | Missing `config.txt` | Copy example file |
| No idle elevator available | All cabs busy | Expected in phase 1; wait for phase 2 |

---

## Tips for demonstrations

- Use **5 floors, 2 elevators, 1 request** for clearest output  
- Delete old log before demo for clean file  
- Use option **5** to show empty FEL at end  

See [DEMO_SCRIPT.md](DEMO_SCRIPT.md).

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Program closes immediately | Run from terminal, not double-click |
| Config not found | Run from repo root; copy `config.txt.example` |
| Build fails | Install GCC or open VS project |
| Hebrew path issues | Use short path or move project to ASCII path |

---

## Further reading

- [DEMO_SCRIPT.md](DEMO_SCRIPT.md) — presentation walkthrough  
- [FAQ.md](FAQ.md) — common questions  
- [CONFIGURATION.md](CONFIGURATION.md) — config details
