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

기존 분산 상태:

```text
CAIKey.h
-> key name 정의

ACAIController::ValidateBlackboardKeys
-> required key 수동 검증

ACAIController::InitializeBlackboardValues
-> initial runtime value 수동 설정

ACAIController::ClearBlackboardValues
-> teardown clear 대상 수동 나열

BT Service / Task / Decorator
-> CAIKey 직접 read / write
```

---

## 문제

### 1. key 추가 / 삭제 시 수정 지점이 많다

기존에는 key를 하나 추가하면 다음 위치를 모두 맞춰야 했다.

```text
CAIKey.h
ValidateBlackboardKeys
InitializeBlackboardValues
ClearBlackboardValues
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

`CAIKeyTypes.h`는 key spec의 공통 타입을 정의하고, `CAIKeyFactory.h`는 spec 생성 helper를 제공한다.

`CAIKey.h`는 개별 key vocabulary와 key별 계약 정보를 정의하고, `CAIKeyRegistry.h`는 전체 key 목록 등록과 검증 흐름을 담당한다.

key spec은 다음 정보를 제공한다.

```text
FName KeyName
EAIBlackboardKeyValueType ValueType
EAIBlackboardInitialValuePolicy InitialValuePolicy
fixed default value fields
bool bRequired
bool bClearOnRuntimeTeardown
```

초기값 적용 방식은 key 성격에 따라 나눈다.

```text
공통 fixed initial value
-> FAIBlackboardKeySpec의 InitialValuePolicy / default value 기반으로 registry 순회 처리

owner 위치 기반 value
-> FromOwnerLocation policy 기반으로 registry 순회 처리

runtime update value
-> RuntimeValue factory로 정의하고 possession 초기화에서는 값을 쓰지 않음

custom value
-> custom helper에서 별도 적용 유지
```

예:

```text
CAIKey::State::AIIntentState.KeyName
-> fixed initial value: Idle

CAIKey::Navigation::HomeLocation.KeyName
-> owner pawn location 필요

CAIKey::Patrol::bUsePatrol.KeyName
-> custom value 적용 필요
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
InitializeBlackboardValues
-> CAIBlackboardValueHelper::InitializeValues: registry 1회 순회
-> Fixed / FromOwnerLocation / Custom policy 분기
-> Custom key는 pending set에 등록
-> InitializeCustomBlackboardValues에서 custom value 명시 적용
-> 처리된 custom key는 pending set에서 제거
-> CAIBlackboardValueHelper::ValidateCustomKeysApplied로 누락 검증
```

고정 초기값과 owner 위치 기반 값은 key spec의 policy로 자동 적용한다.

Custom value는 key마다 source가 다르므로 controller에서 명시 적용한다. 이 부분까지 함수 포인터 / lambda / variant로 자동화하면 key contract가 gameplay object access까지 알게 되므로 이번 범위에서는 제외한다.

`Custom` policy는 registry가 자동 적용하지 않는 domain-specific 초기값을 의미한다. 현재 구현에서는 `ACAIController::InitializeCustomBlackboardValues`가 custom value를 명시적으로 적용한다.

Custom key는 조용히 무시하지 않는다. Registry 순회 중 pending set에 등록하고, domain-specific 적용이 끝난 뒤에도 남아 있으면 `CAIBlackboardValueHelper`의 ensure로 누락을 드러낸다.

`RuntimeValue`는 key name / type / required / clear 계약은 필요하지만 possession 초기화 시점에는 의미 있는 값이 없는 key에 사용한다. 예를 들어 `LastSeenTime`, `LastKnownLocation`, `DistanceToTarget`, `DistanceToHome`은 perception / BT service update에서 의미 있는 runtime 값으로 채워진다.

### 5. Clear는 registry 기반으로 단순화한다

```text
ClearBlackboardValues
-> CAIBlackboardValueHelper::ClearValues 호출
-> CAIKeyRegistry::GetKeySpecs 순회
-> bClearOnRuntimeTeardown 대상만 ClearValue 반복
```

단, `DeadState`처럼 clear보다 Alive reset이 의미적으로 맞는 key는 registry에서 clear 대상이 아닌 reset 대상으로 분류할 수 있다.

---

## 구현 결정

### 선택한 형태

```text
CAIKeyTypes.h
-> EAIBlackboardKeyValueType
-> EAIBlackboardInitialValuePolicy
-> FAIBlackboardKeySpec

CAIKeyFactory.h
-> fixed / runtime / owner / custom key spec factory helper

CAIKey.h
-> CAIKey::Category::KeySpec

CAIKeyRegistry.h
-> GetKeySpecs()
-> ValidateRequiredKeys()
-> missing required key ensure

CAIBlackboardValueHelper.h
-> InitializeValues()
-> ApplyFixedValue()
-> ApplyOwnerLocationValue()
-> ApplyCustom*()
-> ValidateCustomKeysApplied()
-> ClearValues()

BT Service / Task / Decorator
-> CAIKey::Category::KeySpec.KeyName 사용
```

이 형태는 다음 기준을 만족한다.

```text
- key category 네임스페이스 사용감 유지
- key name / expected type / initial policy / fixed default 정의 위치 결합
- 전체 key 등록 목록은 registry에서 별도 관리
- Blackboard API에는 명시적으로 KeyName만 전달
- fixed / owner initial value와 clear runtime value는 helper에서 registry 순회로 적용
- custom value는 controller가 source를 읽고 helper apply API로 적용
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
Source/Portfolio/AI/Blackboard/CAIKeyTypes.h
Source/Portfolio/AI/Blackboard/CAIKeyFactory.h
Source/Portfolio/AI/Blackboard/CAIKey.h
Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
Source/Portfolio/AI/Blackboard/CAIBlackboardValueHelper.h
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
7. initial blackboard value helper 분리
8. fixed / owner initial value를 registry 순회로 변경
9. 빌드 / PIE smoke test
```

---

## 완료 조건

```text
- required blackboard key 목록이 registry에서 확인된다.
- ValidateBlackboardKeys의 수동 bool 나열이 제거된다.
- 검증 실패 시 누락 key 목록과 expected type이 ensure로 확인된다.
- Blackboard API 호출부는 KeySpec이 아니라 KeySpec.KeyName을 전달한다.
- initial blackboard value 설정이 fixed / owner / custom 기반 value로 구분된다.
- fixed / owner initial value는 `CAIBlackboardValueHelper`에서 registry 순회로 적용된다.
- custom value source 적용은 controller에 남고, blackboard write / pending 검증은 `CAIBlackboardValueHelper`가 처리한다.
- clear runtime value는 `bClearOnRuntimeTeardown` 기준으로 registry 순회 적용된다.
- BT node behavior는 변경되지 않는다.
- 빌드와 PIE AI smoke test가 통과한다.
```

## 후속 작업

이번 작업에서는 AI update interval을 조정하지 않는다.

후속 작업:

```text
perf/ai-update-interval-audit
```

에서 BT Service interval, perception update, engage rebuild interval을 별도로 검토한다.

---
