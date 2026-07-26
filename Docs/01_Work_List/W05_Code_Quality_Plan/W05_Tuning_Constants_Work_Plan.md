# W05 Tuning Constants Work Plan

## 제목

**W05: 상수 / 튜닝 데이터 정리 작업 계획**

## 날짜

**2026.07.25**

## 상태

- [x] Source/Portfolio 전수조사
- [x] 내부 규칙값 / 튜닝 데이터 / 유지 / 외부 계약값 분류
- [ ] 상수 정리형 후보 적용
- [ ] 튜닝 데이터 소유권 후보 확정
- [ ] build 검증
- [ ] PIE smoke 검증

---

## 1. 목적

이 문서는 `refactor/tuning-constants-cleanup` 작업에서 발견된 실제 프로젝트 후보와 처리 판정을 관리한다.

일반 규칙은 `W05_Tuning_Constants_Rules.md`를 따른다. 이 문서는 현재 코드 기준의 파일 / 값 / 후보 목록을 기록한다.

---

## 2. 전수조사 기준

조사 범위:

```text
Source/Portfolio/**/*.h
Source/Portfolio/**/*.cpp
Source/Portfolio/**/*.cs
```

확인 축:

```text
-> numeric literal
-> CVar default / mode / clamp
-> constructor setup value
-> UPROPERTY default
-> sentinel / invalid value
-> index / mode / closed choice
-> TEXT / FName string contract
```

분류:

```text
A. 상수 정리형
B. 튜닝 데이터 소유권 후보
C. 유지 권장 literal
D. 위험 / 외부 계약값
```

---

## 3. 상수 정리형 후보

값 변경 없이 enum / helper / constexpr / 기존 UE sentinel로 의미를 고정한다.

### 3.1 Guard phase index

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/Type/CActionKeyTypes.h
-> Guard phase index 1 / 2 / 3 / 4 / 5
-> GetGuardActionPhaseIndex
-> ResolveGuardActionPhase

Source/Portfolio/Notify/CAnimNotify_SwitchToGuard.cpp
-> TriggerActionIndex = 1

Source/Portfolio/Notify/CAnimNotify_AllowGuardStart.cpp
-> TriggerActionIndex = 2
```

판정:

```text
-> EGuardActionPhase 중심으로 의미를 표현한다.
-> GetGuardActionPhaseIndex(EGuardActionPhase)에서 int32 ActionIndex로 변환한다.
-> Notify 기본값은 raw 1 / 2 대신 GetGuardActionPhaseIndex(EGuardActionPhase::In / Out)를 사용한다.
-> GuardActionPhaseIndex namespace를 외부 API처럼 노출하지 않는다.
```

### 3.2 Investigate index sentinel

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_EndInvestigate.cpp
-> InvestigateIndex = -1
```

판정:

```text
-> 새 constexpr를 만들지 않는다.
-> 기존 UE / project sentinel인 INDEX_NONE으로 맞춘다.
```

### 3.3 Debug screen print duration

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/Core/Debug/FLog.h
-> InDuration = 10.f 반복
```

판정:

```text
-> FLog 소유 static constexpr로 DefaultScreenPrintDuration을 둔다.
-> 기본 인자는 해당 상수를 사용한다.
```

### 3.4 Montage stop blend out

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/Action/CAction.h
Source/Portfolio/Reaction/CReaction.h
Source/Portfolio/Reaction/CReaction.cpp
-> StopMontage blend out 0.1f
```

판정:

```text
-> Action / Reaction 공통 execution 기본값이다.
-> 공용 execution constants 위치를 정한 뒤 DefaultMontageStopBlendOutTime으로 둔다.
-> 함수 signature 의미는 유지하고 값만 named constant로 교체한다.
```

### 3.5 CombatEngage rebuild / warmup sentinel

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.h
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp
-> AssignmentWarmupStartTime = -1.f
-> AssignmentWarmupStartTime < 0.f / >= 0.f
-> AssignmentRebuildId == 1
```

판정:

```text
-> UnsetAssignmentWarmupStartTime constexpr 후보
-> InitialAssignmentRebuildId constexpr 후보
-> FirstAssignmentRebuildId constexpr 후보
```

### 3.6 RuntimeLOD / BT mode

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/AI/RuntimeLOD/CAIStateRuntimeLODPolicy.cpp
-> State policy mode 0 / 1

Source/Portfolio/AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.cpp
-> Movement mode 0 / 1 / 2

Source/Portfolio/AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.cpp
-> Animation mode 0 / 1

Source/Portfolio/Character/Enemy/CEnemy.cpp
-> Enemy mesh mode 0 / 1
-> Enemy actor tick mode 0 / 1

Source/Portfolio/AI/BehaviorTree/Service/CBTServiceIntervalHelper.cpp
-> BT interval mode 0 / 1 / 2
```

