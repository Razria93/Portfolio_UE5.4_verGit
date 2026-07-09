# AI AlertCap Comparison Plan

## 목적

`CombatEngage` assignment cap이 AI Runtime LOD 비용에 주는 영향을 분리 측정한다.

P35에서 `Engage 2 / Alert 6 / Idle` 계층화와 BT service interval split은 안정화됐다.
다만 Alert 후보 수 제한 자체가 movement 후보와 BT service work를 얼마나 줄였는지는 별도로 분리하지 않았다.

이번 측정은 `AlertCap 6`과 `AlertCap 40`을 같은 map / 같은 코드 경로에서 비교한다.

## 측정 질문

```text
AlertCap을 6에서 40으로 늘리면
Alert 상태 Enemy 수가 늘어나면서
Movement / CharacterMovement / BT service work / AIContext request 수가 얼마나 증가하는가?
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

## 측정 템플릿

```text
Case: 40 Enemy / AlertCapComparison / AlertCap 6
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Map: MAP_AIPerf_BTUpdateInterval_40Enemy
Camera: fixed camera

CVar:
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit 1
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit 0
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

Observed:
- EngageCap / AlertCap summary 출력 확인
- Engage 2 유지 여부
- Alert 6 유지 여부
- 나머지 Idle 유지 여부
- GC 이벤트 여부
```

```text
Case: 40 Enemy / AlertCapComparison / AlertCap 40
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Map: MAP_AIPerf_BTUpdateInterval_40Enemy
Camera: fixed camera

CVar:
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit 1
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit 0
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 40

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

Observed:
- EngageCap / AlertCap summary 출력 확인
- Engage 2 유지 여부
- Alert 수 증가 여부
- Alert 확대로 movement 후보가 증가하는지
- GC 이벤트 여부
```

80 Enemy 측정은 같은 템플릿에서 `Map`과 `Case`의 Enemy 수만 바꾼다.

## 우선 지표

```text
FinalEngage / FinalAlert / FinalTotal
AIContext Count
AIIntent Count
EngageContext Count
BehaviorTreeTick p95
BT_UpdateAIContext p95
BT_UpdateAIIntentState p95
CharacterMovement p95
FrameTime p95
GameThreadTime p95
```

## 해석 기준

`AlertCap 40`에서 FinalAlert가 증가하고 CharacterMovement / AIContext / AIIntent Count가 함께 증가하면,
Alert assignment cap은 movement 후보와 BT service work를 줄이는 유효한 Runtime LOD 정책으로 본다.

Frame / GameThread p95가 크게 바뀌지 않아도 호출 수와 movement 후보 수가 줄면 정책 효과는 인정한다.
P35의 BT interval split과 마찬가지로, 이 측정은 frame gain보다 work reduction과 상태 계층화 효과를 우선 본다.
