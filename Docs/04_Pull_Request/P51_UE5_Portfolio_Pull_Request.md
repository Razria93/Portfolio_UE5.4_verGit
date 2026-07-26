# UE5 Portfolio Pull Request

## 제목

**P51: Tuning Constants Cleanup**

## 날짜

**2026.07.25**

## 상태

- [x] 상수 / 튜닝 데이터 분류 규칙 문서화
- [x] 내부 규칙값 / sentinel / mode literal 이름 부여
- [x] Character setup tuning 구조체화
- [x] AI perception setup tuning 구조체화
- [x] CombatEngage assignment tuning 구조체화
- [x] Feedback tuning 기본값 구조체화
- [x] Patrol editor visualization literal 이름 부여
- [x] 에이전트 교차검증 기반 최종 literal 전수조사
- [x] 즉시 처리 literal 후보 처리
- [x] DataAsset / Project Settings 전환 후보 보류 기록
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] PIE smoke 확인

## 브랜치

- `refactor/tuning-constants-cleanup`

## 요약

이번 PR은 `Source/Portfolio`의 숫자 literal을 기계적으로 제거하는 작업이 아니라, 값의 성격을 `내부 규칙값`, `튜닝 데이터`, `외부 계약값`, `유지 literal`로 분류하고 각 값에 맞는 소유권을 부여하는 정리 작업이다.

값 변경은 하지 않았고, 이미 editor-owned인 값은 불필요하게 옮기지 않았다. 여러 값이 함께 움직이는 actor setup, perception, feedback, assignment 값은 config USTRUCT로 묶고, CVar 이름/default, debug 문자열, request fallback 기본값, loop/reset/sentinel 계열은 유지하거나 명시적으로 보류했다.

## 변경 배경

기존 코드에는 다음 성격의 literal이 섞여 있었다.

```text
-> 내부 정책값인데 숫자만 남아 있어 의미가 약한 값
-> actor / AI / feedback tuning 값인데 소유권이 흩어진 값
-> CVar / Blackboard / debug string처럼 외부 계약에 가까워 건드리면 안 되는 값
-> loop index, reset 0, validity guard처럼 그대로 두는 편이 더 읽기 좋은 값
```

이번 PR에서는 "모든 literal 제거"가 아니라 "값의 성격에 맞는 표현과 소유권 정리"를 기준으로 삼았다.

## 변경 범위

### 1. 규칙 / 작업 계획 문서화

무엇

`W05_Tuning_Constants_Rules.md`, `W05_Tuning_Constants_Work_Plan.md`를 추가하고 W05 work list에 연결했다.

어떻게

다음 분류 기준을 문서화했다.

```text
-> 내부 규칙값
-> 튜닝 데이터
-> 외부 계약값
-> 유지 literal
-> DataAsset / Project Settings 후속 후보
```

결과

상수 정리와 DataAsset 전환을 같은 문제로 섞지 않고, 현재 PR에서 처리할 값과 후속 검증이 필요한 값을 분리했다.

### 2. 내부 규칙값 / sentinel 정리

무엇

값 변경 없이 내부 정책값에 이름을 부여했다.

어떻게

```text
-> guard action phase index는 EGuardActionPhase 중심 helper로 해석
-> notify guard trigger index는 GetGuardActionPhaseIndex() 사용
-> InvestigateIndex raw -1은 INDEX_NONE 사용
-> PatrolIndex raw -1은 INDEX_NONE 사용
-> montage stop blend out 기본값은 CExecutionConstants로 이동
-> screen print 기본 duration은 FLog::DefaultScreenPrintDuration으로 이동
-> CombatEngage warmup / rebuild sentinel은 CCombatEngageConstants로 이동
-> missing assignment lease age sentinel에 이름 부여
-> RuntimeLOD / BT interval mode raw number는 enum/helper로 감쌈
-> guard damage multiplier를 CDefenseComponent 소유 튜닝 값으로 이동
```

결과

정책 의미가 있는 숫자는 코드에서 역할을 읽을 수 있게 되었고, UE가 제공하는 sentinel은 새 상수보다 `INDEX_NONE`을 우선 사용하도록 정리됐다.

