# Action Orchestrator 내부 Request 처리 흐름 결정

## 1. 제목

M05-S12: Action Orchestrator 내부 Request 처리 흐름 결정

---

## 2. 목적

본 문서는 `feature/action-orchestration` 브랜치에서 `UCActionOrchestratorComponent`가 action request를 어떻게 해석하고 `UCActionComponent`로 전달하는지 정리하는 문서임.

핵심은 `UCActionOrchestratorComponent`가 Player input과 AI BehaviorTree에서 들어온 request를 공통 gate로 검증하고, intent를 action type으로 변환한 뒤, component execution으로 전달하는 흐름을 명확히 하는 것임.

이 문서는 `UCActionOrchestratorComponent` 내부 request 처리 책임과 아직 component / executor에 남아 있는 판단 책임의 경계를 설명하는 기준으로 사용함.

---

## 3. 관련 브랜치

- `feature/action-orchestration`

---

## 4. 기존 시스템의 형태

### 1) Player / AI 실행 진입점

브랜치 진입 전에는 Player와 AI의 action 실행 진입점이 분리되어 있었음.

```yaml
Player
-> input
-> UCActionComponent / UCAction

AI
-> BehaviorTree task
-> Montage_Play
-> Blackboard state 갱신
```

### 2) Orchestrator 부재 상태

브랜치 진입 전에는 Player input과 AI attack 흐름을 공통 request로 받아 처리하는 `UCActionOrchestratorComponent`가 없었음.

```yaml
입력 / AI 판단
-> 각 실행 계층으로 직접 전달
-> 실행 조건과 lifecycle을 각자 처리
```

---

## 5. 기존 시스템의 문제 분석 및 한계

### 1) 공통 Gate 부재

Player와 AI가 서로 다른 진입점에서 action을 실행하면 dead / reaction / invalid component 같은 공통 차단 조건을 같은 위치에서 처리하기 어려웠음.

```yaml
공통으로 확인해야 하는 조건
- owner 유효성
- health component 유효성
- state component 유효성
- alive 여부
- reaction 상태 여부
- dead 상태 여부
```

### 2) Intent 해석 위치 분리

Player input과 AI BehaviorTree가 생성한 의도는 서로 다르지만, 최종적으로는 `EActionType`으로 변환되어야 했음.

```yaml
해석이 필요한 intent
- movement intent
- equipment intent
- combat action intent
```

해석 위치가 분리되면 Player와 AI가 같은 action을 서로 다른 규칙으로 선택할 가능성이 커짐.

### 3) Request Result 기준 부재

외부 호출자가 action 요청 결과를 확인할 공통 결과 구조가 필요했음.

```yaml
필요한 결과 기준
- request가 처리되었는지
- action이 시작되었는지
- combo chain으로 처리되었는지
- 요청이 무시되었는지
- 요청이 거절되었는지
- 어떤 action type으로 해석되었는지
```

AI task는 request 성공 여부에 따라 cooldown을 commit해야 하므로 결과 기준이 특히 중요했음.

---

## 6. 구조 결정 및 내용

### 1) 메인 아이디어

`UCActionOrchestratorComponent`는 action execution을 직접 수행하지 않고, request 검증과 intent 해석, request result 변환을 담당함.

```yaml
UCActionOrchestratorComponent
- request entry 제공
- 공통 gate 수행
- intent -> EActionType 변환
- UCActionComponent로 execution 전달
- FActionRequestResult 구성
```

### 2) Request Entry 분리

request 종류에 따라 entry를 분리함.

```yaml
RequestMovementAction
- movement intent 처리
- MovementComponent API 호출
- release-style StopJump 처리

RequestEquipmentAction
- equipment intent 처리
- Equip / Unequip action type resolve
- UCActionComponent::ExecuteAction 호출

RequestCombatAction
- combat action intent 처리
- ComboAttack action type resolve
- UCActionComponent::ExecuteAction 호출
```

movement request는 대부분 movement component API로 직접 연결되고, equipment / combat request는 action execution pipeline으로 전달됨.

### 3) Common Request Gate

`CanAcceptActionRequest()`는 action request의 공통 차단 조건을 처리함.