판정:

```text
-> CVar는 int32 외부 계약으로 유지한다.
-> CVar 이름과 숫자값은 변경하지 않는다.
-> 내부 해석은 enum class 또는 helper로 감싼다.
-> clamp min / max도 mode enum 기준으로 명명한다.
```

### 3.7 Guard damage mitigation

상태:

```text
-> 완료
```

대상:

```text
Source/Portfolio/Component/CCombatSignalTargetComponent.cpp
-> guard mitigation multiplier 0.5f
```

판정:

```text
-> 1차 적용 완료: guard damage multiplier 소유권을 CDefenseComponent로 이동.
-> FDefenseGuardTuning은 Type/CDefenseTuningTypes.h로 분리한다.
-> UCDefenseComponent는 FDefenseGuardTuning GuardTuning을 직접 소유한다.
-> 현재는 방어 튜닝 묶음 단위가 Guard 하나뿐이므로 FDefenseTuning aggregate는 만들지 않는다.
-> Parry tuning이 실제로 생기면 FDefenseParryTuning을 같은 Type 헤더에 추가하고 UCDefenseComponent에 ParryTuning으로 병렬 배치한다.
-> FDefenseTuning aggregate는 Guard / Parry 등 여러 defense tuning을 하나의 preset / DataAsset / runtime config 단위로 다룰 필요가 생길 때만 검토한다.
-> Dodge는 현재 Action 소유이므로 defense tuning에 미리 포함하지 않는다.
-> CCombatSignalTargetComponent는 target damage 계산 중 DefenseComp에서 GetGuardDamageTakenMultiplier()를 조회한다.
-> 값 이름은 mitigation 결과보다 실제 의미가 분명한 GuardDamageTakenMultiplier로 둔다.
-> 기본값은 기존 0.5f와 동일하게 유지한다.
```

---

## 4. 튜닝 데이터 소유권 후보

이 그룹은 단순 constexpr보다 값의 소유권을 정해야 한다. 이번 문서에서는 후보로 기록하고, 적용 시 UPROPERTY / config USTRUCT / DataAsset 중 선택한다.

### 4.1 Character setup

대상:

```text
Source/Portfolio/Character/Player/CPlayer.cpp
-> capsule radius / half height 40.f / 90.f
-> mesh relative location / rotation
-> walk speed 600.f
-> spring arm relative location
-> spring arm target length 300.f
-> camera relative location

Source/Portfolio/Character/Enemy/CEnemy.cpp
-> capsule radius / half height 40.f / 90.f
-> mesh relative location / rotation
-> walk speed 600.f
```

판정:

```text
-> UPROPERTY(EditDefaultsOnly) 후보.
-> capsule / mesh / movement / camera 값은 config USTRUCT 후보.
-> Player / Enemy / NPC archetype 공유가 필요하면 CharacterSetup DataAsset 후보.
```

적용 상태:

```text
-> 1차 적용 완료: CPlayer / CEnemy 생성자 literal을 UPROPERTY(EditDefaultsOnly) 기본값으로 이동.
-> 공통 setup 값은 CCharacterSetupTypes.h의 FCharacterCapsuleSetup / FCharacterMeshSetup / FCharacterMovementSetup으로 묶음.
-> Player 전용 camera 값은 FPlayerCameraSetup으로 묶음.
-> ApplyCharacterSetup()을 constructor / OnConstruction()에서 호출해 native 기본값과 editor default override 적용 지점을 맞춤.
-> 값 변경 없음.
-> config USTRUCT / CharacterSetup DataAsset 전환은 asset / archetype 공유 기준을 정한 뒤 후속 작업에서 검토.
```

### 4.2 AI perception / memory

대상:

```text
Source/Portfolio/Controller/CAIController.cpp
-> SightRadius = 500.f
-> LoseSightRadius = 600.f
-> PeripheralVisionAngleDegrees = 45.f
-> SetMaxAge(2.f)

Source/Portfolio/Controller/CAIController.h
-> TargetMemoryTimeout = 3.0f
```

판정:

```text
-> AI perception config 후보.
-> enemy archetype별 차이가 필요하면 DataAsset 후보.
```

적용 상태:

```text
-> 1차 적용 완료: CAIController sight / memory tuning 값을 FAIControllerPerceptionSetup으로 묶음.
-> SightRadius / LoseSightRadius / PeripheralVisionAngleDegrees / MaxAge / TargetMemoryTimeout 값 변경 없음.
-> detection affiliation 기본값도 같은 setup 구조체에 포함.
-> constructor에서 SightConfig를 기본 등록해 UE AIPerception listener 등록 흐름을 유지한다.
-> BeginPlay에서 ConfigureSightConfig()를 다시 호출해 PerceptionSetup 값을 runtime 적용한다.
-> ConfigureSightConfig()에서 ConfigureSense와 SetDominantSense를 함께 수행한다.
-> AI perception DataAsset 전환은 enemy archetype별 공유 기준을 정한 뒤 후속 작업에서 검토.
```

소유권 기준:

```text
-> PerceptionSetup은 프로젝트가 정의한 AI perception tuning source of truth다.
-> AIPerceptionComponent의 SensesConfig 배열은 UE listener 등록을 위한 engine-facing mirror다.
-> SensesConfig는 Details에 보일 수 있지만 직접 편집 기준으로 보지 않는다.
-> 기존 Blueprint SensesConfig 값은 PerceptionSetup으로 migration한 뒤 더 이상 직접 수정하지 않는다.
```

### 4.3 AI behavior tuning

대상:

```text
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.h
-> MovableRange = 1000.f

Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartRevive.h
-> ReviveHP = 30.f
```

판정:

```text
-> BT node UPROPERTY로 유지 가능.
-> 여러 enemy archetype이 공유하면 AI tuning config / DataAsset 후보.
```

적용 상태:

```text
-> 코드 변경 없음: MovableRange / ReviveHP는 이미 BT node-local UPROPERTY로 소유권이 잡혀 있음.
-> 값 / 필드명 / 타입 / category 변경 없음.
-> 구조체화 / DataAsset 전환은 BT asset override 리스크가 있으므로 공유 필요성이 생길 때 별도 검토.
```

### 4.4 CombatEngage tuning

대상:

```text
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.h
-> RebuildInterval = 0.1f
-> AssignmentLeaseDuration = 0.5f

Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp
-> warmup time CVar default 0.0f
-> engage cap CVar default 2
-> alert cap CVar default 6
```

판정:

```text
-> subsystem runtime tuning 후보.
-> CVar 이름 / 값 계약은 유지한다.
-> config USTRUCT 또는 project settings / DataAsset 후보는 별도 검토한다.
```

적용 상태:

```text
-> 1차 적용 완료: RebuildInterval / AssignmentLeaseDuration을 FEngageAssignmentTuning으로 묶음.
-> 값 변경 없음.
-> subsystem은 AssignmentTuning.RebuildInterval / AssignmentTuning.LeaseDuration을 참조.
-> warmup time / engage cap / alert cap CVar 이름과 기본값은 외부 조정 계약으로 보고 변경하지 않음.
-> project settings / DataAsset 전환은 subsystem 설정 소유권을 정한 뒤 후속 작업에서 검토.
```

### 4.5 Feedback tuning

대상:

```text
Source/Portfolio/Component/CHitFeedbackComponent.h
-> HitStopDuration = 0.04f
-> HitStopDilation = 0.05f
-> CameraShakeBaseScale = 1.f

Source/Portfolio/Component/CPlayerFeedbackComponent.h
-> LocalTargetShakeScale = 1.0f
-> LocalSourceShakeScale = 0.5f

Source/Portfolio/Type/CCombatFeedbackTypes.h
-> HitStopDuration = 0.04f
-> HitStopDilation = 0.05f
-> CameraShakeBaseScale = 1.f
```

판정:

```text
-> feedback component 기본값과 request struct 기본값의 소유권 중복을 검토한다.
-> hit stop / camera shake는 feedback tuning config 후보.
-> 여러 feedback preset이 필요하면 DataAsset 후보.
```

적용 상태:

```text
-> 1차 적용 완료: component 기본값은 FHitStopFeedbackTuning / FHitCameraShakeFeedbackTuning / FPlayerCameraShakeFeedbackTuning으로 묶음.
-> FHitStopRequest / FCameraShakeRequest는 runtime request 타입으로 유지.
-> 값 변경 없음.
-> request struct 기본값은 direct request 생성 시 fallback 기본값으로 유지.
-> feedback preset DataAsset 전환은 preset 공유 기준을 정한 뒤 후속 작업에서 검토.
-> legacy HitStop / CameraShake field는 asset 저장 이후 제거 완료.
```

asset migration 대상:

```text
Content/01_Character/01_Player/BP_CPlayer.uasset
Content/01_Character/02_Enemy/BP_CEnemy.uasset
```

처리 순서:

```text
1. 코드에서 legacy field -> FHitStopFeedbackTuning / FHitCameraShakeFeedbackTuning PostLoad migration을 추가했다.
2. Editor에서 대상 Blueprint asset을 열고 저장했다.
3. 저장 검증 이후 legacy field와 PostLoad migration code를 제거했다.
```

