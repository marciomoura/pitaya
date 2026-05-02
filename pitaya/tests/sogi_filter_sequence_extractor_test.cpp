#include "pitaya/sogi_filter_sequence_extractor.hpp"

#include <gtest/gtest.h>
#include <mojito/mojito.hpp>

namespace {

using namespace pitaya;
using namespace mojito;

TEST(SogiFilterSequenceExtractorTest, BasicInit) {
    duration_t ts{50e-6f};
    sogi_filter_sequence_extractor extractor(ts);
    EXPECT_NEAR(extractor.get_positive_sequence_alpha().value(), 0.0, 1e-6);
}

TEST(SogiFilterSequenceExtractorTest, ExtractionNominal) {
    duration_t ts{1e-4f};
    sogi_filter_sequence_extractor extractor(ts);
    extractor.configure(1.414f, duration_t{0.02f});
    angular_frequency_t omega{static_cast<float>(2.0 * pi * 50.0)};
    
    for(int i=0; i<2000; ++i) {
        float t = i * ts.value();
        alphabeta<voltage_pu_t> v_in{voltage_pu_t{std::sin(omega.value() * t)}, voltage_pu_t{-std::cos(omega.value() * t)}};
        extractor.update(v_in, omega);
    }
    
    EXPECT_NEAR(extractor.get_positive_sequence().magnitude().value(), 1.0, 0.02);
}

}  // namespace
