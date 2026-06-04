# A14 UE5 Portfolio Execution Layer Responsibility Decision

## 1. 목적

본 문서는 action / reaction execution pipeline에서 Local Level, Snapshot, Orchestration Level, Component, Executor가 각각 어떤 책임을 가져야 하는지 정리하기 위한 문서임.

특히 다양한 실행 조건과 개입 조건이 추가될 때 각 판단을 어느 계층에 배치해야 하는지 다룸.

## 2. 기존 시스템의 형태

### 2.1 실행 흐름

현재 action / reaction 실행은 request를 받아 candidate를 만들고, 실행 context를 resolve한 뒤 component와 executor를 통해 실제 montage lifecycle을 수행하는 흐름을 가짐.

기본 흐름은 다음과 같음.

```text
Request
-> Candidate
-> ExecutionContext
-> Local Level
-> Orchestration Level
-> Component Apply
-> Executor Lifecycle
```

### 2.2 계층별 현재 역할

현재 구조에서 executor와 component의 lifecycle은 action / reaction 양쪽이 점점 대칭화되고 있음.

```text
Executor
-> Start / Stop / Complete
-> montage lifecycle
-> notify window
-> feedback

Component
-> active execution state 저장
-> orchestration result 적용
-> directive 소비

Orchestrator
-> request 해석
-> candidate / context resolve
-> 최종 실행 result 구성
```

이 구조는 확장 가능한 기반이지만, local decision과 orchestration decision의 경계가 명확하지 않으면 cancel, interrupt, chain, dodge 같은 기능이 추가될수록 책임이 섞일 수 있음.

## 3. 기존 시스템의 문제 분석 및 한계

### 3.1 Local Level이 실행 방식까지 결정하는 문제

Local Level이 `Start`, `Cancel`, `Interrupt` 같은 값을 직접 반환하면 executor가 실행 가능성뿐 아니라 active execution과의 충돌 관계까지 판단하는 형태가 됨.

그러나 cancel과 interrupt는 단순한 실행 조건이 아니라 incoming execution과 active execution 사이의 관계 판단임.

따라서 Local Level은 실행 방식 전체를 결정하기보다, 해당 executor가 현재 context에서 실행 가능한지만 판단하는 것이 더 적절함.

권장되는 Local Level의 판단 범위는 다음과 같음.

```text
Executable
-> 실행 가능함

Chainable
-> active action 내부 chain으로 소비 가능함

Reject
-> 실행 불가함

Ignore
-> 요청을 무시함
```

### 3.2 현재 상태 조회가 여러 계층에 흩어지는 문제

실행 가능 여부와 개입 여부를 판단하려면 현재 body state, active action, active reaction 상태가 필요함.

이 상태를 각 executor나 component가 직접 조회하기 시작하면 판단 기준이 분산됨.

따라서 orchestrator가 한 시점의 상태를 snapshot으로 압축하고, local level과 orchestration level이 같은 snapshot을 기준으로 판단하는 것이 좋음.

### 3.3 개입 판단과 실행 적용이 섞이는 문제

active reaction을 cancel하고 dodge를 실행하거나, active action을 interrupt하고 hit reaction을 실행하는 경우에는 다음 정보가 필요함.

```text
무엇을 멈출지
왜 멈출지
멈춘 뒤 incoming execution을 시작할지
stop 실패 시 incoming execution을 막을지
```

이 판단은 component나 executor가 아니라 orchestration level에서 이루어져야 함.

Component는 이 결정을 다시 판단하지 않고 directive를 소비해야 함.

### 3.4 Local / Orchestration 용어의 한계

초기 구조에서는 executor 쪽 판단을 `Local Level`, active / incoming 관계를 조율하는 판단을 `Orchestration Level`로 표현했음.

이 표현은 과도기적으로는 유효했지만, 실제 책임을 설명하기에는 범위가 다소 넓었음.

