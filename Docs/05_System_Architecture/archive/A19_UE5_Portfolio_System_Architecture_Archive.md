# A19 UE5 Portfolio Execution Refactor Difficulty Analysis

## 0. 문서 역할

본 문서는 execution refactor 과정에서 실제로 혼란이 컸던 지점과 리팩터링 난이도를 높인 원인을 기록하는 difficulty / retrospective 문서임.

같은 주제를 다루는 A18, A20과의 역할 차이는 다음과 같음.

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

따라서 본 문서는 정식 설계 결정을 반복하기보다, 어떤 개념이 섞여 있었고 왜 판단이 어려웠는지를 보존하는 데 초점을 둠.

본 문서의 문제 서술은 A20의 formal issue summary와 일부 주제가 겹치지만, 목적은 해결안 자체가 아니라 리팩터링 과정에서 판단을 어렵게 만든 혼동 지점을 보존하는 것임.

## 1. 목적

본 문서는 action / reaction execution refactor 과정에서 실제로 난이도를 높였던 구조적 쟁점과, 그 쟁점을 해결하면서 작업 흐름이 어떻게 바뀌었는지 정리하기 위한 문서임.

A18이 전체 refactor history를 압축해서 설명하는 문서라면, 본 문서는 그 과정에서 특히 혼란이 컸던 논점들을 더 명시적으로 남기는 문서임.

핵심은 다음과 같음.

```text
왜 구조가 어려워졌는가
어떤 책임이 서로 섞였는가
어떤 개념이 잘못 묶여 있었는가
어떤 기준으로 다시 분리했는가
```

## 2. 난이도를 만든 배경

### 2.1 Component 중심 구조가 책임 경계를 흐리게 만듦

기존 action / reaction 실행 흐름은 component가 많은 책임을 직접 가지고 있었음.

Component는 active state를 소유하는 것뿐 아니라, request 해석, 실행 조건 판단, executor 호출, notify 전달, stop / finish 처리까지 넓은 범위를 담당했음.

단순화하면 다음과 같음.

```text
Request
-> Component 판단
-> Executor 실행
-> Notify 수신
-> Component 상태 갱신
```

이 구조에서는 작은 기능을 추가할 때는 빠르게 구현할 수 있었지만, action과 reaction이 서로 개입하기 시작하면 책임 경계가 흐려졌음.

### 2.2 Action과 Reaction의 비대칭이 구조 통일을 어렵게 만듦

Action과 reaction은 모두 montage 기반 execution object를 실행한다는 점에서 유사함.

하지만 기존에는 서로 다른 흐름으로 발전했음.

```text
Action
-> input / AI intent 중심
-> component 내부 실행 판단 비중이 큼
-> combo chain, equip, unequip 등 action 종류별 분기가 늘어남

Reaction
-> damage event 중심
-> incoming reaction과 active reaction의 경쟁 판단이 먼저 발전함
-> replace에 가까운 stop-start 흐름이 존재함
```

이 비대칭은 action orchestration을 reaction 구조에 맞춰 고도화하려는 시점에서 큰 난이도가 되었음.

단순히 API 이름만 맞추는 것으로는 부족했고, 실행 흐름과 책임 자체를 맞춰야 했음.

### 2.3 Chain이 예외처럼 보이면서 실행 관계 판단을 어렵게 만듦

Combo chain은 기존 구조 안에서 계속 예외처럼 취급되었음.

입력이 들어온 시점에 즉시 다음 montage를 실행하지 않고, chain window 안에서 다음 데이터를 저장한 뒤, notify timing에서 소비하는 방식이었기 때문임.

```text
Input timing
-> 다음 chain data 저장

Notify timing
-> 저장된 chain data 소비
-> 다음 montage 실행
```

이 흐름은 start / stop / interrupt 같은 일반 실행 흐름과 다르게 보였고, orchestration 구조를 설계할 때 계속 혼란을 만들었음.

## 3. 혼란이 컸던 지점

### 3.1 네 계층의 책임 경계가 흐렸음

이번 refactor에서 가장 먼저 정리해야 했던 것은 다음 네 계층의 책임이었음.

```text
Orchestrator
Component
Executor
Notify
```

기존 구조에서는 다음 책임들이 쉽게 섞였음.