```yaml
CanAcceptActionRequest
-> OwnerCharacter 유효성 확인
-> HealthComponent / StateComponent 유효성 확인
-> IsAlive 확인
-> ExecutionState == Reaction 차단
-> ExecutionState == Dead 차단
```

이 gate를 통해 Player input과 AI request가 같은 상태 차단 규칙을 공유함.

### 4) Intent Resolve

orchestrator는 request intent를 concrete action type으로 변환함.

```yaml
ResolveEquipmentActionType
- Equip   -> EActionType::Equip
- Unequip -> EActionType::Unequip
- Toggle  -> 현재 weapon state 기준 Equip / Unequip 선택

ResolveCombatActionType
- ComboAttack -> EActionType::ComboAttack
```

이 단계는 action data를 직접 resolve하는 것이 아니라, request intent를 실행할 action type으로 해석하는 단계임.

### 5) Component Execution 전달

equipment / combat action은 `UCActionComponent::ExecuteAction()`으로 전달됨.

```yaml
RequestEquipmentAction / RequestCombatAction
-> ResolveActionType
-> UCActionComponent::ExecuteAction
-> FActionExecutionResult
-> BuildRequestResult
```

이 구조에서 orchestrator는 request entry와 결과 변환을 담당하고, 실제 action object 조회와 execution decision은 `UCActionComponent`와 `UCAction`에 남아 있음.

### 6) Request Result 변환

`FActionExecutionResult`는 외부 호출자가 사용할 `FActionRequestResult`로 변환됨.

```yaml
EActionExecutionDecision::Start
-> EActionRequestResultType::Started

EActionExecutionDecision::Chain
-> EActionRequestResultType::Chained

EActionExecutionDecision::Interrupt
-> EActionRequestResultType::Interrupted

EActionExecutionDecision::Ignore
-> EActionRequestResultType::Ignored

EActionExecutionDecision::Reject
-> EActionRequestResultType::Rejected
```

이 결과는 Player input handler와 AI task가 action request 처리 결과를 판단하는 기준으로 사용됨.

---

## 7. 결과

### 1) Player / AI 공통 Request Entry 확보

Player input과 AI BehaviorTree는 서로 다른 source를 유지하지만, action 실행 요청은 `UCActionOrchestratorComponent`를 통해 들어가게 됨.

```yaml
Player
-> FCombatActionRequest
-> RequestCombatAction

AI
-> FCombatActionRequest
-> RequestCombatAction
```

### 2) 공통 상태 Gate 적용

dead / reaction 상태처럼 action request를 차단해야 하는 조건을 orchestrator에서 공통 처리하게 됨.

```yaml
공통 차단 결과
- Dead
- InReaction
- InvalidOwner
- InvalidComponent
```

### 3) 판단 책임 잔존 지점 명확화

이 브랜치 기준으로 최종 execution decision은 아직 `UCActionComponent::ExecuteAction()`과 `UCAction::DecideExecution()`에 남아 있음.

```yaml
남아 있는 판단 책임
- action object 조회
- FActionExecutionQuery 구성
- UCAction::DecideExecution
- StartAction / ApplyActionChain
```

이 지점은 후속 refactor에서 더 정교한 decision / intervention 모델로 옮길 수 있는 경계가 됨.

---

## 8. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 또는 선행 구조 기준으로 함께 읽을 수 있음.

### 1) 같은 작업 단위

- `D16`
- `P15`

### 2) 선행 구조

- `S10`
- `S11`

---

## 9. 결론

S12의 핵심은 `feature/action-orchestration` 브랜치에서 `UCActionOrchestratorComponent`가 action request의 공통 진입점으로 어떤 책임을 갖는지 정리하는 것임.

이 구조를 통해 Player와 AI는 같은 request entry와 공통 gate를 공유하게 되었고, equipment / combat intent는 action type으로 해석된 뒤 `UCActionComponent` execution pipeline으로 전달됨.

다만 이 단계에서는 action object 조회, execution query 구성, 최종 execution decision이 아직 `UCActionComponent`와 `UCAction`에 남아 있으므로, S12는 완성된 arbitration 모델이 아니라 action request entry를 공통화한 내부 처리 흐름 문서로 보는 것이 적절함.

---
