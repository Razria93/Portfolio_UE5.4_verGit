# Action Execution Decision 구조 도입

## 1. 제목

M05-S13: Action Execution Decision 구조 도입

---

## 2. 목적

본 문서는 `feature/action-orchestration` 브랜치에서 action request가 `UCActionOrchestratorComponent`를 거쳐 `UCActionComponent`로 전달된 이후, 실제 실행 여부를 어떤 decision 구조로 판단했는지 정리하는 문서임.

핵심은 request entry를 공통화하는 것만으로는 action 실행 결과를 결정할 수 없으며, 현재 실행 중인 action과 새로 요청된 action을 비교하여 `Start / Chain / Reject` 같은 실행 결정을 명시해야 했다는 점임.

이 문서는 해당 브랜치 기준의 `FActionExecutionQuery`, `EActionExecutionDecision`, `FActionExecutionResult` 구조와 `UCActionComponent -> UCAction` 사이의 decision 흐름을 설명함.

---

## 3. 관련 브랜치

- `feature/action-orchestration`

---

## 4. 기존 시스템의 형태

### 1) Request 전달 이후 실행 처리

S12에서 정리한 orchestrator는 action request를 해석한 뒤 `EActionType`을 결정하고, 실제 실행 처리는 `UCActionComponent`로 전달함.

```yaml
UCActionOrchestratorComponent
-> RequestCombatAction
-> ResolveCombatActionType
-> UCActionComponent::ExecuteAction
```

### 2) Component 내부 실행 처리

`UCActionComponent`는 action object를 조회하고, 현재 실행 상태와 incoming action을 기준으로 execution query를 구성함.

```yaml
UCActionComponent::ExecuteAction
-> incoming action object 조회
-> BuildActionExecutionQuery
-> UCAction::DecideExecution
-> decision 기준 실행 적용
```

### 3) ComboAttack 판단 흐름

`UCAction_ComboAttack`은 현재 상태와 chain window를 기준으로 `Start`, `Chain`, `Reject`를 반환함.

```yaml
UCAction_ComboAttack::DecideExecution
-> Idle 상태이면 Start
-> 같은 ComboAttack 실행 중이고 chain window가 열려 있으면 Chain
-> 그 외 Reject
```

---

## 5. 기존 시스템의 문제 분석 및 한계

### 1) Request Entry만으로 실행 결과 확정 불가

orchestrator가 request source와 intent를 해석하더라도, 실제 action 실행 여부는 현재 실행 상태와 incoming action의 관계를 봐야 결정할 수 있음.

```yaml
필요한 판단 기준
- 현재 ExecutionState
- 현재 실행 중인 ActionType
- 현재 실행 중인 Action executor
- 새로 요청된 ActionType
- 새로 요청된 Action executor
```

따라서 request entry와 action type resolve 이후에도 별도의 execution decision 단계가 필요했음.

### 2) Start와 Chain의 의미 분리 필요

ComboAttack은 단순히 montage를 시작하는 action이 아니라, 실행 중인 combo 안에서 다음 action data를 이어갈 수 있어야 함.

```yaml
Start
- active action이 없거나 Idle 상태에서 새 action 시작

Chain
- active ComboAttack 내부에서 다음 combo data로 연결

Reject
- 요청 또는 현재 상태가 실행 조건을 만족하지 않음
```

Start와 Chain을 같은 실행 결과로 처리하면 combo reserve / consume timing과 신규 action start timing이 섞임.

### 3) 외부 Request Result와 내부 Execution Decision 분리 필요

외부 호출자는 request가 `Started`, `Chained`, `Rejected` 되었는지를 알아야 하고, 내부 component는 어떤 실행 경로를 적용할지 알아야 함.

```yaml
외부 반환 기준
- Started
- Chained
- Ignored
- Rejected

내부 적용 기준
- StartAction
- ApplyActionChain
- Reject 처리
```

따라서 action 실행 판단은 내부 execution decision으로 표현하고, orchestrator는 이를 request result로 변환하는 구조가 필요했음.

---

## 6. 구조 결정 및 내용

### 1) 메인 아이디어

action execution decision은 component가 query를 만들고, action executor가 자기 action 기준의 판단을 반환한 뒤, component가 decision에 맞는 실행 경로를 적용하는 방식으로 구성함.

```yaml
Decision Pipeline
-> UCActionComponent가 execution query 구성
-> UCAction이 action-specific decision 반환
-> UCActionComponent가 decision별 실행 경로 적용
-> UCActionOrchestratorComponent가 request result로 변환
```

### 2) FActionExecutionQuery 구성

`FActionExecutionQuery`는 현재 실행 상태와 incoming action 정보를 함께 전달하기 위한 query 구조체임.

```yaml
FActionExecutionQuery
- ExecutionState     : 현재 execution state
- CurrentActionType  : 현재 실행 중인 action type
- CurrentAction      : 현재 실행 중인 action executor
- IncomingActionType : 새로 요청된 action type
- IncomingAction     : 새로 요청된 action executor
```

이 query를 통해 action executor는 자기 action이 현재 상태에서 시작 가능한지, chain으로 이어질 수 있는지 판단함.

### 3) EActionExecutionDecision 구성

