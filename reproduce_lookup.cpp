#include "pitaya/lookup_table_1d.hpp"
#include <iostream>
#include <array>
#include <cmath>
#include <cassert>
#include <algorithm>

int main() {
    // Test 1: NaN in axis
    std::array<float, 3> x_axis_nan{10.0f, NAN, 20.0f};
    std::cout << "Checking is_sorted with {10, NaN, 20}: " << std::is_sorted(x_axis_nan.begin(), x_axis_nan.end()) << std::endl;
    
    // Test 2: find_lower_bound_index with duplicates at the end
    std::array<float, 4> x_axis_dup{10.0f, 20.0f, 30.0f, 30.0f};
    std::array<float, 4> y_values_dup{100.0f, 200.0f, 300.0f, 400.0f};
    pitaya::lookup_table_1d<float, 4> table_dup(x_axis_dup, y_values_dup);
    
    std::cout << "Value at 30: " << table_dup.get_value(30.0f) << std::endl;
    std::cout << "Value at 35: " << table_dup.get_value(35.0f) << std::endl;

    // Test 7: Triggering assert with duplicates in axis
    std::cout << "Attempting to construct lookup_table_1d with duplicates in axis..." << std::endl;
    std::array<float, 3> x_axis_dup_real{10.0f, 10.0f, 20.0f};
    std::array<float, 3> y_values_real{100.0f, 200.0f, 300.0f};
    pitaya::lookup_table_1d<float, 3> table_dup2(x_axis_dup_real, y_values_real);

    return 0;
}
