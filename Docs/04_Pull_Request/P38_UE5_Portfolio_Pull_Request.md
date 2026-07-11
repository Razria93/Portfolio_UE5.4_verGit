# UE5 Portfolio Pull Request

## 제목

**P38: AI Combat Collision / Hit Window 비용 분리 측정**

## 날짜

**2026.07.11**

## 상태

- [x] Combat Collision / Hit Window 계측 기준 정리
- [x] event 기반 카운터 flush 구조 추가
- [x] HitProcessing profiling gate 추가
- [x] 40 / 80 Enemy 측정 완료
- [x] EngageCap 4 stress 참고 측정 완료
- [x] Runtime LOD 우선 제어 후보 여부 판단

## 브랜치

- `feature/ai-combat-collision-profiling`

## 요약

이번 PR은 AI Runtime LOD 후보 중 `Combat Collision / Hit Window` 축을 분리 측정한다.

P36 / P37에서 `Engage / Alert / Observe` 계층과 assignment cap이 정리되었기 때문에, 이번 브랜치에서는 `WeaponActor`, attack montage, AnimNotify route를 유지한 상태에서 hit window / overlap / hit processing 경로만 계측했다.

결론적으로 `DisableEnemyHitProcessing` gate는 의도대로 `HitProcessing / CombatSignal` 경로를 차단했지만, 40 / 80 Enemy 및 EngageCap 4 stress 조건 모두에서 Frame / Game p95 개선은 유의미하지 않았다. 따라서 `Combat Collision / HitProcessing`은 Runtime LOD v1의 우선 제어 후보로 보지 않고 후순위로 닫는다.

## 주요 변경

```text
1. Combat Collision profiling counter 추가
   - event 지점에서는 카운터만 누적
   - UCWorldSubsystem_CombatEngage::Tick()에서 FlushToCsv()
   - CSV 해석 기준을 *_FlushCount 컬럼으로 통일

2. Hit Window / Combat Signal route 계측 추가
   - Collision notify begin / end
   - Action collision window begin / end
   - Weapon collision window open / close
   - HitWindow open / close / overlap
   - HitProcessing
   - CombatSignal
   - CombatSignalCue route

3. HitProcessing profiling gate 추가
   - Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing
   - Enemy overlap과 hit window는 유지
   - RequestCombatSignalSource 이후 hit processing 경로만 skip

4. Combat Collision 전용 profiling map 추가
   - 40 Enemy
   - 80 Enemy

5. 측정 문서 정리
   - event 기반 CSV 계측 시행착오 기록
   - FullCombat baseline 정리
   - HitProcessingDisabled 결과 정리
   - EngageCap 4 stress 참고 측정 정리
   - Runtime LOD 우선 제어 후보 여부 결론 정리
```

## CVar

```text
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing
```

의미:

```text
0: 기본 hit processing 수행
1: Enemy hit processing 경로 skip
```

이번 CVar는 profiling 전용이다. 목적은 `HitWindow Overlap` 후보 수집과 `HitProcessing / CombatSignal` 이후 경로를 분리해서 보는 것이다.

## 계측 정책

이번 축은 AnimNotify, overlap delegate, combat signal request처럼 event 기반 호출이 많다.

초기에는 event 함수 안에서 직접 `CSV_CUSTOM_STAT_GLOBAL` 또는 `CSV_SCOPED_TIMING_STAT_GLOBAL`을 호출하는 방식을 시도했지만, 로그 호출이 확인되어도 CSV 컬럼에 안정적으로 남지 않았다.

따라서 최종 계측 기준은 다음으로 정리했다.

```text
event 지점:
-> FCombatCollisionProfilingCounters::Record...()로 count만 누적

flush 지점:
-> UCWorldSubsystem_CombatEngage::Tick()에서 FlushToCsv()

분석 기준:
-> *_FlushCount 컬럼만 공식 count로 사용
```

이 방식은 event 발생 자체를 직접 CSV에 남기는 것이 아니라, stable tick phase에서 누적값을 CSV로 내보낸다.

## 변경 파일

```text
Source/Portfolio/Core/Profiling/CCombatCollisionProfilingCounters.h
Source/Portfolio/Core/Profiling/CCombatCollisionProfilingCounters.cpp

Source/Portfolio/Notify/CAnimNotifyState_Collision.cpp
Source/Portfolio/Notify/CAnimNotify_CombatSignalCue.cpp
Source/Portfolio/Component/CActionComponent.cpp
Source/Portfolio/Component/CWeaponComponent.cpp
Source/Portfolio/Weapon/CWeaponActor.cpp
Source/Portfolio/Component/CCombatSignalSourceComponent.h
Source/Portfolio/Component/CCombatSignalSourceComponent.cpp
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp

Content/00_Profiling/00_AI_Performance/00_Map/09_CombatCollision/MAP_AIPerf_CombatCollision_40Enemy.umap
Content/00_Profiling/00_AI_Performance/00_Map/09_CombatCollision/MAP_AIPerf_CombatCollision_80Enemy.umap

Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P38_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Combat_Collision_HitWindow_Measurement_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/Enemy_Mesh_Runtime_LOD_Measurements.md
```

## 측정 조건

```text
Capture Duration: 약 36~37초
Analysis Window: first 3s / last 3s trimmed
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6

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
```

## 대표 FullCombat Baseline

`20260711_104859 / 40`은 계측 검증에는 유효했지만 `HitWindow Overlap / HitProcessing`이 348로 과다하게 튀었다. 대표 baseline에서는 제외하고 참고 측정으로만 둔다.

