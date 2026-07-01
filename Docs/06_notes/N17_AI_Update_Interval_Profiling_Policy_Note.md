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
Duration: 30s per case
Stats: stat unit, stat game, stat ai, stat behavior
Capture: csvprofile start / csvprofile stop
```

`stat behavior`는 엔진 / 실행 상태에 따라 별도 overlay 변화가 명확하지 않을 수 있다. 기준 측정은 `stat unit`, `stat game`, `stat ai`, CSV `PortfolioAI` scope를 우선한다.

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

---

## Case 08 Setup: 60 Enemy Distributed Patrol-Engage

Purpose:

```text
Separate crowd blocking / stuck movement from AI polling and combat-heavy load.
The previous 60 Enemy cases were dense enough that most enemies entered Engage/Assault and several enemies repeatedly blocked each other.
```

Asset / map changes:

```text
Patrol point spacing:
- nearest patrol point distance expanded to about 1500 units

Patrol pattern:
- PatrolMode changed to Random

Enemy placement:
- enemies moved around the patrol-area center
- density reduced compared to the previous combat-heavy setup

Enemy collision:
- capsule radius changed from 40 to 10

Patrol MoveTo:
- acceptable radius changed from 50 to 200
```

Expected effect:

```text
CharacterMovement stuck / failed-to-move cases should be reduced.
Some enemies may remain in Patrol because lower density reduces perception/engage overlap.
This case is not directly equivalent to Case 06/07; it is a controlled distributed-load comparison.
```

Interpretation rule:

```text
If GameThread p95 drops clearly while BT / Perception costs remain similar, crowd blocking and combat density were the dominant factors.
If BT / Perception also drops clearly, the previous dense setup also affected AI state distribution and target perception load.
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
dirty flag 실제 구조 도입
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
측정 결과를 바탕으로 interval 조정, dirty flag, event-driven 후보가 분리되어 있다.
git diff --check가 통과한다.
PortfolioEditor Win64 Development 빌드가 통과한다.
PIE AI smoke test가 통과한다.
```
