# S15 UE5 Portfolio Action Orchestration Refactor Retrospective

## 1. 목적

본 문서는 action orchestration refactor 과정에서 실제로 혼란이 컸던 지점과, 그 시행착오를 통해 정리된 구조적 결론을 기록하기 위한 문서임.

이번 브랜치의 어려움은 단순히 API 이름을 바꾸거나 action 실행 코드를 정리하는 데 있지 않았음.

핵심은 action / reaction / component / executor / orchestration 사이에서 `누가 판단하고`, `누가 멈추고`, `누가 후속 실행을 시작하며`, `어떤 계층이 경쟁 상태를 해결해야 하는가`를 정리하는 것이었음.

## 2. 기존 시스템의 형태

기존 action 실행 흐름에서는 action component 내부에 query와 decision 성격의 처리가 많이 남아 있었음.

또한 action type이 다양해질수록 request를 곧바로 실행 API로 연결하기 어렵기 때문에, intent를 기반으로 실행 후보를 구성하는 candidate 단계가 필요해짐.

기존 구조를 단순화하면 다음과 같았음.

```text
Input / AI request
-> ActionComponent 중심 실행 판단
-> CAction decision
-> Start / Chain / Replace 계열 실행
-> Montage / Notify 처리
```

Reaction 쪽에서는 이미 active reaction을 다른 reaction으로 교체하는 흐름이 `Replace`에 가까운 형태로 구현되어 있었음.

하지만 action refactor를 진행하면서 action과 reaction 사이의 개입은 단순 replace로 처리하기 어렵다는 점이 드러남.

특히 action이 active reaction을 멈추고 자신이 실행되는 경우, reaction 내부의 replace가 아니라 action orchestration 결과가 reaction component에 stop 요청을 보내야 함.

## 3. 기존 시스템의 문제 분석 및 한계

첫 번째 혼란은 candidate의 필요성이었음.

Action은 reaction보다 입력 타입과 실행 타입이 다양함.

`ComboAttack`, `Equip`, `Unequip`, `Dodge`, 이후 추가될 `Guard`, `Parry`, `Counter`는 모두 같은 request 흐름으로 들어오더라도 현재 상태에 따라 다른 action data key로 해석될 수 있음.

따라서 action request는 곧바로 executor를 호출하는 구조가 아니라, 먼저 candidate를 구성하고 이를 execution context로 구체화해야 함.

```text
Intent
-> Candidate
-> ActionData
-> ActionExecutor
-> ActionExecutionContext
```

두 번째 혼란은 cancel과 interrupt의 정의였음.

처음에는 의도적인 중단이면 cancel, 외부 요인이면 interrupt라는 기준을 세울 수 있었음.

하지만 실제로는 action끼리, reaction끼리, action과 reaction 사이에서 모두 개입이 발생할 수 있고, 개입 이후의 후속 처리도 서로 다름.

예시는 다음과 같음.

```text
Reaction -> Reaction
-> 기존에는 replace처럼 stop 후 start가 자연스러웠음

Action -> Reaction
-> hit reaction이 active action을 끊고 들어오는 흐름은 interrupt에 가까움

Action -> Reaction stop only
-> 어떤 경우에는 active reaction을 멈추기만 하고 후속 action을 시작하지 않을 수도 있음

Reaction -> Action
-> dodge처럼 active reaction을 cancel하고 action으로 진입하는 흐름이 있음
```

기존 `TryReplace`처럼 stop과 start를 하나의 API에 묶어두면, 모든 중단이 “정지 후 새 실행”이라는 전제를 갖게 됨.

그러나 cross-domain intervention에서는 `정지 후 종료`, `정지 후 시작`, `정지 없이 무시`, `정지 실패 후 reject`가 모두 가능함.

따라서 cancel과 interrupt를 1차원적인 API 증가로 해결하면 불필요하게 많은 API가 생길 위험이 있었음.

세 번째 혼란은 orchestration 내부의 계층 순서였음.

현재 구현은 local decision을 먼저 구하고, 그 decision을 policy가 필터링하는 형태에 가까움.

```text
Local decision
-> Policy filter
-> Orchestration result
```

이 구조는 동작은 가능하지만, local rule이 전체 body state나 active reaction 여부 같은 외부 상태를 알아야 하고, policy도 다시 local decision을 보고 허용/거부를 판단해야 함.

결과적으로 local과 policy의 책임이 겹침.

더 적절한 방향은 `policy`라는 모호한 중간 개념을 줄이고, 현재 실행 환경을 `ExecutionSnapshot`으로 먼저 압축한 뒤 local rule에게 전달하는 구조임.

```text
ResolvedContext
-> ExecutionSnapshot
-> LocalDecision
-> Arbitration
-> Directive / Command
```

