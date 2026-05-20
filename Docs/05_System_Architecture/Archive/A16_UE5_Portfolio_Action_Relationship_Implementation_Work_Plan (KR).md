# A16 UE5 Portfolio Action Relationship Implementation Work Plan

## 1. 목적

본 문서는 action execution relationship 모델을 실제 코드에 반영하기 위한 작업 항목을 정리하는 문서임.

A15가 구조적 판단 기준을 정리하는 문서라면, 본 문서는 이후 브랜치에서 어떤 enum / struct / API / executor naming을 수정할지 기록하기 위한 작업 문서임.

## 2. 현재 코드의 형태

### 2.1 현재 action orchestration 흐름

현재 action orchestration은 다음 흐름을 가짐.

```text
RequestAction
-> ResolveActionCandidate
-> ResolveActionContext
-> BuildExecutionDecisionQuery
-> Executor ResolveExecutionDecision
-> BuildActionExecutionResult
-> ResolveInterventionDirective
-> DispatchActionDecision
-> ActionComponent ApplyActionDecision
```

이 흐름은 action component 내부에 있던 query와 decision 책임을 orchestration으로 옮긴 상태임.

따라서 이전 구조보다 책임 분리는 좋아졌지만, relationship과 apply mode가 아직 명시적으로 분리되어 있지는 않음.

### 2.2 현재 decision 값의 한계

현재 `EExecutionDecision`은 다음 값을 사용함.

```text
Executable
Chainable
Reject
Ignore
```

이 값은 실행 가능 여부와 실행 관계를 함께 표현함.

특히 `Chainable`은 decision이라기보다 active/incoming 사이의 sequential relationship에 가까움.

## 3. 수정 방향

### 3.1 Execution decision 정리

`EExecutionDecision`은 실행 요청 자체의 accept/reject/ignore만 표현하도록 정리하는 것이 적절함.

권장 구조는 다음과 같음.

```cpp
UENUM(BlueprintType)
enum class EExecutionDecision : uint8
{
	None = 0,

	Reject,
	Ignore,
	Accept,

	Max,
};
```

### 3.2 Execution relationship 추가

active와 incoming의 관계는 별도 enum으로 분리함.

```cpp
UENUM(BlueprintType)
enum class EExecutionRelationship : uint8
{
	None = 0,

	Independent,
	Sequential,
	Exclusive,

	Max,
};
```

의미는 다음과 같음.

```text
Independent
-> active execution이 없거나 incoming과 충돌하지 않음

Sequential
-> active execution의 흐름 안에서 incoming이 다음 실행으로 이어짐

Exclusive
-> active와 incoming이 공존할 수 없어 stop/intervention 판단이 필요함
```

### 3.3 Execution apply mode 추가

component가 어떤 방식으로 실행을 적용할지도 별도 enum으로 분리함.

```cpp
UENUM(BlueprintType)
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

의미는 다음과 같음.

```text
Start
-> incoming execution을 즉시 시작함

Reserve
-> incoming execution을 active executor 내부에 예약함

Intervene
-> active execution을 stop한 뒤 incoming을 실행함

StopOnly
-> active execution만 stop하고 incoming은 시작하지 않음
```

## 4. 구조체 수정안

### 4.1 FExecutionDecisionResult

현재 decision result는 decision만 담고 있음.

리팩터링 이후에는 decision, relationship, apply mode를 함께 담는 것이 적절함.

```cpp
USTRUCT(BlueprintType)
struct FExecutionDecisionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	EExecutionRelationship Relationship = EExecutionRelationship::None;

	UPROPERTY(Transient)
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;

public:
	bool IsAccepted() const
	{
		return Decision == EExecutionDecision::Accept;
	}

	bool RequiresIntervention() const
	{
		return Relationship == EExecutionRelationship::Exclusive
			&& ApplyMode == EExecutionApplyMode::Intervene;
	}

	bool RequiresReserve() const
	{
		return Relationship == EExecutionRelationship::Sequential
			&& ApplyMode == EExecutionApplyMode::Reserve;
	}
};
```

### 4.2 FActionExecutionResult

Action execution result는 request reject reason과 resolved context, directive를 포함함.

```cpp
USTRUCT(BlueprintType)
struct FActionExecutionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionDecisionResult ExecutionDecision = FExecutionDecisionResult();

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	UPROPERTY(Transient)
	FActionExecutionContext ResolvedContext = FActionExecutionContext();

	UPROPERTY(Transient)
	FExecutionInterventionDirective InterventionDirective = FExecutionInterventionDirective();

public:
	bool IsAcceptedDecision() const
	{
		return ExecutionDecision.IsAccepted();
	}

	bool RequiresIntervention() const
	{
		return ExecutionDecision.RequiresIntervention();
	}

	bool RequiresReserve() const
	{
		return ExecutionDecision.RequiresReserve();
	}
};
```

## 5. API 수정안

### 5.1 Orchestrator API

현재 `ResolveInterventionDirective`는 accepted action 전체를 처리하는 이름으로는 범위가 좁음.

권장 API 분리는 다음과 같음.

```cpp
private:
	bool ResolveActionExecutionApplication(
		const FExecutionDecisionQuery& InQuery,
		FActionExecutionResult& InOutResult,
		EActionRequestRejectReason& OutRejectReason
	) const;

	bool ResolveIndependentActionApplication(
		const FExecutionDecisionQuery& InQuery,
		FActionExecutionResult& InOutResult,
		EActionRequestRejectReason& OutRejectReason
	) const;

	bool ResolveSequentialActionApplication(
		const FExecutionDecisionQuery& InQuery,
		FActionExecutionResult& InOutResult,
		EActionRequestRejectReason& OutRejectReason
	) const;

	bool ResolveExclusiveActionIntervention(
		const FExecutionDecisionQuery& InQuery,
		FActionExecutionResult& InOutResult,
		EActionRequestRejectReason& OutRejectReason
	) const;
