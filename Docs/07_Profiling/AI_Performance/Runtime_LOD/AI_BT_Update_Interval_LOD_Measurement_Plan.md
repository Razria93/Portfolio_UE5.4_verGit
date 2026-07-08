# AI BT Update Interval LOD Measurement Plan

## 목적

`BT Update Interval` 축이 40 / 80 Enemy 조건에서 frame budget과 movement / perception downstream 비용에 어떤 영향을 주는지 분리한다.

이 측정은 Behavior Tree 자체를 끄는 작업이 아니다.
BT Service의 context 갱신 주기를 늘렸을 때 다음 항목이 어떻게 변하는지 확인한다.

```text
BehaviorTreeTick
BT_UpdateAIContext
BT_UpdateAIIntentState
BT_UpdateEngageContext
CharacterMovement
FrameTime / GameThreadTime
AI 반응 지연
Engage / Alert / Attack 전환 안정성
```

Movement / Nav 측정에서 `MovementMode 2`는 frame 개선을 만들었지만 gameplay state를 크게 바꿨다.
따라서 다음 단계는 이동을 직접 막는 것이 아니라, BT context 갱신 빈도를 낮춰 movement / target / engage 판단이 얼마나 자주 발생해야 하는지 확인하는 것이다.

## 현재 코드 스캔 결과

### 주요 BT Service

| Service | 기본 Interval | CSV stat | 주요 역할 | 측정 의미 |
| --- | ---: | --- | --- | --- |
| `UCBTService_UpdateAIContext` | `0.1s` | `PortfolioAI_BT_UpdateAIContext` | Perception top target, home metric, alert range, engage assignment, dead / reaction context 갱신 | target / movement context 갱신 비용 |
| `UCBTService_UpdateAIIntentState` | `0.2s` | `PortfolioAI_BT_UpdateAIIntentState` | Blackboard context 기반 AIIntentState 결정 | state transition 빈도와 반응 지연 |
| `UCBTService_UpdateEngageContext` | `0.1s` | `PortfolioAI_BT_UpdateEngageContext` | engage range, combat action 가능 여부, cooldown 기반 engage context 갱신 | 공격 가능 판단 / combat timing 보조 비용 |
| `UCBTService_UpdateInvestigateContext` | `0.1s` | `PortfolioAI_BT_UpdateInvestigateContext` | investigate context 갱신 | 현재 Engage 측정에서는 보조 축 |

현재 집중 대상:

```text
UpdateAIContext
UpdateAIIntentState
UpdateEngageContext
```

`UpdateInvestigateContext`는 Engage 중심 측정에서 호출 비중이 낮을 가능성이 높으므로 1차 측정 대상에서는 제외한다.
필요하면 Investigate 전용 측정에서 별도로 본다.

### 비용 전파 구조

```text
Perception / TargetDataMap
-> UpdateAIContext
-> TargetActor / bHasLOS / Distance / AlertRange / EngageAssignment 갱신
-> UpdateAIIntentState
-> Idle / Investigate / Chase / Alert / Engage 결정
-> BT branch 전환
-> MoveTo / MovementIntent / Engage action 시도
```

`BT Update Interval`을 늘리면 직접적으로는 BT Service 호출 빈도가 줄어든다.
하지만 더 중요한 관찰 지점은 다음이다.

```text
movement request 빈도 감소 여부
CharacterMovement p95 감소 여부
Engage / Alert 전환 지연 증가 여부
attack timing이 과도하게 느려지는지 여부
```

## 구현 후보

첫 구현은 profiling 전용 CVar로 둔다.

```text
Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode
```

값:

```text
0: Default
1: Reduced
2: AggressiveReduced
```

권장 interval:

| Mode | UpdateAIContext | UpdateAIIntentState | UpdateEngageContext | 목적 |
| ---: | ---: | ---: | ---: | --- |
| 0 | `0.1s` | `0.2s` | `0.1s` | 기준값 |
| 1 | `0.1s` | `0.3s` | `0.1s` | 의도 상태 갱신만 보수적으로 감소 |
| 2 | `0.1s` | `0.5s` | `0.1s` | 의도 상태 갱신만 공격적으로 감소 |

설계 기준:

```text
UpdateAIContext는 target / movement context와 CombatEngage request를 갱신하므로 기본 interval을 유지한다.
UpdateAIIntentState는 너무 느리면 state transition 체감 지연이 생길 수 있다.
UpdateEngageContext는 combat action 가능 판단과 연결되므로 기본 interval을 유지한다.
```

대안:

