# AudioVisualizer

A real-time audio visualizer for Windows and Linux built in C++ using RtAudio, KissFFT, and ImGui.

## What it does

Scans your connected audio devices, finds your default output device, and feeds the output back in as an input to create a loopback. Reads the float values off that stream and renders them as a live waveform and spectrum in its own window.

## How to run

Download the exe from the Releases page and run it. No install needed.

### Windows
Download the exe from the Releases page and run it. No install needed.

### Linux
Build from source (see below). Requires PipeWire or PulseAudio (standard on Ubuntu 22.04+).

## How to build

### Windows

#### Requirements
- Windows 10/11
- CMake 3.20+
- MSVC compiler (Visual Studio 2022 Build Tools or later)

#### Steps
```bash
git clone https://github.com/moemente/AudioVisualizer.git
cd AudioVisualizer
cmake -B build -A x64
cmake --build build --config Release
```

The exe will be at `build/Release/AudioVisualizer.exe`.

### Linux

#### Requirements
- CMake 3.20+
- GCC or Clang
- libpulse-dev

```bash
sudo apt install cmake build-essential libpulse-dev
```

#### Steps
```bash
git clone https://github.com/moemente/AudioVisualizer.git
cd AudioVisualizer
cmake -B build
cmake --build build
./build/AudioVisualizer
```

## Libraries used

- [RtAudio](https://github.com/thestk/rtaudio)
- [ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)
- [KissFFT](https://github.com/mborgerding/kissfft)