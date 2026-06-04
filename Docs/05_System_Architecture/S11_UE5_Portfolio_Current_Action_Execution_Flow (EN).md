# Current Action Execution Flow Baseline

## 1. Purpose

This document records the current action execution flow before starting the action orchestration refactor.

The purpose is not to define the new structure first.

The purpose is to fix the current code baseline for where request, decision, and lifecycle responsibilities currently live.

This document is used as the comparison baseline when responsibilities are moved into the `ActionOrchestrator -> ActionComponent -> CAction` structure.

---

## 2. Current Entry Paths

Player and Enemy currently send movement / equipment / combat action requests through `UCActionOrchestratorComponent`.

Player entry paths are:

```text
ACPlayer::HandleMove()
ACPlayer::HandleWalk()
ACPlayer::HandleRun()
ACPlayer::HandleSprint()
ACPlayer::HandleJump()
ACPlayer::HandleStopJump()
-> UCActionOrchestratorComponent::RequestMovementAction()

ACPlayer::HandleEquipmentAction()
-> UCActionOrchestratorComponent::RequestEquipmentAction()

ACPlayer::HandleCombatAction()
-> UCActionOrchestratorComponent::RequestCombatAction()
```

Enemy entry paths are:

```text
ACEnemy::HandleAIWalk()
ACEnemy::HandleAIRun()
ACEnemy::HandleAISprint()
ACEnemy::HandleAIJump()
ACEnemy::HandleAIStopJump()
-> UCActionOrchestratorComponent::RequestMovementAction()

ACEnemy::HandleAIEquipmentAction()
-> UCActionOrchestratorComponent::RequestEquipmentAction()

ACEnemy::HandleAICombatAction()
-> UCActionOrchestratorComponent::RequestCombatAction()
```

Therefore, the current request entry already shares the same action orchestrator between Player and Enemy.

---

## 3. AI Combat Action Flow

Enemy combat action starts from a Behavior Tree task.

The current flow is:

```text
UCBTTask_StartCombatAction::ExecuteTask()
-> checks Blackboard bCanCombatAction
-> checks Blackboard bIsCombatAction
-> optionally calls AIController::StopMovement()
-> calls ACEnemy::HandleAICombatAction(CombatActionIntent)
-> calls UCActionOrchestratorComponent::RequestCombatAction()
```

The task succeeds only when the action request result is `Started`.

```text
requestResult.IsAccepted()
requestResult.ResultType == Started
```

After success, the task writes `NextCombatActionTime` to the blackboard.

Therefore, the current BT task performs some pre-checks, while the actual combat action start request still goes through the orchestrator.

---

## 4. Current ActionOrchestrator Responsibility

`UCActionOrchestratorComponent` currently receives request source and intent, applies a common gate, and resolves a simple action type.

The common gate is handled by `CanAcceptActionRequest()`.

```text
validates OwnerCharacter
validates HealthComponent / StateComponent
checks alive state
blocks ExecutionState::Reaction
blocks ExecutionState::Dead
```

Movement requests are executed directly through `MovementComponent`.

```text
Move   -> MovementComp::OnMove()
Walk   -> MovementComp::OnWalk()
Run    -> MovementComp::OnRun()
Sprint -> MovementComp::OnSprint()
Jump   -> MovementComp::OnJump()
```

`StopJump` is treated as release-style cleanup and is handled before the hard-block gate.

Equipment requests are resolved into an action type and delegated to `ActionComponent::ExecuteAction()`.

```text
Equip   -> EActionType::Equip
Unequip -> EActionType::Unequip
Toggle  -> Equip or Unequip based on current weapon type
```

Combat requests currently resolve only `ComboAttack`.

```text
ComboAttack -> EActionType::ComboAttack
```

Therefore, the current action orchestrator is not the final action execution decision owner.

Its current responsibility is closer to:

```text
request gate
intent -> action type conversion
delegation to ActionComponent
FActionExecutionResult -> FActionRequestResult conversion
```

---

## 5. Current ActionComponent Responsibility

`UCActionComponent` currently owns both action definition data and executor instance cache.

```text
ActionDefinitions
-> editor-injected action definition data

ActionContainer
-> EActionType -> UCAction instance cache

CurrentActionType
-> current active action type
```

In `BeginPlay()`, it iterates over `ActionDefinitions` and creates `UCAction` instances through `CreateAction()`.

`ExecuteAction()` currently performs:

```text
validates owner
finds executor by incoming action type
builds FActionExecutionQuery
calls incomingAction->DecideExecution(query)
dispatches Start / Chain / Enqueue / Interrupt / Ignore / Reject by decision
```

Therefore, `ActionComponent::ExecuteAction()` is not only a decision application owner.

It currently owns execution query construction and decision dispatch as well.

Current Start flow:

```text
StartAction()
-> EnterActionState()
-> StateComp::SetActionState()
-> changes CurrentActionType
-> calls UCAction::Start()
-> rolls back with ExitActionState() on failure
```

Current Chain flow:

```text
ApplyActionChain()
-> calls UCAction::ApplyChain(query)
```

`Enqueue` and `Interrupt` exist as enum values, but they are currently rejected.

```text
Enqueue   -> TODO then Reject
Interrupt -> TODO then Reject
```

---

## 6. Current CAction Responsibility

`UCAction` is the base executor class, but it also owns part of the final decision.

Base `UCAction::DecideExecution()` returns `Start` under:

```text
ExecutionState == Idle
CurrentActionType == Idle
```

Otherwise, it returns `Reject`.

`UCAction::Start()` enables the runtime flag, requests feedback, and emits an action event.

```text
bIsAction = true
RequestFeedback(ActionStart)
EmitActionEvent(ActionStarted)
```

