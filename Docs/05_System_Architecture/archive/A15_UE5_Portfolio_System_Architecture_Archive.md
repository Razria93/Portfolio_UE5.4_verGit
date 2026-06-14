# A15 UE5 Portfolio Action Execution Relationship Decision

## 1. 목적

본 문서는 action orchestration에서 incoming execution과 active execution의 관계를 어떻게 분류하고, 그 관계에 따라 decision / intervention / component apply 흐름을 어떻게 나눌지 정리하기 위한 문서임.

핵심은 chain을 예외 케이스로 보는 것이 아니라, active와 incoming 사이의 execution relationship 중 하나로 보고 처리 흐름을 분리하는 것임.

## 2. 기존 시스템의 형태

### 2.1 기본 실행 흐름

현재 action request는 intent를 기반으로 action candidate를 만들고, candidate를 resolved execution context로 변환한 뒤 executor에게 실행 가능 여부를 묻는 흐름을 가짐.

기본 흐름은 다음과 같음.

```text
Request
-> Candidate
-> ActionExecutionContext
-> ExecutionDecisionQuery
-> ExecutionDecisionResult
-> Intervention resolve
-> Common ExecutionResult
-> ActionExecutionResult
-> Component Apply
-> Executor Lifecycle
```

현재 decision은 다음 의미를 가짐.

```text
Executable
-> incoming action 자체가 실행 가능함

Chainable
-> incoming action이 active action의 다음 실행으로 예약 가능함

Reject
-> 실행 불가능함

Ignore
-> 유효한 요청이지만 처리하지 않음
```

### 2.2 기존 분기 방식

기존 흐름은 accepted decision이 나오면 이후 intervention directive resolve 단계로 진입하는 형태를 고려했음.

그러나 모든 accepted decision이 같은 성격을 가지는 것은 아님.

`Executable`은 active execution이 없으면 즉시 start할 수 있지만, active execution이 있으면 active와 incoming이 공존 가능한지 판단해야 함.

`Chainable`은 active execution을 멈추는 구조가 아니라 active action 내부에 다음 실행 데이터를 예약하고, 특정 notify 시점에서 소비하는 구조임.

따라서 `Executable`과 `Chainable`을 같은 intervention 경로로 보내면 책임이 흐려짐.

## 3. 기존 시스템의 문제 분석 및 한계

### 3.1 Decision과 Relationship의 혼재

현재 `EExecutionDecision`은 실행 가능 여부와 active/incoming 관계를 동시에 표현함.

예시는 다음과 같음.

```text
Executable
-> 실행 가능 여부를 표현함

Chainable
-> 실행 가능 여부와 sequential relationship을 동시에 표현함
```

이 구조에서는 decision만 보고 다음 흐름을 결정하기 어려움.

실제로 중요한 것은 incoming이 실행 가능한지 여부와, active가 있을 때 incoming이 active와 어떤 관계를 갖는지임.

### 3.2 Chain과 Intervention의 성격 차이

`Intervention`은 incoming execution을 실행하기 위해 active execution을 멈춰야 하는 상황에서 필요함.

예시는 다음과 같음.

```text
Dodge Action -> active Hit Reaction cancel
Hit Reaction -> active Attack Action interrupt
Dead Reaction -> active Action/Reaction force stop
```

이 경우 orchestration은 다음을 판단해야 함.

```text
무엇을 멈출 것인가
왜 멈출 것인가
멈춘 뒤 incoming을 시작할 것인가
incoming executor가 개입을 원하는가
active executor가 개입을 허용하는가
```

반면 chain은 active action을 멈추지 않음.

Chain은 active action의 흐름 안에서 다음 action data를 예약하고, 이후 notify 시점에서 다음 montage를 실행하는 sequential relationship임.

따라서 chain에는 `TargetDomain`, `StopReason`, `AfterStopAction` 같은 intervention directive가 필요하지 않음.

### 3.3 입력 시점과 소비 시점의 분리

