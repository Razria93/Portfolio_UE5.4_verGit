# A11 UE5 Portfolio Execution Stop Directive Decision

## 1. Purpose

This document records how active execution stops should be represented in the action/reaction execution structure.

During the action orchestration refactor, actions such as dodge created a requirement to stop an active reaction and enter action execution.

In that case, a single `Cancel` or `Interrupt` decision is not enough to express:

```text
What should be stopped?
Why should it stop?
What should happen after stop?
Who requested the stop?
```

Therefore, this document records the direction of separating gameplay decision from stop directive.

## 2. Current State

Current action orchestration resolves an incoming action request and builds the final action decision.

Current action decisions roughly mean:

```text
Start
-> starts incoming action when there is no active action

Chain
-> links to the next action index inside the active action flow

Interrupt
-> reserved for stopping active action and replacing it with incoming action

Cancel
-> currently used as minimum cross-domain cancel that stops active reaction and starts incoming action
```

In the current branch, `Cancel` does not complete the whole cancel model. It is the minimum structure needed to enter dodge action from reaction.

`FReactionStopDirective` only expresses active reaction stop.

```text
StopReason
-> EReactionStopReason::Cancelled

StopSource
-> EReactionStopSource::ActionOrchestration
```

Whether incoming action starts afterward is represented by the action orchestration decision and `ActionComponent::ApplyActionDecision()`, not by the stop directive.

## 3. Problem Analysis

Execution stop is not a single meaning.

Examples:

```text
Dodge during hit reaction
-> cancels active reaction and starts dodge action

Dead reaction during attack
-> interrupts active action and starts dead reaction

External cleanup
-> aborts active execution with no follow-up execution

Guard success
-> keeps active action/reaction and only adjusts damage response
```

These can all be described as "stopping something", but they have different axes.

```text
Stop target
-> Action / Reaction / None

Stop reason
-> Cancelled / Interrupted / Ignored / Aborted

Stop source
-> ActionOrchestration / ReactionOrchestration / CombatInteraction / System

After stop action
-> StopOnly / StartIncoming / ResumePrevious
```

Putting all of these axes into one enum decision makes the decision name too heavy and makes component responsibilities unclear.

Therefore, gameplay decision and stop directive should be separated.

## 4. Decision

The current branch does not immediately implement a common execution directive.

For the current scope, action orchestration only creates `FReactionStopDirective` when it must stop active reaction. `ActionComponent` applies that directive first, then consumes the action decision.

The minimum flow is:

```text
ActionOrchestrator
-> resolves incoming action request
-> gets local decision
-> gets orchestration decision
-> creates FReactionStopDirective when needed

ActionComponent
-> requests active reaction stop first when directive exists
-> executes incoming action according to action decision

ReactionComponent
-> consumes FReactionStopDirective and performs active reaction stop/end
```

Current `FReactionStopDirective` only expresses why the reaction should stop.

The follow-up action start is represented by the action decision.

## 5. Recommended Structure

Long term, action/reaction stop should be expressed by separating these axes.

### 5.1 Stop Target

```cpp
UENUM(BlueprintType)
enum class EExecutionDomain : uint8
{
	None = 0,

	Action,
	Reaction,

	Max,
};
```

Stop target expresses which domain's active execution should be cleaned up.

```text
Action
-> stop active action

Reaction
-> stop active reaction
```

### 5.2 Stop Reason

```cpp
UENUM(BlueprintType)
enum class EExecutionStopReason : uint8
{
	None = 0,

	Cancelled,
	Interrupted,
	Ignored,
	Aborted,

	Max,
};
```

Stop reason expresses why active execution ends.

```text
Cancelled
-> stopped by same-owner intent or allowed defensive response

Interrupted
-> stopped by external event or higher-priority execution

Ignored
-> consumed without valid follow-up execution

Aborted
-> cleaned up by invalid state or system fallback
```

`Aborted` should be treated as cleanup/fallback reason rather than gameplay decision.

### 5.3 Stop Source

```cpp
UENUM(BlueprintType)
enum class EExecutionStopSource : uint8
{
	None = 0,

	ActionOrchestration,
	ReactionOrchestration,
	CombatInteraction,
	System,

	Max,
};
```

Stop source records where the stop request came from.

This is important for debugging and responsibility tracing.

### 5.4 After Stop Action

```cpp
UENUM(BlueprintType)
enum class EExecutionAfterStopAction : uint8
{
	None = 0,

	StopOnly,
	StartIncoming,
	ResumePrevious,

	Max,
};
```

After stop action expresses what happens after active execution is stopped.

```text
StopOnly
-> clean up active execution only

StartIncoming
-> clean up active execution and start incoming execution

ResumePrevious
-> resume previous execution when stack or queue execution exists
```

The current branch does not implement `AfterStopAction` as a separate enum. It expresses this through action decision and component apply flow.

## 6. Recommended Directive Model

Long term, the following common directive model can be considered:

```cpp
USTRUCT(BlueprintType)
struct FExecutionInterventionDirective
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bRequested = false;

	UPROPERTY(Transient)
	EExecutionDomain TargetDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	EExecutionStopReason StopReason = EExecutionStopReason::None;

	UPROPERTY(Transient)
	EExecutionStopSource StopSource = EExecutionStopSource::None;

	UPROPERTY(Transient)
	EExecutionAfterStopAction AfterStopAction = EExecutionAfterStopAction::None;
};
```

This structure separates:

```text
TargetDomain
-> what should be stopped

StopReason
-> why it should stop

StopSource
-> where the stop request came from

AfterStopAction
-> what should happen after stop
```

However, the current code already has `EActionStopReason`, `EReactionStopReason`, and `FReactionStopDirective`.

Generalizing them immediately in this branch would make the action orchestration refactor too broad.

So the current branch keeps `FReactionStopDirective`, and later cross-domain arbitration can promote it into a common directive.

## 7. Current Branch Application

The current action orchestration refactor applies this rule:

```text
Dodge action request during Reaction
-> local decision returns Cancel
-> orchestration decision becomes Cancel
-> FReactionStopDirective is created
-> ActionComponent requests active reaction stop first
-> ActionComponent starts incoming dodge action
```

Current `FReactionStopDirective` only expresses:

```text
StopReason
-> EReactionStopReason::Cancelled

StopSource
-> EReactionStopSource::ActionOrchestration
```

Whether action starts after stop is expressed by action orchestration decision, not the directive.

Therefore, in current `ActionComponent::ApplyActionDecision()`, `Cancel` should start incoming action instead of replacing active action.

```cpp
case EActionOrchestrationLevelDecision::Cancel:
	return TryStartAction(InActionOrchestrationResult.ResolvedContext);
```

## 8. Future Extension Criteria

The common execution directive model should be reviewed when the following requirements appear:

```text
Action cancels active action and transitions to another action
Reaction interrupts active action
Dead reaction force-stops every execution
Guard break force-stops active action or reaction
Counter attack cleans up current execution and enters forced action
Execution stack or queue requires resume after stop
```

At that stage, `FReactionStopDirective` will not be enough.

A common structure such as `FExecutionInterventionDirective` should be introduced, and action/reaction components should consume directives by domain.
