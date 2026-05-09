# Action / Reaction Orchestration Structure Comparison

## 1. Purpose

This document explains how `ReactionOrchestrator` should be designed when using the existing `ActionOrchestrator` structure as a reference.  
It clarifies which parts can stay symmetric and which parts should intentionally differ.

The key points are as follows.

- Both Action and Reaction can receive external requests and return request results.
- Therefore, external API shapes such as request / result / reject reason can be structured symmetrically.
- However, the location and meaning of internal decisions must differ between Action and Reaction.
- This difference comes from the different problem each system is trying to solve.


---

## 2. Core Conclusion

The most important difference between Action and Reaction can be summarized as follows.

```text
Action decisions are centered on an action's own progression rules.
Reaction decisions are centered on conflict resolution between reactions.
```

In other words, Action is mainly about “how this action can execute now,”  
while Reaction is mainly about “how an incoming reaction should relate to the current reaction state.”

Therefore, the external request structure can remain similar,  
but the responsibility location for internal decisions should be different.


---

## 3. Nature of Action Orchestration

Action represents an intentional behavior started by the character.

Representative examples include:

```text
Equip
Unequip
ComboAttack
Guard
Dodge
```

Action requests start from Player input or AI decisions.

```text
PlayerInput / AI
-> ActionIntent
-> ActionOrchestrator
-> ActionComponent
-> CAction
```

The main responsibilities of `ActionOrchestrator` are:

- Check whether an external request can be accepted.
- Handle common gates such as Dead / Reaction / invalid component.
- Unify Player input and AI requests into a shared action request path.
- Convert intent into an executable `EActionType`.
- Delegate actual execution to `ActionComponent` and `CAction`.

For Action, internal decisions naturally belong more to `CAction` or `ActionComponent`.

This is because each action has its own progression rules.

```text
ComboAttack
- Current combo index
- Chain window
- Buffered input
- Whether the next combo step can be entered

Equip / Unequip
- Current equipped state
- Whether a transition is already in progress
- Toggle intent resolution

Guard / Dodge
- Valid start timing
- Cancel / interrupt availability
- Stamina / resource requirements
```

Therefore, Action decisions strongly depend on the internal state and progression rules of a specific action.


---

## 4. Nature of Reaction Orchestration

Reaction is not an intentional behavior started by the character.  
It is a response caused by external stimulus.

The first-pass input source is currently `TakeDamage`.

```text
TakeDamageResult
-> ReactionIntent
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

A reaction request needs to evaluate information such as:

```text
CommittedDamage
DeadState_Before
DeadState_After
ApplyDamageSpecKey
Current Active Reaction
Incoming Reaction
Priority
Interruptible Window
Incoming Executor Policy
Current Executor Policy
```

The important question for Reaction is not simply “can this reaction execute?”

The more important questions are:

- Is a new reaction coming in while another reaction is already active?
- Can the incoming reaction replace the active reaction?
- Should Dead reaction override Hit reaction?
- Is the current montage window interruptible?
- Does the current executor allow interruption?
- Does the incoming executor want to interrupt?
- Does the incoming reaction have stronger priority?

Therefore, Reaction decisions are centered less on a single reaction's internal progression rule,  
and more on coordinating the relationship between current reaction and incoming reaction.


---

## 5. Parts That Can Stay Symmetric

Both Action and Reaction can receive external requests and return results.

Therefore, the external API shape can be symmetric.

### Action

```cpp
EActionRequestResultType
EActionRequestRejectReason
FActionRequestResult
```

### Reaction

```cpp
EReactionRequestResultType
EReactionRequestRejectReason
FReactionRequestResult
```

This mapping is appropriate.

It lets callers read “how the request was handled” in a consistent way.

```text
Rejected
Ignored
Started
Chained / Interrupted / Cancelled
```

Common gate functions and result builder shapes can also remain symmetric.

```text
CanAcceptActionRequest()
BuildActionRequestResult()

