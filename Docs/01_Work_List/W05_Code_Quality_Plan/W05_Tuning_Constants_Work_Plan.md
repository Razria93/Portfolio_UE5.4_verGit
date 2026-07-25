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
-> 미진행
```

대상:

```text
Source/Portfolio/Component/CCombatSignalTargetComponent.cpp
-> guard mitigation multiplier 0.5f
```

판정:

```text
-> 현재는 internal policy constexpr 후보.
-> 장기적으로 defense / guard tuning 데이터 후보이기도 하므로 이름은 GuardDamageMitigationMultiplier처럼 정책 의미를 드러낸다.
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

## 7. 검증 계획

```powershell
rg -n "GetGuardActionPhaseIndex|ResolveGuardActionPhase|TriggerActionIndex\\s*=|FMath::Clamp\\([^\\n]*GetValueOnGameThread|AssignmentRebuildId ==|AssignmentWarmupStartTime|InDuration = 10\\.f|StopMontage\\(float InBlendOutTime =|0\\.5f" Source/Portfolio -g "*.h" -g "*.cpp"
rg -n "InitCapsuleSize|SetRelativeLocation|SetRelativeRotation|MaxWalkSpeed|TargetArmLength|SightRadius|LoseSightRadius|PeripheralVisionAngleDegrees|SetMaxAge|TargetMemoryTimeout|MovableRange|ReviveHP|HitStopDuration|HitStopDilation|CameraShakeBaseScale|LocalTargetShakeScale|LocalSourceShakeScale" Source/Portfolio -g "*.h" -g "*.cpp"
git diff --check
& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" PortfolioEditor Win64 Development -Project="C:\UE5_Portfolio\Portfolio_UE5.4_verGit\Portfolio\Portfolio.uproject" -WaitMutex -FromMsBuild
```

PIE smoke는 사용자가 확인한다.
