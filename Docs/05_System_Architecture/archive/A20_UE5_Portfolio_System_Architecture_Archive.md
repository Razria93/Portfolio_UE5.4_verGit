# A20 UE5 Portfolio Execution Orchestration Refactor Issue Summary

## 0. 문서 역할

본 문서는 execution orchestration refactor에서 도출된 문제와 리팩터링 방향을 정리하는 formal issue summary 문서임.

같은 주제를 다루는 A18, A19와의 역할 차이는 다음과 같음.

```text
A18
-> 어떻게 여기까지 왔는가
-> 작업 흐름, 전환점, 책임 이동을 기록함

A19
-> 왜 이 리팩터링이 어려웠는가
-> 시행착오와 혼란 지점을 기록함

A20
-> 그래서 아키텍처 이슈와 결정은 무엇인가
-> 정식 issue summary로 사용함
```

따라서 본 문서는 A18, A19의 모든 사고 흐름을 반복하기보다, 최종적으로 남겨야 할 아키텍처 이슈와 리팩터링 방향을 압축해서 정리하는 데 초점을 둠.

같은 주제를 더 자세한 작업 흐름으로 보고 싶다면 A18을, 리팩터링 과정에서 헷갈렸던 지점을 보고 싶다면 A19를 참고함. 본 문서는 이후 GitHub issue나 정식 설계 정리에 사용할 primary summary로 취급함.

## 1. 목적

본 문서는 action / reaction execution orchestration refactor 과정에서 드러난 핵심 이슈를 정리하고, 이후 execution 구체화 작업에서 어떤 기준으로 구조를 재정의해야 하는지 설명하기 위한 문서임.

핵심은 리팩터링 과정에서 반복적으로 충돌했던 개념을 정리하는 것임.

특히 본 문서는 다음 문제를 중심으로 다룸.

```text
책임 분리
Action / Reaction 구조 통일
Cross-domain intervention
Snapshot / Participant 분리
Cancel / Interrupt 의미 정리
Chain과 execution relationship 재정의
```

## 2. 기존 시스템의 형태

### 2.1 Component 중심 실행 흐름

기존 action / reaction 실행 흐름은 component가 많은 책임을 직접 담당하는 구조에 가까웠음.

Component는 active state를 소유하는 것뿐 아니라, request 해석, 실행 판단, executor 호출, notify 전달, stop / finish 처리까지 넓은 범위를 담당했음.

```text
Request
-> Component 판단
-> Executor 실행
-> Notify 수신
-> Component 상태 갱신
```

이 구조는 단순 기능을 빠르게 붙이기에는 편했지만, action과 reaction이 서로 개입하기 시작하면서 책임 경계가 흐려졌음.

### 2.2 Action과 Reaction의 비대칭 구조

Action과 reaction은 모두 execution object를 실행하지만, 기존에는 서로 다른 흐름으로 발전했음.

```text
Action
-> input / AI intent 중심
-> component 내부 판단 비중이 큼
-> combo, equip, unequip 등 action type별 분기가 늘어남

Reaction
-> damage event 중심
-> active reaction과 incoming reaction의 경쟁 판단이 먼저 발전함
-> replace에 가까운 stop-start 흐름이 존재함
```

이 비대칭은 action orchestration을 reaction orchestration 수준으로 고도화하려는 시점에서 큰 난이도가 되었음.

단순히 API 이름만 맞추는 것이 아니라, request 해석부터 component apply, executor lifecycle까지 전체 흐름을 다시 정렬해야 했음.

### 2.3 Chain의 예외적 처리

Combo chain은 기존 구조 안에서 계속 예외처럼 보였음.

입력이 들어온 시점에 즉시 다음 montage를 실행하지 않고, chain window 안에서 다음 데이터를 저장한 뒤 notify timing에서 소비하는 방식이었기 때문임.

```text
Input timing
-> next chain data 저장

Notify timing
-> 저장된 chain data 소비
-> next montage 실행
```

이 구조는 일반 start / stop / interrupt 흐름과 다르게 보였고, orchestration 구조를 설계할 때 계속 혼란을 만들었음.

## 3. 기존 시스템의 문제 분석 및 한계

### 3.1 책임 분리 문제

리팩터링의 가장 큰 출발점은 Orchestrator / Component / Executor / Notify 간 책임을 명확히 분리하는 것이었음.

