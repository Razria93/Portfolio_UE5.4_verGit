# Current Action Execution Flow Baseline

## 1. 목적

본 문서는 action orchestration refactor를 시작하기 전에 현재 action 실행 흐름을 코드 기준으로 고정하기 위한 baseline 문서임.

목적은 새 구조를 먼저 정의하는 것이 아니라, 현재 코드에서 request / decision / lifecycle 책임이 어디에 위치하는지 기록하는 것임.

이 문서는 이후 `ActionOrchestrator -> ActionComponent -> CAction` 구조로 책임을 이동할 때 비교 기준으로 사용함.

---

## 2. 현재 진입 경로

현재 Player와 Enemy는 모두 `UCActionOrchestratorComponent`를 통해 movement / equipment / combat action request를 전달함.

Player 진입 경로는 다음과 같음.

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

Enemy 진입 경로는 다음과 같음.

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

따라서 현재 request entry 자체는 Player와 Enemy가 같은 action orchestrator를 공유하는 구조임.

---

## 3. AI Combat Action 흐름

Enemy combat action은 Behavior Tree task에서 시작됨.

현재 흐름은 다음과 같음.

```text
UCBTTask_StartCombatAction::ExecuteTask()
-> Blackboard bCanCombatAction 확인함
-> Blackboard bIsCombatAction 확인함
-> 필요 시 AIController::StopMovement() 호출함
-> ACEnemy::HandleAICombatAction(CombatActionIntent) 호출함
-> UCActionOrchestratorComponent::RequestCombatAction() 호출함
```

Task는 action request 결과가 `Started`인 경우에만 성공으로 처리함.

```text
requestResult.IsAccepted()
requestResult.ResultType == Started
```

성공 이후에는 `NextCombatActionTime`을 blackboard에 기록함.

따라서 현재 BT task는 action 실행 가능 여부를 일부 선검사하고, 실제 combat action 시작 요청은 orchestrator를 경유함.

---

## 4. 현재 ActionOrchestrator 책임

현재 `UCActionOrchestratorComponent`는 request source와 intent를 받아 공통 gate와 단순 action type resolve를 수행함.

공통 gate는 `CanAcceptActionRequest()`에서 처리함.

```text
OwnerCharacter 유효성 확인함
HealthComponent / StateComponent 유효성 확인함
alive 상태 확인함
ExecutionState::Reaction 차단함
ExecutionState::Dead 차단함
```

Movement request는 orchestrator가 직접 `MovementComponent` API를 호출함.

```text
Move   -> MovementComp::OnMove()
Walk   -> MovementComp::OnWalk()
Run    -> MovementComp::OnRun()
Sprint -> MovementComp::OnSprint()
Jump   -> MovementComp::OnJump()
```

`StopJump`는 release-style cleanup으로 보고 hard-block gate 전에 처리함.

Equipment request는 intent를 action type으로 변환한 뒤 `ActionComponent::ExecuteAction()`에 위임함.

```text
Equip   -> EActionType::Equip
Unequip -> EActionType::Unequip
Toggle  -> current weapon type 기준 Equip 또는 Unequip
```

Combat request는 현재 `ComboAttack`만 action type으로 변환함.

```text
ComboAttack -> EActionType::ComboAttack
```

즉 현재 action orchestrator는 final action execution decision owner가 아님.

현재 orchestrator는 다음 책임에 가까움.

```text
request gate
intent -> action type 변환
ActionComponent 실행 위임
FActionExecutionResult -> FActionRequestResult 변환
```

---

## 5. 현재 ActionComponent 책임

현재 `UCActionComponent`는 action definition data와 executor instance cache를 함께 소유함.

```text
ActionDefinitions
-> editor-injected action definition data

ActionContainer
-> EActionType -> UCAction instance cache

CurrentActionType
-> 현재 active action type
```

`BeginPlay()`에서 `ActionDefinitions`를 순회하고 `CreateAction()`을 통해 `UCAction` instance를 생성함.

현재 `ExecuteAction()`은 다음 작업을 수행함.

```text
owner 유효성 확인함
incoming action type으로 executor 조회함
FActionExecutionQuery 구성함
incomingAction->DecideExecution(query) 호출함
decision에 따라 Start / Chain / Enqueue / Interrupt / Ignore / Reject 분기함
```