`UCAction::Complete()` requests action end feedback, emits complete event, and clears the runtime flag.

```text
RequestFeedback(ActionEnd)
EmitActionEvent(ActionCompleted)
bIsAction = false
```

`UCAction::Abort()` emits abort event and clears the runtime flag.

```text
EmitActionEvent(ActionAborted)
bIsAction = false
```

Therefore, `CAction` currently owns montage lifecycle and feedback while also owning part of execution decision through `DecideExecution()`.

---

## 7. Current ComboAttack Flow

`UCAction_ComboAttack` is the most important current action execution baseline.

Current `DecideExecution()` checks:

```text
validates OwnerCharacter
validates WeaponComponent
rejects when current weapon type is Unarmed
validates ActionData and Montage for current ActionIndex
starts when ExecutionState is Idle and CurrentActionType is Idle
chains when CurrentActionType is ComboAttack and chain window is open
rejects otherwise
```

On first entry, `Start()` plays the montage for the current `ActionIndex`.

```text
ActionDatas[ActionIndex].BeginPlayMontage()
```

Chain input does not immediately play the next montage.

Current `ApplyChain()` only records:

```text
bChainWindowOpened = false
bHasChainedInput = true
```

Actual combo advancement occurs when `AdvanceCombo()` is called by notify.

```text
AdvanceCombo()
-> checks bHasChainedInput
-> checks CanAdvanceCombo()
-> ++ActionIndex
-> ActionDatas[ActionIndex].BeginPlayMontage()
```

Current combo chain flow is:

```text
ChainWindowOpened
-> action request re-enters
-> DecideExecution() returns Chain
-> ApplyChain() buffers chained input
-> AdvanceCombo notify plays next montage
```

Enemy automatically requests chain again from `OnActionEvent(ChainWindowOpened)`.

```text
ACEnemy::OnActionEvent()
-> RequestChainCombatAction()
-> ResolveChainCombatIntent()
-> HandleAICombatAction()
```

This flow is a major gameplay baseline that must not change during refactor.

---

## 8. Current Equip / Unequip Flow

`UCAction_Equip` and `UCAction_Unequip` use montage data at action data index 0.

Equip decision is:

```text
validates OwnerCharacter
validates WeaponComponent
rejects when current weapon type is not Unarmed
validates ActionDatas[0] and Montage
starts when ExecutionState is Idle and CurrentActionType is Idle
```

Unequip decision is:

```text
validates OwnerCharacter
validates WeaponComponent
rejects when current weapon type is Unarmed
validates ActionDatas[0] and Montage
starts when ExecutionState is Idle and CurrentActionType is Idle
```

Actual weapon state mutation is performed by montage notify APIs.

```text
UCAction_Equip::AttachWeapon()
-> WeaponComp::AttachWeaponToHand()
-> WeaponComp::CommitEquipWeapon()

UCAction_Unequip::DetachWeapon()
-> WeaponComp::AttachWeaponToHolster()
-> WeaponComp::CommitUnequipWeapon()
```

Equip / Unequip are simpler than combo, but they depend on weapon state and montage notify timing.

---

## 9. Current Lifecycle Flow

Action lifecycle is currently split between component and executor.

Start flow:

```text
ActionComponent::StartAction()
-> EnterActionState()
-> Action::Start()
-> ActionData::BeginPlayMontage()
```

Complete flow:

```text
ActionComponent::CompleteCurrentAction()
-> CurrentAction::Complete()
-> ExitActionState()
```

Abort flow:

```text
ActionComponent::AbortCurrentAction(reason)
-> CurrentAction::Abort(reason)
-> ExitActionState()
```

Combo / Equip / Unequip call `EndPlayMontage()` in `Complete()` and `Abort()` before calling base cleanup.

The important lifecycle point is that `ActionComponent` cleans `ExecutionState` and `CurrentActionType`, while `CAction` cleans action-local flags / feedback / events / montage state.

---

## 10. Current Problem Summary

The current structure already has the benefit that Player and Enemy share the orchestrator entry.

However, final action execution decision is not yet owned by the orchestrator.

Current responsibility distribution is:

```text
ActionOrchestrator
-> owns common gate and intent-to-action-type conversion

ActionComponent
-> owns executor lookup, query build, decision dispatch, and state mutation together

CAction
-> owns montage lifecycle and feedback while also owning part of final decision through DecideExecution()
```

This works for the current combo / equip / unequip scope.

However, when guard / parry / dodge / counter / cancel / queue are added, decision responsibility will become less clear.

Therefore, the next refactor should move toward:

```text
ActionOrchestrator
-> expands into candidate / context / policy / query / final decision owner

ActionComponent
-> shrinks into active state and decision application owner

CAction
-> shrinks into montage lifecycle and executor-local rule provider
```

---

## 11. Refactor Baseline

Gameplay results that must be preserved in the first refactor:

```text
Player basic combo attack must start the same way
Player combo chain must continue with the same chain window and ActionIndex rules
Enemy BT combat action request must still succeed based on Started result
Enemy chain request must still receive Chained result after ChainWindowOpened event
Equip / Unequip must keep the same weapon type conditions and notify timing
CurrentActionType and ExecutionState cleanup must not be missed after Complete / Abort
Action feedback / action event / hit context flow must not happen earlier or later than before
```

Parts allowed to change:

```text
where final decision is made
query/context structure names and scopes
ActionComponent internal dispatch API names
CAction::DecideExecution() role and name
```

Parts that must not change:

```text
timing where combo chain plays the next montage
timing where equip / unequip notify commits weapon state
BT task contract that commits cooldown based on Started result
Complete / Abort cleanup result
```

---