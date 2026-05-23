# Algorithms Reference

Algorithms used in the foundation phase — suitable for presentation slides and complexity discussion.

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

## 6. Dispatch — first idle elevator

**Function:** `elevator_find_first_idle` (`elevator.c`)

```text
find_idle(elevators, n):
  for i = 0 to n-1:
    if elevators[i].status == IDLE and doors == CLOSED:
      return i
  return -1
```

**O(n)** — n = number of elevators (≤ 10).

### Policy name

**FCFS on elevators** (first cab in index order), not FCFS on passengers globally.

---

## 7. Movement — instant assign (phase 1)

**Function:** `elevator_assign_to_floor`

```text
assign(elevator, floor):
  targetFloor = floor
  update direction from current vs target
  currentFloor = floor    // instant — phase 1 only
  status = MOVING
```

Phase 2 replaces last line with schedule arrival event only.

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

| Structure | Algorithm | Phase 1 complexity |
|-----------|-----------|-------------------|
| FEL | Sorted linked list insert | O(n) |
| Floor queue | FIFO tail insert | O(1) |
| Dispatch | Linear scan idle | O(n) |
| Movement | Direct assign | O(1) |

---

## 13. Phase 2 algorithm ideas

| Feature | Suggested algorithm |
|---------|---------------------|
| Nearest elevator | Min distance scan O(n) |
| SCAN dispatch | Sort floor requests on direction |
| Statistics | Welford’s online mean or batch at end |
| FEL | Binary heap |

Document chosen algorithm in README when implemented.
