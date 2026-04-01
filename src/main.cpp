#include "vulkan_context.h"
#include "particle_system.h"
#include "compute_pipeline.h"
#include "graphics_pipeline.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <cmath>

static constexpr int WIDTH = 1024;
static constexpr int HEIGHT = 1024;

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

static void initImGui(VulkanContext& ctx) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 3.0f;
    style.Alpha = 0.95f;

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

static void buildSettingsUI(ParticleSystem& particles, VulkanContext& ctx,
                            ComputePipeline& computePipe, GraphicsPipeline& graphicsPipe,
                            float fps, bool& needsReinit, bool& attractionDirty,
                            const std::string& shaderDir) {
    SimParams& p = particles.getSimParams();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Settings (Tab to toggle)");

    // FPS
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Particles: %u | Types: %u", p.particleCount, p.numTypes);
    ImGui::Separator();

    // Simulation parameters
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Time Step", &p.deltaTime, 0.001f, 0.1f, "%.3f");
        ImGui::SliderFloat("Friction", &p.frictionFactor, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Force Scale", &p.forceScale, 0.001f, 0.5f, "%.3f");
    }

    // Interaction parameters
    if (ImGui::CollapsingHeader("Interaction", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Max Distance", &p.maxDistance, 0.01f, 0.5f, "%.3f");
        ImGui::SliderFloat("Min Distance", &p.minDistance, 0.001f, p.maxDistance, "%.3f");
        ImGui::SliderFloat("Repulsion", &p.repulsionStrength, 0.0f, 5.0f, "%.2f");
    }

    // Particle settings
    if (ImGui::CollapsingHeader("Particles")) {
        int count = static_cast<int>(p.particleCount);
        int types = static_cast<int>(p.numTypes);

        if (ImGui::SliderInt("Count", &count, 100, 20000)) {
            p.particleCount = static_cast<uint32_t>(count);
            needsReinit = true;
        }
        if (ImGui::SliderInt("Types", &types, 1, static_cast<int>(ParticleSystem::MAX_TYPES))) {
            p.numTypes = static_cast<uint32_t>(types);
            needsReinit = true;
        }

        if (ImGui::Button("Randomize Attractions")) {
            particles.randomizeAttractions();
            attractionDirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Particles")) {
            needsReinit = true;
        }
    }

    // Attraction matrix
    if (ImGui::CollapsingHeader("Attraction Matrix")) {
        float* matrix = particles.getAttractionMatrix();
        uint32_t n = p.numTypes;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

        // Column headers
        ImGui::Text("     ");
        for (uint32_t j = 0; j < n; j++) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, TYPE_COLORS[j]);
            ImGui::Text("  %u  ", j);
            ImGui::PopStyleColor();
        }

        for (uint32_t i = 0; i < n; i++) {
            ImGui::PushStyleColor(ImGuiCol_Text, TYPE_COLORS[i]);
            ImGui::Text("  %u  ", i);
            ImGui::PopStyleColor();
            for (uint32_t j = 0; j < n; j++) {
                ImGui::SameLine();
                float& val = matrix[i * ParticleSystem::MAX_TYPES + j];

                // Color the slider based on value: red for negative, green for positive
                float t = (val + 1.0f) * 0.5f;
                ImVec4 col(1.0f - t, t, 0.2f, 0.7f);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, col);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(col.x, col.y, col.z, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(col.x, col.y, col.z, 1.0f));

                ImGui::PushItemWidth(36);
                char label[16];
                snprintf(label, sizeof(label), "##a%u%u", i, j);
                if (ImGui::SliderFloat(label, &val, -1.0f, 1.0f, "")) {
                    attractionDirty = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Type %u -> %u: %.2f", i, j, val);
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleColor(3);
            }
        }
        ImGui::PopStyleVar();
    }

    ImGui::End();
}

