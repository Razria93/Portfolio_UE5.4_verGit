# A13 UE5 Portfolio Execution Intervention Responsibility Decision

## 1. Purpose

This document defines how execution intervention should be separated between decision making and runtime application.

The core concern is that cancellation, interruption, replacement, and stop-only flows should not be expressed by expanding one-off `TryXAndY` APIs. They should be represented as an explicit intervention decision and consumed by the runtime component.

## 2. Previous System Shape

The existing execution flow was close to:

```text
Request
-> Candidate / context resolve
-> Local decision
-> Orchestration decision
-> Component apply
-> Executor lifecycle
```

In this shape, the executor could decide more than local executability, and the component could still participate in high-level execution decisions.

Cross-domain control was also incomplete.

```text
Action -> Reaction
Reaction -> Action
Reaction -> Reaction
Action -> Action
```

Each of these paths can require stopping an active execution before applying an incoming execution.

## 3. Problems And Limits

Local decision became too broad when it started to imply cancellation, interruption, and replacement behavior.

The target of cancellation was also unclear. A request may need to stop an active action, an active reaction, or only clear state without starting a new execution.

Window state and orchestration decision were also mixed. Whether an executor wants to intervene and whether an active executor allows the intervention are different questions.

## 4. Refactoring Direction

The responsibility split should be:

```text
Decision Level
-> decides whether the incoming execution can be considered

Intervention Level
-> decides whether an active execution must be stopped

Directive
-> describes what to stop, why to stop, who requested it, and what happens after stop

Component
-> consumes the directive and mutates runtime state
```

The recommended execution flow is:

```text
Request
-> Candidate
-> ExecutionContext
-> ExecutionDecisionQuery
-> ExecutionDecisionResult
-> InterventionQuery
-> InterventionDirective
-> Component apply
-> Executor lifecycle
```

## 5. Future Direction

The local decision should stay narrow. It should not decide the full cross-domain arbitration result.

Execution intervention queries should be shared between action and reaction so that the same model can describe action/action, action/reaction, reaction/action, and reaction/reaction conflicts.

Executor capability should also be split into incoming-side and active-side rules.

## 6. Conclusion

Execution intervention is not just a stop call. It is an orchestration decision that must produce a directive.

The component should consume the directive, and the executor should remain focused on lifecycle and local rules.
