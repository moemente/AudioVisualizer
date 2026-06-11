cmake_minimum_required(VERSION 3.20)
project(AudioVisualizer)

set(CMAKE_CXX_STANDARD 17)

set(GLFW_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/include")
set(IMGUI_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui")
set(RTAUDIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/rtaudio")
set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/kissfft")

add_executable(AudioVisualizer
    audioV.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
    ${RTAUDIO_DIR}/RtAudio.cpp
    ${KISSFFT_DIR}/kiss_fft.c
)

target_include_directories(AudioVisualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_INCLUDE}
    ${IMGUI_DIR}
    ${RTAUDIO_DIR}
    ${KISSFFT_DIR}
)

if(WIN32)
    target_compile_definitions(AudioVisualizer PRIVATE __WINDOWS_WASAPI__)
    target_link_libraries(AudioVisualizer PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/lib-vc2022/glfw3.lib"
        opengl32 winmm ole32 ksuser
    )
    set_target_properties(AudioVisualizer PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
else()
    target_compile_definitions(AudioVisualizer PRIVATE __LINUX_PULSE__)
    target_link_libraries(AudioVisualizer PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/lib-linux/libglfw3.a"
        GL dl pthread pulse pulse-simple
    )
endif()