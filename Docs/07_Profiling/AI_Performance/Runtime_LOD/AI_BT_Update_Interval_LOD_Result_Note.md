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

## 40 Enemy Result

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

## Conclusion

```text
BTUpdateIntervalMode 정책은 40 / 80 Enemy 조건 모두에서 정상 동작한다.
Mode가 올라갈수록 AIIntentState service work는 줄어든다.
다만 Frame p95 개선은 뚜렷하지 않으므로, 이 결과는 직접적인 frame gain보다 Runtime LOD 정책 검증과 service work reduction으로 해석한다.

다음 비교는 Alert assignment cap을 CVar로 분리한 뒤 AlertCap 6 / 40 조건에서 수행한다.
```

