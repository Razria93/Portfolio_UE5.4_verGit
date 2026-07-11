# UE5 Portfolio Pull Request

## 제목

**P37: AI Observe Intent 및 Investigate Lifecycle 정리**

## 날짜

**2026.07.11**

## 상태

- [x] Observe intent 정책 정리
- [x] Investigate lifecycle flag 분리
- [x] Blackboard / BehaviorTree asset 반영
- [x] 40 / 80 Enemy smoke 측정 완료

## 브랜치

- `feature/ai-observe-intent-state`

## 요약

P36에서 `Engage / Alert / Idle` 계층화와 AlertCap 비교를 통해 Alert 후보 수가 movement cost에 영향을 준다는 점을 확인했다.

이번 PR은 그 후속으로, target은 인식했지만 `CombatEngage` assignment 권한이 없는 Enemy가 Chase / Alert / Investigate로 번지지 않도록 `Observe` intent를 명확히 분리한다.

또한 기존 `bCanInvestigate` / `bIsInvestigating` 흐름에서 Investigate 진입 조건과 실행 상태가 겹치던 부분을 정리하고, Investigate 시작 / 진행 / 종료 요청을 별도 Blackboard flag로 분리한다.

## 주요 변경

```text
1. Observe intent 정책 정리
   - TargetActor / bHasLOS를 awareness 신호로 사용
   - CombatRole이 없는 인지 대상은 Observe로 대기
   - CombatRole이 있는 대상만 Chase / Alert / Engage 허용

2. Investigate lifecycle flag 분리
   - bCanInvestigate 제거
   - bShouldInvestigate 추가
   - bShouldEndInvestigate 추가
   - bIsInvestigating은 실제 Investigate route 실행 상태로 유지

3. Investigate task / service 책임 정리
   - StartInvestigate는 진입 요청을 소비하고 실행 상태를 켬
   - AdvanceInvestigateIndex는 max index 도달 시 종료 요청을 올림
   - UpdateInvestigateContext는 timeout 시 종료 요청을 올림
   - EndInvestigate는 flag와 위치 / index / last seen 정보를 정리

4. Blackboard / BT asset 반영
   - BB_Default / BB_AIPerf_Default key 정리
   - BT_Investigate / BT_AIPerf_Investigate guard 조건 정리
   - Observe intent smoke 검증용 map 갱신
```

## AI Intent 정책

이번 PR의 핵심 판단은 다음과 같다.

```text
TargetActor || bHasLOS == awareness
```

`TargetActor`는 LOS 흔들림을 완충하는 target memory이고, `bHasLOS`는 현재 시야에 대한 즉시 반응 신호다.

상태 판단은 다음 순서로 정리했다.

```text
1. awareness 없음
   - bShouldInvestigate 또는 bIsInvestigating이 true이면 Investigate
   - 아니면 Idle

2. awareness 있음 + CombatRole 없음
   - Observe

3. awareness 있음 + CombatRole 있음
   - alert range 밖이면 Chase
   - CombatRole Engage이면 Engage
   - CombatRole Alert이면 Alert
   - fallback은 Observe
```

따라서 AlertCap 밖의 Enemy는 target을 인식해도 Chase / Alert Spread / Investigate로 대량 진입하지 않는다.

## Investigate Lifecycle

Investigate 관련 Blackboard flag는 다음 의미로 분리했다.

```text
bShouldInvestigate
-> Investigate 진입 예약.
-> Engage였던 Enemy가 target awareness를 완전히 잃었을 때만 사용한다.

bIsInvestigating
-> 실제 Investigate route 실행 중.

bShouldEndInvestigate
-> Investigate route 종료 요청.
-> max index 도달 또는 timeout에서 true가 된다.
```

BT guard는 다음 형태로 정리했다.

```text
Start Investigate
-> bShouldInvestigate == true
-> bIsInvestigating == false

Do Investigate
-> bIsInvestigating == true
-> bShouldEndInvestigate == false

End Investigate
-> bIsInvestigating == true
-> bShouldEndInvestigate == true
```

`EndInvestigate`는 `bShouldInvestigate`, `bIsInvestigating`, `bShouldEndInvestigate`, `InvestigateLocation`, `InvestigateIndex`, `LastSeenTime`, `LastKnownLocation`을 정리한다.

## 변경 파일

