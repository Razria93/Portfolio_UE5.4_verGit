# A13 UE5 Portfolio Execution Intervention Responsibility Decision

## 1. 목적

본 문서는 action / reaction execution 사이에서 cancel, interrupt, stop 같은 개입이 발생할 때 executor, orchestrator, component가 가져야 할 책임 범위를 정리하기 위한 문서임.

특히 무엇을 멈출지, 왜 멈출지, 멈춘 뒤 무엇을 할지를 어느 계층에서 결정해야 하는지 다룸.

## 2. 기존 시스템의 형태

### 2.1 전체 실행 흐름

현재 action / reaction 구조는 request를 orchestration pipeline에서 해석하고, component가 그 결과를 적용하며, executor가 실제 montage lifecycle을 수행하는 흐름을 가짐.

큰 실행 흐름은 다음과 같음.

```text
Request
-> Candidate
-> ExecutionContext
-> Local Level
-> Orchestration Level
-> InterventionDirective
-> Component Apply
-> Executor Lifecycle
```

### 2.2 계층별 기존 책임

Executor는 montage 실행, notify window, feedback, local runtime state를 관리함.

Component는 active execution state를 보관하고 orchestrator가 만든 result를 적용함.

Orchestrator는 request를 해석하고 현재 상태를 기준으로 어떤 실행을 적용할지 결정함.

### 2.3 현재 개입 구조

현재 구조에서는 action과 reaction 모두 active execution을 stop하고 incoming execution을 start할 수 있는 공통 directive 흐름을 갖기 시작함.

이 흐름은 다음 정보로 표현됨.

```text
TargetDomain
-> 멈출 대상 domain을 의미함

StopReason
-> 왜 멈추는지 의미함

StopSource
-> 어느 orchestration 또는 시스템이 stop을 결정했는지 의미함

AfterStopAction
-> stop 이후 incoming execution을 시작할지, stop만 하고 끝낼지 의미함
```

즉 현재 구조는 단순 start 결과뿐만 아니라, stop이 필요한 실행 전환도 result 안에 함께 담을 수 있는 방향으로 정리되고 있음.

## 3. 기존 시스템의 문제 분석 및 한계

### 3.1 Local decision의 의미가 커지는 문제

첫 번째 한계는 local decision이 너무 많은 의미를 가질 수 있다는 점임.

Local level이 `Start`, `Cancel`, `Interrupt` 같은 decision을 직접 반환하면, local executor가 active execution과의 관계까지 판단하는 것처럼 보임.

그러나 cancel과 interrupt는 단순한 실행 가능 여부가 아니라 incoming execution과 active execution 사이의 관계 판단임.

예를 들어 dodge action 자체는 실행 가능한 action일 수 있음.

하지만 현재 active reaction을 cancel하고 들어갈 수 있는지는 다음 조건을 함께 봐야 함.

```text
incoming action이 reaction cancel 능력을 갖는가
active reaction이 cancel을 허용하는가
현재 body state가 action 진입을 허용하는가
stop 이후 incoming action을 시작해도 되는가
stop 실패 시 incoming action을 막아야 하는가
```

이 판단을 executor local decision에 넣으면 executor가 orchestration 책임까지 가지게 됨.

### 3.2 Cancel target과 후속 처리의 불명확성

두 번째 한계는 cancel target과 after-stop behavior가 명확하지 않다는 점임.

cancel 또는 interrupt라는 이름만으로는 다음 정보가 부족함.

```text
무엇을 멈출 것인가
왜 멈추는가
누가 멈추라고 결정했는가
멈춘 뒤 incoming execution을 시작할 것인가
stop only로 끝낼 것인가
```

이 정보가 없으면 component는 실행을 적용하는 과정에서 다시 판단해야 함.

결과적으로 component와 executor가 orchestration 책임을 일부 떠안게 됨.

### 3.3 Window state와 orchestration decision이 섞이는 문제

세 번째 한계는 window state와 orchestration decision이 섞일 수 있다는 점임.

`bCancelable`, `bInterruptible` 같은 상태는 executor의 현재 runtime state임.

이 값은 "내가 지금 개입을 허용하거나 수행할 수 있는가"를 표현할 수는 있지만, "어떤 domain을 멈추고 어떤 후속 실행을 할 것인가"를 결정하는 값은 아님.

따라서 window state는 판단 재료이고, 최종 판결은 orchestration level에서 만들어져야 함.

## 4. 리팩터링 방향 및 내용

### 4.1 책임 분리 원칙

권장 책임 분리는 다음과 같음.

```text
Executor
-> 자기 자신의 실행 가능 조건과 runtime window state를 제공함
-> cancel / interrupt 가능 여부 또는 허용 여부를 local rule로 제공함

Orchestrator
-> incoming execution과 active execution의 관계를 판단함
-> 무엇을 멈출지 결정함
-> stop reason / stop source / after-stop action을 결정함
-> 최종 decision과 directive를 구성함

Component
-> orchestration result와 directive를 소비함
-> 대상 domain component에 stop을 요청함
-> stop 성공 이후 incoming execution을 시작함
-> active state를 갱신함
```

즉 executor는 "능력과 허용 상태"를 말하고, orchestrator는 "판결과 지시사항"을 구성하며, component는 "지시 이행과 상태 갱신"을 담당함.

### 4.2 권장 실행 흐름

권장 흐름은 다음과 같음.

```text
Intent / Damage Event
-> Candidate resolve
-> ExecutionContext resolve
-> Local executable check
-> Active execution snapshot 확인
-> Intervention 필요 여부 판단
-> Incoming & active Executor local rule 조회
-> InterventionDirective 구성
-> Component Apply
-> Executor Start / Stop / Complete
```

