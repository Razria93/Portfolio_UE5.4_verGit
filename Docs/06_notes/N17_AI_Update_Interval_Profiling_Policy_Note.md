# N17. AI Update Interval Profiling Policy Note

## 목적

이 문서는 `refactor/ai-update-interval-policy` 작업의 측정 / 분석 기준을 정리한다.

목표는 AI BehaviorTree 동작을 바로 바꾸는 것이 아니라, 현재 AI update 경로의 비용을 수치로 확인할 수 있게 만들고 이후 interval 조정, dirty flag, event-driven 전환 후보를 근거 있게 분류하는 것이다.

---

## 문제의식

현재 프로젝트는 일반적인 Actor Tick 사용은 많지 않지만, Enemy AI 쪽에서는 BehaviorTree Service, BehaviorTree Task polling, CombatEngage subsystem tick이 주기적으로 동작한다.

전투 규모가 작을 때는 문제가 체감되지 않을 수 있지만, Enemy 수가 늘어나면 다음 비용이 누적될 수 있다.

```text
BT Service interval polling
BT Task every-frame polling
CombatEngage assignment rebuild
Perception target data update
Blackboard read / write
Hot path debug log
```

dirty flag 방향도 이 문제와 연결된다. 현재 구조는 대부분 "주기적으로 다시 계산"하는 방식이고, dirty flag / event-driven 구조는 "변화가 발생한 경로만 다시 계산"하는 방식이다.

---

## 현재 Update 경로

### BT Service

```text
UCBTService_UpdateAIContext
-> Interval: 0.1s
-> 역할: perception top target, home metric, alert range, engage assignment, reaction/dead state 갱신
-> 리스크: 가장 넓은 context를 다루는 중심 service

UCBTService_UpdateEngageContext
-> Interval: 0.1s
-> 역할: engage range, combat cooldown, combat action 가능 여부 갱신
-> 리스크: 전투 중 빈번히 필요한 값이지만 Enemy 수만큼 누적됨

UCBTService_UpdateInvestigateContext
-> Interval: 0.1s
-> 역할: investigate timeout 확인
-> 리스크: 단순 timeout 확인이므로 낮은 빈도로도 충분할 가능성이 큼

UCBTService_UpdateAIIntentState
-> Interval: 0.2s
-> 역할: Blackboard context 기반 AI intent state 결정
-> 리스크: 반응성 요구와 비용 사이의 균형점
```

### BT Task Polling

```text
UCBTTask_WaitDeadState
-> TickTask every frame
-> 역할: target dead state 도달 대기
-> 후보: Health/DeadState event 기반 전환

UCBTTask_WaitEndCombatAction
-> TickTask every frame
-> 역할: bIsCombatAction false 대기
-> 후보: Action end event 또는 Blackboard observer 전환

UCBTTask_WaitEndReaction
-> TickTask every frame
-> 역할: bIsActiveReaction false 대기
-> 후보: Reaction end event 또는 Blackboard observer 전환

UCBTTask_SelectPatrolPoint
-> ExecuteTask
-> 역할: patrol index / patrol location / patrol reverse 갱신
-> 메모: 현재 patrol context 갱신은 Service polling이 아니라 Task 실행 시점에 처리된다.
```

### Subsystem

```text
UCWorldSubsystem_CombatEngage
-> RebuildInterval: 0.1s
-> 역할: target별 engage role assignment rebuild
-> 리스크: Enemy 수와 target 경쟁 상황에 따라 request bucket / sort 비용 증가
```

### AIController / Perception

```text
ACAIController::OnTargetPerceptionUpdated
-> Perception event callback
-> TargetDataMap 갱신

ACAIController::BuildPerceptionContext
-> BT Service에서 호출
-> TargetDataMap 정리 / top priority 선택

TargetMemoryTimeout
-> 3.0s
-> target memory 유지 기준
```

Perception callback 자체는 event-driven에 가깝지만, target selection / memory timeout / Blackboard 반영은 service polling에 의해 주기적으로 처리된다.

---

## 측정 목표

이번 작업의 1차 목표는 다음 질문에 답할 수 있게 만드는 것이다.

```text
Enemy 수가 늘어날 때 Game Thread 비용이 증가하는가?
증가한다면 BT Service, BT Task polling, CombatEngage 중 어디가 주요 후보인가?
0.1s interval service들이 같은 타이밍에 몰려 hitch를 만들 가능성이 있는가?
Debug log가 측정 결과를 왜곡하는가?
단순 interval 조정으로 충분한 경로와 dirty flag/event-driven 전환이 필요한 경로는 무엇인가?
```

