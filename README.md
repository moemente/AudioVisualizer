# AudioVisualizer

A real-time audio visualizer for Windows and Linux built in C++ using RtAudio, KissFFT, and ImGui.

Captures your default audio output via loopback and renders it as a live waveform and frequency spectrum.

---

## Running from a Release

### Windows — `.zip`
1. Download and extract `AudioVisualizer-windows.zip`
2. Run `AudioVisualizer.exe`

No install needed.

### Linux — `.tar.gz`
1. Download and extract `AudioVisualizer-linux.tar.gz`
2. Mark the binary as executable and run it:
```bash
chmod +x AudioVisualizer
./AudioVisualizer
```

Requires PipeWire or PulseAudio (standard on Ubuntu 22.04+).

---

## Building from Source

### Windows

**Requirements**
- Windows 10/11
- CMake 3.20+
- MSVC (Visual Studio 2022 Build Tools or later)

```bash
git clone https://github.com/moemente/AudioVisualizer.git
cd AudioVisualizer
cmake -B build -A x64
cmake --build build --config Release
```

Output: `build/Release/AudioVisualizer.exe`

---

### Linux

**Requirements**
- CMake 3.20+
- GCC or Clang
- libpulse-dev

```bash
sudo apt install cmake build-essential libpulse-dev
```

```bash
git clone https://github.com/moemente/AudioVisualizer.git
cd AudioVisualizer
cmake -B build
cmake --build build
```

Output: `build/AudioVisualizer`

---

## Libraries

- [RtAudio](https://github.com/thestk/rtaudio)
- [ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)
- [KissFFT](https://github.com/mborgerding/kissfft)
