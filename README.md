Kai Lindskog
Oregon State University
CS 462

A-Life simulation project. Agents move around a grid, consume resources, reproduce, and evolve over time. Built in C++17.

## Build & Run

g++ -std=c++17 -I include test/test_main.cpp src/resource_node.cpp -o test_executable
./test_executable

## Persistence (save/load system)

Requires PostgreSQL. On macOS: `brew install postgresql@14`

Set up the database once:
```
psql -U <your_user> -d postgres -c "CREATE DATABASE alife_sim;"
psql -U <your_user> -d alife_sim -f db/schema.sql
```

Set env vars (or add to ~/.zshrc):
```
export ALIFE_DB_HOST=localhost
export ALIFE_DB_PORT=5432
export ALIFE_DB_NAME=alife_sim
export ALIFE_DB_USER=<your_user>
```

Build the persistence module:
```
mkdir -p build && cd build
cmake ../persistence
make -j4
```

Run persistence tests:
```
./test_persistence
./test_auto_save
```

## Running the simulation with live stats

Always run from the project root so `db/schema.sql` resolves correctly.

**Terminal 1 — run the simulation:**
```
./build_main/main_exe
```

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