```

역할은 다음과 같음.

```text
ResolveActionExecutionApplication
-> decision result를 보고 independent / sequential / exclusive 흐름으로 분기함

ResolveIndependentActionApplication
-> active가 없거나 충돌하지 않는 start 흐름을 처리함

ResolveSequentialActionApplication
-> chain/reserve 관계를 검증함

ResolveExclusiveActionIntervention
-> active stop이 필요한 경우 intervention directive를 구성함
```

### 5.2 ActionComponent API

Component는 orchestration result를 소비하는 계층임.

권장 API는 다음과 같음.

```cpp
public:
	bool ApplyActionDecision(const FActionExecutionResult& InResult);
	bool RequestStopActiveAction(const FExecutionInterventionDirective& InDirective);

public:
	bool CanCommitReservedAction(const UCAction* InAction, const FActionData& InData) const;

private:
	bool ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective);
	bool StartAction(const FActionExecutionContext& InContext);
	bool ReserveAction(const FActionExecutionContext& InContext);
	bool StopActiveAction(EExecutionStopReason InStopReason);
	bool EndActiveAction(EActionFinishReason InFinishReason);
```

`ReserveAction`은 현재 `ChainActiveAction`의 장기 대체 이름으로 볼 수 있음.

다만 기존 combo 구현과의 호환을 위해 실제 교체는 reaction 구조 안정화 이후 진행하는 것이 적절함.

### 5.3 Combo executor API

Combo는 sequential relationship의 대표 케이스임.

기존 이름과 권장 이름은 다음과 같음.

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

CanCommitChain
-> CanCommitReservedCombo
```

권장 API는 다음과 같음.

```cpp
public:
	bool ReserveNextCombo(const FActionData& InData) override;

private:
	bool CanResolveSequentialCombo(const FExecutionDecisionQuery& InQuery) const;
	bool CanReserveNextCombo(const FActionData& InData) const;
	bool CanCommitReservedCombo(const FActionData& InData) const;
	bool CanConsumeReservedCombo() const;

	void ConsumeReservedCombo();
	void ClearReservedCombo();
```

## 6. 권장 작업 순서

### 6.1 Reaction 안정화 이후 진행함

현재 우선순위는 reaction executor/component/orchestrator 구조를 action 쪽과 맞추고 안정화하는 것임.

Action relationship 모델은 구조적으로 타당하지만, reaction 쪽 흐름이 다시 바뀔 가능성이 있으므로 먼저 reaction 안정화 커밋을 만든 뒤 적용하는 것이 적절함.

### 6.2 1차 작업

1차 작업은 enum과 result 구조를 정리하는 것임.

```text
EExecutionDecision 정리함
EExecutionRelationship 추가함
EExecutionApplyMode 추가함
FExecutionDecisionResult 확장함
FActionExecutionResult 사용부 갱신함
```

### 6.3 2차 작업

2차 작업은 orchestrator 분기를 정리하는 것임.

```text
ResolveInterventionDirective 역할 축소함
ResolveActionExecutionApplication 추가함
Independent / Sequential / Exclusive 분기 추가함
Exclusive에서만 intervention directive 생성함
```

### 6.4 3차 작업

3차 작업은 component와 executor naming을 정리하는 것임.

```text
ChainActiveAction을 ReserveAction 계열로 정리함
ApplyChain을 ReserveNextCombo로 정리함
AdvanceCombo를 ConsumeReservedCombo로 정리함
PendingChainData를 ReservedComboData로 정리함
소비 시점 검증 API를 추가함
```

### 6.5 4차 작업

4차 작업은 dodge를 실제 기능으로 연결하는 것임.

```text
UCAction_Dodge 추가함
Dodge action data 등록함
Dodge는 active reaction을 Cancelled stop reason으로 개입함
ReactionComponent stop directive 소비 흐름을 검증함
```

## 7. 결론

현재 action orchestration은 component 내부 decision 책임을 밖으로 꺼낸 단계까지는 도달했지만, decision / relationship / apply mode가 아직 분리되지 않아 orchestration level의 의미가 얇게 보이는 상태임.

이를 해결하려면 실행 가능 여부와 실행 관계를 분리하고, relationship에 따라 start / reserve / intervene 흐름을 명확하게 나눠야 함.

특히 combo chain은 intervention이 아니라 sequential relationship이며, 기능적으로는 pending보다 reserve/consume 모델로 표현하는 것이 더 정확함.

따라서 이후 작업은 reaction 구조 안정화 이후 action relationship 모델을 반영하는 순서로 진행하는 것이 적절함.
