# S17 UE5 Portfolio Execution Layer Responsibility Decision

## 1. Purpose

This document defines the responsibility boundaries between Orchestrator, Component, Executor, and Notify in the action / reaction execution pipeline.

The core goal is to separate execution decision making from runtime application by using Decision Level, Intervention Level, Component Apply, and Executor Lifecycle.

## 2. Previous System Shape

The previous action / reaction execution flow was close to a component-centered model.

```text
Request
-> Component decision
-> Executor execution
-> Notify
-> Component state update
```

The component owned active state, but it also interpreted requests, evaluated execution conditions, called executors, routed notify events, and handled stop / finish logic.

Action and reaction both execute montage-based execution objects, but they evolved differently.

```text
Action
-> input / AI intent driven
-> heavy component-side decision logic

Reaction
-> damage event driven
-> active/incoming reaction comparison developed earlier
```

## 3. Problems And Limits

When the component owns too much responsibility, the boundary becomes unclear once action and reaction can intervene in each other.

Typical problems are:

```text
Component makes decisions that should belong to the orchestrator
Executor mutates state that should be consumed by the component
Executor starts to understand cross-domain flow beyond local rules
Notify routing is unclear between component and executor
```

The earlier Local / Policy / Orchestration terminology also became too broad.

Local looked like it could decide cancel, interrupt, and chain semantics. Orchestration looked like it could mean decision conversion, intervention resolution, directive composition, and result composition all at once.

## 4. Refactoring Direction

Responsibilities should be split as:

```text
Decision Level
-> evaluates incoming execution executability and relationship

Intervention Level
-> resolves active/incoming conflicts
-> decides stop target, stop reason, and after-stop behavior

Component Apply
-> owns active context and execution state
-> consumes directives and execution results

Executor Lifecycle
-> owns montage play / stop / complete
-> handles notify commands, feedback, and local rules
```

Object responsibilities are:

```text
Orchestrator
-> request interpretation
-> candidate/context resolution
-> decision/intervention/directive composition

Component
-> active context owner
-> execution state mutation
-> directive consumption

Executor
-> montage lifecycle
-> notify command handling
-> feedback
-> local execution rule

Notify
-> timing event delivery to the running component/executor
```

## 5. Trial And Error

At first, aligning action and reaction API names looked like the key to symmetry.

The real problem was not naming. It was that decision and runtime execution were mixed.

Policy booleans such as `bCanStart`, `bCanChain`, `bCanInterrupt`, and `bCanCancel` also became thin filters over decisions that had already been made.

The clearer direction was to pass current state as a snapshot and let decision / intervention layers use it for their own responsibilities.

## 6. Conclusion

The execution pipeline should be:

```text
Intent / Event
-> Candidate
-> ExecutionContext
-> Snapshot
-> Decision
-> Intervention
-> Directive
-> Component Apply
-> Executor Lifecycle
```

This prevents the component from deciding everything and prevents the executor from coordinating cross-domain execution flow.