| Case | CSV | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 | HitWindow Open / Close | HitWindow Overlap | HitProcessing | CombatSignal |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 40 Enemy / FullCombat | `Profile(20260711_111308).csv` | 12.6371ms | 12.6204ms | 0.4925ms | 0.2055ms | 24 / 24 | 42 | 42 | 38 |
| 80 Enemy / FullCombat | `Profile(20260711_110514).csv` | 17.5514ms | 17.5532ms | 0.7883ms | 0.4098ms | 24 / 24 | 34 | 34 | 30 |

## HitProcessingDisabled 측정 결과

공통 확인:

```text
Engage 2 유지
Alert 6 유지
attack montage 유지
hit feedback 유지
GC 이벤트 없음
```

| Case | CSV | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 | HitWindow Open / Close | HitWindow Overlap | HitProcessing | CombatSignal | CombatSignalCue |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 40 Enemy / FullCombat | `Profile(20260711_111308).csv` | 12.6371ms | 12.6204ms | 0.4925ms | 0.2055ms | 24 / 24 | 42 | 42 | 38 | 8 |
| 40 Enemy / HitProcessingDisabled | `Profile(20260711_120705).csv` | 12.5523ms | 12.5267ms | 0.5171ms | 0.2042ms | 26 / 24 | 47 | 0 | 0 | 8 |
| 80 Enemy / FullCombat | `Profile(20260711_110514).csv` | 17.5514ms | 17.5532ms | 0.7883ms | 0.4098ms | 24 / 24 | 34 | 34 | 30 | 8 |
| 80 Enemy / HitProcessingDisabled | `Profile(20260711_121022).csv` | 17.4999ms | 17.4853ms | 0.7731ms | 0.4024ms | 26 / 25 | 30 | 0 | 0 | 8 |

## EngageCap 4 Stress 참고 측정

기본 `Engage 2 / Alert 6` 조건에서 frame 영향이 작았기 때문에, 전투 밀도를 올린 참고 측정을 추가했다.

Stress CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 4
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6
```

주의:

```text
debugging을 위해 Enemy collision capsule size를 10으로 줄인 상태에서 측정했다.
Enemy가 한쪽에 밀집해 공격하는 상황이 발생했으므로 HitWindow Overlap 수는 실제 기본 collision size보다 높게 잡힐 수 있다.
따라서 Overlap count는 순수 hit processing 비용이 아니라 전투 밀도 / 군집 후보 수 참고값으로 본다.
```

| Case | CSV | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 | HitWindow Open / Close | HitWindow Overlap | HitProcessing | CombatSignal | CombatSignalCue |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 80 Enemy / Engage 4 / FullCombat | `Profile(20260711_123416).csv` | 18.2174ms | 18.1873ms | 0.8017ms | 0.4137ms | 48 / 48 | 103 | 103 | 54 | 16 |
| 80 Enemy / Engage 4 / HitProcessingDisabled | `Profile(20260711_123952).csv` | 18.8448ms | 18.8774ms | 0.8388ms | 0.4236ms | 52 / 49 | 133 | 0 | 0 | 16 |

## 해석

`DisableEnemyHitProcessing 1`은 40 / 80 Enemy와 EngageCap 4 stress 조건에서 모두 `HitProcessing / CombatSignal`을 0으로 낮췄다.

반면 `HitWindow Open / Close`, `HitWindow Overlap`, `CombatSignalCue`는 유지됐다. 즉 gate 위치는 `collision window / overlap`이 아니라 `hit processing 이후 전투 신호 경로`를 분리한다.

Frame / Game p95는 유의미하게 개선되지 않았다.

```text
Engage 2:
-> 40 / 80 모두 Frame / Game p95 변화가 작음

Engage 4 stress:
-> 전투 밀도와 HitWindow count는 증가했지만 HitProcessingDisabled에서 Frame / Game p95 개선은 관찰되지 않음
```

따라서 이번 PR의 결론은 다음과 같다.

```text
Combat Collision / HitProcessing 축은 계측과 분리는 성공했다.
하지만 Runtime LOD v1의 우선 제어 후보로 보기는 어렵다.
attack montage가 이미 재생 중인 상태에서 hit processing만 끄는 정책은 gameplay 의미도 애매하다.
해당 축은 후순위로 닫고, 다음 측정 축은 Feedback Presentation으로 넘긴다.
```

## 검증

```text
1. PortfolioEditor Development build
2. 40 Enemy / FullCombat 측정
3. 40 Enemy / HitProcessingDisabled 측정
4. 80 Enemy / FullCombat 측정
5. 80 Enemy / HitProcessingDisabled 측정
6. 80 Enemy / EngageCap 4 / FullCombat stress 측정
7. 80 Enemy / EngageCap 4 / HitProcessingDisabled stress 측정
8. GC 이벤트 없음 확인
9. HitProcessing / CombatSignal gate 동작 확인
10. event 기반 계측은 *_FlushCount 기준으로 해석
```

## 제외 범위

```text
1. Feedback Presentation 제어
   - 다음 브랜치에서 측정한다.

2. WeaponActor 생성 / 제거 비용
   - 이전 Runtime LOD 측정 축에서 별도로 다뤘다.

3. Combat action 자체 제거
   - 이번 PR은 attack montage를 유지한 상태의 hit window / processing 분리만 다룬다.

4. Proxy / actor count 최적화
   - Runtime LOD 후속 후보로 유지한다.
```

## 후속 작업

```text
feature/ai-feedback-presentation-profiling
-> Niagara / trail / sound / camera shake / cue route 비용 분리 측정
```
