# Glossary

Terms used in code, docs, and presentations.

---

| Term | Definition |
|------|------------|
| **DES** | Discrete Event Simulation — time jumps between events. |
| **FEL** | Future Event List — pending events sorted by `time` (`event.c`). |
| **Simulation time** | `Simulation.currentTime` — not wall clock. |
| **Display floor** | Number shown to user (0 = ground, negative = basement). |
| **Internal floor index** | Array index `0 .. numFloors-1` after mapping (`config_display_to_index`). |
| **Hall call** | Passenger requests service at `sourceFloor`. |
| **Queue wait** | `boardTime - requestTime` while waiting on a floor. |
| **SLA (queue)** | Target max queue wait = `MAX_QUEUE_WAIT_SECONDS` (180 s). |
| **Ride-sharing** | Multiple passengers per cab; clustered destinations. |
| **Stop mask** | `Elevator.floorStops[]` — floors this cab must visit. |
| **SCAN-style routing** | Visit stops ahead in current direction; pickups prioritized by wait. |
| **On-the-way pickup** | Assign call to a **moving** cab if same direction and not past call floor. |
| **ETA dispatch** | Choose cab with lowest estimated time to pickup (+ load, zone). |
| **Batch dispatch** | `simulation_batch_dispatch_round` — global greedy passenger–elevator match. |
| **Cluster span** | Max destination spread in one cab batch (3 / 5 / 10 floors by wait). |
| **Zone** | Floor band; soft penalty if cab serves far outside its zone (large buildings). |
| **Seed file** | `random_seed.txt` — reproducible passenger list + config. |
| **Service rate** | `passengers served / total requests` in results file. |
| **Utilization** | Fraction of horizon each elevator was `MOVING` (`statistics.c`). |

---

## Deprecated terms (old docs)

| Old term | Current meaning |
|----------|-----------------|
| Foundation / Phase 1 only | Project now includes movement + dispatch + stats |
| Instant movement / teleport | Replaced by `simulation_schedule_elevator_travel` |
| First idle dispatch | Replaced by ETA + batch + clustering |
| `activePassengersByElevator` | Replaced by `Elevator.onboardHead` list |