기존 구조에서는 실행 판단, 상태 변경, notify 처리, executor 호출의 책임이 여러 계층에 걸쳐 섞여 있었음.

```text
Orchestrator
-> 무엇을 판단해야 하는지 불명확함

Component
-> 무엇을 소유하고 무엇을 소비해야 하는지 불명확함

Executor
-> 실제 실행과 local rule만 담당해야 하는지 불명확함

Notify
-> component를 거쳐야 하는지 executor를 직접 호출해야 하는지 불명확함
```

또한 action이나 reaction은 하나의 단일 동작이 아니라 `Start`, `Stop`, `Complete`, `Feedback`, `Chain` 등 여러 상태 조합으로 구성됨.

이 때문에 책임 경계가 흐려질수록 흐름을 추적하기 어려워졌음.

### 3.2 Action / Reaction 구조 통일 문제

Action과 reaction은 서로 다른 도메인이지만, 모두 execution object를 실행한다는 점에서 공통 구조를 가져야 함.

그러나 기존에는 다음이 서로 다르게 구성되어 있었음.

```text
API 구조
Component apply / stop / finish 흐름
Executor lifecycle
Notify command 처리
Intervention 기본 정책
```

이 비대칭은 action과 reaction 사이의 교차 간섭이 들어오면서 더 큰 문제가 되었음.

### 3.3 Cross-domain intervention 문제

기존에는 action은 action끼리, reaction은 reaction끼리만 고려하면 되는 것처럼 보였음.

하지만 실제 전투 흐름에서는 action과 reaction이 서로 교차 간섭할 수 있음.

```text
Action -> Reaction
-> active reaction을 cancel할 수 있음

Reaction -> Action
-> active action을 interrupt할 수 있음

Reaction -> Reaction
-> active reaction을 interrupt할 수 있음

Action -> Action
-> active action을 cancel하거나 sequential chain으로 이어질 수 있음
```

이 흐름을 처리하려면 단순 stop API가 아니라 다음 정보가 명시되어야 함.

```text
누가 개입하는가
누구를 멈추는가
왜 멈추는가
멈춘 뒤 무엇을 할 것인가
```

### 3.4 Decision / Intervention 책임 분리 문제

기존 설계 과정에서는 local decision, policy, orchestration decision의 책임이 중첩되는 문제가 있었음.

이 문제는 단순히 계층 이름이 애매했던 것이 아니라, 이후 `Decision Level`과 `Intervention Level`로 분리해야 하는 핵심 이유였음.

```text
Local
-> 실행 가능 여부뿐 아니라 cancel / interrupt 성격까지 판단하려 함

Policy
-> local decision을 단순 필터링하는 수준에 머묾

Orchestration
-> local decision을 다시 request result로 변환하는 수준에 머묾
```

이 구조에서는 같은 decision이 여러 단계에서 반복되고, policy가 실질적인 판단을 하지 못하는 문제가 발생함.

따라서 policy bool 구조에서 snapshot 중심 구조로 전환하고, 실행 가능성 판단은 Decision Level로, active / incoming 충돌 판단은 Intervention Level로 분리할 필요가 있었음.

### 3.5 Intervention 판단과 실행의 혼재

Intervention은 단순히 `Stop`을 호출하는 문제가 아님.

Intervention에는 판단 메커니즘과 실행 메커니즘이 모두 포함되어 있음.

```text
incoming execution이 active execution을 멈추고 싶은가
active execution이 incoming execution에 의해 멈춰도 되는가
stop reason은 무엇인가
stop source는 무엇인가
target domain은 무엇인가
after-stop action은 무엇인가
```

따라서 intervention 판단은 Orchestrator가 수행하고, 실제 실행은 Component가 directive를 소비하는 구조로 분리해야 함.

### 3.6 Cancel / Interrupt 의미와 Window 책임 혼재

Cancel과 interrupt의 의미도 명확히 정리할 필요가 있었음.

기본 정의는 다음과 같음.

```text
Cancel
-> 의도적 중단

Interrupt
-> 외부 요인 또는 강제 개입에 의한 중단
```

하지만 action과 reaction의 교차 간섭이 생기면 이 정의만으로는 부족함.

```text
Action이 Reaction을 끊는 경우
-> cancel에 가까움

Reaction이 Action을 끊는 경우
-> interrupt에 가까움
```