CanAcceptReactionRequest()
BuildReactionRequestResult()
```

This kind of structural symmetry improves maintainability and readability.


---

## 6. Parts That Should Differ

The internal decision types for Action and Reaction should differ in both name and responsibility location.

### Action Internal Decision

For Action, the following types are natural.

```cpp
EActionExecutionDecision
FActionExecutionQuery
FActionExecutionResult
```

The word `Execution` fits because the decision is centered on  
“how this action can execute now.”

Examples include:

```text
Start
Chain
Enqueue
Interrupt
Ignore
Reject
```

In particular, `Chain` strongly depends on the progression rules of actions such as `ComboAttack`.

Therefore, action internal decisions should live closer to `CAction` or `ActionComponent`.


### Reaction Internal Decision

For Reaction, the following types are more appropriate.

```cpp
EReactionOrchestrationDecision
FReactionOrchestrationQuery
FReactionOrchestrationResult
```

The word `Orchestration` fits better than `Execution` because the decision is centered on  
conflict resolution between reactions rather than the execution possibility of a single reaction.

Examples include:

```text
Start
Interrupt
Cancel
Ignore
Reject
```

`Interrupt` and `Cancel` are not intrinsic progression rules of a single reaction.
They are results of coordinating the relationship between active reaction and incoming reaction or an external cancel request.

Therefore, reaction internal decisions should live in `ReactionOrchestrator`.


---

## 7. Role Difference Between CAction and CReaction

`CAction` and `CReaction` both look like execution units, but they differ in decision ownership.

### CAction

`CAction` owns the progression rules of its action.

```text
Can it start?
Can it chain?
Is the input window open?
Is the same action already active?
Should buffered input be accepted?
```

Therefore, `CAction` can actively participate in decision-making.


### CReaction

`CReaction` owns reaction montage lifecycle and local policy hooks.

```text
Montage Play
Montage Stop
Montage End Callback
Interruptible Flag
Cancelable Flag
WantToInterrupt
AllowInterruptionBy
```

However, the final decision should not be made by `CReaction` alone.

The final decision requires current / incoming / pending relationships, damage result, priority, and dead-state transition information.

Therefore, `CReaction` should provide local policy only.

```text
Current CReaction
-> Do I allow interruption now?

Incoming CReaction
-> Do I want to interrupt the current reaction?

ReactionOrchestrator
-> Combine both reaction policies with damage result and produce the final decision.
```


---

## 8. Difference in Input Nature

Action and Reaction also differ in input nature.

### Action Input

Action input is usually limited and intentional.

```text
Attack
Equip
Dodge
Guard
Move
```

The Player or AI clearly expresses “what it wants to do.”

Therefore, Action focuses on the execution conditions and feel of an intended behavior.


### Reaction Input

Reaction input is caused by external results and is irregular.

```text
Hit
Dead
GuardBreak
Launch
KnockDown
TrapHit
ScriptedReaction
```

The timing and frequency of these inputs are difficult to control.  
It is common for a new reaction to arrive while another reaction is already active.

Therefore, Reaction focuses less on input variety itself and more on resolving conflicts between irregular reaction requests.


---

## 9. Recommended Type Mapping

The recommended mapping between Action and Reaction types is:

```text
EActionRequestResultType
<-> EReactionRequestResultType

EActionRequestRejectReason
<-> EReactionRequestRejectReason

FActionRequestResult
<-> FReactionRequestResult

EActionExecutionDecision
<-> EReactionOrchestrationDecision

FActionExecutionQuery
<-> FReactionOrchestrationQuery

FActionExecutionResult
<-> FReactionOrchestrationResult
```

The important point is not to copy the name `ExecutionDecision` directly into Reaction.

For Reaction, the core of the decision is coordination rather than execution itself,  
so `EReactionOrchestrationDecision` is the more accurate name.


---

## 10. Recommended Structure

The target structure for Action is:

```text
ActionOrchestrator
-> Request gate
-> Intent resolve
-> Domain component routing

ActionComponent
-> Action storage
-> Current action state
-> ExecuteAction

CAction
-> Action-specific progress rule
-> Montage / notify / cleanup
```

The target structure for Reaction is:

```text
ReactionOrchestrator
-> Request gate
-> Damage result -> Reaction intent
-> Reaction type / data / executor resolve
-> Active / incoming conflict resolution
-> Decision generation

ReactionComponent
-> Reaction data / executor ownership
-> Active runtime state
-> ApplyReactionDecision
-> Movement / state / action abort application

CReaction
-> Montage lifecycle
-> Notify window runtime flags
-> Local interrupt / cancel policy hook
```

In short, the structure should remain symmetric in shape,  
but the decision center should differ between Action and Reaction.


---

## 11. Design Principles

The following principles should be preserved.

- External request / result shapes should be as symmetric as possible between Action and Reaction.
- Action internal decisions should be centered on an action's own progression rules.
- Reaction internal decisions should be centered on conflict resolution between reactions.
- `ReactionOrchestrator` should generate decisions but should not directly own montage lifecycle.
- `ReactionComponent` should own runtime state and decision application.
- `CReaction` should own execution timing and local policy hooks.
- The difference between Action and Reaction comes from their input nature and the problem each system solves.


---

## 12. Summary

Both Action and Reaction can have orchestration structures.

However, the two structures should not be identical.

```text
Action
= A problem of precisely determining execution possibility and progression rules for intended behavior.

Reaction
= A problem of reliably coordinating priority and conflicts between irregular reaction requests.
```

Therefore, the guiding rule is:

```text
Symmetric shape
Asymmetric internal decision ownership
```

With this rule, the `ActionOrchestrator` structure can be used as a reference  
while still handling Reaction's unique conflict-resolution problem clearly.


---
