# S16 UE5 Portfolio Action Reaction Execution Symmetry Implementation Plan

## 1. Purpose

This document records the implementation plan for aligning action/reaction execution flow and implementing the minimum dodge behavior in the current branch.

The goal of this branch is not just to clean up action orchestration. It is to make action and reaction readable through the same execution language.

The minimum functional goal is to let dodge action cancel active reaction and enter action execution.

## 2. Current System Shape

The action side is currently organized so that requests pass through candidate, context, local decision, orchestration result, and component apply.

The current action flow is close to:

```text
Action Request
-> Candidate
-> ActionResolvedContext
-> LocalLevelQuery
-> LocalLevelResult
-> ResolvedPolicy
-> OrchestrationLevelResult
-> ActionComponent::ApplyActionDecision
-> CAction lifecycle
```

The reaction side has reaction orchestration and component/executor structure, but it does not yet read exactly like the action-side API layers.

The action result currently has `FReactionStopDirective`, which provides only the minimum connection for an action to stop active reaction.

Current stop handling is close to:

```text
ActionComponent::ApplyActionDecision
-> ApplyReactionStopDirective
-> apply action decision
```

This can support the minimum dodge implementation, but it is not enough to symmetrically support action/action, reaction/action, and reaction/reaction.

## 3. Problems And Limits

The first limitation is that stop directive is reaction-specific.

`FReactionStopDirective` can only express active reaction stop.

Future flows require:

```text
Action -> Action
-> cancel active action and start incoming action

Action -> Reaction
-> cancel active reaction and start incoming action

Reaction -> Action
-> interrupt active action and start incoming reaction

Reaction -> Reaction
-> interrupt active reaction and start incoming reaction
```

Fixing stop target to reaction will block the structure soon.

The second limitation is that combination APIs such as `TryReplaceAction` and `TryReplaceReaction` implicitly bind stop and start together.

Cross-domain intervention can require:

```text
stop only
stop then start incoming
ignore without stop
reject when stop fails
```

Therefore, it is better to explicitly store stop directive in orchestration result and let component consume it before applying incoming execution, rather than adding more replace-style APIs.

The third limitation is that action and reaction components do not yet apply execution through fully symmetric flow.

Long term, both components should follow:

```text
ApplyDecision
-> ApplyExecutionInterventionDirective
-> Start / Chain / Enqueue or other domain-specific execution
```

## 4. Refactoring Direction And Content

This work promotes `FReactionStopDirective` into common `FExecutionInterventionDirective`.

Recommended structure:

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

Each field means:

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

At this stage, `AfterStopAction` can exist structurally, while actual control can still be handled by decision and component apply flow.

The minimum implementation follows:

```text
Directive
-> expresses what should stop

Decision
-> expresses how incoming execution should be applied
```

Example:

```text
active reaction stop
-> represented by directive

dodge action start
-> represented by action decision
```

ActionComponent and ReactionComponent consume directive in the same way.

```text
TargetDomain == own domain
-> stop active execution directly

TargetDomain != own domain
-> delegate to the target domain component through RequestStop
```

ActionComponent flow:

```text
ApplyActionDecision
-> result validation
-> ApplyExecutionInterventionDirective
-> Start / Chain / Enqueue action
```

ReactionComponent flow:

```text
ApplyReactionDecision
-> result validation
-> ApplyExecutionInterventionDirective
-> Start reaction
```

Important rule:

```text
directive stop failed
-> do not start incoming execution
```

## 5. Work Order

### 5.1 Add Common Execution Stop Structures

Add common enums and directive structure.

```text
EExecutionDomain
EExecutionStopReason
EExecutionStopSource
EExecutionAfterStopAction
FExecutionInterventionDirective
```

Remove or replace existing `FReactionStopDirective` with the common directive.

### 5.2 Change Action Result Structure

Change `FActionOrchestrationLevelResult` to contain `FExecutionInterventionDirective`.

Before:

```text
FReactionStopDirective StopDirective
```

After:

```text
FExecutionInterventionDirective InterventionDirective
```

Rename `ResolveReactionStopDirective()` to a more general name.

Recommended name:

```text
ResolveExecutionInterventionDirective
```

Minimum current behavior:

When action decision is `Cancel` and active reaction exists, build:

```text
TargetDomain = Reaction
StopReason = Cancelled
StopSource = ActionOrchestration
AfterStopAction = StartIncoming
```

### 5.3 Change ActionComponent Directive Consumption

Remove `ApplyReactionStopDirective()` and replace it with `ApplyExecutionInterventionDirective()`.

Recommended flow:

```text
ApplyActionDecision
-> ApplyExecutionInterventionDirective
-> apply action execution by decision
```

`Cancel` and `Interrupt` should start incoming action after stop has already been handled by directive.

```text
Start
-> StartAction

Cancel
-> StartAction

Interrupt
-> StartAction

Chain
-> ChainAction
```

Long term, replace-style APIs can be removed.

If stop-combination APIs are no longer needed in this work, remove or reduce them.

### 5.4 Change ReactionComponent Directive Consumption

Add the same directive consumption function to ReactionComponent.

```text
ApplyExecutionInterventionDirective
RequestStopActiveReaction
StopActiveReaction
```

Reaction orchestration result can also hold the same directive field.

Reaction-vs-Reaction interrupt should eventually become:

```text
directive stops active reaction
-> decision starts incoming reaction
```

### 5.5 Simplify Action / Reaction Component APIs

If stop is handled at the beginning of `ApplyDecision`, the meaning of `TryReplace` APIs becomes weak.

Recommended API direction:

```text
ApplyDecision
RequestStopActive
Start
Chain
Enqueue
StopActive
EndActive
```

The `Try` prefix should be reduced because it makes the component look like the primary decision maker.

However, validation must remain.

```text
Removing Try API
!= removing runtime guards
```

Components must still guard context validity, active state, and executor validity.

### 5.6 Implement Minimum Dodge

After common directive consumption is established, add dodge action.

Minimum implementation scope:

```text
ECombatActionIntent::Dodge
EActionType::Dodge
CAction_Dodge
ActionData registration
Montage registration
Notify wiring
```

Dodge local decision:

```text
Idle state
-> Start

Reaction state + active reaction exists
-> Cancel

Otherwise
-> Reject or Ignore
```

Dodge cancel flow:

```text
Dodge request
-> ActionOrchestrator candidate/context resolve
-> Dodge local decision = Cancel
-> intervention directive created
-> ActionComponent requests active reaction stop
-> if stop succeeds, start dodge action
```

### 5.7 Verification

Verify:

```text
Dodge runs in Idle state
Dodge input during HitReaction stops active reaction and starts Dodge
Dodge is rejected during Dead reaction
Existing ComboAttack behavior remains
Existing Equip / Unequip behavior remains
Existing Hit / Dead reaction behavior remains
Incoming execution does not start when directive stop fails
```

## 6. Conclusion

The core of this work is making action and reaction execution flows read through the same structure.

To do this, `FReactionStopDirective` is promoted into common `FExecutionInterventionDirective`, and both action/reaction components consume directives before applying incoming execution.

With this structure, stop target and follow-up execution can be expressed clearly through orchestration result instead of relying on replace-style combination APIs.

Completion criteria for this branch:

```text
Common intervention directive exists
ActionComponent and ReactionComponent consume directive through the same flow
Dodge cancels active reaction and enters action execution
Existing action/reaction behavior remains intact
```
