# Sample Runs — Expected Behavior

Reference outputs for presentation backup and testing.  
Exact log order may vary slightly if multiple events share `t=0.0` (insertion order).

---

## Run A — Single passenger (0 → 3)

### Input (menu 1)

| Field | Value |
|-------|-------|
| Floors | 5 |
| Elevators | 2 |
| Capacity | 10 |
| Max time | 100 |
| Requests | 1 |
| Source | 0 |
| Destination | 3 |

### Expected behavior (narrative)

1. Passenger 1 created, enqueued on floor 0  
2. `PASSENGER_CALL` processed → elevator 0 assigned (first idle)  
3. Instant arrival at floor 0 → doors open → passenger boards  
4. Doors close → instant travel to floor 3  
5. Arrival at 3 → doors open → passenger exits  
6. FEL empty → simulation ends  

### Example log excerpts (representative)

```text
[t=0.00][INFO] Elevator DES foundation started
[t=0.00][INFO] Simulation started
[t=0.00][INFO] Passenger 1 request queued: floor 0 -> 3
[t=0.00][INFO] Event created: PASSENGER_CALL (elev=-1 pass=1 floor=0)
[t=0.00][INFO] Event handled: PASSENGER_CALL
[t=0.00][INFO] Processing call for passenger 1 on floor 0
[t=0.00][INFO] Assigning elevator 0 to floor 0
[t=0.00][INFO] Event created: ELEVATOR_ARRIVAL (elev=0 pass=1 floor=0)
[t=0.00][INFO] Event handled: ELEVATOR_ARRIVAL
[t=0.00][INFO] Elevator 0 arrived at floor 0
[t=0.00][INFO] Event created: DOORS_OPEN (elev=0 pass=1 floor=0)
[t=0.00][INFO] Event handled: DOORS_OPEN
[t=0.00][INFO] Doors opened
[t=0.00][INFO] Passenger boarded elevator
[t=0.00][INFO] Event created: DOORS_CLOSE (elev=0 pass=1 floor=3)
...
[t=0.00][INFO] Simulation finished at t=0.00
```

*Note: With instant movement, many events occur at `t=0.0`; phase 2 will spread times.*

### Final state (option 5)

- Elevator 0: floor 3, IDLE, 0 passengers  
- All floor queues empty  
- FEL size: 0  

---

## Run B — Load config then add passenger

### Steps

1. Copy `config.txt.example` → `config.txt`  
2. Menu `2` (load)  
3. Menu `4` — source 2, destination 4  
4. Menu `1` NOT required — to **run** DES you still need option 1 OR call run from code path  

**Important:** Option 4 only **schedules** request; it does **not** run `simulation_run` unless events are processed. For presentation, prefer **option 1** which runs automatically.

### Teaching point

> “Option 4 is for adding requests to an initialized sim; option 1 seeds and runs in one flow.”

---

## Run C — Invalid input (validation demo)

| Input | Expected |
|-------|----------|
| Source = 99 | Error: invalid floor |
| Source = destination = 2 | Warning: same floor |
| Floors = 1 | Rejected (min 2) |

Shows `read_int_in_range` and `simulation_validate_floor`.

---

## Run D — No idle elevator (conceptual)

With 1 elevator and two simultaneous requests (phase 1):

1. First request takes elevator 0  
2. Second may log: `No idle elevator available - request queued`  

Passenger 2 remains on floor queue until phase 2 dispatch improves.

---

## Run E — Max time cutoff

Set `max_simulation_time=0.01` with many delayed events (phase 2).  
In phase 1, simulation often ends before limit because events finish at `t=0`.

---

## Comparing console vs log file

They should match line-for-line (except startup before logger).  
Use `fc simulation_log.txt` backup copy on Windows for diff.

---

## Screenshot checklist for slides

Capture:

1. Menu on screen  
2. Mid-run log (assignment line visible)  
3. `simulation_log.txt` in editor  
4. Option 5 state dump with empty FEL  

Store in `docs/images/` if team adds screenshots later.
