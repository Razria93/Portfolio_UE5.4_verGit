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
refactor(ai): guard repeated blackboard writes in services
```

---

## 요약

이번 PR은 Enemy AI의 BehaviorTree Service / Task polling / CombatEngage subsystem update 경로를 전수 조사하고, AI 수 증가 시 비용을 측정할 수 있는 profiling 기준을 정리한다.

목표는 interval 값을 감으로 조정하는 것이 아니라, 현재 구조에서 어떤 경로가 비용을 만들 수 있는지 수치로 확인하고, 단순 interval 조정 / dirty flag / event-driven 전환 후보를 분리하는 것이다.

추가로 full dirty flag 구조를 바로 도입하지 않고, polling service가 같은 Blackboard 값을 반복 기록하지 않도록 `Set...IfChanged` 기반 dirty write guard를 먼저 적용한다.

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
Blackboard dirty write guard
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
context dirty ownership을 포함한 dirty flag 실제 구조 도입
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
| 08 | 60 | Distributed Patrol-Engage | 34.74s | avg 16.11ms / p95 19.04ms / p99 19.85ms | avg 18.96ms / p95 18.98ms / p99 19.60ms | BT Tick avg 0.3208ms / p95 0.4993ms, AIPerception p95 0.2134ms | - | BT_UpdateAIContext p95 0.2583ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0069ms | Distributed setup reduces hit/stuck density; about three enemies receive hits and many enemies remain in Alert/Patrol range. Frame/GameThread improve compared with Case 06/07, but the case is not equivalent to the dense combat-heavy setup. |
| 09 | 60 | Distributed Patrol-Engage / Friendly Hit Disabled | 34.30s | avg 16.64ms / p95 19.51ms / p99 20.17ms | avg 19.34ms / p95 19.58ms / p99 20.13ms | BT Tick avg 0.3702ms / p95 0.5767ms, AIPerception p95 0.2642ms | - | BT_UpdateAIContext p95 0.2884ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0081ms | Enemy끼리 피격이 발생하지 않도록 friendly hit를 차단한 비교 케이스다. Frame/GameThread는 Case 08과 비슷하고, AIPerception은 Case 06/07보다 낮다. BT Tick p95는 약간 증가했으므로 friendly hit만이 전체 병목이라고 보기는 어렵다. |
| 10 | 40 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 31.92s | avg 12.72ms / p95 14.11ms / p99 14.55ms | avg 12.98ms / p95 14.11ms / p99 14.57ms | BT Tick avg 0.2764ms / p95 0.4090ms, AIPerception p95 0.1435ms | Green | BT_UpdateAIContext p95 0.1909ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0064ms | Boundary 측정 기준을 PIE F11 fullscreen으로 고정한 뒤 다시 측정한 결과다. Enemy끼리 피격 / 길막이 발생하지 않는 조건에서 BT Tick p95가 0.5ms 아래로 유지되어 Green 구간으로 판정한다. |
| 11 | 60 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 31.77s | avg 15.49ms / p95 16.90ms / p99 17.65ms | avg 16.03ms / p95 16.92ms / p99 17.62ms | BT Tick p95 0.5424ms, AIPerception p95 0.2162ms | Yellow | BT_UpdateAIContext p95 0.2664ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0072ms | 60 Enemy boundary 측정 결과다. Enemy끼리 피격 / 길막이 발생하지 않는 조건에서도 BT Tick p95가 0.5ms를 넘어 Yellow 초입으로 진입했다. CSV 일부 평균값에는 비정상 max outlier가 있어 p95/p99 중심으로 해석한다. |
| 12 | 80 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 31.97s | avg 18.96ms / p95 21.19ms / p99 22.21ms | avg 19.74ms / p95 21.18ms / p99 21.84ms | BT Tick avg 0.5252ms / p95 0.7012ms, AIPerception p95 0.3476ms | Yellow | BT_UpdateAIContext p95 0.3499ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0086ms | 80 Enemy boundary 측정 결과다. BT Tick p95가 0.7ms 수준까지 증가해 Yellow 구간이 명확해졌으며, Frame/GameThread p95도 20ms를 넘어 60fps 기준을 벗어난다. |
| 13 | 100 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 31.88s | avg 20.89ms / p95 23.21ms / p99 24.50ms | avg 21.89ms / p95 23.37ms / p99 24.27ms | BT Tick avg 0.5351ms / p95 0.6601ms, AIPerception p95 0.3297ms | Yellow | BT_UpdateAIContext p95 0.4100ms, BT_UpdateEngageContext p95 0.0022ms, CombatEngage_Rebuild p95 0.0062ms | 100 Enemy boundary 측정 결과다. -noailogging 조건에서 다시 측정했으며, BT Tick p95는 Yellow 구간에 머물러 Red 기준인 1.0ms에는 도달하지 않았다. Frame/GameThread p95는 23ms 수준으로 증가해 전체 플레이 부하는 더 무거워졌다. |
| 14 | 120 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 32.13s | avg 23.69ms / p95 26.72ms / p99 27.53ms | avg 25.60ms / p95 26.77ms / p99 27.43ms | BT Tick avg 0.5917ms / p95 0.7676ms, AIPerception p95 0.3915ms | Yellow | BT_UpdateAIContext p95 0.4738ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0060ms | 120 Enemy boundary 측정 결과다. 초반 2~3초 정도 Engage 공백이 있었을 수 있으나 p95 기준 BT Tick은 여전히 Yellow 구간이며 Red 기준인 1.0ms에는 도달하지 않았다. Frame/GameThread p95는 26ms대로 증가해 전체 플레이 부하가 먼저 한계에 가까워진다. |
| 15 | 140 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 32.57s | avg 27.03ms / p95 31.67ms / p99 32.68ms | avg 30.50ms / p95 31.66ms / p99 32.67ms | BT Tick avg 0.6449ms / p95 0.8595ms, AIPerception p95 0.4621ms | Yellow | BT_UpdateAIContext p95 0.5244ms, BT_UpdateEngageContext p95 0.0020ms, CombatEngage_Rebuild p95 0.0068ms | 140 Enemy boundary 측정 결과다. BT Tick p95는 아직 Red 기준인 1.0ms 아래지만 p99가 0.98ms 수준까지 올라 Red 경계에 근접했다. Frame/GameThread p95는 31ms대로 증가해 30fps 경계에 가까워졌으며, perception 인지 지연이 약 10초 관찰됐다. |
| 16 | 160 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 28.74s | avg 27.45ms / p95 34.39ms / p99 54.22ms | avg 32.45ms / p95 35.62ms / p99 35.62ms | BT Tick avg 0.6359ms / p95 0.9204ms, AIPerception p95 0.5346ms | Yellow Upper | BT_UpdateAIContext p95 0.5630ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0074ms | 160 Enemy boundary 측정 결과다. 측정 시작 직전 crash를 피하기 위해 csvprofile을 미리 켠 뒤 대기했으므로 앞 10초를 제외한 값을 공식값으로 사용한다. BT Tick p95는 Red 기준인 1.0ms 아래지만 Yellow 상단이며, Frame p95는 33ms를 넘어 30fps 아래 플레이 상태 중단 기준에 도달했다. 이 시점부터 OUT OF MEMORY 경고와 약 15초의 perception 인지 지연이 관찰됐다. |
| 17 | 180 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 32.10s | avg 32.03ms / p95 38.68ms / p99 56.06ms | avg 36.62ms / p95 38.51ms / p99 41.47ms | BT Tick avg 0.7517ms / p95 1.0169ms, AIPerception p95 0.6158ms | Red | BT_UpdateAIContext p95 0.6118ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0066ms | 180 Enemy boundary 측정 결과다. 앞 5초 대기 구간을 제외한 값을 공식값으로 사용한다. BT Tick p95가 1.0ms를 넘어 Red에 진입했으며, Frame/GameThread p95도 30fps 아래 플레이 상태 기준을 크게 넘었다. 160 Enemy 이후의 OUT OF MEMORY 경고가 이어진 상태에서 약 20초의 perception 인지 지연이 관찰됐다. |
| 18 | 200 | Boundary / Friendly Hit Disabled / F11 Fullscreen | 32.23s | avg 33.98ms / p95 42.27ms / p99 43.34ms | avg 40.42ms / p95 42.27ms / p99 43.72ms | BT Tick avg 0.8021ms / p95 1.1133ms, AIPerception p95 1.1034ms | Red / Stress Limit | BT_UpdateAIContext p95 0.6694ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0065ms | 200 Enemy boundary 측정 결과다. 앞 15초 대기 구간을 제외한 값을 공식값으로 사용한다. 160 Enemy 이후 이어진 OUT OF MEMORY 경고가 200 Enemy에서도 관찰됐고, 약 25초의 perception 인지 지연도 확인됐다. 이 케이스는 일반 최적화 기준선이 아니라 PIE CSV / runtime stress limit 확인용으로 기록한다. |
| 19 | 120 | Boundary / Dirty Write Guard / F11 Fullscreen | 31.92s | avg 22.84ms / p95 26.02ms / p99 33.31ms | avg 24.23ms / p95 25.81ms / p99 28.31ms | BT Tick avg 0.5808ms / p95 0.7464ms, AIPerception p95 0.3726ms | Yellow | BT_UpdateAIContext p95 0.4585ms, BT_UpdateEngageContext p95 0.0021ms, CombatEngage_Rebuild p95 0.0062ms | Blackboard dirty write guard 적용 이후 120 Enemy 재측정 결과다. Case 14 대비 BT_UpdateAIContext p95와 BT Tick p95가 소폭 감소했지만, 계산 / perception / render 부하는 그대로이므로 체감 개선 축이 아니라 micro optimization 결과로 분류한다. |

Raw CSV:

```text
Docs/07_Profiling/AI_Update_Interval/CSV/MANIFEST.md
```

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
60 Enemy / Distributed Patrol-Engage profiling 기록 완료
60 Enemy / Distributed Patrol-Engage friendly-hit-disabled profiling 기록 완료
Boundary profiling viewport 기준을 PIE F11 fullscreen으로 고정
40 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
60 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
80 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
100 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
120 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
140 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
160 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
Boundary stress 측정 범위를 200 Enemy 또는 Red 진입까지 확장
180 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
180 Enemy에서 BT Tick Red 진입 확인
200 Enemy / Boundary friendly-hit-disabled fullscreen profiling 기록 완료
200 Enemy를 PIE CSV / runtime stress limit 케이스로 기록
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
