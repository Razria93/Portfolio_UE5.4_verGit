# A14 UE5 Portfolio Execution Layer Responsibility Decision

## 1. Purpose

This document defines the responsibility boundaries between Decision Level, Intervention Level, Snapshot, Component, Executor, and Notify in the action / reaction execution pipeline.

The goal is to prevent execution conditions, current state lookup, collision arbitration, runtime state mutation, and montage lifecycle handling from being mixed across layers.

## 2. Previous System Shape

The previous flow was:

```text
Request
-> Candidate
-> ExecutionContext
-> Local Level
-> Orchestration Level
-> Component Apply
-> Executor Lifecycle
```

The executor and component had already been partially separated, but the boundary between local decision and orchestration decision was not explicit enough.

```text
Executor
-> Start / Stop / Complete
-> montage lifecycle
-> notify window
-> feedback

Component
-> active execution state owner
-> orchestration result consumer
-> directive consumer

Orchestrator
-> request interpretation
-> candidate / context resolve
-> final execution result construction
```

## 3. Problems And Limits

If the local layer directly returns values such as `Start`, `Cancel`, or `Interrupt`, the executor effectively decides not only its own executability but also its conflict relationship with the active execution.

Current body state, active action, and active reaction state were also easy to query from multiple layers, which makes decision criteria scattered.

Intervention decision and runtime application were also mixed. The system needs to answer:

```text
What should stop?
Why should it stop?
Should the incoming execution start after stop?
What happens if stop fails?
```

These questions should be resolved by orchestration and consumed by the component.

The terms `Local Level` and `Orchestration Level` were useful during transition, but they were too broad. The later model should be read as:

```text
Decision Level
-> incoming execution executability and relationship

Intervention Level
-> active/incoming conflict and stop directive

Result Composition
-> common execution result converted to action/reaction result
```

## 4. Refactoring Direction

The recommended responsibility split is:

```text
Decision Level
-> evaluates executability and relationship

Snapshot
-> captures current body / execution state

Intervention Level
-> resolves active/incoming conflict
-> decides what to stop, why, and what to do after stop

Result Composition
-> composes domain-specific action/reaction result

Component
-> consumes directives and results

Executor
-> owns montage lifecycle, notify windows, feedback, and local rules
```

## 5. Examples

Combo chain should be handled as a sequential relationship, not as an intervention.

Dodge during hit reaction should be treated as action-driven cancellation of an active reaction.

A dead reaction can forcefully interrupt other active executions because it has stronger semantic priority.

## 6. Future Direction

Action and reaction orchestrators should both use shared execution snapshot and intervention query structures.

Dodge should be the first practical action-to-reaction cancel case used to validate this model.

Future parry, guard, counter, and execution flows should connect to combat interaction logic without breaking execution responsibility boundaries.

## 7. Conclusion

Execution architecture should separate executability, current state, conflict arbitration, runtime application, and montage lifecycle.

This gives a stable expansion path for actions and reactions with complex conditions.
