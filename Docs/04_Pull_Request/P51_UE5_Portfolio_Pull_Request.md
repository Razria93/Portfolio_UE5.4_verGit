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
-> guard mitigation multiplier에 이름 부여
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
-> CHitFeedbackComponent legacy HitStop / CameraShake 필드: PostLoad migration
-> CAIController legacy SightConfig / TargetMemoryTimeout 값: PostLoad migration
-> Player / Enemy parry threshold 내부 상수: unity build 충돌 방지용 namespace 분리
```

결과

값 변경 없이 의미가 불명확한 남은 literal을 정리했다.

### 6. 리뷰 대응

무엇

`CHitFeedbackComponent`의 hit feedback 설정을 구조체로 묶는 과정에서 기존 Blueprint asset이 저장하던 legacy scalar property 값을 잃지 않도록 호환 레이어를 추가했다.

어떻게

```text
-> 기존 HitStopAudience / HitStopDuration / HitStopDilation property를 deprecated property로 유지
-> 기존 bEnableCameraShake / CameraShakeAudience / CameraShakeClass / CameraShakeBaseScale property를 deprecated property로 유지
-> PostLoad에서 legacy 값이 의미를 가진 경우 새 HitStopTuning / CameraShakeTuning으로 복사
-> 새 struct 값이 이미 설정된 경우 legacy 기본값으로 덮어쓰지 않도록 조건부 migration 적용
-> deprecated field에는 raw default literal을 두지 않고 새 config struct 기본값에서 초기화
```

결과

기존 `BP_CPlayer`, `BP_CEnemy` 같은 Blueprint asset이 저장한 camera shake class와 hit feedback 기본값을 새 구조체 설정으로 이어받을 수 있게 했다.

추가로 `CAIController`의 perception 설정도 기존 Blueprint가 `SightConfig` subobject와 `TargetMemoryTimeout`에 저장한 값을 잃지 않도록 `PostLoad` migration을 추가했다. 기존 asset 값이 native default와 다를 때만 `PerceptionSetup`으로 복사하고, 이후 BeginPlay의 sight config 적용은 migration된 값을 기준으로 수행한다.

AI perception 설정의 기준은 `PerceptionSetup`으로 둔다. `AIPerceptionComponent`의 SensesConfig 배열은 legacy migration 입력으로만 보고, 최종 asset 상태에서는 비워 중복 설정 지점이 보이지 않게 한다. 생성자에서는 `AIPerceptionComponent`만 만들고, BeginPlay에서 runtime `SightConfig`를 구성한다. `ConfigureSightConfig()`는 `ConfigureSense`와 `SetDominantSense`를 함께 수행한다.

이 migration layer는 영구 구조가 아니다. 대상 asset을 Editor에서 열고 저장해 새 config struct 값이 asset에 기록되면, 후속 커밋에서 deprecated field와 `PostLoad` migration 코드를 제거한다.

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
-> BT service Interval / RandomDeviation 정책 변경
-> GuardDamageMitigationMultiplier 소유권 이동
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
