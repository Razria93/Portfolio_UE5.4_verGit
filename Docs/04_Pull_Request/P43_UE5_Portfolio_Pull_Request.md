# UE5 Portfolio Pull Request

## 제목

**P43: CVar Ownership Policy**

## 날짜

**2026.07.22**

## 상태

- [x] Debug / Diagnostic CVar ownership 재점검
- [x] Profiling audit / CSV counter CVar ownership 분리
- [x] RuntimeLOD policy / tuning CVar 유지 기준 정리
- [x] `CSV_CUSTOM_STAT_GLOBAL` 직접 호출을 `Core/Profiling` helper로 이동
- [x] `CSV_SCOPED_TIMING_STAT_GLOBAL`은 측정 scope 본문 유지
- [x] RuntimeLOD policy의 profiling audit gate 노출 제거
- [x] CVar ownership 최종 기준 문서화
- [x] `PortfolioEditor Win64 Development` build 통과

## 브랜치

- `refactor/cvar-ownership-policy`

## 요약

이번 PR은 P42 `Debug Log Policy v1` 이후 남아 있던 CVar ownership 경계를 정리한다.

P42에서 debug log / diagnostic helper 분리는 완료했지만, 일부 profiling gate, CSV counter, RuntimeLOD policy CVar의 책임 경계가 아직 섞여 있었다. 이번 PR에서는 CVar를 다음 기준으로 재분류했다.

```text
Core/Debug
-> 사람이 읽는 Output Log / diagnostic text / debug dump CVar 소유

Core/Profiling
-> profiling audit gate / CSV counter / profiling behavior gate 소유

AI/RuntimeLOD 또는 owner system
-> 실제 gameplay policy / tuning CVar 소유
```

이 기준에 따라 debug 출력 CVar, profiling audit CVar, RuntimeLOD policy/tuning CVar를 분리하고, CSV counter 직접 호출도 profiling helper 내부로 모았다.

## 주요 변경

```text
1. AI audit / debug CVar gate 정리
   - AI perception audit summary 출력 helper 분리
   - AI combat BT / CanMove decorator audit gate 정리
   - CombatEngage assignment audit output helper 분리

2. Profiling behavior gate CVar 분리
   - DisableEnemyPerception
   - DisableEnemyHitProcessing
   - DisableEnemyWeaponActor
   - DisableEnemyCombatFeedback
   - StatePolicyAudit
   - AnimationRefreshAudit

3. CSV counter ownership 정리
   - animation refresh counter를 CAIAnimationProfiling으로 이동
   - 현재 profiling plan의 animation refresh counter CVar를 AnimationRefreshAudit으로 통일
   - BT service / interval preset counter를 CAIBehaviorTreeProfiling으로 이동
   - Core/Profiling 밖 CSV_CUSTOM_STAT_GLOBAL 직접 호출 제거

4. RuntimeLOD policy 경계 정리
   - StatePolicyMode는 policy source 선택만 담당
   - StatePolicyAudit은 state tier CSV counter만 담당
   - FAIAnimationRuntimeLODPolicy에서 profiling audit gate 노출 제거

5. 문서화
   - N26 ownership inventory 정정
   - N27 CVar ownership final rule 추가
```

## 최종 CVar ownership 기준

```text
Debug / Diagnostic output CVar
-> Core/Debug helper

Debug Dump CVar
-> Core/Debug helper

Profiling audit / CSV counter CVar
-> Core/Profiling helper

Runtime policy / tuning CVar
-> owning policy/system cpp
```

혼합 CVar는 금지한다.

```text
StatePolicyMode
-> RuntimeLOD policy source만 선택

StatePolicyAudit
-> state tier CSV profiling counter만 제어
```

## CSV macro 기준

```text
CSV_CUSTOM_STAT_GLOBAL
-> event / counter 기록
-> 직접 호출은 Core/Profiling helper 구현부에만 둔다.

CSV_SCOPED_TIMING_STAT_GLOBAL
-> RAII scope timing 계측
-> 측정 범위가 중요하므로 측정 대상 scope 본문에 둔다.
```

## 주요 CVar

Core/Debug:

```text
Portfolio.Debug.*
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit
```

Core/Profiling:

```text
Portfolio.AI.RuntimeLOD.AnimationRefreshAudit
Portfolio.AI.RuntimeLOD.StatePolicyAudit
Portfolio.AI.RuntimeLOD.DisableEnemyPerception
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing
Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback
```

Runtime policy / tuning owner 유지:

```text
Portfolio.AI.RuntimeLOD.StatePolicyMode
Portfolio.AI.RuntimeLOD.EnemyMovementMode
Portfolio.AI.RuntimeLOD.EnemyAnimationMode
Portfolio.AI.RuntimeLOD.EnemyAnimationReducedRefreshInterval
Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap
```

## 변경 파일 범위

```text
Docs/04_Pull_Request/00_Pull_Request_Index.md
Docs/04_Pull_Request/P43_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N26_Diagnostic_Log_Full_Audit_Inventory_Note.md
Docs/06_notes/N27_Debug_Profiling_CVar_Ownership_Final_Note.md

Source/Portfolio/Core/Debug/*
Source/Portfolio/Core/Profiling/*
Source/Portfolio/AI/RuntimeLOD/*
Source/Portfolio/AI/BehaviorTree/*
Source/Portfolio/Character/*
Source/Portfolio/System/Combat/*
```

## 검증

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Static check

```text
Core/Debug 밖 직접 출력 호출
Result: 0

Core/Profiling 밖 CSV_CUSTOM_STAT_GLOBAL 직접 호출
Result: 0

Core/Profiling 밖 남은 CSV_ 호출
Result: CSV_SCOPED_TIMING_STAT_GLOBAL only

Core/Debug / Core/Profiling 밖 TAutoConsoleVariable
Result: RuntimeLOD policy / tuning CVar만 유지
```

### Remaining Direct Scoped Timing

```text
CWorldSubsystem_CombatEngage
-> PortfolioAI_CombatEngage_Tick
-> PortfolioAI_CombatEngage_RebuildAssignments

BT services
-> PortfolioAI_BT_UpdateAIContext
-> PortfolioAI_BT_UpdateAIIntentState
-> PortfolioAI_BT_UpdateEngageContext
-> PortfolioAI_BT_UpdateInvestigateContext
```

위 항목은 `CSV_SCOPED_TIMING_STAT_GLOBAL`이라 helper로 이동하지 않고 측정 scope 본문에 유지한다.

## 후속 작업

이번 PR에서 처리하지 않는 항목:

```text
EBTServiceIntervalPreset 위치
-> 구조체 나누기 / 헤더 배치 규칙에서 처리

combat profiling API suffix 통일
-> 네이밍 / 매개변수 작명 규칙에서 처리

Debug helper 섹션명 전체 통일
-> TODO / 주석 및 섹션 정리에서 처리

CEnemy RuntimeLOD CVar 이동
-> 데이터 에셋 분리 또는 RuntimeLOD config 구조 정리에서 처리
```
