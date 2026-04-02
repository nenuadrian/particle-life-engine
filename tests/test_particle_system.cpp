#include <gtest/gtest.h>
#include "particle_system.h"

// --- Struct layout (must match GPU expectations) ---

TEST(ParticleStruct, SizeIs32Bytes)
{
    EXPECT_EQ(sizeof(Particle), 32u);
}

TEST(ParticleStruct, FieldOffsets)
{
    // Verify fields are at expected offsets for GPU SSBO layout
    EXPECT_EQ(offsetof(Particle, posX), 0u);
    EXPECT_EQ(offsetof(Particle, posY), 4u);
    EXPECT_EQ(offsetof(Particle, velX), 8u);
    EXPECT_EQ(offsetof(Particle, velY), 12u);
    EXPECT_EQ(offsetof(Particle, type), 16u);
}

// --- SimParams defaults ---

TEST(SimParams, DefaultConstruction)
{
    SimParams p{};
    EXPECT_EQ(p.particleCount, 0u);
    EXPECT_EQ(p.numTypes, 0u);
    EXPECT_FLOAT_EQ(p.deltaTime, 0.0f);
    EXPECT_FLOAT_EQ(p.worldSize, 0.0f);
}

// --- Constants ---

TEST(ParticleSystemConstants, Limits)
{
    EXPECT_EQ(ParticleSystem::MAX_TYPES, 8u);
    EXPECT_EQ(ParticleSystem::MAX_PARTICLE_COUNT, 50000u);
    EXPECT_EQ(ParticleSystem::DEFAULT_PARTICLE_COUNT, 4000u);
    EXPECT_EQ(ParticleSystem::DEFAULT_NUM_TYPES, 6u);
}

// --- Attraction matrix (CPU-side, no Vulkan needed) ---

class AttractionMatrixTest : public ::testing::Test {
protected:
    ParticleSystem ps;
};

TEST_F(AttractionMatrixTest, SetAndGet)
{
    ps.setAttraction(0, 1, 0.75f);
    EXPECT_FLOAT_EQ(ps.getAttraction(0, 1), 0.75f);

    ps.setAttraction(3, 5, -0.5f);
    EXPECT_FLOAT_EQ(ps.getAttraction(3, 5), -0.5f);
}

TEST_F(AttractionMatrixTest, IndependentEntries)
{
    ps.setAttraction(0, 1, 0.5f);
    ps.setAttraction(1, 0, -0.3f);
    EXPECT_FLOAT_EQ(ps.getAttraction(0, 1), 0.5f);
    EXPECT_FLOAT_EQ(ps.getAttraction(1, 0), -0.3f);
}

TEST_F(AttractionMatrixTest, RandomizeFillsAllEntries)
{
    // Zero out first
    float* matrix = ps.getAttractionMatrix();
    for (uint32_t i = 0; i < ParticleSystem::MAX_TYPES * ParticleSystem::MAX_TYPES; i++)
    {
        matrix[i] = 0.0f;
    }

    ps.randomizeAttractions();

    // Check that at least some entries changed (probabilistically certain)
    int nonZero = 0;
    for (uint32_t i = 0; i < ParticleSystem::MAX_TYPES * ParticleSystem::MAX_TYPES; i++)
    {
        if (matrix[i] != 0.0f)
            nonZero++;
    }
    EXPECT_GT(nonZero, 0);
}

TEST_F(AttractionMatrixTest, RandomizeInRange)
{
    ps.randomizeAttractions();
    float* matrix = ps.getAttractionMatrix();

    for (uint32_t i = 0; i < ParticleSystem::MAX_TYPES * ParticleSystem::MAX_TYPES; i++)
    {
        EXPECT_GE(matrix[i], -1.0f);
        EXPECT_LE(matrix[i], 1.0f);
    }
}

TEST_F(AttractionMatrixTest, DirectPointerAccess)
{
    float* matrix = ps.getAttractionMatrix();
    matrix[2 * ParticleSystem::MAX_TYPES + 3] = 0.42f;
    EXPECT_FLOAT_EQ(ps.getAttraction(2, 3), 0.42f);
}
