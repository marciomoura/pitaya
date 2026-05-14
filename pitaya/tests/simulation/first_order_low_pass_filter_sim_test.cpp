#include <gtest/gtest.h>

#include "pitaya/first_order_low_pass_filter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

class FirstOrderLowPassFilterSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        filter.configure(frequency_t{10.0f});
        sim.register_lambda(duration_t{100e-6}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();
            input = (current_time >= 0.05) ? voltage_pu_t{1.0f} : voltage_pu_t{0.0f};
            output = filter.update(input);
        });

        sim.register_signal("input", [this]() { return input.value(); });
        sim.register_signal("output", [this]() { return output.value(); });

        // Step response assertions
        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "initial_zero", [this]() { return output; }, voltage_pu_t{0.0f}, voltage_pu_t{1e-3f},
            time_range(duration_t{0.0}, duration_t{0.045})));

        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "steady_state", [this]() { return output; }, voltage_pu_t{1.0f}, voltage_pu_t{0.02f},
            time_range(duration_t{0.4}, duration_t{0.5})));

        // Tau approx 0.0159s. At t = 0.05 + 0.0159 = 0.0659, output should be approx 0.632
        sim.register_assertion(make_near_assert<voltage_pu_t>(
            "one_tau", [this]() { return output; }, voltage_pu_t{0.632f}, voltage_pu_t{0.05f},
            at_time(duration_t{0.0659}, duration_t{100e-6})));

        sim.initialize();
    }

    simulator sim = make_gtest_simulator();
    first_order_low_pass_filter<voltage_pu_t> filter{100e-6};
    voltage_pu_t input{0.0f};
    voltage_pu_t output{0.0f};
};

TEST_F(FirstOrderLowPassFilterSimTest, StepResponse)
{
    gtest_exporter exporter(sim);
    sim.simulate_for(duration_t(0.5));
}

}  // namespace
