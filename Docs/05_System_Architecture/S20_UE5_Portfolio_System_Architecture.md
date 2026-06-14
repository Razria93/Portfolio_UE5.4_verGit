# 실행 계층 책임 분리 결정

## 1. 목적

본 문서는 action / reaction execution pipeline에서 Orchestrator, Component, Executor, Notify가 각각 어떤 책임을 가져야 하는지 정리하기 위한 문서임.

핵심은 실행 판단과 실행 적용을 분리하고, execution을 `Decision Level`, `Intervention Level`, `Component Apply`, `Executor Lifecycle`로 나누어 관리하는 것임.

---

## 2. 관련 브랜치

- `orchestration-refactor`

---

## 3. 이전 시스템의 형태

### Component 중심 실행 구조

기존 action / reaction 실행 흐름은 component가 많은 책임을 직접 담당하는 구조에 가까웠음.

```yaml
Request
-> Component 판단
-> Executor 실행
-> Notify 수신
-> Component 상태 갱신
```

Component는 active state를 소유하는 것뿐 아니라 request 해석, 실행 조건 판단, executor 호출, notify 전달, stop / finish 처리까지 넓은 범위를 담당했음.

### Action / Reaction 발전 방향 차이

Action과 reaction은 모두 montage 기반 execution object를 실행하지만, 기존에는 서로 다른 흐름으로 발전했음.

```yaml
Action
-> input / AI intent 중심
-> component 내부 판단 비중이 큼

Reaction
-> damage event 중심
-> active reaction과 incoming reaction의 경쟁 판단이 먼저 발전함
```

---

## 4. 이전 시스템의 문제 분석 및 한계

### Component 책임 과다

Component가 너무 많은 책임을 가지면 action / reaction이 서로 개입하기 시작할 때 경계가 흐려짐.

대표적인 문제는 다음과 같음.

```yaml
Orchestrator가 판단해야 할 일을 component가 판단함
Component가 소비해야 할 runtime state를 executor가 직접 조정함
Executor가 local rule을 넘어 cross-domain 흐름을 의식함
Notify가 component를 거칠지 executor를 직접 호출할지 불명확함
```

또한 초기에는 `Local Level`, `Policy Level`, `Orchestration Level`이라는 용어를 사용했지만, 이 용어는 책임을 충분히 구분하지 못했음.

### Level 용어의 한계

`Local Level`은 executor의 실행 가능성 판단을 의미했지만, 이름만 보면 cancel / interrupt / chain 같은 실행 방식까지 executor가 결정하는 것처럼 읽힐 수 있었음.

`Orchestration Level`도 decision 변환, intervention 판단, directive 구성, component result 구성까지 모두 포함하는 것처럼 읽힐 수 있었음.

---

## 5. 리팩터링 방안 제안

### Execution Layer 분리

책임은 다음 계층으로 분리하는 것이 적절함.

```yaml
Decision Level
-> incoming execution의 실행 가능성과 실행 관계를 판단함

Intervention Level
-> active execution과 incoming execution의 충돌을 해결함
-> stop 대상, stop 이유, stop 이후 동작을 결정함

Component Apply
-> active context와 execution state를 소유함
-> directive와 execution result를 소비함

Executor Lifecycle
-> montage play / stop / complete를 담당함
-> notify command, feedback, local rule을 담당함
```

각 객체의 책임은 다음과 같음.

### 객체별 책임

```yaml
Orchestrator
-> request 해석
-> candidate / context resolve
-> decision / intervention / directive 구성

Component
-> active context 소유
-> execution state 갱신
-> directive 소비

Executor
-> montage lifecycle
-> notify command 처리
-> feedback
-> local execution rule

Notify
-> 실행 중인 component/executor에 시점 이벤트 전달
```

---

## 6. 시행착오 과정

처음에는 action과 reaction의 API 이름을 맞추는 것이 구조 통일의 핵심처럼 보였음.

하지만 실제 문제는 API 이름이 아니라 판단과 실행 책임이 섞여 있다는 점이었음.

또한 policy bool 구조를 두어 `bCanStart`, `bCanChain`, `bCanInterrupt`, `bCanCancel`을 관리하려 했지만, 이 방식은 이미 local decision에서 판단한 내용을 다시 필터링하는 수준에 머물렀음.

결국 policy를 별도 권한 테이블처럼 두기보다, 현재 상태를 snapshot으로 전달하고 decision / intervention이 각자 필요한 판단을 수행하는 방향이 더 명확하다고 판단함.

---

## 7. 결론

Execution pipeline은 다음 흐름으로 정리하는 것이 적절함.

```yaml
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

이 구조를 따르면 component가 모든 것을 판단하지 않고, executor가 cross-domain 흐름을 직접 조율하지 않게 됨.

따라서 이후 action / reaction 기능은 이 책임 분리를 기준으로 추가하는 것이 적절함.








