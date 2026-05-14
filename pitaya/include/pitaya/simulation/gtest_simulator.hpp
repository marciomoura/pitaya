#pragma once

#ifdef PITAYA_HAS_GTEST

#include <gtest/gtest.h>

#include "pitaya/simulation/gtest_data_exporter.hpp"
#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

/// Creates a simulator configured to report assertion failures through Google Test
/// and automatically export simulation data to CSV on destruction.
inline simulator make_gtest_simulator()
{
    simulator sim;
    sim.on_assertion_failure([](const std::string& message) { ADD_FAILURE() << message; });
    sim.on_destruction([](const simulator& s) { export_to_csv_gtest(s); });
    return sim;
}

}  // namespace pitaya

#endif  // PITAYA_HAS_GTEST
