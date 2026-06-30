# UE5 Portfolio Pull Request

## 제목

**P32: AI Blackboard Key Registry**

## 날짜

**2026.06.30**

## 상태

- [x] 작업 방향 수립
- [ ] 코드 / 문서 반영
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

현재 `CAIKey`는 key name vocabulary 역할을 하지만, required key 검증과 runtime value 초기화 / 정리는 `ACAIController`에서 수동으로 길게 나열되어 있다. 이 구조는 key 추가 또는 삭제 시 누락 위험이 크고, BehaviorTree key 계약을 코드에서 설명하기 어렵다.

핵심 목표는 AI 행동 로직을 바꾸지 않고, blackboard key 계약을 한 곳에서 읽고 검증할 수 있게 만드는 것이다.

---

## 작업 배경

P31에서 `ACAIController` lifecycle cleanup을 정리하면서 blackboard setup / runtime value / behavior tree start 순서가 명확해졌다.

그 과정에서 다음 문제가 남아 있는 것이 확인됐다.

```text
CAIKey.h
-> key name만 정의

ACAIController::ValidateBlackboardKeys
-> required key를 수동 나열

ACAIController::SetInitialBlackboardRuntimeValues
-> 초기 runtime 값을 수동 나열

ACAIController::ClearBlackboardRuntimeValues
-> clear 대상을 수동 나열
```

즉 key를 하나 추가하면 최소 세 곳 이상을 같이 수정해야 한다.

---

## 사전 조회 결과

```text
핵심 key 정의:
Source/Portfolio/AI/Blackboard/CAIKey.h

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

### 1. Blackboard key registry 도입

목표:

```text
- key name
- expected blackboard value type
- required 여부
- initial value 적용 대상 여부
- clear 대상 여부
```

를 한 곳에서 확인할 수 있게 한다.

후보:

```text
Source/Portfolio/AI/Blackboard/CAIKey.h
Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
```

registry가 너무 커지면 별도 파일로 분리한다.

### 2. Required key validation 단일화

현재 `ValidateBlackboardKeys()`는 모든 required key를 수동 변수로 선언하고 다시 `bAllValid`에 합산한다.

변경 방향:

```text
registry의 required key 목록 순회
-> ValidateBlackboardKey 호출
-> 누락 key 로그 또는 ensure 정책 적용
```

### 3. Initial / Clear runtime value 기준 정리

초기값이 필요한 key와 단순 clear만 필요한 key를 구분한다.

```text
SetInitialBlackboardRuntimeValues
-> registry 기반 공통 초기값 적용
-> enemy instance에서 가져와야 하는 tuning 값은 별도 apply helper 유지

ClearBlackboardRuntimeValues
-> registry 기반 clear 대상 순회
```

### 4. BT Service / Task / Decorator 사용처 확인

BT 노드의 blackboard read / write 로직은 이번 PR에서 기능 변경하지 않는다.

확인 기준:

```text
- registry에 없는 key를 직접 사용하지 않는지
- key type과 GetValueAs / SetValueAs API가 일치하는지
- controller 초기화 / clear 대상에서 누락된 key가 없는지
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
registry required key와 ValidateBlackboardKeys 경로 확인
registry initial / clear 대상과 controller runtime value 경로 확인
git diff --check
PortfolioEditor Win64 Development 빌드
PIE AI basic loop smoke test
```

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N15_AI_Blackboard_Key_Registry_Policy_Note.md
Docs/04_Pull_Request/P31_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N13_Component_Lifecycle_Cleanup_Policy_Note.md
```

---
