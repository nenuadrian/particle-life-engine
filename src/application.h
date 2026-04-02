#pragma once

#include "vulkan_context.h"
#include "particle_system.h"
#include "compute_pipeline.h"
#include "graphics_pipeline.h"
#include "math_utils.h"

#include <string>
#include <chrono>

class Application {
public:
    void init(int width, int height, const std::string& title);
    void run();
    void cleanup();

private:
    // Core systems
    VulkanContext ctx;
    ParticleSystem particles;
    ComputePipeline computePipe;
    GraphicsPipeline graphicsPipe;
    std::string shaderDir;

    // Frame state
    int currentFrame = 0;
    bool showUI = true;
    float zoom = 1.0f;

    // Input edge-detection
    bool tabWasPressed = false;
    bool zoomInWasPressed = false;
    bool zoomOutWasPressed = false;

    // FPS tracking
    std::chrono::steady_clock::time_point lastFPSTime;
    float fps = 0.0f;
    int frameCount = 0;

    // Per-frame flags set by UI
    bool needsReinit = false;
    bool attractionDirty = false;

    // Initialization helpers
    void initImGui();

    // Main loop phases
    void processInput();
    void updateFPS();
    void buildUI();
    void handleReinit();
    void submitCompute();
    void submitGraphics(uint32_t imageIndex);
    void present(uint32_t imageIndex, VkSemaphore renderSignal);

    // Zoom helper (mutates member zoom)
    void applyZoomSteps(float steps);
};
