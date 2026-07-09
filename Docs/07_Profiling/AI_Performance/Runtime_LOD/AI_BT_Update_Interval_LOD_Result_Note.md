# AI BT Update Interval LOD Result Note

## Purpose

BTUpdateIntervalMode 0 / 1 / 2가 AssignmentGate 적용 상태에서 의도대로 동작하는지 기록한다.

이번 측정의 1차 기준은 Frame p95 개선이 아니라 다음 항목이다.

```text
AIIntentState 호출 수가 줄어드는지
Default / Reduced / Aggressive interval preset 분포가 정책대로 나뉘는지
AIContext 호출 수가 request/context producer 역할 때문에 유지되는지
Engage / Alert / Idle 계층화가 깨지지 않는지
```

## Common Conditions

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Camera: fixed camera
GC Event: none

Runtime LOD CVar:
EnemyMeshMode 0
EnemyAnimationMode 0
EnemyAnimationRefreshCounter 0
DisableEnemyWeaponActor 0
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
CanMoveDecoratorAudit 0
EnemyMovementMode 0
```

## 40 Enemy Result - AssignmentGate

측정 파일:

```text
Mode 0: Profile(20260708_213701).csv
Mode 1: Profile(20260708_213854).csv
Mode 2: Profile(20260708_214143).csv
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | Default Count | Reduced Count | Aggressive Count |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BT Final 40-0 | 0 | 11.5960ms | 10.7646ms | 0.1999ms | 11720 | 6080 | 6080 | 0 | 0 |
| BT Final 40-1 | 1 | 11.6864ms | 11.0142ms | 0.2020ms | 11760 | 4178 | 302 | 3876 | 0 |
| BT Final 40-2 | 2 | 11.7361ms | 10.8924ms | 0.2034ms | 11920 | 3090 | 304 | 1110 | 1676 |

해석:

```text
AIIntentState 호출 수는 6080 -> 4178 -> 3090으로 감소한다.
Mode 0은 Default만 선택한다.
Mode 1은 Default / Reduced를 선택한다.
Mode 2는 Default / Reduced / Aggressive를 모두 선택한다.
AIContext 호출 수는 유사하게 유지된다.
40 Enemy 조건에서 Frame / Game / BT Tick p95 개선은 작다.
```

## 40 Enemy Result - AssignmentLease

측정 파일:

```text
Mode 0: Profile(20260709_111406).csv
Mode 1: Profile(20260709_111732).csv
Mode 2: Profile(20260709_111916).csv
```

관측:

```text
BTUpdateIntervalMode 0 / 1 / 2 모두 Engage 2 / Alert 6 / 나머지 Idle이 안정적으로 유지됐다.
GC Event는 없었다.
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | Default Count | Reduced Count | Aggressive Count |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BT Lease 40-0 | 0 | 12.4386ms | 11.3758ms | 0.2058ms | 11800 | 6080 | 6080 | 0 | 0 |
| BT Lease 40-1 | 1 | 11.7803ms | 11.3470ms | 0.2016ms | 11720 | 4180 | 304 | 3876 | 0 |
| BT Lease 40-2 | 2 | 11.7323ms | 11.2466ms | 0.2075ms | 12280 | 3100 | 318 | 794 | 1988 |

해석:

```text
AssignmentLease 적용 후에도 AIIntentState 호출 수는 6080 -> 4180 -> 3100으로 감소한다.
Mode 1 / 2에서 호출 수를 줄여도 Engage / Alert / Idle assignment가 끊기지 않았다.
AIContext 호출 수는 request/context producer 역할 때문에 유지된다.
40 Enemy 조건에서 Frame / Game / BT Tick p95 개선은 오차 범위로 본다.
이번 결과는 frame gain보다 assignment 안정화와 service work reduction을 동시에 확인한 결과로 해석한다.
```

## 80 Enemy Result

측정 파일:

```text
Mode 0: Profile(20260708_224210).csv
Mode 1: Profile(20260708_224356).csv
Mode 2: Profile(20260708_224639).csv
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | EngageContext Count | Default Count | Reduced Count | Aggressive Count |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BT Final 80-0 | 0 | 17.0098ms | 16.9991ms | 0.3917ms | 22480 | 12080 | 560 | 12080 | 0 | 0 |
| BT Final 80-1 | 1 | 17.0418ms | 17.0108ms | 0.3812ms | 22960 | 8340 | - | 306 | 8034 | 0 |
| BT Final 80-2 | 2 | 17.1633ms | 17.1902ms | 0.3919ms | 22240 | 5578 | 556 | 296 | 1101 | 4181 |

해석:

```text
AIIntentState 호출 수는 12080 -> 8340 -> 5578로 감소한다.
Mode 0은 Default만 선택한다.
Mode 1은 Default / Reduced를 선택한다.
Mode 2는 Default / Reduced / Aggressive를 모두 선택한다.
AIContext 호출 수는 Mode와 무관하게 유사하게 유지된다.
80 Enemy 조건에서도 Frame / Game / BT Tick p95 개선은 뚜렷하지 않다.
```

## 40 Enemy Result - AssignmentWarmup 1.2

측정 파일:

