#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>

namespace pitaya {

/**
 * @brief A 2D lookup table with bilinear interpolation using std::array for fixed-size, no-allocation storage.
 *
 * This class is designed for embedded systems where dynamic memory allocation is disallowed.
 * The sizes of the axes must be known at compile time.
 *
 * It stores a 2D grid of data points (z) defined over two independent axes (x and y).
 * It automatically sorts the input data during initialization to ensure correct interpolation.
 * It provides a method to query a value at any (x, y) coordinate, using bilinear
 * interpolation for points that fall between the grid lines.
 *
 * For points outside the defined grid, the value is clamped to the nearest edge/corner.
 *
 * @tparam T The floating-point type of the data (e.g., float, double).
 * @tparam NumX The number of points on the x-axis.
 * @tparam NumY The number of points on the y-axis.
 */
template <typename T, size_t NumX, size_t NumY>
class lookup_table_2d {
public:
    // Compile-time checks for minimum dimensions
    static_assert(NumX >= 2, "x-axis must have at least 2 points for interpolation.");
    static_assert(NumY >= 2, "y-axis must have at least 2 points for interpolation.");

    lookup_table_2d() = default;

    /**
     * @brief Constructs and initializes the 2D lookup table.
     *
     * @param x_axis An array representing the breakpoints on the x-axis.
     * @param y_axis An array representing the breakpoints on the y-axis.
     * @param z_values A 2D array of data points. Dimensions are enforced at compile time.
     */
    lookup_table_2d(const std::array<T, NumX>& x_axis, const std::array<T, NumY>& y_axis,
                    const std::array<std::array<T, NumX>, NumY>& z_values)
    {
        configure(x_axis, y_axis, z_values);
    }

    /**
     * @brief Configures the lookup table with new axes and values, sorting them internally.
     *
     * @param x_axis New x-axis breakpoints.
     * @param y_axis New y-axis breakpoints.
     * @param z_values New 2D data points.
     */
    void configure(const std::array<T, NumX>& x_axis, const std::array<T, NumY>& y_axis,
                   const std::array<std::array<T, NumX>, NumY>& z_values)
    {
        // 1. Sort x_axis and get permutation indices
        std::array<size_t, NumX> x_indices;
        std::iota(x_indices.begin(), x_indices.end(), 0);
        std::sort(x_indices.begin(), x_indices.end(), [&x_axis](size_t a, size_t b) {
            return x_axis[a] < x_axis[b];
        });

        for (size_t i = 0; i < NumX; ++i) {
            _x_axis[i] = x_axis[x_indices[i]];
        }

        // 2. Sort y_axis and get permutation indices
        std::array<size_t, NumY> y_indices;
        std::iota(y_indices.begin(), y_indices.end(), 0);
        std::sort(y_indices.begin(), y_indices.end(), [&y_axis](size_t a, size_t b) {
            return y_axis[a] < y_axis[b];
        });

        for (size_t i = 0; i < NumY; ++i) {
            _y_axis[i] = y_axis[y_indices[i]];
        }

        // 3. Permute z_values according to both x and y sortings
        for (size_t y = 0; y < NumY; ++y) {
            size_t original_y = y_indices[y];
            for (size_t x = 0; x < NumX; ++x) {
                size_t original_x = x_indices[x];
                _z_values[y][x] = z_values[original_y][original_x];
            }
        }
        
        _last_x_idx = 0;
        _last_y_idx = 0;
    }

    /**
     * @brief Retrieves a value from the table for a given (x, y) coordinate.
     *
     * Performs bilinear interpolation if the point is within the grid.
     * Clamps the result to the edge if the point is outside the grid.
     * Sequential queries are optimized to O(1).
     *
     * @param x The coordinate on the x-axis.
     * @param y The coordinate on the y-axis.
     * @return The interpolated or clamped value.
     */
    [[nodiscard]] T get_value(T x, T y) const {
        // --- Step 1: Find indices and clamp coordinates ---
        size_t x_idx = find_lower_bound_index_x(x);
        size_t y_idx = find_lower_bound_index_y(y);

        // Clamp the input coordinates to the table boundaries
        x = std::clamp(x, _x_axis.front(), _x_axis.back());
        y = std::clamp(y, _y_axis.front(), _y_axis.back());

        // --- Step 2: Get the four corner points of the interpolation cell ---
        const T x0 = _x_axis[x_idx];
        const T x1 = _x_axis[x_idx + 1];
        const T y0 = _y_axis[y_idx];
        const T y1 = _y_axis[y_idx + 1];

        const T z00 = _z_values[y_idx][x_idx];
        const T z10 = _z_values[y_idx][x_idx + 1];
        const T z01 = _z_values[y_idx + 1][x_idx];
        const T z11 = _z_values[y_idx + 1][x_idx + 1];

        // --- Step 3: Calculate interpolation fractions ---
        T x_frac = (x1 - x0) > T{0} ? (x - x0) / (x1 - x0) : T{0};
        T y_frac = (y1 - y0) > T{0} ? (y - y0) / (y1 - y0) : T{0};

        // --- Step 4: Perform bilinear interpolation ---
        T z_interp_y0 = z00 + x_frac * (z10 - z00);
        T z_interp_y1 = z01 + x_frac * (z11 - z01);
        T result = z_interp_y0 + y_frac * (z_interp_y1 - z_interp_y0);

        return result;
    }

private:
    /**
     * @brief Finds the index of the lower bound for x, caching the result.
     */
    [[nodiscard]] size_t find_lower_bound_index_x(T value) const {
        if (value >= _x_axis[_last_x_idx] && value <= _x_axis[_last_x_idx + 1]) {
            return _last_x_idx;
        }

        auto it = std::lower_bound(_x_axis.begin(), _x_axis.end(), value);
        if (it == _x_axis.begin()) {
            _last_x_idx = 0;
        } else if (it == _x_axis.end()) {
            _last_x_idx = NumX - 2;
        } else {
            _last_x_idx = static_cast<size_t>(std::distance(_x_axis.begin(), it)) - 1;
        }
        return _last_x_idx;
    }

    /**
     * @brief Finds the index of the lower bound for y, caching the result.
     */
    [[nodiscard]] size_t find_lower_bound_index_y(T value) const {
        if (value >= _y_axis[_last_y_idx] && value <= _y_axis[_last_y_idx + 1]) {
            return _last_y_idx;
        }

        auto it = std::lower_bound(_y_axis.begin(), _y_axis.end(), value);
        if (it == _y_axis.begin()) {
            _last_y_idx = 0;
        } else if (it == _y_axis.end()) {
            _last_y_idx = NumY - 2;
        } else {
            _last_y_idx = static_cast<size_t>(std::distance(_y_axis.begin(), it)) - 1;
        }
        return _last_y_idx;
    }

    std::array<T, NumX> _x_axis;
    std::array<T, NumY> _y_axis;
    std::array<std::array<T, NumX>, NumY> _z_values;
    mutable size_t _last_x_idx{0};
    mutable size_t _last_y_idx{0};
};

}  // namespace pitaya