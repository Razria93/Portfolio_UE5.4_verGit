# 1차 Action Orchestration 결과와 2차 리팩터링 필요성

## 1. 제목

M05-S15: 1차 Action Orchestration 결과와 2차 리팩터링 필요성

---

## 2. 목적

본 문서는 1차 action orchestration 작업 이후의 Action Execution Pipeline을 코드 기준으로 고정하고, 이후 2차 orchestration-refactor가 왜 필요했는지 정리하기 위한 문서임.

목적은 1차 action orchestration 결과에서 request / decision / lifecycle 책임이 어떻게 배치되었는지 기록하고, 남은 한계를 후속 리팩터링 기준으로 정리하는 것임.

이 문서는 2차 리팩터링에서 action / reaction execution pipeline을 더 일반화할 때 비교 기준으로 사용함.

---

## 3. 관련 브랜치

- `feature/action-orchestration`

### 문서 기준

본 문서는 `feature/action-orchestration` 브랜치 기준의 구형 구조 문서임.

본문의 `LocalLevel`, `ResolvedPolicy`, `StopDirective` 계열 표현은 해당 브랜치에서 사용하던 구조와 용어를 유지한 것임.

이후 `feature/orchestration-refactor` 브랜치에서 해당 구조는 Decision / Relationship / ApplyMode / InterventionDirective 중심으로 재정리되었으며, 자세한 내용은 `S18`에서 다룸.

---

## 4. 2차 리팩터링 이전 Request Entry

### 개요

2차 리팩터링 이전 Player와 Enemy는 모두 `UCActionOrchestratorComponent`를 통해 movement / equipment / combat action request를 전달함.

### Player Request Entry

Player의 Request Entry는 다음과 같음.

```yaml
1. Movement
ACPlayer::HandleMove()
ACPlayer::HandleWalk()
ACPlayer::HandleRun()
ACPlayer::HandleSprint()
ACPlayer::HandleJump()
ACPlayer::HandleStopJump()
-> UCActionOrchestratorComponent::RequestMovementAction()

2. Equipment
ACPlayer::HandleEquipmentAction()
-> UCActionOrchestratorComponent::RequestEquipmentAction()

3. Combat
ACPlayer::HandleCombatAction()
-> UCActionOrchestratorComponent::RequestCombatAction()
```

### Enemy Request Entry

Enemy의 Request Entry는 다음과 같음.

```yaml
1. Movement
ACEnemy::HandleAIWalk()
ACEnemy::HandleAIRun()
ACEnemy::HandleAISprint()
ACEnemy::HandleAIJump()
ACEnemy::HandleAIStopJump()
-> UCActionOrchestratorComponent::RequestMovementAction()

2. Equipment
ACEnemy::HandleAIEquipmentAction()
-> UCActionOrchestratorComponent::RequestEquipmentAction()

3. Combat
ACEnemy::HandleAICombatAction()
-> UCActionOrchestratorComponent::RequestCombatAction()
```

따라서 2차 리팩터링 이전 Request Entry 자체는 Player와 Enemy가 같은 action orchestrator를 공유하는 구조였음.

---

## 5. AI의 Combat Action 시작

### 개요

AI Enemy combat action은 2차 리팩터링 이전에는 Behavior Tree task에서 시작됨.

### Combat Action Request Flow

```yaml
1. BT task
UCBTTask_StartCombatAction::ExecuteTask()
-> Blackboard bCanCombatAction 확인함
-> Blackboard bIsCombatAction 확인함

2. AIController
-> AIController::StopMovement() 필요시 호출함

3. Enemy
-> ACEnemy::HandleAICombatAction(CombatActionIntent) 호출함

4. Orchestrator
-> UCActionOrchestratorComponent::RequestCombatAction() 호출함
```

### Request Result Contract

Task는 action request 결과가 `Started`인 경우에만 성공으로 처리함.

```yaml
1. requestResult.IsAccepted()
2. requestResult.ResultType == Started
```

성공 이후에는 `NextCombatActionTime`을 blackboard에 기록함.

따라서 2차 리팩터링 이전 BT task는 action 실행 가능 여부를 일부 검사하고, 실제 combat action 시작 요청은 orchestrator를 경유함.

---

## 6. 2차 리팩터링 이전 ActionOrchestrator의 Request 처리

### 개요

2차 리팩터링 이전 `UCActionOrchestratorComponent`는 request source와 intent를 받아 공통 gate와 단순 action type resolve를 수행함.

### Common Gate

공통 gate는 `CanAcceptActionRequest()`에서 처리함.

