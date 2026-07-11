# UE5 Portfolio Pull Request

## 제목

**P39: AI Combat Feedback Presentation 비용 분리 측정**

## 날짜

**2026.07.11**

## 상태

- [x] Combat Feedback Presentation 측정 범위 정리
- [x] Enemy combat feedback presentation gate 추가
- [x] event 기반 feedback counter flush 구조 추가
- [x] Trail on / cleanup counter 분리
- [x] 40 / 80 Enemy 측정 완료
- [x] Runtime LOD 우선 제어 후보 여부 판단

## 브랜치

- `feature/ai-combat-feedback-profiling`

## 요약

이번 PR은 AI Runtime LOD 후보 중 `Combat Feedback Presentation` 축을 분리 측정한다.

P38에서 `Combat Collision / HitProcessing`은 계측과 분리는 가능했지만 Runtime LOD v1 우선 제어 후보로 보기 어렵다고 판단했다. 이번 브랜치에서는 전투 판정, hit processing, combat signal, hit stop은 유지하고 Enemy 쪽 action presentation만 끄는 조건을 만들어 feedback presentation 비용을 분리했다.

결론적으로 `DisableEnemyCombatFeedback` gate는 의도대로 Enemy action trail / VFX / SFX를 차단했다. 하지만 40 / 80 Enemy 조건 모두에서 Frame / Game p95 개선은 유의미하지 않았다. 따라서 `Combat Feedback Presentation`은 Runtime LOD v1의 핵심 병목 축이 아니라, 최하위 representation 단계에서 선택적으로 줄일 수 있는 후보로 둔다.

## 주요 변경

```text
1. Combat Feedback profiling helper 추가
   - Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback
   - Enemy owner feedback presentation skip 판정
   - feedback event counter 누적
   - CombatEngage tick에서 CSV flush

2. ActionFeedback gate 추가
   - Enemy ActionFeedback request는 유지
   - DisableEnemyCombatFeedback 1에서 presentation 실행은 skip
   - Trail / VFX / SFX counter 기록

3. Trail counter 분리
   - Trail: 실제 trail on presentation
   - TrailClear: action cleanup 과정의 trail off 보장 호출
   - Disabled 조건에서 TrailClear가 남아도 presentation 실행으로 해석하지 않도록 분리

4. ReactionFeedback / HitFeedback gate 추가
   - Enemy owner feedback presentation skip
   - Player가 맞아서 Player component에서 실행되는 HitFeedback은 유지
   - HitStop은 timing / simulation에 영향을 주므로 유지

5. Combat Feedback 전용 profiling map 추가
   - 40 Enemy
   - 80 Enemy

6. 측정 문서 정리
   - 포함 / 제외 범위 명시
   - event 기반 counter flush 기준 명시
   - 40 / 80 Enemy 대표 측정 결과 정리
   - Runtime LOD 우선 제어 후보 여부 결론 정리
```

## CVar

```text
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback
```

의미:

```text
0: Enemy combat feedback presentation 실행
1: Enemy combat feedback presentation skip
```

이번 CVar는 profiling 전용이다. 목적은 전투 결과를 유지한 상태에서 presentation route만 분리해서 보는 것이다.

## 계측 정책

feedback route는 AnimNotify, action feedback request, reaction feedback, hit feedback처럼 event 기반 호출이 많다.

단일 event 지점에서 직접 CSV timing stat을 남기면 호출 빈도가 낮거나 phase가 흔들려 해석이 불안정해질 수 있다. 따라서 P38과 같은 기준으로 event 지점에서는 counter만 누적하고, `UCWorldSubsystem_CombatEngage::Tick()`에서 flush counter를 CSV로 기록한다.

```text
event 지점:
-> FCombatFeedbackProfiling::Record...()로 count만 누적

flush 지점:
-> UCWorldSubsystem_CombatEngage::Tick()에서 FlushToCsv()

분석 기준:
-> *_FlushCount 컬럼을 공식 count로 사용
```

Trail은 on/off 의미가 섞이면 해석이 깨진다. 따라서 실제 presentation 실행은 `Trail`, cleanup 호출은 `TrailClear`로 분리했다.

## 변경 파일

```text
Source/Portfolio/Core/Profiling/CCombatFeedbackProfiling.h
Source/Portfolio/Core/Profiling/CCombatFeedbackProfiling.cpp

Source/Portfolio/Component/CActionFeedbackComponent.cpp
Source/Portfolio/Component/CReactionFeedbackComponent.cpp
Source/Portfolio/Component/CHitFeedbackComponent.cpp
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp

Content/00_Profiling/00_AI_Performance/00_Map/10_CombatFeedback/MAP_AIPerf_CombatFeedback_40Enemy.umap
Content/00_Profiling/00_AI_Performance/00_Map/10_CombatFeedback/MAP_AIPerf_CombatFeedback_80Enemy.umap

Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P39_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N19_Code_Quality_PR_Status_Summary_Note.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/CSV_Evidence_Manifest.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Combat_Feedback_Presentation_Measurement_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/README.md
```

## 측정 조건

