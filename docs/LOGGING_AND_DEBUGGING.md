# Logging and Debugging Guide

---

## Logging system overview

| Output | Function | File |
|--------|----------|------|
| Console | `printf` in `log_message` | stdout |
| File | `fprintf` to log file | `simulation_log.txt` |

**Initialization:** `logger_init()` at program start (truncate/write mode).  
**Shutdown:** `logger_close()` on exit.

---

## Log format

```text
[t=<simulation_time>][<LEVEL>] <message>
```

### Levels

| Level | String | When to use |
|-------|--------|-------------|
| `LOG_INFO` | INFO | Normal operations |
| `LOG_WARNING` | WARN | Degraded behavior (no idle elevator) |
| `LOG_ERROR` | ERROR | Invalid config, allocation failure |

---

## What gets logged automatically

| Category | Example |
|----------|---------|
| Event created | `Event created: PASSENGER_CALL (...)` |
| Event handled | `Event handled: DOORS_OPEN` |
| Assignments | `Assigning elevator 0 to floor 2` |
| Queue | `Passenger 1 request queued: floor 0 -> 3` |
| Doors | `Doors opened` / `Doors closed` |
| Lifecycle | `Simulation started` / `Simulation finished` |

---

## Using logs for debugging

### 1. Verify event order

Events should be non-decreasing in `t=` at pop order.  
If not, FEL insert bug.

### 2. Trace one passenger

Search log for `Passenger 1` — should see queue → assign → board → exit.

### 3. Find missing assignment

Search `No idle elevator` — passenger stuck in queue.

### 4. Compare two runs

```bash
fc simulation_log_A.txt simulation_log_B.txt
```

Deterministic phase 1 should match for same inputs.

---

## Interactive debugging (no debugger)

| Tool | Menu / function |
|------|-----------------|
| State dump | Menu option 5 |
| FEL print | Included in option 5 |
| Event list only | `event_list_print` (developer) |

---

## Debugger breakpoints (developers)

Suggested breakpoints in Visual Studio / gdb:

| Location | Why |
|----------|-----|
| `simulation_run` while loop | Each event iteration |
| `event_list_pop_earliest` | See next event |
| `handle_passenger_call` | Dispatch logic |
| `simulation_destroy` | Memory leak check |

---

## Common bugs and log signatures

| Symptom | Likely cause |
|---------|--------------|
| Simulation ends instantly at t=0 | All events same time (phase 1 normal) |
| Passenger not found warning | CALL without enqueue |
| Invalid elevator error | Bad `elevatorId` on event |
| Empty log file | `logger_init` failed — check permissions |
| Crash on exit | Double-free passenger — check destroy paths |

---

## Memory debugging (recommended phase 2)

**Linux:**

```bash
valgrind --leak-check=full ./des_elevator
```

Run menu option 1 once, exit, read leak summary.

---

## Increasing verbosity (future)

Add `#define LOG_DEBUG` in `constants.h` and extra level in `logger.c` if needed for development.

---

## Presentation tip

Show **`simulation_log.txt`** side-by-side with code slide of `log_message` — connects implementation to observable behavior.
