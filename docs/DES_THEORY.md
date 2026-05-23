# Discrete Event Simulation — Theory for This Project

Textbook-style reference for presentations and oral exams.

---

## 1. Definition

**Discrete Event Simulation (DES)** models a system whose state changes at a **countable set of time points** (events), not continuously over every instant.

For elevators:

- State = positions, door states, queue lengths, passenger statuses
- Events = call, arrival, doors open/close, exit
- Between events, state is **frozen** (assuming no continuous processes modeled)

---

## 2. World view vs machine view

| Concept | Elevator project |
|---------|------------------|
| **Entities** | Passengers, elevators |
| **Resources** | Elevator cabs (capacity-limited) |
| **Queues** | Floor waiting lists |
| **Activities** | Travel, boarding, alighting |
| **Events** | `EventType` enum in `event.h` |

---

## 3. Simulation clock

Two common approaches:

### 3.1 Time-step (NOT used)

```text
t = 0, Δt, 2Δt, 3Δt, ...
  update all entities each step
```

**Problem:** Most steps nothing happens in a quiet building — wasted work.

### 3.2 Event-step (USED)

```text
while events remain:
    t = time of next event
    process that event
```

**Advantage:** CPU proportional to **number of events**, not simulated duration.

Our clock variable: `Simulation.currentTime` (`double`).

---

## 4. Future Event List (FEL)

The FEL (also called **pending event list** or **calendar**) holds tuples:

```text
(time, event_type, attributes...)
```

**Invariant:** events are ordered by non-decreasing `time`.

### Operations we implement

| Operation | Function | Complexity |
|-----------|----------|------------|
| Schedule | `event_list_insert_sorted` | O(n) |
| Cancel | *not implemented* | — |
| Next event | `event_list_pop_earliest` | O(1) |

For coursework building sizes (up to 100 elevators, 2000 requests), O(n) FEL insert is acceptable. A binary heap would be an optional optimization.

---

## 5. Event scheduling discipline

When handler runs at time `t`, it may schedule new events at:

- `t` — same moment (different tie-breaking order in list)
- `t + δ` — after door delay (`DOOR_OPEN_SECONDS`, `DOOR_CLOSE_SECONDS` in `constants.h`)

**Causality:** never schedule at time `< currentTime` (unless modeling retroactive — we don't).

---

## 6. State transition view

Each handler is a **state transition function:**

```text
(state, event) → (state', set of new events)
```

Example:

```text
handle_passenger_call:
  state: passenger in floor queue
  → assign elevator, schedule ARRIVAL
```

Formal notation for slides:

\[
S_{k+1} = f(S_k, e_k)
\]

---

## 7. Stopping conditions

We stop when:

1. FEL is empty, **or**
2. `currentTime >= maxSimulationTime`

Second condition discards events beyond horizon (see `simulation_run`).

---

## 8. Randomness (future)

**Deterministic seeds:** `random_seed.txt` from menu 6 with spread arrivals across the horizon.

Manual entry via menu 1 or config remains supported.

---

## 9. Validation & verification

| Technique | Our project |
|-----------|-------------|
| **Trace log** | `simulation_log.txt` |
| **State dump** | menu option 5 |
| **Assert invariants** | *phase 2* (e.g. passengerCount ≤ capacity) |
| **Replication** | same config + requests → same log |

---

## 10. Comparison to other DES examples

| System | Events |
|--------|--------|
| Bank | Customer arrival, service end |
| Traffic light | Phase change |
| CPU scheduler | Task arrival, time slice end |
| **Elevator** | Call, arrival, doors, exit |

Same FEL pattern — different domain logic in handlers.

---

## 11. Why elevators fit DES

- Changes are **sparse** (not every millisecond)
- Natural **event types** map to real hardware signals
- Industry uses simulation for **dispatch tuning**

---

## 12. Implementation choices (current build)

| Choice | Rationale |
|--------|-----------|
| Sorted linked-list FEL | Simple, correct for coursework scale |
| Event-time clock advance | Classic DES; not real-time animation |
| Batch dispatch between events | Reduces starvation under load |
| 180 s queue-wait SLA | Reported in `simulation_results.txt` |

Early prototypes used instant travel and first-idle dispatch; **current code** uses scheduled travel and ETA/batch dispatch. See [ALGORITHMS.md](ALGORITHMS.md).

---

## 13. References (for bibliography slide)

- Law, A. M. — *Simulation Modeling and Analysis* (general DES)
- Course lecture notes — FEL, clock advance
- Project docs: [ARCHITECTURE.md](ARCHITECTURE.md), [EVENT_CATALOG.md](EVENT_CATALOG.md)

---

## 14. Oral exam one-liners

**Q: What is DES?**  
A: Simulation where time jumps between events; state constant between them.

**Q: What is the FEL?**  
A: Priority queue of future events by timestamp; our implementation is a sorted linked list.

**Q: What advances time?**  
A: Popping the earliest event and setting `currentTime = event.time`.