```text
Orchestrator가 판단해야 할 일을 component가 판단함
Component가 소비해야 할 runtime state를 executor가 직접 조정함
Executor가 local rule을 넘어 cross-domain 흐름을 의식함
Notify가 component를 거칠지 executor를 직접 호출할지 불명확함
```

이 상태에서는 기능을 추가할수록 같은 판단이 여러 곳에서 반복될 수밖에 없음.

### 3.2 Local / Orchestration 용어가 책임 중첩을 가렸음

작업 중간에는 local level과 orchestration level을 나누었지만, 실제 구현에서는 역할이 명확하지 않았음.

대표적인 문제는 다음과 같음.

```text
Local에서 cancel / interrupt 성격까지 판단함
Policy가 local decision을 다시 필터링함
Orchestration level이 local decision을 거의 변환만 함
같은 decision이 여러 단계에서 반복됨
```

이 흐름에서는 policy가 독립적인 판단 계층이 아니라, 이미 나온 결론을 다시 허용/거절하는 얇은 필터가 되기 쉬웠음.

따라서 이후 방향은 policy라는 용어보다 snapshot / decision / intervention / directive로 책임을 나누는 쪽으로 이동했음.

### 3.3 Intervention을 판단이 아니라 실행 API로 이해하기 쉬웠음

Intervention은 단순히 stop API를 호출하는 것이 아님.

먼저 판단해야 하는 것은 다음임.

```text
incoming은 active를 멈추고 싶은가
active는 incoming에 의해 멈춰도 되는가
stop reason은 무엇인가
stop target은 action인가 reaction인가
stop 이후 incoming을 시작할 것인가
```

기존에는 이런 판단과 실행이 `TryReplace`, `TryInterrupt`, `TryCancel` 같은 API 이름 안에 섞이기 쉬웠음.

하지만 action / reaction 교차 개입이 생기면 다음 조합이 모두 가능해짐.

```text
Action -> Action
Action -> Reaction
Reaction -> Action
Reaction -> Reaction
```

따라서 stop과 start를 조합한 API를 계속 늘리는 방식은 적절하지 않았음.

판단은 intervention query / directive로 정리하고, 실행은 component가 directive를 소비하는 방향으로 분리해야 했음.

### 3.4 Cancel과 Interrupt가 의미가 아니라 API 조합처럼 보였음

초기에는 cancel과 interrupt를 다음처럼 이해했음.

```text
Cancel
-> 의도적 중단

Interrupt
-> 외부 요인에 의한 중단
```

이 기준은 지금도 유효함.

문제는 이 의미가 API나 window와 직접 1:1로 대응된다고 생각하면 혼란이 생긴다는 점이었음.

예를 들어 dodge가 hit reaction을 끊고 들어가는 것은 외부 강제 중단이 아니라 의도적 cancel에 가까움.

반대로 hit reaction이 attack action을 끊는 것은 active action 입장에서는 interrupt임.

따라서 cancel / interrupt는 단순히 어느 domain에서 발생했는지가 아니라, incoming과 active의 관계, stop reason, stop source, after-stop action을 함께 보고 해석해야 함.

### 3.5 Want와 Allow가 같은 window처럼 보였음

Intervention을 설계하면서 헷갈렸던 핵심 중 하나는 “끊고 싶은 상태”와 “끊겨도 되는 상태”를 같은 window로 볼 수 있는가였음.

결론적으로 둘은 다른 축임.

```text
WantIntervention
-> incoming execution이 active를 멈추고 싶은가

AllowInterventionBy
-> active execution이 incoming에 의해 멈춰도 되는가
```

예를 들어 reaction은 incoming hit reaction이 active reaction을 interrupt하고 싶은 상태와, active reaction이 interrupt를 허용하는 상태를 분리해서 표현할 수 있어야 함.

Action도 마찬가지로 일반 action은 기본적으로 active execution을 멈추고 들어가지 않지만, dodge나 counter 같은 action은 override를 통해 intervention을 원할 수 있음.

### 3.6 Snapshot과 Participant의 책임 경계가 헷갈렸음

Execution snapshot과 active participant를 설계할 때도 책임 중복이 생겼음.

