#pragma once

#include <vulkan/vulkan.h>
#include "particle_system.h"
#include <string>
#include <vector>

class GraphicsPipeline {
public:
    void init(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
              ParticleSystem& particles, const std::string& shaderDir);
    void cleanup(VkDevice device);
    void recreate(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                  const std::string& shaderDir);

    void recordCommands(VkCommandBuffer cmd, ParticleSystem& particles, int currentFrame);
    VkDescriptorSet getDescriptorSet(int frame) const { return descriptorSets[frame]; }

private:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    void createDescriptorSetLayout(VkDevice device);
    void createPipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                        const std::string& shaderDir);
    void createDescriptorPool(VkDevice device);
    void createDescriptorSets(VkDevice device, ParticleSystem& particles);
};
