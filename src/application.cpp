#include "application.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>

static constexpr float DEFAULT_ZOOM = 1.0f;

static const ImVec4 TYPE_COLORS[ParticleSystem::MAX_TYPES] = {
    {1.0f, 0.2f, 0.2f, 1.0f},
    {0.2f, 1.0f, 0.2f, 1.0f},
    {0.3f, 0.5f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.2f, 1.0f},
    {1.0f, 0.2f, 1.0f, 1.0f},
    {0.2f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.6f, 0.2f, 1.0f},
    {0.8f, 0.8f, 0.8f, 1.0f},
};

namespace
{
constexpr ImVec4 ACCENT_COLOR(0.35f, 0.68f, 1.0f, 1.0f);
constexpr ImVec4 SUCCESS_COLOR(0.34f, 0.82f, 0.58f, 1.0f);
constexpr ImVec4 WARNING_COLOR(0.95f, 0.66f, 0.24f, 1.0f);
constexpr ImVec4 TEXT_COLOR(0.93f, 0.95f, 0.99f, 1.0f);
constexpr ImVec4 TEXT_MUTED_COLOR(0.54f, 0.58f, 0.66f, 1.0f);
constexpr ImVec4 SURFACE_COLOR(0.10f, 0.12f, 0.16f, 1.0f);
constexpr ImVec4 SURFACE_ALT_COLOR(0.13f, 0.15f, 0.20f, 1.0f);

ImVec4 withAlpha(const ImVec4 &color, float alpha)
{
    return ImVec4(color.x, color.y, color.z, alpha);
}

ImVec4 lerpColor(const ImVec4 &a, const ImVec4 &b, float t)
{
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t);
}

ImVec4 attractionCellColor(float value)
{
    const ImVec4 repelColor(0.90f, 0.33f, 0.40f, 1.0f);
    const ImVec4 neutralColor(0.15f, 0.18f, 0.23f, 1.0f);
    const ImVec4 attractColor(0.25f, 0.80f, 0.63f, 1.0f);

    return value < 0.0f
               ? lerpColor(neutralColor, repelColor, -value)
               : lerpColor(neutralColor, attractColor, value);
}

void applyModernDarkTheme()
{
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.55f;
    style.WindowPadding = ImVec2(18.0f, 16.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.CellPadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 10.0f;
    style.TabRounding = 12.0f;
    style.WindowTitleAlign = ImVec2(0.03f, 0.5f);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_Text] = TEXT_COLOR;
    colors[ImGuiCol_TextDisabled] = TEXT_MUTED_COLOR;
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.08f, 0.11f, 0.96f);
    colors[ImGuiCol_ChildBg] = SURFACE_COLOR;
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.10f, 0.13f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.22f, 0.28f, 0.90f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg] = SURFACE_ALT_COLOR;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.22f, 0.29f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.25f, 0.33f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.08f, 0.11f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.14f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.10f, 0.13f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = withAlpha(SURFACE_COLOR, 0.35f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.31f, 0.39f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.34f, 0.39f, 0.49f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.46f, 0.57f, 1.0f);
    colors[ImGuiCol_CheckMark] = ACCENT_COLOR;
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.74f, 1.0f, 0.85f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.63f, 0.82f, 1.0f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.22f, 0.31f, 0.95f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.32f, 0.45f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.39f, 0.54f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.12f, 0.16f, 0.22f, 0.96f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.24f, 0.33f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.29f, 0.40f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.31f, 0.90f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.37f, 0.55f, 0.83f, 1.0f);
    colors[ImGuiCol_SeparatorActive] = ACCENT_COLOR;
    colors[ImGuiCol_ResizeGrip] = withAlpha(ACCENT_COLOR, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = withAlpha(ACCENT_COLOR, 0.55f);
    colors[ImGuiCol_ResizeGripActive] = withAlpha(ACCENT_COLOR, 0.90f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.14f, 0.18f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.21f, 0.30f, 0.42f, 1.0f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.16f, 0.24f, 0.35f, 1.0f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.10f, 0.12f, 0.16f, 1.0f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.18f, 0.25f, 1.0f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.10f, 0.13f, 0.17f, 1.0f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.22f, 0.28f, 0.90f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.15f, 0.19f, 0.24f, 0.60f);
    colors[ImGuiCol_TableRowBg] = withAlpha(SURFACE_COLOR, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = withAlpha(SURFACE_ALT_COLOR, 0.22f);
}

void drawMetricCard(const char *id, const char *label, const char *value, const char *hint, const ImVec4 &tint, float width)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, withAlpha(tint, 0.08f));
    ImGui::PushStyleColor(ImGuiCol_Border, withAlpha(tint, 0.28f));

    ImGui::BeginChild(id, ImVec2(width, 96.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushStyleColor(ImGuiCol_Text, withAlpha(tint, 0.92f));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::TextUnformatted(value);
    ImGui::TextDisabled("%s", hint);
    ImGui::EndChild();

    ImGui::PopStyleColor(2);
}
}