```yaml
1. OwnerCharacter 유효성 확인함
2. HealthComponent / StateComponent 유효성 확인함
3. alive 상태 확인함
4. ExecutionState::Reaction 차단함
5. ExecutionState::Dead 차단함
```

### Movement Request

Movement request는 orchestrator가 직접 `MovementComponent` API를 호출함.

```yaml
1. Move   -> MovementComp::OnMove()
2. Walk   -> MovementComp::OnWalk()
3. Run    -> MovementComp::OnRun()
4. Sprint -> MovementComp::OnSprint()
5. Jump   -> MovementComp::OnJump()
```

`StopJump`는 release-style cleanup으로 보고 hard-block gate 전에 처리함.

### Equipment Request

Equipment request는 intent를 action type으로 변환한 뒤 `ActionComponent::ExecuteAction()`에 위임함.

```yaml
1. Equip   -> EActionType::Equip
2. Unequip -> EActionType::Unequip
3. Toggle  -> current weapon type 기준 Equip 또는 Unequip
```

### Combat Request

Combat request는 당시 `ComboAttack`만 action type으로 변환함.

```yaml
ComboAttack -> EActionType::ComboAttack
```

즉 2차 리팩터링 이전 action orchestrator는 final action execution decision owner가 아니었음.

### 책임 요약

2차 리팩터링 이전 orchestrator는 다음 책임에 가까웠음.

```yaml
1. request gate
2. intent -> action type 변환
3. ActionComponent 실행 위임
4. FActionExecutionResult -> FActionRequestResult 변환
```

---

## 7. 2차 리팩터링 이전 ActionComponent의 실행 판단과 적용

### Runtime Data / Executor Cache

2차 리팩터링 이전 `UCActionComponent`는 action definition data와 executor instance cache를 함께 소유함.

```yaml
1. ActionDefinitions
-> editor-injected action definition data

2. ActionContainer
-> EActionType -> UCAction instance cache

3. CurrentActionType
-> active action type
```

`BeginPlay()`에서 `ActionDefinitions`를 순회하고 `CreateAction()`을 통해 `UCAction` instance를 생성함.

### ExecuteAction 책임

2차 리팩터링 이전 `ExecuteAction()`은 다음 작업을 수행함.

```yaml
1. owner 유효성 확인함
2. incoming action type으로 executor 조회함
3. FActionExecutionQuery 구성함
4. incomingAction->DecideExecution(query) 호출함
5. decision에 따라 Start / Chain / Enqueue / Interrupt / Ignore / Reject 분기함
```

따라서 2차 리팩터링 이전 `ActionComponent::ExecuteAction()`은 단순 decision application owner가 아니라, execution query 생성과 decision dispatch를 함께 담당함.

### Start 처리

2차 리팩터링 이전 Start 처리는 다음과 같음.

```yaml
StartAction()
-> EnterActionState()
-> StateComp::SetActionState()
-> CurrentActionType 변경함
-> UCAction::Start() 호출함
-> 실패 시 ExitActionState()로 rollback함
```

### Chain 처리

2차 리팩터링 이전 Chain 처리는 다음과 같음.

```yaml
ApplyActionChain()
-> UCAction::ApplyChain(query) 호출함
```

### 미구현 Decision

2차 리팩터링 이전 Enqueue / Interrupt decision은 enum에 존재하지만 실제 구현은 reject로 처리됨.

```yaml
1. Enqueue   -> TODO 후 Reject
2. Interrupt -> TODO 후 Reject
```

---

## 8. 2차 리팩터링 이전 CAction의 Executor 책임

### 개요

2차 리팩터링 이전 `UCAction`은 executor base class이지만 final decision 일부도 담당함.

### 기본 실행 판단

기본 `UCAction::DecideExecution()`은 다음 조건에서 `Start`를 반환함.

```yaml
1. ExecutionState == Idle
2. CurrentActionType == Idle
```

그 외에는 `Reject`를 반환함.

### Start

`UCAction::Start()`는 action runtime flag를 켜고 feedback과 action event를 발생시킴.

```yaml
1. bIsAction = true
2. RequestFeedback(ActionStart)
3. EmitActionEvent(ActionStarted)
```

### Complete

`UCAction::Complete()`는 action end feedback과 complete event를 발생시키고 runtime flag를 정리함.

```yaml
1. RequestFeedback(ActionEnd)
2. EmitActionEvent(ActionCompleted)
3. bIsAction = false
```

### Abort

`UCAction::Abort()`는 abort event를 발생시키고 runtime flag를 정리함.

```yaml
1. EmitActionEvent(ActionAborted)
2. bIsAction = false
```

즉 2차 리팩터링 이전 `CAction`은 montage lifecycle과 feedback을 담당하면서 동시에 `DecideExecution()`을 통해 실행 판단 일부도 담당함.

