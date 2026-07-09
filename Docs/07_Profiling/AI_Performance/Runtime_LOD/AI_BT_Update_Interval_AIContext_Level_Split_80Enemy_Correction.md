# AIContext Interval Split 80 Enemy Correction

## Purpose

`80 Enemy / BTUpdateIntervalMode 2` 대표 측정값을 재측정본 기준으로 정정한다.

기존 `Profile(20260709_203058).csv`는 호출수는 유사했지만 Frame / Game / CharacterMovement p95가 튄 보조 측정으로 기록한다.
최종 비교표에는 `Profile(20260709_203937).csv`를 대표값으로 사용한다.

## Final 80 Enemy Result

측정 파일:

```text
Mode 0: Profile(20260709_202805).csv
Mode 1: Profile(20260709_202920).csv
Mode 2: Profile(20260709_203937).csv
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

## Interpretation

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

## Policy Note

```text
Mode 1은 80 Enemy 조건에서 보수적인 AIContext / AIIntentState 호출수 감소 후보로 볼 수 있다.
Mode 2는 호출수와 BT Tick p95 감소 효과가 가장 크지만, combat-capable gameplay stress 조건에서는 반복 안정성 확인이 필요하다.
Mode 2는 더 먼 거리, 화면 밖, NonCombat / Dormant에 가까운 tier 후보로 우선 분류한다.
```