또한 “내가 끊고 싶은 상태”와 “내가 끊겨도 되는 상태”는 서로 다른 질문임.

따라서 `WantIntervention()`과 `AllowInterventionBy()`를 분리해야 했음.

### 3.7 Snapshot / Participant 책임 중복

Snapshot과 participant 구조체의 책임도 중복될 수 있었음.

Snapshot이 active 상세 데이터를 들고 있고, ActivePart도 active execution 상세 데이터를 들고 있으면 같은 정보를 두 군데에서 관리하게 됨.

정리된 기준은 다음과 같음.

```text
Snapshot
-> body / execution 공통 상태

Participant
-> incoming / active execution의 상세 context
```

또한 active와 incoming이 동시에 존재하기 때문에, 어떤 조건이 incoming에 대한 것인지 active에 대한 것인지 혼동이 생길 수 있었음.

예를 들어 executor가 자기 자신의 incoming type을 확인하는 코드를 “현재 active가 Hit일 때만 interrupt한다”로 잘못 읽을 수 있었음.

### 3.8 DataKey / Wildcard / Validity 문제

Wildcard의 존재 때문에 `IsValid` 정의가 어려웠음.

`All`이나 `INDEX_NONE`은 runtime execution key로는 애매하지만, data map lookup key로는 유효할 수 있음.

```text
DataKey
-> lookup key로서 wildcard를 허용할 수 있음

ExecutionContext
-> 실제 실행 가능한 resolved data여야 함
```

따라서 DataKey와 ExecutionContext의 유효성 기준을 분리해야 함.

### 3.9 Data / Executor cache 위치 문제

Data와 executor cache가 Component 안에 있는 구조도 고민 지점이었음.

Component가 runtime state owner이면서 data registry와 executor cache까지 담당하고 있었기 때문임.

현재는 다음 방향으로 정리됨.

```text
Component
-> data map / executor cache 제공

Orchestrator
-> resolve 흐름 주도
```

장기적으로 data registry를 별도 객체로 분리할 수 있지만, 현재 단계에서는 component가 map/cache를 제공하는 구조를 유지함.

### 3.10 Montage lifecycle 종료 책임 문제

Montage 기반 실행에서는 자연 종료, 외부 중단, 엔진 콜백이 모두 종료 흐름에 관여함.

이 때문에 종료 처리가 중복되거나, 이미 처리된 중단이 다시 종료 콜백에서 처리될 가능성이 있었음.

```text
MontageEnd
-> 엔진 콜백

Complete
-> 자연 종료

Stop
-> 외부 요청에 의한 중단
```

Component는 active state를 정리하고, Executor는 montage lifecycle을 처리해야 함.

Stop 이후 MontageEnd delegate가 다시 호출될 수 있으므로 fallback 처리와 unexpected interruption log의 기준도 필요함.

### 3.11 Chain으로 드러난 실행 방식 분류 문제

Chain은 리팩터링 과정에서 예외처럼 보였지만, 실제 문제는 chain 자체가 아니라 기존 구조가 실행 방식의 차이를 구분하지 못했다는 점이었음.

기존 구조에서는 execution decision이 대체로 “실행 가능한가”에 가까웠고, 실행 가능하다고 판단된 이후의 처리 흐름은 대부분 intervention 흐름으로 이어졌음.

하지만 chain은 active execution을 멈추고 들어가는 흐름이 아니라, 기존 active execution과 연속되는 흐름임.

즉 chain은 competitive execution이 아니라 sequential execution임.

```text
Competitive
-> active와 incoming이 공존할 수 없음
-> intervention 판단 필요

Sequential
-> active와 incoming이 같은 흐름에 속함
-> continuity 검증과 reserve / consume 필요
```

따라서 `Chainable`일 때 intervention directive를 만들지 않고 반환하는 흐름은 예외가 아니라 sequential execution의 결과로 이해해야 함.

### 3.12 Execution decision 개념 재분리 필요성

기존 `EExecutionDecision`은 실행 가능 여부와 실행 방식을 함께 표현하고 있었음.

하지만 실행 판단은 최소한 다음 세 축으로 나누어야 함.

```text
실행 가능 여부
-> Accept / Reject / Ignore

실행 방식
-> Independent / Sequential / Exclusive

실행 결정
-> Start / Reserve / Intervene / StopOnly
```

이 세 가지가 섞이면 `Executable`, `Chainable` 같은 값이 애매해짐.