`Local Level`은 executor가 자기 실행 가능성을 판단하는 계층이라는 의미였지만, 이름만 보면 cancel / interrupt / chain 같은 실행 방식까지 executor가 결정하는 것처럼 읽힐 수 있음.

`Orchestration Level` 역시 request 해석, decision 변환, intervention 판단, directive 구성, component result 구성까지 모두 포함하는 것처럼 읽힐 수 있음.

따라서 이후 구조에서는 다음처럼 용어를 더 명확히 나누는 것이 적절함.

```text
Decision Level
-> incoming execution 자체의 실행 가능성과 실행 관계를 판단함

Intervention Level
-> active execution과 incoming execution의 충돌, stop 대상, stop 이유, stop 이후 동작을 판단함

Result Composition
-> 공통 execution 결과를 action / reaction domain result로 재구성함
```

즉 A14에서 사용하는 Local / Orchestration 표현은 기존 구조와 과도기적 사고 과정을 설명하는 용어이고, 이후 구현 방향에서는 Decision / Intervention / Result Composition 기준으로 해석하는 것이 더 명확함.

## 4. 리팩터링 방향 및 내용

### 4.1 권장 계층 책임

권장 책임 분리는 다음과 같음.

```text
Decision Level
-> 실행 가능성만 판단함
-> Executable / Chainable / Reject / Ignore

Snapshot
-> 현재 body/action/reaction 상태를 압축함

Intervention Level
-> active execution과 incoming execution의 충돌을 판정함
-> 무엇을 멈출지, 왜 멈출지, 멈춘 뒤 무엇을 할지 결정함

Result Composition
-> 공통 execution 결과를 action / reaction 실행 결과로 재구성함

Component
-> directive를 소비함
-> stop 후 incoming execution을 start/chain함

Executor
-> montage lifecycle, notify window, feedback, local rule만 담당함
```

이 분리를 지키면 조건이 늘어나도 각 조건이 들어갈 위치가 명확해짐.

### 4.2 Snapshot의 역할

Snapshot은 현재 owner의 실행 상태를 한 시점에서 캡처한 값임.

권장 구조는 다음과 같음.

```cpp
USTRUCT(BlueprintType)
struct FActionExecutionSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	EActionType ActionType = EActionType::None;

	UPROPERTY(Transient)
	int32 ActionIndex = INDEX_NONE;

public:
	bool IsActive() const
	{
		return bIsActive;
	}
};

USTRUCT(BlueprintType)
struct FReactionExecutionSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bIsActive = false;

	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

public:
	bool IsActive() const
	{
		return bIsActive;
	}
};

USTRUCT(BlueprintType)
struct FExecutionSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionState ExecutionState = EExecutionState::Idle;

	UPROPERTY(Transient)
	FActionExecutionSnapshot ActiveAction = FActionExecutionSnapshot();

	UPROPERTY(Transient)
	FReactionExecutionSnapshot ActiveReaction = FReactionExecutionSnapshot();

public:
	bool HasAnyActiveExecution() const
	{
		return ActiveAction.IsActive() || ActiveReaction.IsActive();
	}

	bool IsIdle() const
	{
		return ExecutionState == EExecutionState::Idle
			&& !HasAnyActiveExecution();
	}

	bool IsInAction() const
	{
		return ExecutionState == EExecutionState::Action
			&& ActiveAction.IsActive();
	}

	bool IsInReaction() const
	{
		return ExecutionState == EExecutionState::Reaction
			&& ActiveReaction.IsActive();
	}
};
```

Snapshot은 executor pointer나 전체 data를 보관하지 않고, 현재 상태를 판단하는 데 필요한 최소 상태만 담는 것이 좋음.

### 4.3 Intervention context의 역할

Intervention context는 active execution과 incoming execution 사이의 개입 판정을 위해 사용하는 참여자 정보임.

Action / Reaction 고유 필드를 분리하고, 통합 context가 domain 기준으로 필요한 값을 반환하는 구조가 적절함.

