#pragma once

namespace pitaya {

/// Detects rising and falling edges of a boolean input signal.
class edge_detector {
public:
    edge_detector() = default;

    /// Updates the edge detector with the latest input value.
    /// Returns true if a rising edge was detected.
    bool update(bool input);

    /// Checks if a rising edge was detected in the last update.
    bool is_rising_edge() const { return _rising_edge; }

    /// Checks if a falling edge was detected in the last update.
    bool is_falling_edge() const { return _falling_edge; }

    /// Returns true if a rising edge was detected.
    operator bool() const { return _rising_edge; }

    /// Resets the edge detector state.
    void reset();

private:
    bool _rising_edge{false};     ///< Indicates if a rising edge was detected.
    bool _falling_edge{false};    ///< Indicates if a falling edge was detected.
    bool _previous_input{false};  ///< Stores the previous input value.
};

}  // namespace pitaya
