#include <gtest/gtest.h>

#include "pitaya/first_order_low_pass_filter.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
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

        sim.initialize();
    }

    simulator sim;
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
