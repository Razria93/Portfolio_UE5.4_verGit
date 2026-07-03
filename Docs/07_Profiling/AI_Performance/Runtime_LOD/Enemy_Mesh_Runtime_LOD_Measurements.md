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
Enemy: case-dependent placed AIPerf Enemy
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
| M01 | `Profile(20260703_144556).csv` | 40 | 1 HiddenKeepPose | 3.989s-33.989s | 12.3922ms | 12.4150ms | 7.3382ms | 0.1998ms | 0.3057ms | 0.1233ms | 377.7 | 377,250 | 82 | 유지 | Mesh hidden, weapon animation path maintained |
| M02 | - | 40 | 2 HiddenAllowPoseSkip | - | - | - | - | - | - | - | - | - | - | 깨짐 | Gameplay unsafe. 정규 성능 측정 제외 |
| M03 | `Profile(20260703_161310).csv` | 80 | 0 VisibleDefault | 4.261s-34.261s | 21.2578ms | 21.2928ms | 9.3746ms | 0.1776ms | 0.5091ms | 0.2852ms | 583 | 3,771,918 | 162 | 정상 기준 | 80 Enemy Mode 0 comparison baseline |

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

---

## Case M01 - 40 Enemy / Engage / EnemyMeshMode 1

원본 CSV:

```text
Portfolio/Csvprofile/Profile(20260703_144556).csv
```

사용자 기록:

```text
Case: 40 Enemy / Engage / EnemyMeshMode 1
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
PlayerStart near Enemy
Mode: 1 HiddenKeepPose
```

관찰:

```text
mesh hidden 확인.
WeaponActor는 애니메이션 경로를 따라 정상적으로 이동한다.
combo attack 정상 실행.
공격 피드백인 trail / Niagara가 의도된 타이밍에 정상 재생된다.
```

여기서 `WeaponActor socket follow`는 WeaponActor가 hand / holster socket의 pose 갱신을 따라 정상 이동하는지 확인하는 항목이다.
`smoke`는 전체 기능 검증이 아니라 combo attack, hit, guard / parry, feedback timing 같은 기본 전투 흐름이 깨지지 않는지 확인하는 간단한 동작 검증을 뜻한다.

분석 구간:

```text
Total Duration: 37.978s
Analysis Window: 3.989s - 33.989s
Window Duration: 29.998s
Frames: 2667
```

주요 지표:

| Metric | Avg | p95 | p99 | Max |
| --- | ---: | ---: | ---: | ---: |
| FrameTime | 11.2479ms | 12.3922ms | 12.9690ms | 15.8627ms |
| GameThreadTime | 11.2436ms | 12.4150ms | 12.9589ms | 15.3482ms |
| GPUTime | 6.3080ms | 7.3382ms | 8.0136ms | 8.7060ms |
| RenderThreadTime | 0.1356ms | 0.1998ms | 0.2155ms | 0.3265ms |
| BehaviorTreeTick | 0.2326ms | 0.3057ms | 0.3366ms | 0.5585ms |
| AIPerception | 0.0969ms | 0.1233ms | 0.1615ms | 0.2493ms |
| BT_UpdateAIContext | 0.1437ms | 0.1608ms | 0.1868ms | 0.4830ms |
| BT_UpdateAIIntentState | 0.0186ms | 0.0255ms | 0.0364ms | 0.0620ms |
| BT_UpdateEngageContext | 0.0018ms | 0.0023ms | 0.0025ms | 0.0089ms |
| CombatEngage_Tick | 0.0008ms | 0.0063ms | 0.0071ms | 0.0134ms |
| CombatEngage_RebuildAssignments | 0.0006ms | 0.0059ms | 0.0067ms | 0.0131ms |

Render / count:

| Metric | Avg | p95 | p99 | Max |
| --- | ---: | ---: | ---: | ---: |
| RHI/DrawCalls | 343.5673 | 377.7 | 392 | 411 |
| RHI/PrimitivesDrawn | 357,255 | 377,250 | 386,943 | 397,490 |
| Ticks/SkeletalMeshComponent | 82 | 82 | 82 | 82 |
| Ticks/CEnemy | 40 | 40 | 40 | 40 |
| Ticks/CAIController | 40 | 40 | 40 | 40 |
| Ticks/BehaviorTreeComponent | 39.68 | 40 | 40 | 40 |
| ActorCount/CEnemy | 80 | 80 | 80 | 80 |
| ActorCount/CAIController | 40 | 40 | 40 | 40 |
| ActorCount/CWeaponActor | 42 | 42 | 42 | 42 |

Mode 0 대비:

```text
FrameTime p95: 12.6880ms -> 12.3922ms
GameThreadTime p95: 12.7354ms -> 12.4150ms
GPUTime p95: 8.4217ms -> 7.3382ms
RHI/DrawCalls p95: 572 -> 377.7
RHI/PrimitivesDrawn p95: 4,181,001 -> 377,250
Ticks/SkeletalMeshComponent: 82 유지
```

해석:

```text
Mode 1은 mesh visibility를 끄되 pose / socket update를 유지하는 비교다.
SkeletalMesh tick 수가 유지되고 WeaponActor가 animation path를 따라가므로, pose / socket update는 유지된 것으로 본다.
GPU p95, draw call, primitives drawn이 줄어 render 비용 분리 측정으로 유효하다.
GameThread / BT / AIPerception 값은 Mode 0과 큰 차이가 없어 AI update 비용과는 분리된 변화로 해석한다.
```

---