Chain은 입력 시점에 즉시 다음 montage를 실행하지 않음.

현재 흐름은 다음과 같음.

```text
입력 발생
-> incoming chain candidate resolve
-> chain 가능 여부 판단
-> active combo executor에 next combo data 예약
-> advance combo notify 도달
-> 예약된 combo data 소비
-> 다음 montage 실행
```

따라서 입력 시점의 검증만으로는 충분하지 않음.

입력 이후 reaction 진입, action stop, dead state 전환 등이 발생할 수 있으므로 실제 소비 시점에서도 다시 검증해야 함.

## 4. 리팩터링 방향 및 내용

### 4.1 Decision / Relationship / Apply Mode 분리

권장 구조는 decision, relationship, apply mode를 분리하는 것임.

```text
Decision
-> incoming execution 자체가 유효한지 판단함

Relationship
-> incoming과 active의 관계를 분류함

Apply Mode
-> component가 어떤 방식으로 실행을 적용할지 결정함
```

권장 enum 구조는 다음과 같음.

```cpp
enum class EExecutionDecision : uint8
{
	None = 0,

	Reject,
	Ignore,
	Accept,

	Max,
};

enum class EExecutionRelationship : uint8
{
	None = 0,

	Independent, // active 없음 또는 active와 충돌하지 않음
	Sequential,  // active 흐름 안에서 다음 실행으로 이어짐
	Exclusive,   // active와 공존할 수 없어 stop/intervention 필요

	Max,
};

enum class EExecutionApplyMode : uint8
{
	None = 0,

	Start,
	Reserve,
	Intervene,
	StopOnly,

	Max,
};
```

이 구조에서는 `Chainable` 같은 값이 decision에 들어가지 않음.

Chain은 accepted decision 이후 relationship이 `Sequential`이고 apply mode가 `Reserve`인 케이스로 표현함.

### 4.2 관계성에 따른 분기

Action execution application은 active와 incoming의 관계에 따라 분기해야 함.

권장 분기는 다음과 같음.

```text
Independent
-> active execution이 없거나 충돌하지 않음
-> incoming을 Start함

Sequential
-> active와 incoming이 같은 실행 흐름에 속함
-> incoming을 active executor 내부에 Reserve함
-> active를 stop하지 않음

Exclusive
-> active와 incoming이 공존할 수 없음
-> intervention query를 만들고 stop directive를 resolve함

Invalid
-> Reject 또는 Ignore로 종료함
```

즉 chain은 특수 예외가 아니라 sequential relationship의 한 형태임.

### 4.3 Reserve / Consume 용어로 정리

기존 combo chain 구현에서 `PendingChainData`, `ApplyChain`, `AdvanceCombo` 같은 이름은 실제 책임을 정확하게 드러내지 못함.

기능적으로는 다음과 같음.

```text
입력 시점
-> 다음 combo data를 예약함

notify 시점
-> 예약된 combo data를 소비함
```

따라서 장기적으로는 다음 이름이 더 적절함.

```text
PendingChainData
-> ReservedComboData 또는 ReservedNextComboData

ApplyChain
-> ReserveNextCombo

AdvanceCombo
-> ConsumeReservedCombo

CanAcceptChain
-> CanReserveNextCombo

CanAdvanceCombo
-> CanConsumeReservedCombo
```

`Pending`은 외부 실행 대기 요청처럼 읽히기 쉬우므로, active executor 내부의 다음 실행 슬롯을 뜻하는 경우에는 `Reserve`가 더 명확함.

### 4.4 소비 시점 재검증

Sequential relationship은 입력 시점과 소비 시점이 분리되므로 소비 직전에 재검증해야 함.

권장 흐름은 다음과 같음.

```text
CanResolveSequentialExecution
-> 입력 시점에 active/incoming 관계가 sequential인지 판단함

ReserveSequentialExecution
-> active executor 내부에 다음 실행 데이터를 예약함

CanCommitReservedExecution
-> notify 소비 직전에 현재 runtime state에서 실행 가능한지 다시 검증함

ConsumeReservedExecution
-> 검증에 성공하면 다음 montage를 실행함
```

