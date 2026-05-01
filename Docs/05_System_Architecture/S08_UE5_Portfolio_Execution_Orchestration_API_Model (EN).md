# Execution Orchestration API Model

## 1. Purpose

This document organizes the execution orchestration flow into  
`Request -> Resolve -> Policy -> Orchestrate -> Dispatch -> Execute` stages,  
so that API responsibilities stay clear across Action / Reaction execution systems.

The main goals are:

- Clarify how an Orchestrator receives external requests and resolves concrete execution candidates.
- Separate policy interpretation from current-state orchestration.
- Keep Components responsible for applying orchestration results to runtime state.
- Keep execution objects such as `CAction` and `CReaction` responsible for actual lifecycle and local policy hooks.


---

## 2. Problem Recognition

The following concepts can easily become mixed in the current API structure.

```text
Request
Resolve
Decide
Apply
Execute
Pending
Replace
Finish
```

Especially in Reaction Orchestration, several questions can end up inside one function.

```text
What should be executed?
How does this execution candidate want to be handled?
Can it run under the current active / pending state?
Who mutates the actual runtime state?
Who owns the actual montage lifecycle?
```

When these questions are mixed, the boundary between Orchestrator and Component becomes unclear.

The execution flow should therefore be separated into explicit stages.


---

## 3. Recommended Execution Flow

Execution orchestration should follow this structure.

```text
[Orchestration]
RequestExecution()
-> ResolveExecutionContext()
-> ResolveExecutionPolicy()
-> OrchestrateExecution()
-> DispatchExecutionDecision()

[Component]
-> Component Execute / Pending / Replace / Enqueue

[Execution]
-> Execution Object Begin / Stop / End
```

Each stage has the following meaning.

```text
RequestExecution
- External entry API
- Receives intent and payload
- Returns request result

ResolveExecutionContext
- Concretizes what should be executed
- Builds execution candidate from intent / payload

ResolveExecutionPolicy
- Interprets how the execution candidate wants to be handled
- Describes execution tendency and policy, not final availability

OrchestrateExecution
- Determines whether it is possible under current runtime state
- Compares active / pending / current state with policy and produces final decision

DispatchExecutionDecision
- Delegates the orchestration decision to Component API
- Actual runtime mutation happens in the Component
```


---

## 4. Orchestrator Responsibility

The Orchestrator does not directly own execution lifecycle.

The Orchestrator owns the following responsibilities.

```text
1. Receive external request
2. Apply common request gate
3. Resolve execution context
4. Resolve execution policy
5. Generate orchestration decision by referencing current runtime state
6. Dispatch decision to component
7. Return request result
```

In other words, the Orchestrator is not the object that directly performs execution.  
It evaluates execution candidates and decides which path they should take.


---

## 5. Component Responsibility

The Component applies the Orchestrator's decision to actual character runtime state.

The Component owns the following responsibilities.

```text
1. Manage active / pending / queued execution state
2. Manage executor instance cache
3. Apply Start / Pending / Replace / Enqueue decisions
4. Apply side effects such as movement / state / action abort
5. Handle execution finish
```

Component APIs should correspond to Orchestrator decisions.

```text
StartExecution()
SetPendingExecution()
ReplaceActiveExecution()
EnqueueExecution()
FinishExecution()
```

Short names such as `Execute`, `Pending`, and `Replace` are possible,  
but explicit names such as `StartExecution`, `SetPendingExecution`, and `ReplaceActiveExecution` make API meaning clearer.


---

## 6. Execution Object Responsibility

Execution objects such as `CAction` and `CReaction` own actual lifecycle and local policy hooks.

They can commonly own:

```text
Begin
Stop
End
Cleanup
Tick
Notify window handling
Local policy hook
```

Reaction local policy hooks include:

```cpp
WantToInterrupt()
AllowInterruptionBy()
WantToCancel()
AllowCancelBy()
```

Execution objects should not usually create the final orchestration decision alone.  
Instead, they should provide local policy that the Orchestrator can use during decision-making.


