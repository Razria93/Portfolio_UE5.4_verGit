# N15. AI Blackboard Key Registry Policy Note

## 목적

이 문서는 `refactor/ai-blackboard-key-registry` 작업의 설계 기준을 정리한다.

목표는 BehaviorTree 동작을 바꾸는 것이 아니라, blackboard key 계약을 코드에서 한 곳에 모아 검증 / 초기화 / 정리 누락을 줄이는 것이다.

`CAIKey`를 data-driven 입력으로 풀지 않고 C++ contract vocabulary로 유지한 결정 배경은 `N16_AI_Blackboard_Key_Contract_Decision_Note.md`에 별도로 정리한다.

---

## 현재 구조

현재 key name은 `CAIKey.h`에 namespace로 묶여 있다.

```text
Targeting
State
Perception
Metric
Navigation
Patrol
Investigate
Chase
Alert
Engage
Reaction
Dead
```

이 구조는 key vocabulary로는 읽기 쉽지만, 다음 정보는 별도로 흩어져 있다.

```text
key type
required 여부
initial value 적용 여부
clear 대상 여부
runtime owner
```

현재 분산 상태:

```text
CAIKey.h
-> key name 정의

ACAIController::ValidateBlackboardKeys
-> required key 수동 검증

ACAIController::SetInitialBlackboardRuntimeValues
-> initial runtime value 수동 설정

ACAIController::ClearBlackboardRuntimeValues
-> teardown clear 대상 수동 나열

BT Service / Task / Decorator
-> CAIKey 직접 read / write
```

---

## 문제

### 1. key 추가 / 삭제 시 수정 지점이 많다

현재는 key를 하나 추가하면 다음 위치를 모두 맞춰야 한다.

```text
CAIKey.h
ValidateBlackboardKeys
SetInitialBlackboardRuntimeValues
ClearBlackboardRuntimeValues
BT node usage
Blackboard asset
```

누락되면 컴파일은 되지만 runtime blackboard key가 없거나 초기값이 비어 있는 상태가 될 수 있다.

### 2. required key 기준을 코드에서 설명하기 어렵다

`ValidateBlackboardKeys()`는 수동 변수와 `bAllValid` 누적이 길게 나열되어 있다.

이 방식은 key 목록 전체를 검토하기 어렵고, 누락된 key 이름을 추적하기 어렵다.

### 3. 초기값과 clear 정책이 key 정의와 떨어져 있다

어떤 key가 controller possess 시점에 초기값을 받아야 하는지, 어떤 key가 teardown에서 clear되어야 하는지 `CAIKey`만 보고 알 수 없다.

---

## 설계 방향

### 1. CAIKey는 spec vocabulary로 유지한다

BT Service / Task / Decorator에서 이미 `CAIKey::Category::KeyName` 형태를 넓게 사용하고 있으므로, category namespace는 유지한다.

다만 `CAIKey`의 값은 단순 `FName`이 아니라 `FAIBlackboardKeySpec`로 정의한다.

```text
CAIKey::Targeting::TargetActor
-> FAIBlackboardKeySpec

CAIKey::Targeting::TargetActor.KeyName
-> Blackboard API에 전달하는 FName
```

이렇게 하면 key category 사용감은 유지하면서 key name / expected type / required 여부 같은 계약 정보를 key 정의 근처에 둘 수 있다.

### 2. registry는 key 등록과 검증을 담당한다

`CAIKey.h`는 개별 key의 계약 정보를 정의하고, `CAIKeyRegistry.h`는 전체 key 목록 등록과 검증 흐름을 담당한다.

key spec은 다음 정보를 제공한다.

```text
FName KeyName
EAIBlackboardKeyValueType ValueType
bool bRequired
bool bClearOnRuntimeTeardown
```

초기값까지 registry에 넣을지 여부는 key 성격에 따라 나눈다.

```text
공통 fixed initial value
-> registry 또는 helper table에서 처리 가능

enemy instance에서 읽어야 하는 value
-> controller helper에서 별도 적용 유지
```

예:

```text
CAIKey::State::AIIntentState.KeyName
-> fixed initial value: Idle

CAIKey::Navigation::HomeLocation.KeyName
-> owner pawn location 필요

CAIKey::Patrol::bUsePatrol.KeyName
-> ACEnemy instance 설정 필요
```

### 3. Validation은 registry 순회로 바꾼다

```text
ACAIController::SetupBlackboardComponent
-> CAIKeyRegistry::ValidateRequiredKeys
-> CAIKeyRegistry::GetKeySpecs 순회
-> required key 검증
```

검증 실패 시 누락된 key name과 expected type을 모아 `ensureMsgf` 1회로 남긴다.

