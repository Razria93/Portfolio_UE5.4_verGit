# A12 UE5 Portfolio Action Orchestration Transition Arbitration Decision

## 1. 목적

본 문서는 action orchestration refactor 이후 현재 action orchestration 구조가 어디까지 도달했는지, 그리고 다음 단계에서 어떤 방향으로 transition arbitration을 확장해야 하는지 정리하기 위한 문서임.

현재 action orchestration은 request pipeline을 갖추었지만, 아직 완성된 arbitration system은 아님.

따라서 본 문서는 현재 구조를 중간 단계로 정의하고, dodge / cancel / interrupt / execution arbitration / combat subsystem으로 확장하기 위한 기준을 기록함.

## 2. 현재 시스템의 형태

현재 action request는 다음 흐름으로 처리됨.

```text
Request
-> Candidate
-> ResolvedContext
-> LocalDecision
-> ResolvedPolicy
-> OrchestrationResult
-> Component Apply
-> Executor Lifecycle
```

각 단계의 현재 역할은 다음과 같음.

```text
Candidate
-> request와 현재 상태를 기반으로 실행 후보 key를 구성함

ResolvedContext
-> candidate key를 실제 ActionData와 ActionExecutor로 해석함

LocalDecision
-> incoming action executor가 현재 조건에서 원하는 transition을 말함

ResolvedPolicy
-> local decision이 현재 body/runtime state에서 최소 실행 권한을 갖는지 확인함

OrchestrationResult
-> local decision과 policy를 최종 request result로 변환하고 필요한 directive를 붙임

ActionComponent
-> orchestration result를 active action runtime state에 적용함

CAction
-> montage lifecycle, notify command, feedback, local rule을 처리함
```

현재 local decision은 incoming executor의 실행 의도를 표현함.

예시는 다음과 같음.

```text
Idle + Attack
-> Start

Action + ComboAttack + ChainWindow
-> Chain

Reaction + Dodge
-> Cancel
```

현재 `RequestMovementAction()`은 `MovementComponent`를 직접 호출함.

이는 현재 movement request가 대부분 `Move`, `Walk`, `Run`, `Sprint`, `Jump`, `StopJump` 같은 단순 movement command이기 때문임.

반면 dodge / roll / step / dash attack처럼 montage, cancel, invincible window, reaction state와 결합되는 이동은 action execution으로 분류하는 것이 적합함.

## 3. 현재 시스템의 문제 분석 및 한계

현재 action orchestration의 가장 큰 한계는 orchestration level이 아직 경쟁 상태를 충분히 판정하지 않는다는 점임.

현재 orchestration level은 대부분 다음 역할에 머물러 있음.

```text
local decision을 orchestration decision으로 변환함
policy 값이 true인지 확인함
Cancel인 경우 reaction stop directive를 붙임
```

즉 현재 구조는 다음에 가까움.

```text
Local Level
-> incoming execution이 어떤 transition을 원하는지 판단함

Policy Level
-> local decision이 실행 가능한지 단순 필터링함

Orchestration Level
-> local decision과 policy를 request result로 변환하고 stop directive를 붙임
```

이 상태에서는 orchestration이라는 이름에 비해 실제 판정 책임이 약함.

진짜 orchestration이 되려면 active execution과 incoming execution 사이의 관계를 판단해야 함.

필요한 질문은 다음과 같음.

```text
active execution이 존재하는가
incoming execution과 active execution은 같은 domain인가
incoming이 active에 개입할 권한이 있는가
active가 그 개입을 허용하는가
priority 경쟁 결과는 어떤가
force transition인가
active window를 무시할 수 있는가
최종 decision은 무엇인가
intervention directive가 필요한가
```

현재 `Cancel`과 `Interrupt`의 의미도 아직 완전히 분리되지 않았음.

후속 실행이 action인지 reaction인지로 cancel / interrupt를 나누면 예외가 많아짐.

더 적절한 기준은 다음과 같음.

```text
Cancel
-> 같은 owner의 의도적 전환 또는 허용된 defensive response에 의해 자기 active execution을 접는 전환임

Interrupt
-> 외부 사건 또는 더 높은 우선순위 execution에 의해 active execution이 밀려나는 전환임
```

예시는 다음과 같음.

```text
Dodge cancels hit reaction
-> 같은 owner가 자기 active reaction을 접고 dodge action으로 진입하므로 Cancel에 가까움

Hit reaction interrupts attack action
-> damage event가 active attack action을 밀어내므로 Interrupt에 가까움

Dead reaction interrupts dodge action
-> death result가 dodge action보다 우선하므로 Interrupt에 가까움
```

따라서 `Action -> Action`, `Action -> Reaction`, `Reaction -> Action`, `Reaction -> Reaction` 같은 domain 방향만으로 cancel / interrupt를 결정하면 안 됨.

source, ownership, force level, priority, active allow rule을 기준으로 판단해야 함.

## 4. 이후 해결방안 및 예상 리팩터링

다음 단계의 핵심은 local decision과 orchestration arbitration을 분리하는 것임.

다만 현재 논의 기준으로는 `ResolvedPolicy`라는 이름보다 `ExecutionSnapshot`과 `InterventionQuery / Assessment / Directive`로 나누는 편이 더 명확함.

권장 책임 분리는 다음과 같음.

```text
ExecutionSnapshot
-> 현재 body/runtime/active 상태를 압축함

LocalDecision
-> incoming executor가 주어진 snapshot에서 원하는 transition을 제안함

InterventionQuery
-> local decision이 active execution에 개입하는 경우 active/incoming 정보를 묶음

InterventionAssessment
-> incoming want rule, active allow rule, priority, force, window를 평가함

InterventionDirective
-> component가 소비할 stop target, stop reason, stop source, after stop action을 표현함
```

