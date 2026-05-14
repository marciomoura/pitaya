#include "pitaya/second_order_band_reject_filter.hpp"

#include <gtest/gtest.h>

#include <vector>

using namespace pitaya;

class SecondOrderBandRejectFilterTest : public ::testing::Test {
protected:
    static constexpr real_t sampling_freq = 100000.0f;
    static constexpr real_t sampling_period = 1.0f / sampling_freq;
    static constexpr real_t center_freq = 100.0f;
    static constexpr real_t bandwidth = 10.0f;
    static constexpr real_t test_duration = 0.5f;
    static constexpr std::size_t num_samples = static_cast<std::size_t>(test_duration * sampling_freq);

    second_order_band_reject_filter<real_t> filter{sampling_period};

    void SetUp() override { filter.configure(center_freq, bandwidth); }
};

TEST_F(SecondOrderBandRejectFilterTest, MagnitudeResponseAtCenterFrequency)
{
    const real_t magnitude_db = filter.get_magnitude_response_db(center_freq);
    EXPECT_LT(magnitude_db, -40.0f);  // Notch should be deep
}

TEST_F(SecondOrderBandRejectFilterTest, MagnitudeResponseAtVariousFrequencies)
{
    // Frequencies far from center should have ~0dB gain
    const std::vector<real_t> test_freqs = {center_freq / 10.0f, center_freq * 10.0f};
    const std::vector<real_t> expected_db = {0.0f, 0.0f};

    for (size_t i = 0; i < test_freqs.size(); ++i) {
        const real_t magnitude_db = filter.get_magnitude_response_db(test_freqs[i]);
        EXPECT_NEAR(magnitude_db, expected_db[i], 0.1f) << "Failed at frequency " << test_freqs[i] << " Hz ";
    }
}

TEST_F(SecondOrderBandRejectFilterTest, PhaseResponseAtVariousFrequencies)
{
    // Phase is negative below center frequency and positive above
    EXPECT_LT(filter.get_phase_response_degrees(center_freq * 0.9f), 0.0);
    EXPECT_GT(filter.get_phase_response_degrees(center_freq * 1.1f), 0.0);

    // Far from notch, phase should be close to 0
    EXPECT_NEAR(filter.get_phase_response_degrees(center_freq / 10.0f), 0.0, 1.0);
    EXPECT_NEAR(filter.get_phase_response_degrees(center_freq * 10.0f), 0.0, 1.0);
}

TEST_F(SecondOrderBandRejectFilterTest, SinusoidalResponseMagnitude)
{
    // Test rejection at center frequency
    const real_t test_freq = center_freq;
    const real_t input_amplitude = 1.0f;
    real_t max_output = 0.0f;
    const real_t omega = 2.0f * mojito::pi * test_freq;

    filter.reset();
    for (size_t i = 0; i < 2 * num_samples; ++i) {
        const real_t t = i * sampling_period;
        const real_t input = input_amplitude * std::sin(omega * t);
        const real_t output = filter.update(input);
        if (i >= num_samples) {  // Wait for transient to settle
            max_output = std::max(max_output, std::abs(output));
        }
    }

    EXPECT_LT(max_output, 0.01f);  // Heavily attenuated
}

TEST_F(SecondOrderBandRejectFilterTest, PassThroughFrequencies)
{
    // Test pass-through far from notch
    const real_t test_freq = center_freq * 10.0f;
    const real_t input_amplitude = 1.0f;
    real_t max_output = 0.0f;
    const real_t omega = 2.0f * mojito::pi * test_freq;

    filter.reset();
    for (size_t i = 0; i < 2 * num_samples; ++i) {
        const real_t t = i * sampling_period;
        const real_t input = input_amplitude * std::sin(omega * t);
        const real_t output = filter.update(input);
        if (i >= num_samples) {
            max_output = std::max(max_output, std::abs(output));
        }
    }

    // With 100k sampling freq, 1000Hz has 100 samples per cycle, error will be small
    EXPECT_NEAR(max_output, 1.0f, 0.01f);
}
