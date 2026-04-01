#pragma once

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <optional>
#include <functional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapchainDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanContext {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    void init(int width, int height, const std::string& title);
    void cleanup();
    bool shouldClose() const;
    void pollEvents() const;

    // Accessors
    GLFWwindow* getWindow() const { return window; }
    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getComputeQueue() const { return computeQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    VkSwapchainKHR getSwapchain() const { return swapchain; }
    VkFormat getSwapchainFormat() const { return swapchainFormat; }
    VkExtent2D getSwapchainExtent() const { return swapchainExtent; }
    VkRenderPass getRenderPass() const { return renderPass; }
    const std::vector<VkFramebuffer>& getFramebuffers() const { return framebuffers; }
    const std::vector<VkImageView>& getSwapchainImageViews() const { return swapchainImageViews; }
    const QueueFamilyIndices& getQueueFamilies() const { return queueFamilies; }
    VkCommandPool getCommandPool() const { return commandPool; }

    // Frame sync
    VkSemaphore getImageAvailableSemaphore(int frame) const { return imageAvailableSemaphores[frame]; }
    VkSemaphore getRenderFinishedSemaphore(int frame) const { return renderFinishedSemaphores[frame]; }
    VkFence getInFlightFence(int frame) const { return inFlightFences[frame]; }
    VkSemaphore getComputeFinishedSemaphore(int frame) const { return computeFinishedSemaphores[frame]; }
    VkFence getComputeInFlightFence(int frame) const { return computeInFlightFences[frame]; }

    VkCommandBuffer getCommandBuffer(int frame) const { return commandBuffers[frame]; }
    VkCommandBuffer getComputeCommandBuffer(int frame) const { return computeCommandBuffers[frame]; }

    void recreateSwapchain();
    bool framebufferResized = false;

private:
    GLFWwindow* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    QueueFamilyIndices queueFamilies;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkSemaphore> computeFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> computeInFlightFences;

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    void cleanupSwapchain();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapchainDetails querySwapchainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
