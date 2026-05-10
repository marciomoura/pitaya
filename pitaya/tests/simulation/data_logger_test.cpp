#include "pitaya/simulation/data_logger.hpp"

#include <gtest/gtest.h>

#include "pitaya/simulation/data_logger_utils.hpp"

namespace {

using namespace pitaya;

TEST(DataLoggerTest, SampleView)
{
    float raw_data[] = {1.0f, 2.0f, 3.0f};
    sample_view view(raw_data, 3);

    EXPECT_EQ(view.dimension(), 3);
    EXPECT_FLOAT_EQ(view[0], 1.0f);
    EXPECT_FLOAT_EQ(view[1], 2.0f);
    EXPECT_FLOAT_EQ(view[2], 3.0f);

    int i = 0;
    for (float val : view) {
        EXPECT_FLOAT_EQ(val, raw_data[i++]);
    }
}

TEST(DataLoggerTest, SignalDataView)
{
    std::vector<float> raw_buffer = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    signal_data_view view(raw_buffer, 2);

    EXPECT_EQ(view.size(), 3);
    EXPECT_EQ(view.dimension(), 2);

    EXPECT_FLOAT_EQ(view[0][0], 1.0f);
    EXPECT_FLOAT_EQ(view[0][1], 2.0f);
    EXPECT_FLOAT_EQ(view[1][0], 3.0f);
    EXPECT_FLOAT_EQ(view[1][1], 4.0f);
    EXPECT_FLOAT_EQ(view[2][0], 5.0f);
    EXPECT_FLOAT_EQ(view[2][1], 6.0f);
}

TEST(DataLoggerTest, BasicLogging)
{
    data_logger logger;
    float val = 0.0f;

    logger.register_signal("test", [&](float* out) { *out = val; }, 1);

    logger.allocate(2);

    val = 1.1f;
    logger.capture();

    val = 2.2f;
    logger.capture();

    auto data = logger.get_data("test");
    ASSERT_EQ(data.size(), 2);
    EXPECT_FLOAT_EQ(data[0][0], 1.1f);
    EXPECT_FLOAT_EQ(data[1][0], 2.2f);

    logger.clear_data();
    EXPECT_EQ(logger.get_data("test").size(), 0);
}

TEST(DataLoggerTest, MultiDimLogging)
{
    data_logger logger;
    float a = 1, b = 2;

    logger.register_signal(
        "vec",
        [&](float* out) {
            out[0] = a;
            out[1] = b;
        },
        2);

    logger.capture();

    auto data = logger.get_data("vec");
    ASSERT_EQ(data.size(), 1);
    EXPECT_FLOAT_EQ(data[0][0], 1.0f);
    EXPECT_FLOAT_EQ(data[0][1], 2.0f);
}

struct mock_abc {
    float a() const { return 10.0f; }
    float b() const { return 20.0f; }
    float c() const { return 30.0f; }
};

struct mock_quantity {
    double value() const { return 42.0; }
};

TEST(DataLoggerUtilsTest, TypeExtraction)
{
    data_logger logger;

    logger_utils::register_signal(logger, "abc", []() { return mock_abc{}; });
    logger_utils::register_signal(logger, "qty", []() { return mock_quantity{}; });
    logger_utils::register_signal(logger, "float", []() { return 3.14f; });

    logger.capture();

    auto abc = logger.get_data("abc");
    EXPECT_EQ(abc.dimension(), 3);
    EXPECT_FLOAT_EQ(abc[0][0], 10.0f);
    EXPECT_FLOAT_EQ(abc[0][1], 20.0f);
    EXPECT_FLOAT_EQ(abc[0][2], 30.0f);

    auto qty = logger.get_data("qty");
    EXPECT_EQ(qty.dimension(), 1);
    EXPECT_FLOAT_EQ(qty[0][0], 42.0f);

    auto f = logger.get_data("float");
    EXPECT_EQ(f.dimension(), 1);
    EXPECT_FLOAT_EQ(f[0][0], 3.14f);
}

}  // namespace