### 4.6 Patrol editor visualization

대상:

```text
Source/Portfolio/AI/Patrol/CPatrolPoint.cpp
-> TextRender size 50.f
-> TextRender height 200.f
-> TextRender yaw 180.f
```

판정:

```text
-> gameplay tuning보다 editor visualization default에 가깝다.
-> local constexpr 또는 patrol editor config 후보.
-> DataAsset 우선순위는 낮다.
```

적용 상태:

```text
-> 1차 적용 완료: TextRender size / height / facing yaw는 CPatrolPoint.cpp local constexpr로 이름 부여.
-> 값 변경 없음.
-> ExtraWaitTime / FaceYaw / PointTag 같은 patrol gameplay 값은 기존 UPROPERTY 소유권 유지.
-> DataAsset 전환 대상 아님.
```

---

## 5. 유지 권장 literal

다음은 전수조사에서 발견되지만 상수 정리 대상으로 보지 않는다.

```text
-> 초기화 / reset용 0, 0.f
-> loop index 0 / ++i
-> count 비교 / count reset
-> HP / damage / heal guard의 0.f
-> hash seed uint32 H = 0
-> pure virtual = 0
-> enum None = 0 / Idle = 0
-> INDEX_NONE / NAME_None / INT_MAX / KINDA_SMALL_NUMBER
-> FVector::ZeroVector / FVector::OneVector
-> ActionIndex + 1 같은 명확한 sequence arithmetic
-> angleDeg *= -1.f 같은 방향 부호 반전
```

---

## 6. 위험 / 외부 계약값

다음 값은 상수화보다 호환성 유지가 우선이다.

```text
-> CreateDefaultSubobject(TEXT(...)) 이름
-> input binding 이름
-> CVar 이름 문자열
-> Blackboard key 이름
-> debug / audit / log taxonomy 문자열
-> DamageEventId numeric id
-> gameplay tag / asset path
```

판정:

```text
-> 이름 / 값 변경 금지.
-> CVar mode는 외부 숫자값을 유지하고 내부 해석만 정리한다.
-> 문자열 중앙화는 별도 logging / taxonomy cleanup에서만 검토한다.
```

---

## 6.1 최종 소스 전수조사 후속 처리

에이전트 교차검증:

```text
-> Component / Character 범위 확인
-> AI / Controller / System 범위 확인
-> Action / Reaction / Notify / Type / Core 범위 확인
```

바로 처리한 항목:

```text
-> CAIKey::Patrol::PatrolIndex는 raw -1 대신 INDEX_NONE을 사용한다.
-> assignment lease age 누락값은 CCombatEngageConstants::MissingAssignmentLeaseAge로 이름을 부여한다.
-> CActionFeedbackTypes의 RelativeScale 기본값은 FVector::OneVector를 사용한다.
-> Execution Intervention notify editor color는 ExecutionInterventionWindowEditorColor로 이름을 부여한다.
-> ParryStaggerThreshold는 ClampMin = 1과 runtime MinimumParryStaggerThreshold guard를 함께 사용한다.
```

설계상 보류:

```text
-> DataAsset / Project Settings 전환은 asset / Blueprint / PIE 검증이 필요하므로 후속 작업으로 남긴다.
```

## 6.2 Serialized UPROPERTY regrouping migration 대상

이번 PR에서 `UPROPERTY`가 config `USTRUCT`로 묶이면서 legacy asset 값을 보호한 대상이다.

대상:

```text
CHitFeedbackComponent
-> legacy: HitStopAudience / HitStopDuration / HitStopDilation
-> new: HitStopTuning
-> legacy: bEnableCameraShake / CameraShakeAudience / CameraShakeClass / CameraShakeBaseScale
-> new: CameraShakeTuning

CAIController
-> legacy: TargetMemoryTimeout
-> new: PerceptionSetup.TargetMemoryTimeout
-> legacy: SightConfig subobject의 SightRadius / LoseSightRadius / PeripheralVisionAngleDegrees / MaxAge / DetectionByAffiliation
-> new: PerceptionSetup
```

대상 asset:

```text
Content/01_Character/01_Player/BP_CPlayer.uasset
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/02_Controller/02_Enemy/BP_CAIController.uasset
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/BP_AIPerf_CAIController.uasset
```

처리 결과:

```text
1. 규칙 문서 + PostLoad migration layer 추가 완료.
2. 대상 asset Editor 저장 완료.
3. AI controller asset은 PerceptionSetup 값을 기준으로 저장 완료.
4. asset 저장 검증 이후 DeprecatedProperty field / PostLoad migration code 제거 완료.
```

