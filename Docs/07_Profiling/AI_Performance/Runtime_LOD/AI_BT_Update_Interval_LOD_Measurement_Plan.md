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
| 1 | `0.2s` | `0.3s` | `0.2s` | 보수적 감소 |
| 2 | `0.4s` | `0.5s` | `0.3s` | 공격적 감소 |

설계 기준:

```text
UpdateAIContext는 target / movement context의 핵심이므로 가장 먼저 본다.
UpdateAIIntentState는 너무 느리면 state transition 체감 지연이 생길 수 있다.
UpdateEngageContext는 combat action 가능 판단과 연결되므로 공격 템포가 깨지는지 확인한다.
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