Snapshot이 active action / reaction의 상세 정보를 많이 들고 있으면 participant와 역할이 겹침.

반대로 snapshot이 너무 빈약하면 local decision이나 intervention 판단에 필요한 전역 상태를 전달하지 못함.

따라서 기준은 다음처럼 정리됨.

```text
Snapshot
-> body / execution의 공통 상태 압축

Participant
-> incoming 또는 active execution의 상세 context
```

Snapshot은 “현재 몸 상태가 어떤가”를 말해야 하고, participant는 “누가 들어오고 있으며 누가 active인가”를 말해야 함.

### 3.7 Wildcard 때문에 IsValid 기준이 흔들렸음

DataKey는 map lookup key이면서, wildcard data를 표현하는 역할도 가짐.

예를 들어 `All`, `INDEX_NONE`은 런타임 execution request에서는 애매할 수 있지만, data table key로는 유효한 값일 수 있음.

따라서 `IsValidMinimal()`에서 `All`이나 `INDEX_NONE`을 무조건 invalid로 막으면 wildcard data를 사용할 수 없게 됨.

반대로 너무 느슨하게 허용하면 실제 execution context로 들어오면 안 되는 key가 통과할 수 있음.

이 문제는 “data key로서의 유효성”과 “resolved execution context로서의 유효성”을 구분해야 한다는 점을 드러냈음.

### 3.8 Montage End와 Complete / Stop 책임이 겹쳤음

Executor는 montage lifecycle을 가장 잘 알고 있음.

하지만 component는 active context와 execution state를 소유함.

따라서 다음 두 흐름이 공존하면서 책임이 겹치기 쉬웠음.

```text
Montage naturally ends
-> executor complete
-> component end

System requests stop
-> executor stop
-> component end
```

Stop이 곧바로 finish까지 처리할지, montage end delegate를 기다릴지, fallback을 둘지에 대한 고민이 있었음.

현재 방향은 명시적 stop 요청에서는 executor가 stop reason에 맞게 정리하고 component에 finish를 알리는 구조에 가깝지만, montage delegate와 fallback의 경계는 이후에도 계속 조심해야 하는 부분임.

### 3.9 Active와 Incoming의 관점이 자주 혼동되었음

Intervention query는 incoming participant와 active participant를 동시에 가짐.

이 때문에 코드를 읽을 때 다음이 자주 혼동될 수 있었음.

```text
이 executor는 incoming인가 active인가
이 context는 자기 자신의 data인가 상대 data인가
이 조건은 incoming type을 보는가 active type을 보는가
```

예를 들어 HitReaction executor가 incoming reaction type이 Hit인지 확인하는 코드는 “현재 active가 Hit일 때만 interrupt하겠다”는 뜻이 아님.

그 executor가 처리하는 incoming reaction이 Hit인지 확인하는 것임.

이 구분이 명확하지 않으면 intervention policy를 잘못 읽기 쉬움.

## 4. 혼란을 줄이기 위해 정리한 기준

### 4.1 네 계층 책임 분리

최종적으로 작업 방향은 다음 계층 분리로 정리되었음.

```text
Orchestrator
-> request 해석, candidate/context resolve, decision/intervention/directive 구성

Component
-> active context 소유, execution state 갱신, directive 소비

Executor
-> montage lifecycle, notify command 처리, feedback, local rule

Notify
-> 실행 중인 component/executor에 시점 이벤트 전달
```

이 구분을 통해 component가 모든 것을 판단하지 않고, executor가 cross-domain 흐름을 직접 조율하지 않게 되었음.

### 4.2 Action / Reaction 구조 통일

Action과 reaction은 서로 다른 도메인이지만, execution이라는 관점에서는 같은 흐름을 가질 수 있음.

```text
Candidate
-> ExecutionContext
-> ExecutionDecisionQuery
-> ExecutionDecisionResult
-> InterventionDirective
-> Component Apply
-> Executor Lifecycle
```

이 통일 덕분에 action이 reaction을 멈추거나, reaction이 action을 멈추는 cross-domain intervention을 공통 구조로 표현할 수 있게 되었음.

### 4.3 Policy에서 Snapshot으로 전환

기존 policy는 “무엇을 할 수 있는가”를 bool로 들고 있었음.

