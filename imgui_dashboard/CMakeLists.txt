cmake_minimum_required(VERSION 3.16)
project(alife_imgui_dashboard LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.89.9
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

FetchContent_Declare(implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v0.16
)
FetchContent_GetProperties(implot)
if(NOT implot_POPULATED)
    FetchContent_Populate(implot)
endif()

find_package(OpenGL REQUIRED)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${implot_SOURCE_DIR}
)
target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)

add_executable(imgui_visualizer
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_visualization/ImGuiVisualizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_visualization/main_imgui_visualizer.cpp
)
target_include_directories(imgui_visualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_visualization
)
target_link_libraries(imgui_visualizer PRIVATE imgui_lib)

set(SIM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(live_visualizer
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_visualization/ImGuiVisualizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_visualization/main_live_imgui.cpp
    ${SIM_ROOT}/source/Simulation.cpp
    ${SIM_ROOT}/source/Environment.cpp
    ${SIM_ROOT}/source/resource_node.cpp
    ${SIM_ROOT}/decision_center/entity.cpp
    ${SIM_ROOT}/decision_center/brain.cpp
    ${SIM_ROOT}/decision_center/biology.cpp
    ${SIM_ROOT}/perception_movement/perception.cpp
    ${SIM_ROOT}/perception_movement/movement.cpp
)
target_include_directories(live_visualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../data_visualization
    ${SIM_ROOT}/source
    ${SIM_ROOT}/include
    ${SIM_ROOT}/decision_center
    ${SIM_ROOT}/perception_movement
)
target_link_libraries(live_visualizer PRIVATE imgui_lib)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../simulation_stats_out.csv")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../simulation_stats_out.csv"
        "${CMAKE_CURRENT_BINARY_DIR}/simulation_stats_out.csv"
        COPYONLY
    )
endif()