```text
서비스별 CVar를 개별로 둔다.
예: AIContextInterval, AIIntentInterval, EngageContextInterval
```

1차 구현에서는 mode 기반 CVar를 권장한다.
서비스별 CVar는 조합 수가 늘어나고 측정 케이스가 과도하게 분산된다.
mode 기반으로 유효성이 보이면 후속 구현에서 거리 / combat relevance 기반으로 세분화한다.

## 구현 위치 제안

각 BT Service 생성자에서 기본 interval을 설정하고 있으므로, profiling mode 적용 위치는 다음 중 하나를 선택한다.

### 후보 1. Service 생성자에서 CVar 기반 interval 설정

장점:

```text
구현이 단순하다.
BT Service interval의 출처가 명확하다.
PIE 시작 전 CVar를 설정하면 측정 조건이 고정된다.
```

단점:

```text
PIE 중 CVar 변경을 즉시 반영하지 못한다.
runtime LOD 전환 모델을 직접 검증하기 어렵다.
```

### 후보 2. TickNode에서 interval mode 변경 감지

장점:

```text
PIE 중 CVar 변경을 반영할 수 있다.
향후 runtime LOD 전환 구조와 유사하다.
```

단점:

```text
모든 service tick마다 CVar와 mode 상태를 확인한다.
profiling 제어 코드가 service 본문에 더 많이 섞인다.
```

1차 측정 권장:

```text
생성자 또는 OnBecomeRelevant 계열에서 PIE 시작 전 CVar 기준으로 적용한다.
```

이유:

```text
이번 측정 목적은 runtime toggle 검증이 아니라 interval 변화에 따른 비용 분리다.
측정 조건은 PIE 시작 전 고정하는 편이 결과 해석이 쉽다.
```

## 측정 범위

40 / 80 Enemy 기준으로 같은 scale을 사용한다.

```text
BT00: 40 Enemy / BTUpdateIntervalMode 0
BT01: 40 Enemy / BTUpdateIntervalMode 1
BT02: 40 Enemy / BTUpdateIntervalMode 2

BT03: 80 Enemy / BTUpdateIntervalMode 0
BT04: 80 Enemy / BTUpdateIntervalMode 1
BT05: 80 Enemy / BTUpdateIntervalMode 2
```

