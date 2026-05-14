cmake_minimum_required(VERSION 3.16)
project(a_life_cs461 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PostgreSQL REQUIRED)

add_library(alife_persistence STATIC
    src/db_connector.cpp
    src/save_manager.cpp
    src/auto_save.cpp
)
target_include_directories(alife_persistence PUBLIC include ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(alife_persistence PUBLIC ${PostgreSQL_LIBRARIES})

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

# filter_perception stack-overflow regression tests
enable_testing()
add_executable(test_filter_perception
    source/simulation/test_filter_perception.cpp
    source/simulation/Simulation.cpp
    source/environment/Environment.cpp
    source/environment/resource_node.cpp
    ${PERCEPTION_MOVEMENT_SOURCES}
    ${DECISION_CENTER_SOURCES}
)
target_include_directories(test_filter_perception PRIVATE
    ${PROJECT_SOURCE_DIR}/source/entity/decision_center
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/source/entity/perception_movement
    ${PROJECT_SOURCE_DIR}/source/entity/decision_center/tests
)
add_test(NAME FilterPerceptionTests COMMAND test_filter_perception)

if(TARGET alife_persistence)
    target_link_libraries(main_exe PRIVATE alife_persistence)
    target_compile_definitions(main_exe PRIVATE ALIFE_USE_DB)
endif()
