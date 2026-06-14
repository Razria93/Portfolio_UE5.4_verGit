# 실행 관계 분류 결정

## 1. 목적

본 문서는 action / reaction execution이 active execution과 어떤 관계를 맺는지 분류하고, 그 관계에 따라 실행 흐름을 다르게 처리해야 하는 이유를 정리하기 위한 문서임.

핵심은 execution을 단순히 실행 가능한지 여부로만 판단하지 않고, active / incoming 관계를 기준으로 `Independent`, `Sequential`, `Exclusive` 흐름으로 나누는 것임.

---

## 2. 관련 브랜치

- `orchestration-refactor`

---

## 3. 이전 시스템의 형태

### 기존 Decision 값

기존 구조에서는 execution decision이 대체로 “실행 가능한가”에 가까웠음.

예시는 다음과 같음.

```yaml
Executable
-> 실행 가능함

Chainable
-> chain 가능함

Reject
-> 실행 불가

Ignore
-> 요청 무시
```

이 구조에서는 실행 가능하다고 판단된 이후의 처리 흐름이 대부분 start 또는 intervention 흐름으로 이어지기 쉬웠음.

### Chain이 예외처럼 보이는 구조

Combo chain은 이 구조 안에서 계속 예외처럼 보였음.

```yaml
Input timing
-> next chain data reserve

Notify timing
-> reserved chain data consume
-> next montage 실행
```

---

## 4. 이전 시스템의 문제 분석 및 한계

### 실행 가능 여부와 실행 방식 혼재

`Executable`과 `Chainable`은 실행 가능 여부와 실행 방식을 동시에 표현함.

하지만 실행 판단은 최소한 다음 세 축으로 분리되어야 함.

```yaml
실행 가능 여부
-> Accept / Reject / Ignore

실행 관계
-> Independent / Sequential / Exclusive

실행 적용 방식
-> Start / Reserve / Intervene / StopOnly
```

이 세 축이 섞이면 chain 같은 흐름이 예외처럼 보임.

### Chain의 실제 관계

Chain은 active execution을 멈추고 들어가는 흐름이 아니라, 기존 active execution과 연속되는 흐름임.

즉 chain은 competitive execution이 아니라 sequential execution임.

---

## 5. 리팩터링 방안 제안

### Execution Relationship 분류

Execution relationship은 다음 기준으로 분리하는 것이 적절함.

```yaml
Independent
-> active execution과 충돌하지 않거나 active가 없는 상태에서 독립적으로 실행됨

Sequential
-> active execution과 incoming execution이 같은 흐름에 속함
-> continuity 검증 후 reserve / consume 흐름으로 처리함

Exclusive
-> active execution과 incoming execution이 공존할 수 없음
-> intervention 판단이 필요함
```

실행 적용 방식은 relationship에 따라 달라져야 함.

### Relationship별 Apply 방식

```yaml
No active
-> immediate start

Sequential
-> reserve / consume

Exclusive
-> intervention query / directive

Invalid
-> reject / ignore
```

Combo chain은 다음 흐름으로 정리함.

### Combo Chain Reserve / Consume

```yaml
Resolve
-> incoming index 결정

Reserve
-> incoming data 저장

Consume
-> 저장된 data를 active context로 commit
```

---

## 6. 시행착오 과정

초기에는 chain이 특수 예외처럼 보였음.

특히 accepted execution이 모두 intervention 판단으로 들어가는 구조에서는 chain도 intervention 대상처럼 보였음.

```cpp
// Chain은 active execution을 멈추지 않으므로 intervention 대상이 아님.
if (InOutResult.Decision == EExecutionDecision::Chainable) return true;
```

이 흐름은 처음에는 어색해 보였지만, 실제로는 chain이 active execution을 멈추고 들어가는 흐름이 아니었기 때문에 intervention 대상이 아니었음.

문제는 chain이 특수한 것이 아니라, execution relationship을 충분히 구분하지 못한 것이었음.

---

## 7. 결론

Execution decision은 단순히 가능한지 여부만 표현해서는 부족함.

향후 구조는 다음 세 축을 분리해야 함.

```yaml
실행 가능 여부
실행 관계
실행 적용 방식
```

Chain은 예외가 아니라 `Sequential relationship`의 대표 사례임.

따라서 chain은 intervention directive를 만들지 않고 reserve / consume 흐름으로 처리하는 것이 적절함.