필수 공통 조건:

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Camera: fixed camera
EnemyMeshMode 0
EnemyAnimationMode 0
EnemyAnimationRefreshCounter 0
DisableEnemyWeaponActor 0
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
CanMoveDecoratorAudit 0
EnemyMovementMode 0
```

맵 조건:

```text
MAP_AIPerf_MovementNav_40Enemy 또는 동일한 gameplay stress 조건의 BT interval 전용 맵
MAP_AIPerf_MovementNav_80Enemy 또는 동일한 gameplay stress 조건의 BT interval 전용 맵
Player는 빠르게 인식 가능한 위치
Enemy끼리 피격 없음
Enemy끼리 길막이 측정 불가능할 정도로 심하지 않음
고정 카메라
```

측정 전 확인:

```text
BTUpdateIntervalMode가 의도한 값으로 적용됐는지 확인한다.
stat ai에서 BehaviorTree tick이 정상적으로 표시되는지 확인한다.
PIE 관측상 Enemy가 Perception -> Alert / Engage로 진입하는지 확인한다.
Mode 1 / 2에서 반응 지연이 너무 커져 측정 자체가 다른 상태로 바뀌지 않는지 확인한다.
```

## 주요 지표

Primary:

```text
FrameTime p95
GameThreadTime p95
Exclusive/GameThread/BehaviorTreeTick p95
GameThread/PortfolioAI_BT_UpdateAIContext p95
GameThread/PortfolioAI_BT_UpdateAIIntentState p95
GameThread/PortfolioAI_BT_UpdateEngageContext p95
PortfolioAI_BT_UpdateAIContext_Count
PortfolioAI_BT_UpdateAIIntentState_Count
PortfolioAI_BT_UpdateEngageContext_Count
Exclusive/GameThread/CharacterMovement p95
```

Secondary:

```text
Exclusive/GameThread/AIPerception p95
Exclusive/GameThread/BehaviorTreeSearch p95
Ticks/BehaviorTreeComponent p95
Ticks/CAIController p95
Ticks/CEnemy p95
Animation p95
AnimationParallelEvaluation TotalTaskTime p95
RHI/DrawCalls p95
```

`Ticks/*` 지표 해석:

```text
Ticks/BehaviorTreeComponent, Ticks/CAIController, Ticks/CEnemy는 시간 비용이 아니라 해당 frame에서 tick된 object / component 수다.
BTUpdateIntervalMode가 바뀌어도 tick count가 반드시 줄어드는 것은 아니다.
실제 비용 감소는 BehaviorTreeTick과 각 PortfolioAI_BT_* timing stat으로 판단한다.
```

## 해석 기준

```text
BTUpdateIntervalMode 1에서 BehaviorTreeTick / BT_UpdateAIContext p95가 줄고 Frame / GameThread p95도 줄면,
BT context update interval은 유효한 Runtime LOD 후보로 본다.

BT_UpdateAIContext는 줄지만 Frame / GameThread p95가 거의 변하지 않으면,
BT context update는 직접 병목이 아니라 downstream trigger 또는 작은 비용 축으로 본다.

Mode 2에서 비용은 줄지만 Alert / Engage / Attack 반응이 과도하게 늦어지면,
aggressive interval은 gameplay-safe 후보에서 제외하고 distance / non-combat 전용 후보로 둔다.

CharacterMovement p95가 함께 줄면,
BT context update 빈도가 movement request / path request 빈도에 영향을 준 것으로 해석한다.

CharacterMovement p95가 줄지 않으면,
movement 비용은 BT interval보다 active movement / path following 자체의 비용으로 본다.
```

## PIE 관측 체크리스트

수치와 별개로 다음 항목을 함께 기록한다.

```text
Enemy가 Player를 인식하는 시간
Alert Spread가 자연스럽게 시작되는지
Engage 진입이 지나치게 늦지 않은지
가까운 Enemy가 공격을 너무 늦게 시작하지 않는지
Idle / Alert / Engage 전환이 흔들리지 않는지
MovementMode 0 조건과 비교해 gameplay smoke가 유지되는지
```

특히 Mode 2는 다음을 주의한다.

```text
BT context가 너무 늦게 갱신되면 Enemy가 이미 위치를 바꿨는데도 이전 Blackboard 값을 기준으로 판단할 수 있다.
EngageContext interval이 너무 길면 공격 가능 조건이 늦게 열리거나 늦게 닫힐 수 있다.
```

## 측정 결과

40 Enemy / fixed camera / gameplay stress 조건에서 `BTUpdateIntervalMode` 0 / 1 / 2를 비교했다.

측정 조건:

```text
Map: MAP_AIPerf_BTUpdateInterval_40Enemy
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
EnemyMeshMode 0
EnemyAnimationMode 0
EnemyAnimationRefreshCounter 0
DisableEnemyWeaponActor 0
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
CanMoveDecoratorAudit 0
EnemyMovementMode 0
```

측정 CSV:

```text
BT00 / Mode 0: Profile(20260707_204029).csv
BT01 / Mode 1: Profile(20260707_204226).csv
BT02 / Mode 2: Profile(20260707_204428).csv
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext p95 | AIIntent p95 | EngageContext p95 | CharacterMovement p95 | 판정 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| BT00 | 0 | 13.8327ms | 13.7779ms | 0.2833ms | 0.1353ms | 0.0279ms | 0.0033ms | 1.2632ms | 기준 |
| BT01 | 1 | 13.7358ms | 13.7197ms | 0.2806ms | 0.1354ms | 0.0322ms | 0.0032ms | 1.2853ms | 변화 없음 |
| BT02 | 2 | 13.6433ms | 13.6495ms | 0.2845ms | 0.1342ms | 0.0305ms | 0.0031ms | 1.2381ms | 변화 없음 |

해석:

```text
Mode 1 / 2에서 BehaviorTreeTick, BT_UpdateAIContext, BT_UpdateAIIntentState, BT_UpdateEngageContext p95가 줄지 않았다.
따라서 현재 구현처럼 OnBecomeRelevant에서 Service Interval 값을 덮어쓰는 방식은 기존 BT asset service scheduling에 유효하게 반영되지 않은 것으로 본다.

Frame / GameThread / CharacterMovement p95도 오차 수준에서만 변했다.
즉 이번 측정은 BT update interval 축의 성능 효과가 없다는 결과라기보다, 현재 runtime override 방식이 측정 축을 제대로 만들지 못했다는 결과다.
```

후속 보완:

```text
BTUpdateIntervalMode를 계속 측정하려면 Service Interval 속성만 바꾸는 방식이 아니라,
service 내부에서 자체 elapsed time gate를 두거나,
BT service memory / next tick scheduling을 직접 제어하는 방식으로 다시 구현해야 한다.

현재 결과만으로 BT Update Interval LOD의 효과 유무를 확정하지 않는다.
이번 결과는 "OnBecomeRelevant 기반 interval override는 유효 측정 방식이 아니었다"로 기록한다.
```

## 종료 조건

```text
40 / 80 Enemy 기준 BTUpdateIntervalMode 0 / 1 / 2를 비교한다.
BehaviorTreeTick과 BT_UpdateAIContext가 frame budget에 미치는 영향을 확인한다.
Frame / GameThread 개선과 gameplay 반응 지연을 함께 기록한다.
Mode 1이 비용 감소와 gameplay 안정성을 모두 만족하면 Runtime LOD 후보로 남긴다.
Mode 2는 효과가 크더라도 combat-capable 단계에 바로 적용하지 않고 far / non-combat 후보로 분류한다.
효과가 제한적이면 Perception Active Budget 또는 Collision / Overlap 축으로 넘어간다.
```

## 권장 커밋 분리

```text
docs(ai): plan bt update interval lod measurement
feat(ai): add bt update interval profiling control
docs(ai): record bt update interval profiling results
```

---

## ScheduleNextTick 재측정 결과

`OnBecomeRelevant`에서 `Interval` 속성을 덮어쓰는 방식은 BT service scheduling에 유효하게 반영되지 않았다.
이후 `ScheduleNextTick`에서 `SetNextTickTime`을 직접 호출하는 방식으로 보완했고, 40 Enemy 조건에서 `BTUpdateIntervalMode` 0 / 1 / 2를 다시 측정했다.

측정 조건:

```text
Map: MAP_AIPerf_BTUpdateInterval_40Enemy
Camera: fixed camera
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
EnemyMeshMode 0
EnemyAnimationMode 0
EnemyAnimationRefreshCounter 0
DisableEnemyWeaponActor 0
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
CanMoveDecoratorAudit 0
EnemyMovementMode 0
```

측정 CSV:

```text
BT10 / Mode 0: Profile(20260707_223738).csv
BT11 / Mode 1: Profile(20260707_223956).csv
BT12 / Mode 2: Profile(20260707_224402).csv
```

| Case | Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext active | AIIntent active | EngageContext active | 판정 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| BT10 | 0 | 13.2421ms | 13.2233ms | 0.2476ms | 294 | 152 | 384 | 기준 |
| BT11 | 1 | 12.7211ms | 12.7218ms | 0.2149ms | 152 | 103 | 4 | interval 제어는 동작하나 전투 상태 전환 불안정 |
| BT12 | 2 | 12.7915ms | 12.7800ms | 0.1925ms | 77 | 61 | 0 | interval 제어는 동작하나 Engage 판단 붕괴 |

해석:

```text
ScheduleNextTick 기반 제어는 정상 동작한다.
AIContext / AIIntent active count와 BT Tick p95가 감소했으므로 BT service 작업량은 줄어든 것으로 해석했다.
다만 active count는 frame 단위 기록 수라 호출 횟수의 근사치일 뿐이며, 후속 측정에서는 `*_Count` counter를 우선 지표로 사용한다.
다만 Frame / Game p95 개선 폭은 작다.
현재 조건에서 BT service interval은 primary bottleneck이 아니라 보조 최적화 축으로 본다.
```

Gameplay 관찰:

```text
Mode 0은 정상적으로 Engage / Attack 흐름이 유지됐다.
Mode 1은 Engage에 들어갔다가 빠지는 현상이 반복됐고, Attack 진입이 불안정했다.
Mode 2는 Engage에 들어가는 객체가 사실상 사라졌다.
```

정책 결론:

```text
BT interval LOD는 전역 최적값을 찾는 문제가 아니다.
어떤 Enemy에게 어떤 service precision을 줄 것인지 먼저 정책화해야 한다.

AIContext처럼 CombatEngage request를 생산하는 service는 high precision을 유지한다.
EngageContext처럼 전투 진입, 공격 가능 여부, 할당, 거리 판단에 직접 연결되는 service도 high precision을 유지한다.
AIIntentState처럼 상위 상태 결정 성격이 강한 service는 distance / combat relevance / LOD tier에 따라 완화할 수 있다.
Patrol / Alert / Investigate처럼 반응 지연 허용 폭이 큰 상태는 low precision 후보로 본다.
```

후속 설계 방향:

```text
전역 BTUpdateIntervalMode는 측정용 fallback으로 남긴다.
실제 Runtime LOD에서는 Enemy별 Runtime LOD tier 또는 AI update priority를 계산한다.
BT service는 해당 tier와 service role을 함께 보고 다음 tick interval을 결정한다.

High Precision:
현재 Engage 중인 Enemy, Engage 후보군, Player와 가까운 Enemy, Attack / Guard / Reaction 가능 상태

Reduced Precision:
Alert spread 중인 Enemy, 전투권 밖 추적 객체, 상위 context 갱신만 필요한 객체

Low Precision:
Patrol / Investigate / ReturnToHome, 멀리 있는 객체, 화면 밖 또는 전투 영향권 밖 객체

Dormant:
BT pause 또는 매우 낮은 빈도 후보
```

빈도값 튜닝 기준:

```text
최적 interval 값 탐색은 Enemy별 tier와 service별 precision policy가 생긴 뒤에 진행한다.
정책 없이 전역 interval만 조절하면 Engage / Attack 기준과 Patrol / Alert 기준이 충돌한다.
따라서 현재 단계의 결론은 "BT interval 제어는 유효하지만, 전역 적용은 부적합하다"로 기록한다.
```

## Precision Policy 구현 v1

구현 내용:

```text
UCWorldSubsystem_CombatEngage가 AI update precision을 제공한다.
EAIUpdatePrecision은 High / Reduced / Low로 구분한다.
BT service interval helper는 AIContext를 기본 interval로 고정하고, AIIntentState interval만 AIController precision에 따라 결정한다.
EngageContext는 전투 상태 전환 안정성을 위해 Runtime LOD mode와 관계없이 기본 interval을 유지한다.
```

Precision 기준:

```text
High:
현재 Engage assignment를 받은 AIController

Reduced:
Alert assignment를 받은 AIController
현재 Engage request container에 들어온 AIController

Low:
Engage assignment / request가 없는 AIController
```

Assignment 상한:

```text
MaxEngagersPerTarget:
target당 Engage assignment를 받을 수 있는 AIController 수

MaxAlertersPerTarget:
target당 Alert assignment를 받을 수 있는 AIController 수

정렬된 request 중 MaxEngagersPerTarget 범위는 Engage로 저장한다.
그 다음 MaxAlertersPerTarget 범위는 Alert로 저장한다.
나머지는 AssignmentContainer에 저장하지 않는다.
```

따라서 `AssignmentContainer`는 precision policy의 기준 테이블 역할을 한다.

Blackboard 반영:

```text
UpdateAIContext는 CombatEngage subsystem의 assignment 결과를 Blackboard에 기록한다.
CombatRole == Engage이면 AIIntentState는 Engage로 진입한다.
CombatRole == Alert이면 AIIntentState는 Alert로 진입한다.
CombatRole == None이면 target이 있어도 Investigate / Chase / Alert / Engage로 진입하지 않고 Idle로 되돌린다.
Investigate / Chase는 CombatRole == Engage 또는 Alert인 객체가 target을 잃거나 거리 조건을 벗어났을 때만 허용한다.
```

이 구조에서는 `bShouldEngage`를 Engage 호환 플래그로 유지하지만, Alert / Idle 분기는 `CombatRole`을 기준으로 한다.
따라서 `MaxAlertersPerTarget` 밖의 Enemy는 target을 인식해도 Chase / Alert Spread에 들어가지 않고 Idle_Router의 wait 경로로 남는다.

Blackboard asset 확인:

```text
Engage/CombatRole enum key가 Blackboard asset에 존재해야 한다.
누락되면 CAIKeyRegistry required key 검증에서 ensure가 발생한다.
```

Interval 정책:

```text
BTUpdateIntervalMode 0:
모든 service가 기본 interval을 사용한다.

BTUpdateIntervalMode 1:
High는 기본 interval을 유지한다.
Reduced / Low는 reduced interval을 사용한다.

BTUpdateIntervalMode 2:
High는 기본 interval을 유지한다.
Reduced는 reduced interval을 사용한다.
Low는 aggressive interval을 사용한다.
```

Service별 적용:

```text
AIContext:
항상 기본 interval 유지

AIIntentState:
precision policy 적용

EngageContext:
항상 기본 interval 유지
```

검증 기준:

```text
Mode 1 / 2에서 AIIntentState count가 줄어드는지 확인한다.
AIContext count는 Mode 0과 유사하게 유지되는지 확인한다.
EngageContext count는 Mode 0과 유사하게 유지되는지 확인한다.
Engage / Attack 상태 전환이 Mode 1 / 2에서도 깨지지 않는지 확인한다.
Frame / Game p95 개선보다 service count와 gameplay 안정성을 함께 본다.
```