```text
Mode 0: Profile(20260709_172705).csv
Mode 1: Profile(20260709_173756).csv
Mode 2: Profile(20260709_173016).csv
```

조건:

```text
EngageAssignmentWarmupTime 1.2
EngageAssignmentAudit 0
EngageAssignmentVerboseAudit 0
Engage 2 / Alert 6 / 나머지 Idle 안정 유지
GC Event: none
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext p95 | AIIntent p95 | EngageContext p95 | AIContext Count | AIIntent Count | EngageContext Count | Default Count | Reduced Count | Aggressive Count |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BT Warmup 40-0 | 0 | 11.6406ms | 10.9127ms | 0.2070ms | 0.1415ms | 0.0257ms | 0.0072ms | 12520 | 6520 | 624 | 6520 | 0 | 0 |
| BT Warmup 40-1 | 1 | 11.8182ms | 10.9590ms | 0.2047ms | 0.1406ms | 0.0099ms | 0.0071ms | 11640 | 4178 | 580 | 302 | 3876 | 0 |
| BT Warmup 40-2 | 2 | 11.8810ms | 10.9224ms | 0.2071ms | 0.1432ms | 0.0132ms | 0.0071ms | 11720 | 2970 | 582 | 304 | 810 | 1856 |

해석:

```text
Warmup 1.2 적용 후에도 AIIntentState 호출 수는 6520 -> 4178 -> 2970으로 감소한다.
EngageContext 호출 수는 624 -> 580 -> 582로 유지된다.
AIContext는 request/context producer 역할 때문에 유사하게 유지된다.
40 Enemy 조건에서는 Frame / Game p95 개선은 오차 범위로 본다.
```

## 80 Enemy Result - AssignmentWarmup 1.2

측정 파일:

```text
Mode 0: Profile(20260709_181038).csv
Mode 1: Profile(20260709_181231).csv
Mode 2: Profile(20260709_181433).csv
```

조건:

```text
EngageAssignmentWarmupTime 1.2
EngageAssignmentAudit 0
EngageAssignmentVerboseAudit 0
Engage 2 / Alert 6 / 나머지 Idle 안정 유지
GC Event: none
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext p95 | AIIntent p95 | EngageContext p95 | AIContext Count | AIIntent Count | EngageContext Count | Default Count | Reduced Count | Aggressive Count |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BT Warmup 80-0 | 0 | 16.7495ms | 16.7296ms | 0.3874ms | 0.2616ms | 0.0796ms | 0.0072ms | 22800 | 12080 | 570 | 12080 | 0 | 0 |
| BT Warmup 80-1 | 1 | 16.4366ms | 16.3790ms | 0.3836ms | 0.2564ms | 0.0154ms | 0.0073ms | 23280 | 8180 | 580 | 302 | 7878 | 0 |
| BT Warmup 80-2 | 2 | 16.1559ms | 16.2050ms | 0.3858ms | 0.2572ms | 0.0309ms | 0.0073ms | 23360 | 5752 | 582 | 298 | 1704 | 3750 |

해석:

```text
Warmup 1.2 적용 후 80 Enemy에서도 AIIntentState 호출 수는 12080 -> 8180 -> 5752로 감소한다.
Mode 1은 Reduced preset 중심으로 분포한다.
Mode 2는 Reduced / Aggressive preset으로 분산된다.
EngageContext 호출 수는 570 -> 580 -> 582로 유지된다.
AIContext 호출 수는 Mode와 무관하게 유사하게 유지된다.
Frame / Game p95는 Mode가 올라갈수록 약간 낮아졌지만, 개선 폭이 작아 frame gain으로 단정하지 않는다.
이 결과는 assignment warmup으로 bootstrap 변수를 줄인 상태에서도 service work reduction이 유지된다는 근거로 본다.
```

## Conclusion

```text
BTUpdateIntervalMode 정책은 40 / 80 Enemy 조건 모두에서 정상 동작한다.
Mode가 올라갈수록 AIIntentState service work는 줄어든다.
AssignmentLease 적용 후 40 Enemy 조건에서 Mode 1 / 2도 Engage 2 / Alert 6 / Idle 계층을 안정적으로 유지했다.
AssignmentWarmup 1.2 적용 후 40 / 80 Enemy 조건에서도 동일한 호출 수 감소 경향이 유지됐다.
다만 Frame p95 개선은 뚜렷하지 않으므로, 이 결과는 직접적인 frame gain보다 Runtime LOD 정책 검증, assignment 안정화, service work reduction으로 해석한다.

다음 비교는 Alert assignment cap을 CVar로 분리한 뒤 AlertCap 6 / 40 조건에서 수행하거나, 현재 Warmup 1.2 기준을 baseline으로 Runtime LOD tier 설계를 진행한다.
```
## Follow-up - Assignment Bootstrap Warmup

초기 request 후보가 한 번에 모두 들어오지 않고 `6 -> 13 -> 32 -> 62 -> 80`처럼 단계적으로 채워지는 현상을 확인했다.
따라서 BT interval LOD 재측정 전에 CombatEngage 최초 assignment 확정 시점을 안정화한다.

상세 계획:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_CombatEngage_Assignment_Bootstrap_Warmup_Plan.md
```
