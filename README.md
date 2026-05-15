Kai Lindskog, Zach Wilkinson
Oregon State University
CS 461 / CS 462

A-Life simulation. Agents navigate a procedurally generated environment, consume resources, reproduce sexually with genetic crossover and neural weight mutation, and evolve over generations. Built in C++17.

---

## Build

```bash
mkdir -p build && cmake -B build -S . && cmake --build build -j4
```

Requires PostgreSQL (`libpq-dev`) for the persistence layer.

---

## Running the simulation

Always run from the project root so `db/schema.sql` resolves correctly.

```bash
source env.sh && ./build/main_exe
```

At startup you will be prompted:

| Prompt | Default | Description |
|--------|---------|-------------|
| Population size | 5 | Number of agents to spawn |
| Display interval | 1 | Render the grid every N ticks |
| Render delay (ms) | 150 | Pause between rendered frames for readability |
| God mode | N | Scripted immortal agent — eats/reproduces on a fixed cycle, never dies. Useful for testing the visualizer |

### Runtime controls

| Key | Action |
|-----|--------|
| `P` | Pause / Resume |
| `F` | Toggle fast-forward (renders every 10 ticks) |
| `Q` | Quit |

### Terminal display

The grid renders in-place (no scroll). Each cell is 2 characters wide:

| Symbol | Color | Meaning |
|--------|-------|---------|
| `██` | White → red (health) | Living agent |
| `██` | Yellow (intensity = energy) | Food resource |
| `██` | Blue (intensity = energy) | Water resource |
| `░░` | Red-green gradient (Perlin) | Empty terrain |
| `░░` | Dark red | Agent trail — tile occupied last frame |

Stats bar below the grid shows tick, alive count, average energy, and resource count. God mode is flagged `[GOD]`.

---

## Live visualizer

Polls the database every N seconds and prints a rolling stats view.

```bash
source env.sh && ./build/data_visualization/live_visualizer_tool
# optional args: <poll_interval_seconds> <history_limit>
# e.g. poll every 2s, show last 60 ticks:
./build/data_visualization/live_visualizer_tool 2 60
```

Run in a second terminal alongside the simulation.

## Post-run visualizer (ImGui)

Reads a CSV file for offline analysis.

```bash
./build/data_visualization/imgui_visualizer
# or pass a path:
./build/data_visualization/imgui_visualizer path/to/stats.csv
```

Defaults to `simulation_stats_out.csv` in the working directory.

---

## Architecture

```
main.cpp                        CLI entry point — prompts, controls, autosave wiring
source/
  Simulation.cpp / .hpp         Tick loop, multi-agent dispatch, display, god mode
  Environment.cpp / .hpp        Perlin-noise grid, tile values
decision_center/
  brain.cpp / .hpp              Multi-layer ReLU neural network
  biology.cpp / .hpp            Energy / water / health model, genetics
  entity.cpp / .hpp             Agent — owns brain + biology
  mutate.cpp / .hpp             Genetic crossover and weight mutation
perception_movement/
  perception.cpp / .hpp         Tile extraction within vision radius
  movement.cpp / .hpp           Wraparound movement, terrain energy drain
include/
  circular_buffer.h             Rolling state history
  simulation_state.h            Per-tick snapshot struct
  db_connector.h                PostgreSQL connection wrapper
  save_manager.h                Slot-based save/load
  auto_save.h                   Tick-driven autosave scheduler
src/
  db_connector.cpp
  save_manager.cpp
  auto_save.cpp
  resource_node.cpp
persistence/                    Standalone CMakeLists for the DB module
data_visualization/
  LiveVisualizer                DB-polling terminal stats
  ImGuiVisualizer               Offline ImGui/ImPlot dashboard (reads CSV)
db/
  schema.sql                    PostgreSQL schema
```

---

## Persistence (PostgreSQL)

Set up once:

```bash
psql -U <user> -d postgres -c "CREATE DATABASE alife_sim;"
psql -U <user> -d alife_sim -f db/schema.sql
```

Environment variables (put in `env.sh`):

```bash
export ALIFE_DB_HOST=localhost
export ALIFE_DB_PORT=5432
export ALIFE_DB_NAME=alife_sim
export ALIFE_DB_USER=<user>
export ALIFE_DB_PASSWORD=<password>
```

The simulation connects automatically at startup and autosaves every 3 ticks by default. Build and test the persistence module standalone:

```bash
cmake --build build --target test_persistence test_auto_save
./build/persistence/test_persistence
./build/persistence/test_auto_save
```

---

## Biology & tuning

**Death conditions:** health ≤ 0 OR 25 consecutive ticks at 0 water. Health also drains each tick energy falls below `1 - Mass` (quadratic on the deficit, compounding).

**Key constants** (`decision_center/biology_constants.hpp`):

| Constant | Value | Effect |
|----------|-------|--------|
| `ENERGY_DRAIN_COEFFICIENT` | 0.5 | Scales per-tick energy loss |
| `TERRAIN_ENERGY_COEFFICIENT` | 0.5 | Energy cost of moving across terrain |
| `TERRAIN_WATER_COEFFICIENT` | 0.08 | Water lost per move |
| `WATER_DRINK_COEFFICIENT` | 2.0 | Water gained per drink action |
| `HEALTH_COEFFICIENT` | 0.5 | Health drain scalar |
| `MUTATION_CHANCE` | 0.02 | Per-weight/bias mutation probability |

---

## Tests

```bash
cmake --build build --target test_filter_perception test_softmax test_biology test_entity
cd build && ctest --output-on-failure
```
