# UE5 Portfolio Pull Request

## 제목

**P33: AI Update Interval Profiling 정책 정리**

## 날짜

**2026.07.01**

## 상태

- [x] 작업 방향 수립
- [x] 코드 / 문서 반영
- [ ] 검증 완료

---

## 브랜치

- `refactor/ai-update-interval-policy`

---

## 커밋

```text
docs(ai): plan update interval profiling policy
refactor(ai): add profiling scopes for update intervals
```

---

## 요약

이번 PR은 Enemy AI의 BehaviorTree Service / Task polling / CombatEngage subsystem update 경로를 전수 조사하고, AI 수 증가 시 비용을 측정할 수 있는 profiling 기준을 정리한다.

목표는 interval 값을 감으로 조정하는 것이 아니라, 현재 구조에서 어떤 경로가 비용을 만들 수 있는지 수치로 확인하고, 단순 interval 조정 / dirty flag / event-driven 전환 후보를 분리하는 것이다.

---

## 작업 배경

현재 프로젝트는 일반 Actor Tick을 넓게 사용하지 않지만, Enemy AI 쪽에는 짧은 주기의 BehaviorTree Service와 polling Task가 존재한다.

```text
BT Service interval polling
BT Task every-frame polling
CombatEngage assignment rebuild
Perception target data update
Blackboard read / write
Hot path debug log
```

전투 시연 규모에서는 문제가 체감되지 않을 수 있지만, Enemy 수가 늘어나면 service tick과 subsystem rebuild 비용이 누적될 수 있다.

dirty flag 방향은 이 문제와 직접 연결된다. 현재 구조가 주기적으로 다시 계산하는 방식이라면, dirty flag / event-driven 구조는 변경이 발생한 경로만 다시 계산하는 방식이다.

---

## 사전 조사 결과

### BT Service

```text
UCBTService_UpdateAIContext
-> Interval: 0.1s
-> 역할: perception top target, home metric, alert range, engage assignment, reaction/dead state 갱신

UCBTService_UpdateEngageContext
-> Interval: 0.1s
-> 역할: engage range, combat cooldown, combat action 가능 여부 갱신

UCBTService_UpdateInvestigateContext
-> Interval: 0.1s
-> 역할: investigate timeout 확인

UCBTService_UpdateAIIntentState
-> Interval: 0.2s
-> 역할: Blackboard 기반 AI intent state 결정
```

### BT Task Polling

```text
UCBTTask_WaitDeadState
-> TickTask every frame

UCBTTask_WaitEndCombatAction
-> TickTask every frame

UCBTTask_WaitEndReaction
-> TickTask every frame

UCBTTask_SelectPatrolPoint
-> ExecuteTask
-> patrol index / patrol location / patrol reverse 갱신
```

### Subsystem

```text
UCWorldSubsystem_CombatEngage
-> RebuildInterval: 0.1s
-> 역할: target별 engage role assignment rebuild
```

### AIController / Perception

```text
ACAIController::OnTargetPerceptionUpdated
-> perception event callback

ACAIController::BuildPerceptionContext
-> BT Service에서 호출
-> TargetDataMap 정리 / top priority 선택

TargetMemoryTimeout
-> 3.0s
```

---

## 작업 범위

### 1. 측정 정책 정리

```text
Enemy 수별 측정 케이스 정의
Idle / Player Detected / Engage 상태별 측정 기준 정의
stat unit / stat game / stat ai / stat behavior / csvprofile 사용 기준 정리
CSV 결과 기록 양식 정리
```

`stat behavior`는 보조 지표로 둔다. 기준 측정은 `stat unit`, `stat game`, `stat ai`, CSV `PortfolioAI` scope를 우선한다.

### 2. CSV profiling 계측 후보 정리

1차 후보:

```text
UCBTService_UpdateAIContext::TickNode
UCBTService_UpdateAIIntentState::TickNode
UCBTService_UpdateEngageContext::TickNode
UCBTService_UpdateInvestigateContext::TickNode
UCWorldSubsystem_CombatEngage::Tick
UCWorldSubsystem_CombatEngage::RebuildAssignments
```

2차 후보:

```text
UCBTTask_WaitDeadState::TickTask
UCBTTask_WaitEndCombatAction::TickTask
UCBTTask_WaitEndReaction::TickTask
UCBTTask_SelectPatrolPoint::ExecuteTask
ACAIController::BuildPerceptionContext
ACAIController::UpdateTargetDataMap
ACAIController::SelectTopPriority
```

1차 적용:

```text
CSV Global Stat Prefix: PortfolioAI_

PortfolioAI_BT_UpdateAIContext
PortfolioAI_BT_UpdateAIIntentState
PortfolioAI_BT_UpdateEngageContext
PortfolioAI_BT_UpdateInvestigateContext
PortfolioAI_CombatEngage_Tick
PortfolioAI_CombatEngage_RebuildAssignments
```

### 3. 개선 후보 분류

```text
Interval 조정
RandomDeviation 적용
Dirty flag
Event-driven Blackboard update
Hybrid update model
AI LOD / batch update 후속 후보
```

---

## 제외 범위

```text
BehaviorTree asset 재설계
AI 행동 로직 변경
dirty flag 실제 구조 도입
event-driven Blackboard update 전환
대규모 AI LOD / batch manager 구현
Enhanced Input migration
```

---

## 검증 계획

```text
rg 기반 AI update / tick / interval 사용처 전수 확인
CSV profiling scope compile 확인
git diff --check
PortfolioEditor Win64 Development 빌드
PIE AI smoke test
```

