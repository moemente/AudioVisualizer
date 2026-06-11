#include "audioV.h"

int  main()
{
    RtAudio audio;

    auto dataFlow = std::make_unique<RingBuffer>();

    // Where the default device's values will be stored for RtAudio's loopback stream.
    RtAudio::StreamParameters defaultDevice;
    // store the default device's sample rate for ease of use.
    unsigned int defaultSampRate = 0;

    // Gets the number of audio devices connected to user's computer.
    unsigned int deviceCount = audio.getDeviceCount();
    // Errors if no devices are found
    if (deviceCount <= 0)
    {
        fprintf(stderr, "No audio devices found.");
        return 1;
    }

    // Stores all ids of connected devices to a vector to loop over.
    std::vector<unsigned int> ids = audio.getDeviceIds();
    for (unsigned int id : ids)
    {
        RtAudio::DeviceInfo device = audio.getDeviceInfo(id);
        // Save the values of the default output device for RtAudio's loopback stream.
        if (device.isDefaultOutput == true)
        {
            defaultDevice.deviceId = device.ID;
            // Stereo audio (left and right channel).
            defaultDevice.nChannels = 2;
            defaultDevice.firstChannel = 0;
            // Default device's sample rate.
            defaultSampRate = device.preferredSampleRate;
        }
    }

    //
    // --- GLFW SETUP ---
    //
    // Initializes GLFW, errors out if it fails.
    if (glfwInit() == false)
    {

        fprintf(stderr, "Error with initializing GLFW library.");
        return 1;
    }
    // Creates a 640x480 window titled AudioV, canvas for GUI.
    GLFWwindow* window = glfwCreateWindow(640, 480, "AudioV", NULL, NULL);
    // If window fails to create, errors out.
    if (window == nullptr)
    {
        glfwTerminate();
        fprintf(stderr, "Error with creating GLFW window.");
        return 1;
    }
    // Tells OpenGL what window to draw stuff to.
    glfwMakeContextCurrent(window);
    // Caps buffer swap to be monitor's refresh rate to prevent uneeded processing.
    glfwSwapInterval(1);

    //
    // --- IMGUI SETUP---
    //
    // Initializes ImGui.
    ImGui::CreateContext();
    // Attach GUI onto GLFW window for input detection.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // Run GUI in OpenGL3.
    ImGui_ImplOpenGL3_Init();


    //
    // --- RTAUDIO SETUP ---
    //
    /*
    Audio is captured at the default output device's sampling rate.
    bufferFrames tells RtAudio how many samples to collect before invoking the
    callback function. RtAudio calls audioLoopBack() and provides the bufferFrames
    as input. After the callback returns, RtAudio continues capturing the next chunk
    and repeats the process.
    */
    RtAudioErrorType err = audio.openStream(
        NULL,
        &defaultDevice,
        RTAUDIO_FLOAT32,
        defaultSampRate,
        &bufferFrames,
        audioLoopBack,
        dataFlow.get()
    );
    // Checks if opening stream has any error.
    if (err != RTAUDIO_NO_ERROR)
    {
        fprintf(stderr, "Error with opening RtAudio audio stream.");
        return 1;
    }
    // Tells RtAudio to begin processing data.
    err = audio.startStream();
    // Checks if starting stream has any error.
    if (err != RTAUDIO_NO_ERROR)
    {
        fprintf(stderr, "Error with starting audio stream.");
        return 1;
    }


    //
    // --- KISSFFT SETUP ---
    //
    // Initialize KissFFT with the context of how many samples are in 1 chunk of data.
    kiss_fft_cfg mycfg = kiss_fft_alloc(sampleSize, 0, NULL, NULL);


    //
    // --- MAIN RENDER LOOP ---
    //
    // Used for PushStyleColor.
    ImVec4 green = ImVec4(0, 1, 0, 1);
    ImVec4 transparent = ImVec4(0, 0, 0, 0);
    while (glfwWindowShouldClose(window) == false)
    {
        // Initializes UI for a new frame. Every frame is redrawn, so clear old state.
        ImGui_ImplOpenGL3_NewFrame();
        // Manage internal ImGui state.
        ImGui_ImplGlfw_NewFrame();
        // Starts a new cycle to allow the UI code to be written immediately.
        ImGui::NewFrame();
        // Checks for inputs to the window. Allows for resize and closing program.
        glfwPollEvents();
        // Sets clear color to black.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // Using the black colored clear, clear the screen.
        glClear(GL_COLOR_BUFFER_BIT);
        // Gets the current size of the window, allows user to resize.
        glfwGetFramebufferSize(window, &width, &height);
        // Set the GUI's starting position to be centered on the canvas.
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        // Fill the GUI to the entire size of the canvas
        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
        // Disable imgui.ini
        ImGui::GetIO().IniFilename = NULL;

        // Remove all padding from the GUI to the canvas. Utilizes the full canvas.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        // Set the frame background to be transparent
        ImGui::PushStyleColor(ImGuiCol_FrameBg, transparent);
        // Set the window background to  be transparent
        ImGui::PushStyleColor(ImGuiCol_WindowBg, transparent);
        // Set the line that plots the audio float values to green
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, green);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, green);
        ImGui::PushStyleColor(ImGuiCol_PlotLines, green);
        ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, green);
        // Disables built-in hover behavior
        ImGui::PushStyleColor(ImGuiCol_Text, transparent);
        // Renders background tooltip to  be transparent
        ImGui::PushStyleColor(ImGuiCol_PopupBg, transparent);
        // Renders border tooltip to be transparent
        ImGui::PushStyleColor(ImGuiCol_Border, transparent);

        /*
        Create the GUI window on the canvas. Prevent the user from dragging/scrolling
        the GUI window around on the canas
        */
        ImGui::Begin("##main", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );


        //
        // --- TOP GRAPH WAVEFORM (and fft spectrum initialization) ---
        //
        // We want the oldest sampled index in our dataFlow data array.
        unsigned int start = dataFlow->writeIndex.load() - sampleSize;
        /*
        Where we are going to store the most recent sampleSize amount of samples.
        This is what we will pass into
        */
        float orderedWave[sampleSize];
        /*
        kiss_fft_cpx is a complex number struct with .r(real) and .i(imaginary) numbers.
        Feed it the audio samples as the waveform. Real audio has no imaginary comps
        so .i = 0.0f.
        */
        kiss_fft_cpx fftIn[sampleSize];
        kiss_fft_cpx fftOut[sampleSize];
        /*
        What we will use to find the max height of the orderedWave signal.
        Used to normalize the wave so quiet and loud audio sources behave similarly.
        */
        float wavePeak = 0.001f;
        for (unsigned int i = 0; i < sampleSize; ++i)
        {
            /*
            Fills the orderedWave array with data gathered from the callback function.
            The index to the data array is wrapped around since start (writeIndex) never
            is reset. It increments indefinitely so it can be a num > sampleSize
            */
            float sample = dataFlow->data[(start + i) & (sampleSize - 1)];
            orderedWave[i] = sample;
            // Fills the fftIn .r and .i values at the same time.
            fftIn[i].r = sample;
            fftIn[i].i = 0.0f;
            // Treat negative float signals the same as positive
            float x = std::abs(orderedWave[i]);
            /*
            Checks to see if the float signal from the data array on this pass is our
            current peak. If so, update the peak.
            */
            if (x > wavePeak) 
            {
                wavePeak = x;
            }
        }
        for (unsigned int i = 0; i < sampleSize; ++i)
        {
            // After we find our peak and fill out orderedWave array, normalize the values.
            orderedWave[i] /= wavePeak;
        }
        /*
        Cap waveheight to whichever is smaller -- 50% of the window height, 
        or 33% of window width. The prevents the waveform from becoming too tall
        on a vertical window size. Keeping it looking naturally wide and short
        regardless of window shape.
        */
        float waveHeight = std::min((float)height * 0.5f, (float)width * 0.33f);
        // Same thing as waveform graph, but half the size
        float specHeight = std::min((float)height * 0.5f, (float)width / 2.0f);
        // Combined height of both waves on the screen.
        float totalHeight = waveHeight + specHeight;
        /*
        Normally, always displays from the bottom, but we want to always offset it
        to the centered of the rendered canvas. Great for vertical view
        */
        float topOffset = std::max(0.0f, ((float)height - totalHeight) / 2.0f);
        // Tells PlotLines to draw the waveform graph @ x = 0(left edge) y = topOffset
        ImGui::SetCursorPos(ImVec2(0, topOffset));
        // Draws the waveform using the normalized orderedWave using the scaled height
        ImGui::PlotLines(
            "##wave", 
            orderedWave, 
            sampleSize, 
            0, 
            NULL, 
            -1.0f, 
            1.0f, 
            ImVec2((float)width, waveHeight)
        );


        //
        // --- BOTTOM GRAPH FFT SPECTRUM ---
        //
        /*
        Runs the fft algorithm to take the float value samples and outputs sampleSize
        complex numbers representing the frequency content
        */
        kiss_fft(mycfg, fftIn, fftOut);
        /*
        Only half the values outputted by KissFFT are useful as the second half 
        is mirrored of the first. Magnitude tells us how loud a freq is at this moment.
        */
        float magnitude[sampleSize / 2];
        for (unsigned int i = 0; i < sampleSize / 2; ++i)
        {
            // Converts the complex number into a single "loudness" value.
            float sample = sqrtf(fftOut[i].r * fftOut[i].r + fftOut[i].i * fftOut[i].i);
            // Log scaling so mids/highs aren't crushed by loud lows.
            magnitude[i] = log1pf(sample);
        }
        // Similar to wavePeak
        static float magnitudePeak = 0.001f;
        // We want the spectrum histogram to decay slowly and not immediate.  
        float currentPeak = 0.0f;
        for (unsigned int i = 1; i < sampleSize / 2; ++i)
        {
            // Update currentPeak if the magnitude is greater.
            if (magnitude[i] > currentPeak) 
            {
                currentPeak = magnitude[i];
            }
        }
        // If the currentPeak is greater than out previous peak, update it.
        if (currentPeak > magnitudePeak) 
        {
            magnitudePeak = currentPeak;
        }
        // Otherwise, slowly decay the peak instead of setting it to 0.
        else
        {
            magnitudePeak *= 0.75f;
        }
        /*
        How large 1 fft bin is. Based on how many samples we can collect per second
        divided by the total size of samples we are displaying at once. Meaning,
        1 bin covers a range of hz. @ 48000hz and 1024 sampleSize, means 46.8hz per bin.
        */ 
        float binWidth = (float)defaultSampRate / (float)sampleSize;
        // Calc the number of bins to display, based on what actually can be heard.
        unsigned int displayBins = (unsigned int)(20000.0f / binWidth);
        // Don't let displayBins go beyond the needed number of bins (sampleSize / 2).
        if (displayBins > sampleSize / 2) 
        {
            displayBins = sampleSize / 2;
        }
        /*
        Tells PlotHistogram to draw the spectrum graph @ x = 0,
        y = topOffset + waveHeight. Right below the waveform graph.
        */
        ImGui::SetCursorPos(ImVec2(0, topOffset + waveHeight));
        /*
        Draws the spectrum with normalized magnitude, number of bins, scaled to magPeak.
        Skips the first 4 bins near DC noise (0hz garbage that makes edges look weird).
        */
        ImGui::PlotHistogram(
            "##spectrum", 
            magnitude + 4, 
            displayBins - 4, 
            0, 
            NULL, 
            0.0f, 
            magnitudePeak, 
            ImVec2((float)width, specHeight)
        );


        //
        // --- CLEANUP AND END ROUTINE PER FRAME ---
        //
        // End this frame's state.
        ImGui::End();
        // Pop all 9 pushed color styles and 1 pushed style var, restoring to default.
        ImGui::PopStyleColor(9);
        ImGui::PopStyleVar();
        // Converts GUI commands to OpenGL draw calls.
        ImGui::Render();
        // Execute those draw calls. Puts pixels on the screen.
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        /*
        Display the frame we drew and swap. GLFW renders frames via front and back 
        buffer, front is displayed, back is rendered while front is displayed. 
        Swap to make updates.
        */
        glfwSwapBuffers(window);
    }


    //
    // --- SHUTDOWN ---
    //
    // Cleans OpenGL side of ImGui allocation: Shaders, Buffers, textures...
    ImGui_ImplOpenGL3_Shutdown();
    // Cleans up ImGui's GLFW integration: Mouse and Keyboard hooks.
    ImGui_ImplGlfw_Shutdown();
    // Frees the core ImGui context that was created at the start.
    ImGui::DestroyContext();
    // Frees the core KissFFT context allocated at the start.
    kiss_fft_free(mycfg);
    // Frees any global state KissFFT allocated internally.
    kiss_fft_cleanup();
    // Tells RtAudio to stop capturing audio.
    audio.stopStream();
    // Releases audio device.
    audio.closeStream();

    return 0;
}


//
// --- CALLBACK FUNCTION ---
//
int audioLoopBack(void*, void* inputBuffer, unsigned int numOfFrames, double, RtAudioStreamStatus, void* userData)
{
    /*
    These are the frames we collected via RtAudio's stream. RtAudio returns a void*
    so we must cast to float to extract data from it.
    */
    float* sampleFrames = (float*)inputBuffer;
    // Where we are going to write the audio stream frames to from RtAudio.
    RingBuffer* dataFlow = (RingBuffer*)userData;
    /*
    Loops for samples requested to RtAudio's stream. Two samples per frame (L & R).
    We are averaging the L and R sample together to get a monostream of audio.
    So if we need 1024 mono samples, we actually need 2048 samples.
    */
    for (unsigned int i = 0; i < numOfFrames * 2; i += 2)
    {
        // Copies the streamed inputBuffer values to the data array.
        float monoSample = (sampleFrames[i] + sampleFrames[i + 1]) / 2.0f;
        // Ensure writeIndex loads with atomic constraints and wraps around.
        dataFlow->data[dataFlow->writeIndex.load() & (sampleSize - 1)] = monoSample;
        // Increment the index we are at in the data array. Follows atomic restrictions.
        dataFlow->writeIndex++;
    }

    return 0;
}
