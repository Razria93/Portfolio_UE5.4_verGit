# Execution Orchestration API 모델

## 1. 목적

본 문서는 Action / Reaction 계열 실행 구조에서 API 책임이 혼잡해지는 문제를 줄이기 위해,  
`Request -> Resolve -> Policy -> Orchestrate -> Dispatch -> Execute` 단계로 실행 오케스트레이션 흐름을 정리하기 위한 설계 문서임.

핵심 목표는 다음과 같음.

- Orchestrator가 외부 요청을 받아 실행 후보를 구체화하는 흐름을 명확히 함.
- 실행 후보의 정책 해석과 현재 상태 기반 판정을 분리함.
- Component는 orchestration 결과를 실제 runtime state에 적용하는 계층으로 정리함.
- `CAction`, `CReaction` 같은 실행 객체는 실제 실행 lifecycle과 local policy hook을 담당하도록 유지함.


---

## 2. 문제 인식

현재 구조에서 API 이름과 책임은 다음 개념들이 섞이기 쉬움.

```text
Request
Resolve
Decide
Apply
Execute
Interrupt
Cancel
Finish
```

특히 Reaction Orchestration에서는 다음 질문들이 한 함수 안에 섞일 수 있음.

```text
무엇을 실행할 것인가?
어떤 방식으로 실행되고 싶은가?
현재 active 상태에서 실행 가능한가?
실행 가능하다면 실제 상태를 누가 바꿀 것인가?
실제 montage lifecycle은 누가 관리할 것인가?
```

이 질문들이 섞이면 Orchestrator와 Component의 경계가 흐려짐.

따라서 실행 흐름을 단계별로 분리하는 것이 필요함.


---

## 3. 권장 실행 흐름

실행 오케스트레이션은 다음 흐름으로 구성하는 것이 적절함.

```text
[Orchestration]
RequestExecution()
-> ResolveExecutionContext()
-> ResolveExecutionPolicy()
-> OrchestrateExecution()
-> DispatchExecutionDecision()

[Component]
-> Component Apply / Start / Interrupt / Cancel

[Execution]
-> Execution Object Start / Stop / Finish
```

각 단계의 의미는 다음과 같음.

```text
RequestExecution
- 외부 진입 API
- intent와 payload를 받음
- request result를 반환함

ResolveExecutionContext
- 무엇을 실행할 것인지 구체화함
- intent / payload를 기반으로 실행 후보를 구성함

ResolveExecutionPolicy
- 해당 실행 후보가 어떤 방식으로 처리되길 원하는지 해석함
- 최종 가능 여부가 아니라 실행 성향과 정책을 정리함

OrchestrateExecution
- 현재 runtime state에서 가능한지 판단함
- active / current state와 policy를 비교해 최종 decision을 생성함

DispatchExecutionDecision
- orchestration decision을 Component API로 위임함
- 실제 상태 변경은 Component에서 수행함
```


---

## 4. Orchestrator 책임

Orchestrator는 실행 lifecycle을 직접 소유하지 않음.

Orchestrator는 다음 책임을 가짐.

```text
1. 외부 요청 수신
2. 공통 request gate 처리
3. 실행 context resolve
4. 실행 policy resolve
5. 현재 runtime state를 참조하여 orchestration decision 생성
6. decision을 component에 dispatch
7. request result 반환
```

즉 Orchestrator는 “실행을 직접 수행하는 객체”가 아니라,  
“실행 후보를 평가하고 어떤 처리로 보낼지 결정하는 객체”임.


---

## 5. Component 책임

Component는 Orchestrator의 decision을 실제 캐릭터 runtime state에 적용함.

Component는 다음 책임을 가짐.

```text
1. active execution state 관리
2. executor instance cache 관리
3. Start / Interrupt / Cancel decision 적용
4. movement / state / action abort 같은 side effect 적용
5. execution finish 처리
```

Component API는 Orchestrator decision과 대응되도록 구성하는 것이 좋음.

```text
StartExecution()
InterruptExecution()
CancelExecution()
FinishExecution()
```

단순히 `Execute`, `Stop`처럼 짧게 둘 수도 있지만,
API 의미를 명확히 하려면 `StartExecution`, `InterruptExecution`, `CancelExecution`처럼 구체적으로 두는 것이 적절함.


---

## 6. Execution Object 책임

`CAction`, `CReaction` 같은 실행 객체는 실제 실행 lifecycle과 local policy hook을 담당함.

공통적으로 다음 책임을 가질 수 있음.

```text
Begin
Stop
End
Cleanup
Tick
Notify window 처리
Local policy hook
```

Reaction 기준 local policy hook 예시는 다음과 같음.

