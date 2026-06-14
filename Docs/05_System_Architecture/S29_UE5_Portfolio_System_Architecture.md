# 실행 개입 정책 / 게이트 리팩터링

## 1. 목적

본 문서는 현재 `Action / Reaction Orchestration` 구조에서 남아 있는 intervention 판단 구조의 한계를 정리하고, 이후 구현해야 할 개선 방향을 정의한다.

핵심은 다음 세 가지다.

```yaml
Cancel / Interrupt 의미 재정의
Want / Allow 정책과 timing gate 분리
Action / Reaction Orchestrator 공통 알고리즘 분리
```

현재 구조는 `Decision -> Relationship -> ApplyMode -> InterventionDirective -> Component Apply` 흐름을 갖추고 있다.

하지만 intervention 판단에서 `Cancel / Interrupt`, `Want / Allow`, `NotifyWindow`, `Default Policy`의 의미가 아직 완전히 분리되어 있지 않다.

따라서 본 문서는 현재 구조를 유지하면서, 다음 단계에서 어떤 구조로 확장해야 하는지 정리한다.

---

## 2. 관련 브랜치

- `intervention-policy-refactor`

---

## 3. 현재 구조의 형태

현재 orchestration 흐름은 다음 형태에 가깝다.

```yaml
Request
-> Candidate
-> ExecutionContext
-> ExecutionDecisionQuery
-> ExecutionDecisionResult
-> ResolveExecutionApplyMode
-> ResolveInterventionDirective
-> ExecutionResult
-> Component Apply
-> Executor Lifecycle
```

현재 구조에서 `Decision`, `Relationship`, `ApplyMode`는 이미 분리되어 있다.

```yaml
ExecutionDecision
-> Accept / Reject / Ignore

ExecutionRelationship
-> Independent / Sequential / Exclusive

ExecutionApplyMode
-> Start / Reserve / Intervene
```

이 구조 자체는 유지한다.

문제는 `Exclusive` 관계 이후 intervention을 판단하는 과정에서 stop 의미, policy, timing window가 아직 섞여 있다는 점이다.

---
## 4. 현재 구조의 한계

### 1) Cancel / Interrupt 의미가 stop reason에 과하게 묶여 있음

현재 구조에서는 Action 쪽 intervention은 대체로 `Cancelled`, Reaction 쪽 intervention은 대체로 `Interrupted`로 처리된다.

```yaml
Action -> active execution 중단
-> Cancelled

Reaction -> active execution 중단
-> Interrupted
```

하지만 실제로 두 경우 모두 본질은 같다.

```yaml
incoming execution이 active execution에 개입하여 active를 중단한다.
```

차이는 `중단되었다`는 사실이 아니라, 다음 정보에 있다.

```yaml
누가 중단을 요청했는가
어떤 실행이 어떤 실행을 중단하려는가
시스템 강제 중단인가
중단 이후 incoming을 시작할 것인가
```

따라서 `Cancel / Interrupt`를 stop reason의 핵심 분기로 두면 이후 guard, parry, counter, system abort 같은 흐름에서 의미가 계속 충돌한다.

### 2) Notify window가 policy를 대체하고 있음

현재 Want / Allow 판단은 대부분 montage `NotifyState`로 열린 runtime filter에 의존한다.

```yaml
NotifyState Begin
-> Want / Allow filter 등록

Intervention 판단
-> filter match 확인

NotifyState End
-> filter 제거
```

이 구조에서는 다음과 같은 기본 정책도 notify window로 표현해야 한다.

```yaml
AttackAction은 HitReaction에 의해 기본적으로 중단될 수 있다.
HitReaction은 Action을 기본적으로 중단하려고 한다.
DeadReaction은 모든 active execution을 강제로 중단한다.
```

이런 정책은 특정 montage 구간의 timing 문제가 아니라 execution의 기본 정책에 가깝다.

따라서 notify window가 policy 자체를 대체하면, montage 전체 구간에 notify state를 배치하는 방식으로 상시 정책을 흉내 내게 된다.

### 3) Action / Reaction Orchestrator의 알고리즘 중복

현재 `ActionOrchestrator`와 `ReactionOrchestrator`는 도메인은 다르지만 많은 공통 흐름을 반복한다.

```yaml
BuildSnapshot
BuildActiveExecutionParticipant
BuildInterventionQuery
BuildInterventionDirective
ResolveExecutionApplyMode
Independent relationship 검증
Exclusive relationship 검증
Want / Allow 검사
Directive 생성
```