```cpp
USTRUCT(BlueprintType)
struct FActionInterventionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bIsValid = false;

	UPROPERTY(Transient)
	EActionType ActionType = EActionType::None;

	UPROPERTY(Transient)
	int32 ActionIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 Priority = 0;

	UPROPERTY(Transient)
	class UCAction* ActionExecutor = nullptr;

public:
	bool IsValidMinimal() const
	{
		return bIsValid
			&& ActionType != EActionType::None
			&& ActionType != EActionType::Max
			&& IsValid(ActionExecutor);
	}
};

USTRUCT(BlueprintType)
struct FReactionInterventionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bIsValid = false;

	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(Transient)
	int32 Priority = 0;

	UPROPERTY(Transient)
	class UCReaction* ReactionExecutor = nullptr;

public:
	bool IsValidMinimal() const
	{
		return bIsValid
			&& ReactionType != EReactionType::None
			&& ReactionType != EReactionType::Max
			&& IsValid(ReactionExecutor);
	}
};

USTRUCT(BlueprintType)
struct FExecutionInterventionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDomain Domain = EExecutionDomain::None;

	UPROPERTY(Transient)
	FActionInterventionContext ActionContext = FActionInterventionContext();

	UPROPERTY(Transient)
	FReactionInterventionContext ReactionContext = FReactionInterventionContext();

public:
	bool IsValidMinimal() const
	{
		switch (Domain)
		{
		case EExecutionDomain::Action:
			return ActionContext.IsValidMinimal();

		case EExecutionDomain::Reaction:
			return ReactionContext.IsValidMinimal();

		default:
			return false;
		}
	}

	int32 GetPriority() const
	{
		switch (Domain)
		{
		case EExecutionDomain::Action:
			return ActionContext.Priority;

		case EExecutionDomain::Reaction:
			return ReactionContext.Priority;

		default:
			return 0;
		}
	}

	UObject* GetExecutor() const
	{
		switch (Domain)
		{
		case EExecutionDomain::Action:
			return ActionContext.ActionExecutor;

		case EExecutionDomain::Reaction:
			return ReactionContext.ReactionExecutor;

		default:
			return nullptr;
		}
	}
};
```

Snapshot은 현재 전체 상태를 나타내고, Intervention context는 충돌 판정에 참여하는 incoming / active 실행 단위를 나타냄.

### 4.4 Intervention query와 directive

Intervention query는 executor local rule을 조회하고 orchestration level에서 개입 가능성을 판단하기 위한 질의 구조임.

```cpp
USTRUCT(BlueprintType)
struct FExecutionInterventionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSnapshot Snapshot = FExecutionSnapshot();

	UPROPERTY(Transient)
	FExecutionInterventionContext Incoming = FExecutionInterventionContext();

	UPROPERTY(Transient)
	FExecutionInterventionContext Active = FExecutionInterventionContext();

	UPROPERTY(Transient)
	EExecutionStopReason RequestedStopReason = EExecutionStopReason::None;

public:
	bool IsValidMinimal() const
	{
		return Incoming.IsValidMinimal()
			&& Active.IsValidMinimal()
			&& RequestedStopReason != EExecutionStopReason::None
			&& RequestedStopReason != EExecutionStopReason::Max;
	}
};
```

Directive는 orchestration level의 최종 개입 지시사항임.

```text
TargetDomain
-> 무엇을 멈출지 표현함

StopReason
-> 왜 멈출지 표현함

StopSource
-> 누가 멈추라고 결정했는지 표현함

AfterStopAction
-> 멈춘 뒤 무엇을 할지 표현함
```

## 5. 적용 예시

### 5.1 4타 콤보와 좌클릭 / 우클릭 분기

좌클릭과 우클릭에 따라 light / heavy branch가 나뉘고, 각 index별 montage가 다르다면 candidate 단계에서 분기하는 것이 적절함.

```text
Input
-> LightAttack / HeavyAttack intent
-> Candidate에서 ActionType, Variant, Index 결정
-> Local Level에서 chain 가능 여부 판단
-> Orchestration Level에서 Chain 결정
-> Executor가 특정 notify 시점에서 next montage 소비
```

