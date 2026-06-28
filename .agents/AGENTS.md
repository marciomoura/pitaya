# Workspace Agentic Engineering Rules

These rules apply project-wide to all AI agents performing design, implementation, testing, or review in this repository.

## 1. Safety and Determinism
- **Allocation-Free Execution**: No dynamic memory allocations are allowed in the control loop (`new`, `malloc`, `std::vector`, `std::shared_ptr`, etc.). All control classes must be fully deterministic and non-allocating.
- **Strong Typing and Physical Dimensions**: Raw float or double types must not be used for physical quantities. You must use the `mojito::` unit and coordinate library (e.g., `voltage_t`, `current_t`, `frequency_t`, `duration_t`, or their per-unit/percent equivalents).
- **No Unbounded Loops**: Loop boundaries must be statically known at compile time to ensure real-time execution safety.
- **Explicit Variable Declarations**: The use of `auto` is strongly discouraged within control loops and critical math blocks. All variable types must be explicitly specified (e.g. `const current_t`, `const voltage_t`). All variables must be tagged as `const` by default unless mutation is strictly required.
- **Observability and Test Probes**: Control algorithms must expose critical internal states (e.g., PI integrator terms, tracking errors, intermediate rotating coordinate outputs) via public getter functions or a diagnostics struct. These serve as "test probes" so that internal signals can be logged and verified in simulation suites.


## 2. Coordinate Systems & Units
- Always specify the target coordinate system: three-phase phase-locked frame ($abc$), stationary orthogonal frame ($\alpha\beta$), or rotating synchronous frame ($dq$).
- When performing transformations, utilize the `mojito::to_dq()`, `mojito::to_alphabeta()`, and `mojito::to_abc()` APIs to ensure compile-time type verification.
- Always match per-unit bases using Mojito's per-unit conversions before mixing quantities of different scales.

## 3. Implementation Workflow
The workspace operates under a **Plan-Build-Verify-Ship** cycle:
1. **Control Architect** defines the header specifications and mathematical equations.
2. **Embedded Implementer** implements the logic in C++ source files.
3. **Simulation Verifier** writes tests and runs Pitaya closed-loop simulations to verify stability.
4. **Automated Reviewer** checks safety constraints and code quality.

If any compiler errors or simulation test failures occur during validation, the agent must fall back to the **Re-Planning** phase and analyze the logs rather than blindly retrying.

## 4. Repository & Mojito Enhancements
- **Proactive Improvement**: Agents are encouraged to propose and implement improvements or new features in the core `mojito::` namespace or common `pitaya::` library components (e.g., adding missing filter structures, PLLs, or coordinate transforms) rather than writing local workarounds. If a feature or library module is missing, specify the core addition in the implementation plan.

