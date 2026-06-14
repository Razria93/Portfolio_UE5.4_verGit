# 실행 개입 디렉티브 구조 결정

## 1. 목적

본 문서는 action / reaction execution 사이의 cross-domain intervention을 공통 directive 구조로 표현해야 하는 이유와 권장 구조를 정리하기 위한 문서임.

핵심은 stop과 start를 하나의 replace API로 묶지 않고, “누가, 누구를, 왜 멈추며, 멈춘 뒤 무엇을 할 것인가”를 명시적으로 분리하는 것임.

---

## 2. 관련 브랜치

- `orchestration-refactor`

---

## 3. 이전 시스템의 형태

### Reaction Replace에 가까운 초기 구조

초기 reaction 구조는 replace에 가까운 흐름으로 이해되기 쉬웠음.

```yaml
incoming reaction
-> active reaction이 있으면 멈춤
-> incoming reaction 시작
```

이 방식은 reaction끼리만 볼 때는 자연스러웠지만, action과 reaction이 서로 개입하기 시작하면 부족했음.

### Cross-Domain Intervention 예시

예시는 다음과 같음.

```yaml
Reaction -> Action
-> HitReaction이 AttackAction을 interrupt함

Action -> Reaction
-> DodgeAction이 HitReaction을 cancel함

Reaction -> Reaction
-> DeadReaction이 HitReaction을 interrupt함

Action -> Action
-> 특정 action이 active action을 cancel하고 진입할 수 있음
```

---

## 4. 이전 시스템의 문제 분석 및 한계

### Stop API만으로 표현하기 어려운 정보

Intervention을 stop API 호출로만 보면 다음 정보가 누락됨.

```yaml
누가 개입하는가
누구를 멈추는가
왜 멈추는가
멈춘 뒤 무엇을 할 것인가
```

초기에는 `TryInterruptAndStart`, `TryCancelAndStart`, `TryInterruptAndEnd`, `TryCancelAndEnd`처럼 API를 나눌 수 있어 보였음.

### API 조합 증가 문제

하지만 action / reaction 교차 조합이 늘어날수록 API 조합이 폭발함.

또한 모든 개입이 stop 후 start는 아님.

### Stop 이후 Action 차이

```yaml
Reaction -> Reaction
-> active reaction stop
-> incoming reaction start

Action -> Reaction cancel
-> active reaction stop
-> incoming action start

StopOnly
-> active execution stop
-> 후속 실행 없음
```

---

## 5. 리팩터링 방안 제안

### 공통 Directive 구조

Intervention은 공통 directive로 표현하는 것이 적절함.

```yaml
FExecutionInterventionDirective
-> TargetDomain
-> StopReason
-> StopSource
-> AfterStopAction
```

각 필드의 의미는 다음과 같음.

### Directive 필드 의미

```yaml
TargetDomain
-> 무엇을 멈출지

StopReason
-> 왜 멈출지

StopSource
-> 누가 멈추라고 결정했는지

AfterStopAction
-> 멈춘 뒤 무엇을 할지
```

권장 흐름은 다음과 같음.

### Directive 소비 Flow

```yaml
Orchestrator
-> active / incoming participant 구성
-> intervention 가능성 판단
-> directive 생성

Component
-> directive consume
-> target domain stop 요청
-> after-stop action 수행
```

---

## 6. 시행착오 과정

처음에는 stop과 start를 함께 처리하는 `TryReplace` 계열 API가 자연스럽게 보였음.

하지만 action이 reaction을 cancel하고 들어가는 흐름은 reaction replace가 아니며, action orchestration 결과가 reaction component에 stop 요청을 보내는 구조에 가까움.

또한 interrupt / cancel / end / start 조합을 API 이름으로 모두 표현하면 구조가 빠르게 복잡해짐.

따라서 API 조합을 늘리는 것이 아니라, stop 대상과 후속 동작을 directive로 분리하는 쪽이 더 안정적이라고 판단함.

---

## 7. 결론

Cross-domain intervention은 단순 stop 호출이 아니라 조율 결과임.

따라서 판단은 orchestrator가 수행하고, 실행은 component가 directive를 소비하는 구조가 적절함.

이 구조를 따르면 action -> reaction, reaction -> action, reaction -> reaction, action -> action 흐름을 같은 모델로 표현할 수 있음.