최종 상태:

```text
-> runtime read path는 새 config USTRUCT만 사용한다.
-> DeprecatedProperty field는 남기지 않는다.
-> PostLoad migration code는 남기지 않는다.
-> SensesConfig 배열은 UE listener 등록 mirror로만 남기고 직접 편집 기준으로 보지 않는다.
```

---

## 7. 검증 계획

```powershell
rg -n "GetGuardActionPhaseIndex|ResolveGuardActionPhase|TriggerActionIndex\\s*=|FMath::Clamp\\([^\\n]*GetValueOnGameThread|AssignmentRebuildId ==|AssignmentWarmupStartTime|InDuration = 10\\.f|StopMontage\\(float InBlendOutTime =|0\\.5f" Source/Portfolio -g "*.h" -g "*.cpp"
rg -n "InitCapsuleSize|SetRelativeLocation|SetRelativeRotation|MaxWalkSpeed|TargetArmLength|SightRadius|LoseSightRadius|PeripheralVisionAngleDegrees|SetMaxAge|TargetMemoryTimeout|MovableRange|ReviveHP|HitStopDuration|HitStopDilation|CameraShakeBaseScale|LocalTargetShakeScale|LocalSourceShakeScale" Source/Portfolio -g "*.h" -g "*.cpp"
git diff --check
& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" PortfolioEditor Win64 Development -Project="C:\UE5_Portfolio\Portfolio_UE5.4_verGit\Portfolio\Portfolio.uproject" -WaitMutex -FromMsBuild
```

PIE smoke는 사용자가 확인한다.

---

## 8. BT service interval default cleanup

처리 결과:

```text
-> BT service constructor Interval / RandomDeviation raw defaults를 CBTServiceIntervalHelper public default API로 중앙화했다.
-> UpdateAIContext / UpdateAIIntentState / UpdateEngageContext / UpdateInvestigateContext 생성자는 helper default를 사용한다.
-> runtime interval selection 정책과 CVar mode 계약은 변경하지 않았다.
-> 값 변경 없음: AIContext 0.1f, AIIntentState 0.2f, EngageContext 0.1f, InvestigateContext 0.1f, RandomDeviation 0.0f.
```

---

## 9. 최종 전수조사 후보 전체

이 섹션은 상수 / 매직넘버 / 튜닝 소유권 관점에서 전수조사 중 후보로 잡힌 항목을 빠짐없이 추적하기 위한 목록이다.

주의:

```text
-> 아래 항목은 모두 "즉시 수정해야 하는 버그"를 뜻하지 않는다.
-> 이미 이번 브랜치에서 처리한 항목, 유지 가능한 항목, 후속 설계 후보를 모두 포함한다.
-> 추후 작업에서는 각 항목의 성격을 먼저 확인한 뒤 상수 정리 / 튜닝 소유권 / migration / validation 정책 중 하나로 분류한다.
```

### 9.1 이번 브랜치 처리 완료

Character setup:

```text
Source/Portfolio/Character/Player/CPlayer.cpp
Source/Portfolio/Character/Enemy/CEnemy.cpp
-> capsule radius / half height
-> mesh relative location / rotation
-> default walk speed
-> player camera boom / camera offset

처리:
-> CCharacterSetupTypes.h의 setup USTRUCT로 이동.
-> CPlayer / CEnemy는 setup 값을 constructor / OnConstruction에서 적용한다.
-> 값 변경 없음.
```

AI perception:

```text
Source/Portfolio/Controller/CAIController.h
Source/Portfolio/Controller/CAIController.cpp
Source/Portfolio/Type/CAIPerceptionSetupTypes.h
-> SightRadius
-> LoseSightRadius
-> PeripheralVisionAngleDegrees
-> MaxAge
-> TargetMemoryTimeout
-> DetectionByAffiliation flags

처리:
-> PerceptionSetup을 프로젝트 설정 소유자로 둔다.
-> constructor에서 SightConfig를 생성 / 등록해 UE AIPerception listener 등록 경로를 유지한다.
-> BeginPlay에서 ConfigureSightConfig()로 PerceptionSetup 값을 재적용한다.
-> AIPerceptionComponent의 SensesConfig는 engine-facing mirror로 보고 직접 편집 기준으로 보지 않는다.
```

Defense guard tuning:

```text
Source/Portfolio/Component/CCombatSignalTargetComponent.cpp
Source/Portfolio/Component/CDefenseComponent.h
Source/Portfolio/Type/CDefenseTuningTypes.h
-> GuardDamageMitigationMultiplier = 0.5f

처리:
-> FDefenseGuardTuning::GuardDamageTakenMultiplier로 이동.
-> UCDefenseComponent가 GuardTuning을 소유한다.
-> CCombatSignalTargetComponent는 DefenseComponent에서 값을 조회한다.
-> 값 변경 없음.
```

