# S16 UE5 Portfolio Action Reaction Execution Symmetry Implementation Plan

## 1. 목적

본 문서는 현재 브랜치에서 진행할 action / reaction execution flow 대칭화 작업과 dodge 최소 구현 계획을 정리하기 위한 문서임.

이번 브랜치의 목표는 단순히 action orchestration을 정리하는 것이 아니라, action과 reaction이 같은 실행 언어로 읽히도록 구조를 맞추는 것임.

최소 기능 목표는 dodge action이 active reaction을 cancel하고 action execution으로 진입하는 흐름을 구현하는 것임.

## 2. 현재 시스템의 형태

현재 action 쪽은 request가 candidate, context, local decision, orchestration result를 거쳐 component에 적용되는 구조로 정리되어 있음.

현재 action flow는 다음에 가까움.

```text
Action Request
-> Candidate
-> ActionResolvedContext
-> LocalLevelQuery
-> LocalLevelResult
-> ResolvedPolicy
-> OrchestrationLevelResult
-> ActionComponent::ApplyActionDecision
-> CAction lifecycle
```

Reaction 쪽은 reaction orchestration과 component/executor 구조가 존재하지만, action 쪽에서 정리된 API 계층과 완전히 같은 흐름으로 읽히지는 않음.

또한 현재 action result 안에는 `FReactionStopDirective`가 있으며, action이 active reaction을 멈춰야 하는 최소 연결 구조만 갖추고 있음.

현재 stop 처리 흐름은 다음에 가까움.

```text
ActionComponent::ApplyActionDecision
-> ApplyReactionStopDirective
-> action decision 적용
```

이 구조는 dodge 최소 구현에는 사용할 수 있지만, action/action, reaction/action, reaction/reaction까지 대칭적으로 확장하기에는 부족함.

## 3. 현재 시스템의 문제 분석 및 한계

첫 번째 한계는 stop directive가 reaction 전용이라는 점임.

`FReactionStopDirective`는 active reaction stop만 표현함.

하지만 이후에는 다음 흐름이 모두 필요함.

```text
Action -> Action
-> active action을 cancel하고 incoming action 시작

Action -> Reaction
-> active reaction을 cancel하고 incoming action 시작

Reaction -> Action
-> active action을 interrupt하고 incoming reaction 시작

Reaction -> Reaction
-> active reaction을 interrupt하고 incoming reaction 시작
```

따라서 stop target을 reaction으로 고정하면 구조가 곧 막힘.

두 번째 한계는 `TryReplaceAction`, `TryReplaceReaction` 같은 조합 API가 stop과 start를 암묵적으로 묶는다는 점임.

그러나 cross-domain intervention에서는 다음 경우가 모두 가능함.

```text
stop only
stop then start incoming
ignore without stop
reject when stop fails
```

따라서 replace 계열 API를 늘리는 것보다, orchestration result에 stop directive를 명시하고 component가 이를 먼저 소비한 뒤 incoming execution을 실행하는 구조가 더 적합함.

세 번째 한계는 action과 reaction component의 실행 적용 흐름이 완전히 대칭적이지 않다는 점임.

장기적으로는 두 컴포넌트 모두 다음 흐름을 가져야 함.

```text
ApplyDecision
-> ApplyExecutionInterventionDirective
-> Start / Chain / Enqueue 등 domain-specific execution
```

## 4. 리팩터링 방향 및 내용

이번 작업에서는 `FReactionStopDirective`를 공통 `FExecutionInterventionDirective`로 승격함.

권장 구조는 다음과 같음.

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

각 필드의 의미는 다음과 같음.

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

이번 단계에서 `AfterStopAction`은 구조에는 포함하되, 실제 제어는 decision과 component apply 흐름이 담당해도 됨.

즉 현재 최소 구현에서는 다음 원칙을 따름.

```text
Directive
-> 무엇을 멈출지 표현함

Decision
-> incoming execution을 어떻게 적용할지 표현함
```

예시는 다음과 같음.

```text
active reaction stop
-> directive가 표현함

dodge action start
-> action decision이 표현함
```

ActionComponent와 ReactionComponent는 같은 방식으로 directive를 소비함.

```text
TargetDomain == 자기 domain
-> 직접 StopActive 처리함

TargetDomain != 자기 domain
-> 해당 domain component에 RequestStop으로 위임함
```

ActionComponent 기준 흐름은 다음과 같음.

```text
ApplyActionDecision
-> result validation
-> ApplyExecutionInterventionDirective
-> Start / Chain / Enqueue action
```

ReactionComponent 기준 흐름은 다음과 같음.

```text
ApplyReactionDecision
-> result validation
-> ApplyExecutionInterventionDirective
-> Start reaction
```

중요한 원칙은 stop directive 적용이 실패하면 incoming execution을 시작하지 않는 것임.

```text
directive stop failed
-> incoming start 금지
```

## 5. 작업 순서

### 5.1 공통 실행 중단 구조 추가

먼저 공통 enum과 directive 구조를 추가함.

