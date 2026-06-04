# A18 UE5 Portfolio Execution Model Refactor History

## 0. 문서 역할

본 문서는 execution refactor의 작업 흐름과 구조적 전환점을 기록하는 history 문서임.

같은 주제를 다루는 A19, A20과의 역할 차이는 다음과 같음.

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

따라서 본 문서는 세부 이슈를 모두 정식 결정으로 정리하기보다, refactor 과정에서 사고방식과 책임 구조가 어떻게 이동했는지를 보존하는 데 초점을 둠.

본 문서에서 반복적으로 등장하는 문제들은 A20처럼 정식 issue 목록으로 읽기보다, 각 전환점이 왜 발생했는지를 설명하는 history context로 읽는 것이 적절함.

## 1. 목적

본 문서는 action / reaction orchestration refactor 과정에서 어떤 구조적 고민이 있었고, 그 고민을 거치며 execution 모델이 어떻게 수정되었는지 기록하기 위한 문서임.

핵심은 단순히 어떤 API가 바뀌었는지를 나열하는 것이 아님.

이번 작업에서 중요했던 전환점은 다음과 같음.

```text
ActionComponent 중심 실행 판단
-> Orchestrator 중심 request 해석
-> Component 중심 runtime 상태 소비
-> Executor 중심 montage lifecycle
-> Action / Reaction 공통 Execution 모델
```

즉 본 문서는 현재 구조가 한 번에 정해진 것이 아니라, action / reaction / component / executor / orchestration 사이의 책임을 계속 재분류하면서 형성되었다는 점을 남기기 위한 문서임.

## 2. 기존 시스템의 형태

### 2.1 Action 중심 구조

기존 action 실행 흐름은 action component 내부에 많은 판단이 집중되어 있었음.

입력이나 AI 요청이 들어오면 component가 현재 상태를 보고 action type을 결정하고, action data와 executor를 찾아 직접 실행 흐름으로 연결하는 형태에 가까웠음.

단순화하면 다음과 같음.

```text
Input / AI request
-> ActionComponent
-> action type 판단
-> action data / executor resolve
-> executor start / chain / stop
```

이 구조는 처음에는 단순했지만, action 종류가 늘어나면서 한계가 드러남.

`ComboAttack`, `Equip`, `Unequip`, 이후 추가될 `Dodge`, `Guard`, `Parry`, `Counter`는 모두 action이지만 실행 조건과 active execution과의 관계가 서로 다름.

따라서 action request를 곧바로 실행 API로 연결하는 구조만으로는 확장성이 부족했음.

### 2.2 Reaction 중심 구조

Reaction은 action보다 먼저 orchestration 성격이 강한 구조로 발전했음.

Damage event 이후 reaction type을 결정하고, active reaction과 incoming reaction 사이의 관계를 보고 start / interrupt / ignore를 판단하는 흐름이 이미 존재했음.

다만 초기 reaction 구조는 `Replace`에 가까운 형태로 이해되기 쉬웠음.

```text
incoming reaction
-> active reaction이 있으면 멈춤
-> incoming reaction 시작
```

이 방식은 reaction끼리만 볼 때는 자연스러웠지만, action과 reaction이 서로 개입하기 시작하면 부족했음.

예를 들어 action이 active reaction을 cancel하고 들어가야 하는 경우는 reaction replace가 아니라 action orchestration 결과가 reaction component에 stop 요청을 보내는 흐름이어야 함.

## 3. 전환을 만든 구조적 압력

### 3.1 Action request 해석이 Candidate 단계로 분리됨

Action은 reaction보다 request 해석이 복잡함.

Reaction은 damage result를 기반으로 `Hit` 또는 `Dead`처럼 비교적 명확한 candidate를 만들 수 있음.

반면 action은 입력 intent와 현재 상태가 결합되어야 실행 후보가 결정됨.

예시는 다음과 같음.

```text
좌클릭
-> idle 상태라면 ComboAttack 0
-> ComboAttack 0 진행 중 chain window라면 ComboAttack 1
-> 특정 상태라면 Skill 또는 Counter
```

따라서 action은 request를 바로 executor 실행으로 연결하지 않고, 먼저 candidate를 구성한 뒤 resolved context로 확정하는 단계가 필요했음.

