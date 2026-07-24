# W05 Type Header Organization Work Plan

## 제목

**W05: 구조체 나누기 / Type 헤더 배치 작업 계획**

## 날짜

**2026.07.23**

## 브랜치

```text
refactor/type-header-organization
```

## 상태

- [x] 브랜치 생성
- [x] Type 헤더 라인 수 / UHT 대상 필수 스캔
- [x] include 의존성 필수 스캔
- [x] UHT / Blueprint / asset reference 위험 기준 정리
- [x] 구조체 / 헤더 배치 규칙 문서 작성
- [x] 책임 단위 Type 헤더 권장 분류 검토
- [x] enum / struct 배치 규칙 문서화
- [x] include 배치 규칙 문서화
- [x] `DamageEventId.h` 정책 결정
- [x] 에이전트 기반 작업 순서 검토
- [x] 대규모 Type 파일명 / 패턴 정리
- [x] 입력 signal / world feedback / engage 타입 분리
- [x] `CWeaponStructure.h` 내 계층 identity / rule 분리
- [x] hit / damage / combat result 타입 분리
- [x] combat signal source / target 타입 분리
- [x] action / reaction 저장형 data 타입 분리
- [x] execution / observable overlay 타입 분리
- [x] orchestration / feedback 타입 정리
- [x] 사용처 include 교체 및 umbrella 의존 제거
- [x] 규칙 문서와 작업 계획 문서 역할 분리
- [x] action orchestration candidate 섹션 위치 정리
- [x] AI Type runtime context / patrol state 섹션 정리
- [x] Action / Reaction KeyTypes 분리 계획 고정
- [x] Action / Reaction KeyTypes 실제 분리
- [x] BT service interval preset type 위치 정리
- [x] 최종 빌드 / PIE / 로그 검증
- [ ] PR 문서 작성

---

## 1. 목표

이번 브랜치는 `Source/Portfolio/Type` 아래 공용 Type 헤더를 책임 단위로 세분화하고, 대형 umbrella 헤더였던 `CWeaponStructure.h`의 과결합을 해소한다.

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

`Source/Portfolio/Type` 초기 스캔 결과:

```text
2061 CWeaponStructure.h
 298 CAIStructure.h -> CAITypes.h
 279 CCombatSignalStructure.h -> CCombatSignalTypes.h
 252 CActionOrchestrationStructure.h -> CActionOrchestrationTypes.h
 236 CReactionFeedbackStructure.h -> CReactionFeedbackTypes.h
 140 CWorldSubsystemStructure.h -> CCombatFeedbackTypes.h / CEngageAssignmentTypes.h
  81 CReactionOrchestrationStructure.h -> CReactionOrchestrationTypes.h
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

CActionOrchestrationTypes.h / CReactionOrchestrationTypes.h / CReactionFeedbackTypes.h
-> CWeaponStructure.h를 직접 include하면 Type 내부 분리가 실제 include 경량화로 이어지지 않음

CWorldSubsystemStructure.h
-> engage assignment와 hit feedback request가 섞여 있음

DamageEventId.h
-> FDamageEvent::ClassID 구분용 C++ 내부 ID
-> Blueprint / UPROPERTY / editor 노출 대상이 아니므로 UENUM() 사용 대상 아님
```

`CWeaponStructure.h` 직접 include 사용처는 소스를 합쳐 40개 이상이었다. 최종 목표는 기존 umbrella include를 필요한 Type 헤더 직접 include로 교체하는 것이다.

---

## 3. 최종 목표 분류

최종 분류는 `W05_Type_Header_Organization_Rules.md`의 분류 원칙을 기준으로 한다.

권장 Type 헤더:

```text
CWeaponTypes.h
CActionTypes.h
CActionKeyTypes.h
CActionDataTypes.h
CReactionTypes.h
CReactionKeyTypes.h
CReactionDataTypes.h
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

권장 분류 상세:

```text
CActionTypes.h
-> EActionType, EGuardActionPhase, EActionNotifyCommand, EActionEventType

CActionKeyTypes.h
-> FActionDataKey
-> GetTypeHash(FActionDataKey)
-> GetGuardActionPhaseIndex / ResolveGuardActionPhase

CActionDataTypes.h
-> FActionData, FActionExecutionContext

CReactionTypes.h
-> EReactionType, EReactionNotifyCommand

CReactionKeyTypes.h
-> FReactionDataKey
-> GetTypeHash(FReactionDataKey)

CReactionDataTypes.h
-> FReactionData, FReactionExecutionContext