`ExecutionSnapshot`은 body state, active action/reaction 유무, active context 유효성, 생존 여부처럼 local rule이 전역 컴포넌트를 직접 조회하지 않도록 필요한 상태를 압축해야 함.

Local rule은 이 snapshot과 incoming execution context를 보고 `Start`, `Chain`, `Cancel`, `Interrupt`, `Reject`, `Ignore` 중 무엇을 원하는지 제안해야 함.

네 번째 혼란은 local decision 이후 경쟁 상태를 어떻게 해결할 것인가였음.

`Cancel`이나 `Interrupt`처럼 active execution에 개입하는 decision이 나오면, 단순히 bool policy로 허용 여부를 정하는 것만으로는 부족함.

이때는 active와 incoming 양쪽의 입장을 모두 확인해야 함.

```text
Incoming executor
-> active execution에 개입하고 싶은가

Active executor
-> incoming execution에 의해 중단되어도 되는가

Orchestrator
-> 양쪽 응답, priority, force, window, body state를 종합하여 최종 판단함
```

따라서 이 단계는 `Policy`보다 `InterventionQuery -> InterventionAssessment -> InterventionDirective` 흐름으로 보는 것이 더 명확함.

```text
InterventionQuery
-> active와 incoming의 domain, context, requested intent, snapshot을 담음

InterventionAssessment
-> incoming want rule, active allow rule, priority, force 판단 결과를 담음

InterventionDirective
-> 무엇을 멈추고, 왜 멈추고, 멈춘 뒤 무엇을 할지 component가 소비할 수 있게 표현함
```

다섯 번째 혼란은 orchestration과 component의 역할 경계였음.

오케스트레이션은 경쟁 상태를 판단하고 지시사항을 만들어야 하지만, 실제 action/reaction 상태를 바꾸고 montage를 멈추거나 시작하는 것은 component와 executor의 책임임.

따라서 orchestration result는 단순 enum이 아니라 component가 소비할 수 있는 명확한 command 또는 directive를 포함해야 함.

```text
Orchestration
-> 누구를 멈출지 결정함
-> 왜 멈추는지 결정함
-> 멈춘 뒤 무엇을 할지 결정함
-> component가 소비할 수 있는 지시사항을 구성함

Component
-> 지시사항을 소비함
-> active action/reaction을 멈춤
-> 필요한 후속 실행을 시작함
-> 상태값을 갱신함
```

여섯 번째 혼란은 이름 체계였음.

`FActionResolvedContext`와 `FReactionContext`는 같은 층위의 개념인데 이름이 다르게 보임.

둘 다 실행 key, 실행 data, 실행 executor를 포함하는 실행 컨텍스트에 가깝기 때문에 장기적으로는 다음처럼 맞추는 것이 더 명확함.

```text
FActionExecutionContext
FReactionExecutionContext
```

마찬가지로 공통 arbitration 계층에는 action local decision enum이나 reaction local decision enum을 그대로 넣기보다, 공통 intervention intent로 변환해서 전달하는 편이 안정적임.

```text
ActionLocalDecision::Cancel
ReactionLocalDecision::Interrupt
-> ExecutionInterventionIntent로 변환함
```

일곱 번째 혼란은 이후 시스템 레벨과의 관계였음.

현재 action/reaction orchestration은 자기 캐릭터의 실행 상태를 중심으로 판단함.

하지만 이후 guard, parry, counter, execution, damage control이 들어오면 상대 캐릭터의 상태도 필요해질 수 있음.

이 경우 orchestration이 직접 상대 객체를 조회하는 구조보다는, 중재자 subsystem이나 interaction subsystem을 통해 필요한 상태 snapshot 또는 판단 결과를 받아 orchestration 판단에 반영하는 방향이 적합함.

## 4. 리팩터링 방향 및 내용

이번 브랜치에서 정리한 1차 방향은 다음과 같음.

```text
ActionOrchestrator
-> intent와 상태를 기반으로 incoming action data와 executor를 구성함
-> 현재 실행 환경을 snapshot으로 압축함
-> local rule과 orchestration result를 통해 실행 결정을 구성함
-> 필요한 경우 reaction stop directive를 생성함

ActionComponent
-> orchestration result를 소비함
-> active action state를 관리함
-> 필요하면 reaction component에 stop을 요청함
-> action start / chain / replace / stop을 실제로 수행함

CAction
-> action 실행 기능을 집약함
-> montage 실행, 종료, notify, feedback을 처리함
-> 실행 중 필요한 data만 캐싱함
-> local rule을 제공함
```

현재 구현에서는 아직 `FActionResolvedPolicy`가 local decision 이후에 존재함.

따라서 현재 구조는 최종형이 아니라 중간 단계임.

