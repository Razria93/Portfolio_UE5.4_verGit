# S14 UE5 Portfolio Action Arbitration Follow-up Decision

## 1. 목적

본 문서는 action orchestration refactor 이후에도 남아 있는 한계와 후속 arbitration 작업 방향을 정리하기 위한 문서임.

이번 브랜치에서는 action execution flow의 골자를 정리했지만, action / reaction 사이의 실제 경쟁 상태 판정은 아직 완성하지 않았음.

따라서 본 문서는 현재 구조가 어디까지 구현되었고, 이후 어떤 방향으로 확장되어야 하는지 기록함.

## 2. 기존 시스템의 형태

이번 refactor 이전에는 action과 reaction이 서로 개입하는 규칙이 명확한 공통 모델로 정리되어 있지 않았음.

Reaction orchestration에서는 active reaction과 incoming reaction 사이의 priority, interruption, executor hook을 판단하는 구조가 먼저 만들어졌음.

반면 action orchestration은 기존 action 실행을 정리하는 단계였고, 다음과 같은 판단은 아직 충분히 일반화되지 않았음.

```text
active action을 incoming action이 cancel할 수 있는가
active reaction을 incoming action이 cancel할 수 있는가
active action을 incoming reaction이 interrupt할 수 있는가
active execution과 incoming execution의 priority는 어떻게 비교하는가
force transition은 어떤 조건에서 허용되는가
```

현재 action orchestration은 local decision과 policy를 통해 request를 실행 가능한 형태로 만드는 데 집중되어 있음.

## 3. 기존 시스템의 문제 분석 및 한계

현재 orchestration level은 아직 얇음.

현재 흐름은 다음에 가까움.

```text
Local Level
-> incoming executor가 원하는 transition을 결정함

Policy Level
-> 해당 transition이 최소 상태 조건을 만족하는지 확인함

Orchestration Level
-> local decision과 policy를 최종 result로 변환하고 필요한 directive를 붙임
```

즉 현재 orchestration level은 active execution과 incoming execution의 경쟁 상태를 완전히 판정하지 않음.

특히 `Cancel`과 `Interrupt`는 단순히 결과 action이 무엇인지로 구분하면 안 됨.

더 적절한 기준은 다음과 같음.

```text
Cancel
-> 같은 owner의 의도적 전환 또는 허용된 defensive response에 의해 자기 active execution을 접는 흐름임

Interrupt
-> 외부 사건 또는 더 높은 우선순위 execution이 active execution을 밀어내는 흐름임
```

예시는 다음과 같음.

```text
Dodge cancels hit reaction
-> 같은 캐릭터가 자기 active reaction을 접고 dodge action으로 진입하므로 Cancel에 가까움

Hit reaction interrupts attack action
-> damage event가 active attack action을 밀어내므로 Interrupt에 가까움

Dead reaction interrupts any action
-> death result가 현재 실행보다 우선하므로 Interrupt에 가까움
```

따라서 action / reaction의 결과 domain만으로 cancel과 interrupt를 결정하면 예외가 많아짐.

## 4. 리팩터링 방향 및 내용

현재 브랜치에서는 공통 arbitration을 바로 구현하지 않고, 최소한의 skeleton만 구성함.

현재 구현된 범위는 다음과 같음.

```text
FActionOrchestrationLevelResult
-> action final decision을 보관함
-> 필요한 경우 FReactionStopDirective를 보관함

ResolveReactionStopDirective
-> action decision이 Cancel이고 active reaction이 있을 때 reaction stop directive를 구성함

ActionComponent::ApplyActionDecision
-> directive가 있으면 ReactionComponent에 active reaction stop을 요청함
-> 이후 action decision을 적용함

ReactionComponent::RequestStopActiveReaction
-> 외부 도메인의 협조 요청으로 active reaction stop을 처리함
```

이 구조는 아직 공통 arbitration 모델이 아니라, dodge 같은 후속 기능을 구현하기 위한 최소 연결 구조임.

장기적으로는 다음 축을 분리해야 함.

```text
Target
-> 무엇을 멈출 것인가

StopReason
-> 왜 멈추는가

StopSource
-> 누가 멈추라고 요청했는가

AfterStopAction
-> 멈춘 뒤 새 execution을 시작할 것인가, 정리만 할 것인가
```

현재 `FReactionStopDirective`는 reaction stop만 표현하므로, 장기적으로는 공통 `FExecutionInterventionDirective` 계열로 승격할 수 있음.

## 5. 이후 작업의 방향성

후속 작업은 세 단계로 나누는 것이 적합함.

첫 번째 단계는 reaction API와 flow를 action refactor 구조에 맞추는 작업임.

```text
ReactionComponent
-> Apply / Request / Try / Internal API 계층 정리함

CReaction
-> Start / Stop / Complete / notify / feedback lifecycle을 action과 유사한 흐름으로 정리함

ReactionOrchestrator
-> 기존 reaction-vs-reaction 판단을 유지하되 이름과 result 구조를 정리함
```

두 번째 단계는 dodge를 기준으로 action-to-reaction cancel flow를 검증하는 작업임.

```text
Input
-> Dodge intent
-> Dodge candidate
-> Dodge local decision = Cancel
-> Policy checks reaction state
-> Orchestration builds reaction stop directive
-> ReactionComponent stops active reaction
-> ActionComponent starts dodge action
```

세 번째 단계는 공통 execution arbitration 모델을 검토하는 작업임.

```text
Incoming execution
-> active execution과 충돌하는가

Active execution
-> incoming intervention을 허용하는가

Incoming executor
-> active execution에 개입하려는가

Policy
-> priority, force, window ignore, body state를 종합함

Result
-> Start / Chain / Cancel / Interrupt / Ignore / Reject를 결정함
```

이 단계는 guard, parry, counter, execution, combat interaction subsystem과 함께 설계하는 것이 적합함.

## 6. 결론

현재 action orchestration refactor는 최종 arbitration 모델을 완성한 작업이 아니라, 그 모델을 올릴 수 있도록 action execution flow를 정리한 작업임.

현재 `FReactionStopDirective`는 임시적인 cross-domain stop skeleton에 가깝지만, dodge와 reaction API 정렬을 진행하기 위한 연결점으로는 충분함.

후속 작업에서는 reaction flow 정렬, dodge cancel 구현, 공통 execution arbitration 모델을 순차적으로 진행하는 것이 적합함.