CActionTypes.h 임시 잔류 / 후속 정리 후보
-> FActionContext는 현재 hit / combat signal에 실리는 action identity snapshot 성격이다.
-> FActionExecutionContext와 다른 역할이며, FActionDataKey와 필드가 중복된다.
-> 단순 대칭 목적의 FReactionContext는 추가하지 않는다.
-> 최종 후보는 FActionContext 제거 후 FActionDataKey 직접 사용이다.
-> USTRUCT / UPROPERTY 변경 리스크가 있으므로 별도 rename / removal pass에서 처리한다.

CActionOrchestrationTypes.h
-> EActionStopReason / EActionFinishReason
-> action intent / request / candidate / deferred / request result / execution result

CReactionOrchestrationTypes.h
-> EReactionStopReason / EReactionFinishReason
-> reaction intent / request / candidate / request result / execution result

CExecutionTypes.h
-> execution decision / relationship / apply mode / domain
-> participant / query / result / intervention directive

CCombatSignalTypes.h
-> FCombatSignalHeader, FCombatSignal 중심
-> generic combat signal pipeline 타입은 코드 품질 정리 직후 구현 예정인 계획 타입 / reserved scaffold로 예외 유지

CAITypes.h
-> perception runtime state / AI runtime context / patrol data / engage context
-> audit / debug / profiling state는 AI gameplay context로 유지하지 않음

CEngageAssignmentTypes.h
-> ECombatRole
-> engage request / assignment runtime state
-> debug / profiling rebuild state는 engage assignment gameplay type으로 유지하지 않음
```

권장 분류에서 제외 / 제거 완료:

```text
EActionStopSource
EReactionStopSource
EAIUpdatePrecision
```

적용 결과:

```text
EActionStopSource / EReactionStopSource
-> StopReason / FinishReason과 책임이 겹치므로 제거했다.
-> orchestration API에서 원인과 결과를 중복 표현하지 않도록 정리했다.

EAIUpdatePrecision
-> 실제 gameplay assignment 타입이 아니라 update scheduling / profiling 정책에 가까운 미사용 타입이므로 제거했다.
```

---

## 4. 계속 정리 후보

아래 항목은 현재 프로젝트 스캔 결과 기준으로 추가 판단이 필요하다.

### 4.1 보류 / 계획 타입 후보

```text
CCombatSignalTypes.h
-> ECombatSignalOutcome
-> ECombatSignalResultType
-> FCombatSignalContext
-> FCombatSignalEvaluation
-> FCombatSignalApplyResult
-> FCombatSignalResult
```

판정:

```text
-> 현재 source / target pipeline에서는 실사용이 없지만, 코드 품질 정리 직후 generic combat signal pipeline 구현 예정이므로 유지한다.
-> 다음 generic combat signal pipeline 작업에서 실제 사용처를 연결한다.
-> 해당 구현 이후에도 사용처가 없으면 제거 후보로 재평가한다.
```

### 4.2 미사용 타입 제거 완료

```text
CActionOrchestrationTypes.h
-> EActionStopSource 제거 완료

CReactionOrchestrationTypes.h
-> EReactionStopSource 제거 완료

CEngageAssignmentTypes.h
-> EAIUpdatePrecision 제거 완료
```

사유:

```text
EActionStopSource / EReactionStopSource
-> StopReason / FinishReason과 책임이 겹쳐 별도 source enum을 유지하지 않는다.

EAIUpdatePrecision
-> gameplay assignment 타입보다 update scheduling / profiling 정책에 가까웠고, 실제 사용처가 없어 제거했다.
```

### 4.3 debug / audit state 이동 완료

```text
CAITypes.h
-> FPerceptionCandidateAuditState 이동 완료
-> FBlackboardEngageLatencyAuditState 이동 완료

CEngageAssignmentTypes.h
-> FEngageAssignmentRebuildDebugState 이동 완료
```

적용 위치:

```text
Core/Debug/FAIPerceptionDebugTypes.h
-> FPerceptionCandidateAuditState
-> FBlackboardEngageLatencyAuditState

Core/Debug/FCombatEngageDebugTypes.h
-> FEngageAssignmentRebuildDebugState
```

사유:

```text
-> gameplay shared Type 헤더에 audit / debug / profiling state가 섞이지 않도록 분리했다.
-> 일반 gameplay include가 진단 전용 타입에 의존하지 않게 했다.
```

### 4.4 배치 / 구성 정리 후보

#### Action / Reaction key / context 구조 정리

```text
현재 Action
-> CActionTypes.h: enum + FActionDataKey + FActionContext
-> CActionDataTypes.h: FActionData + FActionExecutionContext

현재 Reaction
-> CReactionTypes.h: enum
-> CReactionDataTypes.h: FReactionDataKey + FReactionData + FReactionExecutionContext
```

판단:

```text
-> 현재 분리는 의도된 도메인 설계라기보다 include 부담을 줄이다가 굳어진 비대칭으로 본다.
-> FActionDataKey는 가볍기 때문에 CActionTypes.h에 남았고, FReactionDataKey는 FDamageSpecKey 의존 때문에 CReactionTypes.h에 올라가지 못한 것으로 본다.
-> 최종 목표는 Types / KeyTypes / DataTypes / OrchestrationTypes 책임 분리다.
```

권장 최종 구조:

```text
CActionTypes.h
-> EActionType, EGuardActionPhase, EActionNotifyCommand, EActionEventType