---

## 측정 케이스

기본 측정 단위:

```text
Map: TestRoom
Mode: PIE
Viewport: F11 fullscreen
Duration: 30s per case
Stats: stat unit, stat game, stat ai, stat behavior
Capture: csvprofile start / csvprofile stop
```

`stat behavior`는 엔진 / 실행 상태에 따라 별도 overlay 변화가 명확하지 않을 수 있다. 기준 측정은 `stat unit`, `stat game`, `stat ai`, CSV `PortfolioAI` scope를 우선한다.

Boundary 측정은 viewport / editor layout 변수를 줄이기 위해 PIE 실행 후 F11 fullscreen 상태를 기준으로 진행한다.

Enemy 수:

```text
1
5
10
20
```

상태:

```text
Idle
Player Detected
Engage
```

최소 측정:

```text
1 Enemy / Idle
1 Enemy / Engage
10 Enemy / Engage
20 Enemy / Engage
```

로그 영향 분리:

```text
Debug log OFF 기준 측정
Debug log ON 또는 현재 상태 기준 측정
```

---

## 측정 절차

PIE 실행 전:

```text
1. TestRoom에서 측정할 Enemy 수를 고정한다.
2. 측정 상태를 Idle / Player Detected / Engage 중 하나로 맞춘다.
3. Editor viewport FPS overlay를 켜서 프레임 체감 변화를 같이 확인한다.
4. Output Log를 비워 측정 중 오류 / ensure / hot path log를 확인하기 쉽게 만든다.
```

PIE 실행 후:

```text
1. 콘솔에서 stat unit 실행
2. 콘솔에서 stat game 실행
3. 콘솔에서 stat ai 실행
4. 필요 시 콘솔에서 stat behavior 실행
5. 콘솔에서 csvprofile start 실행
6. 30초 동안 동일한 상태 유지
7. 콘솔에서 csvprofile stop 실행
8. 생성된 CSV 파일에서 PortfolioAI_ prefix column 확인
```

CSV 저장 위치:

```text
Saved/Profiling/CSV
```

우선 확인할 CSV scope:

```text
PortfolioAI_BT_UpdateAIContext
PortfolioAI_BT_UpdateAIIntentState
PortfolioAI_BT_UpdateEngageContext
PortfolioAI_BT_UpdateInvestigateContext
PortfolioAI_CombatEngage_Tick
PortfolioAI_CombatEngage_RebuildAssignments
```

주의:

```text
FPS overlay는 체감 프레임 확인용이다.
stat 명령은 에디터 안에서 대략적인 시스템 비용을 확인하기 위한 보조 지표다.
CSV는 계측 scope별 duration을 확인하기 위한 후속 분석 자료다.
CSV 결과가 없으면 interval 조정이나 dirty flag 도입을 바로 결정하지 않는다.
BehaviorTree 비용은 `stat behavior`보다 CSV의 `Exclusive/GameThread/BehaviorTreeTick`와 `PortfolioAI` scope를 우선 확인한다.
```

---

## 기록 양식

```text
Case:
Enemy Count:
State:
Duration:
Log State:
Viewport State:

Frame ms Avg:
Frame ms Max:
Game ms Avg:
Game ms Max:
AI ms Avg:
Behavior ms Avg:
Hitch Observed:

CSV Hot Path:
- UpdateAIContext:
- UpdateAIIntentState:
- UpdateEngageContext:
- UpdateInvestigateContext:
- CombatEngageTick:
- CombatEngageRebuild:

Notes:
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

---

## Case 08 Setup: 60 Enemy Distributed Patrol-Engage

목적:

```text
crowd blocking / stuck movement 변수를 AI polling 및 combat-heavy 부하와 분리한다.
이전 60 Enemy 측정은 밀도가 높아 대부분의 Enemy가 Engage / Assault 상태로 진입했고, 일부 Enemy가 서로 반복적으로 이동을 막았다.
```

Asset / map 변경:

```text
Patrol point spacing:
- 가장 가까운 patrol point 간 직선거리를 약 1500 unit 수준으로 확대

Patrol pattern:
- PatrolMode를 Random으로 변경

Enemy placement:
- Enemy 위치를 patrol area 중심 기준으로 재배치
- 기존 combat-heavy setup보다 밀도를 낮춤

