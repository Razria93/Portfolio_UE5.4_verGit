# S14 UE5 Portfolio Action Arbitration Follow-up Decision

## 1. Purpose

This document records the remaining limitations after the action orchestration refactor and the intended direction for follow-up arbitration work.

This branch reorganizes the core action execution flow, but it does not complete the full competition model between actions and reactions.

The purpose of this document is to define what is implemented now and what should be extended later.

## 2. Previous System Shape

Before this refactor, there was no shared model for action and reaction intervention rules.

Reaction orchestration already had a structure for judging priority, interruption, and executor hooks between active and incoming reactions.

Action orchestration, however, was still focused on cleaning up the existing action execution flow. The following rules were not generalized yet:

```text
Can an incoming action cancel an active action?
Can an incoming action cancel an active reaction?
Can an incoming reaction interrupt an active action?
How should active and incoming execution priority be compared?
When is a forced transition allowed?
```

The current action orchestration primarily focuses on turning a request into an executable result through local decisions and policy filtering.

## 3. Problems And Limits

The current orchestration level is still thin.

Its current flow is closer to:

```text
Local Level
-> incoming executor decides which transition it wants

Policy Level
-> checks whether that transition satisfies minimum state requirements

Orchestration Level
-> converts local decision and policy into a final result and attaches directives if needed
```

This means the current orchestration level does not fully arbitrate competition between active execution and incoming execution.

In particular, `Cancel` and `Interrupt` should not be determined only from the resulting execution domain.

A better distinction is:

```text
Cancel
-> the same owner intentionally exits its active execution, or performs an allowed defensive response

Interrupt
-> an external event or higher-priority execution pushes out the active execution
```

Examples:

```text
Dodge cancels hit reaction
-> the same character exits its active reaction and enters dodge action, so this is closer to Cancel

Hit reaction interrupts attack action
-> a damage event pushes out active attack action, so this is closer to Interrupt

Dead reaction interrupts any action
-> death result outranks current execution, so this is closer to Interrupt
```

Therefore, cancel and interrupt should not be decided only by whether the next execution is an action or a reaction.

## 4. Refactoring Direction And Content

This branch does not implement common arbitration yet. It only creates the minimum skeleton needed for later work.

The current implementation scope is:

```text
FActionOrchestrationLevelResult
-> stores final action decision
-> stores FReactionStopDirective when needed

ResolveReactionStopDirective
-> builds a reaction stop directive when action decision is Cancel and an active reaction exists

ActionComponent::ApplyActionDecision
-> requests active reaction stop when a directive exists
-> applies the action decision afterward

ReactionComponent::RequestStopActiveReaction
-> handles external-domain cooperation requests to stop active reaction
```

This is not a shared arbitration model yet. It is the minimal connection needed to implement follow-up features such as dodge.

Long term, the following axes should be separated:

```text
Target
-> what execution should be stopped

StopReason
-> why it is stopped

StopSource
-> who requested the stop

AfterStopAction
-> whether to start a new execution afterward or only clean up
```

`FReactionStopDirective` currently expresses only reaction stop. It can later be promoted into a common `FExecutionInterventionDirective` model.

## 5. Future Direction

Follow-up work should be split into three stages.

The first stage is aligning reaction API and flow with the action refactor structure.

```text
ReactionComponent
-> organize Apply / Request / Try / Internal API layers

CReaction
-> align Start / Stop / Complete / notify / feedback lifecycle with action flow

ReactionOrchestrator
-> preserve reaction-vs-reaction logic while cleaning naming and result structure
```

The second stage is validating action-to-reaction cancel flow using dodge.

```text
Input
-> Dodge intent
-> Dodge candidate
-> Dodge local decision = Cancel
-> Policy checks reaction state
-> Orchestration builds reaction stop directive
-> ReactionComponent stops active reaction
-> ActionComponent starts dodge action
```

The third stage is reviewing a common execution arbitration model.

```text
Incoming execution
-> does it collide with active execution?

Active execution
-> does it allow incoming intervention?

Incoming executor
-> does it want to intervene in active execution?

Policy
-> combines priority, force, window ignore, and body state

Result
-> decides Start / Chain / Cancel / Interrupt / Ignore / Reject
```

This stage should be designed together with guard, parry, counter, execution, and the combat interaction subsystem.

## 6. Conclusion

The current action orchestration refactor does not finish the final arbitration model. It prepares the action execution flow so that such a model can be added later.

`FReactionStopDirective` is currently a temporary cross-domain stop skeleton, but it is enough as a connection point for dodge and reaction API alignment.

The recommended next steps are reaction flow alignment, dodge cancel implementation, and then common execution arbitration.