CActionKeyTypes.h
-> FActionDataKey
-> GetTypeHash(FActionDataKey)
-> GetGuardActionPhaseIndex / ResolveGuardActionPhase

CActionDataTypes.h
-> FActionData
-> FActionExecutionContext

CReactionTypes.h
-> EReactionType, EReactionNotifyCommand

CReactionKeyTypes.h
-> FReactionDataKey
-> GetTypeHash(FReactionDataKey)

CReactionDataTypes.h
-> FReactionData
-> FReactionExecutionContext
```

`FActionContext` 판단:

```text
-> 실제 역할은 action runtime context가 아니라 hit / combat signal에 실리는 action identity snapshot이다.
-> 현재 필드는 FActionDataKey와 동일하게 ActionType + ActionIndex다.
-> 최종 후보는 FActionContext 제거 후 FActionDataKey 직접 사용이다.
-> hit source metadata가 추가될 명확한 계획이 생기면 FActionHitSourceContext 또는 FCombatSourceActionContext rename을 별도 검토한다.
-> 단순 대칭 목적의 FReactionContext 추가는 금지한다.
```

진행 판단:

```text
-> KeyTypes 실제 분리는 현재 Type header organization 브랜치의 다음 코드 작업으로 진행한다.
-> FActionContext 제거 / rename은 USTRUCT / UPROPERTY 변경이므로 Editor load, Blueprint compile, PIE smoke 검증과 묶는 별도 pass에서 처리한다.
```

```text
CActionFeedbackTypes.h / CReactionFeedbackTypes.h
-> VFX Data -> VFX RuntimeKey / PlaybackKey -> SFX Data -> SFX RuntimeKey / PlaybackKey 순서로 통일
-> Action feedback execution key와 Reaction feedback execution key의 dedupe 기준 통일

Feedback key model 정리

정책:
-> FeedbackMatchKey는 어떤 feedback data를 선택할지 결정한다.
-> FeedbackPlaybackKey는 선택된 feedback data의 실제 playback identity를 기준으로 중복 실행 여부를 결정한다.
-> Playback dedupe는 feedback request identity가 아니라 effect asset + playback condition 기준으로 통일한다.
-> Timing / TriggerKey / ActionType / ReactionType은 matching 단계의 입력이며 playback dedupe key에는 포함하지 않는다.

rename / 구조 후보:
-> FActionFeedbackKey -> FActionFeedbackMatchKey
-> FReactionFeedbackKey -> FReactionFeedbackMatchKey
-> FActionVFXExecutionKey -> FActionVFXPlaybackKey
-> FActionSFXExecutionKey -> FActionSFXPlaybackKey
-> FReactionVFXExecutionKey -> FReactionVFXPlaybackKey
-> FReactionSFXExecutionKey -> FReactionSFXPlaybackKey

구조 변경:
-> Action PlaybackKey에서 ActionFeedbackKey / Timing / TriggerKey 제거 검토
-> Action / Reaction PlaybackKey를 동일한 playback identity 기준으로 통일
-> 이 항목은 단순 rename pass가 아니라 의미 모델 정리 + rename + dedupe 동작 검증으로 처리

CAITypes.h
-> audit state가 gameplay context보다 먼저 나오는 배치 정리
-> EPatrolMode와 FPatrolPointData가 함께 읽히도록 배치 정리

FActionDataKey vs FActionFeedbackKey
FReactionDataKey vs FReactionFeedbackKey
-> 목적이 feedback matching이면 FeedbackMatchKey로 분리 명명
-> DataKey 재사용보다 feedback matching 의미를 우선 확인

CCombatSignalSourceTypes.h / CCombatSignalTargetTypes.h
-> RequestDamage vs RequestedDamage 표기 불일치 정리
```

### 4.5 rename / 구조 변경 보류 후보

```text
FTargetData
-> 설정 Data가 아니라 perception runtime state
-> 후보: FTargetPerceptionState / FPerceptionTargetState

FPatrolPointData
-> 저장 Data라기보다 patrol point runtime snapshot에 가까움
-> 후보: FPatrolPointSnapshot

FDamageImpactInfo
-> Info가 넓고 실제 의미는 hit impact context
-> 후보: FHitImpactContext / FDamageImpactContext

FDamageAmount
-> RequestDamage float 하나만 감싼 wrapper로는 의미가 약함
-> 후보: 제거 또는 FDamageRequestAmount로 존속

