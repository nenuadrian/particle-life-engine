#include <gtest/gtest.h>
#include "math_utils.h"

using namespace math_utils;

// --- clampZoom ---

TEST(ClampZoom, WithinRange)
{
    EXPECT_FLOAT_EQ(clampZoom(1.0f), 1.0f);
    EXPECT_FLOAT_EQ(clampZoom(2.5f), 2.5f);
}

TEST(ClampZoom, BelowMin)
{
    EXPECT_FLOAT_EQ(clampZoom(0.1f), MIN_ZOOM);
    EXPECT_FLOAT_EQ(clampZoom(-5.0f), MIN_ZOOM);
    EXPECT_FLOAT_EQ(clampZoom(0.0f), MIN_ZOOM);
}

TEST(ClampZoom, AboveMax)
{
    EXPECT_FLOAT_EQ(clampZoom(10.0f), MAX_ZOOM);
    EXPECT_FLOAT_EQ(clampZoom(100.0f), MAX_ZOOM);
}

TEST(ClampZoom, ExactBoundaries)
{
    EXPECT_FLOAT_EQ(clampZoom(MIN_ZOOM), MIN_ZOOM);
    EXPECT_FLOAT_EQ(clampZoom(MAX_ZOOM), MAX_ZOOM);
}

// --- worldSizeForZoom ---

TEST(WorldSizeForZoom, ZoomOne)
{
    EXPECT_FLOAT_EQ(worldSizeForZoom(1.0f), 1.0f);
}

TEST(WorldSizeForZoom, ZoomedIn)
{
    // zoom > 1 -> worldSize = 1.0 (smaller arena, zoomed in)
    EXPECT_FLOAT_EQ(worldSizeForZoom(2.0f), 1.0f);
    EXPECT_FLOAT_EQ(worldSizeForZoom(4.0f), 1.0f);
    EXPECT_FLOAT_EQ(worldSizeForZoom(MAX_ZOOM), 1.0f);
}

TEST(WorldSizeForZoom, ZoomedOut)
{
    // zoom < 1 -> worldSize = 1/zoom (larger arena, zoomed out)
    EXPECT_FLOAT_EQ(worldSizeForZoom(0.5f), 2.0f);
    EXPECT_FLOAT_EQ(worldSizeForZoom(MIN_ZOOM), 1.0f / MIN_ZOOM);
}

TEST(WorldSizeForZoom, ClampsInput)
{
    // Values outside zoom range get clamped first
    EXPECT_FLOAT_EQ(worldSizeForZoom(0.1f), worldSizeForZoom(MIN_ZOOM));
    EXPECT_FLOAT_EQ(worldSizeForZoom(20.0f), worldSizeForZoom(MAX_ZOOM));
}

// --- applyZoomSteps ---

TEST(ApplyZoomSteps, ZeroSteps)
{
    EXPECT_FLOAT_EQ(applyZoomSteps(2.0f, 0.0f), 2.0f);
}

TEST(ApplyZoomSteps, PositiveStep)
{
    float result = applyZoomSteps(1.0f, 1.0f);
    EXPECT_FLOAT_EQ(result, ZOOM_STEP);
}

TEST(ApplyZoomSteps, NegativeStep)
{
    float result = applyZoomSteps(1.0f, -1.0f);
    EXPECT_FLOAT_EQ(result, 1.0f / ZOOM_STEP);
}

TEST(ApplyZoomSteps, ClampsResult)
{
    // Many positive steps should clamp to MAX
    float result = applyZoomSteps(1.0f, 100.0f);
    EXPECT_FLOAT_EQ(result, MAX_ZOOM);

    // Many negative steps should clamp to MIN
    result = applyZoomSteps(1.0f, -100.0f);
    EXPECT_FLOAT_EQ(result, MIN_ZOOM);
}

// --- wrapPosition ---

TEST(WrapPosition, AlreadyInRange)
{
    EXPECT_FLOAT_EQ(wrapPosition(0.5f, 1.0f), 0.5f);
    EXPECT_FLOAT_EQ(wrapPosition(0.0f, 1.0f), 0.0f);
}

TEST(WrapPosition, ExceedsWorldSize)
{
    EXPECT_FLOAT_EQ(wrapPosition(1.5f, 1.0f), 0.5f);
    EXPECT_FLOAT_EQ(wrapPosition(2.3f, 1.0f), 0.3f);
}

TEST(WrapPosition, Negative)
{
    EXPECT_NEAR(wrapPosition(-0.1f, 1.0f), 0.9f, 1e-6f);
    EXPECT_NEAR(wrapPosition(-1.3f, 1.0f), 0.7f, 1e-6f);
}

TEST(WrapPosition, LargerWorld)
{
    EXPECT_FLOAT_EQ(wrapPosition(5.0f, 4.0f), 1.0f);
    EXPECT_NEAR(wrapPosition(-0.5f, 4.0f), 3.5f, 1e-6f);
}
