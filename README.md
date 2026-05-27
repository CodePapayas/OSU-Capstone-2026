```
╔══════════════════════════════════════════════════════════════════════╗
║                                                                      ║
║    ___    __    ___  ____  ____     ___  ___  __  __                 ║
║   / _ |  / /   |_ _||  __||  __|   / __||_ _||  \/  |               ║
║  / __ | / /__   | | | |_  | |_     \__ \ | | | |\/| |               ║
║ /_/ |_|/____/  |___||_|   |___|    |___/|___||_|  |_|               ║
║                                                                      ║
║  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ║
║       ARTIFICIAL  LIFE  EVOLUTION  SIMULATOR  //  C++17              ║
║  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ║
║                                                                      ║
║   Oregon State University  ·  CS 462  ·  2026                       ║
║   Lindskog · Wilkins-Olson · Sherman · Bliss · Stickler              ║
║                                                                      ║
╚══════════════════════════════════════════════════════════════════════╝
```

> Agents evolve. Resources deplete. The fit survive.
> Watch emergence happen — one tick at a time.

---

<!-- GIF: drop a screen recording of the sim running here -->
<!-- ![Simulation running](docs/demo.gif) -->

![Simulation Demo](docs/demo.gif)

---

## What Is This

Grid-based artificial life simulation. Agents navigate a world, consume resources, reproduce, and evolve over thousands of ticks. Neural decision-making, persistent state, real-time visualizer. Built in C++17 with PostgreSQL persistence and an ImGui renderer.

Key systems:
- **Agents** — sense environment, make decisions, compete for food and mates
- **Evolution** — fitness-weighted reproduction, trait drift over generations
- **Persistence** — tick-accurate autosave to PostgreSQL + CSV export
- **Visualizer** — live ImGui dashboard backed by DB snapshots

---

<!-- IMAGE: drop a screenshot of the visualizer / grid state here -->
<!-- ![Visualizer screenshot](docs/visualizer.png) -->

![Visualizer](docs/visualizer.png)

---

## Build & Run

```bash
mkdir -p build && cd build
cmake ..
make -j4
./main --ticks 100 --autosave 25
```

Quick test (no DB required):
```bash
g++ -std=c++17 -I include test/test_main.cpp src/resource_node.cpp -o test_executable
./test_executable
```

### CLI Flags

| Flag | Default | Description |
|------|---------|-------------|
| `--ticks N` | 10 | Simulation ticks to run |
| `--autosave K` | 0 (off) | Save state every K ticks |
| `--buffer-size N` | 1000 | Circular buffer capacity |
| `--save-dir DIR` | `saves/` | Autosave output directory |
| `--help` | | Show usage |

---

## Persistence (PostgreSQL)

Requires PostgreSQL. macOS: `brew install postgresql@14`

**One-time setup:**
```bash
psql -U <your_user> -d postgres -c "CREATE DATABASE alife_sim;"
psql -U <your_user> -d alife_sim -f db/schema.sql
```

**Env vars** (add to `~/.zshrc`):
```bash
export ALIFE_DB_HOST=localhost
export ALIFE_DB_PORT=5432
export ALIFE_DB_NAME=alife_sim
export ALIFE_DB_USER=<your_user>
```

**Build persistence module:**
```bash
mkdir -p build && cd build
cmake ../persistence
make -j4
```

**Run persistence tests:**
```bash
./test_persistence
./test_auto_save
```

---

## Project Structure

```
include/          headers — circular buffer, fitness, resource node, save system
src/              implementations
db/               PostgreSQL schema
persistence/      CMakeLists for save/load module
test/             integration tests
decision_center/  neural decision-making brain
```

---

```
[ SYSTEM READY ]  ████████████████████  100%   >> SIMULATION LOADED
```