### 3. 튜닝 데이터 구조체화

무엇

여러 값이 함께 움직이는 튜닝 기본값을 config USTRUCT로 묶었다.

어떻게

```text
Character setup
-> FCharacterCapsuleSetup
-> FCharacterMeshSetup
-> FCharacterMovementSetup
-> FPlayerCameraSetup

AI perception
-> FAIControllerPerceptionSetup

CombatEngage
-> FEngageAssignmentTuning

Feedback
-> FHitStopFeedbackTuning
-> FHitCameraShakeFeedbackTuning
-> FPlayerCameraShakeFeedbackTuning
```

결과

actor setup, perception, assignment, feedback 기본값의 소유권이 명확해졌다. DataAsset 전환은 하지 않았고, asset / Blueprint / PIE 검증이 필요한 후속 후보로 남겼다.

### 4. Editor 표시 기본값 정리

무엇

튜닝 데이터가 아니라 editor 표시 기본값에 가까운 literal에 이름을 부여했다.

어떻게

```text
CPatrolPoint.cpp
-> PatrolPointTextWorldSize
-> PatrolPointTextHeight
-> PatrolPointTextFacingYaw

CAnimNotifyState_ExecutionInterventionWindow.cpp
-> ExecutionInterventionWindowEditorColor
```

결과

값은 그대로 유지하면서 editor-only / editor-facing literal의 의미를 드러냈다.

### 5. 최종 전수조사 후 즉시 처리 후보 반영

무엇

에이전트 3개와 로컬 `rg` 스캔으로 `Source/Portfolio`를 범위별로 전수조사했다.

어떻게

```text
-> Component / Character
-> AI / Controller / System
-> Action / Reaction / Notify / Type / Core
```

즉시 처리한 항목:

```text
-> CAIKey::Patrol::PatrolIndex: -1 -> INDEX_NONE
-> CombatEngage missing lease age: 이름 있는 sentinel
-> CActionFeedbackTypes RelativeScale: FVector::OneVector
-> Execution intervention notify editor color: 이름 있는 editor color
-> ParryStaggerThreshold: ClampMin = 1 + runtime 최소값 guard
-> CHitFeedbackComponent legacy HitStop / CameraShake 필드: PostLoad migration 후 제거
-> CAIController legacy SightConfig / TargetMemoryTimeout 값: PostLoad migration 후 제거
-> Player / Enemy parry threshold 내부 상수: unity build 충돌 방지용 namespace 분리
```

결과

값 변경 없이 의미가 불명확한 남은 literal을 정리했다.

### 6. 리뷰 대응

무엇

`CHitFeedbackComponent`의 hit feedback 설정을 구조체로 묶는 과정에서 기존 Blueprint asset이 저장하던 legacy scalar property 값을 새 config struct로 이전하고, asset 저장 후 migration layer를 제거했다.

어떻게

```text
-> 기존 HitStopAudience / HitStopDuration / HitStopDilation 값을 HitStopTuning으로 migration
-> 기존 bEnableCameraShake / CameraShakeAudience / CameraShakeClass / CameraShakeBaseScale 값을 CameraShakeTuning으로 migration
-> 대상 Blueprint asset 저장
-> DeprecatedProperty field 제거
-> PostLoad migration code 제거
```

결과

기존 `BP_CPlayer`, `BP_CEnemy` 같은 Blueprint asset이 저장한 camera shake class와 hit feedback 기본값을 새 구조체 설정으로 이어받을 수 있게 했다.

추가로 `CAIController`의 perception 설정도 기존 Blueprint가 `SightConfig` subobject와 `TargetMemoryTimeout`에 저장한 값을 `PerceptionSetup`으로 이전한 뒤 migration layer를 제거했다. 이후 BeginPlay의 sight config 적용은 저장된 `PerceptionSetup` 값을 기준으로 수행한다.

AI perception 설정의 기준은 `PerceptionSetup`으로 둔다. `AIPerceptionComponent`의 SensesConfig 배열은 UE listener 등록을 위한 engine-facing mirror로 보고 직접 편집 기준으로 보지 않는다. 생성자에서 `SightConfig`를 기본 등록해 UE AIPerception listener 등록 흐름을 유지하고, BeginPlay에서 `ConfigureSightConfig()`를 다시 호출해 저장된 `PerceptionSetup` 값을 runtime 적용한다.

