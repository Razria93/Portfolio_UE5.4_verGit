# A12 UE5 Portfolio Action Orchestration Transition Arbitration Decision

## 1. Purpose

This document records how far the current action orchestration structure has progressed after the action orchestration refactor, and how transition arbitration should be extended in the next stage.

Current action orchestration has a request pipeline, but it is not a completed arbitration system yet.

This document defines the current structure as an intermediate stage and records the criteria for expanding toward dodge, cancel, interrupt, execution arbitration, and combat subsystem integration.

## 2. Current System Shape

Current action requests are processed as:

```text
Request
-> Candidate
-> ResolvedContext
-> LocalDecision
-> ResolvedPolicy
-> OrchestrationResult
-> Component Apply
-> Executor Lifecycle
```

The current responsibility of each stage is:

```text
Candidate
-> builds an execution candidate key from request and current state

ResolvedContext
-> resolves the candidate key into ActionData and ActionExecutor

LocalDecision
-> incoming action executor says which transition it wants

ResolvedPolicy
-> checks whether local decision has minimum permission in current body/runtime state

OrchestrationResult
-> converts local decision and policy into final request result and attaches directive if needed

ActionComponent
-> applies orchestration result to active action runtime state

CAction
-> handles montage lifecycle, notify command, feedback, and local rule
```

Current local decision expresses the execution intent of incoming executor.

Examples:

```text
Idle + Attack
-> Start

Action + ComboAttack + ChainWindow
-> Chain

Reaction + Dodge
-> Cancel
```

Current `RequestMovementAction()` directly calls `MovementComponent`.

This is acceptable because current movement requests are mostly simple movement commands such as `Move`, `Walk`, `Run`, `Sprint`, `Jump`, and `StopJump`.

However, movement that combines montage, cancel, invincible window, or reaction state, such as dodge, roll, step, or dash attack, should be classified as action execution.

## 3. Problems And Limits

The largest current limitation is that orchestration level does not fully judge competition state yet.

Current orchestration level mostly does:

```text
converts local decision into orchestration decision
checks whether policy value is true
attaches reaction stop directive for Cancel
```

The current structure is closer to:

```text
Local Level
-> decides what transition incoming execution wants

Policy Level
-> simply filters whether local decision is executable

Orchestration Level
-> converts local decision and policy into request result and attaches stop directive
```

This is weaker than what the word orchestration implies.

Real orchestration must judge the relationship between active execution and incoming execution.

Required questions include:

```text
Does active execution exist?
Are incoming and active execution in the same domain?
Does incoming have authority to intervene in active?
Does active allow that intervention?
What is the priority result?
Is it a force transition?
Can it ignore active window?
What is the final decision?
Is intervention directive needed?
```

The meaning of `Cancel` and `Interrupt` is also not fully separated yet.

They should not be decided only by whether the follow-up execution is action or reaction.

A better distinction is:

```text
Cancel
-> same-owner intentional transition or allowed defensive response that exits own active execution

Interrupt
-> external event or higher-priority execution pushes out active execution
```

Examples:

```text
Dodge cancels hit reaction
-> same owner exits active reaction and enters dodge action, so this is closer to Cancel

Hit reaction interrupts attack action
-> damage event pushes out active attack action, so this is closer to Interrupt

Dead reaction interrupts dodge action
-> death result outranks dodge action, so this is closer to Interrupt
```

Therefore, cancel/interrupt should not be decided only by domain direction such as `Action -> Action`, `Action -> Reaction`, `Reaction -> Action`, or `Reaction -> Reaction`.

They should be judged from source, ownership, force level, priority, and active allow rule.

## 4. Refactoring Direction

The next stage should separate local decision and orchestration arbitration.

Based on the current discussion, `ExecutionSnapshot` and `InterventionQuery / Assessment / Directive` are clearer than the name `ResolvedPolicy`.

Recommended responsibility split:

```text
ExecutionSnapshot
-> compresses current body/runtime/active state

LocalDecision
-> incoming executor proposes the transition it wants from the given snapshot

InterventionQuery
-> packages active/incoming information when local decision intervenes in active execution

InterventionAssessment
-> evaluates incoming want rule, active allow rule, priority, force, and window

InterventionDirective
-> expresses stop target, stop reason, stop source, and after stop action for component consumption
```

