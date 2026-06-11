#pragma once
#include <atomic>
#include <memory>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "kiss_fft.h"
#include "RtAudio.h"


//
// --- CONSTANTS ---
//
// The number of samples/duration of time being display to visualizer at once.
const unsigned int sampleSize = 1024 * 2;
/* 
How many audio frames RtAudio collects before calling the callback function.
If samplerate = 48000hz, RtAudio runs the callback every (1024 / 48000) = 0.0213 seconds.
Setting this too high can greatly degrade response time.
*/
unsigned int bufferFrames = 1024;
// Used to get the GLFW's window size.
int width = 0, height = 0;


//
// --- FUNCTION DECLARATIONS ---
//
// Feeds default output device in as the input device to create a loopback.
int audioLoopBack(
    void*,
    void* inputBuffer, 
    unsigned int numOfFrames, 
    double, RtAudioStreamStatus, 
    void* userData
);

//
// --- STRUCT/CLASSES ---
//
struct RingBuffer
{
    /*
    Where we averaged together the L and R samples into a mono sample.
    Also, what we pass to KissFFT to form the fftOut array.
    Data race can happen here, but refresh time is so quick that it's not important.
    */
    float data[sampleSize] = {};
    /*
    Set to atomic to avoid race conditions. Avoid main loop accessing at the same time
    the callback function is writing to it.
    */
    std::atomic<unsigned int> writeIndex = 0;
};