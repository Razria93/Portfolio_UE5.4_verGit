# AI Performance CSV Evidence Manifest

이 문서는 `P34` 이후 AI performance profiling 전용 asset에서 측정한 CSV 근거자료를 추적한다.

원본 raw CSV / log dump는 repo에 포함하지 않는다.
CSV dump는 크기가 크고 Git history를 빠르게 무겁게 만들기 때문에 local archive 기준으로 보관한다.
PR과 note에는 대표 수치, 측정 조건, 해석, 설계 분기 근거만 문서화한다.

Local archive 기준 경로:

```text
Csvprofile/
Docs/07_Profiling/AI_Performance/CSV/runtime_lod/
```

`Docs/07_Profiling/AI_Performance/CSV/runtime_lod/`는 `.gitignore` 대상이며, 필요할 때만 로컬에서 raw evidence를 복사해 확인한다.

## Folder Layout

```text
baseline/
-> AIPerf 전용 asset 기준 baseline 측정.

runtime_lod/p35_ai_context_interval_split/
-> P35 AIContext / AIIntentState interval split 대표 측정 CSV와 측정 로그. Repo에는 포함하지 않는 local archive 기준 경로.

runtime_lod/p35_design_pivot_evidence/
-> P35 진행 중 측정 방향과 설계 분기를 만든 근거 CSV와 측정 로그. Repo에는 포함하지 않는 local archive 기준 경로.
```

## Files

| Case | File | Source Condition |
| --- | --- | --- |
| 01 | `baseline/case_01_040_enemy_aiperf_engage.csv` | 40 Enemy / AIPerf Engage / F11 Fullscreen / -noailogging |
| P35-01 | `runtime_lod/p35_ai_context_interval_split/case_01_040_enemy_mode0_ai_context_interval_split.csv` | 40 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 0 / F11 Fullscreen / -noailogging |
| P35-02 | `runtime_lod/p35_ai_context_interval_split/case_02_040_enemy_mode1_ai_context_interval_split.csv` | 40 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 1 / F11 Fullscreen / -noailogging |
| P35-03 | `runtime_lod/p35_ai_context_interval_split/case_03_040_enemy_mode2_ai_context_interval_split.csv` | 40 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 2 / F11 Fullscreen / -noailogging |
| P35-04 | `runtime_lod/p35_ai_context_interval_split/case_04_080_enemy_mode0_ai_context_interval_split.csv` | 80 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 0 / F11 Fullscreen / -noailogging |
| P35-05 | `runtime_lod/p35_ai_context_interval_split/case_05_080_enemy_mode1_ai_context_interval_split.csv` | 80 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 1 / F11 Fullscreen / -noailogging |
| P35-06 | `runtime_lod/p35_ai_context_interval_split/case_06_080_enemy_mode2_ai_context_interval_split.csv` | 80 Enemy / AIContextIntervalSplit / BTUpdateIntervalMode 2 / F11 Fullscreen / -noailogging / 대표값 |
| P35-P01 | `runtime_lod/p35_design_pivot_evidence/pivot_01_080_enemy_perception_before_affiliation.csv` | Perception affiliation 보정 전 / 80 Enemy / Enemy 후보 누수와 FirstValidLatency 지연 근거 |
| P35-P02 | `runtime_lod/p35_design_pivot_evidence/pivot_02_080_enemy_perception_team_attitude.csv` | TeamAttitude 적용 후 / 80 Enemy / RawActors, InvalidProviders, FirstValidLatency 감소 근거 |
| P35-P03 | `runtime_lod/p35_design_pivot_evidence/pivot_03_080_enemy_movement_mode0_baseline.csv` | MovementMode 0 / 80 Enemy / movement baseline |
| P35-P04 | `runtime_lod/p35_design_pivot_evidence/pivot_04_080_enemy_movement_mode1_tick_disabled.csv` | MovementMode 1 / 80 Enemy / state refresh disabled가 representation을 깨뜨린 근거 |
| P35-P05 | `runtime_lod/p35_design_pivot_evidence/pivot_05_080_enemy_movement_mode2_intent_blocked.csv` | MovementMode 2 / 80 Enemy / movement intent block의 성능 효과와 gameplay 변화 근거 |
| P35-P06 | `runtime_lod/p35_design_pivot_evidence/pivot_06_080_enemy_assignment_warmup_1_0.csv` | AssignmentWarmup 1.0 / 80 Enemy / request snapshot 흔들림과 warmup 1.2 결정 근거 |

## P35 AIContext Interval Split Summary

아래 파일들은 repo-tracked 파일이 아니라 local archive 기준 파일명이다.
PR 대표 표와 Runtime LOD note의 수치가 최종 근거이며, raw CSV는 필요 시 로컬에서 재확인하는 용도로만 둔다.

