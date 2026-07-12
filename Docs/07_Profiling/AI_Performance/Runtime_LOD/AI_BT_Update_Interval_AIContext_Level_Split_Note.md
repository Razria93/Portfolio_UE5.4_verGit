# AI BT Update Interval AIContext Level Split Note

## 현재 상태

이 문서는 `AIContextService` 호출수도 Runtime LOD precision에 따라 줄일 수 있는지 확인한 과거 실험 기록이다.

이후 Runtime LOD tier snapshot 설계 검토에서 `AIContext`는 Blackboard / CombatRole / target awareness를 갱신하는 producer 계층으로 정리했다.
따라서 현재 정책에서는 `AIContext` interval split을 적용하지 않는다.

현재 적용 기준:

```text
AIContext: 기본 interval 유지
AIIntentState: Runtime LOD tier 기반 interval 조정
EngageContext: 기본 interval 유지
```

관련 후속 문서:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Tier_Snapshot_Refactor_Plan.md
```

## 목적

Assignment warmup과 Engage / Alert cap으로 전투 참여 계층이 안정화된 상태에서 `AIContextService` 호출수도 Runtime LOD precision에 따라 줄일 수 있는지 확인한다.

이 작업은 `AIContext` 내부 계산량을 줄이는 최적화가 아니다.
서비스 자체의 다음 tick interval을 `High / Reduced / Low` precision에 따라 다르게 배정하는 호출수 분리 실험이다.

## 변경 전 정책

```text
AIContext:
항상 0.1s

AIIntentState:
Mode 0: High 0.2 / Reduced 0.2 / Low 0.2
Mode 1: High 0.2 / Reduced 0.3 / Low 0.3
Mode 2: High 0.2 / Reduced 0.3 / Low 0.5

EngageContext:
항상 0.1s
```

이 구조에서는 `AIIntentState` 호출수만 감소했고, `AIContext`는 request/context producer 역할 때문에 Mode와 무관하게 거의 유지됐다.

## 변경 후 정책

```text
AIContext:
Mode 0: High 0.1 / Reduced 0.1 / Low 0.1
Mode 1: High 0.1 / Reduced 0.2 / Low 0.2
Mode 2: High 0.1 / Reduced 0.2 / Low 0.4

AIIntentState:
Mode 0: High 0.2 / Reduced 0.2 / Low 0.2
Mode 1: High 0.2 / Reduced 0.3 / Low 0.3
Mode 2: High 0.2 / Reduced 0.3 / Low 0.5

EngageContext:
항상 0.1s
```

`EngageContext`는 combat action 판단과 직접 연결되므로 이번 실험에서도 기본 interval을 유지한다.

## CSV 확인 지표

직접 호출수:

```text
PortfolioAI_BT_UpdateAIContext_Count
PortfolioAI_BT_UpdateAIIntentState_Count
PortfolioAI_BT_UpdateEngageContext_Count
```

AIContext interval 선택 분포:

```text
PortfolioAI_BT_AIContextInterval_Default_Count
PortfolioAI_BT_AIContextInterval_Reduced_Count
PortfolioAI_BT_AIContextInterval_Aggressive_Count
```

AIIntentState interval 선택 분포:

```text
PortfolioAI_BT_AIIntentInterval_Default_Count
PortfolioAI_BT_AIIntentInterval_Reduced_Count
PortfolioAI_BT_AIIntentInterval_Aggressive_Count
```

## 판단 기준

성공 조건:

```text
Mode 1 / 2에서 AIContext Count가 Mode 0보다 감소한다.
AIContext interval preset count가 Mode별 정책과 일치한다.
AIIntentState Count 감소도 기존처럼 유지된다.
Engage 2 / Alert 6 / 나머지 Idle 계층이 유지된다.
Attack 진입이 깨지지 않는다.
```

주의 조건:

```text
AIContext Count는 줄었지만 Engage / Alert assignment가 흔들리면 gameplay-safe하지 않다.
Frame / Game p95 개선이 작아도 호출수 감소가 확인되면 service work reduction으로 기록한다.
Mode 2에서 반응 지연, assignment 누락, attack 진입 실패가 보이면 aggressive interval은 현재 combat-capable 조건에 부적합한 것으로 본다.
```

## 측정 조건

```text
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Camera: fixed camera

Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit 0
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit 0
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

권장 측정 순서:

```text
40 Enemy / BTUpdateIntervalMode 0
40 Enemy / BTUpdateIntervalMode 1
40 Enemy / BTUpdateIntervalMode 2

80 Enemy / BTUpdateIntervalMode 0
80 Enemy / BTUpdateIntervalMode 1
80 Enemy / BTUpdateIntervalMode 2
```

## 40 Enemy 측정 결과

측정 파일:

```text
Mode 0: Profile(20260709_191603).csv
Mode 1: Profile(20260709_191821).csv
Mode 2: Profile(20260709_192202).csv
```

