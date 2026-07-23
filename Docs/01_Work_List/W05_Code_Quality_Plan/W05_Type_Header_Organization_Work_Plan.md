# W05 Type Header Organization Work Plan

## 제목

**W05: 구조체 나누기 / 헤더 배치 작업 계획**

## 날짜

**2026.07.23**

## 브랜치

```text
refactor/type-header-organization
```

## 상태

- [x] 브랜치 생성
- [x] Type 헤더 라인 수 / UHT 타입 전수 스캔
- [x] include 의존성 전수 스캔
- [x] UHT / Blueprint / asset reference 위험 기준 정리
- [x] 구조체 / 헤더 배치 규칙 문서 작성
- [x] 책임 단위 Type 헤더 권장 분류 재검토
- [x] enum / struct 배치 규칙 문서화
- [x] include 배치 규칙 문서화
- [x] `DamageEventId.h` 정책 결정
- [x] 에이전트 기반 작업 순서 재검토
- [x] 저위험 Type 파일명 / 패턴 정리
- [x] 독립 signal / world feedback / engage 타입 분리
- [x] `CWeaponStructure.h` 낮은 계층 identity / rule 분리
- [ ] hit / damage / combat result 타입 분리
- [ ] combat signal source / target 타입 분리
- [ ] action / reaction 저장형 data 타입 분리
- [ ] execution / observable overlay 타입 분리
- [ ] orchestration / feedback 타입 정리
- [ ] 사용처 include 교체 및 umbrella 의존 제거
- [ ] 최종 빌드 / PIE / 로그 검증
- [ ] PR 문서 작성

---

## 1. 목표

이번 브랜치는 `Source/Portfolio/Type` 아래 공용 타입 헤더를 책임 단위로 재분류하고, 대형 umbrella 헤더인 `CWeaponStructure.h`의 과결합을 해소한다.

이번 작업의 핵심은 타입 rename이 아니라 헤더 책임 재배치다.

고정 조건:

```text
- 타입명 유지
- USTRUCT / UENUM 이름 유지
- UPROPERTY 이름 유지
- enum entry 이름과 값 유지
- BlueprintType 여부 유지
```

파일명과 include 구조는 변경할 수 있다.

---

## 2. 현재 스캔 요약

`Source/Portfolio/Type` 헤더 라인 수:

```text
2061 CWeaponStructure.h
 298 CAIStructure.h
 279 CCombatSignalStructure.h -> CCombatSignalTypes.h
 252 CActionOrchestrationStructure.h
 236 CReactionFeedbackStructure.h
 140 CWorldSubsystemStructure.h -> CCombatFeedbackTypes.h / CEngageAssignmentTypes.h
  81 CReactionOrchestrationStructure.h
  30 CStateStructure.h
  28 CMovementStructure.h
  28 CCharacterComponentReferenceStructure.h
  20 CHealthStructure.h
   9 DamageEventId.h
```

주요 문제:

```text
CWeaponStructure.h
-> Weapon / Action / Reaction / Combat / Execution / Overlay / Feedback 타입이 한 파일에 섞여 있음
-> CStateStructure.h, CHealthStructure.h, Engine/DamageEvents.h까지 직접 include
-> action만 필요한 파일도 health/state/damage 변경에 재컴파일될 수 있음

CActionOrchestrationStructure.h / CReactionOrchestrationStructure.h / CReactionFeedbackStructure.h
-> CWeaponStructure.h를 직접 include해 Type 내부 분리가 실제 include 경량화로 이어지지 않음

CWorldSubsystemStructure.h
-> engage assignment와 hit feedback request가 섞여 있음

DamageEventId.h
-> FDamageEvent::ClassID 구분용 C++ 내부 ID
-> Blueprint / UPROPERTY / editor 노출 대상이 아니므로 UENUM() 사용 안 함
```

`CWeaponStructure.h` 직접 include는 헤더와 소스를 합쳐 40개 이상이다. 따라서 최종 목표는 기존 umbrella include를 필요한 Type 헤더 직접 include로 교체하는 것이다.

---

## 3. 최종 목표 분류

최종 분류는 `W05_Type_Header_Organization_Rules.md`를 기준으로 한다.

핵심 분류:

```text
CWeaponTypes.h
CActionTypes.h
CReactionTypes.h
CActionOrchestrationTypes.h
CReactionOrchestrationTypes.h
CExecutionTypes.h
CObservableOverlayTypes.h
CCombatHitTypes.h
CCombatDamageTypes.h
CCombatSignalTypes.h
CCombatSignalSourceTypes.h
CCombatSignalTargetTypes.h
CCombatResultTypes.h
CActionFeedbackTypes.h
CReactionFeedbackTypes.h
CCombatFeedbackTypes.h
CAITypes.h
CEngageAssignmentTypes.h
CHealthTypes.h
CMovementTypes.h
CStateTypes.h
CCharacterComponentReferenceTypes.h
```

이행 중 compatibility header를 잠깐 둘 수는 있지만, 최종 상태에서는 `CWeaponStructure.h`를 새 코드의 umbrella include로 사용하지 않는다.

---

## 4. 작업 순서

### 4.1 저위험 Type 파일명 / 패턴 정리

대상:

