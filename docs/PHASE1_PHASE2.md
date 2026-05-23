# Project Status — Foundation & Advanced Features

Historical note: development started as “Phase 1 foundation + Phase 2 realism.” **Most Phase 2 items are now implemented** on `main`.

---

## Implemented (current `main`)

| Area | Status | Where |
|------|--------|--------|
| DES loop + FEL | ✓ | `simulation_run`, `event.c` |
| Travel time + doors | ✓ | `simulation_schedule_elevator_travel`, `constants.h` |
| Multi-passenger cabs | ✓ | `Elevator.onboardHead`, capacity checks |
| Wait-priority dispatch | ✓ | `simulation_find_elevator_for_pickup`, batch round, clustering |
| On-the-way pickup | ✓ | `elevator_will_serve_call` (fleets &lt; 30) |
| 180 s queue SLA reporting | ✓ | `statistics.c`, `MAX_QUEUE_WAIT_SECONDS` |
| Random seed + spread | ✓ | `random_seed.c`, menus **6** / **7** |
| Statistics + results file | ✓ | `simulation_results.txt` |
| Building grid (2D) | ✓ | `building_grid.c`, menu **5** |
| Underground + 100 elevators | ✓ | `constants.h`, menu **6** |
| Documentation + presentation guide | ✓ | `docs/HOW_TO_PRESENT.md` |

---

## Optional extensions (not required for demo)

| Feature | Priority | See |
|---------|----------|-----|
| Energy model | Optional | [TODO.md](../TODO.md) |
| Emergency / maintenance | Optional | `simulation.c` TODO |
| Mid-flight retarget at SLA breach | Optional | [TODO.md](../TODO.md) |
| Doc sync (legacy pages) | Low | Ongoing |

---

## Comparison table (for slides: before vs now)

| Aspect | Early foundation | Current system |
|--------|------------------|----------------|
| Travel | Instant assign | Delay = floors × 1 s + doors |
| Dispatch | First idle | ETA, batch, clustering, zones |
| Passengers/cab | One | Linked list, up to `capacity` |
| End report | Log only | `simulation_results.txt` + per-passenger table |
| Max scale | Small demo | Up to 171 floors, 100 elevators, 2000 requests |

---

## Timeline narrative (presentation)

```text
Architecture, FEL, queues          → done
Handlers, menu, logging, config    → done
Movement, doors, ride-sharing      → done
Dispatch + SLA + statistics        → done
Optional: energy, emergencies      → backlog
```

---

## Definition of “project complete” (course)

- [x] Non-zero travel times  
- [x] Dispatch when cabs are busy (queue + retry)  
- [x] Capacity enforced  
- [x] Statistics file + SLA line  
- [x] README + presentation docs  
- [ ] Energy / emergency (optional)

---

## Handoff

New work should **extend** `simulation.c` / `statistics.c`, not rewrite the FEL or floor queues. Read [ARCHITECTURE.md](ARCHITECTURE.md) and [ALGORITHMS.md](ALGORITHMS.md) first.
