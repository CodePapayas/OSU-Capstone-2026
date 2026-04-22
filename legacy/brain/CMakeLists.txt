cmake_minimum_required(VERSION 3.18)
project(a_life_brain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

find_package(Torch REQUIRED)

# Reusable library
add_library(a_life_brain STATIC
  brain.cpp
)
target_include_directories(a_life_brain PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(a_life_brain PUBLIC "${TORCH_LIBRARIES}")

# Optional demo (off in CI; can be run locally)
option(ALIFE_BUILD_DEMO "Build the a_life_brain demo executable" ON)
if(ALIFE_BUILD_DEMO)
  add_executable(brain_demo brain_demo.cpp)
  target_link_libraries(brain_demo PRIVATE a_life_brain "${TORCH_LIBRARIES}")
endif()