```text
Capture Duration: 약 36~37초
Analysis Window: first 3s / last 3s trimmed
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
Fixed camera
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode 0
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationRefreshCounter 0
Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 0
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 0
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit 0
Portfolio.AI.RuntimeLOD.EnemyMovementMode 0
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 0
```

## 대표 측정

| Enemy | Case | CVar | CSV |
| ---: | --- | ---: | --- |
| 40 | FeedbackBaseline | 0 | `Profile(20260711_191545).csv` |
| 40 | FeedbackDisabled | 1 | `Profile(20260711_191931).csv` |
| 80 | FeedbackBaseline | 0 | `Profile(20260711_193111).csv` |
| 80 | FeedbackDisabled | 1 | `Profile(20260711_193802).csv` |

`Profile(20260711_192806).csv`는 80 Enemy baseline 계열이지만 action feedback counter가 대표 baseline보다 한 사이클 더 들어간 형태라 최종 대표값에서 제외했다.

## 측정 결과

### Frame / Game

| Enemy | Case | Frame p95 | Game p95 | GPU p95 | CharacterMovement p95 | BT Tick p95 |
| ---: | --- | ---: | ---: | ---: | ---: | ---: |
| 40 | Baseline | 12.4115ms | 12.3616ms | 10.4383ms | 0.5052ms | 0.2125ms |
| 40 | Disabled | 12.2223ms | 12.2703ms | 10.0552ms | 0.5309ms | 0.2104ms |
| 80 | Baseline | 17.9264ms | 17.8648ms | 11.3880ms | 0.7770ms | 0.4195ms |
| 80 | Disabled | 18.3894ms | 18.4272ms | 10.9936ms | 0.9020ms | 0.4174ms |

### Feedback Counter

| Enemy | Case | Request | Skipped | Trail | TrailClear | VFX | SFX |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 40 | Baseline | 104 | 0 | 22 | 12 | 20 | 34 |
| 40 | Disabled | 102 | 102 | 0 | 6 | 0 | 0 |
| 80 | Baseline | 104 | 0 | 22 | 12 | 20 | 34 |
| 80 | Disabled | 104 | 104 | 0 | 6 | 0 | 0 |

## 해석

`DisableEnemyCombatFeedback 1`에서 `ActionFeedback_Skipped`가 request 수와 동일하게 증가하고 `Trail / VFX / SFX`가 0으로 떨어졌다. 따라서 CVar gate는 의도대로 동작한다.

`TrailClear`는 action cleanup에서 trail off를 보장하는 호출이다. Disabled 조건에서 `TrailClear`가 남는 것은 presentation이 살아있는 것이 아니라 cleanup route가 유지된 것이다.

Player가 맞아서 Player component에서 실행되는 `HitFeedback / HitStop`은 유지된다. 이 PR의 CVar는 Enemy owner feedback presentation을 끄는 축이지, Enemy 공격으로 발생하는 모든 target-side hit feedback을 끄는 축이 아니다.

Frame / Game p95는 40 / 80 Enemy 모두에서 유의미하게 개선되지 않았다. GPU p95는 일부 감소했지만 GameThread와 frame budget을 회복할 만큼의 안정적인 개선으로 보기는 어렵다.

따라서 이번 PR의 결론은 다음과 같다.

```text
Combat Feedback Presentation 축은 기능적으로 분리 가능하다.
하지만 현재 Engage / Alert 조건에서 Runtime LOD v1의 핵심 병목은 아니다.
최하위 representation 단계에서 선택적으로 줄일 수 있는 후보로만 유지한다.
```

## 검증

```text
1. PortfolioEditor Development build
2. 40 Enemy / FeedbackBaseline 측정
3. 40 Enemy / FeedbackDisabled 측정
4. 80 Enemy / FeedbackBaseline 측정
5. 80 Enemy / FeedbackDisabled 측정
6. GC 이벤트 없음 확인
7. ActionFeedback Skipped / Trail / VFX / SFX counter 확인
8. TrailClear cleanup counter 분리 확인
9. HitFeedback / HitStop 유지 확인
10. event 기반 계측은 *_FlushCount 기준으로 해석
```

## 제외 범위

```text
1. HitProcessing / CombatSignal 제거
   - P38에서 별도로 측정했고 Runtime LOD v1 우선 후보에서 제외했다.

2. WeaponActor 생성 / 제거 비용
   - 이전 Runtime LOD 측정 축에서 별도로 다뤘다.

3. Combat action 자체 제거
   - 이번 PR은 attack montage와 전투 결과를 유지한 presentation 분리만 다룬다.

4. Player-side HitFeedback 제거
   - Player 피격 체감과 timing에 직접 영향을 줄 수 있어 이번 축에서 제외했다.

5. Proxy / actor count 최적화
   - Runtime LOD 후속 후보로 유지한다.
```

## 후속 작업

```text
1. Runtime LOD v1 구현 후보 정리
   - AlertCap / assignment 계층
   - BT service update precision
   - movement 후보 제한

2. Component Tick Audit
   - CEnemy / CActionComponent / CMovementComponent 등 non-essential tick 제한 후보 확인

3. Perception Active Budget / Cap
   - 감지 후보 수 또는 active perception budget 제어 후보 확인
```
