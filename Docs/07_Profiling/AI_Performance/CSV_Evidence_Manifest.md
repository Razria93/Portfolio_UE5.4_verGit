# AI Performance CSV Evidence Manifest

raw CSV / log 캡처는 `Docs/07_Profiling` 아래에 저장하지 않는다.
이 manifest는 AI performance 프로파일링 노트에서 사용한 대표 source ID와 측정 의도를 기록한다.

해석된 수치의 기준은 각 Runtime LOD 문서다. 이 manifest는 특정 측정 또는 설계 분기의 근거가 된 로컬 캡처를 식별할 때만 사용한다.

## 로컬 raw 캡처 위치

```text
Csvprofile/
Saved/Profiling/CSV/
```

## Baseline Evidence

| Case | Source Capture | 측정 조건 |
| --- | --- | --- |
| AP-01 | `case_01_040_enemy_aiperf_engage.csv` | 40 Enemy / AIPerf Engage / F11 Fullscreen / -noailogging |

## Runtime LOD Evidence

| Case | Source Capture | 측정 조건 |
| --- | --- | --- |
| P35-01 | `Profile(20260709_191603).csv` | 40 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 0 |
| P35-02 | `Profile(20260709_191821).csv` | 40 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 1 |
| P35-03 | `Profile(20260709_192202).csv` | 40 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 2 |
| P35-04 | `Profile(20260709_202805).csv` | 80 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 0 |
| P35-05 | `Profile(20260709_202920).csv` | 80 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 1 |
| P35-06 | `Profile(20260709_203937).csv` | 80 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 2 / 대표값 |
| P36-01 | `Profile(20260710_100333).csv` | 40 Enemy / AlertCap 6 |
| P36-02 | `Profile(20260710_100515).csv` | 40 Enemy / AlertCap 40 |
| P36-03 | `Profile(20260710_100737).csv` | 80 Enemy / AlertCap 6 |
| P36-04 | `Profile(20260710_100921).csv` | 80 Enemy / AlertCap 40 |
| P37-01 | `Profile(20260710_235840).csv` | 40 Enemy / Observe-Investigate lifecycle smoke |
| P37-02 | `Profile(20260711_000137).csv` | 80 Enemy / Observe-Investigate lifecycle smoke |

## Design Pivot Evidence

| Case | Source Capture | 설계 분기 |
| --- | --- | --- |
| P35-P01 | `Profile(20260705_204255).csv` / `Profile(20260705_204744).csv` | 최종 audit pair 확정 전 Perception disable 비교 |
| P35-P02 | `Profile(20260706_021904).csv` / `Profile(20260706_022337).csv` | TeamAttitude / affiliation 보정 확인 |
| P35-P03 | `Profile(20260707_141109).csv` | MovementMode 0 baseline |
| P35-P04 | `Profile(20260707_141359).csv` | MovementMode 1 state refresh disabled 후보 제외 근거 |
| P35-P05 | `Profile(20260707_141911).csv` | MovementMode 2 movement intent blocked 후보 |
| P35-P06 | `Profile(20260709_165800).csv` | Assignment warmup 1.0 request snapshot 불안정성 |

## Combat Collision / Hit Window Evidence

Combat collision / hit window 브랜치는 raw 캡처를 로컬 자료로만 사용했다. 해석 결론은 현재 hit processing toggle이 측정된 Engage 조건에서 Runtime LOD 정책으로 분리할 만큼 크고 안정적인 이득을 보여주지 않았다는 것이다.

| Case | Source Capture | 측정 조건 |
| --- | --- | --- |
| CC-01 | `Profile(20260711_111308).csv` | 40 Enemy / Full combat reference |
| CC-02 | `Profile(20260711_120705).csv` | 40 Enemy / DisableEnemyHitProcessing 1 |
| CC-03 | `Profile(20260711_110514).csv` | 80 Enemy / Full combat reference |
| CC-04 | `Profile(20260711_121022).csv` | 80 Enemy / DisableEnemyHitProcessing 1 |
| CC-05 | `Profile(20260711_123416).csv` | 80 Enemy / Engage stress / hit processing on |
| CC-06 | `Profile(20260711_123952).csv` | 80 Enemy / Engage stress / hit processing off |
