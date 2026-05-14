#include "pitaya/second_order_high_pass_filter.hpp"

#include <gtest/gtest.h>

#include <vector>

using namespace pitaya;

class SecondOrderHighPassFilterTest : public ::testing::Test {
protected:
    static constexpr real_t sampling_freq = 100000.0f;
    static constexpr real_t sampling_period = 1.0f / sampling_freq;
    static constexpr real_t cutoff_freq = 100.0f;
    static constexpr real_t damping_ratio = 0.707f;
    static constexpr real_t test_duration = 0.5f;
    static constexpr std::size_t num_samples = static_cast<std::size_t>(test_duration * sampling_freq);

    second_order_high_pass_filter<real_t> filter{sampling_period};

    void SetUp() override { filter.configure(cutoff_freq, damping_ratio); }
};

TEST_F(SecondOrderHighPassFilterTest, MagnitudeResponseAtCutoff)
{
    const real_t magnitude_db = filter.get_magnitude_response_db(cutoff_freq);
    EXPECT_NEAR(magnitude_db, -3.0f, 0.1f);
}

TEST_F(SecondOrderHighPassFilterTest, MagnitudeResponseAtVariousFrequencies)
{
    const std::vector<real_t> test_freqs = {
        cutoff_freq / 10.0f, cutoff_freq, cutoff_freq * 10.0f, cutoff_freq * 100.0f};
    const std::vector<real_t> expected_db = {-40.0f, -3.0f, 0.0f, 0.0f};

    for (size_t i = 0; i < test_freqs.size(); ++i) {
        const real_t magnitude_db = filter.get_magnitude_response_db(test_freqs[i]);
        EXPECT_NEAR(magnitude_db, expected_db[i], 1.0f) << "Failed at frequency " << test_freqs[i] << " Hz ";
    }
}

TEST_F(SecondOrderHighPassFilterTest, PhaseResponseAtVariousFrequencies)
{
    // Phase is 180 deg at DC, 90 deg at cutoff, 0 deg at high frequencies
    EXPECT_NEAR(filter.get_phase_response_degrees(cutoff_freq / 100.0f), 180.0, 1.0);
    EXPECT_NEAR(filter.get_phase_response_degrees(cutoff_freq), 90.0, 1.0);
    EXPECT_NEAR(filter.get_phase_response_degrees(cutoff_freq * 100.0f), 0.0, 1.0);
}

TEST_F(SecondOrderHighPassFilterTest, SinusoidalResponseMagnitudeHighFreq)
{
    const real_t test_freq = cutoff_freq * 10.0f;
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

    EXPECT_NEAR(max_output, 1.0f, 0.01f);
}

TEST_F(SecondOrderHighPassFilterTest, SinusoidalResponseMagnitudeLowFreq)
{
    const real_t test_freq = cutoff_freq / 10.0f;
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

    // At 1/10th cutoff, magnitude should be approx -40dB (0.01)
    EXPECT_NEAR(max_output, 0.01f, 0.005f);
}
