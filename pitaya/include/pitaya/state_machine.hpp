#pragma once

namespace pitaya {

/// Generic base class for state machine states.
/// @tparam StateEnum Enumeration type defining state identifiers
/// @tparam InputType Type of input data structure
/// @tparam OutputType Type of output data structure
template <typename StateEnum, typename InputType, typename OutputType>
class state_base {
public:
    /// Called when entering this state.
    virtual void on_entry() noexcept {}

    /// Called when exiting this state.
    virtual void on_exit() noexcept {}

    /// Main state update function.
    /// @return Pointer to next state (nullptr if no transition)
    virtual state_base* update(const InputType& inputs) noexcept = 0;

    /// Get the current state outputs.
    virtual OutputType get_outputs() const noexcept = 0;

    /// Get the state identifier.
    virtual StateEnum get_id() const noexcept = 0;

    virtual ~state_base() = default;
};

/// Generic state machine implementation.
/// @tparam StateEnum Enumeration type defining state identifiers
/// @tparam InputType Type of input data structure
/// @tparam OutputType Type of output data structure
template <typename StateEnum, typename InputType, typename OutputType>
class state_machine {
public:
    using state_type = state_base<StateEnum, InputType, OutputType>;

    explicit state_machine(state_type* initial_state) noexcept
        : _current_state(initial_state), _previous_state(nullptr), _has_transitioned_this_cycle(false)
    {
    }

    /// Main update function - executes current state logic.
    void update(const InputType& inputs) noexcept
    {
        // Reset transition flag at start of each update cycle
        _has_transitioned_this_cycle = false;

        if (_current_state == nullptr) return;

        // Execute current state logic and check for transition
        state_type* next_state = _current_state->update(inputs);

        // Handle state transition if needed
        if (next_state != nullptr && next_state != _current_state) {
            _current_state->on_exit();
            _previous_state = _current_state;
            _current_state = next_state;
            _current_state->on_entry();
            _has_transitioned_this_cycle = true;
        }

        // Update outputs from current state
        if (_current_state != nullptr) {
            _outputs = _current_state->get_outputs();
        }
    }

    /// Reset state machine to specified state.
    void reset(state_type* reset_state) noexcept
    {
        if (_current_state != nullptr) {
            _current_state->on_exit();
        }
        _current_state = reset_state;
        _previous_state = nullptr;
        _has_transitioned_this_cycle = false;
        if (_current_state != nullptr) {
            _current_state->on_entry();
            _outputs = _current_state->get_outputs();
        }
    }

    /// Get current state outputs.
    OutputType get_outputs() const noexcept { return _outputs; }

    /// Get current state identifier.
    StateEnum get_current_state() const noexcept { return _current_state ? _current_state->get_id() : StateEnum{}; }

    /// Check if a state transition occurred in last update.
    bool has_transitioned() const noexcept { return _has_transitioned_this_cycle; }

    /// Get pointer to current state (for advanced usage).
    state_type* get_current_state_ptr() const noexcept { return _current_state; }

private:
    state_type* _current_state{nullptr};
    state_type* _previous_state{nullptr};
    OutputType _outputs{};
    bool _has_transitioned_this_cycle{false};
};

}  // namespace pitaya