Recommended flow:

```text
Request
-> Candidate
-> ExecutionContext
-> ExecutionSnapshot
-> LocalDecision
-> Arbitration
-> Directive / Command
-> Component Apply
```

Orchestration arbitration should expand in this order:

```text
1. Check whether active execution exists
2. If active does not exist, approve Start / Handle
3. If active exists, inspect local transition type
4. If Chain, check same action flow and chain rule
5. If Cancel, check self-transition authority and active allow rule
6. If Interrupt, check external / force authority and active allow rule
7. Check priority / force / window / active allow / incoming want
8. Emit final decision and intervention directive
```

Dodge is the first suitable validation case in the next branch.

Dodge is not a simple movement command. It should be modeled as action execution.

Initial `CAction_Dodge` local decision can start like this:

```cpp
EActionLocalLevelDecision UCAction_Dodge::ResolveLocalLevelDecision(
	const FActionLocalLevelQuery& InQuery
) const
{
	const FExecutionSnapshot& snapshot = InQuery.Snapshot;

	if (!snapshot.bIsAlive)
	{
		return EActionLocalLevelDecision::Reject;
	}

	if (snapshot.ExecutionState == EExecutionState::Idle
		&& !snapshot.bHasActiveAction
		&& !snapshot.bHasActiveReaction)
	{
		return EActionLocalLevelDecision::Start;
	}

	if (snapshot.ExecutionState == EExecutionState::Reaction
		&& snapshot.bHasActiveReaction)
	{
		return EActionLocalLevelDecision::Cancel;
	}

	return EActionLocalLevelDecision::Reject;
}
```

Then orchestration level must judge whether active reaction allows dodge cancel.

Judgment factors include:

```text
Is active reaction cancelable?
Does incoming dodge have cancel intent?
Is cancel window open?
Is priority or force policy sufficient?
```

Stop directive should eventually expand into common intervention directive.

Expected structure:

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

This separates:

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

Long term, `ActionComponent::ApplyActionDecision()` may be better as a consumer of execution command rather than direct orchestration result.

Expected direction:

```text
Orchestrator
-> creates command containing final decision and directive

Component
-> consumes command and applies stop / start / chain / end

Executor
-> handles montage lifecycle and notify command
```

With this structure, action orchestrator, reaction orchestrator, and combat subsystem can share the same command application path.

## 5. Work Order

The current branch should end at:

```text
keep existing action behavior
clean up local query structure
separate active action state and active context
restore interrupt preparation flow
keep reaction stop directive skeleton
```

The next branch should proceed as:

```text
1. Add CAction_Dodge
2. Return Cancel from dodge local decision while in Reaction state
3. Judge reaction cancel permission at orchestration level
4. Clean up ReactionStopDirective naming or start common directive design
5. Review ActionExecutionCommand or ExecutionCommand structure
6. Add cancel / interrupt hooks to action local rule
7. Implement active/incoming arbitration at orchestration level
8. Review CombatSubsystem connection later
```

CombatSubsystem should be treated as a higher-level judge/coordinator, not an executor.

Actual runtime mutation should be performed by each domain component.

```text
ActionComponent
-> applies action runtime

ReactionComponent
-> applies reaction runtime

OutgoingDamageComponent
-> builds outgoing damage payload

IncomingDamageComponent
-> applies incoming damage
```

If CombatSubsystem directly plays montages or changes state, orchestrator / component / executor responsibilities collapse.

## 6. Conclusion

Current action orchestration is not a complete arbitration system. It is an intermediate structure-building stage.

Current local level says what transition incoming execution wants.

Current policy level is closer to a filter that checks whether local decision has minimum permission.

In the follow-up structure, it is better to reduce the term policy and split responsibility into `ExecutionSnapshot` and `InterventionQuery / Assessment / Directive`.

Cancel and Interrupt should not be distinguished by follow-up domain. They should be distinguished by source, ownership, force level, priority, and active allow rule.

Dodge is the appropriate first action case to validate reaction cancel.
