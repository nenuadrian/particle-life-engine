#pragma once

#include <vulkan/vulkan.h>
#include "particle_system.h"

class ComputePipeline {
public:
    void init(VkDevice device, ParticleSystem& particles, const std::string& shaderDir);
    void cleanup(VkDevice device);

    void recordCommands(VkCommandBuffer cmd, ParticleSystem& particles, int currentFrame);
    VkDescriptorSet getDescriptorSet(int frame) const { return descriptorSets[frame]; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

    void updateDescriptors(VkDevice device, ParticleSystem& particles);

private:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    void createDescriptorSetLayout(VkDevice device);
    void createPipeline(VkDevice device, const std::string& shaderDir);
    void createDescriptorPool(VkDevice device);
    void createDescriptorSets(VkDevice device, ParticleSystem& particles);
};