migration layer는 영구 구조가 아니므로 최종 코드에는 남기지 않았다.

### 7. Defense guard tuning 소유권 정리

무엇

`CCombatSignalTargetComponent`에 있던 guard damage multiplier를 `CDefenseComponent`의 guard tuning 값으로 이동했다.

어떻게

```text
-> Type/CDefenseTuningTypes.h에 FDefenseGuardTuning 추가
-> UCDefenseComponent는 FDefenseGuardTuning GuardTuning을 직접 소유
-> FDefenseTuning aggregate는 아직 만들지 않음
-> GuardDamageTakenMultiplier 기본값 0.5 유지
-> CCombatSignalTargetComponent는 DefenseComp.GetGuardDamageTakenMultiplier()를 통해 조회
```

결과

guard 중 받는 데미지 배율의 소유권이 target signal 처리 코드가 아니라 defense / guard 정책 쪽으로 이동했다.

구조 판단:

```text
-> 현재 defense tuning은 guard damage multiplier 하나뿐이므로 넓은 FDefenseTuning aggregate를 만들지 않는다.
-> Parry tuning이 실제로 생기면 FDefenseParryTuning을 같은 Type 헤더에 추가하고 UCDefenseComponent가 ParryTuning으로 병렬 소유한다.
-> Dodge는 현재 Action 소유이므로 defense tuning에 미리 포함하지 않는다.
-> Guard / Parry 등을 하나의 preset / DataAsset / runtime config 단위로 다룰 필요가 생길 때만 FDefenseTuning aggregate를 검토한다.
```

### 8. BT service interval default 정리

무엇

BT service 생성자에 남아 있던 `Interval` / `RandomDeviation` raw 기본값을 `CBTServiceIntervalHelper`로 중앙화했다.

어떻게

```text
-> GetDefaultAIContextInterval()
-> GetDefaultAIIntentStateInterval()
-> GetDefaultEngageContextInterval()
-> GetDefaultInvestigateContextInterval()
-> GetDefaultRandomDeviation()
```

결과

BT service editor 기본 표시값과 runtime interval helper의 기본값 소유권이 같은 helper로 모였다. Runtime LOD interval selection과 CVar mode 계약은 변경하지 않았다.

## 명시적 보류

이번 PR에서 하지 않은 것:

```text
-> DataAsset 전환
-> Project Settings 전환
-> 광범위 Blueprint / serialized asset migration
-> USTRUCT / UPROPERTY rename
-> enum entry rename
-> CVar 이름 / default 계약 변경
-> debug / audit / log 문자열 중앙화
```

후속 후보:

```text
-> CharacterSetup DataAsset
-> AI perception Project Settings / DataAsset
-> Enemy AI tuning DataAsset
-> Feedback preset DataAsset
-> CombatEngage subsystem Project Settings
-> DataAsset validation / editor helper
```

## 검증

수행한 검증:

```text
git diff --check
PortfolioEditor Win64 Development build
PIE smoke
```

결과:

```text
-> git diff --check 통과
-> PortfolioEditor Win64 Development build 통과
-> PIE 정상 동작 확인
```

## 리뷰 포인트

```text
-> 값 변경이 없는 정리인지 확인
-> config USTRUCT로 묶은 값의 소유권이 적절한지 확인
-> CVar / Blackboard / debug string 같은 외부 계약값을 건드리지 않았는지 확인
-> request struct fallback 기본값을 runtime request 용도로 유지한 판단이 적절한지 확인
-> DataAsset 전환을 후속 후보로 남긴 범위가 적절한지 확인
```

## 커밋

대표 커밋:

```text
refactor(constants): name runtime lod mode values
refactor(constants): name guard mitigation multiplier
refactor(character): group setup tuning defaults
refactor(ai): group perception tuning defaults
refactor(combat): group engage assignment tuning
refactor(feedback): group feedback tuning defaults
refactor(ai): name patrol editor visualization defaults
refactor(core): clean up remaining literal defaults
```