Enemy collision:
- capsule radius를 40에서 10으로 축소

Patrol MoveTo:
- acceptable radius를 50에서 200으로 확대
```

예상 효과:

```text
CharacterMovement stuck / failed-to-move 상황을 줄인다.
밀도가 낮아지면서 perception / engage overlap이 줄어 일부 Enemy는 Patrol 상태로 남을 수 있다.
이 케이스는 Case 06 / 07과 직접 동등 비교하지 않고, controlled distributed-load 비교 케이스로 해석한다.
```

해석 기준:

```text
GameThread p95가 명확히 내려가고 BT / Perception 비용이 비슷하면 crowd blocking과 combat density가 주요 변수였다고 본다.
BT / Perception도 함께 내려가면 기존 dense setup이 AI state distribution 및 target perception 부하에도 영향을 줬다고 본다.
```

---

## Case 09 Setup: 60 Enemy Distributed Patrol-Engage / Friendly Hit Disabled

목적:

```text
Enemy끼리의 피격 / HitStop / CombatSignal 처리 변수를 제거하고,
60 Enemy distributed setup에서 AI polling 비용과 player-facing combat 비용을 다시 비교한다.
```

조건:

```text
Log State:
- Unreal Editor를 -noailogging 옵션으로 실행
- 프로젝트 combat 로그 출력은 비활성화 상태 유지

Combat condition:
- 플레이어 2명에게 Hit 발생
- 나머지 Enemy는 Engage / Alert 상태
- Enemy끼리는 피격이 발생하지 않음
- Enemy끼리 피격이 발생할 만한 거리에도 들어오지 않음

Capture timing:
- 플레이어가 2명에게 Engage되고 Combo cycle이 0일 때 측정 시작
```

해석:

```text
Friendly hit 차단 후에도 Frame / GameThread p95는 Case 08과 큰 차이가 없다.
따라서 Enemy끼리의 피격 자체는 distributed setup에서 주요 병목으로 보기 어렵다.

다만 Case 06 / 07 dense setup과 비교하면 GameThread p95는 여전히 낮으므로,
실제 병목은 friendly hit 하나가 아니라 crowd density, perception overlap,
combat interaction density가 함께 만든 부하로 보는 것이 타당하다.
```

---

## Profiling Environment Calibration Note

배경:

```text
초기 60 Enemy dense 측정은 BT Service tick 비용을 보기 위한 목적이었지만,
실제 측정 환경에는 아래 변수가 함께 섞여 있었다.
```

확인된 변수:

```text
Crowd density:
- Enemy가 좁은 영역에 밀집되어 서로 이동을 막는 상황이 발생했다.
- CharacterMovement / collision / path following 비용이 BT polling 비용과 섞였다.

Combat density:
- Enemy끼리 피격이 발생하면서 HitStop / CombatSignal / damage route가 추가로 실행됐다.
- 이 비용은 AI tick 최적화 대상이 아니라 combat interaction 비용이다.

Perception overlap:
- 밀집 배치에서는 대부분의 Enemy가 동시에 Engage / Assault 상태로 진입했다.
- 60 Enemy 전체가 동일한 high-cost 상태에 가까워져 일반적인 patrol-engage 상황과 거리가 생겼다.

Logging:
- combat 로그 출력이 일부 포함되어 있어 측정 변수로 남아 있었다.
```

보정 내용:

```text
1. Unreal Editor는 -noailogging 옵션으로 실행했다.
2. 프로젝트 combat 로그 출력을 비활성화했다.
3. Patrol point spacing을 넓히고 PatrolMode를 Random으로 변경했다.
4. Enemy 배치를 patrol area 중심 기준으로 넓게 분산했다.
5. Enemy capsule radius를 줄이고 MoveTo acceptable radius를 늘렸다.
6. Enemy끼리의 friendly hit를 차단한 비교 케이스를 추가했다.
```

해석:

```text
보정 전 dense 60 Enemy 결과는 "BT Service tick 단독 부하"가 아니라
crowd density / combat density / perception overlap이 함께 섞인 stress case로 해석한다.

보정 후 distributed / friendly-hit-disabled 결과는
AI polling 비용을 비교하기 위한 기준선으로 더 적합하다.
다만 이 역시 3D 액션 게임의 렌더링, 애니메이션, movement, collision 비용을 포함하므로
전체 Frame / GameThread 비용과 BT Service 비용을 분리해서 해석한다.
```

포트폴리오 관점:

```text
이 작업의 의미는 단순히 60 Enemy 측정값을 얻는 것이 아니라,
측정 환경에 섞인 변수를 관찰하고 분리한 뒤
최적화 전후 비교가 가능한 기준선을 만든 것이다.