반면 도메인별로 달라야 하는 부분도 있다.

```yaml
Action request 해석
Reaction request 해석
ActionData / ReactionData resolve
ActionExecutor / ReactionExecutor resolve
ActionExecutionResult / ReactionExecutionResult 구성
ActionComponent / ReactionComponent dispatch
RequestResult 변환
```

따라서 전체 orchestrator를 바로 하나의 base class로 합치는 것은 이르다.

우선 공통 알고리즘을 helper 또는 utility로 분리하고, 도메인별 request / context / dispatch는 각 orchestrator에 남기는 방식이 적절하다.

---
## 5. Stop 의미 재정의

`Cancel`과 `Interrupt`를 stop reason의 핵심 분기로 두기보다, active execution이 중단되는 사건을 하나의 intervention으로 보고 source를 분리한다.

권장 구조는 다음과 같다.

```cpp
UENUM(BlueprintType)
enum class EExecutionStopReason : uint8
{
	None = 0,

	Completed,
	Intervened,
	Aborted,

	Max,
};
```

```cpp
UENUM(BlueprintType)
enum class EExecutionStopSource : uint8
{
	None = 0,

	IncomingExecution,
	OwnerExecution,
	System,
	External,

	Max,
};
```

의미는 다음과 같다.

```yaml
Completed
-> montage notify command 또는 정상 흐름에 의한 자연 종료

Intervened
-> 다른 execution 또는 시스템 판단에 의해 active execution이 중단됨

Aborted
-> 실패, 강제 정리, 비정상 복구, fallback cleanup
```

```yaml
IncomingExecution
-> 새로 들어온 execution이 active execution을 중단하려는 경우
-> 예: HitReaction이 AttackAction을 중단

OwnerExecution
-> 같은 owner의 의도적 실행 전환
-> 예: DodgeAction이 active reaction을 중단하고 진입

System
-> death, forced cleanup, state recovery 같은 시스템 중단

External
-> cutscene, debug command, scripted event 같은 외부 제어
```

이렇게 하면 “이건 cancel인가 interrupt인가”가 아니라, “어떤 source가 active execution을 중단했는가”를 기준으로 해석할 수 있다.

---
## 6. Policy와 Gate 분리

Intervention 판단은 두 단계로 나누어야 한다.

```yaml
Policy
-> 무엇을 허용하는가
-> 누구를 중단하고 싶은가
-> 누구에게 중단되어도 되는가

Gate
-> 그 policy가 언제 활성화되는가
-> 항상인가
-> notify window 안에서만인가
-> runtime state 조건인가
```

현재 구조는 `NotifyWindow`가 policy 존재 여부를 대체하는 형태에 가깝다.

개선 후 구조에서는 policy가 먼저 존재하고, notify는 그 policy를 활성화하는 gate 중 하나가 되어야 한다.

```yaml
Policy가 먼저 존재한다.
Gate가 policy의 활성 조건을 결정한다.
NotifyState는 Gate의 한 종류일 뿐이다.
```

---
## 7. Intervention Rule 구조 제안

기본 단위는 `InterventionRule`로 정의한다.

```cpp
UENUM(BlueprintType)
enum class EExecutionInterventionPolicyGate : uint8
{
	None = 0,

	Always,
	NotifyWindow,
	RuntimeState,

	Max,
};
```

```cpp
USTRUCT(BlueprintType)
struct FExecutionInterventionRule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EExecutionInterventionWindowRole WindowRole = EExecutionInterventionWindowRole::Allow;

	UPROPERTY(EditAnywhere)
	EExecutionStopSource StopSource = EExecutionStopSource::IncomingExecution;

	UPROPERTY(EditAnywhere)
	EExecutionInterventionPolicyGate Gate = EExecutionInterventionPolicyGate::Always;

	UPROPERTY(EditAnywhere)
	TArray<FExecutionInterventionParticipantFilter> CounterpartFilters;
};
```

`WindowRole`은 이 rule이 Want인지 Allow인지 결정한다.

```yaml
Want
-> 내가 counterpart를 중단하고 싶은가

Allow
-> 내가 counterpart에 의해 중단되어도 되는가
```

`Gate`는 rule의 활성 조건을 결정한다.

