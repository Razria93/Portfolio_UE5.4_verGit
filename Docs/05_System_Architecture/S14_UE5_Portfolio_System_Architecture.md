# Action Competition Arbitration 도입 필요성과 후속 방향

## 1. 제목

M05-S14: Action Competition Arbitration 도입 필요성과 후속 방향

---

## 2. 목적

본 문서는 `feature/action-orchestration` 브랜치에서 action execution decision 구조를 도입한 이후에도 남아 있던 competition arbitration의 필요성을 정리하는 문서임.

S13에서 `Start / Chain / Reject` 중심의 action execution decision은 명시화되었지만, active execution과 incoming execution이 서로 공존할 수 없는 상황을 일반화하는 구조는 아직 완성되지 않았음.

이 문서는 해당 브랜치 기준으로 구현된 decision 범위와 미구현 경쟁 판단 범위를 구분하고, 후속 refactor에서 어떤 arbitration 구조가 필요했는지 설명함.

---

## 3. 관련 브랜치

- `feature/action-orchestration`

---

## 4. 기존 시스템의 형태

### 1) Action Execution Decision 구성

해당 브랜치의 action execution decision은 다음 값을 가지고 있었음.

```yaml
EActionExecutionDecision
- Reject
- Ignore
- Start
- Chain
- Enqueue
- Interrupt
```

이 중 실제 실행 적용의 중심은 `Start`와 `Chain`이었음.

### 2) Component Decision 적용 흐름

`UCActionComponent::ExecuteAction()`은 incoming action의 `DecideExecution()` 결과를 받아 decision별로 실행 경로를 선택했음.

```yaml
UCActionComponent::ExecuteAction
-> IncomingAction->DecideExecution
-> Start
	-> StartAction
-> Chain
	-> ApplyActionChain
-> Enqueue
	-> TODO reject
-> Interrupt
	-> TODO reject
-> Ignore / Reject
	-> request result 변환
```

### 3) Query가 표현하는 Active / Incoming 정보

`FActionExecutionQuery`는 action 내부 판단에 필요한 현재 action과 incoming action 정보를 전달했음.

```yaml
FActionExecutionQuery
- ExecutionState
- CurrentActionType
- CurrentAction
- IncomingActionType
- IncomingAction
```

이 구조는 action 내부의 `Start / Chain` 판단에는 사용할 수 있었지만, action / reaction 전체 실행 경쟁 상태를 일반화한 모델은 아니었음.

---

## 5. 기존 시스템의 문제 분석 및 한계

### 1) Enqueue / Interrupt 결정값의 적용 경로 부재

`EActionExecutionDecision`에는 `Enqueue`와 `Interrupt`가 존재했지만, 해당 브랜치에서는 실제 적용 경로가 구현되지 않았음.

```yaml
Enqueue
-> TODO
-> Reject 반환

Interrupt
-> TODO
-> Reject 반환
```

따라서 action execution decision은 확장 후보를 포함하고 있었지만, 경쟁 상태를 실제로 해결하는 arbitration model은 아직 없었음.

### 2) Active Action 중심 Query 한계

`FActionExecutionQuery`는 current action과 incoming action을 비교할 수 있지만, active reaction이나 다른 execution domain을 같은 방식으로 표현하지 못했음.

```yaml
표현 가능
- current action type
- current action executor
- incoming action type
- incoming action executor

표현 부족
- active reaction
- incoming reaction
- action / reaction domain 구분
- active execution과 incoming execution의 공통 관계
```

이 상태에서는 Dodge가 active reaction을 cancel하거나, HitReaction이 active action을 interrupt하는 상황을 같은 decision 구조로 다루기 어려움.

### 3) Cancel / Interrupt 기준 분리 필요

경쟁 상태에서는 단순히 다음에 실행될 것이 action인지 reaction인지보다, 왜 기존 실행을 멈추는지와 누가 멈추게 하는지가 중요함.

```yaml
구분해야 하는 기준
- incoming execution이 active execution을 밀어내는가
- owner가 자기 active execution을 의도적으로 접는가
- 외부 damage event가 active execution을 중단시키는가
- active execution이 incoming intervention을 허용하는가
```

따라서 `Cancel / Interrupt` 같은 표현은 결과 domain이 아니라 stop source, stop reason, active / incoming 관계를 기준으로 정리될 필요가 있었음.

### 4) Want / Allow 판단 구조 부재

경쟁 상태는 incoming execution이 개입을 원한다는 것만으로 결정할 수 없음.

```yaml
필요한 양방향 판단
- incoming execution이 active execution에 개입하려는가
- active execution이 incoming intervention을 허용하는가
```

