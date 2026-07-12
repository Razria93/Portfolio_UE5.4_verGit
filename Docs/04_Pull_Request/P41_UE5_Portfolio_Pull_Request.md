# UE5 Portfolio Pull Request

## 제목

**P41: AI State Runtime LOD tier snapshot 통합**

## 날짜

**2026.07.12**

## 상태

- [x] Runtime LOD tier resolver / policy 책임 분리
- [x] `ACAIController` tier snapshot 추가
- [x] BT interval helper가 controller snapshot을 소비하도록 변경
- [x] Movement Runtime LOD policy가 controller snapshot을 소비하도록 변경
- [x] Animation Runtime LOD policy 분리 및 snapshot 소비 연결
- [x] StatePolicy 40 / 80 Enemy profiling map 추가
- [x] 40 Enemy StatePolicyMode 0 / 1 smoke 측정 완료
- [x] `PortfolioEditor Win64 Development` build 통과

## 브랜치

- `feature/ai-state-based-runtime-lod`

## 요약

이번 PR은 개별 CVar로 검증해 온 AI Runtime LOD 축을 하나의 `State Runtime LOD tier snapshot` 기반으로 묶는 구조 작업이다.

기존에는 BT interval helper, Movement policy, Animation refresh gate가 각자 Blackboard 또는 CVar를 직접 해석할 수 있는 구조였다. 이 방식은 service별 갱신 주기가 다를 때 stale 값으로 인해 상태가 튀거나, 같은 Enemy를 서로 다른 tier로 판단할 위험이 있다.

이번 PR에서는 Runtime LOD tier 판정을 공통 resolver로 분리하고, `ACAIController`가 현재 tier snapshot을 저장한다. 이후 BT / Movement / Animation 소비자는 같은 snapshot을 읽는다.

이번 PR의 목적은 Frame / Game p95를 크게 줄이는 것이 아니라, Runtime LOD v1 정책을 안전하게 확장할 수 있는 공통 소비 구조를 만드는 것이다.

## 주요 변경

```text
1. Runtime LOD tier resolver / policy 책임 분리
   - AIIntentState 이름을 그대로 성능 tier로 사용하지 않음
   - CombatRole / awareness / forced critical state를 기준으로 tier 판정
   - StateRuntimeLODPolicy는 CVar / audit 계층으로 축소

2. ACAIController tier snapshot
   - CurrentRuntimeLODTier 추가
   - RefreshRuntimeLODTierFromBlackboard() 추가
   - 초기화 단계에서는 refresh 실패 시 runtime init 실패 처리
   - runtime service refresh는 best-effort로 사용

3. BT interval helper snapshot 소비
   - AIContext interval은 고정
   - EngageContext interval은 고정
   - AIIntentState interval만 Runtime LOD tier snapshot 기반으로 선택
   - controller snapshot이 없을 때만 Blackboard fallback

4. Movement Runtime LOD policy snapshot 소비
   - StatePolicyMode off: 기존 EnemyMovementMode CVar 사용
   - StatePolicyMode on: controller tier snapshot 기반으로 movement mode 선택
   - Awareness / Dormant는 movement intent block 후보

5. Animation Runtime LOD policy 분리
   - FAIAnimationRuntimeLODPolicy 추가
   - CAnimInstance에서 CVar / tier mapping 책임 제거
   - CAnimInstance는 parameter refresh / counter 기록만 담당
   - Awareness / Background / Dormant는 parameter refresh reduced 적용

6. Profiling map 추가
   - MAP_AIPerf_StatePolicy_40Enemy
   - MAP_AIPerf_StatePolicy_80Enemy
```

## Runtime LOD Tier

이번 PR의 tier는 행동 상태 이름이 아니라 성능 정책 계층이다.