```text
CHealthStructure.h -> CHealthTypes.h
CStateStructure.h -> CStateTypes.h
CMovementStructure.h -> CMovementTypes.h
CCharacterComponentReferenceStructure.h -> CCharacterComponentReferenceTypes.h
```

적용 상태:

```text
완료
```

목적:

```text
- 새 파일명 규칙 검증
- generated include rename 검증
- include 배치 규칙 적용 검증
- UHT 리스크가 낮은 파일에서 rename 패턴 검증
```

예상 커밋:

```text
refactor(type): rename simple type headers
```

### 4.2 독립 signal / world feedback / engage 타입 분리

대상:

```text
CCombatSignalStructure.h -> CCombatSignalTypes.h
CWorldSubsystemStructure.h -> CCombatFeedbackTypes.h
CWorldSubsystemStructure.h -> CEngageAssignmentTypes.h
```

적용 상태:

```text
완료
```

목적:

```text
- generic combat signal과 source/target damage pipeline 구분
- feedback request를 world subsystem 책임에서 분리
- engage assignment 타입을 AI / combat engage 책임으로 분리
```

예상 커밋:

```text
refactor(type): split signal feedback engage types
```

### 4.3 `CWeaponStructure.h` 낮은 계층 identity / rule 분리

대상:

```text
CWeaponTypes.h
CActionTypes.h
CReactionTypes.h
execution intervention rule 성격 타입
```

주의:

```text
- FExecutionParticipant를 rule 헤더에 넣으면 cycle 위험이 큼
- MatchesParticipant 같은 구현은 필요하면 cpp로 내림
- FActionData / FReactionData보다 낮은 계층을 먼저 고정
```

예상 커밋:

```text
refactor(type): split combat identity rules
```

### 4.4 hit / damage / combat result 타입 분리

대상:

```text
CCombatHitTypes.h
CCombatDamageTypes.h
CCombatResultTypes.h
```

목적:

```text
- weapon overlap에서 만들어지는 hit evidence 분리
- damage spec / amount / damage event 분리
- target 이후 dispatch되는 combat result packet 분리
```

예상 커밋:

```text
refactor(type): split combat hit damage result types
```

### 4.5 combat signal source / target 타입 분리

대상:

```text
CCombatSignalSourceTypes.h
CCombatSignalTargetTypes.h
```

목적:

```text
- source component의 hit 검증 / damage spec resolve 결과 분리
- target component의 damage 수신 / 방어 / health commit 결과 분리
```

예상 커밋:

```text
refactor(type): split combat signal payload types
```

### 4.6 action / reaction 저장형 data 타입 분리

대상:

```text
FActionDataKey
FActionData
FActionExecutionContext
FReactionDataKey
FReactionData
FReactionExecutionContext
```

주의:

```text
- EditAnywhere 저장 데이터라 UHT / asset 리스크가 높음
- UPROPERTY 값 타입은 forward declaration 금지
- damage / rule 계층이 먼저 안정화된 뒤 진행
```

예상 커밋:

```text
refactor(type): split action reaction data types
```

### 4.7 execution / observable overlay 타입 분리

대상:

```text
CExecutionTypes.h
CObservableOverlayTypes.h
```

주의:

```text
- ActionData / ReactionData <-> Execution 순환 include 위험
- FExecutionParticipant 전체 정의가 필요한 helper는 cpp 구현으로 내림
- overlay query / snapshot은 overlay 책임으로 분리
```

예상 커밋:

```text
refactor(type): split execution overlay types
```

### 4.8 orchestration / feedback 타입 정리

대상:

```text
CActionOrchestrationTypes.h
CReactionOrchestrationTypes.h
CActionFeedbackTypes.h
CReactionFeedbackTypes.h
```

목적:

```text
- orchestration 헤더가 CWeaponStructure.h를 끌지 않게 정리
- action / reaction feedback 저장형 data 파일 형식 통일
```

예상 커밋:

```text
refactor(type): split orchestration feedback types
```

### 4.9 사용처 include 교체 및 umbrella 의존 제거

순서:

```text
1. cpp 직접 include 전환
2. UPROPERTY 값 타입이 있는 h 직접 include 전환
3. interface h 최소 Type 헤더 include 전환
4. CWeaponStructure.h compatibility-only 상태 확인
5. umbrella 제거 가능 여부 판단
```

예상 커밋:

```text
refactor(type): replace umbrella type includes
```

### 4.10 최종 검증

검증:

```text
git diff --check
PortfolioEditor Development build
Editor load
Blueprint compile
PIE smoke
Unknown structure / Struct type mismatch / Failed to load /Script/Portfolio 로그 없음
```

우선 확인 에셋:

```text
BP_CPlayer
BP_CEnemy
BP_AIPerf_*
ABP_Character
Combat / dodge montage notify 값
```

---

## 5. PR 가능 조건

```text
- 타입명 / enum entry / UPROPERTY 이름 변경 없음
- UHT generated include 규칙 준수
- include 배치 규칙 준수
- CWeaponStructure.h umbrella 의존 제거 또는 명확한 compatibility 용도만 유지
- PortfolioEditor Development 빌드 통과
- PIE smoke 또는 미확인 항목 명시
- Blueprint / asset load 관련 구조체 경고 없음
```
