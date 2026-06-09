#include <iostream>
#include <ostream>
#include <windows.h>
#include <GLFW/glfw3.h>
#include "RtAudio.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int audioLoopBack(void*, void* inputBuffer, unsigned int numOfFrames, double, RtAudioStreamStatus, void* userData);
BOOL WINAPI onConsoleClose(DWORD signal);

// The number of samples displayed to the visualizer at once.
// Basically, controls how much history of audio we store and display.
// No latency tradeoffs. Must be larger than displaySize.
const unsigned int sampleSize = 2048;

// RtAudio callback buffer size in frames.
// In stereo, each frame will deliver bufferFrames * 2 samples.
// Major latency tradeoffs. Must be a power of 2.
unsigned int bufferFrames = 1024;

// Y-axis range for the waveform plot.
// Smaller = more reactive to quiet audio but clips loud audio.
// Larger = handles loud audio cleanly but quiet audio looks flat.
const float yAxisMin = -.5;
const float yAxisMax = .5f;

bool running = true;

struct RingBuffer
{
    // Buffer array to store samples of audio in float format.
    // Gets written to by the callback function by started by RtAudio.
    // Gets read by loop logic for visualizing the values.
    float data[sampleSize] = {};
    // Atomic because this index is both being accessed/updated by the callback func
    // and being accessed/updated by the loop logic for plotting values.
    // Avoids race conditions.
    std::atomic<unsigned int> writeIndex = 0;
};

int  main()
{
    // Heap allocated RingBuffer variable
    RingBuffer* dataFlow = new RingBuffer();

    // closing console sets ruunning = false
    SetConsoleCtrlHandler(onConsoleClose, TRUE);

    // Where the default device's values will be stored
    // to use in RtAudio's loopback setup.
    RtAudio::StreamParameters defaultDevice;
    unsigned int defaultSampRate = 0;

    RtAudio audio;
    // Gets the number of audio devices connected to user's computer.
    unsigned int deviceCount = audio.getDeviceCount();

    if (deviceCount <= 0)
    {
        std::cout << "---ERROR: No audio devices found---\n\n";
        return 1;
    }

    std::cout << "Number of devices found: " << deviceCount;

    // Stores all ids to a vector to loop over.
    std::vector<unsigned int> ids = audio.getDeviceIds();

    if (ids.empty() == true)
    {
        std::cout << "\n\n---ERROR 2 WITH READING DEVICES---\n\n";
        return 1;
    }

    // Loop over all ids in previously made vector of ids.
    for (unsigned int id : ids)
    {
        // Create a local instance of the device to get info from.
        RtAudio::DeviceInfo device = audio.getDeviceInfo(id);

        // Looking for default output device. If found, save
        // values to the defaultDevice variable.
        if (device.isDefaultOutput == true)
        {
            defaultDevice.deviceId = device.ID;
            // Stereo audio (left and right channel).
            defaultDevice.nChannels = 2;
            // Documentation said default was 0.
            defaultDevice.firstChannel = 0;
            // default device's sample rate.
            defaultSampRate = device.preferredSampleRate;

            std::cout << "\nDefault Device: " << device.ID
                << "\nDefault Name: " << device.name
                << "\nDefault Output Channels: " << device.outputChannels
                << " Default Input Channels: " << device.inputChannels
                << "\nDefault Is Default Input Device: " << device.isDefaultInput
                << "\nDefault Is Default Output Device: " << device.isDefaultOutput
                << "\nDefault Preferred Sample Rate: " << device.preferredSampleRate << "\n";
        }
    }

    // Setup glfw library and error check.
    if (glfwInit() == false)
    {
        std::cout << "\n\n---ERROR WITH INITIALIZING GLFW LIBRARY---\n\n";
        return 1;
    }

    // Creates a 640x480 window title AudioV, canvas for GUI.
    GLFWwindow* window = glfwCreateWindow(640, 480, "AudioV", NULL, NULL);

    if (window == false)
    {
        glfwTerminate();
        std::cout << "\n\n---ERROR WITH CREATING GLFW WINDOW---\n\n";
        return 1;
    }

    // Tells OpenGL what window to draw stuff to.
    glfwMakeContextCurrent(window);

    // Intialize GUI tools
    ImGui::CreateContext();
    // Attach GUI onto GLFW window for input detection
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // Run GUI in OpenGL3
    ImGui_ImplOpenGL3_Init();

    // Audio is captured at the default output device's sampling rate.
    // The passed in bufferFrames says sample into a buffer until it has
    // 1024 frames of data, or 2048 total samples (left and right).
    // The callback processes these 1024 frames and the RtAudio starts
    // collecting another 1024 frames.
    RtAudioErrorType err = audio.openStream(
        NULL,
        &defaultDevice,
        RTAUDIO_FLOAT32,
        defaultSampRate,
        &bufferFrames,
        audioLoopBack,
        dataFlow
    );

    if (err != RTAUDIO_NO_ERROR)
    {
        std::cout << "\n\n---ERROR WITH OPENING AUDIO STREAM---\n\n";
        return 1;
    }

    err = audio.startStream();

    if (err != RTAUDIO_NO_ERROR)
    {
        std::cout << "\n\n---ERROR WITH STARTING AUDIO STREAM---\n\n";
        return 1;
    }

    float* arrayPtr = dataFlow->data;
    int width = 0, height = 0;

    while (running == true && glfwWindowShouldClose(window) == false)
    {
        // Updates timing for GUI (prepare to draw).
        ImGui_ImplOpenGL3_NewFrame();
        // Polls input (tell me what happened).
        ImGui_ImplGlfw_NewFrame();
        // Initializes GUI state (ready to build the UI).
        ImGui::NewFrame();

        // Checks for inputs to the window.
        glfwPollEvents();
        // Sets clear color to black.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // Using the black colored clear, clear the screen.
        glClear(GL_COLOR_BUFFER_BIT);

        // Gets the current size of the window, allows user to resize.
        glfwGetFramebufferSize(window, &width, &height);

        // Set the GUI's starting position to be centered on the canvas.
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        // Then fill the GUI to the entire size of the canvas
        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));

        // Remove all padding from the GUI to the canvas. Utilizes the full canvas
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        // Set the frame background to be transparent
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        // Set the window background to  be transparent
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        // Set the line that plots the audio float values to green
        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0, 1, 0, 1));

        // Create the GUI window on the canvas
        // -- No title
        // -- User cannot drag edges of GUI window
        // -- Window cannot be moved on canvas
        // -- Window has no Background color (extra due to PushStyleColor)
        // -- Window cannot be scrolled on canvas
        ImGui::Begin("##main", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        // Plot the data arrays store information collected from the callback function.
        // Use sample size to read that many values from the array to plot at once.
        // PlotLines has built in wraparound for the array, similar to the modulo done
        // in the callback function, when it gets to the sampleSize, it resets back to 0
        // and continues reading from the start of the array until it reads all sampleSize samples.
        // Y-axis min and max, smaller number makes the graph more "reactive looking" to lower volumes
        // but distorts the graph. High values display a less "reactive looking" graph to lower volumes.
        // Finally, the window size to draw the graph in.
        ImGui::PlotLines("wave", arrayPtr, sampleSize, dataFlow->writeIndex, NULL, yAxisMin, yAxisMax, ImVec2((float)width, (float)height));

        // Close the GUI window and the styles we pushed earlier.
        ImGui::End();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        // Converts GUI commands to OpenGL draw calls.
        ImGui::Render();
        // Execute those draw calls.
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Display the frame we drew and swap. GLFW renders frames
        // via front and back buffer, front is displayed,
        // back is rendered while front is displayed. Swap to make updates.
        glfwSwapBuffers(window);
    }

    // Shutdown everything  
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    audio.stopStream();
    audio.closeStream();

    int status = remove("imgui.ini");

    if (status != 0)
    {
        perror("\nError deleting file");
    }

    else
    {
        std::cout << "\nFile successfully deleted\n";
    }

    return 0;
}