`Executable`은 실행 가능 여부처럼 보이지만 실제로는 start로 이어질 수도 있고, active가 있으면 intervention이 필요할 수도 있음.

`Chainable`은 실행 가능 여부가 아니라 sequential relationship에 가까움.

### 3.13 실행 방식에 따른 처리 흐름 분할 문제

실행 방식이 다르면 decision 이후 처리 흐름도 달라져야 함.

하지만 기존 구조에서는 실행 가능하다고 판단된 execution이 대부분 같은 apply / intervention 흐름을 따라가려 했음.

이 때문에 chain처럼 active execution과 연속되어야 하는 흐름도 intervention 판단 대상처럼 보였고, 구조적으로 어색해졌음.

```text
Independent
-> active가 없거나 기존 실행과 충돌하지 않을 때 바로 Start

Sequential
-> active와 incoming의 동일성/연속성을 확인한 뒤 Reserve

Exclusive
-> active와 incoming이 공존할 수 없으므로 Intervene 판단

StopOnly
-> 후속 execution 없이 active만 정리
```

실행 방식에 따라 Component가 소비하는 command/directive도 달라져야 함.

### 3.14 Chain의 Reserve / Consume 책임 분리

Chain은 실행 방식으로는 sequential이고, 실제 구현으로는 reserve / consume 흐름을 가짐.

입력 시점과 실행 시점이 다르기 때문에, 이 둘을 분리하지 않으면 흐름을 추적하기 어려워짐.

```text
Input timing
-> next chain data reserve

Notify timing
-> reserved chain data consume
```

인덱스는 consume 시점에 계산되는 것이 아니라 resolve 시점에 결정됨.

Reserve된 data가 consume 시점에 active context로 반영됨.

따라서 `ApplyChain` / `AdvanceCombo`보다 `ReserveChain` / `ConsumeChain`이 실제 책임에 더 가까움.

### 3.15 변칙 조건이 있는 Action / Reaction 확장성

향후 action과 reaction은 단순 실행만으로 설명되지 않음.

특정 조건에서만 실행되거나, 특정 상태에서만 cancel / interrupt가 가능한 경우가 많아질 수 있음.

```text
Dodge
-> reaction 중에만 사용할 수 있을 수 있음

Counter
-> 특정 incoming attack에만 반응할 수 있음

Guard
-> damage reaction을 변환하거나 무시할 수 있음

Parry
-> combat interaction result에 따라 attacker reaction을 유발할 수 있음
```

이런 변칙 조건을 고려하면서 구조를 짜야 했기 때문에 복잡도가 증가했음.

## 4. 리팩터링 방향 및 내용

### 4.1 Decision / Intervention / Component 책임 분리

리팩터링 방향은 다음 계층 분리로 정리됨.

```text
Orchestrator
-> request 해석
-> candidate/context resolve
-> decision/intervention/directive 구성

Component
-> active context 소유
-> execution state 갱신
-> directive 소비

Executor
-> montage lifecycle
-> notify command 처리
-> feedback
-> local rule

Notify
-> 실행 중인 component/executor에 시점 이벤트 전달
```

이 구분을 통해 component가 모든 것을 판단하지 않고, executor가 cross-domain 흐름을 직접 조율하지 않게 됨.

### 4.2 Action / Reaction 공통 execution flow

Action과 reaction은 다음 흐름으로 통일됨.

```text
Request / Event
-> Candidate
-> ExecutionContext
-> ExecutionSnapshot
-> ExecutionDecisionQuery
-> ExecutionDecisionResult
-> InterventionDirective
-> Component Apply
-> Executor Lifecycle
```

이 흐름을 통해 action과 reaction은 서로 다른 도메인이지만 같은 방식으로 조율될 수 있는 기반을 갖게 됨.

### 4.3 Intervention directive 기반 실행 지시

Intervention 판단은 Orchestrator에서 수행하고, 실행은 Component가 수행함.

```text
Orchestrator
-> FExecutionInterventionDirective 생성

Component
-> TargetDomain 확인
-> active action 또는 reaction stop 요청
-> after-stop action에 따라 incoming start / stop only 처리
```

이 분리를 통해 stop target, stop reason, stop source, after-stop action이 명확해짐.

### 4.4 Snapshot / Participant 구조 분리

Snapshot과 participant는 다음 기준으로 분리함.