int main() {
    VulkanContext ctx;
    ParticleSystem particles;
    ComputePipeline computePipe;
    GraphicsPipeline graphicsPipe;

    std::string shaderDir = SHADER_DIR;

    try {
        ctx.init(WIDTH, HEIGHT, "Particle Life Engine");
        particles.init(ctx.getDevice(), ctx.getPhysicalDevice());
        computePipe.init(ctx.getDevice(), particles, shaderDir);
        graphicsPipe.init(ctx.getDevice(), ctx.getRenderPass(), ctx.getSwapchainExtent(),
                         particles, shaderDir);

        initImGui(ctx);

        int currentFrame = 0;
        bool showUI = true;
        bool tabWasPressed = false;

        auto lastTime = std::chrono::steady_clock::now();
        float fps = 0.0f;
        int frameCount = 0;

        while (!ctx.shouldClose()) {
            ctx.pollEvents();

            // Toggle UI with Tab
            bool tabDown = glfwGetKey(ctx.getWindow(), GLFW_KEY_TAB) == GLFW_PRESS;
            if (tabDown && !tabWasPressed) {
                showUI = !showUI;
            }
            tabWasPressed = tabDown;

            // FPS calculation
            frameCount++;
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - lastTime).count();
            if (elapsed >= 0.5f) {
                fps = static_cast<float>(frameCount) / elapsed;
                frameCount = 0;
                lastTime = now;
            }

            // ImGui new frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool needsReinit = false;
            bool attractionDirty = false;

            if (showUI) {
                buildSettingsUI(particles, ctx, computePipe, graphicsPipe,
                               fps, needsReinit, attractionDirty, shaderDir);
            }

            ImGui::Render();

            // Handle reinit if particle count/types changed
            if (needsReinit) {
                particles.reinitialize(ctx.getDevice(), ctx.getPhysicalDevice());
                computePipe.updateDescriptors(ctx.getDevice(), particles);
                // Recreate graphics descriptor sets by full reinit
                graphicsPipe.cleanup(ctx.getDevice());
                graphicsPipe.init(ctx.getDevice(), ctx.getRenderPass(), ctx.getSwapchainExtent(),
                                particles, shaderDir);
                continue;
            }

            if (attractionDirty) {
                particles.uploadAttractionMatrix(ctx.getDevice());
            }

            // Wait for compute fence
            VkFence computeFence = ctx.getComputeInFlightFence(currentFrame);
            vkWaitForFences(ctx.getDevice(), 1, &computeFence, VK_TRUE, UINT64_MAX);
            vkResetFences(ctx.getDevice(), 1, &computeFence);

            // Record compute commands
            VkCommandBuffer computeCmd = ctx.getComputeCommandBuffer(currentFrame);
            vkResetCommandBuffer(computeCmd, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            vkBeginCommandBuffer(computeCmd, &beginInfo);
            computePipe.recordCommands(computeCmd, particles, currentFrame);

            // Memory barrier: compute write -> vertex read
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

            // Submit compute
            VkSubmitInfo computeSubmit{};
            computeSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            computeSubmit.commandBufferCount = 1;
            computeSubmit.pCommandBuffers = &computeCmd;
            computeSubmit.signalSemaphoreCount = 1;
            VkSemaphore computeSignal = ctx.getComputeFinishedSemaphore(currentFrame);
            computeSubmit.pSignalSemaphores = &computeSignal;

            vkQueueSubmit(ctx.getComputeQueue(), 1, &computeSubmit,
                         ctx.getComputeInFlightFence(currentFrame));

            // Wait for graphics fence
            VkFence graphicsFence = ctx.getInFlightFence(currentFrame);
            vkWaitForFences(ctx.getDevice(), 1, &graphicsFence, VK_TRUE, UINT64_MAX);

            // Acquire swapchain image
            uint32_t imageIndex;
            VkResult result = vkAcquireNextImageKHR(ctx.getDevice(), ctx.getSwapchain(),
                UINT64_MAX, ctx.getImageAvailableSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                ctx.recreateSwapchain();
                graphicsPipe.recreate(ctx.getDevice(), ctx.getRenderPass(),
                                     ctx.getSwapchainExtent(), shaderDir);
                continue;
            }

            vkResetFences(ctx.getDevice(), 1, &graphicsFence);

            // Record graphics commands
            VkCommandBuffer graphicsCmd = ctx.getCommandBuffer(currentFrame);
            vkResetCommandBuffer(graphicsCmd, 0);

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

            // Render ImGui on top of particles
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), graphicsCmd);

            vkCmdEndRenderPass(graphicsCmd);
            vkEndCommandBuffer(graphicsCmd);

            // Submit graphics
            VkSubmitInfo graphicsSubmit{};
            graphicsSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = {
                ctx.getComputeFinishedSemaphore(currentFrame),
                ctx.getImageAvailableSemaphore(currentFrame)
            };
            VkPipelineStageFlags waitStages[] = {
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            };
            graphicsSubmit.waitSemaphoreCount = 2;
            graphicsSubmit.pWaitSemaphores = waitSemaphores;
            graphicsSubmit.pWaitDstStageMask = waitStages;
            graphicsSubmit.commandBufferCount = 1;
            graphicsSubmit.pCommandBuffers = &graphicsCmd;
            VkSemaphore renderSignal = ctx.getRenderFinishedSemaphore(currentFrame);
            graphicsSubmit.signalSemaphoreCount = 1;
            graphicsSubmit.pSignalSemaphores = &renderSignal;

            if (vkQueueSubmit(ctx.getGraphicsQueue(), 1, &graphicsSubmit,
                             ctx.getInFlightFence(currentFrame)) != VK_SUCCESS) {
                throw std::runtime_error("Failed to submit draw command buffer");
            }

            // Present
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &renderSignal;
            presentInfo.swapchainCount = 1;
            VkSwapchainKHR swapchains[] = { ctx.getSwapchain() };
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(ctx.getPresentQueue(), &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
                ctx.framebufferResized) {
                ctx.framebufferResized = false;
                ctx.recreateSwapchain();
                graphicsPipe.recreate(ctx.getDevice(), ctx.getRenderPass(),
                                     ctx.getSwapchainExtent(), shaderDir);
            }

            currentFrame = (currentFrame + 1) % VulkanContext::MAX_FRAMES_IN_FLIGHT;
        }

        vkDeviceWaitIdle(ctx.getDevice());

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        graphicsPipe.cleanup(ctx.getDevice());
        computePipe.cleanup(ctx.getDevice());
        particles.cleanup(ctx.getDevice());
        ctx.cleanup();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
