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
num_floors=5
num_elevators=2
capacity=10
max_simulation_time=1000.00
```

- No spaces required around `=`
- Lines not matching known keys are ignored
- All four keys **required** for successful load

---

## Parameters

| Key | Type | Valid range | Default (interactive) |
|-----|------|-------------|------------------------|
| `num_floors` | int | 2 – 50 | 5 |
| `num_elevators` | int | 1 – 10 | 2 |
| `capacity` | int | 1 – 20 | 10 |
| `max_simulation_time` | double | > 0 | 1000.0 |

Defined in `constants.h` as `MIN_*` / `MAX_*`.

---

## API

| Function | Description |
|----------|-------------|
| `config_set_defaults` | Reset struct to defaults |
| `config_validate` | Return 1 if all fields in range |
| `config_save` | Write file |
| `config_load` | Read file |

---

## Menu integration

| Option | Action |
|--------|--------|
| 2 | `config_load` + `simulation_init` |
| 3 | `config_save` from current sim or defaults |

---

## Example file

See repository root: `config.txt.example`

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

Use this table on a slide titled **“Experimental parameters”**:

| Parameter | Meaning for audience |
|-----------|---------------------|
| Floors | Building height |
| Elevators | Parallel cabs |
| Capacity | Crowding limit |
| Max time | Simulation horizon (safety stop) |

---

## Phase 2 config ideas

Possible future keys (not implemented):

```ini
seconds_per_floor=1.5
door_open_duration=2.0
dispatch_policy=SCAN
random_seed=42
```

Document here when added.