측정 조건:

```text
Enemy Count: 40
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
```

핵심 결과:

| Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | EngageContext Count |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 11.5721ms | 10.9428ms | 0.2152ms | 11680 | 6040 | 584 |
| 1 | 11.7582ms | 11.0469ms | 0.1881ms | 6278 | 4140 | 576 |
| 2 | 11.6986ms | 11.0404ms | 0.0956ms | 3918 | 3090 | 578 |

AIContext interval preset:

| Mode | Default | Reduced | Aggressive |
| ---: | ---: | ---: | ---: |
| 0 | 11680 | 0 | 0 |
| 1 | 578 | 5700 | 0 |
| 2 | 580 | 900 | 2438 |

AIIntentState interval preset:

| Mode | Default | Reduced | Aggressive |
| ---: | ---: | ---: | ---: |
| 0 | 6040 | 0 | 0 |
| 1 | 302 | 3838 | 0 |
| 2 | 300 | 1180 | 1610 |

해석:

```text
AIContextService 호출수 레벨 분리는 정상 적용됐다.
Mode 1에서 AIContext Count는 11680 -> 6278로 약 46% 감소했다.
Mode 2에서 AIContext Count는 11680 -> 3918로 약 66% 감소했다.

AIIntentState Count도 기존 정책처럼 6040 -> 4140 -> 3090으로 감소했다.
EngageContext Count는 584 -> 576 -> 578로 유지됐다.

Frame / Game p95는 개선되지 않았다.
다만 BehaviorTreeTick p95는 0.2152ms -> 0.1881ms -> 0.0956ms로 감소했다.
따라서 이번 결과는 frame gain보다 BT service work reduction이 명확한 측정으로 본다.
```

주의:

```text
Mode 1 / 2에서 gameplay smoke가 유지되는지가 최종 판단 기준이다.
AIContext Count가 줄어도 Engage / Alert / Idle 계층이나 Attack 진입이 흔들리면 해당 interval은 combat-capable 조건에 부적합하다.
```

## 80 Enemy 측정 결과

측정 파일:

```text
Mode 0: Profile(20260709_202805).csv
Mode 1: Profile(20260709_202920).csv
Mode 2: Profile(20260709_203937).csv
```

Mode 2 대표값 정정 기록:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_BT_Update_Interval_AIContext_Level_Split_80Enemy_Correction.md
```

측정 조건:

```text
Enemy Count: 80
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
Observed: 특이사항 없음
```

핵심 결과:

| Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | EngageContext Count |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 16.2377ms | 16.2519ms | 0.4001ms | 23600 | 12000 | 590 |
| 1 | 16.1284ms | 16.1593ms | 0.3787ms | 12216 | 8100 | 592 |
| 2 | 16.2984ms | 16.2689ms | 0.1641ms | 6947 | 5826 | 580 |

AIContext interval preset:

| Mode | Default | Reduced | Aggressive |
| ---: | ---: | ---: | ---: |
| 0 | 23600 | 0 | 0 |
| 1 | 594 | 11622 | 0 |
| 2 | 581 | 894 | 5472 |

AIIntentState interval preset:

| Mode | Default | Reduced | Aggressive |
| ---: | ---: | ---: | ---: |
| 0 | 12000 | 0 | 0 |
| 1 | 300 | 7800 | 0 |
| 2 | 298 | 1860 | 3668 |

해석:

```text
80 Enemy 조건에서도 AIContextService 호출수 레벨 분리는 정상 적용됐다.
Mode 1에서 AIContext Count는 23600 -> 12216으로 약 48% 감소했다.
Mode 2에서 AIContext Count는 23600 -> 6947로 약 71% 감소했다.

AIIntentState Count도 12000 -> 8100 -> 5826으로 감소했다.
EngageContext Count는 590 -> 592 -> 580으로 유지됐다.

BehaviorTreeTick p95는 0.4001ms -> 0.3787ms -> 0.1641ms로 감소했다.
Mode 1은 Frame / Game p95를 거의 유지하면서 AIContext / AIIntentState 호출수를 줄였다.
Mode 2는 호출수 감소와 BT Tick p95 감소가 가장 크다.
다만 Mode 2는 공격적 후보이므로 gameplay smoke와 반복 안정성은 보수적으로 본다.
```

정책 판단:

```text
Mode 1은 80 Enemy 조건에서 보수적인 AIContext / AIIntentState 호출수 감소 후보로 볼 수 있다.
Mode 2는 호출수와 BT Tick p95 감소 효과가 가장 크지만, combat-capable gameplay stress 조건에서는 반복 안정성 확인이 필요하다.
Mode 2는 더 먼 거리, 화면 밖, NonCombat / Dormant에 가까운 tier 후보로 우선 분류한다.
```