// Feeds default output device in as the input device to create a loopback.
// Output audio is now able to be quantized via the loopback.
int audioLoopBack(void*, void* inputBuffer, unsigned int numOfFrames, double, RtAudioStreamStatus, void* userData)
{
    if (inputBuffer == nullptr || userData == nullptr)
    {
        std::cout << "\n\n---ERROR WITH NULLPTR TO INPUTBUFFER OR DATA ARRAY---\n\n";
        return 1;
    }

    // Cast inputBuffer to float pointer.
    // these are the frames we collected via RtAudio's stream.
    float* sampleFrames = (float*)inputBuffer;

    // RingBuffer passes data to data array.
    // Used for visualization
    RingBuffer* dataFlow = (RingBuffer*)userData;

    // Loops for the max number of samples requested collected via
    // RtAudio's stream. There are two samples per frame (left & right).
    for (unsigned int i = 0; i < numOfFrames * 2; ++i)
    {
        // Adds the streamed inputBuffer values to the data array.
        // Wraps when array is full (writeIndex % sampleSize).
        dataFlow->data[dataFlow->writeIndex % sampleSize] = sampleFrames[i];

        // increment the index we are at in the data array
        ++dataFlow->writeIndex;
    }

    return 0;
}

BOOL WINAPI onConsoleClose(DWORD signal)
{
    if (signal == CTRL_CLOSE_EVENT || signal == CTRL_C_EVENT)
    {
        running = false;

        return TRUE;
    }

    return FALSE;
}