이 검증은 executor 단독으로만 판단하지 않고 component의 현재 runtime state도 확인해야 함.

예시는 다음과 같음.

```cpp
bool UCAction_ComboAttack::CanCommitReservedCombo(const FActionData& InData) const
{
	if (!bIsActive) return false;
	if (!InData.IsValidMinimal()) return false;
	if (!IsValid(OwnerActionComp_Injected)) return false;

	if (!OwnerActionComp_Injected->CanCommitActionChain(this, InData)) return false;

	const FActionDataKey& incomingKey = InData.ActionDataKey;

	if (incomingKey.ActionType != ActiveDataKey_Cached.ActionType) return false;
	if (incomingKey.ActionIndex != ActiveDataKey_Cached.ActionIndex + 1) return false;

	return true;
}
```

## 5. 이후 작업의 방향성

### 5.1 Action 구조 정리

Action 쪽은 다음 순서로 정리하는 것이 적절함.

```text
EExecutionDecision을 accept/reject/ignore 중심으로 정리함
EExecutionRelationship을 추가함
EExecutionApplyMode를 추가함
FExecutionDecisionResult에 relationship/apply mode를 명시함
intervention directive는 exclusive relationship에서만 생성함
combo chain 용어를 reserve/consume 기준으로 교체함
```

### 5.2 Reaction 구조와의 대칭화

Reaction 쪽도 동일한 execution model을 공유할 수 있음.

다만 reaction은 chain보다 intervention과 priority 경쟁이 중심이므로 다음 차이를 유지해야 함.

```text
Action
-> intent 기반 execution candidate가 다양함
-> sequential relationship이 존재할 수 있음
-> cancel action, dodge, counter 같은 자의적 개입이 중요함

Reaction
-> damage event 기반 incoming reaction이 중심임
-> reaction 간 priority / force / interrupt 판단이 중요함
-> dead reaction 같은 강제 개입이 필요함
```

즉 공통 구조는 공유하되, local executor rule과 relationship resolve 규칙은 action/reaction 특성에 맞게 분리해야 함.

### 5.3 Combat Interaction 확장과의 연결

향후 parry, guard, dodge, counter, execution 같은 기능이 들어오면 execution relationship 판단은 combat interaction 결과와 연결될 가능성이 높음.

예시는 다음과 같음.

```text
Perfect Parry
-> attacker action/reaction에 강제 개입 가능함

Guard Break
-> defender reaction 강제 실행 가능함

Dodge
-> active reaction을 cancel하고 dodge action을 start함

Counter
-> defensive success result 이후 active execution을 cancel하고 counter action을 start함
```

이때 orchestration은 단순 실행 호출자가 아니라 incoming과 active 사이의 관계를 분류하고, 필요한 intervention directive를 구성하는 계층으로 남아야 함.

## 6. 결론

Action orchestration에서 중요한 것은 accepted decision을 모두 같은 흐름으로 보내는 것이 아니라, incoming과 active의 relationship을 먼저 분류하는 것임.

Chain은 예외가 아니라 sequential relationship이며, intervention이 아니라 reserve/consume 흐름으로 처리해야 함.

Dodge, hit reaction, dead reaction처럼 active를 멈춰야 하는 흐름은 exclusive relationship이며, 이 경우에만 intervention directive를 생성해야 함.

따라서 장기 구조는 다음 흐름이 가장 명확함.

```text
Intent
-> Candidate
-> Execution Context
-> Execution Snapshot
-> Decision
-> Relationship
-> Apply Mode
-> Intervention Directive
-> Component Apply
-> Executor Lifecycle
```

이 구조를 따르면 action과 reaction의 실행 흐름을 대칭적으로 유지하면서도, chain / dodge / reaction interrupt처럼 성격이 다른 실행 관계를 같은 pipeline 안에서 구분할 수 있음.