따라서 현재 `ActionComponent::ExecuteAction()`은 단순 decision application owner가 아니라, execution query 생성과 decision dispatch를 함께 담당함.

현재 Start 처리는 다음과 같음.

```text
StartAction()
-> EnterActionState()
-> StateComp::SetActionState()
-> CurrentActionType 변경함
-> UCAction::Start() 호출함
-> 실패 시 ExitActionState()로 rollback함
```

현재 Chain 처리는 다음과 같음.

```text
ApplyActionChain()
-> UCAction::ApplyChain(query) 호출함
```

현재 Enqueue / Interrupt decision은 enum에 존재하지만 실제 구현은 reject로 처리됨.

```text
Enqueue   -> TODO 후 Reject
Interrupt -> TODO 후 Reject
```

---

## 6. 현재 CAction 책임

현재 `UCAction`은 executor base class이지만 final decision 일부도 담당함.

기본 `UCAction::DecideExecution()`은 다음 조건에서 `Start`를 반환함.

```text
ExecutionState == Idle
CurrentActionType == Idle
```

그 외에는 `Reject`를 반환함.

`UCAction::Start()`는 action runtime flag를 켜고 feedback과 action event를 발생시킴.

```text
bIsAction = true
RequestFeedback(ActionStart)
EmitActionEvent(ActionStarted)
```

`UCAction::Complete()`는 action end feedback과 complete event를 발생시키고 runtime flag를 정리함.

```text
RequestFeedback(ActionEnd)
EmitActionEvent(ActionCompleted)
bIsAction = false
```

`UCAction::Abort()`는 abort event를 발생시키고 runtime flag를 정리함.

```text
EmitActionEvent(ActionAborted)
bIsAction = false
```

즉 현재 `CAction`은 montage lifecycle과 feedback을 담당하면서 동시에 `DecideExecution()`을 통해 실행 판단 일부도 담당함.

---

## 7. 현재 ComboAttack 흐름

`UCAction_ComboAttack`은 현재 가장 중요한 action execution 기준선임.

현재 `DecideExecution()`은 다음 조건을 판단함.

```text
OwnerCharacter 유효성 확인함
WeaponComponent 유효성 확인함
Unarmed 상태면 Reject함
현재 ActionIndex의 ActionData와 Montage 유효성 확인함
Idle 상태이고 CurrentActionType이 Idle이면 Start함
CurrentActionType이 ComboAttack이고 chain window가 열려 있으면 Chain함
그 외에는 Reject함
```

첫 진입 시 `Start()`는 현재 `ActionIndex`의 montage를 재생함.

```text
ActionDatas[ActionIndex].BeginPlayMontage()
```

Chain 입력은 즉시 다음 montage를 재생하지 않음.

현재 `ApplyChain()`은 다음 상태만 기록함.

```text
bChainWindowOpened = false
bHasChainedInput = true
```

실제 combo 진행은 notify에서 `AdvanceCombo()`가 호출될 때 발생함.

```text
AdvanceCombo()
-> bHasChainedInput 확인함
-> CanAdvanceCombo() 확인함
-> ++ActionIndex
-> ActionDatas[ActionIndex].BeginPlayMontage()
```

따라서 현재 combo chain은 다음 흐름임.

```text
ChainWindowOpened
-> action request 재진입
-> DecideExecution()이 Chain 반환함
-> ApplyChain()이 chained input을 buffer함
-> AdvanceCombo notify에서 다음 montage 재생함
```

Enemy는 `OnActionEvent(ChainWindowOpened)`를 통해 chain request를 자동으로 다시 요청함.

```text
ACEnemy::OnActionEvent()
-> RequestChainCombatAction()
-> ResolveChainCombatIntent()
-> HandleAICombatAction()
```

이 흐름은 리팩터링 중 gameplay 결과가 바뀌면 안 되는 주요 기준선임.

---

## 8. 현재 Equip / Unequip 흐름

`UCAction_Equip`과 `UCAction_Unequip`은 각각 action data index 0의 montage를 사용함.

Equip 실행 판단은 다음과 같음.

```text
OwnerCharacter 유효성 확인함
WeaponComponent 유효성 확인함
현재 weapon type이 Unarmed가 아니면 Reject함
ActionDatas[0]과 Montage 유효성 확인함
Idle 상태이고 CurrentActionType이 Idle이면 Start함
```

Unequip 실행 판단은 다음과 같음.