```yaml
Always
-> execution이 active인 동안 항상 적용

NotifyWindow
-> montage notify state로 열린 구간에서만 적용

RuntimeState
-> guard state, super armor, perfect dodge window 같은 runtime state 조건으로 적용
```

---
## 8. Default Policy와 Runtime Window

Policy는 크게 두 종류로 나눈다.

```yaml
Default Policy
-> Data 또는 Executor가 기본으로 가지는 상시 정책

Runtime Window
-> NotifyState로 열고 닫는 일시적 정책
```

예시는 다음과 같다.

```yaml
AttackAction
Default Allow Policy:
-> HitReaction에게 중단될 수 있음

Gate:
-> Always
```

```yaml
ComboAttack
Runtime Allow Policy:
-> DodgeAction에게 중단될 수 있음

Gate:
-> NotifyWindow
```

```yaml
DeadReaction
Default Want Policy:
-> 모든 active execution을 중단하려고 함

Gate:
-> Always
```

이 구조를 사용하면 montage 전체에 notify state를 배치해서 기본 정책을 흉내 낼 필요가 없다.

---
## 9. Executor 판단 흐름

Executor의 Want / Allow 판단은 다음 순서로 정리한다.

```cpp
bool UCAction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return MatchesDefaultWantPolicy(InQuery)
		|| MatchesRuntimeWantWindow(InQuery)
		|| MatchesRuntimeStateWantPolicy(InQuery);
}
```

```cpp
bool UCAction::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return MatchesDefaultAllowPolicy(InQuery)
		|| MatchesRuntimeAllowWindow(InQuery)
		|| MatchesRuntimeStateAllowPolicy(InQuery);
}
```

Reaction도 같은 구조를 사용한다.

```cpp
bool UCReaction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return MatchesDefaultWantPolicy(InQuery)
		|| MatchesRuntimeWantWindow(InQuery)
		|| MatchesRuntimeStateWantPolicy(InQuery);
}
```

```cpp
bool UCReaction::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return MatchesDefaultAllowPolicy(InQuery)
		|| MatchesRuntimeAllowWindow(InQuery)
		|| MatchesRuntimeStateAllowPolicy(InQuery);
}
```

이때 notify state는 runtime window container만 갱신한다.

```yaml
NotifyState Begin
-> Runtime Want / Allow Window 등록

NotifyState End
-> Runtime Want / Allow Window 제거
```

Default policy는 notify state와 무관하게 data 또는 executor가 보유한다.

---
## 10. Orchestrator 공통화 방향

Orchestrator는 policy 자체를 소유하지 않는다.

Orchestrator의 책임은 다음이다.

```yaml
ExecutionDecisionQuery 구성
ExecutionDecisionResult 수신
Relationship에 따라 ApplyMode 결정
Exclusive 관계일 경우 InterventionQuery 구성
Incoming Want 확인
Active Allow 확인
InterventionDirective 구성
ExecutionResult dispatch
```

Policy는 다음 위치에서 제공한다.

```yaml
Data
-> default policy 제공

Executor
-> local runtime state 기반 policy 제공

NotifyState
-> runtime window gate 제공
```

공통화는 다음 순서로 진행한다.

```yaml
1. Action / Reaction Orchestrator를 바로 상속 구조로 합치지 않는다.
2. 중복되는 순수 알고리즘을 utility 함수로 먼저 분리한다.
3. 도메인별 request 해석, context resolve, dispatch는 각 orchestrator에 유지한다.
4. 공통화 범위가 안정되면 base class 또는 template 구조를 검토한다.
```

공통 helper 후보는 다음과 같다.

```yaml
BuildSnapshot
BuildActiveExecutionParticipant
BuildInterventionQuery
BuildInterventionDirective
ResolveIndependentApplyMode
ResolveExclusiveApplyMode
EvaluateInterventionWantAllow
```

도메인별로 유지할 함수는 다음과 같다.

```yaml
ResolveActionContext
ResolveReactionContext
ResolveActionData
ResolveReactionData
ResolveActionExecutor
ResolveReactionExecutor
BuildActionExecutionResult
BuildReactionExecutionResult
DispatchActionDecision
DispatchReactionDecision
ConvertActionDecisionToResultType
ConvertReactionDecisionToResultType
```

---
## 11. Data 구조 보완 방향

현재 Component는 runtime active context와 data map / executor cache를 함께 관리한다.

현재 규모에서는 유지 가능하지만, default policy가 추가되면 data 책임이 더 커진다.