하지만 이 방식은 local decision과 중복되기 쉬웠음.

```text
bCanStart
bCanChain
bCanEnqueue
bCanInterrupt
bCanCancel
```

이런 값은 이미 local decision이나 intervention 판단에서 다시 해석되어야 했음.

따라서 policy를 별도 권한 테이블처럼 두기보다, 현재 상태를 압축한 snapshot을 전달하고 executor / orchestrator가 각자의 책임 안에서 판단하는 방향이 더 명확함.

```text
Snapshot
-> 현재 body / execution 상태

Decision
-> incoming이 어떤 실행 관계를 원하는가

Intervention
-> active와 충돌한다면 어떻게 조율할 것인가
```

### 4.4 Intervention 판단과 실행 분리

Intervention 판단은 orchestrator에서 수행하고, 실행은 component에서 수행함.

Orchestrator는 directive를 만들고, component는 directive를 소비함.

```text
Orchestrator
-> FExecutionInterventionDirective 생성

Component
-> TargetDomain 확인
-> active action 또는 reaction stop 요청
-> after-stop action에 따라 incoming start / stop only 처리
```

이 분리를 통해 stop target, stop reason, stop source, after-stop action이 명확해짐.

### 4.5 Chain을 실행 관계로 재정의

Chain은 예외가 아니라 action execution relationship 중 하나임.

중요한 전환은 다음 문장으로 요약할 수 있음.

```text
체인이 유별난 것이 아니라, action 간 관계가 독립 실행인지 연속 실행인지가 달랐던 것임.
```

기존에는 action을 단발적인 실행으로만 보고 있었기 때문에 chain이 계속 예외처럼 보였음.

하지만 action 사이의 관계를 나누면 chain의 위치가 명확해짐.

```text
Independent relationship
-> active와 무관하게 실행되거나 active를 멈추고 실행됨

Sequential relationship
-> active와 incoming이 같은 흐름에 속하고, 다음 실행을 예약/소비함

Competitive relationship
-> active와 incoming이 공존할 수 없어 intervention 판단이 필요함
```

이 기준에서 chain은 competitive relationship이 아니라 sequential relationship임.

따라서 chain은 directive를 만들지 않음.

```cpp
// [NOTE] Chain
if (InOutResult.Decision == EExecutionDecision::Chainable) return true;
```

이 코드는 처음에는 납득하기 어려웠지만, chain이 intervention 대상이 아니라는 관점에서는 타당함.

다만 장기적으로는 `Chainable` 같은 decision 이름보다 `ReserveSequentialExecution`처럼 실행 관계를 더 직접적으로 표현하는 구조가 더 명확할 수 있음.

### 4.6 Reserve / Consume으로 이름 정리

`ApplyChain` / `AdvanceCombo`는 구현의 실제 의미를 충분히 드러내지 못했음.

실제로는 다음 흐름임.

```text
Reserve
-> chain window 안에서 다음 실행 데이터를 저장함

Consume
-> notify timing에서 저장된 데이터를 소비하고 다음 montage를 실행함
```

따라서 chain은 단순히 “다음 콤보로 진행”이 아니라, 실행 데이터의 예약과 소비로 이해하는 것이 더 정확함.

이 관점은 인덱스 증가 위치를 이해하는 데도 중요함.

```text
Candidate resolve
-> incoming index 결정

Reserve
-> incoming data 저장

Consume
-> 저장된 data를 active context로 commit
```

즉 chain index는 consume 시점에 새로 계산되는 것이 아니라, candidate resolve 시점에 결정되고 reserve된 뒤 consume 시점에 반영됨.

## 5. 작업 흐름의 굵직한 전환점

### 5.1 Component 중심에서 Orchestrator 중심으로

처음에는 component가 많은 실행 판단을 들고 있었음.

이후 request 해석과 execution decision은 orchestrator가 맡고, component는 result와 directive를 소비하는 방향으로 이동했음.

이 전환이 action orchestration refactor의 첫 번째 큰 축임.

### 5.2 Replace에서 Intervention Directive로

Reaction의 replace 사고는 action / reaction 교차 간섭을 표현하기에 부족했음.

Stop 후 start가 항상 따라온다는 전제가 깨졌기 때문임.