```text
Source/Portfolio/AI/Blackboard/CAIKey.h
Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.cpp
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateInvestigateContext.cpp
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_AdvanceInvestigateIndex.cpp
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_EndInvestigate.cpp
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartInvestigate.cpp
Source/Portfolio/Type/CStateStructure.h

Content/02_Controller/02_Enemy/AI/Blackboard/BB_Default.uasset
Content/02_Controller/02_Enemy/AI/BehaviorTree/BT_Default.uasset
Content/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_Idle.uasset
Content/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_Investigate.uasset
Content/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_Observe.uasset

Content/00_Profiling/00_AI_Performance/00_Map/08_ObserveIntent/GM_AIPerf_ObserveIntent.uasset
Content/00_Profiling/00_AI_Performance/00_Map/08_ObserveIntent/MAP_AIPerf_ObserveIntent_40Enemy.umap
Content/00_Profiling/00_AI_Performance/00_Map/08_ObserveIntent/MAP_AIPerf_ObserveIntent_80Enemy.umap
Content/00_Profiling/00_AI_Performance/01_Character/01_Player/BP_AIPerf_CPlayer_Edit.uasset
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/AI/Blackboard/BB_AIPerf_Default.uasset
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/AI/BehaviorTree/BT_AIPerf_Default.uasset
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_AIPerf_Idle.uasset
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_AIPerf_Investigate.uasset
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_AIPerf_Observe.uasset

Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P37_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N19_Code_Quality_PR_Status_Summary_Note.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
```

## 측정 조건

```text
Capture Duration: 약 37~38초
Analysis Window: first 3s / last 3s trimmed
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
```

공통 조건:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
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
```

## Smoke 결과

| Case | CSV | Frame p95 | Game p95 | CharacterMovement p95 | BT Tick p95 |
| --- | --- | ---: | ---: | ---: | ---: |
| 40 Enemy | `Profile(20260710_235840).csv` | 12.0728ms | 11.6555ms | 0.5393ms | 0.2253ms |
| 80 Enemy | `Profile(20260711_000137).csv` | 17.6108ms | 17.6139ms | 0.9388ms | 0.4207ms |

## 호출 수

| Case | AIContext Count | AIIntent Count | EngageContext Count |
| --- | ---: | ---: | ---: |
| 40 Enemy | 11760 | 6000 | 586 |
| 80 Enemy | 23360 | 12160 | 580 |

## 해석

두 측정 모두 로그상 GC 이벤트가 없었다.

CSV의 `Ticks/CEnemy` p95는 각각 40 / 80이므로 실제 play tick 기준 40 / 80 Enemy 측정으로 분류한다.

40 Enemy는 P36 AlertCap 6 대표 측정과 거의 같은 수준으로 유지됐다.

80 Enemy는 P36 AlertCap 6 대표 측정보다 `CharacterMovement p95`가 소폭 높지만, `BT Tick`, `AIContext`, `AIIntentState` 계열은 큰 회귀 없이 유지됐다.

따라서 이번 변경은 다음 기준으로 해석한다.

```text
Observe / Investigate lifecycle 분리는
AlertCap / BT service interval 정책을 깨지 않고,
assignment 권한이 없는 Enemy의 행동 확산을 막기 위한 상태 정책 정리로 유효하다.
```

## 검증

```text
1. 40 Enemy smoke 측정
2. 80 Enemy smoke 측정
3. GC 이벤트 없음 확인
4. Ticks/CEnemy 기준 실제 Enemy 수 확인
5. Observe 상태에서 비참여 Enemy가 Chase / Alert Spread로 번지지 않는지 확인
6. Investigate Start / Do / End guard가 정상 동작하는지 확인
```

## 제외 범위

```text
1. 최종 Runtime LOD component 구현
   - 이번 PR은 상태 정책과 Investigate lifecycle 정리까지 다룬다.

2. Collision / Hit Window 비용 측정
   - 별도 측정 축으로 분리한다.

3. Feedback Presentation 비용 측정
   - Niagara / trail / sound / camera shake 비용은 별도 축으로 분리한다.

4. Component Tick Audit
   - 별도 최적화 작업으로 분리한다.

5. Perception Active Budget / Cap
   - perception 활성 대상 수 제한은 별도 정책으로 검토한다.
```

## 후속 작업

```text
1. Collision / Hit Window 측정
2. Feedback Presentation 측정
3. Component Tick Audit
4. Perception Active Budget / Cap 검토
5. Runtime LOD Implementation v1
```

## 관련 문서

```text
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_AlertCap_Comparison_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_BT_Update_Interval_LOD_Result_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Debugging_Obstacle_Note.md
Docs/07_Profiling/CSV_Analysis_Guide.md
```