다만 이번 브랜치에서 중요한 변화는 action component 내부에 있던 query와 decision 흐름을 orchestration 쪽으로 끌어올렸다는 점임.

또한 action과 reaction 사이의 개입을 단순 replace로 보지 않고, stop directive를 통해 component가 소비할 수 있는 지시사항으로 표현하기 시작했다는 점이 중요함.

현재 `FReactionStopDirective`는 완성형 공통 intervention 구조가 아니라, active reaction을 action orchestration 결과로 멈출 수 있게 하는 최소 skeleton임.

## 5. 이후 작업의 방향성

후속 구조는 다음 순서로 정리하는 것이 적합함.

첫 번째는 `FActionResolvedPolicy`를 `FExecutionSnapshot` 계열로 재정의하는 것임.

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

`ExecutionSnapshot`은 local rule이 외부 컴포넌트를 직접 조회하지 않아도 되도록 현재 실행 환경을 압축해야 함.

예시는 다음과 같음.

```text
ExecutionState
bIsAlive
bHasActiveAction
bHasActiveReaction
ActiveActionContext
ActiveReactionContext
```

두 번째는 intervention arbitration을 `Query / Assessment / Directive`로 나누는 것임.

```text
FExecutionInterventionQuery
-> incoming과 active의 context, domain, intervention intent, snapshot을 담음

FExecutionInterventionAssessment
-> incoming wants, active allows, priority, force, window 판단 결과를 담음

FExecutionInterventionDirective
-> component가 소비할 stop target, stop reason, stop source, after stop action을 담음
```

세 번째는 action과 reaction의 context 이름과 API 계층을 통일하는 것임.

권장 이름은 다음과 같음.

```text
FActionExecutionContext
FReactionExecutionContext

FActionLocalLevelQuery
FReactionLocalLevelQuery

FActionLocalLevelResult
FReactionLocalLevelResult
```

공통 arbitration 계층에서는 도메인별 local decision enum을 직접 들고 있기보다, 공통 intent로 변환해서 사용하는 것이 적합함.

```text
EExecutionDomain
EExecutionInterventionIntent
EExecutionStopReason
EExecutionStopSource
EExecutionAfterStopAction
```

네 번째는 action과 reaction의 component API 계층을 통일하는 것임.

둘 다 다음 역할 구분을 따라야 함.

```text
Apply
-> orchestration 결과를 소비하는 공식 진입점임

Request
-> 다른 도메인 또는 외부 계층의 협조 요청을 받는 진입점임

Try
-> component 내부 조건부 실행 API임

Internal
-> 실제 state mutation과 executor 호출을 수행함
```

다섯 번째는 subsystem과의 관계를 정리하는 것임.

후속 combat 구조에서는 다음 객체들이 필요할 수 있음.

```text
Mediator / Interaction Subsystem
-> 다른 객체의 상태 snapshot 또는 상호작용 판정을 제공함

Control Subsystem
-> 충돌 외의 방식으로 상대 실행을 제어할 수 있게 함

Damage Request Subsystem
-> 충돌 외의 방식으로 damage request를 구성하고 전달할 수 있게 함
```

이 계층들은 action/reaction component의 책임을 대체하는 것이 아니라, orchestration이 자기 객체 밖의 상태를 안전하게 조회하고 판단할 수 있게 하는 보조 계층에 가까움.

## 6. 결론

이번 브랜치에서 가장 큰 시행착오는 action orchestration을 단순 실행 요청 처리기로 볼 것인지, action/reaction execution 사이의 경쟁 상태를 해결할 수 있는 decision pipeline으로 볼 것인지에 대한 판단이었음.

현재 구조는 아직 완성된 orchestration 모델이 아님.

특히 `FActionResolvedPolicy`는 policy라는 이름에 비해 실제로는 local decision을 다시 필터링하는 중간 구조에 가까우며, 장기적으로는 `ExecutionSnapshot`과 `InterventionQuery / Assessment / Directive`로 나누는 것이 더 명확함.

그러나 이번 작업을 통해 다음 기준은 명확해짐.

```text
Orchestrator
-> incoming execution을 구성하고, 실행 환경을 snapshot으로 압축하고, 경쟁 상태 해결 지시사항을 만든다.

Component
-> 지시사항을 소비하고, active state와 side effect를 실제로 변경한다.

Executor
-> 실행 기능을 집약하고, montage / notify / feedback / local rule을 담당한다.

Data Provider
-> 데이터와 executor cache를 제공하되, runtime state 판단을 소유하지 않는다.
```

결과적으로 이번 refactor는 action과 reaction 구조를 통일하기 위한 중간 단계임.

다음 단계에서는 reaction API/flow 정렬, dodge cancel 검증, 공통 intervention directive, 그리고 subsystem 기반 상호작용 판단으로 확장하는 것이 적합함.
