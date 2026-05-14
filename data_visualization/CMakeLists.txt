cmake_minimum_required(VERSION 3.16)

add_library(data_visualization STATIC
    DataVisualizer.cpp
    LiveVisualizer.cpp
)

target_include_directories(data_visualization PUBLIC
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../source
)

target_link_libraries(data_visualization PUBLIC
    alife_persistence
)

add_executable(visualizer_tool
    main_visualizer.cpp
)

target_include_directories(visualizer_tool PRIVATE
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../source
)

target_link_libraries(visualizer_tool PRIVATE
    data_visualization
)

add_executable(live_visualizer_tool
    main_live_visualizer.cpp
)

target_include_directories(live_visualizer_tool PRIVATE
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/../source
)

target_link_libraries(live_visualizer_tool PRIVATE
    data_visualization
)

# ============================================================
#  Dear ImGui + ImPlot Visualization Dashboard (self-contained)
# ============================================================
option(BUILD_IMGUI_VISUALIZER "Build the Dear ImGui data visualization dashboard" ON)

if(BUILD_IMGUI_VISUALIZER)
    include(FetchContent)

    # ----- GLFW (windowing) -----
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    FetchContent_MakeAvailable(glfw)

    # ----- Dear ImGui (no CMakeLists – just fetch source) -----
    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.89.9
    )
    FetchContent_GetProperties(imgui)
    if(NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif()

    # ----- ImPlot (no CMakeLists – just fetch source) -----
    FetchContent_Declare(implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG        v0.16
    )
    FetchContent_GetProperties(implot)
    if(NOT implot_POPULATED)
        FetchContent_Populate(implot)
    endif()

    # ----- OpenGL -----
    find_package(OpenGL REQUIRED)

    # Build ImGui + backends + ImPlot as a single static library
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

    # ImGui Dashboard executable (no DB dependency – reads CSV)
    add_executable(imgui_visualizer
        ImGuiVisualizer.cpp
        main_imgui_visualizer.cpp
    )
    target_link_libraries(imgui_visualizer PRIVATE imgui_lib)
endif()
