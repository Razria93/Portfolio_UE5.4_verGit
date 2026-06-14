# 실행 컨텍스트 / 스냅샷 책임 결정

## 1. 목적

본 문서는 execution orchestration에서 `Snapshot`, `Participant`, `ExecutionContext`, `DataKey`의 책임을 분리해야 하는 이유와 권장 기준을 정리하기 위한 문서임.

핵심은 전역 상태와 실행 주체 상세 정보를 분리하고, data lookup key와 runtime execution context의 유효성 기준을 구분하는 것임.

---

## 2. 관련 브랜치

- `orchestration-refactor`

---

## 3. 이전 시스템의 형태

### DataKey와 ExecutionContext

Action과 reaction은 각각 data key와 execution context를 가짐.

```yaml
ActionDataKey
-> ActionType
-> ActionIndex

ReactionDataKey
-> ApplyDamageSpecKey
-> ReactionType

ExecutionContext
-> DataKey
-> Data
-> Executor
```

또한 orchestration 과정에서는 현재 body state와 active execution 정보를 함께 참조해야 했음.

---

## 4. 이전 시스템의 문제 분석 및 한계

### Snapshot / Participant 책임 중복

Snapshot과 participant의 책임이 중복될 수 있었음.

Snapshot이 active action / reaction의 상세 정보를 많이 들고 있으면 participant와 역할이 겹침.

반대로 snapshot이 너무 빈약하면 decision이나 intervention 판단에 필요한 전역 상태를 전달하지 못함.

또한 wildcard의 존재 때문에 `IsValidMinimal()` 정의가 어려웠음.

### DataKey Validity 문제

```yaml
All
-> data map fallback lookup에는 유효할 수 있음

INDEX_NONE
-> any index fallback lookup에는 유효할 수 있음

Runtime ExecutionContext
-> 실제 실행 가능한 resolved data여야 함
```

즉 data key로서의 유효성과 runtime execution context로서의 유효성은 다름.

---

## 5. 리팩터링 방안 제안

### Snapshot / Participant 분리 기준

Snapshot과 participant는 다음 기준으로 분리함.

```yaml
Snapshot
-> body / execution 공통 상태
-> 예: execution state, dead state, global flags

Participant
-> incoming 또는 active execution의 상세 context
-> 예: action context, reaction context, executor, priority
```

ExecutionContext는 resolved runtime execution 단위로 취급함.

### ExecutionContext 책임

```yaml
ExecutionContext
-> DataKey
-> Data
-> Executor
```

DataKey 유효성은 장기적으로 다음처럼 분리하는 것이 적절함.

### DataKey 유효성 분리

```yaml
IsValidDataKey
-> data map key로 사용할 수 있는가

IsWildcardKey
-> fallback lookup용 key인가

IsValidResolvedKey
-> 실제 execution context로 실행 가능한가
```

---

## 6. 시행착오 과정

처음에는 snapshot에도 active 정보를 넣고, active participant에도 active 정보를 넣는 식으로 중복이 생길 수 있었음.

이 경우 다음 문제가 발생함.

```yaml
어떤 데이터가 authoritative한지 모호함
Snapshot과 ActivePart가 서로 다른 값을 가질 수 있음
incoming / active context를 읽는 관점이 혼동됨
```

DataKey도 마찬가지로, lookup용 key와 runtime execution context의 유효성 기준을 하나의 `IsValidMinimal()`로 처리하기 어려웠음.

이 문제는 wildcard data를 허용하면서도 runtime execution에는 엄격한 resolved context를 요구해야 한다는 점에서 드러남.

---

## 7. 결론

Snapshot은 전역 상태를 담고, participant는 active / incoming execution의 상세 context를 담아야 함.

DataKey는 lookup 단계와 runtime execution 단계에서 유효성 기준이 다를 수 있음.

따라서 snapshot / participant / data key / execution context는 서로 다른 책임을 갖는 구조로 유지하는 것이 적절함.