```text
OwnerCharacter 유효성 확인함
WeaponComponent 유효성 확인함
현재 weapon type이 Unarmed이면 Reject함
ActionDatas[0]과 Montage 유효성 확인함
Idle 상태이고 CurrentActionType이 Idle이면 Start함
```

Equip / Unequip의 실제 weapon state 변경은 montage notify 계열 API에서 처리됨.

```text
UCAction_Equip::AttachWeapon()
-> WeaponComp::AttachWeaponToHand()
-> WeaponComp::CommitEquipWeapon()

UCAction_Unequip::DetachWeapon()
-> WeaponComp::AttachWeaponToHolster()
-> WeaponComp::CommitUnequipWeapon()
```

현재 Equip / Unequip은 combo보다 단순하지만, weapon state와 montage notify timing에 의존함.

---

## 9. 현재 Lifecycle 흐름

현재 action lifecycle은 component와 executor가 나누어 처리함.

Start 흐름은 다음과 같음.

```text
ActionComponent::StartAction()
-> EnterActionState()
-> Action::Start()
-> ActionData::BeginPlayMontage()
```

Complete 흐름은 다음과 같음.

```text
ActionComponent::CompleteCurrentAction()
-> CurrentAction::Complete()
-> ExitActionState()
```

Abort 흐름은 다음과 같음.

```text
ActionComponent::AbortCurrentAction(reason)
-> CurrentAction::Abort(reason)
-> ExitActionState()
```

Combo / Equip / Unequip은 `Complete()`와 `Abort()`에서 현재 montage를 `EndPlayMontage()`로 정리한 뒤 base cleanup을 호출함.

현재 lifecycle에서 중요한 점은 `ActionComponent`가 `ExecutionState`와 `CurrentActionType`을 정리하고, `CAction`이 action-local flag / feedback / event / montage 정리를 수행한다는 점임.

---

## 10. 현재 문제 요약

현재 구조는 이미 Player와 Enemy가 orchestrator entry를 공유한다는 장점이 있음.

그러나 final action execution decision은 아직 orchestrator에 있지 않음.

현재 책임 분산은 다음과 같음.

```text
ActionOrchestrator
-> common gate와 intent-to-action-type 변환을 담당함

ActionComponent
-> executor lookup, query build, decision dispatch, state mutation을 함께 담당함

CAction
-> montage lifecycle과 feedback을 담당하면서 DecideExecution()으로 final decision 일부를 담당함
```

이 구조는 현재 combo / equip / unequip에서는 동작하지만, guard / parry / dodge / counter / cancel / queue가 들어오면 decision 책임이 더 불명확해질 가능성이 높음.

따라서 후속 리팩터링의 핵심은 다음과 같음.

```text
ActionOrchestrator
-> candidate / context / policy / query / final decision owner로 확장함

ActionComponent
-> active state와 decision application owner로 축소함

CAction
-> montage lifecycle과 executor-local rule provider로 축소함
```

---

## 11. 리팩터링 기준선

첫 리팩터링에서 유지해야 하는 gameplay 결과는 다음과 같음.

```text
Player basic combo attack이 기존과 동일하게 시작되어야 함
Player combo chain이 기존 chain window와 ActionIndex 기준으로 이어져야 함
Enemy BT combat action request가 기존처럼 Started 결과를 기준으로 성공해야 함
Enemy chain request가 ChainWindowOpened event 이후 기존처럼 Chained 결과를 받을 수 있어야 함
Equip / Unequip action이 기존 weapon type 조건과 notify timing 기준으로 동작해야 함
Complete / Abort 이후 CurrentActionType과 ExecutionState 정리가 누락되면 안 됨
Action feedback / action event / hit context 흐름이 기존보다 먼저 또는 늦게 발생하면 안 됨
```

변경해도 되는 부분은 다음과 같음.

```text
final decision을 만드는 책임 위치
query/context 구조체 이름과 범위
ActionComponent 내부 dispatch API 이름
CAction::DecideExecution()의 역할과 이름
```

변경하면 안 되는 부분은 다음과 같음.

```text
combo chain이 실제 다음 montage를 재생하는 timing
equip / unequip notify에서 weapon state가 commit되는 timing
BT task가 Started 결과를 기준으로 cooldown을 commit하는 계약
Complete / Abort cleanup 결과
```

---