측정 케이스:

```text
Enemy Count: 1 / 5 / 10 / 20
State: Idle / Player Detected / Engage
Duration: 30s per case
Stats: stat unit, stat game, stat ai, stat behavior(optional)
Capture: csvprofile start / csvprofile stop
```

측정 절차:

```text
1. PIE 실행
2. stat unit / stat game / stat ai 활성화
3. csvprofile start
4. 30초 동안 동일 상태 유지
5. csvprofile stop
6. Saved/Profiling/CSV 결과 파일 확인
7. PortfolioAI_ prefix의 BT Service / CombatEngage scope 확인
```

기록 항목:

```text
Frame ms Avg / Max
Game ms Avg / Max
AI ms Avg
Behavior ms Avg
Hitch observed
CSV hot path summary
```

측정 결과:

| Case | Enemy Count | State | Duration | FPS / Frame | Game ms | AI ms | Behavior ms | PortfolioAI Hot Path | Notes |
| --- | ---: | --- | ---: | --- | --- | --- | --- | --- | --- |
| 01 | 1 | Idle / Patrol | 29.27s | avg 11.20ms / p95 9.68ms / p99 24.88ms | avg 10.02ms / p95 9.50ms / p99 10.63ms | BT Tick avg 0.0174ms / p95 0.0259ms | - | BT_UpdateAIContext p95 0.0123ms, BT_UpdateAIIntentState p95 0.0037ms, CombatEngage_Rebuild p95 0.0005ms | Patrol context is updated by `UCBTTask_SelectPatrolPoint`; unused patrol service removed. |
| 02 | 1 | Engage | 32.82s | avg 10.29ms / p95 10.91ms / p99 11.85ms | avg 10.41ms / p95 10.91ms / p99 11.64ms | BT Tick avg 0.0267ms / p95 0.0351ms | - | BT_UpdateAIContext p95 0.0230ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0023ms | Engage branch scope recorded normally; Investigate branch not entered. |
| 03 | 10 | Engage | 31.72s | avg 11.53ms / p95 12.35ms / p99 13.07ms | avg 11.97ms / p95 12.35ms / p99 13.01ms | BT Tick avg 0.0719ms / p95 0.0913ms, AIPerception p95 0.0556ms | - | BT_UpdateAIContext p95 0.0534ms, BT_UpdateEngageContext p95 0.0022ms, CombatEngage_Rebuild p95 0.0034ms | 10 AI engage load recorded; active tick counts show 10 AI controllers/enemies. `ActorCount/CEnemy` can include editor/PIE world duplication in PIE CSV. |
| 04 | 20 | Engage | 29.88s | avg 12.64ms / p95 13.54ms / p99 14.00ms | avg 13.38ms / p95 13.51ms / p99 13.93ms | BT Tick avg 0.1127ms / p95 0.1540ms, AIPerception p95 0.0727ms | - | BT_UpdateAIContext p95 0.0831ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0038ms | 20 AI engage load recorded; active tick counts show 20 AI controllers/enemies. GameThread max contains a capture/PIE outlier, so p95/p99 are used for judgment. |
| 05 | 40 | Engage | 30.12s | avg 16.09ms / p95 18.00ms / p99 18.66ms | avg 17.46ms / p95 17.98ms / p99 18.59ms | BT Tick avg 0.2453ms / p95 0.3545ms, AIPerception p95 0.2168ms | - | BT_UpdateAIContext p95 0.1732ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0056ms | 40 AI engage load reaches the 60fps boundary; AI service cost increases but remains below 0.5ms p95. GameThread/combat load is the stronger bottleneck candidate. |
| 06 | 60 | Engage | 30.48s | avg 19.54ms / p95 21.89ms / p99 22.73ms | avg 21.17ms / p95 21.84ms / p99 22.62ms | BT Tick avg 0.3834ms / p95 0.5331ms, AIPerception p95 0.4236ms | - | BT_UpdateAIContext p95 0.2799ms, BT_UpdateEngageContext p95 0.0014ms, CombatEngage_Rebuild p95 0.0072ms | 60 AI engage load is below 60fps; BT service cost crosses 0.5ms p95 but remains below 1ms. GameThread/combat interaction remains the primary optimization candidate. |
| 07 | 60 | Engage / Logs Disabled | 30.45s | avg 19.28ms / p95 21.31ms / p99 22.02ms | avg 20.66ms / p95 21.27ms / p99 21.90ms | BT Tick avg 0.3843ms / p95 0.5156ms, AIPerception p95 0.4453ms | - | BT_UpdateAIContext p95 0.2699ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0071ms | Project combat logs disabled. Frame/GameThread improve slightly, but the result is close to Case 06; logging is not the main bottleneck. |

현재 확인:

```text
git diff --check 통과
PortfolioEditor Win64 Development 빌드 통과
PIE AI smoke test 진행 중
1 Enemy / Idle-Patrol profiling 기록 완료
1 Enemy / Engage profiling 기록 완료
10 Enemy / Engage profiling 기록 완료
20 Enemy / Engage profiling 기록 완료
40 Enemy / Engage profiling 기록 완료
60 Enemy / Engage profiling 기록 완료
60 Enemy / Engage logs-disabled profiling 기록 완료
```

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N17_AI_Update_Interval_Profiling_Policy_Note.md
Docs/04_Pull_Request/P33_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N15_AI_Blackboard_Key_Registry_Policy_Note.md
```

---