---

## 9. 2차 리팩터링 이전 ComboAttack Chain Flow

### 개요

`UCAction_ComboAttack`은 2차 리팩터링 이전 가장 중요한 action execution 기준선임.

### DecideExecution 조건

2차 리팩터링 이전 `DecideExecution()`은 다음 조건을 판단함.

```yaml
1. OwnerCharacter 유효성 확인함
2. WeaponComponent 유효성 확인함
3. Unarmed 상태면 Reject함
4. 해당 ActionIndex의 ActionData와 Montage 유효성 확인함
5. Idle 상태이고 CurrentActionType이 Idle이면 Start함
6. CurrentActionType이 ComboAttack이고 chain window가 열려 있으면 Chain함
7. 그 외에는 Reject함
```

### Start

첫 진입 시 `Start()`는 해당 `ActionIndex`의 montage를 재생함.

```yaml
ActionDatas[ActionIndex].BeginPlayMontage()
```

Chain 입력은 즉시 다음 montage를 재생하지 않음.

### ApplyChain

2차 리팩터링 이전 `ApplyChain()`은 다음 상태만 기록함.

```yaml
1. bChainWindowOpened = false
2. bHasChainedInput = true
```

### AdvanceCombo

실제 combo 진행은 notify에서 `AdvanceCombo()`가 호출될 때 발생함.

```yaml
AdvanceCombo()
-> bHasChainedInput 확인함
-> CanAdvanceCombo() 확인함
-> ++ActionIndex
-> ActionDatas[ActionIndex].BeginPlayMontage()
```

### Chain Flow

따라서 2차 리팩터링 이전 combo chain은 다음 Flow임.

```yaml
ChainWindowOpened
-> action request 재진입
-> DecideExecution()이 Chain 반환함
-> ApplyChain()이 chained input을 buffer함
-> AdvanceCombo notify에서 다음 montage 재생함
```

### Enemy Chain Request

Enemy는 `OnActionEvent(ChainWindowOpened)`를 통해 chain request를 자동으로 다시 요청함.

```yaml
ACEnemy::OnActionEvent()
-> RequestChainCombatAction()
-> ResolveChainCombatIntent()
-> HandleAICombatAction()
```

이 Flow는 리팩터링 중 gameplay 결과가 바뀌면 안 되는 주요 기준선임.

---

## 10. 2차 리팩터링 이전 Equip / Unequip Notify Flow

### 개요

`UCAction_Equip`과 `UCAction_Unequip`은 각각 action data index 0의 montage를 사용함.

### Equip 실행 판단

Equip 실행 판단은 다음과 같음.

```yaml
1. OwnerCharacter 유효성 확인함
2. WeaponComponent 유효성 확인함
3. weapon type이 Unarmed가 아니면 Reject함
4. ActionDatas[0]과 Montage 유효성 확인함
5. Idle 상태이고 CurrentActionType이 Idle이면 Start함
```

### Unequip 실행 판단

Unequip 실행 판단은 다음과 같음.

```yaml
1. OwnerCharacter 유효성 확인함
2. WeaponComponent 유효성 확인함
3. weapon type이 Unarmed이면 Reject함
4. ActionDatas[0]과 Montage 유효성 확인함
5. Idle 상태이고 CurrentActionType이 Idle이면 Start함
```

### Notify 기반 Weapon State 변경

Equip / Unequip의 실제 weapon state 변경은 montage notify 계열 API에서 처리됨.

```yaml
1. UCAction_Equip::AttachWeapon()
-> WeaponComp::AttachWeaponToHand()
-> WeaponComp::CommitEquipWeapon()

2. UCAction_Unequip::DetachWeapon()
-> WeaponComp::AttachWeaponToHolster()
-> WeaponComp::CommitUnequipWeapon()
```

2차 리팩터링 이전 Equip / Unequip은 combo보다 단순하지만, weapon state와 montage notify timing에 의존함.

---

## 11. 2차 리팩터링 이전 Action Cleanup Flow

### 개요

2차 리팩터링 이전 action lifecycle은 component와 executor가 나누어 처리함.

### Start Flow

Start Flow는 다음과 같음.

```yaml
ActionComponent::StartAction()
-> EnterActionState()
-> Action::Start()
-> ActionData::BeginPlayMontage()
```

### Complete Flow

Complete Flow는 다음과 같음.

```yaml
ActionComponent::CompleteCurrentAction()
-> CurrentAction::Complete()
-> ExitActionState()
```

### Abort Flow

Abort Flow는 다음과 같음.

