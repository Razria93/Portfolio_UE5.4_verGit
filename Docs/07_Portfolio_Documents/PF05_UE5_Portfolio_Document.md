## 1. 문서 목적

본 문서는 Action, Reaction, Damage, Feedback에 **필요한 데이터를 data container에서 key 기반 resolve 흐름**으로 구성한 방식을 설명한다.
핵심은 실행 요청이 들어왔을 때 코드가 특정 montage, executor, damage spec, feedback와 같은 데이터를 직접 선택하지 않고, 각 component가 보유한 data container에서 key를 기준으로 필요한 데이터를 찾아 실행 흐름을 구성하는 것이다.

---
## 2. 문제 정의

기능이 늘어날수록 전투 실행에 필요한 설정이 코드 내부에 직접 분기되기 쉽다.

이 구조는 아래 문제를 가진다.

1. 새로운 action / reaction을 추가할 때 코드 수정 범위가 커진다.
2. montage, executor, damage amount, feedback 설정이 실행 로직과 섞인다.
3. weapon / action / index / reaction type 조합이 늘어날수록 분기문이 복잡해진다.
4. 공통 데이터와 세부 데이터를 함께 관리하기 어렵다.
5. Player와 Enemy가 같은 실행 구조를 공유하더라도 data 조회 기준이 흩어지면 일관성이 깨질 수 있다.

---
## 3. Data Container와 Key 구조

전투 데이터는 코드 내부 분기문이 아니라, component가 관리하는 data container에 보관된다.

data container는 여러 data entry를 보관하고, key는 그중 어떤 data를 사용할지 선택하는 기준이 된다.

현재 구조에서는 action, reaction, damage, feedback 모두 같은 방식으로 data를 찾는다.

```text
Data Container
-> Data List
-> Data Map
```

```text
Key
-> ActionDataKey
-> ReactionDataKey
-> ApplyDamageSpecKey
-> FeedbackKey
```

즉, container는 선택 가능한 data를 모아두는 저장소이고, key는 실행 상황에 맞는 data를 찾기 위한 식별자다.

---
## 4. Data Resolve 흐름

실행 요청이 들어오면 먼저 현재 상황에 맞는 key를 만든다.

이후 component가 보유한 data container에서 key에 해당하는 data entry를 찾고, resolve된 data를 실행 흐름에 전달한다.

```text
Request / Damage Result / Feedback Request
-> Key
-> Data Container
-> Data Entry
-> ExecutionContext / DamageSpec / FeedbackData
```

이 구조를 통해 실행 로직은 "어떤 data를 사용할지"를 직접 분기하지 않고, key와 container를 통해 필요한 data를 선택한다.

---
## 5. Wildcard / Fallback Lookup

Reaction과 damage data는 모든 조합을 개별 entry로 작성하면 관리 비용이 커진다.

따라서 일부 key는 wildcard와 fallback lookup을 지원한다.

예시는 다음과 같다.

```text
Exact
Weapon + Action + Index

Fallback 1
Weapon + Action + AnyIndex

Fallback 2
Weapon + AnyAction + AnyIndex

Fallback 3
AnyWeapon + AnyAction + AnyIndex
```

이를 통해 특정 공격에 대한 세부 설정과 공통 hit / damage 설정을 함께 관리할 수 있다.

---
## 6. Feedback Data

Feedback도 실행 로직에 직접 고정하지 않고 data container와 key 기반 resolve 흐름으로 구성한다.

Action, Reaction, Damage 결과는 각각 필요한 feedback request를 만들고, feedback component는 key, timing, trigger key를 기준으로 재생할 feedback data를 찾는다.

```text
Execution or Damage Result
-> Feedback Request
-> Feedback Data Container
-> Feedback Data Resolve
-> VFX / SFX / Trail / Hit Feedback
```

이를 통해 montage 실행 로직과 feedback 재생 로직을 분리할 수 있다.

---
## 7. 문제 해결

이 구조를 통해 기대한 효과는 다음과 같다.

1. Action / Reaction / Damage / Feedback 설정을 실행 로직과 분리할 수 있다.
2. 새로운 montage나 executor를 추가할 때 코드 수정 범위를 줄일 수 있다.
3. Player와 Enemy가 같은 data container와 resolve 규칙을 사용할 수 있다.
4. wildcard / fallback lookup으로 공통 데이터와 세부 데이터를 함께 관리할 수 있다.
5. feedback 재생을 execution lifecycle과 분리해 확장할 수 있다.

---
## 8. 남은 과제

- 현재 data container 구조를 Data Asset 기반 authoring 구조로 전환
- runtime execution key와 data lookup key의 validation 기준 분리
- feedback data container의 파일 책임 정리
- editor에서 key 조합을 검증하는 도구 추가

---
## 9. 정리

Data-Driven 구조의 핵심은 전투 기능을 코드 분기가 아니라, data container에 보관된 entry와 key 기반 resolve 흐름으로 구성하는 것이다.

```text
Request
-> Key
-> Data Container
-> Data Entry
-> Executor / Spec / Feedback
-> Runtime Execution
```

이를 통해 실행 로직은 공통화하고, action / reaction / damage / feedback의 세부 설정은 data로 확장할 수 있는 기반을 마련하였다.

---
