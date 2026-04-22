cmake_minimum_required(VERSION 3.16)
project(a_life_cs461 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add subdirectories for modules
add_subdirectory(decision_center)
add_subdirectory(persistence)
add_subdirectory(data_visualization)

# Perception and Movement sources
set(PERCEPTION_MOVEMENT_SOURCES
    perception_movement/perception.cpp
    perception_movement/movement.cpp
)

# Main sources — resource_node.cpp is provided by alife_persistence
set(MAIN_SOURCES
    main.cpp
    source/Simulation.cpp
    source/Environment.cpp
)

add_executable(main_exe
    ${MAIN_SOURCES}
    ${PERCEPTION_MOVEMENT_SOURCES}
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/decision_center
    ${PROJECT_SOURCE_DIR}/perception_movement
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/include
)

# Link decision_center and persistence libraries
if(TARGET decision_center)
    target_link_libraries(main_exe PRIVATE decision_center)
endif()

if(TARGET alife_persistence)
    target_link_libraries(main_exe PRIVATE alife_persistence)
endif()
