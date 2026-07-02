# AI Performance Raw CSV Manifest

이 폴더는 `P34` 이후 AI performance profiling 전용 asset에서 측정한 raw CSV 파일을 보관한다.

## Folder Layout

```text
baseline/
-> AIPerf 전용 asset 기준 baseline 측정.
```

## Files

| Case | File | Source Condition |
| --- | --- | --- |
| 01 | `baseline/case_01_040_enemy_aiperf_engage.csv` | 40 Enemy / AIPerf Engage / F11 Fullscreen / -noailogging |

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

