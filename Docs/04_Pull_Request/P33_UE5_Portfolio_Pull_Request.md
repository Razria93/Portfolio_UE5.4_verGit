# UE5 Portfolio Pull Request

## 제목

**P33: AI Update Interval Profiling 정책 정리**

## 날짜

**2026.07.01**

## 상태

- [x] 작업 방향 수립
- [x] 코드 / 문서 반영
- [x] 검증 완료

---

## 브랜치

- `refactor/ai-update-interval-policy`

---

## 주요 커밋 흐름

```text
docs(ai): plan update interval profiling policy
refactor(ai): add profiling scopes for update intervals
refactor(ai): guard repeated blackboard writes in services
chore(ai): prepare 120-enemy dirty-write profiling setup
docs(ai): add raw csv profiling records
```

---

## 요약

이번 PR은 Enemy AI의 BehaviorTree Service / Task polling / CombatEngage subsystem update 경로를 전수 조사하고, AI 수 증가 시 비용을 측정할 수 있는 프로파일링 기준을 정리한다.

목표는 interval 값을 감으로 조정하는 것이 아니라, 현재 구조에서 어떤 경로가 비용을 만들 수 있는지 수치로 확인하고, 단순 interval 조정, dirty flag, event-driven 전환 후보를 분리하는 것이다.

추가로 full dirty flag 구조를 바로 도입하지 않고, polling service가 같은 Blackboard 값을 반복 기록하지 않도록 `Set...IfChanged` 기반 dirty write guard를 먼저 적용한다.

---

## 작업 배경

현재 프로젝트는 일반 Actor Tick을 넓게 사용하지 않지만, Enemy AI 쪽에는 짧은 주기의 BehaviorTree Service와 polling Task가 존재한다.

```text
BT Service interval polling
BT Task every-frame polling
CombatEngage assignment rebuild
Perception target data update
Blackboard read, write
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
-> 역할: perception top target, home metric, alert range, engage assignment, reaction state, dead state 갱신

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
-> patrol index, patrol location, patrol reverse 갱신
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
Idle, Patrol, Engage, Boundary 상태별 측정 기준 정의
stat unit, stat game, stat ai, stat behavior, csvprofile 사용 기준 정리
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
AI LOD, batch update 후속 후보
```

---

## 제외 범위

```text
BehaviorTree asset 재설계
AI 행동 로직 변경
context dirty ownership을 포함한 dirty flag 실제 구조 도입
event-driven Blackboard update 전환
대규모 AI LOD, batch manager 구현
Enhanced Input migration
profiling 전용 Enemy / BT / Map asset 구성
공유 gameplay asset의 profiling용 설정 유지
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
Enemy Count: 1, 10, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200
State: Idle, Patrol, Engage, Boundary, Dirty Write Guard
Duration: 30s per case
Stats: stat unit, stat game, stat ai, stat behavior(optional)
Capture: csvprofile start / csvprofile stop
```

측정 절차:

```text
1. Unreal Editor를 -noailogging 옵션으로 실행
2. PIE 실행 후 F11 fullscreen 전환
3. stat unit / stat game / stat ai 활성화
4. 필요 시 stat behavior 활성화
5. csvprofile start
6. 30초 동안 동일 상태 유지
7. csvprofile stop
8. <PROJECT_ROOT>/Saved/Profiling/CSV 결과 파일 확인
9. PortfolioAI_ prefix의 BT Service / CombatEngage scope 확인
```

상세 실행 명령과 boundary 기록 양식은 `N17_AI_Update_Interval_Profiling_Policy_Note.md`의 측정 절차를 따른다.

기록 항목:

```text
Frame ms Avg, p95, p99
Game ms Avg, p95, p99
AI ms Avg
Behavior ms Avg, p95
Hitch observed
CSV hot path summary
```

측정 결과:

### 기준 측정