`EActionExecutionDecision`은 component가 적용할 내부 실행 결정을 표현함.

```yaml
EActionExecutionDecision
- Reject    : 실행 거절
- Ignore    : 유효하지만 현재 처리하지 않음
- Start     : 새 action 시작
- Chain     : active action 내부 chain 적용
- Enqueue   : 후속 예약 후보
- Interrupt : active action 중단 후 전환 후보
```

이 브랜치에서 실제 핵심 적용 대상은 `Start`, `Chain`, `Reject`임.

`Enqueue`, `Interrupt`는 decision 값으로는 존재하지만, 해당 브랜치에서는 본격적인 경쟁 상태 처리 모델로 확장되지 않았음.

### 4) FActionExecutionResult 구성

`FActionExecutionResult`는 executor가 반환하는 decision 결과임.

```yaml
FActionExecutionResult
- Decision   : action execution decision
- ActionType : decision이 대상으로 삼는 action type
```

`IsAccepted()`, `IsStarted()`, `IsChained()` 같은 helper를 통해 component와 orchestrator가 decision을 해석할 수 있게 구성함.

### 5) UCActionComponent Decision 적용

`UCActionComponent::ExecuteAction()`은 decision에 따라 실제 실행 경로를 선택함.

```yaml
UCActionComponent::ExecuteAction
-> BuildActionExecutionQuery
-> IncomingAction->DecideExecution
-> Decision == Start
	-> StartAction
-> Decision == Chain
	-> ApplyActionChain
-> Decision == Reject / Ignore
	-> request result 변환
```

이 구조에서 component는 action object와 current action state를 소유하고, executor가 반환한 decision을 실제 lifecycle 호출로 적용함.

### 6) UCAction 기본 Decision

기본 `UCAction`은 Idle 상태에서만 `Start`를 허용하고, 그 외에는 `Reject`를 반환함.

```yaml
UCAction::DecideExecution
-> ExecutionState가 Idle
-> CurrentActionType이 Idle
-> IncomingActionType이 유효함
-> Start

그 외
-> Reject
```

이는 단일 action의 기본 실행 규칙으로 사용됨.

### 7) UCAction_ComboAttack Decision

`UCAction_ComboAttack`은 combo chain을 위해 Start와 Chain을 분리함.

```yaml
UCAction_ComboAttack::DecideExecution
-> weapon / owner / action data 유효성 확인
-> Idle 상태이면 Start
-> CurrentActionType == ComboAttack
-> bChainWindowOpened == true
-> Chain

그 외
-> Reject
```

`Chain` decision이 적용되면 component는 `ApplyActionChain()`을 호출하고, executor는 이후 notify timing에서 `AdvanceCombo()`를 통해 다음 montage로 진행함.

### 8) Request Result 변환

orchestrator는 내부 execution decision을 외부 request result로 변환함.

```yaml
Execution Decision
- Start  -> Started
- Chain  -> Chained
- Ignore -> Ignored
- Reject -> Rejected
```

이를 통해 Player input과 AI BehaviorTree는 내부 실행 세부 구현을 몰라도 request 결과를 기준으로 후속 처리를 할 수 있음.

---

## 7. 결과

### 1) 실행 판단값 명시화

action 실행 결과가 단순 성공 / 실패가 아니라 `Start`, `Chain`, `Reject` 같은 decision으로 표현됨.

```yaml
Before
-> action 실행 가능 여부를 흐름 안에서 직접 처리

After
-> FActionExecutionResult로 실행 decision 반환
-> component가 decision별 적용 경로 선택
```

### 2) Combo Chain 처리 기준 분리

ComboAttack은 신규 action start와 active combo chain을 서로 다른 decision으로 처리할 수 있게 됨.

```yaml
Start
-> 첫 combo montage 시작

Chain
-> active combo 내부에서 다음 montage 예약 / 진행
```

### 3) 후속 경쟁 판단 구조의 기준 확보

이 브랜치에서는 `Enqueue`, `Interrupt`가 완성된 arbitration 모델로 구현되지는 않았지만, execution decision을 값으로 분리하면서 후속 경쟁 판단 구조를 도입할 기준이 생김.

```yaml
후속 확장 기준
- active execution과 incoming execution의 관계 판단
- interrupt / cancel / intervention 처리
- action / reaction 간 cross-domain competition 처리
```

---

## 8. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 또는 선행 구조 기준으로 함께 읽을 수 있음.

### 1) 같은 작업 단위

- `D16`
- `P15`

### 2) 선행 구조

- `S10`
- `S11`
- `S12`

---

## 9. 결론

`feature/action-orchestration` 브랜치에서 request entry가 공통화된 이후에도, 실제 action 실행은 현재 상태와 incoming action의 관계를 기준으로 별도 decision을 만들어야 했음.

이를 위해 `FActionExecutionQuery`, `EActionExecutionDecision`, `FActionExecutionResult`를 구성하고, `UCActionComponent -> UCAction` 사이에서 action-specific decision을 반환하도록 정리함.

이 문서의 핵심은 해당 브랜치 기준에서 `Start`와 `Chain`을 명시적으로 분리하여 Player / AI 공통 action request가 같은 decision 결과를 소비할 수 있게 만든 것임.

---
