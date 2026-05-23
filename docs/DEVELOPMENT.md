# Development Guide

## Prerequisites

- **GCC** (MinGW on Windows) with C99 support, **or**
- **Visual Studio 2022+** with Desktop development workload
- **Git**
- Optional: `make` (MinGW/Linux/macOS)

---

## Clone and branch

```bash
git clone https://github.com/GalVitrak/Elevator-Managment-DES.git
cd Elevator-Managment-DES
git checkout feature/des-foundation-half
```

**Never commit directly to `main`.** Create a feature branch per task (see `TODO.md`).

---

## Build

### GCC / Make (from repository root)

```bash
make
```

Produces `des_elevator` (Linux/macOS) or `des_elevator.exe` (Windows).

Clean:

```bash
make clean
```

Manual compile:

```bash
gcc -Wall -Wextra -std=c99 -pedantic -o des_elevator \
  main.c simulation.c elevator.c passenger.c floor.c \
  event.c logger.c file_manager.c
```

### Visual Studio

1. Open `DES-Elevator.slnx`.
2. Select configuration (e.g. **Debug | x64**).
3. Build — sources are referenced from parent directory (`..\main.c`, etc.).
4. Set working directory to repo root so `config.txt` and logs are found.

If link fails with “Permission denied”, close a running `des_elevator.exe` from a previous debug session.

---

## Run

From repository root (where `config.txt` / logs are written):

```bash
./des_elevator
# or
des_elevator.exe
```

### Menu reference

| Option | Action |
|--------|--------|
| 1 | New simulation: interactive config + seed requests + **run DES** |
| 2 | Load `config.txt` and initialize (does not auto-run) |
| 3 | Save current config to `config.txt` |
| 4 | Add one passenger request (needs initialized sim) |
| 5 | Print elevators, floor queues, FEL |
| 6 | Exit |

### Sample session

1. Option **1**  
2. Floors: `5`, Elevators: `2`, Capacity: `10`, Max time: `100`  
3. Requests: `1` — e.g. source `0`, destination `3`  
4. Watch log output and `simulation_log.txt`  
5. Option **5** to inspect final state  

### Configuration file

Copy example:

```bash
cp config.txt.example config.txt
```

Format:

```ini
num_floors=5
num_elevators=2
capacity=10
max_simulation_time=1000.00
```

Validation rules (`file_manager.c`):

| Parameter | Min | Max |
|-----------|-----|-----|
| floors | 2 | 50 |
| elevators | 1 | 10 |
| capacity | 1 | 20 |
| max time | > 0 | — |

---

## Project layout

```text
DES-Elevator/                 (repo root)
├── main.c                    Menu + entry
├── constants.h               #defines
├── simulation.c / .h         DES engine
├── elevator.c / .h
├── passenger.c / .h
├── floor.c / .h
├── event.c / .h
├── logger.c / .h
├── file_manager.c / .h
├── Makefile
├── config.txt.example
├── README.md
├── TODO.md                   Status & optional extensions
├── docs/
│   ├── ARCHITECTURE.md
│   ├── DEVELOPMENT.md        (this file)
│   └── MODULES.md
└── DES-Elevator/             Visual Studio project
```

---

## Coding standards

- **C99**, `-Wall -Wextra -pedantic`
- Keep functions short; one responsibility per function
- Comment **why**, not what, above non-obvious logic
- New events: extend `EventType`, `event_type_to_string`, and a `handle_*` case
- Schedule only via `simulation_schedule_event` pattern (or extract to header if refactored)

---

## Debugging tips

1. **Print FEL** — menu option 5 or call `event_list_print`.
2. **Log file** — `simulation_log.txt` after each run.
3. **Stuck simulation** — check for no events scheduled after “No idle elevator”.
4. **Invalid floor** — floors are **0-based** (0 = ground).

---

## CI / GitHub (suggested for team)

Example workflow (not yet in repo — add under `.github/workflows/build.yml` if desired):

```yaml
name: build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: make
```

---

## Pull request checklist

- [ ] Built on GCC and/or VS without warnings
- [ ] Manual test via menu option 1
- [ ] No new memory leaks in normal path
- [ ] `TODO.md` / README updated if behavior changed
- [ ] PR targets `main` from feature branch (not direct push to `main`)
