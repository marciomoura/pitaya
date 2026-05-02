#pragma once

namespace pitaya {

/**
 * @brief Sums two numbers.
 * @param a First number.
 * @param b Second number.
 * @return The sum of a and b.
 */
template <typename T>
constexpr T sum(T a, T b) {
    return a + b;
}

} // namespace pitaya
