# Configuration Reference

---

## File location

| Constant | Default path |
|----------|--------------|
| `CONFIG_FILE_NAME` | `config.txt` |

Must be in **working directory** when program runs (usually repo root).

---

## File format

Plain text, one key per line, `key=value`:

```ini
num_floors=101
num_elevators=20
capacity=16
max_simulation_time=7200.00
```

Underground floors are set via menu **6** (stored in `SimulationConfig.numUndergroundFloors`); seed file may include related fields when saved.

- No spaces required around `=`
- Lines not matching known keys are ignored
- All four keys **required** for successful `config_load`

---

## Parameters (`config.txt` / `SimulationConfig`)

| Key / field | Meaning | Valid range (`constants.h`) |
|-------------|---------|------------------------------|
| `num_floors` | Levels at/above ground incl. 0 | 1 – 151 internal (display 0 .. 150 above) |
| `numUndergroundFloors` | Basements (menu **6**) | 0 – 20 (display -1 .. -N) |
| `num_elevators` | Elevator count | 1 – **100** |
| `capacity` | Max passengers per cab | 1 – 20 |
| `max_simulation_time` | Horizon (seconds) | &gt; 0 |

Interactive limits are printed by menu **6** (`print_configuration_limits` in `main.c`).

---

## Dispatch-related constants (not in config.txt)

| Macro | Default | Role |
|-------|---------|------|
| `MAX_QUEUE_WAIT_SECONDS` | 180 | Queue-wait SLA target |
| `SECONDS_PER_FLOOR` | 1.0 | Travel time per floor |
| `DEST_CLUSTER_SPAN_LOOSE/TIGHT` | 10 / 3 | Dynamic destination grouping |
| `MOVING_PICKUP_MAX_SUM_WAIT` | 120 | Cap for on-the-way assignment |

See `constants.h`.

---

## API

| Function | Description |
|----------|-------------|
| `config_set_defaults` | Reset struct to defaults |
| `config_validate` | Return 1 if all fields in range |
| `config_save` | Write file |
| `config_load` | Read file |
| `config_display_to_index` / `config_index_to_display` | Floor mapping |

---

## Menu integration

| Option | Action |
|--------|--------|
| 2 | `config_load` + `simulation_init` |
| 3 | `config_save` from current sim or defaults |
| 6 | Interactive config + write `random_seed.txt` |
| 7 | Load seed + `simulation_run()` |

---

## Example file

`config.txt.example` in repository root:

```bash
cp config.txt.example config.txt
```

---

## Validation errors

| Log / message | Cause |
|---------------|-------|
| Failed to open config file | Missing path |
| Config file is missing required fields | Incomplete file |
| Loaded configuration values are invalid | Out of range values |
| Cannot save invalid configuration | Save attempted with bad struct |

---

## Presentation slide table

| Parameter | Meaning for audience |
|-----------|---------------------|
| Floors | Building height (display 0 .. max) |
| Underground | Basement levels |
| Elevators | Parallel cabs (stress tests: 20–100) |
| Capacity | Crowding limit per cab |
| Max time | Simulation horizon (safety stop) |
| Requests (seed) | Workload in menu **6** (up to 2000) |

---

## Future config ideas (not implemented)

```ini
seconds_per_floor=1.5
dispatch_policy=SCAN
energy_per_floor_kwh=0.5
```

Document here when added.
