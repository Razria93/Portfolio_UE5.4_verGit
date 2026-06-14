# A11 UE5 Portfolio Execution Stop Directive Decision

## 1. 목적

본 문서는 action / reaction 실행 구조에서 active execution을 중단해야 하는 상황을 어떻게 표현할지 정리하기 위한 문서임.

Action orchestration refactor 과정에서 dodge 같은 action이 active reaction을 중단하고 action으로 진입해야 하는 요구가 발생함.

이때 단순히 `Cancel` 또는 `Interrupt` decision만으로는 다음 정보를 충분히 표현하기 어려움.

```text
무엇을 중단하는가
왜 중단하는가
중단 후 무엇을 하는가
누가 중단을 요청했는가
```

따라서 본 문서는 gameplay decision과 stop directive를 분리하는 방향을 기록함.

## 2. 현재 상태

현재 action orchestration은 incoming action request를 해석한 뒤 최종 action decision을 구성함.

현재 action decision은 대략 다음 의미를 가짐.

```text
Start
-> active action이 없을 때 incoming action을 시작함

Chain
-> active action 흐름 안에서 다음 action index로 연결함

Interrupt
-> active action을 중단하고 incoming action으로 교체하는 흐름을 예약한 decision임

Cancel
-> active reaction을 의도적으로 중단하고 incoming action을 시작하는 최소 cross-domain cancel로 사용함
```

현재 브랜치에서 `Cancel`은 전체 cancel 개념을 완성한 것이 아니라, reaction 중 dodge action으로 진입하기 위한 최소 구조임.

`FReactionStopDirective`는 active reaction stop만 표현함.

```text
StopReason
-> EReactionStopReason::Cancelled

StopSource
-> EReactionStopSource::ActionOrchestration
```

중단 이후 incoming action을 시작하는지는 stop directive가 아니라 action orchestration decision과 `ActionComponent::ApplyActionDecision()` 흐름으로 표현함.

## 3. 문제 분석

실행 중단은 단일 의미가 아님.

예시는 다음과 같음.

```text
Dodge during hit reaction
-> active reaction을 cancel하고 dodge action을 시작함

Dead reaction during attack
-> active action을 interrupt하고 dead reaction을 시작함

External cleanup
-> active execution을 abort하고 후속 실행 없이 정리함

Guard success
-> active action/reaction을 유지하고 damage response만 조정함
```

이 사례들은 모두 “무언가를 멈춘다”는 표현으로 묶을 수 있지만, 실제로는 서로 다른 축을 가짐.

```text
중단 대상
-> Action / Reaction / None

중단 이유
-> Cancelled / Interrupted / Ignored / Aborted

중단 요청 출처
-> ActionOrchestration / ReactionOrchestration / CombatInteraction / System

중단 후 동작
-> StopOnly / StartIncoming / ResumePrevious
```

이 축들을 하나의 enum decision에 모두 넣으면 decision 이름이 비대해지고 component가 어떤 책임을 수행해야 하는지 불명확해짐.

따라서 gameplay decision과 stop directive를 분리하는 것이 적합함.

## 4. 결정 사항

현재 브랜치에서는 공통 execution directive를 즉시 구현하지 않음.

현재 범위에서는 action orchestration이 active reaction을 중단해야 하는 경우에만 `FReactionStopDirective`를 생성하고, `ActionComponent`가 이를 먼저 적용한 뒤 action decision을 소비함.

현재 최소 흐름은 다음과 같음.

```text
ActionOrchestrator
-> incoming action request를 해석함
-> local decision을 얻음
-> orchestration decision을 얻음
-> 필요한 경우 FReactionStopDirective를 생성함

ActionComponent
-> stop directive가 있으면 active reaction 중단을 먼저 요청함
-> 이후 action decision에 따라 incoming action을 실행함

ReactionComponent
-> FReactionStopDirective를 소비하여 active reaction을 stop/end 처리함
```

현재 `FReactionStopDirective`는 “reaction을 왜 중단해야 하는가”만 표현함.

후속 action 시작 여부는 action decision이 표현함.

## 5. 권장 구조

장기적으로는 action / reaction 중단을 공통 모델로 표현하기 위해 다음 축을 분리하는 것이 적합함.

### 5.1 중단 대상

```cpp
UENUM(BlueprintType)
enum class EExecutionDomain : uint8
{
	None = 0,

	Action,
	Reaction,

	Max,
};
```

중단 대상은 어떤 domain의 active execution을 정리할 것인지 표현함.

```text
Action
-> active action을 중단함

Reaction
-> active reaction을 중단함
```

### 5.2 중단 이유

```cpp
UENUM(BlueprintType)
enum class EExecutionStopReason : uint8
{
	None = 0,

	Cancelled,
	Interrupted,
	Ignored,
	Aborted,

	Max,
};
```

중단 이유는 active execution이 왜 종료되는지 표현함.

