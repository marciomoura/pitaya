#include "pitaya/lookup_table_2d.hpp"

#include <gtest/gtest.h>

#include <array>

// Define constants for table dimensions to make tests readable
constexpr size_t NumX = 3;
constexpr size_t NumY = 4;

// Test fixture for creating a common lookup table object for tests
class LookupTable2DTest : public ::testing::Test {
protected:
    // The test data is now stored in std::array
    const std::array<float, NumX> x_axis{10.0f, 20.0f, 30.0f};
    const std::array<float, NumY> y_axis{2.0f, 4.0f, 6.0f, 8.0f};

    const std::array<std::array<float, NumX>, NumY> z_values{{
        {14.0f, 24.0f, 34.0f},  // y=2.0
        {18.0f, 28.0f, 38.0f},  // y=4.0
        {22.0f, 32.0f, 42.0f},  // y=6.0
        {26.0f, 36.0f, 46.0f}   // y=8.0
    }};
};

// Test that values on the exact grid points are retrieved correctly
TEST_F(LookupTable2DTest, HandlesExactGridPoints)
{
    pitaya::lookup_table_2d<float, NumX, NumY> table(x_axis, y_axis, z_values);
    EXPECT_FLOAT_EQ(table.interpolate(10.0f, 2.0f), 14.0f);
    EXPECT_FLOAT_EQ(table.interpolate(30.0f, 8.0f), 46.0f);
    EXPECT_FLOAT_EQ(table.interpolate(20.0f, 6.0f), 32.0f);
}

// Test that values are correctly interpolated when falling between grid points
TEST_F(LookupTable2DTest, PerformsBilinearInterpolation)
{
    pitaya::lookup_table_2d<float, NumX, NumY> table(x_axis, y_axis, z_values);
    EXPECT_FLOAT_EQ(table.interpolate(15.0f, 3.0f), 21.0f);
    EXPECT_FLOAT_EQ(table.interpolate(25.0f, 7.0f), 39.0f);
}

// Test interpolation along a single axis
TEST_F(LookupTable2DTest, InterpolatesAlongSingleAxis)
{
    pitaya::lookup_table_2d<float, NumX, NumY> table(x_axis, y_axis, z_values);
    EXPECT_FLOAT_EQ(table.interpolate(10.0f, 5.0f), 20.0f);
    EXPECT_FLOAT_EQ(table.interpolate(25.0f, 2.0f), 29.0f);
}

// Test that values outside the defined axes are clamped to the boundary
TEST_F(LookupTable2DTest, ClampsValuesOutsideBounds)
{
    pitaya::lookup_table_2d<float, NumX, NumY> table(x_axis, y_axis, z_values);
    EXPECT_FLOAT_EQ(table.interpolate(5.0f, 3.0f), 16.0f);
    EXPECT_FLOAT_EQ(table.interpolate(35.0f, 5.0f), 40.0f);
    EXPECT_FLOAT_EQ(table.interpolate(15.0f, 1.0f), 19.0f);
    EXPECT_FLOAT_EQ(table.interpolate(25.0f, 9.0f), 41.0f);
}

// Test clamping to the four corners of the grid
TEST_F(LookupTable2DTest, ClampsToCorners)
{
    pitaya::lookup_table_2d<float, NumX, NumY> table(x_axis, y_axis, z_values);
    EXPECT_FLOAT_EQ(table.interpolate(0.0f, 0.0f), 14.0f);
    EXPECT_FLOAT_EQ(table.interpolate(40.0f, 10.0f), 46.0f);
    EXPECT_FLOAT_EQ(table.interpolate(5.0f, 10.0f), 26.0f);
    EXPECT_FLOAT_EQ(table.interpolate(35.0f, 1.0f), 34.0f);
}

// Test that the table correctly handles unsorted input data for both axes
TEST_F(LookupTable2DTest, HandlesUnsortedData)
{
    const std::array<float, NumX> unsorted_x{30.0f, 10.0f, 20.0f};
    const std::array<float, NumY> unsorted_y{8.0f, 2.0f, 6.0f, 4.0f};

    // z_values corresponding to original axes:
    // x_axis: 10, 20, 30
    // y_axis: 2, 4, 6, 8
    // z = {
    //   {14, 24, 34}, // y=2
    //   {18, 28, 38}, // y=4
    //   {22, 32, 42}, // y=6
    //   {26, 36, 46}  // y=8
    // }

    // Reordered z according to unsorted_x and unsorted_y:
    // unsorted_y[0]=8 -> {z(x=30,y=8), z(x=10,y=8), z(x=20,y=8)} = {46, 26, 36}
    // unsorted_y[1]=2 -> {z(x=30,y=2), z(x=10,y=2), z(x=20,y=2)} = {34, 14, 24}
    // unsorted_y[2]=6 -> {z(x=30,y=6), z(x=10,y=6), z(x=20,y=6)} = {42, 22, 32}
    // unsorted_y[3]=4 -> {z(x=30,y=4), z(x=10,y=4), z(x=20,y=4)} = {38, 18, 28}

    const std::array<std::array<float, NumX>, NumY> unsorted_z{{
        {46.0f, 26.0f, 36.0f},  // y=8
        {34.0f, 14.0f, 24.0f},  // y=2
        {42.0f, 22.0f, 32.0f},  // y=6
        {38.0f, 18.0f, 28.0f}   // y=4
    }};

    pitaya::lookup_table_2d<float, NumX, NumY> table(unsorted_x, unsorted_y, unsorted_z);

    // Exact points
    EXPECT_FLOAT_EQ(table.interpolate(10.0f, 2.0f), 14.0f);
    EXPECT_FLOAT_EQ(table.interpolate(30.0f, 8.0f), 46.0f);
    EXPECT_FLOAT_EQ(table.interpolate(20.0f, 6.0f), 32.0f);

    // Interpolation
    EXPECT_FLOAT_EQ(table.interpolate(15.0f, 3.0f), 21.0f);
    EXPECT_FLOAT_EQ(table.interpolate(25.0f, 7.0f), 39.0f);
}