| Case | Enemy | 상태           |     시간 | Frame p95 | Game p95 | BT Tick p95 | AIPerception p95 | 주요 경로 p95                                                          | 메모                                                                                         |
| ---- | ----: | ------------ | -----: | --------: | -------: | ----------: | ---------------: | ------------------------------------------------------------------ | ------------------------------------------------------------------------------------------ |
| 01   |     1 | Idle, Patrol | 29.27s |    9.68ms |   9.50ms |    0.0259ms |                - | AIContext 0.0123ms, AIIntent 0.0037ms, EngageRebuild 0.0005ms      | Patrol context는 `UCBTTask_SelectPatrolPoint`에서 갱신된다. 사용하지 않는 patrol service는 제거했다.         |
| 02   |     1 | Engage       | 32.82s |   10.91ms |  10.91ms |    0.0351ms |                - | AIContext 0.0230ms, EngageContext 0.0020ms, EngageRebuild 0.0023ms | Engage branch 계측 scope가 정상 기록됐다. Investigate branch는 진입하지 않았다.                             |
| 03   |    10 | Engage       | 31.72s |   12.35ms |  12.35ms |    0.0913ms |         0.0556ms | AIContext 0.0534ms, EngageContext 0.0022ms, EngageRebuild 0.0034ms | 10 AI engage 부하 기준값이다. PIE CSV의 `ActorCount/CEnemy`는 editor world와 PIE world 중복을 포함할 수 있다. |
| 04   |    20 | Engage       | 29.88s |   13.54ms |  13.51ms |    0.1540ms |         0.0727ms | AIContext 0.0831ms, EngageContext 0.0020ms, EngageRebuild 0.0038ms | 20 AI engage 부하 기준값이다. GameThread max에는 capture/PIE outlier가 있어 p95/p99 중심으로 판단한다.         |
| 05   |    40 | Engage       | 30.12s |   18.00ms |  17.98ms |    0.3545ms |         0.2168ms | AIContext 0.1732ms, EngageContext 0.0020ms, EngageRebuild 0.0056ms | 60fps 경계에 접근한다. AI service 비용은 증가하지만 p95 기준 0.5ms 아래다.                                     |
| 06   |    60 | Engage       | 30.48s |   21.89ms |  21.84ms |    0.5331ms |         0.4236ms | AIContext 0.2799ms, EngageContext 0.0014ms, EngageRebuild 0.0072ms | 60fps 아래로 내려간다. BT service 비용은 0.5ms를 넘지만 1.0ms에는 도달하지 않는다.                                |

### 환경 변수 분리 측정

| Case | Enemy | 상태 | 시간 | Frame p95 | Game p95 | BT Tick p95 | AIPerception p95 | 주요 경로 p95 | 메모 |
| ---- | ----: | ---- | ---: | --------: | -------: | -----------: | ---------------: | ------------ | ---- |
| 07 | 60 | Engage, 로그 제거 | 30.45s | 21.31ms | 21.27ms | 0.5156ms | 0.4453ms | AIContext 0.2699ms, EngageContext 0.0020ms, EngageRebuild 0.0071ms | 전투 로그를 제거한 비교 케이스다. Frame/GameThread는 소폭 개선됐지만 Case 06과 큰 차이는 없어 로그가 주 병목은 아니다. |
| 08 | 60 | 분산 배치, Patrol-Engage | 34.74s | 19.04ms | 18.98ms | 0.4993ms | 0.2134ms | AIContext 0.2583ms, EngageContext 0.0020ms, EngageRebuild 0.0069ms | 배치 밀도, 길막, 피격 밀도를 줄인 케이스다. Frame/GameThread는 개선됐지만 기존 고밀도 전투 조건과 동일 조건은 아니다. |
| 09 | 60 | 분산 배치, 아군 피격 차단 | 34.30s | 19.51ms | 19.58ms | 0.5767ms | 0.2642ms | AIContext 0.2884ms, EngageContext 0.0020ms, EngageRebuild 0.0081ms | Enemy끼리 피격이 발생하지 않도록 차단했다. AIPerception은 Case 06, 07보다 낮지만 BT Tick p95는 약간 증가했다. |

### 경계 측정

공통 조건:

```text
상태: Engage boundary
환경: 아군 피격 차단, Enemy끼리 길막 없음, PIE F11 전체화면
로그: -noailogging
```

| Case | Enemy | 시간 | Frame p95 | Game p95 | BT Tick p95 | AIPerception p95 | 판정 | 메모 |
| ---- | ----: | ---: | --------: | -------: | ----------: | ---------------: | ---- | ---- |
| 10 | 40 | 31.92s | 14.11ms | 14.11ms | 0.4090ms | 0.1435ms | 정상 | BT Tick p95가 0.5ms 아래로 유지된다. 60fps 기준에서도 여유가 있다. |
| 11 | 60 | 31.77s | 16.90ms | 16.92ms | 0.5424ms | 0.2162ms | 주의 | BT Tick p95가 0.5ms를 넘어 주의 구간 초입에 진입한다. CSV 일부 평균값은 비정상 최댓값이 있어 p95/p99 중심으로 해석한다. |
| 12 | 80 | 31.97s | 21.19ms | 21.18ms | 0.7012ms | 0.3476ms | 주의 | BT Tick p95가 0.7ms 수준까지 증가한다. Frame/GameThread p95도 20ms를 넘어 60fps 기준을 벗어난다. |
| 13 | 100 | 31.88s | 23.21ms | 23.37ms | 0.6601ms | 0.3297ms | 주의 | `-noailogging` 조건에서 재측정했다. BT Tick p95는 주의 구간에 머물고, 전체 플레이 부하는 더 무거워진다. |
| 14 | 120 | 32.13s | 26.72ms | 26.77ms | 0.7676ms | 0.3915ms | 주의 | 초반 2~3초 Engage 공백 가능성이 있다. BT Tick은 아직 위험 기준 1.0ms 아래지만 Frame/GameThread가 먼저 한계에 가까워진다. |
| 15 | 140 | 32.57s | 31.67ms | 31.66ms | 0.8595ms | 0.4621ms | 주의 | BT Tick p99가 0.98ms 수준까지 올라 위험 경계에 근접했다. 약 10초의 perception 인지 지연이 관찰됐다. |
| 16 | 160 | 28.74s | 34.39ms | 35.62ms | 0.9204ms | 0.5346ms | 주의 상단 | Crash를 피하기 위해 csvprofile을 미리 켠 뒤 대기했고, 앞 10초를 제외한 값을 공식값으로 사용한다. OUT OF MEMORY 경고와 약 15초의 perception 인지 지연이 관찰됐다. |
| 17 | 180 | 32.10s | 38.68ms | 38.51ms | 1.0169ms | 0.6158ms | 위험 | 앞 5초 대기 구간을 제외했다. BT Tick p95가 1.0ms를 넘어 위험 구간에 진입했고, 약 20초의 perception 인지 지연이 관찰됐다. |
| 18 | 200 | 32.23s | 42.27ms | 42.27ms | 1.1133ms | 1.1034ms | 위험, 스트레스 한계 | 앞 15초 대기 구간을 제외했다. 약 25초의 perception 인지 지연이 관찰됐으며, 일반 최적화 기준선보다 PIE runtime 스트레스 한계 확인용 기록에 가깝다. |