```text
Cancelled
-> 같은 owner의 의도적 취소나 허용된 defensive response에 의한 중단임

Interrupted
-> 외부 사건 또는 더 높은 우선순위 execution에 의한 중단임

Ignored
-> 실행이 소비되었지만 유효한 후속 실행 없이 무시됨

Aborted
-> 비정상 상태 또는 system fallback에 의한 정리임
```

`Aborted`는 gameplay decision보다 cleanup / fallback reason으로 보는 것이 적합함.

### 5.3 중단 요청 출처

```cpp
UENUM(BlueprintType)
enum class EExecutionStopSource : uint8
{
	None = 0,

	ActionOrchestration,
	ReactionOrchestration,
	CombatInteraction,
	System,

	Max,
};
```

중단 요청 출처는 어떤 계층에서 stop 요청이 발생했는지 표현함.

이 값은 디버깅과 책임 추적에 중요함.

### 5.4 중단 후 동작

```cpp
UENUM(BlueprintType)
enum class EExecutionAfterStopAction : uint8
{
	None = 0,

	StopOnly,
	StartIncoming,
	ResumePrevious,

	Max,
};
```

중단 후 동작은 active execution을 멈춘 뒤 무엇을 할지 표현함.

```text
StopOnly
-> active execution만 정리하고 후속 실행을 시작하지 않음

StartIncoming
-> active execution을 정리한 뒤 incoming execution을 시작함

ResumePrevious
-> stack 또는 queue 기반 구조가 생겼을 때 이전 실행으로 복귀함
```

현재 브랜치에서는 `AfterStopAction`을 별도 enum으로 구현하지 않고, action decision과 component apply 흐름으로 표현함.

## 6. 권장 Directive 모델

장기적으로는 다음 공통 directive 모델을 검토할 수 있음.

```cpp
USTRUCT(BlueprintType)
struct FExecutionInterventionDirective
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bRequested = false;

	UPROPERTY(Transient)
	EExecutionDomain TargetDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	EExecutionStopReason StopReason = EExecutionStopReason::None;

	UPROPERTY(Transient)
	EExecutionStopSource StopSource = EExecutionStopSource::None;

	UPROPERTY(Transient)
	EExecutionAfterStopAction AfterStopAction = EExecutionAfterStopAction::None;
};
```

이 구조는 다음 관계를 명확하게 분리함.

```text
TargetDomain
-> 무엇을 멈출 것인지 표현함

StopReason
-> 왜 멈추는지 표현함

StopSource
-> 어디서 중단 요청이 발생했는지 표현함

AfterStopAction
-> 멈춘 뒤 무엇을 할지 표현함
```

다만 현재 코드에는 이미 `EActionStopReason`, `EReactionStopReason`, `FReactionStopDirective`가 존재함.

현재 브랜치에서 이를 즉시 공통화하면 action orchestration refactor 범위가 과도하게 커짐.

따라서 현재는 `FReactionStopDirective`를 유지하고, action/reaction cross-domain arbitration이 본격화될 때 공통 directive로 승격하는 것이 적합함.

## 7. 현재 브랜치 적용 기준

현재 action orchestration refactor에서는 다음 기준을 적용함.

```text
Reaction 중 Dodge action request
-> local decision은 Cancel을 반환함
-> orchestration decision은 Cancel로 결정됨
-> FReactionStopDirective가 생성됨
-> ActionComponent가 active reaction 중단을 먼저 요청함
-> ActionComponent가 incoming dodge action을 시작함
```

현재 `FReactionStopDirective`가 표현하는 정보는 다음으로 제한됨.

```text
StopReason
-> EReactionStopReason::Cancelled

StopSource
-> EReactionStopSource::ActionOrchestration
```

중단 후 action 시작 여부는 directive가 아니라 action orchestration decision이 표현함.

따라서 현재 `ActionComponent::ApplyActionDecision()`에서 `Cancel`은 active action replace가 아니라 incoming action start로 처리하는 것이 적합함.

```cpp
case EActionOrchestrationLevelDecision::Cancel:
	return TryStartAction(InActionOrchestrationResult.ResolvedContext);
```

## 8. 향후 확장 기준

다음 요구가 추가되면 공통 execution directive 모델을 검토해야 함.

```text
action이 active action을 cancel하고 다른 action으로 전환함
reaction이 active action을 interrupt함
dead reaction이 모든 execution을 강제 중단함
guard break가 active action 또는 reaction을 강제 중단함
counter attack이 current execution을 정리하고 forced action으로 진입함
execution stack 또는 queue가 생겨 stop 이후 resume이 필요함
```

이 단계에서는 `FReactionStopDirective`만으로는 부족함.

`FExecutionInterventionDirective` 또는 이에 준하는 공통 구조를 도입하고, action / reaction component가 각 domain별 directive를 소비하는 구조로 확장하는 것이 적합함.
