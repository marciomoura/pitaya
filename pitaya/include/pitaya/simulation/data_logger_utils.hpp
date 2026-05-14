#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include "pitaya/simulation/data_logger.hpp"

namespace pitaya {

/// Utilities for extracting data from various types.
struct logger_utils {
    template <typename T>
    static float extract_value(const T& val)
    {
        if constexpr (requires { val.value(); }) {
            return static_cast<float>(val.value());
        }
        else {
            return static_cast<float>(val);
        }
    }

    template <typename Func>
    static void register_signal(data_logger& logger, plot_metadata metadata, Func func)
    {
        using T = std::invoke_result_t<Func>;

        // abc frame: .a(), .b(), .c()
        if constexpr (requires {
                          std::declval<T>().a();
                          std::declval<T>().b();
                          std::declval<T>().c();
                      }) {
            logger.register_signal(
                std::move(metadata),
                [func](float* out) {
                    auto val = func();
                    out[0] = extract_value(val.a());
                    out[1] = extract_value(val.b());
                    out[2] = extract_value(val.c());
                },
                3);
        }
        // alphabeta frame: .alpha(), .beta()
        else if constexpr (requires {
                               std::declval<T>().alpha();
                               std::declval<T>().beta();
                           }) {
            logger.register_signal(
                std::move(metadata),
                [func](float* out) {
                    auto val = func();
                    out[0] = extract_value(val.alpha());
                    out[1] = extract_value(val.beta());
                },
                2);
        }
        // dq frame: .d(), .q()
        else if constexpr (requires {
                               std::declval<T>().d();
                               std::declval<T>().q();
                           }) {
            logger.register_signal(
                std::move(metadata),
                [func](float* out) {
                    auto val = func();
                    out[0] = extract_value(val.d());
                    out[1] = extract_value(val.q());
                },
                2);
        }
        // Scalar
        else {
            logger.register_signal(std::move(metadata), [func](float* out) { *out = extract_value(func()); }, 1);
        }
    }

    template <typename Func>
    static void register_signal(data_logger& logger, std::string name, Func func)
    {
        register_signal(logger, plot_metadata{.name = std::move(name)}, std::move(func));
    }
};

}  // namespace pitaya
