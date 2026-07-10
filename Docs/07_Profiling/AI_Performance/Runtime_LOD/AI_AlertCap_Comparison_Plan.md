# AI AlertCap 비교 측정 계획 및 결과

## 목적

`CombatEngage` assignment cap이 AI Runtime LOD 비용에 주는 영향을 분리 측정한다.

P35에서는 `Engage 2 / Alert 6 / Idle` 계층화와 BT service interval split을 정리했다. 다만 Alert 후보 수 제한 자체가 movement 후보와 BT service work를 얼마나 줄이는지는 별도로 분리하지 않았다.

이번 측정은 `AlertCap 6`과 `AlertCap 40`을 같은 map / 같은 코드 경로에서 비교한다.

## 측정 질문

```text
AlertCap을 6에서 40으로 늘리면
Alert 상태 Enemy 수가 증가하면서
Movement / CharacterMovement / BT service work / AIContext request가 얼마나 증가하는가?
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

`AlertCap 40` 측정에서는 EngageCap은 그대로 2로 유지하고 AlertCap만 40으로 변경한다.

## 공통 측정 조건

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Map: MAP_AIPerf_BTUpdateInterval_40Enemy / MAP_AIPerf_BTUpdateInterval_80Enemy
Camera: fixed camera
GC Event: none 권장
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit 1
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit 0
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

## 권장 측정 순서

1. 40 Enemy / EngageCap 2 / AlertCap 6
2. 40 Enemy / EngageCap 2 / AlertCap 40
3. 80 Enemy / EngageCap 2 / AlertCap 6
4. 80 Enemy / EngageCap 2 / AlertCap 40

## 대표값 채택 기준

AlertCap 비교는 같은 조건에서 반복 측정했다.

최종 표에는 40 / 80 Enemy 모두에서 가장 일관적인 설명이 가능한 측정 세트를 대표값으로 사용한다.

```text
40 / AlertCap 6  : Profile(20260710_100333).csv
40 / AlertCap 40 : Profile(20260710_100515).csv
80 / AlertCap 6  : Profile(20260710_100737).csv
80 / AlertCap 40 : Profile(20260710_100921).csv
```

대표값 채택 이유:

```text
1. 40 / 80 Enemy 모두 같은 방향으로 해석된다.
2. AlertCap 40에서 CharacterMovement p95가 일관되게 증가한다.
3. BT Tick p95와 service count는 크게 증가하지 않아, 병목 해석이 더 명확하다.
4. 따라서 AlertCap은 BT 호출 수보다 movement 후보 수를 제어하는 정책이라는 설명에 더 적합하다.
```

## 40 Enemy 측정 결과

측정 파일:

```text
AlertCap 6  : Profile(20260710_100333).csv
AlertCap 40 : Profile(20260710_100515).csv
```

로그 확인:

```text
GC Event: none
EngageAssignmentRebuildSummary: provided log에 포함되지 않음
```

제공된 log에는 `EngageAssignmentRebuildSummary`가 포함되지 않았다. 따라서 FinalEngage / FinalAlert 확정 수는 log summary 근거가 아니라, 측정 시 지정한 CVar와 사용자의 관찰 조건을 기준으로 해석한다.

### 40 Enemy 비교표

| Metric | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| Capture Duration | 37.11s | 36.55s | -0.56s |
| Analysis Window | 31.12s | 30.57s | -0.55s |
| Frame p95 | 12.0038ms | 13.0781ms | +1.0743ms |
| Game p95 | 11.5967ms | 13.1117ms | +1.5150ms |
| GPU p95 | 10.3942ms | 9.6148ms | -0.7794ms |
| RenderThread p95 | 0.1215ms | 0.1167ms | -0.0048ms |
| BT Tick p95 | 0.2110ms | 0.1970ms | -0.0140ms |
| AIPerception p95 | 0.0846ms | 0.0731ms | -0.0115ms |
| CharacterMovement p95 | 0.5394ms | 1.3609ms | +0.8215ms |
| BT_UpdateAIContext p95 | 0.1451ms | 0.1390ms | -0.0061ms |
| BT_UpdateAIIntentState p95 | 0.0233ms | 0.0424ms | +0.0191ms |
| BT_UpdateEngageContext p95 | 0.0023ms | 0.0069ms | +0.0046ms |
| CombatEngage Tick p95 | 0.0147ms | 0.0154ms | +0.0007ms |
| CombatEngage Rebuild p95 | 0.0141ms | 0.0150ms | +0.0009ms |
| AnimationParallelEvaluation p95 | 3.4510ms | 3.8304ms | +0.3794ms |
| DrawCalls p95 | 827 | 831 | +4 |
| PrimitivesDrawn p95 | 3247714 | 2971918 | -275796 |

### 40 Enemy 호출 수 비교

| Counter | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| AIContext Count | 11640 | 11600 | -40 |
| AIIntent Count | 6040 | 5920 | -120 |
| EngageContext Count | 580 | 576 | -4 |
| AIContext Default Interval Count | 11640 | 11600 | -40 |
| AIIntent Default Interval Count | 6040 | 5920 | -120 |

### 40 Enemy Actor / Tick 확인

| Metric | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| ActorCount/CEnemy | 80 | 80 | 0 |
| ActorCount/CAIController | 40 | 40 | 0 |
| ActorCount/CWeaponActor | 41 | 41 | 0 |
| ActorCount/TotalActorCount | 316 | 316 | 0 |
| Ticks/CEnemy | 40 | 40 | 0 |
| Ticks/CAIController | 40 | 40 | 0 |
| Ticks/BehaviorTreeComponent | 40 | 40 | 0 |
| Ticks/CharacterMovementComponent | 41 | 41 | 0 |
| Ticks/PathFollowingComponent | 41 | 41 | 0 |

## 40 Enemy 해석

40 Enemy에서는 `AlertCap 40`에서 `CharacterMovement p95`가 가장 크게 증가했다.

```text
0.5394ms -> 1.3609ms
+0.8215ms
```

Frame / Game p95도 함께 증가했다.

```text
Frame p95: 12.0038ms -> 13.0781ms
Game p95 : 11.5967ms -> 13.1117ms
```

반면 BT Tick p95와 AIContext p95는 증가하지 않았고, 호출 수는 측정 window 차이 수준에서 오히려 소폭 감소했다. 따라서 40 Enemy 결과의 핵심은 BT 호출 수 증가가 아니라 Alert 후보 증가에 따른 movement / animation work 증가다.

## 80 Enemy 측정 결과

측정 파일:

```text
AlertCap 6  : Profile(20260710_100737).csv
AlertCap 40 : Profile(20260710_100921).csv
```

로그 확인:

```text
GC Event: none
EngageAssignmentRebuildSummary: provided log에 포함되지 않음
```

### 80 Enemy 비교표

| Metric | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| Capture Duration | 37.07s | 36.99s | -0.08s |
| Analysis Window | 31.09s | 31.00s | -0.09s |
| Frame p95 | 17.5270ms | 19.1846ms | +1.6576ms |
| Game p95 | 17.5329ms | 19.1422ms | +1.6093ms |
| GPU p95 | 11.3695ms | 11.0659ms | -0.3036ms |
| RenderThread p95 | 0.1122ms | 0.1171ms | +0.0049ms |
| BT Tick p95 | 0.4167ms | 0.4014ms | -0.0153ms |
| AIPerception p95 | 0.1069ms | 0.1093ms | +0.0024ms |
| CharacterMovement p95 | 0.8072ms | 1.6427ms | +0.8355ms |
| BT_UpdateAIContext p95 | 0.2837ms | 0.2764ms | -0.0073ms |
| BT_UpdateAIIntentState p95 | 0.0786ms | 0.0807ms | +0.0021ms |
| BT_UpdateEngageContext p95 | 0.0073ms | 0.0071ms | -0.0002ms |
| CombatEngage Tick p95 | 0.0208ms | 0.0240ms | +0.0032ms |
| CombatEngage Rebuild p95 | 0.0203ms | 0.0236ms | +0.0033ms |
| AnimationParallelEvaluation p95 | 5.8641ms | 6.3521ms | +0.4880ms |
| DrawCalls p95 | 1348 | 1345 | -3 |
| PrimitivesDrawn p95 | 5246078 | 5152326 | -93752 |

### 80 Enemy 호출 수 비교

| Counter | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| AIContext Count | 22640 | 23120 | +480 |
| AIIntent Count | 11920 | 11840 | -80 |
| EngageContext Count | 566 | 576 | +10 |
| AIContext Default Interval Count | 22640 | 23120 | +480 |
| AIIntent Default Interval Count | 11920 | 11840 | -80 |

### 80 Enemy Actor / Tick 확인

| Metric | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| ActorCount/CEnemy | 160 | 160 | 0 |
| ActorCount/CAIController | 80 | 80 | 0 |
| ActorCount/CWeaponActor | 81 | 81 | 0 |
| ActorCount/TotalActorCount | 476 | 476 | 0 |
| Ticks/CEnemy | 80 | 80 | 0 |
| Ticks/CAIController | 80 | 80 | 0 |
| Ticks/BehaviorTreeComponent | 80 | 80 | 0 |
| Ticks/CharacterMovementComponent | 81 | 81 | 0 |
| Ticks/PathFollowingComponent | 81 | 81 | 0 |

## 80 Enemy 해석

80 Enemy에서도 40 Enemy와 같은 경향이 유지됐다.

`AlertCap 40`에서 가장 크게 증가한 축은 `CharacterMovement p95`다.

```text
0.8072ms -> 1.6427ms
+0.8355ms
```

Frame / Game p95도 함께 증가했다.

```text
Frame p95: 17.5270ms -> 19.1846ms
Game p95 : 17.5329ms -> 19.1422ms
```

BT Tick p95와 AIContext p95는 증가하지 않았다. 호출 수는 AIContext Count만 소폭 증가했고, AIIntent Count는 오히려 소폭 감소했다. 따라서 80 Enemy에서도 주된 차이는 BT service 호출 수가 아니라 Alert 대상 증가에 따른 movement / animation work다.

Actor / Tick count는 고정되어 있으므로, 이번 비교는 객체 수 차이가 아니라 assignment cap이 실제 행동 후보 수를 조절한 결과로 본다.

## 최종 결론

40 / 80 Enemy 모두에서 `AlertCap 40`은 `AlertCap 6`보다 `CharacterMovement p95`를 크게 증가시켰다.

```text
40 Enemy: 0.5394ms -> 1.3609ms (+0.8215ms)
80 Enemy: 0.8072ms -> 1.6427ms (+0.8355ms)
```

Frame / Game p95도 두 조건 모두 `AlertCap 40`에서 증가했다.

따라서 Alert assignment cap은 Runtime LOD v1에서 유지할 가치가 있는 핵심 정책으로 본다. 효과의 본질은 BT Tick 자체를 크게 줄이는 것이 아니라, Alert movement 후보와 그에 딸린 CharacterMovement / animation work를 제한하는 것이다.

후속 작업에서는 이 cap을 고정한 상태에서 Observe / Aware 상태 분리 또는 Collision / Feedback 축 측정으로 넘어간다.
