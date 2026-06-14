# A10 UE5 Portfolio Execution Arbitration Future Decision

## 1. 목적

본 문서는 action과 reaction이 서로의 active execution에 개입할 수 있는 상황을 처리하기 위한 장기 execution arbitration 방향을 정리하기 위한 문서임.

현재 브랜치에서는 action orchestration refactor를 마무리하는 것이 우선이며, 공통 arbitration 구조를 즉시 구현하지 않음.

본 문서는 dodge, guard, parry, counter, execution 같은 기능을 구현할 때 필요한 판단 구조를 기록하기 위한 설계 메모임.

## 2. 현재 판단

현재 reaction orchestration은 active reaction과 incoming reaction 사이의 경쟁 상태를 일부 판단하고 있음.

핵심 질문은 다음과 같음.

```text
incoming reaction이 active reaction에 개입하려는가
active reaction이 그 개입을 허용하는가
priority와 force 정책이 개입을 허용하는가
최종적으로 start / interrupt / ignore / reject 중 무엇인가
```

Action orchestration은 이번 브랜치에서 request flow를 정리했지만, action/action, action/reaction, reaction/action 사이의 공통 arbitration은 아직 구현하지 않았음.

현재 action flow는 다음 중간 단계에 가까움.

```text
Intent
-> Candidate
-> ExecutionContext
-> LocalDecision
-> OrchestrationResult
-> Component Apply
```

후속 구조에서는 local decision 이전에 현재 실행 환경을 `ExecutionSnapshot`으로 압축하고, local decision 이후에 active/incoming 경쟁 상태를 별도 arbitration으로 판단하는 것이 적합함.

## 3. 문제

장기적으로 action과 reaction은 선형 실행 관계가 아니라 서로 개입 가능한 실행 도메인이 됨.

예시는 다음과 같음.

```text
Action -> Action
- combo chain
- dodge가 attack을 cancel함
- counter가 guard를 cancel함
- equip action은 attack을 interrupt하지 못함

Reaction -> Reaction
- hit reaction 중 dead reaction이 interrupt함
- 낮은 priority hit reaction은 ignore됨

Action -> Reaction
- dodge가 hit reaction을 cancel함
- counter가 stagger reaction을 cancel함
- forced action이 hit reaction을 interrupt함

Reaction -> Action
- hit reaction이 attack action을 interrupt함
- dead reaction이 모든 action을 interrupt함
```

이 관계를 component가 서로 직접 stop 요청을 보내는 방식으로만 처리하면 판단 책임이 분산됨.

필요한 것은 active execution과 incoming execution 사이의 관계를 판단하는 공통 arbitration 구조임.

## 4. 권장 Arbitration 흐름

장기 execution arbitration은 다음 흐름으로 구성하는 것이 적합함.

```text
Incoming request
-> Candidate
-> ExecutionContext
-> ExecutionSnapshot
-> LocalDecision
-> InterventionQuery
-> InterventionAssessment
-> InterventionDirective
-> ComponentCommand
```

각 단계의 책임은 다음과 같음.

```text
ExecutionContext
-> incoming execution의 key, data, executor를 구체화함

ExecutionSnapshot
-> body state, active action/reaction 유무, active context를 압축함

LocalDecision
-> incoming executor가 현재 조건에서 원하는 transition을 제안함

InterventionQuery
-> active와 incoming의 domain, context, intervention intent, snapshot을 담음

InterventionAssessment
-> incoming want rule, active allow rule, priority, force, window 판단 결과를 담음

InterventionDirective
-> 무엇을 멈추고, 왜 멈추고, 멈춘 뒤 무엇을 할지 component가 소비할 수 있게 표현함
```

Local rule과 arbitration은 분리되어야 함.

```text
Incoming executor
-> active에 개입하고 싶은지 자기 입장을 말함

Active executor
-> incoming에 의해 중단되어도 되는지 자기 입장을 말함

Orchestrator
-> 양쪽 응답, priority, force, body state를 종합해 최종 판단함
```

Executor가 최종 판정을 내리면 안 됨. Executor는 자기 실행 규칙만 말하고, 최종 판정은 orchestrator가 내려야 함.

## 5. Decision과 Stop Reason

Gameplay decision과 stop reason은 분리되어야 함.

Gameplay decision의 예시는 다음과 같음.

```text
Start
-> active execution이 없거나 충돌 없이 incoming execution을 시작함

Chain
-> 같은 action 흐름 안에서 다음 실행으로 연결함

Cancel
-> 같은 owner의 의도적 전환 또는 허용된 defensive response로 active execution을 접음

Interrupt
-> 외부 사건 또는 더 높은 우선순위 execution이 active execution을 밀어냄

Ignore
-> request는 유효하지만 현재 상태에서 소비하지 않음

Reject
-> request, context, data, executor, state가 유효하지 않음
```

`Abort`는 gameplay decision보다 system cleanup 또는 fallback reason에 가까움.

따라서 `Abort`는 orchestration decision보다 stop reason 또는 finish reason으로 두는 것이 적합함.

## 6. Interrupt와 Window