void Application::applyZoomSteps(float steps)
{
    zoom = math_utils::applyZoomSteps(zoom, steps);
}

// --- Initialization ---

void Application::init(int width, int height, const std::string &title)
{
    shaderDir = SHADER_DIR;
    zoom = DEFAULT_ZOOM;

    ctx.init(width, height, title);
    particles.init(ctx.getDevice(), ctx.getPhysicalDevice());
    computePipe.init(ctx.getDevice(), particles, shaderDir);
    graphicsPipe.init(ctx.getDevice(), ctx.getRenderPass(), ctx.getSwapchainExtent(),
                      particles, shaderDir);
    initImGui();

    lastFPSTime = std::chrono::steady_clock::now();
}

void Application::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    applyModernDarkTheme();

    ImGui_ImplGlfw_InitForVulkan(ctx.getWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = ctx.getInstance();
    initInfo.PhysicalDevice = ctx.getPhysicalDevice();
    initInfo.Device = ctx.getDevice();
    initInfo.QueueFamily = ctx.getQueueFamilies().graphicsFamily.value();
    initInfo.Queue = ctx.getGraphicsQueue();
    initInfo.DescriptorPool = ctx.getImGuiDescriptorPool();
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = ctx.getSwapchainImageCount();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass = ctx.getRenderPass();
    initInfo.Subpass = 0;

    ImGui_ImplVulkan_Init(&initInfo);
    ImGui_ImplVulkan_CreateFontsTexture();
}

// --- Main loop ---

void Application::run()
{
    while (!ctx.shouldClose())
    {
        ctx.pollEvents();
        processInput();
        updateFPS();

        // Begin ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Mouse wheel zoom (outside ImGui windows)
        ImGuiIO &io = ImGui::GetIO();
        if (!io.WantCaptureMouse && io.MouseWheel != 0.0f)
        {
            applyZoomSteps(io.MouseWheel);
        }

        needsReinit = false;
        attractionDirty = false;

        if (showUI)
        {
            buildUI();
        }

        particles.setWorldSize(ctx.getDevice(), math_utils::worldSizeForZoom(zoom));
        ImGui::Render();

        if (needsReinit)
        {
            handleReinit();
            continue;
        }

        if (attractionDirty)
        {
            particles.uploadAttractionMatrix(ctx.getDevice());
        }

        submitCompute();

        // Acquire swapchain image
        VkFence graphicsFence = ctx.getInFlightFence(currentFrame);
        vkWaitForFences(ctx.getDevice(), 1, &graphicsFence, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(ctx.getDevice(), ctx.getSwapchain(),
                                                UINT64_MAX, ctx.getImageAvailableSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ctx.recreateSwapchain();
            graphicsPipe.recreate(ctx.getDevice(), ctx.getRenderPass(),
                                  ctx.getSwapchainExtent(), shaderDir);
            continue;
        }

        vkResetFences(ctx.getDevice(), 1, &graphicsFence);

        submitGraphics(imageIndex);

        VkSemaphore renderSignal = ctx.getRenderFinishedSemaphore(currentFrame);
        present(imageIndex, renderSignal);

        currentFrame = (currentFrame + 1) % VulkanContext::MAX_FRAMES_IN_FLIGHT;
    }

    vkDeviceWaitIdle(ctx.getDevice());
}

// --- Input handling ---

void Application::processInput()
{
    GLFWwindow *win = ctx.getWindow();

    bool tabDown = glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabDown && !tabWasPressed)
    {
        showUI = !showUI;
    }
    tabWasPressed = tabDown;

    ImGuiIO &io = ImGui::GetIO();
    bool zoomInDown = glfwGetKey(win, GLFW_KEY_EQUAL) == GLFW_PRESS ||
                      glfwGetKey(win, GLFW_KEY_KP_ADD) == GLFW_PRESS;
    bool zoomOutDown = glfwGetKey(win, GLFW_KEY_MINUS) == GLFW_PRESS ||
                       glfwGetKey(win, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;

    if (!io.WantCaptureKeyboard)
    {
        if (zoomInDown && !zoomInWasPressed)
            applyZoomSteps(1.0f);
        if (zoomOutDown && !zoomOutWasPressed)
            applyZoomSteps(-1.0f);
    }

    zoomInWasPressed = zoomInDown;
    zoomOutWasPressed = zoomOutDown;
}

void Application::updateFPS()
{
    frameCount++;
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastFPSTime).count();
    if (elapsed >= 0.5f)
    {
        fps = static_cast<float>(frameCount) / elapsed;
        frameCount = 0;
        lastFPSTime = now;
    }
}

