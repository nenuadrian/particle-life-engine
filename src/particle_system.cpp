#include "particle_system.h"
#include <stdexcept>
#include <cstring>

void ParticleSystem::init(VkDevice device, VkPhysicalDevice physicalDevice,
                          uint32_t count, uint32_t types) {
    simParams.particleCount = count;
    simParams.numTypes = types;
    simParams.deltaTime = 0.016f;
    simParams.frictionFactor = 0.5f;
    simParams.forceScale = 0.05f;
    simParams.maxDistance = 0.15f;
    simParams.minDistance = 0.02f;
    simParams.repulsionStrength = 1.0f;

    VkDeviceSize particleSize = sizeof(Particle) * simParams.particleCount;
    VkBufferUsageFlags ssboUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    createBuffer(device, physicalDevice, particleSize, ssboUsage, memProps, bufferA, memoryA);
    createBuffer(device, physicalDevice, particleSize, ssboUsage, memProps, bufferB, memoryB);
    createBuffer(device, physicalDevice, getAttractionBufferSize(),
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memProps, attractionBuffer, attractionMemory);

    // Initialize particles
    initParticleData(device);

    randomizeAttractions();
    uploadAttractionMatrix(device);
}

void ParticleSystem::initParticleData(VkDevice device) {
    std::vector<Particle> particles(simParams.particleCount);
    std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> typeDist(0, static_cast<int>(simParams.numTypes) - 1);

    for (auto& p : particles) {
        p.posX = posDist(rng);
        p.posY = posDist(rng);
        p.velX = 0.0f;
        p.velY = 0.0f;
        p.type = typeDist(rng);
        p._pad1 = p._pad2 = p._pad3 = 0.0f;
    }

    VkDeviceSize particleSize = sizeof(Particle) * simParams.particleCount;
    void* data;
    vkMapMemory(device, memoryA, 0, particleSize, 0, &data);
    memcpy(data, particles.data(), particleSize);
    vkUnmapMemory(device, memoryA);

    vkMapMemory(device, memoryB, 0, particleSize, 0, &data);
    memcpy(data, particles.data(), particleSize);
    vkUnmapMemory(device, memoryB);
}

void ParticleSystem::cleanup(VkDevice device) {
    vkDestroyBuffer(device, bufferA, nullptr);
    vkDestroyBuffer(device, bufferB, nullptr);
    vkDestroyBuffer(device, attractionBuffer, nullptr);
    vkFreeMemory(device, memoryA, nullptr);
    vkFreeMemory(device, memoryB, nullptr);
    vkFreeMemory(device, attractionMemory, nullptr);
}

void ParticleSystem::randomizeAttractions() {
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (uint32_t i = 0; i < MAX_TYPES * MAX_TYPES; i++) {
        attractions[i] = dist(rng);
    }
}

void ParticleSystem::uploadAttractionMatrix(VkDevice device) {
    void* data;
    vkMapMemory(device, attractionMemory, 0, getAttractionBufferSize(), 0, &data);
    memcpy(data, attractions, getAttractionBufferSize());
    vkUnmapMemory(device, attractionMemory);
}

void ParticleSystem::reinitialize(VkDevice device, VkPhysicalDevice physicalDevice) {
    vkDeviceWaitIdle(device);

    // Destroy old particle buffers
    vkDestroyBuffer(device, bufferA, nullptr);
    vkDestroyBuffer(device, bufferB, nullptr);
    vkFreeMemory(device, memoryA, nullptr);
    vkFreeMemory(device, memoryB, nullptr);

    VkDeviceSize particleSize = sizeof(Particle) * simParams.particleCount;
    VkBufferUsageFlags ssboUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    createBuffer(device, physicalDevice, particleSize, ssboUsage, memProps, bufferA, memoryA);
    createBuffer(device, physicalDevice, particleSize, ssboUsage, memProps, bufferB, memoryB);

    initParticleData(device);
    uploadAttractionMatrix(device);
}

void ParticleSystem::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                                  VkDeviceSize size, VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer& buffer, VkDeviceMemory& memory) {
    VkBufferCreateInfo bufInfo{};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = size;
    bufInfo.usage = usage;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(device, buffer, memory, 0);
}

uint32_t ParticleSystem::findMemoryType(VkPhysicalDevice physicalDevice,
                                         uint32_t filter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}