```yaml
ActionComponent::AbortCurrentAction(reason)
-> CurrentAction::Abort(reason)
-> ExitActionState()
```

### Cleanup 책임

Combo / Equip / Unequip은 `Complete()`와 `Abort()`에서 active montage를 `EndPlayMontage()`로 정리한 뒤 base cleanup을 호출함.

2차 리팩터링 이전 cleanup에서 중요한 점은 `ActionComponent`가 `ExecutionState`와 `CurrentActionType`을 정리하고, `CAction`이 action-local flag / feedback / event / montage 정리를 수행한다는 점임.

---

## 12. 2차 리팩터링 이전 책임 분산 문제

### 기존 장점

2차 리팩터링 이전 구조는 이미 Player와 Enemy가 orchestrator entry를 공유한다는 장점이 있었음.

그러나 final action execution decision은 아직 orchestrator에 있지 않음.

### 책임 분산

2차 리팩터링 이전 책임 분산은 다음과 같음.

```yaml
1. ActionOrchestrator
-> common gate와 intent-to-action-type 변환을 담당함

2. ActionComponent
-> executor lookup, query build, decision dispatch, state mutation을 함께 담당함

3. CAction
-> montage lifecycle과 feedback을 담당하면서 DecideExecution()으로 final decision 일부를 담당함
```

이 구조는 combo / equip / unequip에서는 동작하지만, guard / parry / dodge / counter / cancel / queue가 들어오면 decision 책임이 더 불명확해질 가능성이 높음.

### 후속 리팩터링 방향

따라서 후속 리팩터링의 핵심은 다음과 같음.

```yaml
1. ActionOrchestrator
-> candidate / context / policy / query / final decision owner로 확장함

2. ActionComponent
-> active state와 decision application owner로 축소함

3. CAction
-> montage lifecycle과 executor rule provider로 축소함
```

---

## 13. 리팩터링 기준선

### 유지해야 하는 부분

```yaml
1. Enemy Combat Request
-> Enemy BT combat action request는 2차 리팩터링 이전처럼 Started 결과를 기준으로 성공해야 함
-> Enemy chain request는 ChainWindowOpened event 이후 2차 리팩터링 이전처럼 Chained 결과를 받을 수 있어야 함

2. Combo Attack
-> Player basic combo attack은 2차 리팩터링 이전과 동일하게 시작되어야 함
-> Player combo chain은 2차 리팩터링 이전 chain window와 ActionIndex 기준으로 이어져야 함

3. Equip / Unequip
-> Equip / Unequip action은 2차 리팩터링 이전 weapon type 조건을 유지해야 함
-> weapon state 변경은 2차 리팩터링 이전 notify timing 기준으로 동작해야 함

4. Runtime Cleanup
-> Complete / Abort 이후 CurrentActionType과 ExecutionState 정리가 누락되면 안 됨

5. Timing Contract
-> Action feedback / action event / hit context Flow는 2차 리팩터링 이전보다 먼저 또는 늦게 발생하면 안 됨
```

### 변경해도 되는 부분

```yaml
1. Responsibility Location
-> final decision을 만드는 책임 위치

2. Data Structure Shape
-> query / context 구조체 이름과 범위

3. Internal API
-> ActionComponent 내부 dispatch API 이름
-> CAction::DecideExecution()의 역할과 이름
```

### 변경하면 안되는 부분

```yaml
1. Gameplay Timing
-> combo chain이 실제 다음 montage를 재생하는 timing
-> equip / unequip notify에서 weapon state가 commit되는 timing

2. AI Request Contract
-> BT task가 Started 결과를 기준으로 cooldown을 commit하는 계약

3. Runtime Cleanup Result
-> Complete / Abort cleanup 결과
```

---
## 14. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 또는 선행 / 후속 구조 기준으로 함께 읽을 수 있음.

### 같은 작업 단위

- `D16`
- `P15`

### 선행 구조

- `S10`
- `S11`
- `S12`
- `S13`
- `S14`

### 후속 구조

- `S18`

---

## 15. 결론

1차 Action Orchestration 결과는 Player와 Enemy가 같은 `UCActionOrchestratorComponent` entry를 공유하게 만들었다는 점에서 의미가 있음.

다만 2차 리팩터링 이전 구조에서는 final action execution decision이 아직 `ActionComponent`와 `CAction`에 분산되어 있었고, action / reaction 경쟁 상태를 공통 execution 모델로 다루기에는 한계가 남아 있었음.

따라서 후속 리팩터링에서는 orchestrator를 candidate / context / policy / query / final decision owner로 확장하고, component는 active state와 decision application owner로 축소하며, executor는 montage lifecycle과 executor rule provider로 정리하는 방향이 필요함.

---