```text
Intent
-> Candidate
-> ActionData
-> ActionExecutor
-> ActionExecutionContext
```

이 전환을 통해 action component 내부에 있던 query / decision 성격의 판단이 orchestrator로 이동하게 되었음.

### 3.2 Cancel / Interrupt가 Stop Directive 사고로 이동함

가장 큰 혼란은 cancel과 interrupt의 정의였음.

초기에는 다음처럼 단순하게 나눌 수 있다고 보았음.

```text
Cancel
-> 의도적 중단

Interrupt
-> 외부 요인에 의한 강제 중단
```

이 정의 자체는 여전히 유효함.

하지만 실제 구조에서는 action과 reaction이 서로 다른 방향으로 개입할 수 있었음.

```text
Reaction -> Action
-> HitReaction이 AttackAction을 interrupt함

Action -> Reaction
-> DodgeAction이 HitReaction을 cancel함

Reaction -> Reaction
-> DeadReaction이 HitReaction을 interrupt함

Action -> Action
-> 특정 action이 active action을 cancel하고 진입할 수 있음
```

따라서 cancel / interrupt를 단순히 stop API 이름으로만 늘려가는 방식은 적절하지 않았음.

`TryInterruptAndStart`, `TryCancelAndStart`, `TryInterruptAndEnd`, `TryCancelAndEnd`처럼 API를 1차원적으로 늘리면 조합 폭발이 발생함.

중요한 것은 API 이름을 늘리는 것이 아니라 다음 정보를 명확히 분리하는 것이었음.

```text
누가 개입하는가
누구를 멈추는가
왜 멈추는가
멈춘 뒤 무엇을 할 것인가
```

이 고민이 `FExecutionInterventionDirective` 계열 구조로 이어졌음.

### 3.3 Replace 사고가 Stop / Start 분리로 이동함

기존 replace 개념은 stop 후 start가 자연스럽게 이어지는 구조였음.

하지만 모든 개입이 stop 후 start는 아님.

```text
Reaction -> Reaction
-> active reaction stop
-> incoming reaction start

Action -> Reaction cancel
-> active reaction stop
-> incoming action start

Action -> Reaction stop only
-> active reaction stop
-> incoming action이 없을 수도 있음

System cleanup
-> active execution stop
-> 후속 실행 없음
```

따라서 stop과 start를 하나의 TryReplace 계열 API로 묶는 구조는 장기적으로 적합하지 않음.

현재 방향은 orchestration이 stop directive와 incoming execution을 분리해서 만들고, component가 그 지시사항을 소비하는 방식임.

### 3.4 Local / Policy / Orchestration 용어가 Snapshot / Decision / Intervention으로 이동함

작업 중간에는 local level, policy level, orchestration level로 나누려 했음.

하지만 이 구분은 점점 애매해졌음.

초기 형태는 다음에 가까웠음.

```text
Local Level
-> executor가 실행 유형을 판단함

Policy Level
-> body/action/reaction 상태를 보고 실행 가능 여부를 필터링함

Orchestration Level
-> local decision을 request result로 변환하고 stop directive를 붙임
```

문제는 policy가 local decision 이후에 오면, executor도 외부 상태를 알아야 하고 policy도 local decision을 다시 해석해야 한다는 점이었음.

즉 둘이 서로의 책임을 침범하기 쉬웠음.

이후 방향은 더 명확한 용어로 재정리되었음.

```text
Snapshot
-> 현재 body / execution 상태를 압축함

Decision
-> incoming execution이 어떤 실행 관계를 원하는지 판단함

Intervention
-> active execution과 incoming execution의 충돌을 해결함

Directive
-> component가 소비할 실행 지시사항을 표현함
```

## 4. 책임 이동 흐름

### 4.1 Orchestrator의 책임

Orchestrator는 executor를 직접 실행하는 객체가 아님.

Orchestrator의 책임은 request를 해석하고, 실행 가능한 context를 구성한 뒤, active execution과 incoming execution의 관계를 판단하는 것임.

현재 방향은 다음과 같음.

```text
Request
-> Candidate resolve
-> ExecutionContext resolve
-> ExecutionSnapshot build
-> ExecutionDecisionQuery build
-> Executor decision query
-> Intervention 판단
-> ExecutionResult / Directive build
-> Component apply
```

