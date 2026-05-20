# A18 UE5 Portfolio Execution Model Refactor History

## 0. Document Role

This is a history document for the execution refactor. It records how the execution model moved through several responsibility changes.

A19 focuses on difficulty and confusion points. A20 summarizes formal architecture issues. This document focuses on how the model arrived at the current shape.

## 1. Purpose

This document records the major transition points in the action / reaction orchestration refactor.

The important movement was:

```text
ActionComponent-centered execution decisions
-> Orchestrator-centered request interpretation
-> Component-centered runtime state ownership
-> Executor-centered montage lifecycle
-> Shared Action / Reaction execution model
```

## 2. Previous System Shape

Action execution was initially concentrated in the action component.

```text
Input / AI request
-> ActionComponent
-> action type decision
-> action data / executor resolve
-> executor start / chain / stop
```

Reaction had already developed a stronger orchestration shape because damage-driven reaction must compare incoming and active reactions.

```text
incoming reaction
-> stop active reaction if needed
-> start incoming reaction
```

This replacement-like model became insufficient when action and reaction started to affect each other.

## 3. Structural Pressures That Caused The Transition

Action required candidate resolution because intent and current state must be combined.

Cancel and interrupt moved from API names to stop directive semantics.

Replace thinking had to be split into stop and start because not every stop is followed by a new execution.

The terms Local / Policy / Orchestration became too broad and moved toward Snapshot / Decision / Intervention.

## 4. Responsibility Movement

The orchestrator became responsible for request interpretation, candidate/context resolution, decision construction, and intervention directive construction.

The component became responsible for active context, execution state, and directive consumption.

The executor became responsible for montage lifecycle, notify command handling, feedback, and local rules.

Intervention became a separate responsibility that compares incoming and active execution through a query.

Chain was clarified as not being an intervention. It is a sequential relationship.

## 5. Major Turning Points

The major turning points were:

```text
ActionComponent decision removal
Reaction replace thinking to directive thinking
Cancel / interrupt interpreted by meaning, not API name
Want and Allow split
Chain reinterpreted as reserve / consume
```

## 6. Future Direction

Execution should be further refined around relationship and apply mode.

Snapshot can be expanded with common body state, but it should not duplicate participant detail.

Combat subsystem integration may later coordinate interaction-level decisions without owning execution runtime.

## 7. Conclusion

The execution model was not decided at once. It emerged by repeatedly separating responsibilities across orchestrator, component, executor, and notify.

The current model is a foundation for more explicit execution relationship and intervention handling.