BT service interval / random deviation:

```text
Source/Portfolio/AI/BehaviorTree/Service/CBTServiceIntervalHelper.h
Source/Portfolio/AI/BehaviorTree/Service/CBTServiceIntervalHelper.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.cpp
-> Interval defaults
-> RandomDeviation defaults
-> RuntimeLOD interval preset values

처리:
-> service constructor raw defaults를 CBTServiceIntervalHelper public default API로 중앙화.
-> RuntimeLOD interval mode는 CVar int32 계약을 유지한다.
-> 값 변경 없음.
```

Debug / execution defaults:

```text
Source/Portfolio/Core/Debug/FLog.h
-> screen print duration 10.f
-> DefaultScreenPrintDuration으로 정리 완료.

Source/Portfolio/Type/CExecutionTypes.h
Source/Portfolio/Action/CAction.h
Source/Portfolio/Reaction/CReaction.h
-> montage stop blend out 0.1f
-> DefaultMontageStopBlendOutTime으로 정리 완료.
```

Feedback tuning:

```text
Source/Portfolio/Component/CHitFeedbackComponent.h
Source/Portfolio/Component/CPlayerFeedbackComponent.h
Source/Portfolio/Type/CCombatFeedbackTypes.h
-> hit stop duration / dilation
-> camera shake base scale
-> local source / target scale

처리:
-> component default는 tuning USTRUCT로 묶었다.
-> runtime request struct는 request fallback default로 유지한다.
-> legacy asset migration / save / migration code removal까지 완료.
```

### 9.2 바로 정리 가능한 잔여 후보

문자열 스타일:

```text
Source/Portfolio/Weapon/CWeaponActor.cpp
-> CreateDefaultSubobject<USceneComponent>("RootScene")

후보:
-> TEXT("RootScene")로 UE 문자열 스타일 정합성 맞춤.

성격:
-> 매직넘버 제거가 아니라 문자열 literal 스타일 정리.
-> 동작 변경 없음.

처리:
-> 완료. CreateDefaultSubobject 이름을 TEXT("RootScene")로 정리했다.
```

Action default index:

```text
Source/Portfolio/Component/CActionOrchestratorComponent.cpp
-> ActionIndex = 0 반복
-> combo fallback: IsActiveActionType(ComboAttack) ? GetActiveActionIndex() + 1 : 0

후보:
-> DefaultActionIndex / InitialActionIndex / FirstActionIndex 같은 local constexpr 이름 부여.

주의:
-> 0은 INDEX_NONE이 아니라 실제 0번 action을 의미할 가능성이 높다.
-> 이름은 "invalid"가 아니라 "first/default action" 의미로 잡아야 한다.

처리:
-> 완료. CActionIndexConstants::FirstActionIndex로 의미를 고정했다.
-> combo fallback의 다음 index offset은 CActionIndexConstants::NextSequentialActionOffset을 사용한다.
```

Guard phase index 내부 literal:

```text
Source/Portfolio/Type/CActionKeyTypes.h
-> Guard phase index 1 / 2 / 3 / 4 / 5

현재 상태:
-> 외부 사용부는 GetGuardActionPhaseIndex(EGuardActionPhase)를 통해 의미화되어 있다.

후보:
-> helper 내부 return literal에도 internal constexpr 이름을 부여한다.
-> 외부 API는 int32 ActionIndex 계약을 유지한다.

처리:
-> 완료. GuardInActionIndex / GuardOutActionIndex / GuardHoldActionIndex / GuardHitActionIndex / GuardParryActionIndex로 내부 literal 의미를 고정했다.
```

Combo chain sequence:

```text
Source/Portfolio/Action/CAction_ComboAttack.cpp
-> incomingKey.ActionIndex != activeKey.ActionIndex + 1
-> incomingKey.ActionIndex != ActiveDataKey_Cached.ActionIndex + 1

후보:
-> next combo action index helper 또는 local named expression으로 의도를 명시한다.

성격:
-> 숫자 자체보다 combo sequencing contract를 문서화 / 코드화하는 문제.
-> 값 변경 없음.

처리:
-> 완료. CActionIndexConstants::NextSequentialActionOffset을 사용해 next sequential action 의미를 드러냈다.
```

### 9.3 튜닝 소유권 / migration 후보

Parry result tuning:

```text
Source/Portfolio/Character/Player/CPlayer.h
-> ParryStaggerThreshold = 3

Source/Portfolio/Character/Enemy/CEnemy.h
-> ParryStaggerThreshold = 3

Source/Portfolio/Character/Player/CPlayer.cpp
Source/Portfolio/Character/Enemy/CEnemy.cpp
-> MinimumParryStaggerThreshold = 1

후보:
-> FDefenseParryTuning 또는 combat result tuning으로 이동.
-> Player / Enemy 공통 기준으로 둘지, character archetype별 override로 둘지 결정.

주의:
-> ParryStaggerThreshold는 serialized UPROPERTY이므로 이동 / regrouping 시 migration 대상이다.
-> 지금 바로 옮기면 asset 저장 / Editor load / Blueprint compile / PIE smoke 검증이 필요하다.
```

AI behavior tuning:

```text
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.h
-> MovableRange = 1000.f

Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartRevive.h
-> ReviveHP = 30.f

후보:
-> BT node-local UPROPERTY로 유지.
-> Enemy / AI tuning struct로 이동.
-> AI behavior DataAsset로 이동.

주의:
-> BT asset override와 연결될 수 있으므로 단순 constexpr 치환 대상이 아니다.
-> 여러 enemy archetype이 값을 공유해야 하는지 먼저 판단한다.
```

Health default policy:

```text
Source/Portfolio/Component/CHealthComponent.h
-> InitMaxHP = 0.f
-> InitCurrentHP = 0.f
-> MaxHP / PreviousHP / CurrentHP = 0.f

후보:
-> "BP에서 반드시 설정해야 하는 component" 계약으로 유지.
-> C++ 기본 HP를 제공.
-> character setup / health tuning struct로 이동.

주의:
-> 0 HP default는 component가 미설정 상태에서 사실상 unusable/dead가 되는 계약이다.
-> 의도된 must-configure 패턴이면 유지 가능하지만 문서화가 필요하다.
```

Enemy AI tuning:

```text
Source/Portfolio/Character/Enemy/CEnemy.h
-> patrol / investigate / chase / alert / engage 관련 거리, 시간, flag UPROPERTY

후보:
-> 현재처럼 Enemy UPROPERTY로 유지.
-> FEnemyAITuning / FAIBehaviorTuning struct로 묶음.
-> enemy archetype DataAsset로 이동.

주의:
-> 현재는 CEnemy가 AI behavior tuning 소유자다.
-> 구조체화 / DataAsset화는 serialized field migration과 asset 검증 대상이다.
```

Weapon socket contract:

```text
Source/Portfolio/Weapon/CWeaponActor.h
-> SocketName_Holster
-> SocketName_Hand

후보:
-> BP 필수 설정 계약으로 유지.
-> 기본 socket name constant 제공.
-> weapon tuning / weapon setup struct로 이동.

주의:
-> socket 이름은 skeleton / mesh asset contract와 연결된다.
-> 잘못된 기본값을 넣으면 누락이 숨겨질 수 있다.
```

Input binding string contract:

```text
Source/Portfolio/Controller/CPlayerController.cpp
-> input binding action name string

후보:
-> input action name constants로 중앙화.
-> enhanced input migration 때 함께 정리.

주의:
-> Project Settings input name과 연결되는 외부 계약값이다.
-> 이름 변경 금지.
```

Feedback default ownership:

```text
Source/Portfolio/Type/CCombatFeedbackTypes.h
-> FHitStopRequest default
-> FCameraShakeRequest default

후보:
-> request fallback default로 유지.
-> shared feedback default namespace로 이동.

주의:
-> component tuning과 request fallback default는 목적이 다르다.
-> dedupe / playback model 변경 작업과 섞지 않는다.
```

### 9.4 RuntimeLOD / CVar 후보

AI animation reduced refresh:

```text
Source/Portfolio/AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.cpp
-> EnemyAnimationReducedRefreshInterval = 0.1f

후보:
-> CVar default로 유지.
-> local constexpr default 이름을 부여하고 CVar default에 사용.

주의:
-> CVar 이름 / 설명 / default는 외부 조정 계약값이다.
-> 값 변경 금지.
```

RuntimeLOD mode CVar:

```text
Source/Portfolio/AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.cpp
-> movement mode 0 / 1 / 2

Source/Portfolio/AI/RuntimeLOD/CAIStateRuntimeLODPolicy.cpp
-> state policy mode 0 / 1

Source/Portfolio/Character/Enemy/CEnemy.cpp
-> enemy mesh mode 0 / 1
-> enemy actor tick mode 0 / 1

Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp
-> warmup time default 0.0f
-> engage cap default 2
-> alert cap default 6

현재 상태:
-> enum wrapper / helper / CVar 설명으로 의미가 드러난다.

후보:
-> CVar default 값에 local constexpr 이름 부여.

주의:
-> CVar int32 / float default는 외부 튜닝 계약이므로 이름 / 값 변경 금지.
```

