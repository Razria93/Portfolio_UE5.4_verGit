# Action Request Entry와 Execution Pipeline 구조 결정

## 1. 제목

M05-S11: Action Request Entry와 Execution Pipeline 구조 결정

---

## 2. 목적

본 문서는 `feature/action-orchestration` 브랜치에서 Player / AI 실행 흐름 비대칭을 해소하기 위해 action request entry와 action execution pipeline을 어떻게 구성했는지 정리하는 문서임.

핵심은 Player input과 AI BehaviorTree가 서로 다른 source에서 출발하더라도, 실제 action 실행은 `UCActionOrchestratorComponent -> UCActionComponent -> UCAction` 경로로 진입하도록 정리한 것임.

이 문서는 request entry, action type resolve, execution decision, component apply, executor lifecycle의 책임 경계를 설명하는 기준으로 사용함.

---

## 3. 관련 브랜치

- `feature/action-orchestration`

---

## 4. 기존 시스템의 형태

### 1) Player action 실행

기존 Player action은 input 이후 `UCActionComponent`와 `UCAction` 중심으로 실행되었음.

```yaml
Player Input
-> UCActionComponent
-> UCAction
-> Montage 실행
```

### 2) AI attack 실행

기존 AI attack은 BehaviorTree task가 attack montage와 Blackboard state를 직접 처리했음.

```yaml
BehaviorTree
-> CBTTask_StartAttack
-> AttackMontage 선택
-> Montage_Play
-> WeaponContext push
-> Blackboard attack state / cooldown 갱신
```

### 3) ComboAttack 실행

기존 Player `ComboAttack`은 `UCAction_ComboAttack` 내부 pre-input buffering으로 다음 montage를 이어갔음.

```yaml
UCAction_ComboAttack
-> PreInput notify
-> bEnablePreInput / bExistPreInput 갱신
-> NextPlayAction
-> 다음 ActionIndex montage 재생
```

---

## 5. 기존 시스템의 문제 분석 및 한계

### 1) Player / AI 실행 진입점 분리

Player와 AI는 같은 combat action 계열 동작을 서로 다른 진입점으로 실행했음.

```yaml
분리된 진입점
- Player : input -> UCActionComponent / UCAction
- AI     : BehaviorTree task -> Montage_Play / Blackboard 갱신
```

이 상태에서는 Player와 AI가 같은 action lifecycle을 공유하기 어려웠음.

### 2) Intent와 lifecycle 책임 혼재

AI BehaviorTree는 실행할 action을 결정하는 역할뿐 아니라 montage 실행과 state 갱신도 함께 처리했음.

```yaml
섞인 책임
- AI 판단
- attack montage 실행
- weapon context 전달
- blackboard combat state 갱신
- cooldown commit
```

BehaviorTree는 AI 판단 source에 가깝고, 실제 action lifecycle은 공통 실행 계층에서 처리하는 것이 더 적절했음.

### 3) Combo Chain 공통 처리 어려움

Player combo는 `UCAction_ComboAttack` 내부 state로 이어졌고, AI attack은 Blackboard의 `AttackIndex`와 task montage 선택으로 처리되었음.

```yaml
분리된 chain 기준
- Player : UCAction_ComboAttack 내부 pre-input state
- AI     : Blackboard AttackIndex / AttackMontage
```

같은 `ComboAttack`이라도 Player / AI가 다른 기준으로 다음 실행을 결정하면 chain timing과 cleanup 기준을 공통화하기 어려웠음.

---

## 6. 구조 결정 및 내용

### 1) 메인 아이디어

Player와 AI의 판단 source는 유지하되, 실제 action 실행은 공통 request entry로 진입하도록 정리함.

```yaml
Player
-> input
-> action request

AI
-> BehaviorTree / Blackboard
-> action request

Common
-> UCActionOrchestratorComponent
-> UCActionComponent
-> UCAction
```

### 2) Action Request 구조

Action request는 source, intent type, intent event를 분리해서 표현함.

```yaml
FMovementActionRequest
- IntentSource
- IntentType
- IntentEvent
- Axis2D

FEquipmentActionRequest
- IntentSource
- IntentType
- IntentEvent

FCombatActionRequest
- IntentSource
- IntentType
- IntentEvent
```

이 구조를 통해 Player input과 AI BehaviorTree가 같은 request shape로 orchestrator에 진입할 수 있게 됨.

### 3) Orchestrator Request Entry

`UCActionOrchestratorComponent`는 action 종류별 request entry를 제공함.

```yaml
UCActionOrchestratorComponent
- RequestMovementAction
- RequestEquipmentAction
- RequestCombatAction
- CanAcceptActionRequest
```

