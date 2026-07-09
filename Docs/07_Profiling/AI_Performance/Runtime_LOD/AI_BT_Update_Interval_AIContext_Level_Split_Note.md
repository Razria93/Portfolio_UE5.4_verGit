# AI BT Update Interval AIContext Level Split Note

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
