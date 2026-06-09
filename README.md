# AudioVisualizer

A real-time audio visualizer for Windows built in C++ using RtAudio and ImGui.

## What it does

Scans your connected audio devices, finds your default output device, and feeds the output back in as an input to create a loopback. Reads the float values off that stream and renders them as a live waveform in its own window.

## How to run

Download the exe from the Releases page and run it. No install needed.

## How to build

Requires Visual Studio 2022. Open AudioVisualizer.sln and build in Release mode. Dependencies (RtAudio, ImGui, GLFW) are included in the repo. You will need to manually configure the include and library paths in the project properties to match your local GLFW installation.

## Libraries used

- [RtAudio](https://github.com/thestk/rtaudio)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)