측정 조건:

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
```

대표 CSV:

| Case | Enemy | Mode | CSV | Log |
| --- | ---: | ---: | --- | --- |
| P35-01 | 40 | 0 | `runtime_lod/p35_ai_context_interval_split/case_01_040_enemy_mode0_ai_context_interval_split.csv` | `runtime_lod/p35_ai_context_interval_split/case_01_040_enemy_mode0_ai_context_interval_split.log.txt` |
| P35-02 | 40 | 1 | `runtime_lod/p35_ai_context_interval_split/case_02_040_enemy_mode1_ai_context_interval_split.csv` | `runtime_lod/p35_ai_context_interval_split/case_02_040_enemy_mode1_ai_context_interval_split.log.txt` |
| P35-03 | 40 | 2 | `runtime_lod/p35_ai_context_interval_split/case_03_040_enemy_mode2_ai_context_interval_split.csv` | `runtime_lod/p35_ai_context_interval_split/case_03_040_enemy_mode2_ai_context_interval_split.log.txt` |
| P35-04 | 80 | 0 | `runtime_lod/p35_ai_context_interval_split/case_04_080_enemy_mode0_ai_context_interval_split.csv` | `runtime_lod/p35_ai_context_interval_split/case_04_080_enemy_mode0_ai_context_interval_split.log.txt` |
| P35-05 | 80 | 1 | `runtime_lod/p35_ai_context_interval_split/case_05_080_enemy_mode1_ai_context_interval_split.csv` | `runtime_lod/p35_ai_context_interval_split/case_05_080_enemy_mode1_ai_context_interval_split.log.txt` |
| P35-06 | 80 | 2 | `runtime_lod/p35_ai_context_interval_split/case_06_080_enemy_mode2_ai_context_interval_split.csv` | `runtime_lod/p35_ai_context_interval_split/case_06_080_enemy_mode2_ai_context_interval_split.log.txt` |

설계 분기 근거 CSV:

| Case | Pivot | CSV | Log |
| --- | --- | --- | --- |
| P35-P01 | Perception 후보 누수 확인 | `runtime_lod/p35_design_pivot_evidence/pivot_01_080_enemy_perception_before_affiliation.csv` | `runtime_lod/p35_design_pivot_evidence/pivot_01_080_enemy_perception_before_affiliation.log.txt` |
| P35-P02 | TeamAttitude / affiliation 보정 확인 | `runtime_lod/p35_design_pivot_evidence/pivot_02_080_enemy_perception_team_attitude.csv` | `runtime_lod/p35_design_pivot_evidence/pivot_02_080_enemy_perception_team_attitude.log.txt` |
| P35-P03 | Movement baseline | `runtime_lod/p35_design_pivot_evidence/pivot_03_080_enemy_movement_mode0_baseline.csv` | `runtime_lod/p35_design_pivot_evidence/pivot_03_080_enemy_movement_mode0_baseline.log.txt` |
| P35-P04 | MovementComponent tick disable 후보 제외 | `runtime_lod/p35_design_pivot_evidence/pivot_04_080_enemy_movement_mode1_tick_disabled.csv` | `runtime_lod/p35_design_pivot_evidence/pivot_04_080_enemy_movement_mode1_tick_disabled.log.txt` |
| P35-P05 | Movement intent block 효과와 gameplay 변화 확인 | `runtime_lod/p35_design_pivot_evidence/pivot_05_080_enemy_movement_mode2_intent_blocked.csv` | `runtime_lod/p35_design_pivot_evidence/pivot_05_080_enemy_movement_mode2_intent_blocked.log.txt` |
| P35-P06 | Assignment warmup 필요성 확인 | `runtime_lod/p35_design_pivot_evidence/pivot_06_080_enemy_assignment_warmup_1_0.csv` | `runtime_lod/p35_design_pivot_evidence/pivot_06_080_enemy_assignment_warmup_1_0.log.txt` |

Assignment warmup request snapshot 참고 로그:

```text
runtime_lod/p35_design_pivot_evidence/pivot_07_assignment_warmup_delay_samples.log.txt
runtime_lod/p35_design_pivot_evidence/pivot_08_assignment_rebuild_summary_samples.log.txt
```

최종 80 Enemy Mode 2 비교표에는 `case_06_080_enemy_mode2_ai_context_interval_split.csv`를 대표값으로 사용한다.

## Case 01 Summary

측정 조건:

```text
Map: MAP_AIPerf_40Enemy
Enemy: 40 placed AIPerf Enemy
State: Engage
Duration: 약 30초
Log State: -noailogging
PIE: F11 fullscreen
```

확인 사항:

```text
40 Enemy 정상 배치
AIPerf Enemy -> AIPerf AIController 사용
AIPerf AIController -> AIPerf Blackboard / BehaviorTree 사용
Blackboard TargetActor 정상 갱신
Patrol 랜덤 동작
Engage 진입 가능
Enemy끼리 피격 없음
```

Crowd / collision 조건:

```text
Enemy끼리 완전한 길막 제거 상태는 아니다.
P34 기준에서는 collision radius 축소와 MoveTo 허용반경 증가로 crowd 변수를 줄인다.
군집 해소 알고리즘은 P34 범위가 아니다.
```

CSV 주요 지표:

| Metric | Avg | p95 | Max |
| --- | ---: | ---: | ---: |
| FrameTime | 11.1087ms | 12.0703ms | 33.7046ms |
| GameThreadTime | 11.0671ms | 11.9513ms | 250.7286ms |
| GPUTime | 6.0309ms | 6.9694ms | 7.6379ms |
| RenderThreadTime | 0.0573ms | 0.0635ms | 0.6642ms |
| PortfolioAI_BT_UpdateAIContext | 0.1195ms | 0.1608ms | 0.3891ms |
| PortfolioAI_BT_UpdateAIIntentState | 0.0192ms | 0.0267ms | 0.1731ms |
| PortfolioAI_BT_UpdateEngageContext | 0.0018ms | 0.0024ms | 0.0196ms |
| PortfolioAI_CombatEngage_Tick | 0.0007ms | 0.0051ms | 0.1733ms |
| PortfolioAI_CombatEngage_RebuildAssignments | 0.0005ms | 0.0047ms | 0.0270ms |
| AIPerception | 0.0897ms | 0.1216ms | 0.2510ms |

메모:

```text
CSV ActorCount/CEnemy는 p95 80으로 기록된다.
PIE 환경에서 editor world / play world duplication이 포함될 수 있으므로,
P34의 Enemy 배치 기준은 editor map에서 확인한 40 Enemy를 기준으로 기록한다.
```
