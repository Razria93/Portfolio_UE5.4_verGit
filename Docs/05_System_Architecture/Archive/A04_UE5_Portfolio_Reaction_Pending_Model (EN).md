# Reaction Pending Model

## 1. Purpose

This document explains why `PendingReaction` existed in the previous Reaction structure,  
and why the pending flow should be removed under the Reaction Orchestration structure.

The main goals are:

- Clarify that the old pending flow came from implementing AI reaction first.
- Explain why the old pending consume flow does not match the natural Reaction execution flow.
- Clarify where Reaction execution ownership should move after Reaction Orchestration.
- Explain why BT should observe reaction state instead of starting reactions directly.


---

## 2. Why Pending Was Introduced

The old `PendingReaction` structure was introduced because Reaction was implemented from the AI side first.

At that point, Enemy reaction had to be controlled inside the BT flow.  
Therefore, the damage event did not execute reaction immediately.  
Instead, `ReactionComponent` stored a pending reaction, and a BT task consumed it later.

The old Enemy flow was close to the following.

```text
TakeDamage
-> ReactionComponent::TryRequestPendingDamageReaction()
-> Store PendingReactionContext
-> CBTTask_StartReaction
-> ReactionComponent::TryConsumePendingReaction()
-> ReactionComponent::TryExecuteReaction()
```

In this structure, BT became the direct owner of reaction execution.

Afterwards, the Player also tried to reuse the same reaction component flow.  
Because the Player does not have a BT task that can consume pending reaction,  
the Player needed another consume point.

As a result, the Player consumed pending reaction from Tick.

```text
TakeDamage
-> ReactionComponent::TryRequestPendingDamageReaction()
-> Store PendingReactionContext
-> Player Tick
-> ConsumePendingReaction()
-> ReactionComponent::TryConsumePendingReaction()
-> ReactionComponent::TryExecuteReaction()
```

Therefore, the old pending flow was not a reaction conflict policy.  
It was a deferred bridge created to fit reaction execution into the AI BT flow.


---

## 3. Limits of the Pending Structure

The old pending flow does not match the normal Reaction execution flow.

The basic Reaction flow should be:

```text
Action
-> Apply Damage
-> Take Damage
-> Reaction
```

An attacking action creates a hit, damage is applied, the target processes the damage,  
and reaction occurs from that result.

However, the old structure moved reaction execution ownership out to BT.  
To make that possible, reaction execution was delayed as pending instead of being handled from the damage processing point.

This can look acceptable for Enemy because the BT flow exists there.  
But for Player, it creates an unnatural structure.

```text
Player does not have BT
Player consumes pending reaction from Tick only to match the Enemy flow
An unnecessary frame delay and detour is introduced between damage event and reaction execution
```

In other words, pending did not represent the actual meaning of Reaction.  
It was a control workaround created so that BT could execute reaction.

This structure has the following limits.

```text
The final decision point of a reaction request is unclear
TakeDamage timing and reaction execution timing are unnecessarily separated
Player and Enemy have different consume points
BT acts like the execution owner of an external-event-based reaction
ReactionComponent owns request storage, execution availability checks, and execution application at the same time
```


---

## 4. Default Flow After Reaction Orchestration

Under Reaction Orchestration, the damage event becomes the direct entry point for reaction execution requests.

The recommended flow is:

```text
TakeDamage
-> ReactionOrchestrator::RequestReaction()
-> ResolveReactionContext()
-> ResolveReactionPolicy()
-> OrchestrateQuery()
-> ReactionComponent::ApplyReactionDecision()
-> CReaction execution
```

In this structure, Player Tick or BTTask no longer needs to consume a pending reaction.

The Player flow becomes:

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