```text
Snapshot
-> body / execution 공통 상태

Participant
-> incoming / active execution 상세 context
```

Snapshot은 전역 상태를 압축하고, participant는 실제 execution subject의 상세 정보를 담음.

### 4.5 Execution relationship 중심 재정의

향후 execution decision은 단순히 가능한지 여부가 아니라 실행 관계를 중심으로 재정의해야 함.

```text
Independent
-> 독립 실행

Sequential
-> 연속 실행

Exclusive
-> 경쟁 실행
```

이 관계를 기반으로 실제 실행 결정은 다음처럼 분화될 수 있음.

```text
Start
Reserve
Intervene
StopOnly
```

### 4.6 Chain의 Reserve / Consume 구조

Chain은 sequential execution의 대표 사례임.

```text
Resolve
-> incoming index 결정

Reserve
-> incoming data 저장

Consume
-> 저장된 data를 active context로 commit
```

따라서 chain은 intervention 대상이 아니라 reserve / consume 흐름으로 처리되어야 함.

## 5. 이후 작업의 방향성

### 5.1 Execution decision 구조 재정의

현재 `Executable`, `Chainable`은 실행 관계를 충분히 표현하지 못함.

이후 작업에서는 다음 세 축을 분리하는 방향으로 decision/result 구조를 재검토해야 함.

```text
실행 가능 여부
실행 방식
실행 결정
```

### 5.2 Relationship 판단 분기

모든 accepted execution이 intervention으로 들어가면 안 됨.

먼저 active와 incoming의 관계를 분류해야 함.

```text
No active
-> immediate start

Sequential
-> reserve / consume

Competitive / Exclusive
-> intervention query / directive

Invalid
-> reject / ignore
```

### 5.3 DataKey 유효성 기준 분리

Wildcard를 지원하려면 `IsValidMinimal()` 하나로 모든 상황을 처리하기 어려움.

장기적으로는 다음 기준을 분리할 수 있음.

```text
IsValidDataKey
-> data map key로 사용할 수 있는가

IsValidResolvedKey
-> 실제 execution context로 실행 가능한가

IsWildcardKey
-> fallback lookup용 key인가
```

### 5.4 Montage lifecycle 정리

Complete / Stop / MontageEnd의 책임은 계속 주의해야 함.

장기적으로는 다음 원칙이 필요함.

```text
Complete
-> 자연 종료

Stop
-> 외부 요청에 의한 중단

MontageEnd
-> engine callback이며, 이미 처리된 stop의 중복 정리를 막아야 함
```

### 5.5 Combat interaction 확장

향후 guard, parry, counter, execution, damage response를 처리하려면 action / reaction orchestration만으로는 부족할 수 있음.

Combat subsystem 또는 interaction layer는 다음 책임을 가질 수 있음.

```text
interaction snapshot 수집
attacker / defender 상태 조율
damage request 생성
control request 생성
action / reaction orchestration 호출
```

다만 subsystem이 모든 실행을 직접 처리하는 구조가 되어서는 안 됨.

Subsystem은 판정과 조율을 담당하고, action / reaction component는 자기 runtime 실행을 소비하는 방향이 적절함.

## 6. 결론

이번 refactor의 핵심은 action과 reaction을 단순히 비슷한 API 모양으로 맞춘 것이 아님.

더 중요한 변화는 execution을 다음 흐름으로 재정의한 것임.

```text
Intent / Event
-> Candidate
-> ExecutionContext
-> Snapshot
-> Decision
-> Intervention
-> Directive
-> Component Apply
-> Executor Lifecycle
```

이번 작업에서 가장 큰 난이도는 코드량이 아니라 개념이 섞여 있었던 점임.

특히 다음 개념들을 분리하는 것이 핵심이었음.

```text
판단과 실행
Local과 Orchestration
Cancel과 Interrupt
Want와 Allow
Snapshot과 Participant
Replace와 Directive
실행 가능 여부와 실행 방식
Independent / Sequential / Exclusive
Start / Reserve / Intervene / StopOnly
Chain과 Intervention
```

현재 구조는 완성형이라기보다 이후 execution 구체화를 위한 기반임.

다음 단계에서는 action / reaction의 실행을 단순히 가능한지 여부가 아니라, 어떤 방식과 어떤 관계로 실행되는지 기준으로 재정의해야 함.
