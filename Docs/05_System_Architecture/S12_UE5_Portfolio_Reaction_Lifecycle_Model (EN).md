# Reaction Lifecycle Model

## 1. Purpose

This document defines the responsibilities of `ReactionOrchestrator`, `ReactionComponent`, and `CReaction` in the reaction execution lifecycle.

The core of the reaction orchestration work is not simply adding a new reaction request path.

The actual core is separating reaction execution decision, active state management, montage execution, stop / finish handling, and feedback requests.

Without a clear lifecycle model, the meanings of `Start`, `Interrupt`, `Cancel`, `Stop`, `Finish`, and `MontageEnd` can easily become mixed.

This document fixes those terms and responsibilities so that it can also be used later as a reference for improving the action lifecycle.

---

## 2. Overall Flow

The current reaction execution flow is as follows.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> Montage / Notify / Feedback
-> CReaction finish
-> ReactionComponent active state cleanup
```

The basic responsibility of each layer is as follows.

```text
ReactionOrchestrator
-> interpret the reaction request
-> resolve execution context / policy
-> decide Start / Interrupt / Cancel / Reject

ReactionComponent
-> apply orchestration decisions
-> manage active reaction state
-> call Start / Stop on the reaction executor
-> receive executor finish callbacks and clear active state

CReaction
-> play the actual montage
-> handle reaction control windows
-> create reaction feedback requests
-> convert Stop requests into finish reasons
-> perform Completed / Interrupted / Cancelled / Aborted finish
```

In short, the orchestrator is the decision layer, the component is the execution state management layer, and the reaction executor is the actual execution layer.

---

## 3. Orchestrator Responsibility

`ReactionOrchestrator` decides how a reaction request should be handled in the current body/runtime state.

Its main responsibilities are as follows.

- Convert `TakeDamage` based requests into reaction context.
- Determine reaction type from damage result.
- Resolve reaction data and executor.
- Evaluate competition between the current active reaction and the incoming reaction.
- Combine resolved policy and executor hooks to produce a decision.
- Dispatch the decision to `ReactionComponent`.

The orchestrator does not execute reactions directly.

It also does not play montages or clear active state directly.

This keeps execution state and executor control responsibility in `ReactionComponent`.

---

## 4. Component Responsibility

`ReactionComponent` applies the decision produced by the orchestrator as an actual execution state transition.

Its main responsibilities are as follows.

- Store the active reaction context.
- Resolve the active reaction executor.
- Convert Start decision into active reaction start.
- Convert Interrupt decision into stopping the active reaction and starting the incoming reaction.
- Convert Cancel decision into stopping the active reaction and subsequent handling.
- Clear stale active state.
- Clear active reaction state when the executor reports finish.

`ReactionComponent` is not the final judge of whether a reaction can execute.

Execution availability and competition are decided by the orchestrator.

The component is responsible for applying decisions and keeping active state consistent.

---

## 5. CReaction Responsibility

`CReaction` is the reaction executor.

Therefore, it owns the actual execution and internal execution state.

Its main responsibilities are as follows.

- Play the reaction montage.
- Request reaction start feedback.
- Open and close reaction control windows.
- Handle reaction feedback notifies.
- Hold interruptible / cancelable state.
- Provide local interruption / cancel rules.
- Confirm finish reason when Stop is requested.
- Request feedback for the finish reason.
- Notify the component after finish.

`CReaction` does not manage the active reaction slot directly.

The active reaction slot is managed by `ReactionComponent`.

Therefore, the executor should only report that it has finished and why it finished.

---

## 6. Meaning of Start / Interrupt / Cancel

`Start`, `Interrupt`, and `Cancel` are command-style entries for controlling reaction execution state from outside.

```text
Start
-> start the incoming reaction when no active reaction exists

Interrupt
-> stop the active reaction due to an external cause and replace it with the incoming reaction

Cancel
-> stop the active reaction due to user intent or upper-level state transition
```

Therefore, `Start` and `Interrupt` should not be merged into the same function.

They can both eventually start an executor, but their meaning and preconditions are different.

`Start` is an entry that requires no active reaction.

`Interrupt` is an entry that requires an active reaction and transitions to a new reaction after stopping it.

`Cancel` can be extended as an entry that stops the current active reaction regardless of whether a new incoming reaction starts.

---

## 7. Difference Between Stop and Finish

`Stop` is an API that requests a running reaction executor to stop.

`Finish` is the stage where the executor confirms the finish reason and performs the finish procedure.

They are not the same concept.

```text
Stop
-> external request to stop the executor
-> passes stop reason such as Interrupted / Cancelled / Aborted

Finish
-> executor confirms finish reason
-> requests feedback
-> clears runtime state
-> reports finish to the component
```

In this structure, `ReactionComponent` calls `Stop`, and `CReaction` finalizes the termination through `FinishInterrupted`, `FinishCancelled`, or `FinishAborted`.

Therefore, the stop request belongs to the component, and finish confirmation belongs to the executor.

---

## 8. Role of MontageEnd

`MontageEnd` is a callback for detecting whether a montage ended normally.

In the current structure, system-driven stop is immediately converted into a finish reason in `UCReaction::Stop()`.

Therefore, the montage interrupted callback that occurs after Stop does not confirm reaction finish again.

The recommended meaning is as follows.

```text
MontageEnd with bInterrupted == false
-> handle as normal completed finish

MontageEnd with bInterrupted == true
-> treat as an already handled stop flow
-> do not perform finish again
```

This prevents reaction state cleanup from depending on montage blend-out timing or engine callback order.

Reaction state is gameplay state, so explicit system commands should take priority over animation callbacks.

---

## 9. Finish Reason

Current reaction finish reasons are separated as follows.

```text
Completed
-> montage completed normally

Interrupted
-> stopped by external reaction or damage response

Cancelled
-> stopped by cancel command or upper-level state transition

Aborted
-> stopped because runtime state was invalid or execution could not continue
```

`Interrupted` and `Cancelled` are intentionally separated.

Interrupted is closer to the current reaction being displaced by an external cause.

Cancelled is closer to intentionally dropping the current execution through user input, dodge, counter, or body state transition.

Aborted is closer to cleanup for a state where execution cannot be maintained, rather than a normal gameplay decision.

---

## 10. Feedback Responsibility

In the reaction lifecycle, feedback requests are made by the executor.

This is because feedback timing is tightly coupled to events inside reaction execution.

Examples:

```text
ReactionStart
-> CReaction::Start()

ReactionCompleted
-> CReaction::FinishCompleted()

ReactionInterrupted
-> CReaction::FinishInterrupted()

ReactionCancelled
-> CReaction::FinishCancelled()

WindowBegin / WindowEnd
-> reaction feedback notify state

Notify
-> reaction feedback point notify
```

`ReactionComponent` does not build feedback directly.

The component only acts as a bridge that finds the active executor and forwards notify events.

The actual feedback request is created by `CReaction` based on its active context.

---

## 11. Conclusion

The core of the reaction lifecycle is separating decision, state management, and execution.

```text
ReactionOrchestrator
-> decides

ReactionComponent
-> manages execution state

CReaction
-> executes and confirms finish
```

With this structure, competition between reactions is handled by the orchestrator, the active reaction slot is managed by the component, and montage / notify / feedback is handled by the executor.

Therefore, reaction orchestration is not simple request forwarding. It is a structure for separating runtime execution lifecycle in a stable way.
