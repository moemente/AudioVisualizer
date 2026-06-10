cmake_minimum_required(VERSION 3.20)
project(AudioVisualizer)

set(CMAKE_CXX_STANDARD 17)

# Vendored library paths
set(GLFW_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/include")
set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/lib-vc2022/glfw3.lib")
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui")
set(RTAUDIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/rtaudio")

add_executable(AudioVisualizer
    # Main source
    audioV.cpp

    # ImGui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp

    # RtAudio
    ${RTAUDIO_DIR}/RtAudio.cpp
)

target_include_directories(AudioVisualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_INCLUDE}
    ${IMGUI_DIR}
    ${RTAUDIO_DIR}
)

target_compile_definitions(AudioVisualizer PRIVATE
    __WINDOWS_WASAPI__
)

target_link_libraries(AudioVisualizer PRIVATE
    ${GLFW_LIB}
    opengl32
    winmm
    ole32
    ksuser
)

set_target_properties(AudioVisualizer PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/ENTRY:mainCRTStartup"
)