따라서 장기적으로는 다음 구조를 검토한다.

```yaml
Component
-> active execution context 소유
-> execution result 적용
-> executor lifecycle 제어

Data Provider / Registry
-> action / reaction data resolve
-> default intervention policy 제공
-> feedback / damage / execution metadata 제공

Executor Cache
-> executor instance 생성 및 재사용
```

단, 현재 단계에서 바로 분리할 필요는 없다.

우선은 `FActionData`, `FReactionData` 또는 executor default 설정에 default intervention rule을 추가하고, 이후 data provider 분리를 검토한다.

---
## 12. 구현 순서 제안

구현은 다음 순서가 적절하다.

```yaml
1. StopReason / StopSource 의미 정리
2. FExecutionInterventionRule 추가
3. Default Want / Allow policy container 추가
4. Runtime Window container와 Default Policy match 분리
5. NotifyState는 Runtime Window만 열고 닫도록 정리
6. Action / Reaction의 Want / Allow 판단 순서 통일
7. DeadReaction force intervention을 Default Policy 또는 System Source로 재정리
8. Orchestrator 공통 helper 추출
9. RejectReason / log에 StopSource, Gate, Policy match 결과 출력
```

이 순서가 중요한 이유는 다음과 같다.

```yaml
Stop 의미가 정리되어야 directive 의미가 안정된다.
Policy와 Gate가 분리되어야 notify window를 남용하지 않는다.
Want / Allow 판단 순서가 통일되어야 Action / Reaction 대칭성이 유지된다.
공통 helper 추출은 구조가 안정된 이후 진행해야 한다.
```

---
## 13. 적용 예시

### 1) HitReaction이 AttackAction을 중단하는 경우

```yaml
Incoming:
-> HitReaction

Active:
-> ComboAttack

Relationship:
-> Exclusive

StopReason:
-> Intervened

StopSource:
-> IncomingExecution

HitReaction Want:
-> Default Want Policy match

ComboAttack Allow:
-> Default Allow Policy match

ApplyMode:
-> Intervene
```

### 2) DodgeAction이 HitReaction을 중단하는 경우

```yaml
Incoming:
-> DodgeAction

Active:
-> HitReaction

Relationship:
-> Exclusive

StopReason:
-> Intervened

StopSource:
-> OwnerExecution

DodgeAction Want:
-> Default Want Policy 또는 RuntimeState Policy match

HitReaction Allow:
-> Runtime Allow Window 또는 Default Allow Policy match

ApplyMode:
-> Intervene
```

### 3) ComboAttack이 DodgeAction에게 특정 구간에서만 중단을 허용하는 경우

```yaml
Incoming:
-> DodgeAction

Active:
-> ComboAttack

Relationship:
-> Exclusive

ComboAttack Allow:
-> NotifyWindow Gate가 열린 경우에만 match

ApplyMode:
-> Intervene
```

---
## 14. 비목표

본 문서는 다음을 직접 구현 대상으로 삼지 않는다.

```yaml
Combat Resolution 전체 구조
Guard / Parry / Counter 판정
Damage outcome 분배
Action / Reaction data provider 완전 분리
Execution stack 또는 layered execution model
```

위 항목들은 본 문서의 intervention policy / gate 구조가 정리된 이후 별도 단계에서 다룬다.

---
## 15. 결론

현재 orchestration 구조는 `Decision`, `Relationship`, `ApplyMode`, `InterventionDirective`를 분리하는 데까지는 도달했다.

하지만 intervention 판단에서 아직 다음 문제가 남아 있다.

```yaml
Cancel / Interrupt 의미가 stop reason에 과하게 묶여 있음
NotifyWindow가 policy 자체를 대체하고 있음
Action / Reaction Orchestrator에 공통 알고리즘이 중복되어 있음
Default Policy와 Runtime Window가 분리되어 있지 않음
```

따라서 다음 단계의 핵심은 `Policy`와 `Gate`를 분리하는 것이다.

```yaml
Policy
-> 누구를 끊고 싶은가
-> 누구에게 끊겨도 되는가

Gate
-> 그 policy가 언제 활성화되는가
```

이 구조가 정리되면 notify state는 더 이상 정책 자체를 대체하지 않고, runtime timing gate 역할만 담당한다.

그 결과 Action / Reaction intervention 판단은 더 명확해지고, 이후 Guard / Parry / Counter 같은 복합 전투 판정으로 확장하기 쉬워진다.