이후 구조는 stop 대상과 after-stop action을 분리하는 directive 흐름으로 이동했음.

### 5.3 Policy Level에서 Snapshot 기반 판단으로

Policy bool 묶음은 local과 orchestration 사이에서 책임이 애매했음.

이후에는 현재 상태를 snapshot으로 전달하고, decision과 intervention이 각자 필요한 판단을 수행하는 쪽이 더 명확하다고 판단했음.

### 5.4 Want / Allow 분리

Incoming과 active의 관점을 분리하기 위해 `WantIntervention()`과 `AllowInterventionBy()`를 분리했음.

이 전환은 cancel / interrupt window의 책임 혼동을 줄이는 데 중요했음.

### 5.5 Chain을 Sequential Relationship으로 재해석

Chain은 더 이상 특수 예외가 아니라 sequential execution relationship으로 정의됨.

이 덕분에 chain이 intervention 판단을 거치지 않는 이유가 명확해졌음.

## 6. 이후 작업의 방향성

### 6.1 Execution Decision 이름 재검토

현재 `Executable`, `Chainable`은 임시적으로 의미를 전달하지만, 장기적으로는 실행 관계를 더 직접적으로 표현하는 이름이 필요할 수 있음.

예시는 다음과 같음.

```text
StartImmediate
ReserveSequential
StartAfterCancel
StartAfterInterrupt
Ignore
Reject
```

중요한 것은 “실행 가능 여부”만 표현하는 것이 아니라, active execution과 incoming execution의 관계를 함께 표현하는 것임.

### 6.2 Relationship 판단 분기

모든 accepted execution이 intervention으로 들어가면 안 됨.

먼저 incoming과 active의 관계를 분류해야 함.

```text
No active
-> immediate start

Sequential relationship
-> reserve / consume

Competitive relationship
-> intervention query / directive

Invalid relationship
-> reject / ignore
```

이 분기가 명확해지면 chain 같은 흐름이 예외가 아니라 정상적인 relationship 처리로 들어감.

### 6.3 Snapshot과 Participant 구조 정리

Snapshot과 participant는 계속 분리해서 유지해야 함.

```text
Snapshot
-> body / execution 공통 상태

Participant
-> incoming / active execution 상세 context
```

Snapshot에 active 상세 데이터를 과하게 넣으면 participant와 중복되고, participant에 전역 상태를 넣으면 query의 책임이 흐려짐.

### 6.4 DataKey 유효성 기준 분리

Wildcard를 지원하려면 `IsValidMinimal()` 하나로 모든 상황을 처리하기 어렵다.

장기적으로는 다음 기준을 분리할 수 있음.

```text
IsValidDataKey
-> data map key로 사용할 수 있는가

IsValidResolvedKey
-> 실제 execution context로 실행 가능한가

IsWildcardKey
-> fallback lookup용 key인가
```

이 구분이 없으면 wildcard data와 runtime execution data의 유효성 판단이 계속 충돌할 수 있음.

### 6.5 Montage lifecycle 정리

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

이 원칙이 명확해야 fallback 처리와 unexpected interruption log가 과도하게 섞이지 않음.

## 7. 결론

이번 refactor의 난이도는 단순한 코드량 때문이 아니었음.

실제 난이도는 action과 reaction을 하나의 execution 모델로 통합하면서, 서로 다른 개념들이 같은 API와 같은 흐름 안에 섞여 있었던 데서 발생했음.

특히 중요한 전환은 다음임.

```text
Component 판단
-> Orchestrator 판단 / Component 소비

Replace
-> Intervention Directive

Policy bool
-> Snapshot + Decision + Intervention

Interruptible / Cancelable 단일 window
-> Want / Allow 분리

Combo 예외
-> Sequential execution relationship

Apply / Advance chain
-> Reserve / Consume chain
```

결과적으로 현재 구조는 완성형이라기보다 execution 구체화 작업을 위한 기반임.

하지만 이 기반은 이전보다 훨씬 명확함.

이제 다음 단계에서는 action / reaction이 어떤 관계로 만나는지, 그 관계가 immediate / sequential / competitive 중 무엇인지, 그리고 그 결과를 어떤 directive로 component에 전달할지를 더 구체화하면 됨.
