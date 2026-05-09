# Reaction Orchestration Core Architecture Decision

## 1. Purpose

This document explains why the reaction execution flow was separated into `ReactionOrchestrator -> ReactionComponent -> CReaction`, and summarizes the core decisions of the current implementation.

It reorganizes the archive documents `A01`, `A03`, and `A07` into a single decision flow.

---

## 2. Background

### 2.1 Existing Flow

The previous reaction structure was centered on `PendingReaction`.

The flow was roughly as follows.

```text
TakeDamage
-> store PendingReaction
-> consume from Player Tick or Enemy BT
-> TryExecuteReaction()
-> QueryReplaceReaction()
-> execute or replace CReaction
```

### 2.2 Problem

The problem was that decision and execution were distributed to external consumers after `PendingReaction`.

Enemy consumed pending reaction from a BT task, while Player consumed pending reaction from Tick.

As a result, the same damage event led to different reaction execution paths for Player and Enemy.

### 2.3 Cause

The cause was that reaction was implemented from the AI side first, and the BT-controlled execution model was created first.

Because Player does not have BT, a separate Tick consume path was added to match the same pending model.

Reaction is not an intent action selected by Player or AI.

It is closer to a body execution response caused by an external damage event.

Therefore, after damage is accepted or committed, the reaction request should be evaluated and executed through the same damage-driven execution pipeline for both Player and Enemy.

### 2.4 Direction

The direction is to reorganize the flow and responsibilities from damage event to reaction execution into an orchestration pipeline.

```text
ReactionOrchestrator
-> interprets request and resolves reaction conflict

ReactionComponent
-> owns active runtime state and applies decisions

CReaction
-> owns actual montage lifecycle and local execution rules
```

Reaction orchestration separates request interpretation, conflict resolution, decision application, and actual reaction lifecycle into clear responsibility units.

As a result, the previous pending-driven execution model becomes a damage-driven orchestration model, and Player and Enemy share the same reaction execution flow.

---

## 3. Decision

Reaction execution flow is organized as follows.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

Responsibilities are separated as follows.

```text
ReactionOrchestrator
-> interprets request
-> resolves context and policy
-> compares active reaction and incoming reaction
-> generates Start / Interrupt / Cancel / Ignore / Reject decisions

ReactionComponent
-> owns active reaction state
-> applies orchestrator decision to runtime state
-> resolves and controls executor instance
-> handles action abort / movement lock / execution state transition

CReaction
-> runs actual montage lifecycle
-> handles control windows and feedback notify
-> provides local interrupt / cancel hooks
-> resolves stop reason into finish reason
```

In this structure, `ReactionOrchestrator` is the center of decision, `ReactionComponent` is the center of runtime state application, and `CReaction` is the center of actual execution.

---

## 4. Request Flow

The current reaction request flow is as follows.

```text
CTakeDamageComponent
	-> builds FDamageReactionRequest

-> UCReactionOrchestratorComponent::RequestReaction()
	-> CanAcceptReactionRequest()
	-> ResolveReactionContext()
	-> ResolveReactionPolicy()
	-> BuildOrchestrationQuery()
	-> OrchestrateQuery()
	-> DispatchReactionDecision()

-> UCReactionComponent::ApplyReactionDecision()
-> CReaction::Start() / Stop()
```

1. `RequestReaction()`
	- Public entry point for reaction requests.

2. `ResolveReactionContext()`
	- Converts damage result into reaction execution context.

3. `ResolveReactionPolicy()`
	- Interprets what authority the current request has.

4. `OrchestrateQuery()`
	- Resolves conflict between active reaction and incoming reaction.

5. `DispatchReactionDecision()`
	- Applies the decision to the reaction component.

---

## 5. Decision Types

Current reaction orchestration decisions are:

```text
Start
Interrupt
Cancel
Ignore
Reject
```

1. `Start`
	- Starts incoming reaction when there is no active reaction.

2. `Interrupt`
	- Stops active reaction due to an external cause and replaces it with incoming reaction.

3. `Cancel`
	- Intentionally stops active reaction from an internal request.
	- A follow-up policy such as `dodge action` or `execution reaction` may run after this.

4. `Ignore`
	- The request is valid, but is not processed in the current state.

5. `Reject`
	- The request itself or execution condition is invalid.

`ReplacePending`, `Enqueue`, and queue-based processing are excluded from the first implementation scope.

---

## 6. Lifecycle Semantics

Reaction lifecycle terms are fixed as follows.

```text
Start
-> starts executor when there is no active reaction

Interrupt
-> stops active reaction due to an external cause and starts incoming reaction

Cancel
-> intentionally stops active reaction

Stop
-> component requests the executor to stop

Finish
-> executor finalizes stop reason and notifies component

MontageEnd
-> callback that detects normal montage completion
```

`Stop` and `Finish` are not the same concept.

`Stop` is an external control request, while `Finish` is the stage where the executor clears runtime state and finalizes the finish reason.

Therefore, `ReactionComponent` calls `Stop`, and `CReaction` finalizes the result as one of `FinishCompleted`, `FinishInterrupted`, `FinishCancelled`, or `FinishAborted`.

---

## 7. Current Implementation

The core classes in the current implementation are:

```text
UCReactionOrchestratorComponent
	-> RequestReaction()
	-> ResolveReactionContext()
	-> ResolveReactionPolicy()
	-> OrchestrateQuery()
	-> DispatchReactionDecision()

UCReactionComponent
	-> ApplyReactionDecision()
		- TryStartReaction()
		- TryInterruptReaction()
		- TryCancelReaction()
	-> StopActiveReactionInternal() for Interrupt / Cancel
	-> StartActiveReactionInternal()
	-> EndActiveReactionInternal()

UCReaction
	-> Start()
	-> Stop() for Interrupt / Cancel
	-> FinishCompleted()
	-> FinishInterrupted()
	-> FinishCancelled()
	-> FinishAborted()
	-> OnReactionControlWindowBegin()
	-> OnReactionFeedback()
```

Guard / parry / counter / launch / knockdown / queue are separated as future extension scope.

---

## 8. Consequences

Benefits:

```text
Reaction entry point after TakeDamage is unified
Player and AI share the same reaction execution pipeline
ReactionComponent becomes an active runtime state manager instead of pending storage
CReaction can focus on montage / notify / feedback lifecycle
Reaction conflict can be resolved consistently by the orchestrator
```

Notes:

```text
ReactionOrchestrator must read active state and executor information from ReactionComponent
DataAsset / DataProvider currently inside component should be separated in the long term
Concrete cancel policy for Cancel decision should be defined later
```

---

## 9. Follow-up

Follow-up candidates:

```text
Move reaction definition data to DataAsset or DataProvider layer
Reflect guard / parry / poise / super armor in ResolveReactionPolicy
Define actual request source and policy for cancel decision
Refactor action orchestration using the responsibility separation defined here
```

---

## 10. Related Documents

Related detailed documents:

```text
A01_UE5_Portfolio_Action_Reaction_Orchestration_Comparison
A03_UE5_Portfolio_Execution_Orchestration_API_Model
A07_UE5_Portfolio_Reaction_Lifecycle_Model
```

---