각 request entry는 공통 gate를 통과한 뒤 intent를 `EActionType`으로 resolve하고 `UCActionComponent`로 전달함.

```yaml
RequestCombatAction
-> CanAcceptActionRequest
-> ResolveCombatActionType
-> UCActionComponent::ExecuteAction
-> BuildRequestResult
```

### 4) Component Execution Pipeline

`UCActionComponent`는 action object를 조회하고, execution query를 구성한 뒤 `UCAction`에 실행 판단을 질의함.

```yaml
UCActionComponent::ExecuteAction
-> action object 조회
-> BuildActionExecutionQuery
-> UCAction::DecideExecution
-> StartAction / ApplyActionChain
-> FActionExecutionResult 반환
```

`UCActionComponent`는 active action state와 state enter / exit를 적용하는 계층으로 정리됨.

### 5) Executor Lifecycle

`UCAction`은 montage 기반 action lifecycle과 notify / feedback timing을 처리함.

```yaml
UCAction
- Start
- ApplyChain
- Complete
- Abort
- PushHitContext
- ClearHitContext
- RequestFeedback
- EmitActionEvent
```

`UCAction`은 action별 실행 규칙을 제공하지만, Player / AI 진입 차이는 처리하지 않음.

### 6) Combo Chain Pipeline

`ComboAttack`은 `ChainWindow`와 `ActionEvent`를 기준으로 Player / AI follow-up request를 연결함.

```yaml
UCAnimNotify_ChainWindow
-> UCAction_ComboAttack::OpenChainWindow
-> EActionEventType::ChainWindowOpened
-> UCActionComponent::OnActionEvent
-> Player input 또는 AI follow-up request
-> RequestCombatAction
-> UCAction_ComboAttack::ApplyChain
-> UCAction_ComboAttack::AdvanceCombo
```

이 구조로 AI도 BT 내부에서 montage를 직접 이어가지 않고, Player와 같은 combat request 경로로 다음 combo를 요청할 수 있게 됨.

### 7) Request Result 기준

외부 호출자는 `FActionRequestResult`를 통해 request 처리 결과를 확인함.

```yaml
FActionRequestResult
- ResultType
- RejectReason
- ResolvedActionType
```

AI `UCBTTask_StartCombatAction`은 `Started` 결과일 때만 cooldown을 commit함.

```yaml
UCBTTask_StartCombatAction
-> ACEnemy::HandleAICombatAction
-> RequestCombatAction
-> FActionRequestResult 확인
-> Started일 때 NextCombatActionTime 갱신
```

---

## 7. 결과

### 1) Player / AI 실행 진입점 공통화

Player input과 AI BehaviorTree가 모두 action request를 통해 `UCActionOrchestratorComponent`로 진입하게 됨.

```yaml
공통화 결과
- Player / AI request source 분리
- action 실행 pipeline 공유
- montage lifecycle을 UCAction으로 집중
```

### 2) BehaviorTree 책임 축소

BehaviorTree는 attack montage를 직접 실행하는 계층이 아니라, combat action intent를 선택하고 request result를 관찰하는 계층으로 정리됨.

```yaml
BehaviorTree 역할
- AI 판단 context 확인
- combat action intent 선택
- action request 발행
- request result 기준 cooldown / wait 처리
```

### 3) Combo Chain 경로 통일

AI combo chain은 BT 내부 montage 재생이 아니라 `ChainWindowOpened` 이후 공통 combat request를 다시 발행하는 구조로 변경됨.

```yaml
AI Combo Chain
-> ChainWindowOpened
-> Enemy callback
-> HandleAICombatAction
-> RequestCombatAction
-> Chained result
```

---

## 8. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 또는 선행 구조 기준으로 함께 읽을 수 있음.

### 1) 같은 작업 단위

- `D16`
- `P15`

### 2) 선행 구조

- `S10`

---

## 9. 결론

S11의 핵심은 action execution을 추상적인 후보 / context / policy 설계안으로 확장하는 것이 아니라, `feature/action-orchestration` 브랜치에서 Player와 AI가 같은 action request entry와 execution pipeline을 공유하도록 정리한 것임.

변경 후 Player와 AI는 서로 다른 판단 source를 유지하지만, 실제 action 실행은 `UCActionOrchestratorComponent -> UCActionComponent -> UCAction` 경로에서 처리됨.

이 구조는 이후 더 정교한 decision / intervention 모델로 확장하기 전, Player / AI action lifecycle을 공통화하기 위한 기준 pipeline임.

---
