# 실행 개입 정책 결정

## 1. 목적

본 문서는 action / reaction executor가 intervention 과정에서 `WantIntervention()`과 `AllowInterventionBy()`를 왜 분리해야 하는지 정리하기 위한 문서임.

핵심은 “내가 active execution을 멈추고 싶은가”와 “내가 incoming execution에 의해 멈춰도 되는가”를 서로 다른 질문으로 취급하는 것임.

---

## 2. 관련 브랜치

- `orchestration-refactor`

---

## 3. 이전 시스템의 형태

### Cancel / Interrupt 초기 해석

초기에는 cancel과 interrupt를 다음처럼 단순하게 이해할 수 있었음.

```yaml
Cancel
-> 의도적 중단

Interrupt
-> 외부 요인 또는 강제 개입에 의한 중단
```

또한 interruptible / cancelable window를 통해 실행 중인 action 또는 reaction이 중단 가능한 상태인지 표현했음.

---

## 4. 이전 시스템의 문제 분석 및 한계

### Cancel / Interrupt와 Window 혼재

Cancel / interrupt를 API 이름이나 window와 1:1로 대응시키면 혼란이 생김.

예를 들어 dodge가 hit reaction을 끊고 들어가는 것은 외부 강제 중단이 아니라 의도적 cancel에 가까움.

반대로 hit reaction이 attack action을 끊는 것은 active action 입장에서는 interrupt임.

또한 다음 두 질문은 서로 다름.

### Want와 Allow의 질문 차이

```yaml
WantIntervention
-> incoming execution이 active를 멈추고 싶은가

AllowInterventionBy
-> active execution이 incoming에 의해 멈춰도 되는가
```

이 둘을 같은 window로 처리하면 incoming 쪽 의도와 active 쪽 허용 정책이 섞임.

---

## 5. 리팩터링 방안 제안

### Executor Intervention API

Executor는 intervention query를 기준으로 다음 API를 제공하는 것이 적절함.

```cpp
virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const;
virtual bool AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const;
```

기본 정책은 다음과 같이 볼 수 있음.

### 기본 정책 의미

```yaml
WantIntervention
-> incoming-side rule
-> 일반 action은 기본적으로 active execution을 멈추려 하지 않음
-> dodge / counter 같은 action은 override 가능

AllowInterventionBy
-> active-side rule
-> active execution이 현재 incoming에 의해 멈춰도 되는지 판단함
```

Reaction과 action의 기본 정책은 다를 수 있음.

### 기본 정책 예시

예시는 다음과 같음.

```yaml
Hit / Dead reaction
-> 일반 action을 interrupt할 수 있음

Dodge action
-> active hit reaction을 cancel하고 들어갈 수 있음

Dead reaction
-> 대부분의 active execution보다 높은 우선순위를 가질 수 있음
```

---

## 6. 시행착오 과정

초기에는 interrupt window와 cancel window만 있으면 충분해 보였음.

하지만 action이 reaction을 끊거나 reaction이 action을 끊는 cross-domain 흐름이 들어오면서, “끊고 싶은 쪽”과 “끊겨도 되는 쪽”이 서로 다른 executor라는 점이 중요해졌음.

즉 incoming executor는 자기 의도를 말해야 하고, active executor는 자기 방어 정책을 말해야 함.

이 구분이 없으면 dodge, counter, guard, parry 같은 변칙 action이 추가될 때 조건이 한쪽으로 몰리게 됨.

---

## 7. 결론

Intervention policy는 incoming과 active의 관점을 분리해야 함.

```yaml
incoming
-> WantIntervention()

active
-> AllowInterventionBy()
```

Cancel / interrupt 여부는 단순 enum만으로 결정하지 않고, stop reason, stop source, target domain, incoming / active executor rule을 함께 보고 판단하는 것이 적절함.