---

## 7. Reaction API Model

Reaction Orchestration should move toward the following API shape.

### ReactionOrchestrator

```cpp
FReactionRequestResult RequestReaction(const FDamageReactionRequest& InRequest);

bool ResolveReactionContext(
	const FDamageReactionRequest& InRequest,
	FReactionContext& OutContext,
	EReactionType& OutReactionType,
	EReactionRequestRejectReason& OutRejectReason);

FReactionExecutionPolicy ResolveReactionPolicy(
	const FReactionContext& InContext,
	EReactionType InReactionType) const;

FReactionOrchestrationResult OrchestrateReaction(
	const FReactionContext& InContext,
	const FReactionExecutionPolicy& InPolicy) const;

void DispatchReactionDecision(const FReactionOrchestrationResult& InResult);
```

### ReactionComponent

```cpp
bool StartReaction(const FReactionContext& InContext);
bool SetPendingReaction(const FReactionContext& InContext);
bool ReplaceActiveReaction(const FReactionContext& InContext);
bool EnqueueReaction(const FReactionContext& InContext);
void FinishReaction();

const FReactionContext& GetPendingReactionContext() const;
const FReactionContext& GetActiveReactionContext() const;
```

### CReaction

```cpp
bool Begin(const FReactionData& InData);
void Stop(EReactionStopReason InReason);
void End(bool bInterrupted);

bool WantToInterrupt(const FReactionQueryContext& InContext) const;
bool AllowInterruptionBy(const FReactionQueryContext& InContext) const;
```


---

## 8. ReactionExecutionPolicy

`ResolveReactionPolicy()` can use a dedicated policy struct.

Expected shape:

```cpp
USTRUCT(BlueprintType)
struct FReactionExecutionPolicy
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bWantsReplaceActive = false;

	UPROPERTY(Transient)
	bool bCanUsePending = true;

	UPROPERTY(Transient)
	bool bCanUseQueue = false;

	UPROPERTY(Transient)
	int32 Priority = INDEX_NONE;
};
```

For the first implementation pass, this struct is optional.  
The system can directly use `FReactionData.Priority` and `CReaction` local hooks instead.

If the API should stay explicit, keeping a policy struct is useful.  
If implementation simplicity matters more, `ResolveReactionPolicy()` can stay thin or be deferred.


---

## 9. Stage Meaning for Reaction

For Reaction, each stage means:

```text
RequestReaction
- External entry point after TakeDamage
- Returns request result

ResolveReactionContext
- damage result -> ReactionType
- ReactionType + ApplyDamageSpecKey -> ReactionData
- ReactionData -> ReactionExecutor
- Builds final FReactionContext

ResolveReactionPolicy
- Interprets incoming reaction priority / pending availability / replacement tendency

OrchestrateReaction
- Compares active / pending context against incoming context
- Checks priority and interruptible window
- Decides Start / ReplaceActive / ReplacePending / Enqueue / Ignore / Reject

DispatchReactionDecision
- Calls ReactionComponent API according to the decision
```


---

## 10. Design Principles

The principles of this API model are:

- `Resolve` creates execution candidates.
- `Policy` interprets how the candidate wants to be handled.
- `Orchestrate` evaluates conflict against current state.
- `Dispatch / Apply` mutates actual runtime state.
- `Execution Object` performs actual lifecycle.
- The Orchestrator creates decisions but does not directly commit active / pending state.
- The Component applies decisions but should not be the center of request interpretation and conflict judgment.


---

## 11. Summary

Execution Orchestration API is organized as:

```text
Orchestrator
-> Request
-> Resolve Context
-> Resolve Policy
-> Orchestrate
-> Dispatch

Component
-> Start / Pending / Replace / Enqueue / Finish

Execution Object
-> Begin / Stop / End / Cleanup / Local Policy
```

This structure separates API names and responsibilities,  
and gives Reaction Orchestration a clear location for competing-state evaluation.


---
