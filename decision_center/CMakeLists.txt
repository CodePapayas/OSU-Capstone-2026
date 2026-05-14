cmake_minimum_required(VERSION 3.16)
project(decision_center LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Source files
set(SOURCES
    brain.cpp
    entity.cpp
    biology.cpp
)

# ==================== Main Library ====================
add_library(decision_center STATIC ${SOURCES})
target_include_directories(decision_center PUBLIC
    ${PROJECT_SOURCE_DIR}
)

# ==================== Tests (using doctest) ====================
# Biology tests
add_executable(test_biology test_biology.cpp)
target_link_libraries(test_biology PRIVATE decision_center)
target_include_directories(test_biology PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME BiologyTests COMMAND test_biology)

# Entity tests
add_executable(test_entity test_entity.cpp)
target_link_libraries(test_entity PRIVATE decision_center)
target_include_directories(test_entity PRIVATE ${PROJECT_SOURCE_DIR})
add_test(NAME EntityTests COMMAND test_entity)

# Original math tests (if they exist)
if(EXISTS "${PROJECT_SOURCE_DIR}/tests/test_maths.cpp")
    add_executable(decision_center_tests
        tests/test_maths.cpp
    )
    target_link_libraries(decision_center_tests PRIVATE decision_center)
    add_test(NAME MathTests COMMAND decision_center_tests)
endif()