The Enemy flow becomes the same.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> BT observes ExecutionState::Reaction
```

BT no longer starts reaction.  
BT observes the runtime state and waits or blocks other intent branches while reaction is active.


---

## 5. Responsibility Split

After Reaction Orchestration, the flow should not be built around pending.  
The better structure is to separate responsibilities between Orchestrator, Component, and Execution Object.

### ReactionOrchestrator

`ReactionOrchestrator` receives and interprets external-event-based reaction requests,  
then decides which reaction path is possible under the current runtime state.

Its main responsibilities are:

```text
Receive TakeDamage-based reaction requests
Resolve reaction type from damage result
Resolve reaction data and executor
Compare active reaction and incoming reaction
Produce Start / Interrupt / Ignore / Reject decisions
Dispatch decisions to ReactionComponent
```

The orchestrator does not execute reaction directly.  
It evaluates the reaction request and decides the execution path.

### ReactionComponent

`ReactionComponent` applies orchestrator decisions to the actual character runtime state.

Its main responsibilities are:

```text
Store ActiveReactionContext
Cache reaction executor instances
Apply StartReaction
Apply InterruptReaction
Handle movement / state / action abort side effects
Clear active context when reaction ends
```

The component manages reaction execution state.  
It should not store a reaction request as pending and wait for external consumption.

### CReaction

`CReaction` is the execution object that owns the actual reaction lifecycle.

Its main responsibilities are:

```text
Start montage playback
Handle reaction notify windows
Provide local runtime flags such as interruptible / cancelable
Handle Stop / End / cleanup flow
```

`CReaction` is the final execution object.  
It should not own request resolution or orchestration decisions.


---

## 6. BT Structure Change

After Reaction Orchestration, `CBTTask_StartReaction` should be removed  
or reduced to a compatibility no-op.

The old BT flow was close to:

```text
Blackboard bHasPendingReaction
-> CBTTask_StartReaction
-> ReactionComponent::TryConsumePendingReaction()
-> Execute reaction
```

The new BT flow should be close to:

```text
ExecutionState == Reaction
-> CBTTask_WaitEndReaction or decorator wait
-> Resume normal intent flow after reaction ends
```

Blackboard keys should move in the following direction.

```text
bHasPendingReaction
- Remove
- No longer needed after the pending consume bridge disappears

PendingReactionVersion
- Remove
- BT no longer needs to detect pending changes to start reaction

bHasActiveReaction
- Can remain
- May become duplicate state if it can be derived from ExecutionState::Reaction

EAIIntentState::HitReact
- Long-term removal candidate
- Hit reaction is external-event-based execution state, not AI intent
```


---

## 7. Suggested Implementation Steps

The first implementation pass should remove the old pending consume bridge.

```text
1. Remove PendingReactionContext from ReactionComponent
2. Remove TryRequestPendingDamageReaction from ReactionComponent
3. Remove TryConsumePendingReaction from ReactionComponent
4. Split TryExecuteReaction into StartReaction / InterruptReaction
5. Remove the Player Tick ConsumePendingReaction flow
6. Remove CBTTask_StartReaction or reduce it to a no-op
7. Reduce CBTTask_WaitEndReaction to active reaction waiting
8. Let TakeDamageComponent directly call ReactionOrchestratorComponent
9. Let ReactionOrchestratorComponent own the RequestReaction entry point
```

After that, the core task is not rebuilding pending.  
The core task is completing the orchestrator flow:

```text
1. ReactionOrchestratorComponent converts damage result into reaction request
2. ReactionOrchestratorComponent resolves reaction type / data / executor
3. ReactionOrchestratorComponent compares active reaction and incoming reaction
4. ReactionOrchestratorComponent produces Start / Interrupt / Ignore / Reject decisions
5. ReactionComponent updates runtime state and execution object from the decision
```


---

## 8. Conclusion

The old `PendingReaction` was not an essential Reaction state model.  
It was a deferred bridge created so reaction could be executed inside the AI BT flow.

That structure could work for Enemy, but it produced an unnatural Player flow  
where pending reaction had to be consumed from Tick.

The Reaction flow should be organized as:

```text
Action
-> Apply Damage
-> Take Damage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

The recommended direction for the current phase is:

```text
Remove the old pending bridge
Reduce BT to a reaction observer instead of a reaction executor
Let ReactionOrchestrator own reaction request decisions
Keep ReactionComponent focused on active runtime state and execution application
Keep CReaction responsible for montage lifecycle and local runtime flags
```
