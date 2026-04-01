#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <random>

struct Particle {
    float posX, posY;
    float velX, velY;
    int32_t type;
    float _pad1, _pad2, _pad3;
};

struct SimParams {
    uint32_t particleCount;
    uint32_t numTypes;
    float deltaTime;
    float frictionFactor;
    float forceScale;
    float maxDistance;
    float minDistance;
    float repulsionStrength;
};

class ParticleSystem {
public:
    static constexpr uint32_t DEFAULT_PARTICLE_COUNT = 4000;
    static constexpr uint32_t DEFAULT_NUM_TYPES = 6;
    static constexpr uint32_t MAX_TYPES = 8;

    void init(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t particleCount = DEFAULT_PARTICLE_COUNT, uint32_t numTypes = DEFAULT_NUM_TYPES);
    void cleanup(VkDevice device);
    void randomizeAttractions();

    VkBuffer getBufferA() const { return bufferA; }
    VkBuffer getBufferB() const { return bufferB; }
    VkBuffer getAttractionBuffer() const { return attractionBuffer; }
    VkDeviceSize getBufferSize() const { return sizeof(Particle) * simParams.particleCount; }
    VkDeviceSize getAttractionBufferSize() const { return sizeof(float) * MAX_TYPES * MAX_TYPES; }

    SimParams& getSimParams() { return simParams; }
    const SimParams& getSimParams() const { return simParams; }
    uint32_t getParticleCount() const { return simParams.particleCount; }
    uint32_t getNumTypes() const { return simParams.numTypes; }

    // Attraction matrix access
    float* getAttractionMatrix() { return attractions; }
    float getAttraction(int a, int b) const { return attractions[a * MAX_TYPES + b]; }
    void setAttraction(int a, int b, float val) { attractions[a * MAX_TYPES + b] = val; }
    void uploadAttractionMatrix(VkDevice device);

    // Reinitialize particles (e.g. after changing count or types)
    void reinitialize(VkDevice device, VkPhysicalDevice physicalDevice);

    void swapBuffers() { std::swap(bufferA, bufferB); std::swap(memoryA, memoryB); }

private:
    SimParams simParams{};

    VkBuffer bufferA = VK_NULL_HANDLE;
    VkBuffer bufferB = VK_NULL_HANDLE;
    VkBuffer attractionBuffer = VK_NULL_HANDLE;
    VkDeviceMemory memoryA = VK_NULL_HANDLE;
    VkDeviceMemory memoryB = VK_NULL_HANDLE;
    VkDeviceMemory attractionMemory = VK_NULL_HANDLE;

    float attractions[MAX_TYPES * MAX_TYPES] = {};
    std::mt19937 rng{42};

    void initParticleData(VkDevice device);
    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                      VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer, VkDeviceMemory& memory);
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t filter, VkMemoryPropertyFlags props);
};