Local level은 가능하면 다음 정도만 판단함.

```text
Executable
Chainable
Reject
Ignore
```

`Cancel`과 `Interrupt`는 local decision보다 orchestration intervention 판단에 가까움.

예를 들어 dodge는 local level에서 "실행 가능함"을 반환하고, orchestration level에서 현재 active reaction을 cancel한 뒤 dodge를 시작할지 결정하는 것이 더 적절함.

### 4.3 Intervention directive의 역할

권장 directive 구조는 다음과 같음.

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

중요한 점은 `TargetDomain`과 `AfterStopAction`을 executor가 직접 정하지 않는다는 것임.

Executor는 다음과 같은 local rule만 제공하는 것이 적절함.

```cpp
virtual bool WantToCancel(const FExecutionInterventionQuery& InQuery) const;
virtual bool WantToInterrupt(const FExecutionInterventionQuery& InQuery) const;

virtual bool AllowCancelBy(const FExecutionInterventionQuery& InQuery) const;
virtual bool AllowInterruptionBy(const FExecutionInterventionQuery& InQuery) const;
```

현재 단계에서는 action / reaction별 query hook을 유지해도 됨.

```text
incoming executor
-> active execution에 개입하고 싶은지 판단함

active executor
-> incoming execution에 의해 개입당할 수 있는지 판단함
```

### 4.4 적용 예시

Dodge의 권장 흐름은 다음과 같음.

```text
Dodge input
-> incoming action context resolve
-> local result = Executable
-> active reaction 존재
-> incoming action이 reaction cancel을 원함
-> active reaction이 cancel을 허용함
-> directive.TargetDomain = Reaction
-> directive.StopReason = Cancelled
-> directive.AfterStopAction = StartIncoming
-> result.Decision = Start
```

피격 reaction의 권장 흐름은 다음과 같음.

```text
Damage event
-> incoming reaction context resolve
-> active action 존재
-> incoming reaction이 action interrupt를 원함
-> active action이 interruption을 허용하거나 force rule이 적용됨
-> directive.TargetDomain = Action
-> directive.StopReason = Interrupted
-> directive.AfterStopAction = StartIncoming
-> result.Decision = Start
```

이 구조에서는 `Start` decision이 여러 의미를 가질 수 있음.

```text
Start
-> 그냥 시작함

Start + TargetDomain Action directive
-> active action을 멈춘 뒤 시작함

Start + TargetDomain Reaction directive
-> active reaction을 멈춘 뒤 시작함
```

따라서 "무엇을 멈출지"는 decision enum에 넣지 않고 directive로 표현하는 것이 더 확장성이 높음.

## 5. 이후 작업의 방향성

### 5.1 Local decision 축소

첫 번째 후속 작업은 local decision을 더 얇게 만드는 것임.

현재 action local decision이 `Start`, `Chain`, `Cancel`, `Interrupt` 같은 실행 방식을 직접 반환한다면, 장기적으로는 다음 형태로 정리하는 것이 적절함.

```text
Executable
Chainable
Reject
Ignore
```

### 5.2 공통 intervention query 도입

두 번째 후속 작업은 action / reaction 공통 intervention query를 도입하는 것임.

공통 query는 다음 정보를 포함할 수 있음.

```text
IncomingDomain
ActiveDomain
IncomingActionContext
ActiveActionContext
IncomingReactionContext
ActiveReactionContext
ExecutionState
```

### 5.3 Executor capability 세분화

세 번째 후속 작업은 executor capability를 target별로 세분화하는 것임.

현재는 `bCancelable`, `bInterruptible` 정도로 시작할 수 있지만, 이후 dodge, parry, guard, counter가 들어오면 다음처럼 분리될 수 있음.

```text
CanCancelAction
CanCancelReaction
CanInterruptAction
CanInterruptReaction
```

다만 이번 단계에서 bool을 과도하게 늘리기보다는, 현재 window state를 유지하고 orchestration level에서 target domain을 명시하는 쪽이 적절함.

### 5.4 Combat interaction subsystem과 연결

네 번째 후속 작업은 combat interaction subsystem과의 연결임.

다른 actor의 상태를 조회해야 하는 parry, guard, counter, execution 같은 기능은 단일 actor orchestrator만으로 모든 판단을 하기 어려움.

이 경우 orchestrator는 subsystem에게 상대 actor의 execution snapshot 또는 interaction result를 요청하고, 그 결과를 기반으로 intervention directive를 구성하는 방향이 적절함.

## 6. 결론

Cancel과 interrupt는 executor 내부의 단순 실행 상태가 아니라 incoming execution과 active execution 사이의 관계 판단임.

따라서 executor는 개입 가능 상태와 허용 상태를 제공하고, orchestrator는 무엇을 멈출지와 멈춘 뒤 무엇을 할지 결정해야 함.

Component는 이 결정을 다시 해석하지 않고 directive를 소비하여 stop과 start를 순서대로 이행해야 함.

이 구조를 따르면 dodge, hit reaction, dead reaction, parry, guard, counter처럼 서로 다른 execution domain이 충돌하는 기능을 같은 사고방식으로 확장할 수 있음.

즉 본 문서의 결론은 다음과 같음.

```text
무엇을 멈출지
왜 멈출지
멈춘 뒤 무엇을 할지
```

위 세 가지는 orchestration level에서 결정해야 함.

Executor는 이를 결정하지 않고, 자신이 현재 개입을 수행하거나 허용할 수 있는지만 알려주는 것이 구조적으로 가장 타당함.
