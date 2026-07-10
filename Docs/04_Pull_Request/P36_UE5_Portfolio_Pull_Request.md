# UE5 Portfolio Pull Request

## 제목

**P36: AI AlertCap 비교 측정 및 Assignment Cap 제어 추가**

## 날짜

**2026.07.10**

## 상태

- [x] 작업 방향 정리
- [x] 코드 / 에셋 / 문서 반영
- [x] 40 / 80 Enemy 비교 측정 완료

## 브랜치

- `feature/ai-alert-cap-comparison`

## 요약

AI Runtime LOD 정책에서 `Alert` 상태로 유지할 Enemy 수가 runtime cost에 미치는 영향을 분리 측정했다.

P35에서는 `Engage / Alert / Idle` 계층화와 BT service interval split을 정리했다. 이번 PR은 그 후속으로 `AlertCap 6 / 40`을 같은 코드 경로에서 비교할 수 있도록 `CombatEngage` assignment cap을 CVar로 분리하고, Alert 후보 수가 실제로 어떤 비용을 증가시키는지 확인한다.

## 주요 변경

```text
1. CombatEngage assignment cap CVar 추가
   - EngageCap / AlertCap을 CVar로 제어
   - 기본값은 기존 정책과 동일하게 Engage 2 / Alert 6 유지

2. AlertCap 비교용 profiling map 준비
   - 40 Enemy / 80 Enemy 비교 map 추가
   - 대표 측정은 전용 `MAP_AIPerf_AlertCap_40Enemy / 80Enemy` 기준으로 수행
   - 전용 map은 AlertCap 비교를 BTUpdateInterval 측정 흐름과 분리하기 위해 유지

3. AlertCap 비교 측정 문서 추가
   - 측정 조건
   - 대표값 채택 기준
   - 40 / 80 Enemy 결과표
   - 최종 해석 정리
```

## CVar

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap
```

기본값:

```text
EngageCap 2
AlertCap 6
```

이번 비교에서는 EngageCap은 2로 고정하고, AlertCap만 6 / 40으로 변경했다.

## 변경 파일

```text
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.h

Content/00_Profiling/00_AI_Performance/00_Map/07_AlertCap/GM_AIPerf_AlertCap.uasset
Content/00_Profiling/00_AI_Performance/00_Map/07_AlertCap/MAP_AIPerf_AlertCap_40Enemy.umap
Content/00_Profiling/00_AI_Performance/00_Map/07_AlertCap/MAP_AIPerf_AlertCap_80Enemy.umap

Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P36_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_AlertCap_Comparison_Plan.md
```

## 측정 조건

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Camera: fixed camera
GC Event: none
Map: MAP_AIPerf_AlertCap_40Enemy / MAP_AIPerf_AlertCap_80Enemy
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
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

비교 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 40
```

## 대표값 채택

같은 조건에서 반복 측정한 뒤, 40 / 80 Enemy 모두에서 가장 일관적인 설명이 가능한 측정 세트를 대표값으로 채택했다.

```text
40 / AlertCap 6  : Profile(20260710_100333).csv
40 / AlertCap 40 : Profile(20260710_100515).csv
80 / AlertCap 6  : Profile(20260710_100737).csv
80 / AlertCap 40 : Profile(20260710_100921).csv
```

대표값 채택 기준:

```text
1. 40 / 80 Enemy 모두 같은 방향으로 해석된다.
2. AlertCap 40에서 CharacterMovement p95가 일관되게 증가한다.
3. BT Tick p95와 service count는 크게 증가하지 않아, 병목 해석이 명확하다.
4. 따라서 AlertCap은 BT 호출 수보다 movement 후보 수를 제어하는 정책이라는 설명에 더 적합하다.
```

## 측정 결과

### 40 Enemy

| AlertCap | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 |
| ---: | ---: | ---: | ---: | ---: |
| 6 | 12.0038ms | 11.5967ms | 0.5394ms | 0.2110ms |
| 40 | 13.0781ms | 13.1117ms | 1.3609ms | 0.1970ms |

