# AI Reaction Observation Model

## 1. Purpose

This document defines how AI Behavior Tree should handle reactions after introducing reaction orchestration.

In the previous structure, AI reaction execution was strongly connected to pending consumption inside BT.

However, reaction is not an AI intent. It is an execution triggered by an external damage event.

Therefore, instead of letting BT directly consume reaction triggers, it is more appropriate to connect the damage pipeline directly to reaction orchestration and let BT observe active reaction state.

This document clarifies that direction and separates AI intent from reaction execution state.

---

## 2. Problem in the Previous Structure

The previous reaction structure was strongly coupled to BT and pending state because reaction was first implemented for AI.

The basic flow was as follows.

```text
TakeDamage
-> ReactionComponent stores pending reaction
-> Blackboard pending reaction key is updated
-> CBTTask_StartReaction
-> consume pending reaction
-> execute reaction
```

This structure was created to control reaction execution inside AI BT.

However, when the same pending consumption structure was applied to the player, an unnatural flow appeared where pending reaction was consumed in tick.

The natural reaction flow is as follows.

```text
Action
-> ApplyDamage
-> TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

Therefore, creating pending state just to route reaction execution through BT is not appropriate in the current structure.

---

## 3. Current Recommended Flow

The current recommended flow is as follows.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
-> update active reaction state
-> BT observes active reaction state
```

In this flow, BT does not execute the reaction.

BT observes whether a reaction is running, and if needed, waits, aborts the current task, or moves to another branch.

In other words, BT is not the reaction trigger source. It is a reaction state observer.

---

## 4. Difference Between Intent and Reaction

AI intent is a high-level decision state that represents what AI is trying to do.

Examples:

```text
Patrol
Chase
Engage
Attack
Retreat
```

Reaction, on the other hand, is an execution state that the body is forced to perform due to external stimulus.

Examples:

```text
Hit
Dead
Stagger
Knockback
GuardBreak
```

Therefore, reaction is closer to body state / execution state than a child option of AI intent.

Regardless of which intent AI currently has, reaction can occur when damage is received.

Therefore, it is more stable for the top-level BT branch to consider body state before intent.

---

## 5. Blackboard Role

Blackboard should be used to observe reaction state, not to store reaction execution requests.

Recommended roles are as follows.

```text
IsInReaction
-> whether an active reaction exists

ActiveReactionType
-> current active reaction type

IsDead
-> dead state
```

The following values should not be the main responsibility of blackboard.

```text
PendingReactionContext
PendingReactionVersion
PendingReactionRequest
```

These values were closer to what was needed to transfer reaction execution requests into BT.

In the current structure, reaction execution requests go directly to the orchestrator, so blackboard does not need to hold pending requests.

---

## 6. BT Task Role Change

`CBTTask_StartReaction` is no longer a core task in the current structure.

Reaction execution starts from `TakeDamage -> ReactionOrchestrator -> ReactionComponent`, not from a BT task.

Therefore, `CBTTask_StartReaction` should be removed or reduced to a legacy compatibility no-op.

On the other hand, `CBTTask_WaitEndReaction` still has meaning.

It can be used as an observer task that waits until the active reaction ends, rather than starting a reaction.

Recommended meaning:

```text
CBTTask_StartReaction
-> remove or reduce to legacy compatibility task

CBTTask_WaitEndReaction
-> wait until active reaction state ends
-> not the reaction execution owner
```

---

## 7. BT Service Role

BT Service is responsible for updating AI context and blackboard state.

After reaction orchestration, BT Service does not consume pending reactions.

Instead, it reads character component state and reflects observable state into blackboard.

Examples:

```text
ReactionComponent::IsActiveReaction()
-> Blackboard.IsInReaction

ReactionComponent::GetActiveReactionType()
-> Blackboard.ActiveReactionType

StateComponent / HealthComponent
-> Blackboard.IsDead
```

With this structure, BT can control branches based on reaction state without directly executing reactions.

---

## 8. Body State First Branching

Reaction is closer to body state than intent.

Therefore, the top-level BT branch should preferably follow this order.

```text
Dead?
-> dead branch

InReaction?
-> wait / blocked branch

Otherwise
-> intent branch
   -> patrol / chase / engage / attack
```

This structure ensures that if the body is dead or in reaction, that state is handled first regardless of AI intent.

This is more extensible than putting reaction inside intent state.

Even when parry, guard, dodge, or counter is added, body execution state and intent decision can remain separated.

---

## 9. Symmetry Between Player and AI

When reaction trigger starts from TakeDamage, both player and AI can use the same flow.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

Player does not consume pending reaction in tick.

AI does not consume pending reaction in BT.

Both manage active reaction state through the component, and upper-level systems observe it.

This structure keeps player and AI reaction execution as symmetrical as possible.

---

## 10. Conclusion

AI BT should be a reaction state observer, not the owner of reaction execution.

```text
Damage pipeline
-> produces reaction

ReactionOrchestrator
-> decides reaction execution and competition

ReactionComponent
-> manages active reaction state

BT
-> observes active reaction state and controls branches
```

With this structure, the pending reaction consumption flow can be removed, and player and AI can share the same reaction execution pipeline.

Therefore, reaction should be treated as body execution state rather than AI intent in the current project structure.
