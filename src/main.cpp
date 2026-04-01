#include "vulkan_context.h"
#include "particle_system.h"
#include "compute_pipeline.h"
#include "graphics_pipeline.h"
#include <iostream>
#include <stdexcept>
#include <string>

static constexpr int WIDTH = 1024;
static constexpr int HEIGHT = 1024;

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

        int currentFrame = 0;

        while (!ctx.shouldClose()) {
            ctx.pollEvents();

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
