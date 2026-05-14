#pragma once

#ifdef PITAYA_HAS_GTEST

#include <gtest/gtest.h>

#include "pitaya/simulation/simulator.hpp"

namespace pitaya {

/// Creates a simulator configured to report assertion failures through Google Test.
inline simulator make_gtest_simulator()
{
    simulator sim;
    sim.on_assertion_failure([](const std::string& message) { ADD_FAILURE() << message; });
    return sim;
}

}  // namespace pitaya

#endif  // PITAYA_HAS_GTEST