### 9.5 Data / validation 후보

Action / Reaction play rate:

```text
Source/Portfolio/Type/CActionDataTypes.h
-> PlayRate = 1.0f

Source/Portfolio/Type/CReactionDataTypes.h
-> PlayRate = 1.f

후보:
-> editor clamp metadata 추가.
-> IsValidMinimal / validation 정책과 함께 정리.

주의:
-> 상수 제거라기보다 authoring validation 정책이다.
-> runtime에서 duration <= 0을 거부하는 흐름과 맞춰 검토한다.
```

Defense guard clamp:

```text
Source/Portfolio/Type/CDefenseTuningTypes.h
-> GuardDamageTakenMultiplier = 0.5f
-> ClampMin = 0.0

후보:
-> ClampMax = 1.0 추가 여부 검토.

주의:
-> 1.0 초과를 "피해 증폭"으로 허용할지, guard mitigation으로 제한할지 정책 결정이 필요하다.
```

Notify editor color:

```text
Source/Portfolio/Notify/CAnimNotifyState_ExecutionInterventionWindow.cpp
-> FLinearColor(0.1f, 0.45f, 0.95f, 1.0f)

현재 상태:
-> ExecutionInterventionWindowEditorColor로 이름 부여됨.

판정:
-> editor visualization default로 유지 가능.
```

### 9.6 BT / AI index 후보

Investigate index:

```text
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartInvestigate.cpp
-> InvestigateIndex = 0

Source/Portfolio/AI/BehaviorTree/Task/CBTTask_EndInvestigate.cpp
-> InvestigateIndex = INDEX_NONE

Source/Portfolio/AI/BehaviorTree/Task/CBTTask_AdvanceInvestigateIndex.cpp
-> currentIndex + 1

후보:
-> first investigate index / next investigate index helper.

주의:
-> EndInvestigate의 invalid sentinel은 INDEX_NONE으로 정리 완료.
-> Start / Advance는 sequence contract라 유지 가능.
```

Patrol index:

```text
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_SelectPatrolPoint.cpp
-> 0
-> 1
-> count - 1
-> count - 2
-> nextIndex + 1

후보:
-> patrol index boundary helper.

주의:
-> 대부분 배열 index / boundary arithmetic이다.
-> 과도하게 상수화하면 알고리즘 가독성이 떨어질 수 있다.
```

Alert side multiplier:

```text
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_SelectAlertPoint.cpp
-> side multiplier 1.f / -1.f

후보:
-> named direction multiplier.

주의:
-> 현재는 local algorithm value라 유지 가능.
```

### 9.7 유지 권장 literal

다음 값들은 현재 기준으로 상수 제거 대상에서 제외한다.

```text
-> 초기화 / reset 값 0, 0.f
-> loop index 0 / ++i
-> count 비교 / count reset
-> HP / damage / heal guard 0.f
-> hash seed uint32 H = 0
-> pure virtual = 0
-> enum None = 0 / Idle = 0
-> INDEX_NONE / NAME_None / INT_MAX / KINDA_SMALL_NUMBER
-> FVector::ZeroVector / FVector::OneVector
-> CSV counter increment 1
-> debug / profiling counter reset 0
-> bool false / true
```

판정:

```text
-> 이 값들은 언어 / UE / 알고리즘 관례값이다.
-> 이름을 붙이는 것이 항상 가독성을 높이지 않는다.
-> 의미가 외부 계약이나 도메인 정책으로 올라갈 때만 별도 상수 / config로 승격한다.
```

### 9.8 후속 작업 단위 제안

상수 정리형 처리 완료:

```text
-> CWeaponActor RootScene TEXT() 스타일 정리
-> CActionOrchestratorComponent default ActionIndex 명명
-> CActionKeyTypes guard phase 내부 literal 명명
-> CAction_ComboAttack next index 의도 명명
-> 위 항목은 이번 브랜치에서 처리 완료.
```

튜닝 소유권 follow-up:

```text
-> Parry result tuning 소유권 결정
-> BT node-local tuning 유지 / 이동 기준 결정
-> CHealthComponent 기본 HP 정책 결정
-> CEnemy AI tuning struct / DataAsset 여부 결정
-> Weapon socket default contract 결정
```

validation follow-up:

```text
-> PlayRate Clamp metadata / IsValidMinimal 정책 정리
-> GuardDamageTakenMultiplier ClampMax 허용 여부 결정
```

input follow-up:

```text
-> CPlayerController input binding string contract 중앙화
-> Enhanced Input migration 작업과 병합 여부 결정
```