```text
[AIKeyRegistry] Missing required Blackboard keys | Blackboard=... | Missing=TargetActor:Object, ...
```

개별 key마다 별도 로그를 찍지 않는 이유:

```text
- required blackboard key 누락은 asset 계약 위반이므로 ensure가 적합하다.
- 누락 key마다 ensure / log를 남기면 callstack과 로그가 과해진다.
- 최종 ensure 1회가 Blackboard asset, 누락 key 목록, expected type을 함께 보여준다.
```

### 4. Runtime value setup은 두 단계로 나눈다

```text
SetInitialBlackboardRuntimeValues
-> ApplyFixedInitialBlackboardValues
-> ApplyOwnerBlackboardValues
```

고정 초기값과 enemy instance 기반 값을 구분하면 함수가 길어지는 문제를 줄일 수 있다.

### 5. Clear는 registry 기반으로 단순화한다

```text
ClearBlackboardRuntimeValues
-> GetRuntimeClearBlackboardKeySpecs()
-> ClearValue 반복
```

단, `DeadState`처럼 clear보다 Alive reset이 의미적으로 맞는 key는 registry에서 clear 대상이 아닌 reset 대상으로 분류할 수 있다.

---

## 구현 결정

### 선택한 형태

```text
CAIKey.h
-> EAIBlackboardKeyValueType
-> FAIBlackboardKeySpec
-> CAIKey::Category::KeySpec

CAIKeyRegistry.h
-> GetKeySpecs()
-> ValidateRequiredKeys()
-> missing required key ensure

BT Service / Task / Decorator
-> CAIKey::Category::KeySpec.KeyName 사용
```

이 형태는 다음 기준을 만족한다.

```text
- key category 네임스페이스 사용감 유지
- key name과 expected type 정의 위치 결합
- 전체 key 등록 목록은 registry에서 별도 관리
- Blackboard API에는 명시적으로 KeyName만 전달
```

### 다른 후보를 사용하지 않은 이유

```text
- 단순 FName vocabulary 유지
  -> key type 정의가 registry에만 남아 CAIKey와 중복 관리된다.

- CAIKey.h 안에 전체 registry까지 포함
  -> CAIKey.h가 key 정의 / 전체 등록 / 검증 책임을 모두 가지게 된다.

- 모든 BT 사용처를 wrapper API로 이전
  -> 이번 PR의 목적보다 변경 범위가 커지고 BT node behavior 검증 부담이 커진다.

- 자유 FName / DataAsset 기반 key profile
  -> 현재 규모에서는 유연성보다 추적성 저하와 검증 부담이 더 크다.
  -> enemy별 key profile이 필요해지는 시점에 별도 확장한다.
```

---

## 예상 수정 대상

```text
Source/Portfolio/AI/Blackboard/CAIKey.h
Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
Source/Portfolio/Controller/CAIController.h
Source/Portfolio/Controller/CAIController.cpp
```

확인 대상:

```text
Source/Portfolio/AI/BehaviorTree/Service/*
Source/Portfolio/AI/BehaviorTree/Task/*
Source/Portfolio/AI/BehaviorTree/Decorator/*
Source/Portfolio/Component/CCombatSignalSourceComponent.cpp
```

---

## 작업 순서 제안

```text
1. CAIKey usage 목록을 key category별로 정리
2. FAIBlackboardKeySpec / EAIBlackboardKeyValueType 정의
3. required key registry 구성
4. ValidateBlackboardKeys 수동 검증 제거
5. CAIKey 사용처를 KeySpec.KeyName 전달 방식으로 변경
6. BT Service / Task / Decorator key type 사용처 정적 확인
7. 빌드 / PIE smoke test
```

---

## 완료 조건

```text
- required blackboard key 목록이 registry에서 확인된다.
- ValidateBlackboardKeys의 수동 bool 나열이 제거된다.
- 검증 실패 시 누락 key 목록과 expected type이 ensure로 확인된다.
- Blackboard API 호출부는 KeySpec이 아니라 KeySpec.KeyName을 전달한다.
- BT node behavior는 변경되지 않는다.
- 빌드와 PIE AI smoke test가 통과한다.
```

초기값 설정과 teardown clear registry화는 같은 주제의 후속 후보로 남긴다. 이번 커밋에서는 required key 계약 검증과 key spec 정의를 먼저 안정화한다.

---

## 후속 분리

이번 작업에서는 AI update interval을 조정하지 않는다.

후속 작업:

```text
perf/ai-update-interval-audit
```

에서 BT Service interval, perception update, engage rebuild interval을 별도로 검토한다.

---
