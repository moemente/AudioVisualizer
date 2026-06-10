# AudioVisualizer

A real-time audio visualizer for Windows built in C++ using RtAudio and ImGui.

## What it does

Scans your connected audio devices, finds your default output device, and feeds the output back in as an input to create a loopback. Reads the float values off that stream and renders them as a live waveform in its own window.

## How to run

Download the exe from the Releases page and run it. No install needed.

## How to build

### Requirements
- Windows 10/11
- CMake 3.20+
- MSVC compiler (Visual Studio 2022 Build Tools or later)

### Steps
```bash
git clone https://github.com/moemente/AudioVisualizer.git
cd AudioVisualizer
cmake -B build -A x64
cmake --build build --config Release
```

The exe will be at `build/Release/AudioVisualizer.exe`.

## Libraries used

- [RtAudio](https://github.com/thestk/rtaudio)
- [ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)