FActionCombatSignalCueRequest
-> request가 아니라 notify cue 해석 결과에 가까움
-> 후보: FActionCombatSignalCueResult / FActionCombatSignalCueResolution

FTrailFeedbackData
-> action feedback 파일 안에서 유일하게 Action prefix가 없음
-> 후보: FActionTrailFeedbackData

FAIContext
-> perception / home / alert / engage / reaction / dead가 모두 들어간 aggregate
-> 후보: FAIBlackboardUpdateContext 또는 하위 context 분리
```

보류 사유:

```text
USTRUCT / UENUM BlueprintType rename은 asset / Blueprint serialization 위험이 있다.
rename은 redirect / build / Editor load / Blueprint compile / PIE smoke를 포함하는 별도 rename pass에서 처리한다.
FPatrolPointData rename도 다른 rename 후보들과 함께 별도 pass에서 처리한다.
```

---

## 5. 작업 진행 기록

### 5.1 대규모 Type 파일명 / 패턴 정리

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
- 단순 파일명 규칙 검증
- generated include rename 검증
- include 배치 규칙 적용 검증
- UHT 리스크가 낮은 파일에서 rename 패턴 검증
```

예상 커밋:

```text
refactor(type): rename simple type headers
```

### 5.2 입력 signal / world feedback / engage 타입 분리

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

### 5.3 `CWeaponStructure.h` 내 계층 identity / rule 분리

대상:

```text
CWeaponTypes.h
CActionTypes.h
CReactionTypes.h
execution intervention rule 성격 타입
```

주의:

```text
- FExecutionParticipant를 rule 헤더에 넣으면 cycle 위험이 있음
- MatchesParticipant 같은 구현은 필요하면 cpp로 이동
- FActionData / FReactionData보다 낮은 계층을 먼저 고정
```

예상 커밋:

```text
refactor(type): split combat identity rules
```

### 5.4 hit / damage / combat result 타입 분리

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

### 5.5 combat signal source / target 타입 분리

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

### 5.6 action / reaction 저장형 data 타입 분리

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

### 5.7 execution / observable overlay 타입 분리

대상:

```text
CExecutionTypes.h
CObservableOverlayTypes.h
```

주의:

```text
- ActionData / ReactionData <-> Execution 순환 include 위험
- FExecutionParticipant 전체 정의가 필요한 helper는 cpp 구현으로 이동
- overlay query / snapshot은 overlay 책임으로 분리
```

예상 커밋:

```text
refactor(type): split execution overlay types
```

### 5.8 orchestration / feedback 타입 정리

대상:

```text
CActionOrchestrationTypes.h
CReactionOrchestrationTypes.h
CActionFeedbackTypes.h
CReactionFeedbackTypes.h
```

목적:

```text
- orchestration 헤더가 CWeaponStructure.h를 물지 않게 정리
- action / reaction feedback 저장형 data 파일 형식 통일
```

예상 커밋:

```text
refactor(type): split orchestration feedback types
```

### 5.9 action orchestration candidate 섹션 위치 정리

대상:

```text
CActionOrchestrationTypes.h
-> FActionCandidate
-> FDeferredActionCandidate
```

적용 상태:

```text
완료
```

목적:

```text
- Result 뒤쪽에 있던 candidate 타입을 Request 앞쪽 Candidate 섹션으로 이동
- Reaction orchestration 쪽 candidate 배치와 흐름 통일
- 타입명 / 필드명 / USTRUCT metadata 변경 없음
```

### 5.10 AI Type runtime context / patrol state 섹션 정리

대상:

```text
CAITypes.h
-> EPatrolMode
-> FPatrolPointData
-> FAIContext
-> FEngageContext
```

적용 상태:

```text
완료
```

목적:

```text
- EPatrolMode와 FPatrolPointData를 가까이 배치
- FPatrolPointData는 rename하지 않고 runtime state 섹션 배치만 정리
- 반복되던 Runtime Context 섹션 표기를 하나로 정리
```

### 5.11 사용처 include 교체 및 umbrella 의존 제거

순서:

```text
1. cpp 직접 include 전환
2. UPROPERTY 값 타입이 있는 h 직접 include 전환
3. interface h 최소 Type 헤더 include 전환
4. CWeaponStructure.h compatibility include 제거
5. umbrella 제거 완료 여부 확인
```

예상 커밋:

```text
refactor(type): replace umbrella type includes
```

### 5.12 최종 검증

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

## 6. PR 가능 조건

```text
- 타입명 / enum entry / UPROPERTY 이름 변경 없음
- UHT generated include 규칙 준수
- include 배치 규칙 준수
- CWeaponStructure.h umbrella 의존 제거
- PortfolioEditor Development 빌드 통과
- PIE smoke 또는 미확인 항목 명시
- Blueprint / asset load 관련 구조체 경고 없음
```