Interrupt는 항상 같은 의미가 아님.

보통 다음 두 종류로 나누는 것이 적합함.

```text
Soft Interrupt
-> active가 interruptible window일 때만 허용함
-> hit stagger, weak reaction, 낮은 priority interrupt에 적합함

Force Interrupt
-> active window와 무관하게 강제로 중단함
-> death, execution, grab, guard break, scripted event에 적합함
```

따라서 interruptible window는 필요하지만, 모든 interrupt가 window를 요구하는 것은 아님.

이 차이는 assessment 단계에서 처리하는 것이 적합함.

```text
bIncomingWantsIntervention
bActiveAllowsIntervention
bPriorityAllowed
bForceIntervention
bIgnoreActiveWindow
```

## 7. Snapshot과 Local Rule

기존 `ResolvedPolicy` 같은 decision별 bool 필터는 local decision을 다시 판단하는 구조가 되기 쉬움.

장기적으로 local 이전에는 policy보다 `ExecutionSnapshot`이 더 적합함.

```text
ExecutionSnapshot
-> ExecutionState
-> bIsAlive
-> bHasActiveAction
-> bHasActiveReaction
-> ActiveActionContext
-> ActiveReactionContext
```

Local rule은 snapshot과 incoming context를 보고 자신이 원하는 transition을 제안함.

```text
Action local rule
-> Start / Chain / Cancel / Interrupt / Reject / Ignore 중 하나를 제안함

Reaction local rule
-> Start / Interrupt / Cancel / Reject / Ignore 중 하나를 제안함
```

이후 `Cancel` 또는 `Interrupt`처럼 active execution에 개입하는 decision이 나오면 intervention arbitration으로 넘어감.

## 8. 향후 구조 후보

공통 arbitration을 구현할 때는 다음 구조를 검토할 수 있음.

```cpp
UENUM(BlueprintType)
enum class EExecutionDomain : uint8
{
	None = 0,

	Action,
	Reaction,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionInterventionIntent : uint8
{
	None = 0,

	Cancel,
	Interrupt,

	Max,
};
```

```text
FExecutionSnapshot
FActionExecutionContext
FReactionExecutionContext
FExecutionInterventionQuery
FExecutionInterventionAssessment
FExecutionInterventionDirective
```

`FActionResolvedContext`와 `FReactionContext`는 장기적으로 다음 이름으로 맞추는 것이 적합함.

```text
FActionExecutionContext
FReactionExecutionContext
```

공통 arbitration 계층에는 action/reaction local decision enum을 그대로 넣기보다, `EExecutionInterventionIntent` 같은 공통 intent로 변환해서 전달하는 편이 안정적임.

## 9. 후속 작업 범위

후속 작업에서 구현해야 할 부분은 다음과 같음.

```text
공통 execution arbitration 모델
- incoming execution
- active execution
- domain: action / reaction
- intervention intent: cancel / interrupt
- priority
- active allow rule
- incoming want rule

Action local rule 정리
- WantsToStart()
- WantsToChain()
- WantsToCancelActive()
- WantsToInterruptActive()
- AllowsCancelBy()
- AllowsInterruptionBy()

Reaction local rule 확장
- WantToInterruptActive()
- WantToCancelActive()
- AllowInterventionBy()
```

또는 더 단순하게 공통 query 기반 API를 검토할 수 있음.

```cpp
virtual bool WantToIntervene(const FExecutionInterventionQuery& InQuery) const;
virtual bool AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const;
```

## 10. 방어 액션 구현과 연결

방어 action은 공통 arbitration의 실제 검증 사례가 됨.

```text
Parry
- action으로 선입력함
- hit resolution에서 perfect / general parry를 판정함
- perfect parry는 attacker reaction을 강제할 수 있음

Guard
- action state를 유지함
- incoming damage를 guard response로 변환함
- guard break 시 reaction을 강제할 수 있음

Dodge
- reaction 중 cancel action으로 진입할 수 있음
- active reaction cancel window가 필요함

Counter
- perfect defensive result 이후 현재 execution을 cancel 또는 interrupt하고 실행할 수 있음
```

이 기능들은 action/reaction이 서로 직접 임의로 멈추는 방식보다, combat interaction 또는 hit resolution 결과와 연결하는 것이 적합함.

## 11. 현재 브랜치 범위

현재 브랜치에서는 다음까지만 처리함.

```text
Action executor가 data repository 역할을 하지 않도록 정리함
ActionComponent를 Apply / Request / Try / Internal 구조로 정리함
ActionOrchestrator를 candidate / context / local decision / result 구조로 정리함
Notify를 component gateway 방식으로 정리함
ChainWindow / HitContext / Collision을 NotifyState로 전환함
Reaction stop request는 최소 기능만 구현함
```

현재 브랜치에서는 다음을 구현하지 않음.

```text
공통 FExecutionInterventionQuery 도입
action/action cancel 및 interrupt priority 체계
action/reaction 상호 개입 local rule
parry / guard / dodge / counter policy
combat interaction subsystem 기반 hit resolution arbitration
```