### 80 Enemy

| AlertCap | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 |
| ---: | ---: | ---: | ---: | ---: |
| 6 | 17.5270ms | 17.5329ms | 0.8072ms | 0.4167ms |
| 40 | 19.1846ms | 19.1422ms | 1.6427ms | 0.4014ms |

## 호출 수 비교

### 40 Enemy

| AlertCap | AIContext Count | AIIntent Count | EngageContext Count |
| ---: | ---: | ---: | ---: |
| 6 | 11640 | 6040 | 580 |
| 40 | 11600 | 5920 | 576 |

### 80 Enemy

| AlertCap | AIContext Count | AIIntent Count | EngageContext Count |
| ---: | ---: | ---: | ---: |
| 6 | 22640 | 11920 | 566 |
| 40 | 23120 | 11840 | 576 |

## 해석

`AlertCap 40`은 40 / 80 Enemy 모두에서 `CharacterMovement p95`를 크게 증가시켰다.

```text
40 Enemy: 0.5394ms -> 1.3609ms (+0.8215ms)
80 Enemy: 0.8072ms -> 1.6427ms (+0.8355ms)
```

반면 `BT Tick p95`는 증가하지 않았다.

```text
40 Enemy: 0.2110ms -> 0.1970ms
80 Enemy: 0.4167ms -> 0.4014ms
```

호출 수 역시 핵심 병목으로 보기 어렵다. 

40 Enemy에서는 모든 service count가 소폭 감소했고, 80 Enemy에서는 AIContext Count와 EngageContext Count가 소폭 증가했지만 BT Tick p95는 오히려 감소했다. 따라서 이번 차이는 BT service 호출 수 증가보다 Alert 후보 증가에 따른 CharacterMovement / animation work 증가로 해석하는 편이 더 타당하다.

따라서 이번 PR의 결론은 다음과 같다.

```text
AlertCap은 BT Tick 자체를 줄이는 정책이 아니라,
Alert movement 후보 수를 제한해 CharacterMovement / animation work를 줄이는 정책이다.
```

## 결론

Alert assignment cap은 Runtime LOD v1에서 유지할 가치가 있는 핵심 정책이다.

P35에서 정리한 `Engage / Alert / Idle` 계층화는 단순 상태 정리가 아니라, 실제 movement 후보 수를 제한해 runtime cost를 줄이는 정책으로 확인됐다.

## 검증

```text
1. 40 Enemy / AlertCap 6 측정
2. 40 Enemy / AlertCap 40 측정
3. 80 Enemy / AlertCap 6 측정
4. 80 Enemy / AlertCap 40 측정
5. first 3s / last 3s trim 후 p95 기준 비교
6. Actor / Tick count가 동일한 상태에서 CharacterMovement p95 증가 확인
7. BT Tick p95가 핵심 증가축이 아님을 확인
```

## 제외 범위

```text
1. Observe / Aware 상태 추가
   - 다음 브랜치에서 별도로 검토한다.

2. Collision / Hit Window 측정
   - 별도 측정 축으로 분리한다.

3. Feedback Presentation 측정
   - Niagara / trail / sound / camera shake 비용은 별도 축으로 분리한다.

4. 최종 Runtime LOD component 구현
   - 이번 PR은 cap 제어와 비교 측정까지 다룬다.
```

## 후속 작업

```text
1. Observe / Aware 상태 분리 검토
2. Collision / Hit Window 측정
3. Feedback Presentation 측정
4. Component Tick Audit
5. Perception Active Budget / Cap 검토
```

## 관련 문서

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_AlertCap_Comparison_Plan.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_BT_Update_Interval_LOD_Result_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Debugging_Obstacle_Note.md
Docs/07_Profiling/AI_Performance/CSV_Analysis_Guide.md
```
