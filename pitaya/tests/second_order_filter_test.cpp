#include "pitaya/second_order_filter.hpp"

#include <gtest/gtest.h>

#include <vector>

using namespace pitaya;

class SecondOrderLowPassFilterTest : public ::testing::Test {
protected:
    static constexpr double sampling_freq = 100000.0;
    static constexpr double sampling_period = 1.0 / sampling_freq;
    static constexpr double cutoff_freq = 100.0;
    static constexpr double damping_ratio = 0.707;
    static constexpr double test_duration = 1.0;
    static constexpr std::size_t num_samples = static_cast<std::size_t>(test_duration * sampling_freq);

    second_order_low_pass_filter<real_t> filter{sampling_period};

    void SetUp() override { filter.configure(cutoff_freq, damping_ratio); }
};

TEST_F(SecondOrderLowPassFilterTest, MagnitudeResponseAtCutoff)
{
    const real_t magnitude_db = filter.get_magnitude_response_db(cutoff_freq);
    EXPECT_NEAR(magnitude_db, -3.0f, 0.1f);
}

TEST_F(SecondOrderLowPassFilterTest, MagnitudeResponseAtVariousFrequencies)
{
    const std::vector<real_t> test_freqs = {cutoff_freq / 100.0f, cutoff_freq / 10.0f, cutoff_freq, cutoff_freq * 10.0f};
    const std::vector<real_t> expected_db = {0.0f, 0.0f, -3.0f, -40.0f};

    for (size_t i = 0; i < test_freqs.size(); ++i) {
        const real_t magnitude_db = filter.get_magnitude_response_db(test_freqs[i]);
        EXPECT_NEAR(magnitude_db, expected_db[i], 1.0f) << "Failed at frequency " << test_freqs[i] << " Hz ";
    }
}

TEST_F(SecondOrderLowPassFilterTest, PhaseResponseAtVariousFrequencies)
{
    EXPECT_NEAR(filter.get_phase_response_degrees(cutoff_freq), -90.0, 2.0);
    EXPECT_NEAR(filter.get_phase_response_degrees(cutoff_freq / 100.0), 0.0, 2.0);
    EXPECT_NEAR(filter.get_phase_response_degrees(cutoff_freq * 10.0), -180.0, 10.0);
}

TEST_F(SecondOrderLowPassFilterTest, SinusoidalResponseMagnitude)
{
    const real_t test_freq = 100.0f;
    const real_t input_amplitude = 1.0f;
    real_t max_output = 0.0f;
    const real_t omega = 2.0f * mojito::pi * test_freq;

    for (size_t i = 0; i < 2 * num_samples; ++i) {
        const real_t t = i * static_cast<real_t>(sampling_period);
        const real_t input = input_amplitude * std::sin(omega * t);
        const real_t output = filter.update(input);
        if (i >= num_samples) {
            max_output = std::max(max_output, std::abs(output));
        }
    }

    const real_t measured_magnitude = max_output / input_amplitude;
    const real_t theoretical_magnitude = static_cast<real_t>(filter.get_magnitude_response(test_freq));
    EXPECT_NEAR(measured_magnitude, theoretical_magnitude, 0.01f);
}

class SecondOrderBandRejectFilterTest : public ::testing::Test {
protected:
    static constexpr real_t sampling_freq = 10000.0f;
    static constexpr real_t sampling_period = 1.0f / sampling_freq;
    static constexpr real_t center_freq = 100.0f;
    static constexpr real_t bandwidth = 10.0f;

    second_order_band_reject_filter<real_t> filter{sampling_period};

    void SetUp() override { filter.configure(center_freq, bandwidth); }
};

TEST_F(SecondOrderBandRejectFilterTest, MagnitudeResponseAtCenterFrequency)
{
    const real_t magnitude_db = filter.get_magnitude_response_db(center_freq);
    EXPECT_LT(magnitude_db, -20.0f);
}

class SecondOrderHighPassFilterTest : public ::testing::Test {
protected:
    static constexpr real_t sampling_freq = 100000.0f;
    static constexpr real_t sampling_period = 1.0f / sampling_freq;
    static constexpr real_t cutoff_freq = 100.0f;
    static constexpr real_t damping_ratio = 0.707f;

    second_order_high_pass_filter<real_t> filter{sampling_period};

    void SetUp() override { filter.configure(cutoff_freq, damping_ratio); }
};

TEST_F(SecondOrderHighPassFilterTest, MagnitudeResponseAtCutoff)
{
    const real_t magnitude_db = filter.get_magnitude_response_db(cutoff_freq);
    EXPECT_NEAR(magnitude_db, -3.0f, 0.5f);
}