## Case M02 - 40 Enemy / Engage / EnemyMeshMode 2 Observation

정규 CSV 측정:

```text
수행하지 않음.
```

관찰:

```text
0 또는 1에서 시작한 뒤 PIE 중 2로 전환하면 WeaponActor가 전환 순간 위치에 고정된다.
Actor 자체 위치 이동은 유지될 수 있으나, combo attack 대상의 검 위치가 고정된다.
새로 Engage 조건에 들어온 대상도 동일하게 검 위치 고정 현상이 발생한다.

PIE 시작부터 2로 설정하면 Enemy actor가 위치 이동하지 않고 target을 향해 회전만 한다.
```

해석:

```text
Mode 2는 hidden 상태에서 pose / bone / socket update skip을 허용하는 극단 비교다.
이 모드는 WeaponActor socket follow, montage / notify 기반 전투 흐름, animation-driven 상태 갱신을 깨뜨릴 수 있다.
따라서 gameplay-safe Runtime LOD 후보가 아니다.
```

결정:

```text
Mode 2는 정규 성능 측정에서 제외한다.
P35의 Enemy mesh runtime LOD 비교는 Mode 0 VisibleDefault와 Mode 1 HiddenKeepPose를 중심으로 진행한다.
Mode 2는 pose skip이 전투 시스템에 미치는 부작용 관찰 자료로만 남긴다.
```

---

## Case M03 - 80 Enemy / Engage / EnemyMeshMode 0

원본 CSV:

```text
Portfolio/Csvprofile/Profile(20260703_161310).csv
```

사용자 기록:

```text
Case: 80 Enemy / Engage / EnemyMeshMode 0
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
PlayerStart near Enemy
Mode: 0 VisibleDefault
```

확인 항목:

```text
80 Enemy 정상 배치.
Engage 진입 가능.
Enemy끼리 피격 없음.
Enemy끼리 길막이 측정 불가능할 정도로 심하지 않음.
PlayerStart 기준 초반 Engage 진입이 너무 늦지 않음.
```

분석 구간:

```text
Total Duration: 38.523s
Analysis Window: 4.261s - 34.261s
Window Duration: 30.011s
Frames: 1501
```

주요 지표:

| Metric | Avg | p95 | p99 | Max |
| --- | ---: | ---: | ---: | ---: |
| FrameTime | 19.9937ms | 21.2578ms | 21.9128ms | 24.4188ms |
| GameThreadTime | 19.9878ms | 21.2928ms | 21.9116ms | 24.4064ms |
| GPUTime | 7.9800ms | 9.3746ms | 9.9311ms | 10.6772ms |
| RenderThreadTime | 0.1372ms | 0.1776ms | 0.2064ms | 0.2588ms |
| BehaviorTreeTick | 0.4151ms | 0.5091ms | 0.5679ms | 0.7296ms |
| AIPerception | 0.1706ms | 0.2852ms | 0.3662ms | 0.4847ms |
| BT_UpdateAIContext | 0.2504ms | 0.2890ms | 0.3346ms | 0.4316ms |
| BT_UpdateAIIntentState | 0.0350ms | 0.0458ms | 0.0540ms | 0.0925ms |
| BT_UpdateEngageContext | - | - | - | - |
| CombatEngage_Tick | 0.0012ms | 0.0059ms | 0.0070ms | 0.0195ms |
| CombatEngage_RebuildAssignments | 0.0010ms | 0.0055ms | 0.0067ms | 0.0190ms |

Render / count:

| Metric | Avg | p95 | p99 | Max |
| --- | ---: | ---: | ---: | ---: |
| RHI/DrawCalls | 470.4024 | 583 | 632 | 665 |
| RHI/PrimitivesDrawn | 2,844,471 | 3,771,918 | 4,847,502 | 5,290,218 |
| Ticks/SkeletalMeshComponent | 162 | 162 | 162 | 162 |
| Ticks/CEnemy | 80 | 80 | 80 | 80 |
| Ticks/CAIController | 80 | 80 | 80 | 80 |
| Ticks/BehaviorTreeComponent | 79.84 | 80 | 80 | 80 |
| ActorCount/CEnemy | 162 | 162 | 162 | 162 |
| ActorCount/CAIController | 80 | 80 | 80 | 80 |
| ActorCount/CWeaponActor | 85 | 85 | 85 | 85 |

40 Enemy Mode 0 대비:

```text
FrameTime p95: 12.6880ms -> 21.2578ms
GameThreadTime p95: 12.7354ms -> 21.2928ms
GPUTime p95: 8.4217ms -> 9.3746ms
BehaviorTreeTick p95: 0.3067ms -> 0.5091ms
AIPerception p95: 0.1252ms -> 0.2852ms
BT_UpdateAIContext p95: 0.1612ms -> 0.2890ms
Ticks/CEnemy: 40 -> 80
Ticks/CAIController: 40 -> 80
```

해석:

```text
80 Enemy Mode 0은 P35의 mesh runtime LOD 1차 비교 기준이다.
GameThread / FrameTime p95가 20ms를 넘으므로 60fps 기준에서는 이미 주의 구간에 들어간다.
GPU p95 증가는 제한적이지만 GameThread, BehaviorTreeTick, AIPerception, BT_UpdateAIContext가 Enemy 수 증가에 맞춰 증가한다.
다음 비교는 동일 80 Enemy 조건에서 Mode 1 HiddenKeepPose를 측정해 render visibility 제거가 Frame / GPU / draw call에 얼마나 영향을 주는지 확인한다.
```