```text
CombatCritical
-> 실제 combat result와 timing이 중요한 계층
-> Engage / HitReact / Dead 등

CombatSupport
-> 전투 후보 / 보조 계층
-> Alert role 등

Awareness
-> target awareness는 있지만 combat assignment 권한은 없는 계층

Background
-> target awareness가 없는 일반 활성 계층

Dormant
-> 후속 wake-up manager에서 다룰 비활성 계층
```

`Chase`, `Investigate`, `Observe`, `Idle`은 행동 상태이며 tier 자체가 아니다.

## CVar

```text
Portfolio.AI.RuntimeLOD.StatePolicyMode
Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode
Portfolio.AI.RuntimeLOD.EnemyMovementMode
Portfolio.AI.RuntimeLOD.EnemyAnimationMode
Portfolio.AI.RuntimeLOD.EnemyAnimationRefreshCounter
```

해석:

```text
StatePolicyMode 0
-> 개별 policy fallback / 기존 CVar 중심

StatePolicyMode 1
-> BT / Movement / Animation이 controller tier snapshot을 소비
```

## 변경 파일

```text
Source/Portfolio/AI/BehaviorTree/Service/CBTServiceIntervalHelper.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.cpp

Source/Portfolio/AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.h
Source/Portfolio/AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.cpp
Source/Portfolio/AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.h
Source/Portfolio/AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.cpp
Source/Portfolio/AI/RuntimeLOD/CAIRuntimeLODTierResolver.cpp
Source/Portfolio/AI/RuntimeLOD/CAIStateRuntimeLODPolicy.cpp
Source/Portfolio/AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h

Source/Portfolio/Character/CAnimInstance.h
Source/Portfolio/Character/CAnimInstance.cpp
Source/Portfolio/Component/CMovementComponent.cpp
Source/Portfolio/Controller/CAIController.h
Source/Portfolio/Controller/CAIController.cpp

Content/00_Profiling/00_AI_Performance/00_Map/12_StatePolicy/MAP_AIPerf_StatePolicy_40Enemy.umap
Content/00_Profiling/00_AI_Performance/00_Map/12_StatePolicy/MAP_AIPerf_StatePolicy_80Enemy.umap

Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P41_UE5_Portfolio_Pull_Request.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Tier_Snapshot_Refactor_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_State_Based_Runtime_LOD_Policy_Plan.md
```

## 측정 조건

```text
Case: 40 Enemy / StatePolicyMode 0
ID: 20260712_211525

Case: 40 Enemy / StatePolicyMode 1
ID: 20260712_211733

Capture Duration: 약 37초
Analysis Window: first 3s / last 3s trimmed
Log State: -noailogging
PIE: F11 fullscreen
Fixed camera
GC Event: none
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit 0
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit 0

Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode 2
Portfolio.AI.RuntimeLOD.EnemyMovementMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationRefreshCounter 1

Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 0
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 0
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit 0

Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback 0
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 0
```

Gameplay smoke:

```text
Engage 2 유지
Alert 6 유지
나머지 Observe 또는 Idle 유지
attack montage 정상
movement 이상 없음
animation 이상 없음
GC 이벤트 없음
```

## 측정 결과

| Metric | StatePolicyMode 0 | StatePolicyMode 1 | 해석 |
| --- | ---: | ---: | --- |
| Frame p95 | 11.9403ms | 12.0101ms | 거의 동일 |
| Game p95 | 11.3695ms | 11.3800ms | 거의 동일 |
| BT Tick p95 | 0.2187ms | 0.2123ms | 거의 동일 |
| CharacterMovement p95 | 0.5314ms | 0.5492ms | 거의 동일 |
| Animation p95 | 2.1760ms | 2.1841ms | 거의 동일 |
| AnimParallel p95 | 3.3673ms | 3.3678ms | 거의 동일 |

호출 수:

| Counter | StatePolicyMode 0 | StatePolicyMode 1 |
| --- | ---: | ---: |
| AIContext Count | 11,840 | 11,920 |
| AIIntent Count | 2,860 | 2,900 |
| EngageContext Count | 588 | 596 |
| AnimRefresh Attempt | 110,240 | 110,520 |
| AnimRefresh Executed | 110,240 | 31,640 |
| AnimRefresh Skipped | 0 | 78,880 |

