# A10 UE5 Portfolio Execution Arbitration Future Decision

## 1. Purpose

This document records the long-term execution arbitration direction for cases where actions and reactions can intervene in each other's active execution.

The current branch focuses on completing the action orchestration refactor. It does not implement the common arbitration structure immediately.

This document is a design memo for judgment structures needed by future features such as dodge, guard, parry, counter, and execution.

## 2. Current Judgment

Current reaction orchestration already judges part of the competition between active reaction and incoming reaction.

The core questions are:

```text
Does the incoming reaction want to intervene in the active reaction?
Does the active reaction allow that intervention?
Do priority and force policy allow the intervention?
What is the final result: start / interrupt / ignore / reject?
```

Action orchestration in this branch reorganizes the request flow, but common arbitration between action/action, action/reaction, and reaction/action is not implemented yet.

The current action flow is an intermediate stage:

```text
Intent
-> Candidate
-> ExecutionContext
-> LocalDecision
-> OrchestrationResult
-> Component Apply
```

The follow-up structure should first compress the current execution environment into an `ExecutionSnapshot`, then perform active/incoming arbitration after local decision.

## 3. Problem

Long term, actions and reactions are not a linear execution chain. They are execution domains that can intervene in each other.

Examples:

```text
Action -> Action
- combo chain
- dodge cancels attack
- counter cancels guard
- equip action cannot interrupt attack

Reaction -> Reaction
- dead reaction interrupts hit reaction
- lower priority hit reaction is ignored

Action -> Reaction
- dodge cancels hit reaction
- counter cancels stagger reaction
- forced action interrupts hit reaction

Reaction -> Action
- hit reaction interrupts attack action
- dead reaction interrupts every action
```

If this is handled only by components directly stopping other components, judgment responsibility becomes scattered.

What is needed is a common arbitration structure that judges the relationship between active execution and incoming execution.

## 4. Recommended Arbitration Flow

Long-term execution arbitration should use the following flow:

```text
Incoming request
-> Candidate
-> ExecutionContext
-> ExecutionSnapshot
-> LocalDecision
-> InterventionQuery
-> InterventionAssessment
-> InterventionDirective
-> ComponentCommand
```

Each stage has a distinct role:

```text
ExecutionContext
-> resolves key, data, and executor for incoming execution

ExecutionSnapshot
-> compresses body state, active action/reaction existence, and active context

LocalDecision
-> lets the incoming executor propose the transition it wants

InterventionQuery
-> carries active/incoming domain, context, intervention intent, and snapshot

InterventionAssessment
-> carries incoming want rule, active allow rule, priority, force, and window judgment

InterventionDirective
-> expresses what to stop, why to stop it, and what to do afterward for component consumption
```

Local rule and arbitration should remain separate.

```text
Incoming executor
-> says whether it wants to intervene in active execution

Active executor
-> says whether it allows interruption/cancel by incoming execution

Orchestrator
-> combines both answers, priority, force, and body state into the final judgment
```

Executors should not make the final judgment. They only provide their own execution rules; the orchestrator makes the final decision.

## 5. Decision And Stop Reason

Gameplay decision and stop reason should be separated.

Gameplay decisions include:

```text
Start
-> starts incoming execution when there is no active execution or no conflict

Chain
-> links to the next execution inside the same action flow

Cancel
-> exits active execution due to same-owner intent or an allowed defensive response

Interrupt
-> pushes out active execution due to an external event or higher-priority execution

Ignore
-> request is valid but not consumed in the current state

Reject
-> request, context, data, executor, or state is invalid
```

`Abort` is closer to system cleanup or fallback reason than gameplay decision.

Therefore, `Abort` should be modeled as a stop reason or finish reason rather than an orchestration decision.

## 6. Interrupt And Window

Interrupt does not always mean the same thing.

It should generally be divided into:

```text
Soft Interrupt
-> allowed only while active execution is interruptible
-> suitable for hit stagger, weak reaction, and low-priority interrupt

Force Interrupt
-> stops active execution regardless of active window
-> suitable for death, execution, grab, guard break, and scripted event
```

Interruptible windows are still needed, but not every interrupt must require a window.

This difference should be handled in the assessment stage.

```text
bIncomingWantsIntervention
bActiveAllowsIntervention
bPriorityAllowed
bForceIntervention
bIgnoreActiveWindow
```

## 7. Snapshot And Local Rule

A decision-specific bool filter such as `ResolvedPolicy` can easily become a second judgment pass over local decisions.

Long term, `ExecutionSnapshot` is more suitable before local decision than `Policy`.

```text
ExecutionSnapshot
-> ExecutionState
-> bIsAlive
-> bHasActiveAction
-> bHasActiveReaction
-> ActiveActionContext
-> ActiveReactionContext
```

Local rule proposes the transition it wants from the snapshot and incoming context.

```text
Action local rule
-> proposes Start / Chain / Cancel / Interrupt / Reject / Ignore

Reaction local rule
-> proposes Start / Interrupt / Cancel / Reject / Ignore
```

If the resulting decision intervenes in active execution, such as `Cancel` or `Interrupt`, the flow proceeds to intervention arbitration.

## 8. Future Structure Candidate

When implementing common arbitration, the following structures can be considered:

```cpp
UENUM(BlueprintType)
enum class EExecutionDomain : uint8
{
	None = 0,

	Action,
	Reaction,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionInterventionIntent : uint8
{
	None = 0,

	Cancel,
	Interrupt,

	Max,
};
```

```text
FExecutionSnapshot
FActionExecutionContext
FReactionExecutionContext
FExecutionInterventionQuery
FExecutionInterventionAssessment
FExecutionInterventionDirective
```

`FActionResolvedContext` and `FReactionContext` should eventually be aligned as:

```text
FActionExecutionContext
FReactionExecutionContext
```

The common arbitration layer should not directly depend on action/reaction local decision enums. It should receive a converted common intent such as `EExecutionInterventionIntent`.

## 9. Follow-up Scope

Follow-up work should implement:

```text
Common execution arbitration model
- incoming execution
- active execution
- domain: action / reaction
- intervention intent: cancel / interrupt
- priority
- active allow rule
- incoming want rule

Action local rule cleanup
- WantsToStart()
- WantsToChain()
- WantsToCancelActive()
- WantsToInterruptActive()
- AllowsCancelBy()
- AllowsInterruptionBy()

Reaction local rule expansion
- WantToInterruptActive()
- WantToCancelActive()
- AllowInterventionBy()
```

A simpler common-query API can also be considered.

```cpp
virtual bool WantToIntervene(const FExecutionInterventionQuery& InQuery) const;
virtual bool AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const;
```

## 10. Defensive Action Connection

Defensive actions are practical validation cases for common arbitration.

```text
Parry
- pre-input action
- perfect/general parry is judged in hit resolution
- perfect parry can force attacker reaction

Guard
- maintains action state
- converts incoming damage into guard response
- guard break can force reaction

Dodge
- can enter cancel action during reaction
- requires active reaction cancel window

Counter
- can cancel or interrupt current execution after a perfect defensive result
```

These features should connect with combat interaction or hit resolution results instead of directly stopping action/reaction arbitrarily.

## 11. Current Branch Scope

The current branch only handles:

```text
Action executor no longer acts as a data repository
ActionComponent is organized around Apply / Request / Try / Internal
ActionOrchestrator is organized around candidate / context / local decision / result
Notify flow is routed through component gateway
ChainWindow / HitContext / Collision are converted to NotifyState
Reaction stop request is implemented only as minimum functionality
```

The current branch does not implement:

```text
Common FExecutionInterventionQuery
Action/action cancel and interrupt priority system
Action/reaction cross-domain local rules
Parry / guard / dodge / counter policy
Combat interaction subsystem based hit resolution arbitration
```
