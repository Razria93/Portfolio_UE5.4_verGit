# S13 UE5 Portfolio Action Runtime Lifecycle Decision

## 1. Purpose

This document records how runtime responsibilities are split between `ActionComponent` and `CAction` after the action orchestration refactor.

The key decision is that the orchestrator produces decisions, the component owns active runtime state and side effects, and `CAction` executes montage, notify, and feedback lifecycle behavior.

## 2. Previous System Shape

Previously, action data was closer to being injected into executors during initialization, and executors kept data sets internally.

Anim notifies also called action-specific APIs directly in some places, while the component knew parts of concrete action subclass behavior.

The representative flow was:

```text
ActionComponent
-> run active action
-> use CAction internal data
-> call notify-specific APIs
-> component and executor both partially manage active state
```

This made it harder to reuse one executor class across multiple action data entries, and it risked growing component APIs whenever montage notify commands increased.

## 3. Problems And Limits

The first limitation was that executors held definition data too long.

A `ComboAttack` executor is an execution object. The combo index and montage data to run should come from the resolved request.

Injecting action data lists during initialization weakens executor reuse and data separation.

The second limitation was that notify command flow was scattered.

Features that open and close over time, such as chain windows, collision windows, and hit context windows, are better modeled as notify states rather than one-shot notifies.

The third limitation was unclear active-state ownership between component and executor.

When an executor advances a combo chain, the component must be notified clearly so the executor active data and component active data do not diverge.

## 4. Refactoring Direction And Content

After the refactor, `ActionComponent` owns action runtime state.

Its main responsibilities are:

```text
ActionComponent
-> manages ActionDataMap and ActionExecutorMap
-> manages active action context
-> applies orchestration results to runtime APIs
-> handles action state enter/exit and movement side effects
-> routes notify commands to the active executor
```

`CAction` acts as the execution object.

Its main responsibilities are:

```text
CAction
-> performs Start / ApplyChain / Stop / Complete lifecycle
-> plays and stops montages
-> caches active runtime data only while running
-> interprets notify commands
-> builds feedback requests
-> provides local-level decisions
```

Action execution is organized as:

```text
ActionComponent::ApplyActionDecision
-> TryStartAction / TryChainAction / TryReplaceAction
-> StartActiveActionInternal / ChainActiveActionInternal / StopActiveActionInternal
-> CAction::Start / CAction::ApplyChain / CAction::Stop
```

Notify flow is routed through the component into the active executor.

```text
AnimNotify / AnimNotifyState
-> ActionComponent handle API
-> active CAction
-> HandleNotifyCommand / HandleNotifyFeedback
-> executor-specific behavior
```

This lets notifies avoid knowing concrete action subclasses and keeps the component responsible only for active executor routing.

## 5. Future Direction

Action notify commands are now centered around common commands.

If action-specific commands grow later, the system should choose one of the following:

```text
Keep common commands
-> suitable for shared windows, feedback, completion, and hit context

Extend executor-specific command handling
-> action subclasses interpret commands and override only needed behavior

Add action-specific notifies
-> only for events meaningful to one specific action type
```

At the current stage, routing notify commands to the active executor is preferable to increasing component APIs per action type.

Reaction lifecycle can be aligned with this action lifecycle in the next branch.

## 6. Conclusion

This refactor makes `ActionComponent` the runtime state owner and narrows `CAction` into an executor and local rule provider.

Action data is now resolved per request and passed to the executor, while the executor caches only the runtime data needed during execution.

This improves executor reuse, notify routing consistency, and responsibility separation between component and executor.
