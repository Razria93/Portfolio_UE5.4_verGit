# N16. AI Blackboard Key Contract Decision Note

## 목적

이 문서는 AI Blackboard key를 완전한 data-driven 입력으로 풀지 않고, `CAIKey` / `CAIKeyRegistry` 기반 C++ contract layer로 유지한 이유를 정리한다.

핵심 결론:

```text
Blackboard asset은 data-driven이다.
하지만 BT asset과 C++ 코드가 공유하는 key 이름은 추적 가능한 contract layer가 필요하다.
```

---

## 문제

Blackboard key는 BehaviorTree / Blackboard asset에서 `FName` 기반으로 사용된다.

이를 C++ / BT 노드에서 자유 입력으로 풀면 다음 구조가 된다.

```text
Blackboard asset
-> "TargetActor" key 정의

BT Task / Service / Decorator
-> FName 필드에 "TargetActor" 입력

C++ 코드
-> GetValueAsObject("TargetActor")
```

이 방식은 유연하지만, key 오타 / 타입 불일치 / 누락이 생겼을 때 추적 범위가 커진다.

```text
문제 발생 시 확인해야 하는 위치:
- Blackboard asset
- BehaviorTree asset
- BT Task / Service / Decorator 설정값
- C++ GetValueAs / SetValueAs 호출부
```

즉 단순한 key 오타도 asset과 C++ 양쪽을 모두 뒤져야 하는 문제가 된다.

---

## 선택지

### Option A. 자유 FName 입력

BT 노드나 C++ 설정에 `FName` 필드를 노출하고, asset에서 입력한 이름을 그대로 사용한다.

장점:

```text
- 가장 data-driven에 가깝다.
- key set을 C++ 재컴파일 없이 바꿀 수 있다.
- enemy archetype별 key profile을 구성하기 쉽다.
```

단점:

```text
- 오타가 컴파일 단계에서 잡히지 않는다.
- C++ 호출부와 BT asset 설정값의 연결을 검색하기 어렵다.
- key rename 시 영향 범위 추적이 어렵다.
- 각 BT 노드가 어떤 key contract를 요구하는지 코드만 보고 파악하기 어렵다.
```

### Option B. C++ FName 상수

`CAIKey::Targeting::TargetActor` 같은 C++ 상수로 Blackboard key name을 감싼다.

장점:

```text
- 문자열 직접 입력보다 검색과 rename 추적이 쉽다.
- C++ 호출부에서 key 사용 의도가 드러난다.
- BT / C++ 사이의 key vocabulary를 명시할 수 있다.
```

단점:

```text
- key type / required 여부는 별도 위치에 남는다.
- key name과 key contract가 분리된다.
```

### Option C. C++ key spec contract

`CAIKey`는 category namespace를 유지하고, 각 key를 `FAIBlackboardKeySpec`로 정의한다.

```cpp
static const FAIBlackboardKeySpec TargetActor =
{
    TEXT("TargetActor"),
    EAIBlackboardKeyValueType::Object
};
```

사용처:

```cpp
BlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName);
```

장점:

```text
- BT / Blackboard asset에서 쓰는 key name을 C++ 심볼로 추적할 수 있다.
- key name과 expected type을 한 곳에 둔다.
- CAIKeyRegistry에서 required key 검증을 구성할 수 있다.
- Blackboard API에는 명시적으로 KeyName만 전달하므로 사용 경계가 드러난다.
```

단점:

```text
- 여전히 C++ 코드에 key 목록이 존재한다.
- 완전한 data-driven key profile은 아니다.
- enemy별 key policy가 필요해지면 DataAsset 기반 확장이 필요할 수 있다.
```

---

## 결정

Phase 1에서는 Option C를 선택한다.

```text
CAIKey
-> BT / Blackboard asset에서 쓰는 FName을 C++에서 식별 가능한 심볼로 감싼다.

CAIKeyRegistry
-> 프로젝트가 요구하는 key 목록과 required 검증 흐름을 제공한다.

Blackboard asset
-> 실제 runtime Blackboard data source로 유지한다.
```

이 구조는 data-driven을 부정하지 않는다. Blackboard asset 자체는 여전히 data-driven이다.

다만 C++과 BT가 같은 key 이름을 공유하는 부분은 자유 입력으로 풀지 않고, 의도적으로 코드 계약면을 둔다.

---

## 설계 의도

`CAIKey`의 문자열은 runtime 데이터를 구성하기 위한 값이 아니라, asset key name을 코드 심볼로 치환하기 위한 값이다.

```text
BT / Blackboard asset에 존재해야 하는 이름:
"TargetActor"

C++에서 추적 가능한 이름:
CAIKey::Targeting::TargetActor

Blackboard API에 전달하는 값:
CAIKey::Targeting::TargetActor.KeyName

기대 타입:
EAIBlackboardKeyValueType::Object
```

따라서 `CAIKey`는 data table이나 runtime registry가 아니라, C++ / BT 경계의 contract vocabulary다.

---

## Data-driven 확장 기준

다음 요구가 실제로 생기면 DataAsset 기반 key profile을 검토한다.

```text
- enemy archetype마다 다른 Blackboard key set이 필요하다.
- designer가 key policy를 C++ 수정 없이 구성해야 한다.
- project-wide key contract가 아니라 mode / AI type별 contract가 필요하다.
- initial value / clear policy가 key마다 복잡해져 asset화 이점이 커진다.
```

예상 확장 형태:

```text
UAIKeyProfileDataAsset
-> TArray<FAIBlackboardKeySpec>

ACAIController
-> key profile asset 로드
-> TMap<FName, FAIBlackboardKeySpec> 구성
-> BlackboardData asset과 검증

BT Task / Service / Decorator
-> 필요 시 FName field 또는 key selector 사용
```

단, 이 단계에서는 자유도가 늘어나는 만큼 검증 책임도 같이 늘어난다.

---

## 현재 범위

이번 작업에서는 다음만 수행한다.

```text
- CAIKey를 FAIBlackboardKeySpec 기반 contract vocabulary로 변경
- CAIKeyRegistry에서 required key 목록 검증
- Blackboard API 호출부를 KeySpec.KeyName 전달 방식으로 정리
```

이번 작업에서 하지 않는 것:

```text
- DataAsset 기반 key profile 도입
- BT node에 자유 FName field 노출
- BehaviorTree asset 재설계
- enemy별 key contract 분기
```

---