```text
EExecutionDomain
EExecutionStopReason
EExecutionStopSource
EExecutionAfterStopAction
FExecutionInterventionDirective
```

기존 `FReactionStopDirective`는 제거하거나 공통 directive로 대체함.

### 5.2 Action result 구조 변경

`FActionOrchestrationLevelResult`가 `FExecutionInterventionDirective`를 보유하도록 변경함.

기존:

```text
FReactionStopDirective StopDirective
```

변경:

```text
FExecutionInterventionDirective InterventionDirective
```

`ResolveReactionStopDirective()`는 더 일반적인 이름으로 변경함.

권장 이름은 다음과 같음.

```text
ResolveExecutionInterventionDirective
```

현재 최소 구현에서는 action decision이 `Cancel`이고 active reaction이 존재할 때 다음 directive를 생성함.

```text
TargetDomain = Reaction
StopReason = Cancelled
StopSource = ActionOrchestration
AfterStopAction = StartIncoming
```

### 5.3 ActionComponent directive 소비 구조 변경

`ApplyReactionStopDirective()`를 제거하고 `ApplyExecutionInterventionDirective()`로 변경함.

권장 흐름은 다음과 같음.

```text
ApplyActionDecision
-> ApplyExecutionInterventionDirective
-> decision별 action 실행
```

`Cancel`과 `Interrupt`는 stop 처리가 이미 directive에서 완료되었다는 전제로 incoming action start로 처리함.

```text
Start
-> StartAction

Cancel
-> StartAction

Interrupt
-> StartAction

Chain
-> ChainAction
```

장기적으로 `TryReplaceAction` 계열은 제거할 수 있음.

이번 작업에서도 stop 조합 API가 더 이상 필요 없다면 제거하거나 축소함.

### 5.4 ReactionComponent directive 소비 구조 변경

ReactionComponent에도 ActionComponent와 같은 방식의 directive 소비 함수를 둠.

```text
ApplyExecutionInterventionDirective
RequestStopActiveReaction
StopActiveReaction
```

Reaction orchestration result에도 동일한 directive 필드를 둘 수 있음.

Reaction-vs-Reaction interrupt도 장기적으로는 다음 흐름으로 정리함.

```text
directive로 active reaction stop
-> decision으로 incoming reaction start
```

### 5.5 Action / Reaction component API 단순화

Stop을 `ApplyDecision` 초반에 처리하면 `TryReplace` 계열 API의 의미가 약해짐.

권장 API 방향은 다음과 같음.

```text
ApplyDecision
RequestStopActive
Start
Chain
Enqueue
StopActive
EndActive
```

`Try` 접두사는 component가 판정을 주도하는 것처럼 보이므로 줄이는 방향이 적합함.

단, validation은 제거하지 않음.

```text
Try API 제거
!= runtime guard 제거
```

Component는 여전히 context 유효성, active state, executor 유효성을 방어적으로 확인해야 함.

### 5.6 Dodge 최소 구현

공통 directive 소비 흐름이 정리되면 dodge action을 추가함.

최소 구현 범위는 다음과 같음.

```text
ECombatActionIntent::Dodge
EActionType::Dodge
CAction_Dodge
ActionData 등록
Montage 등록
Notify 연결
```

Dodge local decision은 최소한 다음 기준을 가짐.

```text
Idle 상태
-> Start

Reaction 상태 + active reaction 존재
-> Cancel

그 외
-> Reject 또는 Ignore
```

Dodge cancel 흐름은 다음과 같음.

```text
Dodge request
-> ActionOrchestrator candidate/context resolve
-> Dodge local decision = Cancel
-> intervention directive 생성
-> ActionComponent가 active reaction stop 요청
-> stop 성공 시 dodge action start
```

### 5.7 검증

검증 항목은 다음과 같음.

```text
Idle 상태에서 Dodge 실행됨
HitReaction 중 Dodge 입력 시 active reaction stop 후 Dodge 실행됨
Dead reaction 중 Dodge 입력 시 거부됨
ComboAttack 기존 동작 유지됨
Equip / Unequip 기존 동작 유지됨
Hit / Dead reaction 기존 동작 유지됨
directive stop 실패 시 incoming execution이 시작되지 않음
```

## 6. 결론

이번 작업의 핵심은 action과 reaction의 실행 흐름을 같은 구조로 읽히게 만드는 것임.

이를 위해 `FReactionStopDirective`를 공통 `FExecutionInterventionDirective`로 승격하고, action/reaction component가 같은 방식으로 directive를 먼저 소비한 뒤 incoming execution을 적용하도록 정리함.

이 구조가 되면 `TryReplace` 계열 조합 API에 의존하지 않고, stop target과 follow-up execution을 orchestration result에서 명확히 표현할 수 있음.

이번 브랜치의 완료 기준은 다음과 같음.

```text
공통 intervention directive가 존재함
ActionComponent와 ReactionComponent가 같은 방식으로 directive를 소비함
Dodge가 active reaction을 cancel하고 action으로 진입함
기존 action/reaction 동작이 유지됨
```