해당 브랜치에서는 action 내부 decision이 중심이었기 때문에, active execution과 incoming execution 양쪽의 의사를 모두 확인하는 구조는 아직 분리되어 있지 않았음.

---

## 6. 후속 구조 방향

### 1) 메인 아이디어

후속 arbitration 구조는 action-only decision을 넘어서, active execution과 incoming execution을 같은 형식으로 비교할 수 있어야 함.

```yaml
Arbitration 방향
- incoming execution 구성
- active execution 구성
- 두 execution의 관계 판단
- incoming이 intervention을 원하는지 확인
- active가 intervention을 허용하는지 확인
- stop / start 지시를 명시적인 directive로 구성
```

### 2) Active / Incoming 공통 표현 필요

action과 reaction을 같은 경쟁 판단 안에서 다루려면, active와 incoming을 공통 participant로 표현할 수 있어야 함.

```yaml
필요한 공통 정보
- execution domain
- action context
- reaction context
- executor reference
- priority 또는 policy 기준
```

이 구조가 있어야 action-vs-action, action-vs-reaction, reaction-vs-action을 같은 arbitration 흐름에서 비교할 수 있음.

### 3) Relationship 분리 필요

competition arbitration은 단순 실행 가능 여부가 아니라 active execution과 incoming execution의 관계를 판단해야 함.

```yaml
Relationship 후보
- Independent
- Sequential
- Exclusive
```

`Independent`는 active execution이 없는 일반 시작에 가깝고, `Sequential`은 combo chain 같은 연속 실행에 가까우며, `Exclusive`는 active execution과 incoming execution이 공존할 수 없는 상황을 의미함.

### 4) Intervention Directive 필요

`Exclusive` 관계에서는 active execution을 어떻게 멈추고, 이후 incoming execution을 어떻게 처리할지 별도 지시가 필요함.

```yaml
Directive에 필요한 정보
- 무엇을 멈출 것인가
- 왜 멈추는가
- 누가 stop을 요청했는가
- stop 이후 incoming을 시작할 것인가
```

이 정보가 분리되어야 component가 active execution stop과 incoming execution start를 안정적으로 조율할 수 있음.

### 5) 해당 브랜치에서 남긴 기준

`feature/action-orchestration` 브랜치에서는 완성된 arbitration model을 구현하지 않았지만, 후속 구조를 도입할 기준은 남겼음.

```yaml
남은 기준
- EActionExecutionDecision에 Enqueue / Interrupt 후보 존재
- FActionExecutionQuery로 current / incoming 비교 시작
- FActionExecutionResult로 내부 decision 결과 분리
- FActionRequestResult로 외부 request 결과 변환
```

이 기준은 후속 refactor에서 decision, relationship, apply mode, intervention directive를 분리하는 출발점이 됨.

---

## 7. 결과

### 1) 구현 범위 명확화

해당 브랜치에서 구현된 핵심은 `Start / Chain / Reject` 중심의 action execution decision임.

```yaml
구현됨
- request entry 공통화
- action type resolve
- FActionExecutionQuery 구성
- Start / Chain decision 적용
- request result 변환
```

### 2) 미해결 범위 명확화

반면 active execution과 incoming execution의 일반 competition arbitration은 아직 구현되지 않았음.

```yaml
미해결
- Enqueue 적용
- Interrupt 적용
- action / reaction 공통 active participant
- Want / Allow 기반 intervention 판단
- stop directive 구성
```

### 3) 후속 Refactor 필요성 확보

S14의 결론은 새로운 구조를 이미 구현했다는 것이 아니라, `feature/action-orchestration`의 decision 구조만으로는 cross-domain competition을 처리하기 어렵다는 점을 명확히 한 것임.

---

## 8. 관련 문서

이 문서는 아래 문서들과 같은 작업 시점 또는 선행 / 후속 구조 기준으로 함께 읽을 수 있음.

### 1) 같은 작업 단위

- `D16`
- `P15`

### 2) 선행 구조

- `S10`
- `S11`
- `S12`
- `S13`

### 3) 후속 구조

- `S18`

---

## 9. 결론

`feature/action-orchestration` 브랜치는 action request entry와 execution decision 구조를 정리했지만, active execution과 incoming execution의 경쟁 상태를 일반화하지는 않았음.

따라서 S14의 핵심은 `Enqueue / Interrupt`가 decision 값으로 존재했지만 실제 적용 경로는 남아 있었고, action / reaction 간 competition을 처리하려면 별도의 arbitration 구조가 필요했다는 점을 기록하는 것임.

이 문서는 후속 refactor에서 relationship, intervention, active / incoming participant 구조를 분리해야 했던 이유를 설명하는 기준 문서임.

---
