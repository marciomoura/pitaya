#include <gtest/gtest.h>

#include "pitaya/boolean_debouncer.hpp"
#include "pitaya/off_delay.hpp"
#include "pitaya/on_delay.hpp"
#include "pitaya/on_off_delay.hpp"
#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/gtest_simulator.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace {

using namespace pitaya;
using namespace mojito;

/// @class TimingLogicSimTest
/// @brief Simulation tests for digital timing logic components.
///
/// These tests verify components like boolean debouncers and on-off delays
/// using timing diagrams and simulated noise scenarios.
class TimingLogicSimTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Debouncer: 50ms on, 50ms off
        debouncer.configure_sampling_time(duration_t{1e-3f});
        debouncer.configure_delay(duration_t{0.05f}, duration_t{0.05f});

        // On-Off Delay: 20ms on, 100ms off
        delay_block.configure(0.02f, 0.1f);

        // Individual Delays: 50ms
        ton.configure(0.05f);
        toff.configure(0.05f);

        sim.register_lambda(duration_t{1e-3}, [this]() {
            double current_time = sim.get_current_simulation_time_seconds();

            // Scenario:
            // 0.1s - 0.2s: Chatty signal (rapid toggling) -> Should stay false
            // 0.3s - 0.5s: Stable High -> Should turn true after delay
            // 0.5s - 0.7s: Stable Low -> Should turn false after delay

            if (current_time < 0.1) {
                raw_input = false;
            }
            else if (current_time < 0.2) {
                // 10ms toggle
                raw_input = (static_cast<int>(current_time * 1000) % 20 < 10);
            }
            else if (current_time < 0.3) {
                raw_input = false;
            }
            else if (current_time < 0.5) {
                raw_input = true;
            }
            else if (current_time < 0.7) {
                raw_input = false;
            }
            else {
                raw_input = false;
            }

            debouncer.update(raw_input);
            delay_block.update(raw_input);
            ton.update(raw_input);
            toff.update(raw_input);
        });

        sim.register_signal("raw_input", [this]() { return raw_input ? 1.0f : 0.0f; });
        sim.register_signal("debounced", [this]() { return debouncer.get_output() ? 1.0f : 0.0f; });
        sim.register_signal("delayed", [this]() { return delay_block.get_output() ? 1.0f : 0.0f; });
        sim.register_signal("ton_out", [this]() { return ton.get_output() ? 1.0f : 0.0f; });
        sim.register_signal("toff_out", [this]() { return toff.get_output() ? 1.0f : 0.0f; });
    }

    simulator sim = make_gtest_simulator();

    boolean_debouncer debouncer;
    on_off_delay delay_block{0.001};
    on_delay ton{0.05f, 0.001};
    off_delay toff{0.05f, 0.001};
    bool raw_input{false};
};

/// @test TimingDiagrams
/// @brief Verifies noise immunity and transition delays for digital logic.
///
/// Ensures the debouncer ignores rapid toggling and that both components
/// respect their configured on/off time delays.
TEST_F(TimingLogicSimTest, TimingDiagrams)
{
    // Assertions
    // 1. Noise immunity: Debouncer must ignore 10ms toggling (needs 50ms stable)
    sim.register_assertion(make_lambda_assert({.name = "noise_immunity",
        .func =
            [this]() {
                return !debouncer.get_output() ? assertion_result::pass()
                                               : assertion_result::fail("Debouncer triggered on noise");
            },
        .strategy = time_range({.start = duration_t{0.1}, .end = duration_t{0.25}})}));

    // 2. Correct on-delay: Must be ON during stable high period
    sim.register_assertion(make_lambda_assert({.name = "debouncer_on",
        .func =
            [this]() {
                return debouncer.get_output() ? assertion_result::pass()
                                              : assertion_result::fail("Debouncer failed to turn ON");
            },
        .strategy = time_range({.start = duration_t{0.4}, .end = duration_t{0.5}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.8));
}

/// @test IndividualDelays
/// @brief Verifies standalone on_delay and off_delay behaviors.
TEST_F(TimingLogicSimTest, IndividualDelays)
{
    // 1. On-delay (ton): input true at 0.3s, delay 50ms -> true at 0.35s
    sim.register_assertion(make_lambda_assert({.name = "ton_delayed",
        .func =
            [this]() {
                return ton.get_output() ? assertion_result::pass() : assertion_result::fail("TON failed to turn ON");
            },
        .strategy = time_range({.start = duration_t{0.36}, .end = duration_t{0.49}})}));

    // 2. Off-delay (toff): input false at 0.5s, delay 50ms -> false at 0.55s
    sim.register_assertion(make_lambda_assert({.name = "toff_delayed",
        .func =
            [this]() {
                return !toff.get_output() ? assertion_result::pass()
                                          : assertion_result::fail("TOFF failed to turn OFF");
            },
        .strategy = time_range({.start = duration_t{0.56}, .end = duration_t{0.7}})}));

    sim.initialize();
    sim.simulate_for(duration_t(0.8));
}

}  // namespace