Tier / interval counter:

| Counter | StatePolicyMode 0 | StatePolicyMode 1 |
| --- | ---: | ---: |
| CombatCritical Tier | 0 | 304 |
| CombatSupport Tier | 0 | 612 |
| Awareness Tier | 0 | 1,984 |
| AIIntent Default Interval | 302 | 304 |
| AIIntent Reduced Interval | 606 | 612 |
| AIIntent Aggressive Interval | 1,952 | 1,984 |

## 해석

이번 측정은 `StatePolicyMode 1`의 성능 개선량을 직접 증명하기 위한 측정이 아니라, tier snapshot 통합 구조의 smoke test다.

BT 호출 수가 비슷한 것은 정상이다.

```text
AIContext는 tier 판단 입력값을 만드는 서비스이므로 고정 interval을 유지한다.
EngageContext는 combat timing 계층이므로 고정 interval을 유지한다.
AIIntentState는 두 케이스 모두 BTUpdateIntervalMode 2 조건이므로 interval 분포가 비슷하다.
```

따라서 `AIContext / AIIntent / EngageContext Count`가 비슷한 것은 회귀가 아니라 설계상 자연스러운 결과다.

Mode 1에서 의미 있는 변화는 다음이다.

```text
StateLOD tier counter가 기록된다.
BT interval helper가 controller snapshot을 소비한다.
Movement / Animation policy가 같은 controller snapshot을 소비한다.
Animation parameter refresh가 tier에 따라 줄어든다.
```

단, 줄어든 것은 애니메이션 실행 전체가 아니다.

```text
줄어든 것:
RefreshMovementParameters()
RefreshStateParameters()

줄어들지 않은 것:
NativeUpdateAnimation 호출
AnimGraph evaluation
pose update
SkeletalMeshComponent tick
AnimationParallelEvaluation
```

그래서 `AnimRefresh Executed`는 크게 줄었지만, `Animation p95`와 `AnimParallel p95`는 거의 변하지 않았다.

결론:

```text
StatePolicyMode 1은 기능적으로 안전하게 적용됐다.
BT / Movement / Animation이 같은 tier snapshot을 소비하는 통합 구조는 smoke를 통과했다.
40 Enemy 조건에서 Frame / Game p95 개선은 제한적이다.
이번 PR은 성능 수치 개선 PR이 아니라 Runtime LOD v1 정책을 안전하게 확장하기 위한 구조 PR이다.
```

## 검증

```text
1. PortfolioEditor Win64 Development build
2. StatePolicyMode 0 / 1 40 Enemy smoke 측정
3. CVar 로그 확인
4. GC 이벤트 없음 확인
5. Engage 2 / Alert 6 / Observe 또는 Idle 계층 유지 확인
6. movement / animation 이상 없음 확인
7. tier counter / animation refresh counter 확인
```

## 제외 범위

```text
1. Perception Active Budget
   - 후속 브랜치에서 별도 설계한다.

2. Dormant wake-up manager
   - 거리 / 시야 / damage / script wake-up 정책이 필요하므로 후속 작업으로 분리한다.

3. Mesh hidden / pose skip / proxy
   - gameplay와 representation break 위험이 있어 v1 smoke 이후 고도화로 분리한다.

4. WeaponActor lifecycle off
   - actor 생성 / attach / socket / collision / montage notify 의존성이 있으므로 이번 PR에서 제외한다.

5. Combat hit pipeline / feedback 최적화
   - 측정상 event 기반이며 현재 40 / 80 Enemy 조건에서 핵심 병목으로 보기 어렵다.
```

## 후속 작업

```text
1. Perception Active Budget / Wake-up
2. Dormant manager 설계
3. Background / Dormant mesh visibility 또는 proxy 검토
4. Weapon mesh visibility policy 검토
5. Animation pose skip isolation
```
