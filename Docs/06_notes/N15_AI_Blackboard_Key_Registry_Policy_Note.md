# N15. AI Blackboard Key Registry Policy Note

## 목적

이 문서는 `refactor/ai-blackboard-key-registry` 작업의 설계 기준을 정리한다.

목표는 BehaviorTree 동작을 바꾸는 것이 아니라, blackboard key 계약을 코드에서 한 곳에 모아 검증 / 초기화 / 정리 누락을 줄이는 것이다.

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

### 1. CAIKey는 vocabulary로 유지한다

BT Service / Task / Decorator에서 이미 `CAIKey::Category::KeyName` 형태를 넓게 사용하고 있으므로, key name namespace는 유지한다.

이번 작업에서 모든 BT 노드 사용처를 wrapper API로 강제 이전하지 않는다.

### 2. registry는 key contract를 담당한다

registry는 다음 정보를 제공한다.

```text
FName KeyName
EBlackboardKeyContractType ValueType
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
CAIKey::State::AIIntentState
-> fixed initial value: Idle

CAIKey::Navigation::HomeLocation
-> owner pawn location 필요

CAIKey::Patrol::bUsePatrol
-> ACEnemy instance 설정 필요
```

### 3. Validation은 registry 순회로 바꾼다

```text
ValidateBlackboardKeys
-> GetRequiredBlackboardKeySpecs()
-> ValidateBlackboardKey 반복
```

검증 실패 시 key name과 expected type을 로그에 남길 수 있게 한다.

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

## 구현 후보

### Option A. CAIKey.h 안에 registry 추가

장점:

```text
- key name과 key contract가 한 파일에 있음
- 작은 변경으로 시작 가능
```

단점:

```text
- CAIKey.h가 길어짐
- type / default value helper가 늘어나면 부담
```

### Option B. CAIKeyRegistry.h 추가

장점:

```text
- CAIKey는 name vocabulary로 유지
- registry / validation / initial / clear 정책을 분리 가능
```

단점:

```text
- 파일이 하나 늘어남
- include 정리가 필요
```

추천:

```text
Option B
```

이유:

```text
CAIKey.h는 기존 BT 노드가 넓게 include하고 있으므로 가볍게 유지하는 편이 낫다.
registry는 controller setup / validation 쪽에서 주로 사용하므로 별도 파일이 책임 분리에 맞다.
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
2. FAIBlackboardKeySpec / EAIBlackboardKeyValueType 후보 정의
3. required key registry 구성
4. ValidateBlackboardKeys를 registry 순회로 변경
5. clear 대상 registry 구성
6. ClearBlackboardRuntimeValues를 registry 순회로 변경
7. fixed initial value helper 분리
8. owner / enemy 기반 initial value helper 분리
9. BT Service / Task / Decorator key type 사용처 정적 확인
10. 빌드 / PIE smoke test
```

---

## 완료 조건

```text
- required blackboard key 목록이 registry에서 확인된다.
- ValidateBlackboardKeys의 수동 bool 나열이 제거된다.
- ClearBlackboardRuntimeValues의 수동 ClearValue 나열이 줄어든다.
- initial runtime value 설정이 fixed value와 owner / enemy 기반 value로 구분된다.
- BT node behavior는 변경되지 않는다.
- 빌드와 PIE AI smoke test가 통과한다.
```

---

## 후속 분리

이번 작업에서는 AI update interval을 조정하지 않는다.

후속 작업:

```text
perf/ai-update-interval-audit
```

에서 BT Service interval, perception update, engage rebuild interval을 별도로 검토한다.

---