```cpp
WantToInterrupt()
AllowInterruptionBy()
WantToCancel()
AllowCancelBy()
```

즉 execution object는 orchestration의 최종 decision을 직접 만들기보다,  
orchestrator가 판단할 때 참고할 수 있는 local policy를 제공하는 것이 적절함.


---

## 7. Reaction 기준 API 모델

Reaction Orchestration은 다음 API 형태를 목표로 함.

### ReactionOrchestrator

```cpp
FReactionRequestResult RequestReaction(const FDamageReactionRequest& InRequest);

bool ResolveReactionContext(
	const FDamageReactionRequest& InRequest,
	FReactionContext& OutContext,
	EReactionType& OutReactionType,
	EReactionRequestRejectReason& OutRejectReason);

bool ResolveReactionPolicy(
	const FReactionContext& InContext,
	EReactionType InReactionType,
	FReactionExecutionPolicy& OutPolicy,
	EReactionRequestRejectReason& OutRejectReason) const;

FReactionOrchestrationResult OrchestrateQuery(
	const FReactionOrchestrationQuery& InQuery) const;

void DispatchReactionDecision(const FReactionOrchestrationResult& InResult);
```

### ReactionComponent

```cpp
bool ApplyReactionDecision(const FReactionOrchestrationResult& InResult);

bool IsActiveReaction() const;
bool GetActiveReactionContext(FReactionContext& OutContext) const;
UCReaction* GetActiveReactionExecutor() const;
```

### CReaction

```cpp
bool Start(const FReactionData& InData);
void Stop(EReactionStopReason InReason);
void FinishCompleted();
void FinishInterrupted();
void FinishCancelled();
void FinishAborted();

bool WantToInterrupt(const FReactionQueryContext& InContext) const;
bool AllowInterruptionBy(const FReactionQueryContext& InContext) const;
```


---

## 8. ReactionExecutionPolicy

`ResolveReactionPolicy()`를 분리하려면 policy 구조체를 둘 수 있음.

예상 형태는 다음과 같음.

```cpp
USTRUCT(BlueprintType)
struct FReactionExecutionPolicy
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bCanInterrupt = false;

	UPROPERTY(Transient)
	bool bForceInterrupt = false;

	UPROPERTY(Transient)
	bool bIgnoreInterruptWindow = false;

	UPROPERTY(Transient)
	int32 Priority = 0;
};
```

현재 1차 구현에서는 policy struct를 유지하되, `FReactionData.Priority`와 `CReaction` local hook을 조합해 얇게 판단함.

API 구조를 명확히 유지하려면 policy struct를 두는 편이 좋고,  
구현 복잡도를 줄이려면 `ResolveReactionPolicy()`를 얇게 두거나 후속 단계로 미룰 수 있음.


---

## 9. 단계별 의미 정리

Reaction 기준으로 각 단계의 의미는 다음과 같음.

```text
RequestReaction
- TakeDamage 이후 외부 진입점
- request result를 반환함

ResolveReactionContext
- damage result -> ReactionType
- ReactionType + ApplyDamageSpecKey -> ReactionData
- ReactionData -> ReactionExecutor
- 최종 FReactionContext 구성

ResolveReactionPolicy
- incoming reaction의 priority / interrupt 권한 / force 성향을 해석함

OrchestrateQuery
- active context와 incoming context를 비교함
- priority와 interruptible window를 확인함
- Start / Interrupt / Cancel / Ignore / Reject 결정함

DispatchReactionDecision
- 결정 결과에 따라 ReactionComponent의 API를 호출함
```


---

## 10. 설계 원칙

본 API 모델에서 지켜야 할 원칙은 다음과 같음.

- `Resolve`는 실행 후보를 만드는 단계임.
- `Policy`는 실행 후보의 처리 성향을 해석하는 단계임.
- `Orchestrate`는 현재 상태와 충돌을 평가하는 단계임.
- `Dispatch / Apply`는 실제 runtime state를 변경하는 단계임.
- `Execution Object`는 실제 lifecycle을 수행하는 단계임.
- Orchestrator는 decision을 만들지만 active state를 직접 commit하지 않음.
- Component는 decision을 적용하지만 request 해석과 경쟁 판정의 중심이 되지 않음.


---

## 11. 요약

Execution Orchestration API는 다음 책임 흐름으로 정리함.

```text
Orchestrator
-> Request
-> Resolve Context
-> Resolve Policy
-> Orchestrate
-> Dispatch

Component
-> Start / Interrupt / Cancel / Finish

Execution Object
-> Start / Stop / Finish / Cleanup / Local Policy
```

이 구조를 따르면 API 이름과 책임이 분리되고,  
Reaction Orchestration의 경쟁 상태 평가도 명확한 위치에 둘 수 있음.


---
