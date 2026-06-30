# UE5 Portfolio Pull Request

## 제목

**P32: AI Blackboard Key Registry**

## 날짜

**2026.06.30**

## 상태

- [x] 작업 방향 수립
- [x] 코드 / 문서 반영
- [ ] 검증 완료

---

## 브랜치

- `refactor/ai-blackboard-key-registry`

---

## 커밋

```text
TBD
```

---

## 요약

이번 PR은 AI blackboard key 정의 / 검증 / 초기화 / 정리 기준을 registry 중심으로 정리한다.

현재 `CAIKey`는 key category vocabulary 역할을 하지만, required key 검증과 runtime value 초기화 / 정리는 `ACAIController`에서 수동으로 길게 나열되어 있다. 이 구조는 key 추가 또는 삭제 시 누락 위험이 크고, BehaviorTree key 계약을 코드에서 설명하기 어렵다.

핵심 목표는 AI 행동 로직을 바꾸지 않고, blackboard key 계약을 한 곳에서 읽고 검증할 수 있게 만드는 것이다.

---

## 작업 배경

P31에서 `ACAIController` lifecycle cleanup을 정리하면서 blackboard setup / runtime value / behavior tree start 순서가 명확해졌다.

그 과정에서 다음 문제가 남아 있는 것이 확인됐다.

```text
CAIKey.h
-> key name만 정의
-> key type / required 여부는 별도 위치에 있음

ACAIController::ValidateBlackboardKeys
-> required key를 수동 나열

ACAIController::InitializeBlackboardRuntimeValues
-> 초기 runtime 값을 수동 나열

ACAIController::ClearBlackboardRuntimeValues
-> clear 대상을 수동 나열
```

즉 key를 하나 추가하면 최소 세 곳 이상을 같이 수정해야 한다.

---

## 사전 조회 결과

```text
핵심 key 정의:
Source/Portfolio/AI/Blackboard/CAIKeyTypes.h
Source/Portfolio/AI/Blackboard/CAIKeyFactory.h
Source/Portfolio/AI/Blackboard/CAIKey.h
Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h

blackboard setup / validate / initialize / clear:
Source/Portfolio/Controller/CAIController.*

runtime key 사용처:
Source/Portfolio/AI/BehaviorTree/Service/*
Source/Portfolio/AI/BehaviorTree/Task/*
Source/Portfolio/AI/BehaviorTree/Decorator/*
Source/Portfolio/Component/CCombatSignalSourceComponent.cpp

blackboard key 직접 사용 파일:
23개
```

---

## 작업 범위

### 1. Blackboard key spec / registry 도입

목표:

```text
- key name
- expected blackboard value type
- required 여부
- initial value 적용 대상 여부
- clear 대상 여부
```

를 한 곳에서 확인할 수 있게 한다.

변경:

```text
Source/Portfolio/AI/Blackboard/CAIKey.h
-> FAIBlackboardKeySpec 기반 key 정의
-> CAIKeyFactory helper 기반 key spec 생성
-> RuntimeValue key는 possession 초기화에서 제외
-> CAIKey::Category::KeySpec.KeyName 사용

Source/Portfolio/AI/Blackboard/CAIKeyTypes.h
-> EAIBlackboardKeyValueType / EAIBlackboardInitialValuePolicy / FAIBlackboardKeySpec 정의

Source/Portfolio/AI/Blackboard/CAIKeyFactory.h
-> fixed / runtime / owner / custom key spec 생성 helper 분리

Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
-> 전체 key spec 등록
-> required key validation
```

`CAIKey`는 category namespace 사용감을 유지하고, `CAIKeyRegistry`는 전체 key 목록과 검증 흐름을 담당한다.

`CAIKey`를 자유 FName / DataAsset 기반 입력으로 풀지 않은 이유는 별도 결정 문서에 정리했다.

```text
Docs/06_notes/N16_AI_Blackboard_Key_Contract_Decision_Note.md
```

### 2. Required key validation 단일화

현재 `ValidateBlackboardKeys()`는 모든 required key를 수동 변수로 선언하고 다시 `bAllValid`에 합산한다.

변경 방향:

```text
CAIKeyRegistry::GetKeySpecs 순회
-> CAIKeyRegistry::ValidateRequiredKeys 호출
-> 누락 key 이름과 expected type ensure
```

### 3. Initial / Clear runtime value 기준 정리

초기값이 필요한 key와 단순 clear만 필요한 key를 구분한다.

```text
InitializeBlackboardRuntimeValues
-> ApplyInitialBlackboardValues: registry 1회 순회
-> Fixed / FromOwnerLocation / Custom policy 분기
-> Custom key는 pending set에 등록
-> ApplyCustomBlackboardValues에서 처리된 custom key 제거
-> 남은 custom key가 있으면 ensure

ClearBlackboardRuntimeValues
-> 이번 커밋에서는 기존 흐름 유지
-> 후속 작업에서 clear 대상 registry 순회 검토
```

### 4. BT Service / Task / Decorator 사용처 확인

BT 노드의 blackboard read / write 로직은 이번 PR에서 기능 변경하지 않는다.

확인 기준:

```text
- registry에 없는 key를 직접 사용하지 않는지
- key type과 GetValueAs / SetValueAs API가 일치하는지
- controller 초기화 / clear 대상에서 누락된 key가 없는지
- Blackboard API 호출부가 `CAIKey::Category::KeySpec.KeyName` 형태로 정리됐는지
```

---

## 제외 범위

```text
- BehaviorTree asset 재설계
- BT Service / Task / Decorator 행동 로직 변경
- AI update interval 조정
- perception / target selection 정책 변경
- engage assignment 알고리즘 변경
- Enhanced Input migration
```

---

## 검증 계획

```text
rg 기반 CAIKey / blackboard 사용처 전수 확인
registry required key와 ValidateRequiredKeys 경로 확인
CAIKey spec과 Blackboard API KeyName 전달 경로 확인
CAIKeyTypes / CAIKeyFactory / CAIKey / CAIKeyRegistry 책임 분리 확인
initial blackboard value policy / default value 순회 확인
git diff --check
PortfolioEditor Win64 Development 빌드
PIE AI basic loop smoke test
```

현재 확인:

```text
git diff --check 통과
PortfolioEditor Win64 Development 빌드 통과
required key 누락 시 ensureMsgf 경로 적용
initial blackboard value fixed / owner는 registry 순회 적용
custom value는 명시 helper 유지
PIE AI basic loop smoke test 대기
```

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N15_AI_Blackboard_Key_Registry_Policy_Note.md
Docs/06_notes/N16_AI_Blackboard_Key_Contract_Decision_Note.md
Docs/04_Pull_Request/P31_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N13_Component_Lifecycle_Cleanup_Policy_Note.md
```

---
