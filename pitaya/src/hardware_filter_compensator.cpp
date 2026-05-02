#include "pitaya/hardware_filter_compensator.hpp"

#include <cmath>

namespace pitaya {

void hardware_filter_compensator::configure(mojito::frequency_t cutoff_f1, mojito::frequency_t cutoff_f2, int num_filters)
{
    _num_filters = 0;
    _cutoff_f1 = mojito::frequency_t{0.0f};
    _cutoff_f2 = mojito::frequency_t{0.0f};

    if (num_filters <= 0) {
        return;
    }

    if (cutoff_f1.value() > 0.0f) {
        _cutoff_f1 = cutoff_f1;
        _num_filters = 1;
    }

    if (num_filters >= 2 && cutoff_f2.value() > 0.0f) {
        _cutoff_f2 = cutoff_f2;
        _num_filters = 2;
    }
}

void hardware_filter_compensator::reset()
{
    _cutoff_f1 = mojito::frequency_t{0.0f};
    _cutoff_f2 = mojito::frequency_t{0.0f};
    _num_filters = 0;
}

}  // namespace pitaya
