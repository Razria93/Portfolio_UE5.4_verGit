# Enemy Mesh Runtime LOD Measurements

## 목적

이 문서는 `P35: AI Runtime LOD 정책 정리`에서 수행하는 Enemy mesh runtime LOD 측정 결과를 누적 기록한다.

측정 목표:

```text
EnemyMeshMode 0 / 1 / 2의 비용 차이를 비교한다.
mesh visibility off가 render 비용만 줄이는지, pose / socket update 비용까지 함께 줄이는지 확인한다.
WeaponActor socket follow 유지 여부를 함께 기록한다.
```

---

## 공통 측정 조건

```text
Map: MAP_AIPerf_40Enemy
Enemy: 40 placed AIPerf Enemy
State: Engage
Log State: -noailogging
PIE: F11 fullscreen
PlayerStart: near Enemy
Stats: stat unit / stat game / stat ai
CSV: csvprofile start / csvprofile stop
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
```

`Analysis Window`는 UE 자동 종료 기능이 아니라 CSV 분석 기준이다. CSV는 전체 capture duration을 기록하고, 비교값 계산 시 중앙 30초 구간만 사용한다.

---

## Mode 정의

```text
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
-> VisibleDefault
-> ACEnemy SkeletalMesh visibility on
-> 기존 pose update 설정 복구

Portfolio.AI.RuntimeLOD.EnemyMeshMode 1
-> HiddenKeepPose
-> ACEnemy SkeletalMesh visibility off
-> pose / bone / socket update 유지

Portfolio.AI.RuntimeLOD.EnemyMeshMode 2
-> HiddenAllowPoseSkip
-> ACEnemy SkeletalMesh visibility off
-> visibility 기반 pose skip 허용
```

---

## Summary Table

| Case | CSV | Enemy | Mode | Window | Frame p95 | Game p95 | GPU p95 | Render p95 | BT Tick p95 | AIPerception p95 | DrawCalls p95 | Primitives p95 | SkeletalMesh Tick | Weapon Socket Follow | Note |
| --- | --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| M00 | `Profile(20260703_143400).csv` | 40 | 0 VisibleDefault | 3.861s-33.861s | 12.6880ms | 12.7354ms | 8.4217ms | 0.1110ms | 0.3067ms | 0.1252ms | 572 | 4,181,001 | 82 | 정상 기준 | Mode 0 comparison baseline |

---

## Case M00 - 40 Enemy / Engage / EnemyMeshMode 0

원본 CSV:

```text
Portfolio/Csvprofile/Profile(20260703_143400).csv
```

사용자 기록:

```text
Case: 40 Enemy / Engage / EnemyMeshMode 0
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
PlayerStart near Enemy
Mode: 0 VisibleDefault
```

분석 구간:

```text
Total Duration: 37.721s
Analysis Window: 3.861s - 33.861s
Window Duration: 30.001s
Frames: 2593
```

주요 지표:

| Metric | Avg | p95 | p99 | Max |
| --- | ---: | ---: | ---: | ---: |
| FrameTime | 11.5699ms | 12.6880ms | 13.4509ms | 15.1869ms |
| GameThreadTime | 11.5626ms | 12.7354ms | 13.5110ms | 15.6361ms |
| GPUTime | 7.2633ms | 8.4217ms | 8.9492ms | 9.7528ms |
| RenderThreadTime | 0.0897ms | 0.1110ms | 0.1306ms | 0.1804ms |
| BehaviorTreeTick | 0.2330ms | 0.3067ms | 0.3395ms | 0.5034ms |
| AIPerception | 0.0979ms | 0.1252ms | 0.1659ms | 0.2450ms |
| BT_UpdateAIContext | 0.1438ms | 0.1612ms | 0.2026ms | 0.3166ms |
| BT_UpdateAIIntentState | 0.0189ms | 0.0259ms | 0.0367ms | 0.1614ms |
| BT_UpdateEngageContext | 0.0018ms | 0.0022ms | 0.0026ms | 0.0124ms |
| CombatEngage_Tick | 0.0008ms | 0.0061ms | 0.0070ms | 0.0123ms |
| CombatEngage_RebuildAssignments | 0.0006ms | 0.0057ms | 0.0065ms | 0.0117ms |

Render / count:

| Metric | Avg | p95 | p99 | Max |
| --- | ---: | ---: | ---: | ---: |
| RHI/DrawCalls | 488.6479 | 572 | 601 | 625 |
| RHI/PrimitivesDrawn | 2,804,543 | 4,181,001 | 4,747,162 | 4,869,868 |
| Ticks/SkeletalMeshComponent | 82 | 82 | 82 | 82 |
| Ticks/CEnemy | 40 | 40 | 40 | 40 |
| Ticks/CAIController | 40 | 40 | 40 | 40 |
| Ticks/BehaviorTreeComponent | 39.69 | 40 | 40 | 40 |
| ActorCount/CEnemy | 80 | 80 | 80 | 80 |
| ActorCount/CAIController | 40 | 40 | 40 | 40 |
| ActorCount/CWeaponActor | 42 | 42 | 42 | 42 |

메모:

```text
CSV ActorCount/CEnemy는 80으로 기록된다.
Ticks/CEnemy와 Ticks/CAIController는 40으로 기록되므로, 활성 플레이 기준 Enemy 수는 40으로 해석한다.
ActorCount/CEnemy는 PIE/editor world duplication 또는 CSV actor counter 집계 방식의 영향을 받을 수 있다.
```