따라서 이후 interval / dirty flag / event-driven 개선은
이 보정된 기준선에서 before / after를 비교한다.
```

---

## Scale Test Plan: Green / Yellow / Red Boundary

목적:

```text
현재 60 Enemy 보정 케이스는 BT Service tick p95가 약 0.5ms를 넘는 Yellow 초입이다.
이후 최적화 전후 비교를 위해 Green / Yellow / Red 구간을 명시적으로 확보한다.
```

공통 측정 조건:

```text
Mode:
- PIE

Viewport:
- F11 fullscreen
- 같은 camera position 유지
- 측정 중 editor panel / viewport layout 조작 금지

Runtime:
- -noailogging
- combat logs disabled
- distributed patrol-engage setup
- friendly hit disabled
```

판정 기준:

```text
Green:
- BehaviorTreeTick p95 < 0.5ms
- 현재 구조 유지 가능

Yellow:
- BehaviorTreeTick p95 0.5ms ~ 1.0ms
- scaling risk가 있으므로 interval / dirty flag / event-driven 최적화 후보

Red:
- BehaviorTreeTick p95 > 1.0ms
- polling 구조 개선 필요
```

측정 순서:

```text
40 Enemy:
- Green 상단 후보
- 0.5ms 아래인지 확인

60 Enemy:
- Yellow 초입 기준선
- 이미 보정 케이스 측정 완료

80 Enemy:
- Yellow 확장 확인
- 60 Enemy보다 BT polling 비용이 선형 또는 준선형으로 증가하는지 확인

120 Enemy:
- Red 진입 후보
- BehaviorTreeTick p95가 1.0ms를 넘는지 확인

160 Enemy:
- Red stress upper bound
- PIE / 액션 플레이 상태가 유지되는 범위에서만 측정

180 / 200 Enemy:
- Red 진입 여부를 확인하기 위한 추가 stress case
- 200 Enemy까지 측정하되, Red 진입 시 해당 지점을 상한으로 기록
```

중단 기준:

```text
1. BehaviorTreeTick p95 > 1.0ms를 확인한 경우
2. Frame p95 > 33ms 수준으로 올라가 30fps 아래 플레이 상태가 되는 경우
3. PIE가 안정적으로 측정되지 않는 경우
```

최적화 비교 기준:

```text
Yellow case:
- 60 Enemy 또는 80 Enemy
- 실제 플레이 가능 범위에서 interval / dirty flag 개선폭 확인

Red case:
- 120 Enemy 또는 160 Enemy
- stress condition에서 polling 구조 개선 효과 확인
```

해석 원칙:

```text
3D 액션 게임에서 렌더링, 애니메이션, movement, collision 비용은 기본적으로 발생한다.
이 측정의 목적은 전체 Frame 비용의 모든 원인을 AI tick으로 돌리는 것이 아니다.

목적은 BT Service / AI polling 비용이 허용 가능한 tick budget을 넘는 조건을 찾고,
그 조건에서 interval / dirty flag / event-driven 개선이 어느 정도 효과를 내는지 확인하는 것이다.
```

---

## CSV Profiling Scope 후보

1차 계측 후보:

```text
UCBTService_UpdateAIContext::TickNode
UCBTService_UpdateAIIntentState::TickNode
UCBTService_UpdateEngageContext::TickNode
UCBTService_UpdateInvestigateContext::TickNode
UCWorldSubsystem_CombatEngage::Tick
UCWorldSubsystem_CombatEngage::RebuildAssignments
```

2차 계측 후보:

```text
UCBTTask_WaitDeadState::TickTask
UCBTTask_WaitEndCombatAction::TickTask
UCBTTask_WaitEndReaction::TickTask
UCBTTask_SelectPatrolPoint::ExecuteTask
ACAIController::BuildPerceptionContext
ACAIController::UpdateTargetDataMap
ACAIController::SelectTopPriority
```

원칙:

```text
계측은 성능 측정 목적이므로 기능 동작을 바꾸지 않는다.
계측 추가와 interval 값 변경은 커밋을 분리한다.
CSV scope는 hot path를 식별할 수 있는 최소 단위로 둔다.
너무 세분화해서 코드 가독성을 해치지 않는다.
```

1차 적용 scope:

```text
CSV Global Stat Prefix: PortfolioAI_