### Dirty Write Guard 비교

공통 조건:

```text
비교 기준: Case 14
상태: Engage boundary
환경: 아군 피격 차단, Enemy끼리 길막 없음, PIE F11 전체화면
변경: Blackboard dirty write guard 적용
```

| Case | Enemy | 시간 | Frame p95 | Game p95 | BT Tick p95 | AIPerception p95 | 주요 경로 p95 | 메모 |
| ---- | ----: | ---: | --------: | -------: | ----------: | ---------------: | ------------ | ---- |
| 19 | 120 | 31.92s | 26.02ms | 25.81ms | 0.7464ms | 0.3726ms | AIContext 0.4585ms, EngageContext 0.0021ms, EngageRebuild 0.0062ms | Case 14 대비 `BT_UpdateAIContext` p95와 BT Tick p95가 소폭 감소했다. 계산, perception, 렌더링 부하는 그대로이므로 체감 최적화가 아니라 미세 최적화로 분류한다. |

측정 해석:

```text
1. 40 Enemy는 정상 구간, 60~160 Enemy는 주의 구간, 180 Enemy부터 BT Tick p95 기준 위험 구간에 진입한다.
2. BT service 비용은 Enemy 수에 따라 증가하지만, 120 Enemy까지는 전체 Frame/GameThread 부하가 먼저 한계에 가까워진다.
3. 160 Enemy 이후에는 OUT OF MEMORY 경고와 perception 인지 지연이 함께 관찰되어 일반 플레이 기준선보다 스트레스 한계 측정 성격이 강하다.
4. dirty write guard는 반복 Blackboard write를 줄이는 저위험 개선이다. 다만 계산 자체를 줄이지 않으므로 큰 성능 개선 축은 아니다.
5. 후속 최적화는 runtime LOD, perception LOD, update interval LOD로 분리하는 편이 적절하다.
```

원본 CSV:

```text
Docs/07_Profiling/AI_Update_Interval/CSV_Evidence_Manifest.md
```

현재 확인:

```text
git diff --check 통과
PortfolioEditor Win64 Development 빌드 통과
PIE AI smoke test 완료
1 Enemy Idle-Patrol 측정 기록 완료
1 Enemy Engage 측정 기록 완료
10 Enemy Engage 측정 기록 완료
20 Enemy Engage 측정 기록 완료
40 Enemy Engage 측정 기록 완료
60 Enemy Engage 측정 기록 완료
60 Enemy 로그 제거 비교 측정 기록 완료
60 Enemy 분산 Patrol-Engage 측정 기록 완료
60 Enemy 분산 Patrol-Engage, 아군 피격 차단 측정 기록 완료
경계 측정 viewport 기준을 PIE F11 전체화면으로 고정
40 Enemy 경계 측정 기록 완료
60 Enemy 경계 측정 기록 완료
80 Enemy 경계 측정 기록 완료
100 Enemy 경계 측정 기록 완료
120 Enemy 경계 측정 기록 완료
140 Enemy 경계 측정 기록 완료
160 Enemy 경계 측정 기록 완료
경계 스트레스 측정 범위를 200 Enemy 또는 위험 구간 진입까지 확장
180 Enemy 경계 측정 기록 완료
180 Enemy에서 BT Tick 위험 구간 진입 확인
200 Enemy 경계 측정 기록 완료
200 Enemy를 PIE CSV, runtime 스트레스 한계 케이스로 기록
120 Enemy dirty write guard 비교 측정 기록 완료
원본 CSV profiling records archive 추가
공유 gameplay asset에 들어간 profiling용 설정 변경 제외
```

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N17_AI_Update_Interval_Profiling_Policy_Note.md
Docs/06_notes/N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md
Docs/04_Pull_Request/P33_UE5_Portfolio_Pull_Request.md
Docs/06_notes/N15_AI_Blackboard_Key_Registry_Policy_Note.md
```

---
