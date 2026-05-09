# Reaction Pending Removal and AI Observation Architecture Decision

## 1. Purpose

This document explains why the previous reaction pending structure was removed, and why AI Behavior Tree was reduced from a reaction execution owner to an active reaction state observer.

It reorganizes the archive documents `A04` and `A09` into a single decision flow.

---

## 2. Background

Reaction was implemented from the AI side first, and the BT flow became strongly coupled with pending reaction.

The previous Enemy reaction flow was roughly as follows.

```text
TakeDamage
-> store pending reaction in ReactionComponent
-> update Blackboard pending key
-> CBTTask_StartReaction
-> consume pending reaction
-> execute reaction
```

This structure was created to control reaction execution timing inside BT.

However, Player does not have BT, so Player had to consume pending reaction from Tick to match the same pending model.

As a result, pending was closer to a bridge for AI BT execution than an essential reaction state model.

The orchestration structure defined in S06 removes this bridge and routes reaction requests directly into the orchestration pipeline after damage event.

This document is the follow-up decision for removing pending and reducing the AI BT role.

---

## 3. Problem

The core problem of the previous pending model was distributed ownership of reaction execution.

The same damage event depended on different consumers for actual reaction execution.

```text
Enemy
-> BT task consumes pending reaction

Player
-> Tick consumes pending reaction
```

The natural reaction flow is damage event to reaction execution.

```text
Action
-> ApplyDamage
-> TakeDamage
-> Reaction
```

The previous structure created pending state to route reaction execution through BT or Tick.

Problems:

```text
Damage processing timing and reaction execution timing are unnecessarily separated
Player needs a separate Tick consume flow
Enemy BT task behaves like the reaction execution owner
ReactionComponent owns both pending storage and execution manager roles
Reaction execution path is not symmetric between Player and Enemy
```

Therefore, pending should not remain the central model in the current reaction orchestration structure.

---

## 4. Decision

Remove the reaction pending consume structure, and route reaction request directly from `TakeDamage` to `ReactionOrchestrator`.

Current flow:

```text
TakeDamage
-> ReactionOrchestrator::RequestReaction()
-> ResolveReactionContext()
-> ResolveReactionPolicy()
-> OrchestrateQuery()
-> ReactionComponent::ApplyReactionDecision()
-> execute CReaction
```

Player and Enemy share the same reaction execution pipeline.

```text
Player
-> TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction

Enemy
-> TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> BT observes active reaction state
```

BT does not execute reaction.

BT reads active reaction state, waits, blocks branches, or controls intent flow.

---

## 5. AI Observation Model

AI BT is not a reaction trigger source.

It is a reaction state observer.

Recommended top-level branch order:

```text
Dead?
-> dead branch

InReaction?
-> wait / blocked branch

Otherwise
-> intent branch
   -> patrol / chase / engage / attack
```

This structure prioritizes body state over intent.

Even if AI has an intent, dead or reaction body state should be handled first.

Therefore, reaction is closer to body execution state than AI intent.

---

## 6. Blackboard Role

Blackboard is used to observe reaction state, not to store reaction request.

The current key is:

```text
bIsActiveReaction
```

This value is updated from `ReactionComponent::IsActiveReaction()`.

The following pending request keys are no longer central to the current structure.

```text
PendingReactionContext
PendingReactionVersion
bHasPendingReaction
```

These keys were needed when BT consumed pending requests.

In the current structure, reaction execution request goes directly to orchestrator, so blackboard does not need to store pending requests.

---

## 7. BT Task Role

`CBTTask_StartReaction` is no longer the reaction execution owner.

Its role is reduced to legacy compatibility or active reaction state checking.

`CBTTask_WaitEndReaction` still has meaning.

It can be used as an observer task that waits until active reaction ends.

```text
CBTTask_StartReaction
-> not a reaction execution owner
-> reduced to active reaction check or compatibility task

CBTTask_WaitEndReaction
-> waits until active reaction state becomes false
-> not a reaction execution owner
```

---

## 8. Current Implementation

Current implementation:

```text
CTakeDamageComponent
-> creates FDamageReactionRequest after accepted damage
-> calls ReactionOrchestratorComp_Cached->RequestReaction()

UCReactionComponent
-> has no PendingReactionContext
-> provides IsActiveReaction()
-> applies decisions through ApplyReactionDecision()

BT Service
-> reads ReactionComponent::IsActiveReaction()
-> updates Blackboard bIsActiveReaction

CBTTask_WaitEndReaction
-> observes bIsActiveReaction
```

Player Tick based pending consume flow is removed.

Enemy BT based pending consume flow is also removed.

---

## 9. Consequences

Benefits:

```text
Player and Enemy reaction execution paths become identical
Reaction execution timing is naturally connected to damage processing flow
BT becomes a state observer instead of execution owner
ReactionComponent no longer needs pending storage
Future body-state based BT branch design becomes clearer
```

Notes:

```text
Designs where BT directly starts reaction are no longer central
AI intent branch blocking during reaction depends on blackboard active reaction state
Future queue / buffer model should be designed separately from pending
```

---

## 10. Follow-up

Follow-up candidates:

```text
Clarify BT top-level body state branch order as dead / reaction / intent
Review active reaction state usage in AI combat availability calculation
If future queue / buffered reaction is needed, design it separately from pending
```

---

## 11. Related Documents

Related detailed documents:

```text
A04_UE5_Portfolio_Reaction_Pending_Model
A09_UE5_Portfolio_AI_Reaction_Observation_Model
```

---