즉 orchestrator는 “무엇을 실행할지”와 “무엇을 멈춰야 하는지”를 결정하지만, 실제 runtime 상태 변경은 component가 수행함.

### 4.2 Component의 책임

Component는 orchestration result를 소비하는 runtime owner임.

ActionComponent와 ReactionComponent는 다음 책임을 가짐.

```text
active context 저장
execution state 진입/종료
intervention directive 소비
active executor stop
incoming executor start / chain / complete 반영
notify를 active executor로 전달
```

따라서 component는 판단의 주체가 아니라 실행 상태의 소유자임.

이 구분을 통해 action / reaction 양쪽에서 다음 흐름을 맞출 수 있었음.

```text
Orchestrator
-> decision/result 구성

Component
-> result/directive 소비

Executor
-> montage lifecycle 수행
```

### 4.3 Executor의 책임

Executor는 action 또는 reaction의 실제 실행 객체임.

Executor가 맡아야 하는 것은 다음과 같음.

```text
montage play / stop / complete
notify command 처리
feedback request
runtime window 상태 관리
자신의 local execution decision
intervention want / allow rule
```

반대로 executor가 맡지 않아야 하는 것은 다음과 같음.

```text
전체 request 해석
다른 component의 active 상태 직접 조율
cross-domain stop/start 순서 결정
전역 경쟁 상태 해결
```

이 기준에 따라 action과 reaction executor의 API를 최대한 대칭 구조로 정리했음.

### 4.4 Intervention의 책임

Intervention은 active execution과 incoming execution이 동시에 존재할 때만 의미가 있음.

핵심 질문은 다음과 같음.

```text
incoming은 active를 멈추고 싶은가
active는 incoming에 의해 멈춰도 되는가
priority 또는 force 정책은 어떤가
멈춘 뒤 incoming을 시작할 것인가
```

이 판단은 단순히 “현재 active가 있으면 stop”이 아님.

incoming 쪽과 active 쪽의 규칙을 모두 물어야 함.

```text
incoming executor
-> WantIntervention()

active executor
-> AllowInterventionBy()
```

이 분리 덕분에 다음 정책을 표현할 수 있음.

```text
일반 action
-> 기본적으로 active execution을 멈추고 들어가지 않음

Dodge / Counter
-> 특정 조건에서 active reaction을 cancel하고 들어갈 수 있음

HitReaction / DeadReaction
-> 일반 action을 interrupt할 수 있음

Active reaction
-> allow window가 열려 있을 때만 hit reaction에 의해 교체될 수 있음
```

### 4.5 Chain은 Intervention이 아님

작업 중 중요한 전환점 중 하나는 chain을 intervention으로 보지 않는 것이었음.

처음에는 모든 accepted execution이 intervention 판단으로 들어가야 하는지 고민했음.

하지만 combo chain은 active execution을 끊고 들어가는 관계가 아님.

Chain은 active action과 incoming action이 배척 관계가 아니라 연속 관계임.

```text
Intervention
-> 서로 공존할 수 없으므로 active를 멈출지 판단함

Chain
-> 같은 실행 흐름 안에서 다음 실행을 예약하고 소비함
```

따라서 chain은 stop directive를 만들 필요가 없음.

현재 방향은 다음과 같음.

```text
Input timing
-> chain candidate resolve
-> chainable decision
-> next data reserve

Notify timing
-> reserved data consume
-> next montage play
-> component active context commit
```

이 때문에 `ApplyChain` / `AdvanceCombo`보다 `ReserveChain` / `ConsumeChain` 쪽이 실제 의미에 더 가까운 이름으로 정리되었음.

## 5. 굵직한 전환점

### 5.1 ActionComponent 판단 제거

ActionComponent 내부에서 request를 직접 해석하던 흐름을 줄이고, request 해석과 decision 생성을 ActionOrchestrator로 이동시켰음.

이 전환으로 component는 runtime state owner에 가까워졌고, orchestrator는 request interpreter와 execution coordinator에 가까워졌음.

### 5.2 Reaction replace 사고에서 directive 사고로 전환

기존 reaction의 replace 흐름은 stop 후 start를 암묵적으로 묶고 있었음.

