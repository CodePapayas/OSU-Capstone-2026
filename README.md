Kai Lindskog
Oregon State University
CS 462

A-Life simulation. Agents move around a procedurally generated grid, consume resources, reproduce, and evolve over generations. Built in C++17.

## Build

```bash
mkdir -p build && cd build
cmake ..
make -j4
```

Produces two executables in `build/`: `main` and `alphaDemonstration`.

---

## Executables

### `./alphaDemonstration` — recommended for running the sim

Multi-agent ticked simulation. Runs until all agents die or tick limit is reached.

```bash
./alphaDemonstration --ticks 200 --agents 10
./alphaDemonstration --ticks 500 --agents 20 --fps 5 --autosave 50
```

| Flag | Default | Description |
|------|---------|-------------|
| `--ticks N` | 10 | Ticks to run |
| `--agents N` | 5 | Number of agents to spawn |
| `--pop-cap N` | 200 | Max population (reproduction hard cap) |
| `--fps N` | 0 (unlimited) | Ticks per second — slow it down to watch |
| `--autosave K` | 0 (off) | Save state history every K ticks |
| `--buffer-size N` | 1000 | Circular buffer capacity |
| `--save-dir DIR` | `saves/` | Output directory for autosave files |
| `--help` | | Show usage |

Autosave files are written to `saves/autosave_tick_N.txt`.

---

### `./main` — single-agent sim or interactive menu

With no arguments, runs a single-agent simulation for 10 ticks then exits:

```bash
./main
./main --help
```

Passing exactly two arguments (e.g. `--ticks 100`) accidentally triggers an interactive menu — this is a known bug. Use `./alphaDemonstration` for controlled runs with flags.

#### Interactive menu (triggered unintentionally with 2 args)

| Option | What it does |
|--------|-------------|
| 1 | Evolution demo — 100 generations, 100 entities, up to 10 000 ticks each. **Runs for a very long time.** |
| 2 | Basic sim — N ticks (default 10, configurable via option 4) |
| 3 | Show CLI help |
| 4 | Configure ticks, autosave interval, buffer size, save directory |
| 5 | Exit |

---

## Tests

```bash
cd build

# filter_perception stack-overflow regression tests
./test_filter_perception

# Softmax + Brain stochastic tests (built from source/entity/decision_center/)
cd ../source/entity/decision_center
mkdir -p build && cd build
cmake ..
make test_softmax
./test_softmax
```

Legacy integration tests (custom test harness, no doctest):
```bash
cd build
# these targets are not wired into the root CMake; run from test/ manually if needed
```

---

## Project structure

```
main.cpp                          single-agent CLI + interactive menu
alphaDemonstration.cpp            multi-agent CLI (recommended)
source/
  simulation/                     Simulation — tick loop, perception, movement dispatch
  entity/
    decision_center/              Brain (neural net), Biology, Entity, Mutate
    perception_movement/          Perception (tile extraction), Movement (wraparound)
  environment/                    Environment, PerlinNoise, ResourceNode/Manager
build/                            CMake output (main, alphaDemonstration, test targets)
persistence/                      PostgreSQL save/load module (separate CMakeLists)
test/                             Legacy tests (custom macro harness)
```

## Persistence (optional)

Requires PostgreSQL. Set up once:

```bash
psql -U <your_user> -d postgres -c "CREATE DATABASE alife_sim;"
psql -U <your_user> -d alife_sim -f db/schema.sql
```

Set env vars:
```bash
export ALIFE_DB_HOST=localhost
export ALIFE_DB_PORT=5432
export ALIFE_DB_NAME=alife_sim
export ALIFE_DB_USER=<your_user>
```

Build and test:
```bash
mkdir -p build && cd build
cmake ../persistence
make -j4
./test_persistence
./test_auto_save
```

## Running the simulation with live stats

Always run from the project root so `db/schema.sql` resolves correctly.

**Terminal 1 — run the simulation:**
```
./build_main/main_exe
```

At startup you will be prompted to set the display interval:
```
Display frame every X ticks [default 1]:
```
Enter a number (e.g. `5`) to render the grid every 5 ticks, or press Enter to render every tick.

**Runtime controls** (press without Enter while the simulation is running):

| Key | Action |
|-----|--------|
| `P` | Pause / Resume |
| `F` | Toggle fast forward — renders every 10 ticks regardless of the display interval |
| `Q` | Quit and print the final tick count |

**Terminal 2 — run the live visualizer** (polls the DB and redraws every 2 seconds by default):
```
./build_main/data_visualization/live_visualizer_tool
```

Optional arguments (no brackets):
```
./build_main/data_visualization/live_visualizer_tool <poll_interval_seconds> <history_limit>
# e.g. poll every 2 seconds, show last 60 ticks:
./build_main/data_visualization/live_visualizer_tool 2 60
```

Both executables use the same database env vars (`ALIFE_DB_HOST`, `ALIFE_DB_PORT`, `ALIFE_DB_NAME`, `ALIFE_DB_USER`) — make sure they are set in both terminals.

## Project structure

```
include/        headers (circular buffer, fitness, resource node, save system)
src/            implementations
db/             PostgreSQL schema
persistence/    CMakeLists.txt for the save/load module
test/           integration tests
decision_center/ brain / neural decision making
```