권장 흐름은 다음과 같음.

```text
Request
-> Candidate
-> ExecutionContext
-> ExecutionSnapshot
-> LocalDecision
-> Arbitration
-> Directive / Command
-> Component Apply
```

Orchestration arbitration은 다음 순서로 확장하는 것이 적합함.

```text
1. active execution 존재 여부를 확인함
2. active가 없으면 Start / Handle 계열을 승인함
3. active가 있으면 local transition type을 확인함
4. Chain이면 같은 action flow와 chain rule을 확인함
5. Cancel이면 self-transition 권한과 active allow rule을 확인함
6. Interrupt이면 external / force 권한과 active allow rule을 확인함
7. priority / force / window / active allow / incoming want를 확인함
8. final decision과 intervention directive를 발행함
```

Dodge는 다음 브랜치에서 가장 먼저 검증할 수 있는 사례임.

Dodge는 단순 movement command가 아니라 action execution으로 구성하는 것이 적합함.

초기 `CAction_Dodge`의 local decision은 다음 정도로 시작할 수 있음.

```cpp
EActionLocalLevelDecision UCAction_Dodge::ResolveLocalLevelDecision(
	const FActionLocalLevelQuery& InQuery
) const
{
	const FExecutionSnapshot& snapshot = InQuery.Snapshot;

	if (!snapshot.bIsAlive)
	{
		return EActionLocalLevelDecision::Reject;
	}

	if (snapshot.ExecutionState == EExecutionState::Idle
		&& !snapshot.bHasActiveAction
		&& !snapshot.bHasActiveReaction)
	{
		return EActionLocalLevelDecision::Start;
	}

	if (snapshot.ExecutionState == EExecutionState::Reaction
		&& snapshot.bHasActiveReaction)
	{
		return EActionLocalLevelDecision::Cancel;
	}

	return EActionLocalLevelDecision::Reject;
}
```

이후 orchestration level에서 active reaction이 dodge cancel을 허용하는지 판단해야 함.

판단 요소는 다음과 같음.

```text
active reaction이 cancel 가능한 상태인가
incoming dodge가 cancel intent를 가졌는가
cancel window가 열려 있는가
priority 또는 force policy가 충분한가
```

Stop directive는 장기적으로 공통 intervention directive로 확장하는 것이 적합함.

예상 구조는 다음과 같음.

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

이 구조는 다음 축을 분리함.

```text
TargetDomain
-> 무엇을 멈출 것인지 표현함

StopReason
-> 왜 멈추는지 표현함

StopSource
-> 어디서 중단 요청이 발생했는지 표현함

AfterStopAction
-> 중단 후 무엇을 할지 표현함
```

장기적으로 `ActionComponent::ApplyActionDecision()`은 orchestration result를 직접 받기보다, component가 소비할 수 있는 execution command를 받는 형태가 더 적합할 수 있음.

예상 방향은 다음과 같음.

```text
Orchestrator
-> final decision과 directive를 포함한 command를 생성함

Component
-> command를 소비하여 stop / start / chain / end를 적용함

Executor
-> montage lifecycle과 notify command를 처리함
```

이 구조가 되면 action orchestrator, reaction orchestrator, combat subsystem이 동일한 command 적용 경로를 공유할 수 있음.

## 5. 작업 순서

현재 브랜치에서는 다음 상태로 종료하는 것이 적합함.

```text
기존 action 실행 유지
local query 구조 정리
active action state와 active context 분리
interrupt 준비 흐름 복구
reaction stop directive skeleton 유지
```

다음 브랜치에서는 다음 순서를 권장함.

```text
1. CAction_Dodge를 추가함
2. Dodge local decision에서 Reaction 상태일 때 Cancel을 반환함
3. reaction cancel 가능 조건을 orchestration level에서 판단함
4. ReactionStopDirective 이름 정리 또는 공통 directive 설계를 시작함
5. ActionExecutionCommand 또는 ExecutionCommand 구조를 검토함
6. Action local rule의 cancel / interrupt hook을 추가함
7. Orchestration level에서 active / incoming arbitration을 구현함
8. 이후 CombatSubsystem 연결을 검토함
```

CombatSubsystem은 실행자가 아니라 판정과 조율을 담당하는 상위 객체로 보는 것이 적합함.

실제 runtime mutation은 각 domain component가 수행해야 함.

```text
ActionComponent
-> action runtime 적용

ReactionComponent
-> reaction runtime 적용

OutgoingDamageComponent
-> outgoing damage payload 구성

IncomingDamageComponent
-> incoming damage 적용
```

CombatSubsystem이 직접 montage를 재생하거나 state를 변경하면 orchestrator / component / executor의 책임이 무너짐.

## 6. 결론

현재 action orchestration은 완성된 arbitration system이 아니라 구조를 세우는 중간 단계임.

현재 local level은 incoming execution이 어떤 transition을 원하는지 말하는 계층임.

현재 policy level은 local decision이 최소 실행 권한을 가지는지 필터링하는 계층에 가까움.

후속 구조에서는 policy라는 용어를 줄이고, `ExecutionSnapshot`과 `InterventionQuery / Assessment / Directive`로 책임을 나누는 것이 더 적합함.

Cancel과 Interrupt는 후속 domain으로 구분하지 않고 source, ownership, force level, priority, active allow rule을 기준으로 구분해야 함.

Dodge는 reaction cancel을 실제로 검증할 첫 action 사례로 적합함.