// --- UI ---

void Application::buildUI()
{
    SimParams &p = particles.getSimParams();
    const float worldSize = math_utils::worldSizeForZoom(zoom);
    const ImGuiTreeNodeFlags defaultSectionFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
    const ImGuiTreeNodeFlags sectionFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.96f);

    if (!ImGui::Begin("SettingsPanel", nullptr, ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::End();
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ACCENT_COLOR);
    ImGui::TextUnformatted("Particle Life");
    ImGui::PopStyleColor();
    ImGui::TextDisabled("Tab toggles this panel. Mouse wheel or +/- changes zoom.");
    ImGui::Spacing();

    char fpsValue[32];
    std::snprintf(fpsValue, sizeof(fpsValue), "%.1f FPS", fps);
    const char *fpsHint = fps >= 60.0f ? "Rendering is smooth" : "Workload is climbing";

    char viewValue[32];
    std::snprintf(viewValue, sizeof(viewValue), "%.2fx zoom", zoom);
    char viewHint[48];
    std::snprintf(viewHint, sizeof(viewHint), "Arena %.2f x %.2f", worldSize, worldSize);

    const float cardGap = ImGui::GetStyle().ItemSpacing.x;
    const float cardWidth = (ImGui::GetContentRegionAvail().x - cardGap) * 0.5f;

    drawMetricCard("PerfCard", "Frame Rate", fpsValue, fpsHint, ACCENT_COLOR, cardWidth);
    ImGui::SameLine();
    drawMetricCard("ViewCard", "View Scale", viewValue, viewHint, SUCCESS_COLOR, cardWidth);

    ImGui::SeparatorText("Controls");

    if (ImGui::CollapsingHeader("Simulation", defaultSectionFlags))
    {
        ImGui::SliderFloat("Time Step", &p.deltaTime, 0.001f, 0.1f, "%.3f");
        ImGui::SliderFloat("Friction", &p.frictionFactor, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Force Scale", &p.forceScale, 0.001f, 0.5f, "%.3f");
    }

    if (ImGui::CollapsingHeader("View", defaultSectionFlags))
    {
        ImGui::SliderFloat("Zoom", &zoom, math_utils::MIN_ZOOM, math_utils::MAX_ZOOM, "%.2fx", ImGuiSliderFlags_Logarithmic);
        zoom = math_utils::clampZoom(zoom);
        ImGui::SameLine();
        if (ImGui::Button("Reset Zoom"))
        {
            zoom = DEFAULT_ZOOM;
        }
        ImGui::TextDisabled("Mouse wheel or +/- keys");
        ImGui::Text("Arena: %.2f x %.2f", math_utils::worldSizeForZoom(zoom), math_utils::worldSizeForZoom(zoom));
    }

    if (ImGui::CollapsingHeader("Interaction", defaultSectionFlags))
    {
        ImGui::SliderFloat("Max Distance", &p.maxDistance, 0.01f, 0.5f, "%.3f");
        ImGui::SliderFloat("Min Distance", &p.minDistance, 0.001f, p.maxDistance, "%.3f");
        ImGui::SliderFloat("Repulsion", &p.repulsionStrength, 0.0f, 5.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Particles", sectionFlags))
    {
        int count = static_cast<int>(p.particleCount);
        int types = static_cast<int>(p.numTypes);

        if (ImGui::SliderInt("Count", &count, 100, static_cast<int>(ParticleSystem::MAX_PARTICLE_COUNT)))
        {
            p.particleCount = static_cast<uint32_t>(count);
            needsReinit = true;
        }
        if (ImGui::SliderInt("Types", &types, 1, static_cast<int>(ParticleSystem::MAX_TYPES)))
        {
            p.numTypes = static_cast<uint32_t>(types);
            needsReinit = true;
        }

        const float actionWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

        if (ImGui::Button("Randomize Attractions", ImVec2(actionWidth, 0.0f)))
        {
            particles.randomizeAttractions();
            attractionDirty = true;
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, withAlpha(WARNING_COLOR, 0.20f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, withAlpha(WARNING_COLOR, 0.34f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, withAlpha(WARNING_COLOR, 0.48f));
        if (ImGui::Button("Reset Particles", ImVec2(actionWidth, 0.0f)))
        {
            needsReinit = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::TextDisabled("Higher counts are slower because the compute pass is O(n^2).");
    }

    if (ImGui::CollapsingHeader("Attraction Matrix", sectionFlags))
    {
        float *matrix = particles.getAttractionMatrix();
        uint32_t n = p.numTypes;

        ImGui::TextDisabled("Negative values repel. Positive values attract.");

        const float matrixHeight = std::min(340.0f, 52.0f + static_cast<float>(n) * 34.0f);
        if (ImGui::BeginChild("AttractionMatrixPanel", ImVec2(0.0f, matrixHeight), ImGuiChildFlags_Borders))
        {
            const ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit |
                                               ImGuiTableFlags_BordersInnerH |
                                               ImGuiTableFlags_BordersInnerV |
                                               ImGuiTableFlags_RowBg;

            if (ImGui::BeginTable("AttractionMatrixTable", static_cast<int>(n) + 1, tableFlags))
            {
                ImGui::TableSetupColumn("##row", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                for (uint32_t j = 0; j < n; ++j)
                {
                    char columnLabel[8];
                    std::snprintf(columnLabel, sizeof(columnLabel), "%u", j);
                    ImGui::TableSetupColumn(columnLabel, ImGuiTableColumnFlags_WidthFixed, 44.0f);
                }

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("");
                for (uint32_t j = 0; j < n; ++j)
                {
                    ImGui::TableSetColumnIndex(static_cast<int>(j) + 1);
                    ImGui::PushStyleColor(ImGuiCol_Text, TYPE_COLORS[j]);
                    ImGui::Text("%u", j);
                    ImGui::PopStyleColor();
                }

                for (uint32_t i = 0; i < n; ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushStyleColor(ImGuiCol_Text, TYPE_COLORS[i]);
                    ImGui::Text("%u", i);
                    ImGui::PopStyleColor();

                    for (uint32_t j = 0; j < n; ++j)
                    {
                        ImGui::TableSetColumnIndex(static_cast<int>(j) + 1);
                        float &val = matrix[i * ParticleSystem::MAX_TYPES + j];
                        ImVec4 cellColor = attractionCellColor(val);

                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, withAlpha(cellColor, 0.52f));
                        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, withAlpha(cellColor, 0.72f));
                        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, withAlpha(cellColor, 0.88f));
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, withAlpha(TEXT_COLOR, 0.40f));
                        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, TEXT_COLOR);

                        ImGui::PushItemWidth(-1.0f);
                        char label[16];
                        std::snprintf(label, sizeof(label), "##a%u%u", i, j);
                        if (ImGui::SliderFloat(label, &val, -1.0f, 1.0f, ""))
                        {
                            attractionDirty = true;
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("Type %u -> %u: %.2f", i, j, val);
                        }
                        ImGui::PopItemWidth();
                        ImGui::PopStyleColor(5);
                        ImGui::PopStyleVar();
                    }
                }

                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
    }

    ImGui::End();
}

// --- Reinit ---

void Application::handleReinit()
{
    particles.reinitialize(ctx.getDevice(), ctx.getPhysicalDevice());
    computePipe.updateDescriptors(ctx.getDevice(), particles);
    graphicsPipe.cleanup(ctx.getDevice());
    graphicsPipe.init(ctx.getDevice(), ctx.getRenderPass(), ctx.getSwapchainExtent(),
                      particles, shaderDir);
}

// --- Compute pass ---

void Application::submitCompute()
{
    VkFence computeFence = ctx.getComputeInFlightFence(currentFrame);
    vkWaitForFences(ctx.getDevice(), 1, &computeFence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx.getDevice(), 1, &computeFence);

    VkCommandBuffer computeCmd = ctx.getComputeCommandBuffer(currentFrame);
    vkResetCommandBuffer(computeCmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(computeCmd, &beginInfo);
    computePipe.recordCommands(computeCmd, particles, currentFrame);

    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.buffer = (currentFrame % 2 == 0) ? particles.getBufferB() : particles.getBufferA();
    barrier.size = particles.getBufferSize();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(computeCmd,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                         0, 0, nullptr, 1, &barrier, 0, nullptr);

    vkEndCommandBuffer(computeCmd);

    VkSubmitInfo computeSubmit{};
    computeSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmit.commandBufferCount = 1;
    computeSubmit.pCommandBuffers = &computeCmd;
    computeSubmit.signalSemaphoreCount = 1;
    VkSemaphore computeSignal = ctx.getComputeFinishedSemaphore(currentFrame);
    computeSubmit.pSignalSemaphores = &computeSignal;

    vkQueueSubmit(ctx.getComputeQueue(), 1, &computeSubmit,
                  ctx.getComputeInFlightFence(currentFrame));
}

// --- Graphics pass ---

void Application::submitGraphics(uint32_t imageIndex)
{
    VkCommandBuffer graphicsCmd = ctx.getCommandBuffer(currentFrame);
    vkResetCommandBuffer(graphicsCmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(graphicsCmd, &beginInfo);

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = ctx.getRenderPass();
    rpBegin.framebuffer = ctx.getFramebuffers()[imageIndex];
    rpBegin.renderArea.extent = ctx.getSwapchainExtent();
    VkClearValue clearColor = {{{0.02f, 0.02f, 0.05f, 1.0f}}};
    rpBegin.clearValueCount = 1;
    rpBegin.pClearValues = &clearColor;

    vkCmdBeginRenderPass(graphicsCmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    graphicsPipe.recordCommands(graphicsCmd, particles, currentFrame);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), graphicsCmd);
    vkCmdEndRenderPass(graphicsCmd);
    vkEndCommandBuffer(graphicsCmd);

    // Submit
    VkSemaphore waitSemaphores[] = {
        ctx.getComputeFinishedSemaphore(currentFrame),
        ctx.getImageAvailableSemaphore(currentFrame)};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo graphicsSubmit{};
    graphicsSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    graphicsSubmit.waitSemaphoreCount = 2;
    graphicsSubmit.pWaitSemaphores = waitSemaphores;
    graphicsSubmit.pWaitDstStageMask = waitStages;
    graphicsSubmit.commandBufferCount = 1;
    graphicsSubmit.pCommandBuffers = &graphicsCmd;
    VkSemaphore renderSignal = ctx.getRenderFinishedSemaphore(currentFrame);
    graphicsSubmit.signalSemaphoreCount = 1;
    graphicsSubmit.pSignalSemaphores = &renderSignal;

    if (vkQueueSubmit(ctx.getGraphicsQueue(), 1, &graphicsSubmit,
                      ctx.getInFlightFence(currentFrame)) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer");
    }
}

// --- Present ---

void Application::present(uint32_t imageIndex, VkSemaphore renderSignal)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderSignal;
    presentInfo.swapchainCount = 1;
    VkSwapchainKHR swapchains[] = {ctx.getSwapchain()};
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(ctx.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        ctx.framebufferResized)
    {
        ctx.framebufferResized = false;
        ctx.recreateSwapchain();
        graphicsPipe.recreate(ctx.getDevice(), ctx.getRenderPass(),
                              ctx.getSwapchainExtent(), shaderDir);
    }
}

// --- Cleanup ---

void Application::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    graphicsPipe.cleanup(ctx.getDevice());
    computePipe.cleanup(ctx.getDevice());
    particles.cleanup(ctx.getDevice());
    ctx.cleanup();
}