이 경우 Local Level은 chain window가 열려 있는지와 active combo 상태만 판단함.

### 5.2 특정 상태에서만 사용 가능한 액션

공중 공격, 체력 조건부 스킬, 특정 무기 전용 스킬은 Local Level에서 판단하는 것이 적절함.

```text
Snapshot
-> 현재 grounded 여부
-> health ratio
-> current weapon type

Local Level
-> 해당 executor가 자기 실행 조건을 만족하는지 판단함
```

이 조건은 active execution과의 충돌이 아니라 action 자체의 실행 조건이므로 orchestration level이 아니라 local level에 둠.

### 5.3 리액션 중에만 사용할 수 있는 dodge

Dodge가 hit reaction 중에만 사용 가능하다면 local과 orchestration이 나눠서 판단함.

```text
Local Level
-> active reaction이 있을 때만 Executable

Orchestration Level
-> active reaction을 cancel할 수 있는지 판단
-> incoming dodge가 cancel을 원하는지 확인
-> active reaction이 cancel을 허용하는지 확인
-> Reaction stop directive 발행

Component
-> active reaction stop
-> dodge start
```

### 5.4 특정 조건에서만 interrupt 가능한 액션

강공격이 일반 hit reaction에는 interrupt되지 않고 dead reaction에는 interrupt된다면 active executor local rule에서 처리하는 것이 적절함.

```text
Orchestration Level
-> active action과 incoming reaction의 충돌을 감지함
-> incoming reaction의 개입 의사를 조회함
-> active action의 개입 허용 여부를 조회함

Active Action Executor
-> super armor 상태면 일반 interrupt 거부
-> dead reaction이면 강제 interrupt 허용
-> interrupt window가 열려 있으면 interrupt 허용
```

### 5.5 Reaction vs Reaction 우선순위

Hit reaction 중 더 강한 hit reaction이나 dead reaction이 들어오는 경우는 orchestration level에서 priority와 local rule을 함께 판단함.

```text
incoming priority < active priority
-> Ignore

incoming priority >= active priority
-> incoming reaction이 interrupt를 원하는지 확인
-> active reaction이 interrupt를 허용하는지 확인
-> Reaction stop directive 발행
-> incoming reaction start
```

## 6. 이후 작업의 방향성

첫 번째 후속 작업은 action local decision을 `Executable / Chainable / Reject / Ignore` 중심으로 축소하는 것임.

두 번째 후속 작업은 reaction에도 얇은 local level을 추가하여 incoming reaction 자체의 실행 가능성을 먼저 판단하는 것임.

세 번째 후속 작업은 action / reaction orchestrator 양쪽에서 `FExecutionSnapshot`과 `FExecutionInterventionQuery`를 공통으로 사용하도록 정리하는 것임.

네 번째 후속 작업은 dodge를 첫 번째 action -> reaction cancel 사례로 구현하여 구조의 실사용 흐름을 검증하는 것임.

다섯 번째 후속 작업은 parry, guard, counter, execution 같은 기능을 combat interaction subsystem과 연결하는 것임.

## 7. 결론

확장 가능한 execution 구조를 만들기 위해서는 실행 조건, 현재 상태, 충돌 판정, 실행 적용, 실제 lifecycle을 서로 다른 계층으로 분리해야 함.

본 문서의 권장 구조는 다음과 같음.

```text
Local Level
-> 실행 가능성만 판단함

Snapshot
-> 현재 상태를 압축함

Orchestration Level
-> active execution과 incoming execution의 충돌을 판정함

Component
-> directive를 소비함

Executor
-> 실제 lifecycle과 local rule을 담당함
```

이 구조를 따르면 조건이 많은 action, reaction 중 dodge, 특정 조건 interrupt, reaction 우선순위, parry / guard / counter 같은 기능을 같은 사고방식으로 확장할 수 있음.
