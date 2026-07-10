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

40 Enemy에서 차이가 명확하지 않으면 80 Enemy 비교를 우선한다.

## 40 Enemy 측정 결과

측정 파일:

```text
AlertCap 6  : Profile(20260710_035439).csv
AlertCap 40 : Profile(20260710_035713).csv
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
| Capture Duration | 37.00s | 37.36s | +0.36s |
| Analysis Window | 31.01s | 31.38s | +0.37s |
| Frame p95 | 12.2727ms | 13.0416ms | +0.7689ms |
| Game p95 | 11.3917ms | 12.9853ms | +1.5936ms |
| GPU p95 | 10.4584ms | 9.6634ms | -0.7950ms |
| RenderThread p95 | 0.1125ms | 0.1233ms | +0.0108ms |
| BT Tick p95 | 0.2169ms | 0.2044ms | -0.0125ms |
| AIPerception p95 | 0.0847ms | 0.0804ms | -0.0043ms |
| CharacterMovement p95 | 0.5089ms | 1.2655ms | +0.7566ms |
| BT_UpdateAIContext p95 | 0.1494ms | 0.1421ms | -0.0073ms |
| BT_UpdateAIIntentState p95 | 0.0245ms | 0.0454ms | +0.0209ms |
| BT_UpdateEngageContext p95 | 0.0028ms | 0.0062ms | +0.0034ms |
| CombatEngage Tick p95 | 0.0143ms | 0.0150ms | +0.0007ms |
| CombatEngage Rebuild p95 | 0.0139ms | 0.0146ms | +0.0007ms |
| AnimationParallelEvaluation p95 | 3.4217ms | 3.9940ms | +0.5723ms |
| DrawCalls p95 | 829 | 832 | +3 |
| PrimitivesDrawn p95 | 3279644 | 2803652 | -475992 |

### 호출 수 비교

| Counter | AlertCap 6 | AlertCap 40 | Delta |
| --- | ---: | ---: | ---: |
| AIContext Count | 11560 | 11840 | +280 |
| AIIntent Count | 6000 | 6080 | +80 |
| EngageContext Count | 578 | 592 | +14 |
| AIContext Default Interval Count | 11560 | 11840 | +280 |
| AIIntent Default Interval Count | 6000 | 6080 | +80 |

### Actor / Tick 확인

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

`AlertCap 40`에서 가장 크게 증가한 축은 `CharacterMovement p95`다.

```text
0.5089ms -> 1.2655ms
+0.7566ms
```

같은 조건에서 Actor / Tick count는 변하지 않았다. 따라서 이번 차이는 단순 actor 수 증가가 아니라, 더 많은 Enemy가 Alert assignment를 받아 실제 이동 후보로 활성화되면서 CharacterMovement 쪽 work가 증가한 것으로 해석한다.

Frame / Game p95도 함께 증가했다.

```text
Frame p95: 12.2727ms -> 13.0416ms
Game p95 : 11.3917ms -> 12.9853ms
```

BT Tick p95와 AIContext p95는 거의 증가하지 않았다. AIContext / AIIntent 호출 수도 소폭 증가했지만, CharacterMovement 증가폭에 비하면 주된 차이는 아니다.

`AnimationParallelEvaluation p95`도 증가했다.

```text
3.4217ms -> 3.9940ms
```

AlertCap 증가로 더 많은 Enemy가 이동 / 방향전환 / 상태 갱신에 참여하면서 animation evaluation 쪽도 같이 흔들린 것으로 본다.

## 결론

40 Enemy 조건에서도 `AlertCap 6`은 유효한 Runtime LOD cap으로 보인다.

AlertCap을 40으로 늘리면 전체 actor / tick count는 같아도 실제 Alert movement 후보가 늘어나고, 그 결과 CharacterMovement p95와 GameThread p95가 증가한다. 따라서 Engage / Alert / Idle 계층화는 단순 상태 정리가 아니라 movement 후보 수를 제한하는 성능 정책으로 의미가 있다.

이번 결과만으로 최종 cap 값을 확정하지는 않는다. 40 Enemy에서는 차이가 관찰됐지만, 80 Enemy에서 같은 경향이 유지되는지 확인해야 한다.

## 다음 측정

다음은 같은 조건으로 80 Enemy를 측정한다.

```text
Case: 80 Enemy / AlertCapComparison / AlertCap 6
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6
```

```text
Case: 80 Enemy / AlertCapComparison / AlertCap 40
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 40
```

80 Enemy에서도 `AlertCap 40`에서 CharacterMovement / GameThread p95가 증가하면, Alert assignment cap은 Runtime LOD v1의 핵심 정책으로 유지한다.
