# Sample Runs — Expected Behavior

Reference outputs for presentation backup and testing.  
Times spread across the horizon when using menu **6**/**7** (not all at `t=0`).

---

## Run A — Single passenger (menu 1, small building)

### Input

| Field | Value |
|-------|-------|
| Floors above ground | 4 (display 0–4) |
| Elevators | 2 |
| Capacity | 10 |
| Max time | 200 |
| Request | 0 → 3 |

### Expected behavior

1. Passenger queued on floor 0  
2. Dispatch assigns a cab; travel to floor 0 takes ~0 s if already there or several seconds  
3. Doors open → board → doors close → travel to floor 3 (~3 s + doors)  
4. Alight at 3; FEL empty when done  

### Log patterns to point at

```text
Simulation started
Processing call for passenger ...
Elevator N traveling 0 -> 3 (... arrives t=...)
Elevator N doors opened at floor ... (onboard: ...)
Batch: elevator ... <- passenger ...   [if batch dispatch logs]
Simulation finished at t=...
>>> Simulation summary saved to: simulation_results.txt
```

---

## Run B — Stress test (menu 6 → 7)

### Example configuration

| Field | Example |
|-------|---------|
| Floors above ground | 100 |
| Elevators | 20 |
| Capacity | 16 |
| Requests | 450 |
| Max time | 7200 |

### Expected results (order of magnitude)

| Metric | Typical |
|--------|---------|
| Service rate | ~100% if fleet sized for load |
| Max queue wait | Under 180 s when SLA met |
| Queue waits over SLA | 0 |
| Simulation end | Before max time if all served |

Your exact numbers depend on seed and fleet size.

---

## Run C — Overload teaching example

| Field | Value |
|-------|-------|
| Elevators | 1 |
| Requests | 500+ |

Expect **low service rate**, long waits — demonstrates capacity limits, not dispatch bugs.

---

## Run D — Invalid input

| Input | Expected |
|-------|----------|
| Out-of-range floor | Re-prompt (menu **6**) |
| Bad menu choice | “Choose 1-8” |

---

## Run E — No cab at one instant

Under heavy load, log may show:

```text
No idle elevator available - will retry when a cab is free
```

Passenger should still be served later if horizon and fleet allow.

---

## Screenshot checklist for slides

1. Menu **7** run finishing message  
2. `simulation_results.txt` — service rate + SLA line  
3. Snippet of per-passenger table  
4. Optional: menu **5** building grid  
5. Code: `simulation_run` loop (IDE screenshot)

See [HOW_TO_PRESENT.md](HOW_TO_PRESENT.md).
