# Algorithms Reference

Algorithms used in the elevator DES — suitable for presentation slides and complexity discussion.  
**Dispatch and movement sections reflect the current implementation** (not the early “first idle / instant” prototype).

---

## 1. Future Event List — sorted insert

**Function:** `event_list_insert_sorted` (`event.c`)

### Pseudocode

```text
insert(list, event):
  if list empty OR event.time < head.time:
    prepend event
    return
  walk pointer while next exists AND next.time <= event.time:
    advance
  splice event after current
  increment size
```

### Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Insert | O(n) | O(1) |
| Pop earliest | O(1) | O(1) |

*n = FEL size*

### Why linked list?

- Course requirement  
- Simple to implement and debug  
- n is small in academic scenarios  

**Upgrade path:** binary heap (priority queue) for O(log n) insert.

---

## 2. Future Event List — pop earliest

**Function:** `event_list_pop_earliest`

```text
pop(list):
  if empty: return NULL
  e = head
  head = head.next
  size--
  return e
```

O(1) — FEL always sorted, so head = minimum time.

---

## 3. Floor waiting queue — enqueue

**Function:** `floor_enqueue_passenger` (`floor.c`)

```text
enqueue(floor, passenger):
  passenger.next = NULL
  if rear == NULL:
    front = rear = passenger
  else:
    rear.next = passenger
    rear = passenger
```

**O(1)** — classic tail pointer queue.

---

## 4. Floor waiting queue — dequeue

**Function:** `floor_dequeue_passenger`

```text
dequeue(floor):
  if front == NULL: return NULL
  p = front
  front = front.next
  if front == NULL: rear = NULL
  p.next = NULL
  return p
```

**O(1)**

---

## 5. Queue size

**Function:** `floor_queue_size`

Linear walk — **O(k)** where k = passengers on that floor.  
Used for display only, not hot path.

---

## 6. Dispatch — ETA scoring + batch matching

**Functions:** `simulation_find_elevator_for_pickup`, `simulation_batch_dispatch_round` (`simulation.c`)

### Idle cab selection

```text
for each idle cab with free slots:
  eta = travel_to(call) + pending_stops * door_cycle + zone_penalty
  score = eta + load * W_load - wait_seconds * W_wait_bonus
pick minimum score
```

### Moving cab (fleet size < 30)

**Function:** `elevator_will_serve_call` (`elevator.c`) — same direction, not past call floor.  
Assign only if `eta + current_wait <= MOVING_PICKUP_MAX_SUM_WAIT` (120 s).

### Batch round

```text
for all unassigned passengers and all cabs with slots:
  compute score
pick global minimum (passenger, cab); assign; repeat
```

**Complexity:** O(passengers × elevators) per round; rounds bounded in `simulation_service_waiting_queues`.

### Clustering

**Function:** `simulation_dispatch_direction_group` — FCFS by `requestTime`, group destinations within dynamic span (3 / 5 / 10 floors).

---

## 7. Movement — scheduled travel

**Function:** `simulation_schedule_elevator_travel` (`simulation.c`)

```text
travelTime = |target - current| * SECONDS_PER_FLOOR
arrivalTime = currentTime + DOOR_CLOSE_TIME + travelTime
schedule EVENT_ELEVATOR_ARRIVAL at arrivalTime
elevator status = MOVING (position updates only on arrival)
```

**O(1)** per trip segment; FEL insert O(n).

---

## 8. Passenger lookup in queue

**Function:** `simulation_find_passenger_in_queue` (`simulation.c`)

Linear search by `passengerId` — **O(k)** on floor k.

---

## 9. DES main loop

**Function:** `simulation_run`

```text
while FEL not empty and time < maxTime:
  e = pop_earliest(FEL)
  if e.time > maxTime: discard; break
  currentTime = e.time
  dispatch(e)
  free(e)
```

**Per iteration:** O(1) pop + handler cost + O(n) possible inserts from handler.

---

## 10. Config parse

**Function:** `config_load` (`file_manager.c`)

- Line-by-line `fgets`  
- Prefix match `strncmp` for keys  
- `sscanf` for values  

O(lines) — file is tiny.

---

## 11. Validation

**Function:** `config_validate`, `simulation_validate_floor`, `read_int_in_range`

Constant-time bound checks against `MIN_*` / `MAX_*` macros.

---

## 12. Comparison table for slides

| Structure | Algorithm | Typical complexity |
|-----------|-----------|-------------------|
| FEL | Sorted linked list insert | O(n) per insert |
| Floor queue | FIFO tail insert | O(1) |
| Dispatch | Batch greedy + ETA scan | O(P×E) per round |
| Movement | Schedule arrival event | O(1) + FEL insert |
| Trip report | `qsort` on records | O(R log R) |

---

## 13. Optional future algorithms

| Feature | Suggested approach |
|---------|-------------------|
| FEL at huge scale | Binary heap O(log n) insert |
| Energy | Integrate on `DOORS_CLOSE` / travel events |
| Emergency | Remove cab from dispatch pool; re-queue passengers |
| Mid-flight SLA | Cancel/reschedule `ELEVATOR_ARRIVAL` |

**Already implemented:** ETA dispatch, SCAN-style stops, batch matching, end-of-run statistics (`qsort` trips).

Document chosen algorithm in README when implemented.