PortfolioAI_BT_UpdateAIContext
PortfolioAI_BT_UpdateAIIntentState
PortfolioAI_BT_UpdateEngageContext
PortfolioAI_BT_UpdateInvestigateContext
PortfolioAI_CombatEngage_Tick
PortfolioAI_CombatEngage_RebuildAssignments
```

---

## 개선안 분류

### 1. Interval 조정

```text
0.1s -> 0.2s / 0.3s / 0.5s
RandomDeviation 적용
상태별 interval 분리
```

적용 후보:

```text
UpdateInvestigateContext
UpdateAIIntentState
CombatEngage rebuild
```

장점:

```text
구현 위험이 낮고 빠르게 적용 가능
```

한계:

```text
polling 구조 자체는 유지됨
반응성이 늦어질 수 있음
```

### 2. Dirty Flag

변화가 발생했을 때만 특정 context를 다시 계산하도록 표시하는 방식이다.

이번 브랜치에서 바로 도입한 범위는 full dirty flag가 아니라 Blackboard dirty write guard다.

```text
Service polling은 유지한다.
계산 결과가 기존 Blackboard 값과 같으면 SetValue를 생략한다.
Task의 명령성 Blackboard write는 유지한다.
context dirty ownership / 소비 순서는 아직 도입하지 않는다.
```

예시:

```text
Perception changed -> TargetContextDirty
Target changed -> EngageAssignmentDirty
Combat action started/ended -> CombatStateDirty
Reaction started/ended -> ReactionStateDirty
Movement threshold exceeded -> DistanceContextDirty
```

적용 후보:

```text
Target selection
Engage assignment
AIIntentState
Reaction / CombatAction state
```

주의점:

```text
dirty를 누가 세우고 누가 소비하는지 ownership이 필요하다.
dirty 누락은 상태 갱신 누락 버그로 이어질 수 있다.
fallback interval 또는 validation path가 필요할 수 있다.
```

### 3. Event-driven 전환

상태 변화 시점에 Blackboard 또는 AI context를 갱신하는 방식이다.

적용 후보:

```text
DeadState
Reaction active state
Combat action active state
Perception target gained/lost
```

장점:

```text
불필요한 polling을 줄이고 상태 변화 시점과 갱신 시점을 맞출 수 있다.
```

주의점:

```text
delegate bind/unbind lifecycle 관리가 필요하다.
BT layer와 gameplay component 사이의 책임 경계를 다시 정해야 한다.
```

### 4. Hybrid

현재 프로젝트에 가장 현실적인 후보는 hybrid 방식이다.

```text
Dead / Reaction / CombatAction
-> event-driven

Distance / range / timeout
-> interval polling

CombatEngage assignment
-> dirty rebuild + fallback interval
```

---

## 이번 브랜치 범위

포함:

```text
AI update 경로 전수 조사
측정 케이스 정의
CSV profiling scope 설계
필요한 최소 계측 추가
측정 결과 기록
개선 후보 분류
```

제외:

```text
BehaviorTree asset 재설계
AI 행동 로직 변경
context dirty ownership을 포함한 dirty flag 실제 구조 도입
event-driven Blackboard update 전환
대규모 AI LOD / batch manager 구현
Enhanced Input migration
```

---

## 추천 커밋 단위

```text
docs(ai): plan update interval profiling policy
refactor(ai): add csv profiling scopes for update paths
docs(ai): record update interval profiling results
refactor(ai): guard repeated blackboard writes in services
refactor(ai): define ai update interval defaults
docs(ai): classify dirty flag and event-driven followups
```

interval 값을 실제로 바꾸는 커밋은 profiling 결과를 기록한 뒤에만 진행한다.

---

## 완료 조건

```text
현재 AI update 경로가 service / task / subsystem / perception 기준으로 목록화되어 있다.
CSV profiling 대상과 측정 케이스가 문서화되어 있다.
계측 추가 시 기능 동작이 바뀌지 않는다.
반복 Blackboard write guard가 service polling 경로에 적용되어 있다.
측정 결과를 바탕으로 interval 조정, dirty flag, event-driven 후보가 분리되어 있다.
git diff --check가 통과한다.
PortfolioEditor Win64 Development 빌드가 통과한다.
PIE AI smoke test가 통과한다.
```