이번 작업에서는 stop 대상, stop reason, stop source, after-stop action을 명시하는 directive 사고로 전환했음.

이 전환은 action / reaction cross-domain 개입을 표현하기 위한 핵심 기반임.

### 5.3 Cancel / Interrupt를 API 이름이 아니라 의미로 분리

Cancel과 interrupt를 별도의 Try API 조합으로 계속 늘리는 방향은 폐기했음.

대신 stop reason과 incoming / active 관계를 통해 의미를 표현하는 방향으로 정리했음.

```text
Cancelled
-> 의도적 취소

Interrupted
-> 외부 요인 또는 강제 개입
```

이 정의는 유지하되, 실제 실행은 directive와 component apply 흐름으로 처리함.

### 5.4 Want와 Allow 분리

초기에는 interruptible / cancelable window 하나로 충분해 보였음.

하지만 “내가 끊고 싶은 상태”와 “내가 끊겨도 되는 상태”는 서로 다른 축임.

따라서 executor policy를 다음처럼 분리했음.

```text
WantIntervention
-> incoming executor의 의도

AllowInterventionBy
-> active executor의 허용
```

Reaction은 want / allow window를 분리해서 표현할 수 있고, Action은 기본적으로 보수적인 정책을 가짐.

### 5.5 Chain을 Reserve / Consume으로 재해석

Combo chain은 즉시 다음 action을 실행하는 구조가 아니라, 특정 입력 타이밍에 다음 데이터를 예약하고 notify 타이밍에 소비하는 구조임.

따라서 chain은 intervention도 아니고 일반 start도 아님.

이 구조를 명확히 하기 위해 chain 관련 API는 reserve / consume 의미로 정리되었음.

## 6. 이후 작업의 방향성

### 6.1 Execution 구체화

현재 구조는 action / reaction 공통 execution 모델의 기반을 만든 단계임.

다음 단계에서는 A15 / A16에서 정리한 것처럼 execution 관계를 더 구체화해야 함.

특히 다음 구분이 필요함.

```text
독립 실행
-> active와 관계없이 start 가능한 실행

연속 실행
-> active와 incoming이 같은 흐름에 속하는 실행

경쟁 실행
-> active를 멈추고 들어가야 하는 실행

무시 / 거절
-> 실행하지 않는 것이 정상인 경우
```

이를 위해 `Executable`, `Chainable` 같은 이름보다 실행 관계를 직접 표현하는 decision 구조가 더 적합할 수 있음.

### 6.2 Snapshot 보강

현재 snapshot은 execution state 중심으로 압축되어 있음.

장기적으로는 body execution state뿐 아니라 다음 공통 상태도 snapshot에 들어갈 수 있음.

```text
dead state
active action/reaction 존재 여부
movement locked 여부
stun / super armor / guard 상태
external control 상태
```

단, snapshot은 active participant의 상세 데이터와 중복되면 안 됨.

Snapshot은 전역 상태 압축이고, participant/context는 incoming 또는 active execution의 상세 정보로 구분해야 함.

### 6.3 Combat subsystem과의 연결

장기적으로는 action / reaction orchestration만으로 모든 전투 판정을 처리하기 어려움.

Hit interaction, guard, parry, counter, execution, damage response는 서로의 상태를 물어보고 결과를 조율해야 함.

따라서 향후 combat subsystem은 다음 역할을 가질 수 있음.

```text
interaction snapshot 수집
attacker / defender 상태 조율
damage request 생성
control request 생성
action / reaction orchestration 호출
```

다만 subsystem이 모든 실행을 직접 처리하는 구조가 되어서는 안 됨.

Subsystem은 판정과 조율을 담당하고, action / reaction component는 자기 runtime 실행을 소비하는 방향이 적절함.

## 7. 결론

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

이 구조를 통해 action과 reaction은 서로 다른 도메인이지만 같은 방식으로 조율될 수 있는 기반을 갖게 되었음.

현재 구조는 완성형이라기보다는 다음 단계의 execution 구체화를 위한 기반임.

하지만 기존의 component 중심 판단, replace 중심 reaction, cancel/interrupt API 증식 가능성, chain과 intervention 혼동을 정리했다는 점에서 중요한 전환점임.
