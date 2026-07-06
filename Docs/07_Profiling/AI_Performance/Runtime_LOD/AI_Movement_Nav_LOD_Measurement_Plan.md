# AI Movement / Nav LOD Measurement Plan

## 목적

`Movement / Nav` 축이 40 / 80 Enemy 조건에서 실제 frame budget에 어느 정도 영향을 주는지 분리한다.

이 측정은 최종 Runtime LOD 구현이 아니라 비용 분리 측정이다.
이동을 끄거나 path following을 멈추면 gameplay state가 크게 달라지기 때문에, 먼저 비용 상한과 병목 가능성을 확인한 뒤 실제 적용 방식은 별도로 결정한다.

## 현재 코드 스캔 결과

### 책임 분리

현재 이동 관련 비용은 크게 세 층으로 나뉜다.

| 층 | 주요 위치 | 역할 | 측정 의미 |
| --- | --- | --- | --- |
| Movement Component | `UCMovementComponent` | speed / direction / falling 갱신, gait / rotation mode 적용 | custom movement state tick 비용 |
| CharacterMovement / PathFollowing | `UCharacterMovementComponent`, `PathFollowingComponent`, BT `MoveTo` asset node | 실제 이동, nav path following, 회전/속도 처리 | UE movement / nav runtime 비용 |
| BT Movement Decision | `CBTService_UpdateAIContext`, `CBTService_UpdateAIIntentState`, movement intent task | home / target 거리 계산, state 결정, gait 요청 | movement decision / blackboard update 비용 |

### C++에서 확인된 흐름

`UCMovementComponent`

```text
TickComponent
-> CalculateSpeed
-> CalculateDirection
-> CharacterMovementComp->IsFalling
```

`CBTTask_RequestMovementIntent`

```text
MovementIntent
-> Enemy HandleAIWalk / HandleAIRun / HandleAISprint / HandleAIJump / HandleAIStopJump
-> ActionOrchestrator RequestMovementAction
-> MovementComponent gait / movement policy 반영
```

`CBTDecorator_CanMove`

```text
Pawn FindComponentByClass<UCMovementComponent>
-> CanAcceptMoveInput
```

`CBTService_UpdateAIContext`

```text
HomeLocation 거리 계산
TargetActor 거리 계산
AlertRange 계산
EngageAssignment 계산
```

`CBTService_UpdateAIIntentState`

```text
Target / LOS / AlertRange / EngageAssignment 기반으로
Idle / Investigate / Chase / Alert / Engage 상태 결정
```

### C++에서 직접 보이지 않는 영역

실제 `MoveTo` 실행은 현재 C++ task로 감싸져 있지 않고 BT asset의 built-in `MoveTo` 노드에 있을 가능성이 높다.
따라서 C++ 스캔만으로는 모든 이동 비용을 직접 추적할 수 없다.

측정 시 `stat ai`, `stat game`, CSV의 다음 지표를 함께 본다.

```text
Exclusive/GameThread/CharacterMovement
Ticks/CharacterMovementComponent
Ticks/PathFollowingComponent
Exclusive/GameThread/BehaviorTreeTick
GameThread/PortfolioAI_BT_UpdateAIContext
GameThread/PortfolioAI_BT_UpdateAIIntentState
FrameTime
GameThreadTime
```

## 측정 대상 분류

Movement / Nav 축은 다음 세 가지 비용을 분리해서 본다.

### 1. Movement State Tick

`UCMovementComponent`가 매 frame speed / direction / falling을 갱신하는 비용이다.

측정 후보:

```text
UCMovementComponent tick 유지
UCMovementComponent tick disable
```

주의:

```text
AnimInstance가 MovementComponent의 CurrentSpeed / CurrentDirection을 읽는다.
tick을 끄면 locomotion parameter가 stale 상태가 될 수 있다.
따라서 gameplay-safe 최적화라기보다 비용 분리 측정으로 먼저 본다.
```

### 2. CharacterMovement / PathFollowing

실제 이동, path following, collision 기반 movement 비용이다.

측정 후보:

```text
기본 이동 유지
movement intent 차단
현재 path following 1회 정리
CharacterMovement tick disable 후보
```

주의:

```text
MoveTo를 차단하면 Alert / Engage 진입 위치와 전투 상태가 달라진다.
CharacterMovement tick을 끄면 캐릭터 위치 자체가 정지하고 animation / combat state도 연쇄적으로 달라질 수 있다.
```

### 3. Movement Decision / BT Context

거리 계산, alert range, home range, engage assignment로 이어지는 decision 비용이다.

측정 후보:

```text
BT context update 유지
BT update interval 증가
movement/nav 관련 context만 간소화
```

주의:

```text
이 축은 Movement / Nav와 BT Update Interval 사이에 걸쳐 있다.
P35에서는 Movement / Nav 측정에서 비용 신호를 확인하고, 실제 update interval 조정은 BT Update Interval 축에서 다시 다룬다.
```

## 구현된 CVar

첫 구현에서는 침습도가 낮고 되돌리기 쉬운 측정용 CVar부터 둔다.

```text
Portfolio.AI.RuntimeLOD.EnemyMovementMode
```

값:

```text
0: Default
1: MovementStateRefreshDisabled
2: BlockMovementIntent
```

1차 구현:

```text
0: Default
1: MovementStateRefreshDisabled
2: BlockMovementIntent
```

`DisableCharacterMovementTick`, SkeletalMesh tick disable, `bPauseAnims` 같은 강한 제어는 gameplay / animation state를 크게 바꾼다.
따라서 Movement / Nav 측정에 포함하지 않고 별도 제한 환경 측정으로 분리한다.

현재 구현 위치:

```text
ACEnemy::UpdateRuntimeLODMovementMode
-> EnemyMovementMode CVar를 읽는다.
-> Mode 1은 UCMovementComponent tick을 비활성화해 speed / direction / falling state refresh를 멈춘다.
-> Mode 2는 UCMovementComponent::SetStop으로 movement intent를 차단한다.
-> Mode 2 진입 시 현재 path following은 AIController::StopMovement로 1회 정리한다.
-> Mode 2 유지 중에는 Action / Reaction 종료가 SetMove를 호출해도 SetStop 상태를 다시 유지한다.
```

## 측정 범위

Movement / Nav 측정은 두 제어만 사용한다.

| Mode | 이름 | 목적 |
| ---: | --- | --- |
| 0 | Default | 기준값 |
| 1 | MovementStateRefreshDisabled | `UCMovementComponent`의 speed / direction / falling cache refresh 비용 분리 |
| 2 | BlockMovementIntent | movement intent / MoveTo / path following / CharacterMovement 흐름 비용 분리 |

이 측정은 locomotion animation, pose update, SkeletalMesh tick 비용을 직접 측정하지 않는다.
해당 축은 전투 기능을 제거한 제한 환경에서 별도로 측정한다.

별도 측정 후보:

```text
Character Actor tick
CharacterMovementComponent tick
SkeletalMeshComponent tick
bPauseAnims
VisibilityBasedAnimTickOption
Animation update / pose progression
```

## 제안 측정 케이스

Animation / Pose 축과 같은 40 / 80 scale을 사용한다.
AnimationMode는 기본값으로 고정하고, MovementMode만 바꾼다.

```text
MV00: 40 Enemy / MovementBaseline / MovementMode 0
MV01: 40 Enemy / MovementStateRefreshDisabled / MovementMode 1
MV02: 40 Enemy / BlockMovementIntent / MovementMode 2

MV03: 80 Enemy / MovementBaseline / MovementMode 0
MV04: 80 Enemy / MovementStateRefreshDisabled / MovementMode 1
MV05: 80 Enemy / BlockMovementIntent / MovementMode 2
```

## 공통 측정 조건

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
EnemyMeshMode 0
DisableEnemyWeaponActor 0
DisableEnemyPerception 0
PerceptionCandidateAudit 0
BlackboardEngageLatencyAudit 0
EnemyAnimationMode 0
```

맵 조건:

```text
AIPerf Enemy 40 / 80
Player는 빠르게 인식 가능한 위치
Enemy끼리 피격 없음
Enemy끼리 길막이 측정 불가능할 정도로 심하지 않음
고정 카메라 권장
```

고정 카메라를 권장하는 이유:

```text
Movement / Nav 측정에서도 화면 밖으로 나간 Enemy 수가 크게 달라지면 render / animation 조건이 흔들린다.
RenderCoverage처럼 draw call을 엄격히 통제하려는 목적은 아니지만, 같은 movement stress를 관찰하기 위해 camera 조건은 고정한다.
```

## 주요 지표

Primary:

```text
FrameTime p95
GameThreadTime p95
Exclusive/GameThread/CharacterMovement p95
Ticks/CharacterMovementComponent p95
Ticks/PathFollowingComponent p95
Ticks/CEnemy p95
Ticks/CAIController p95
```

Secondary:

```text
Exclusive/GameThread/BehaviorTreeTick p95
GameThread/PortfolioAI_BT_UpdateAIContext p95
GameThread/PortfolioAI_BT_UpdateAIIntentState p95
Exclusive/GameThread/AIPerception p95
Animation p95
AnimationParallelEvaluation TotalTaskTime p95
RHI/DrawCalls p95
```

## 해석 기준

```text
MovementStateRefreshDisabled로 CharacterMovement p95가 거의 줄지 않으면,
custom movement state refresh는 주요 병목이 아니라고 본다.

BlockMovementIntent로 CharacterMovement / PathFollowing / GameThread p95가 크게 줄면,
실제 이동 요청과 nav path following이 유효한 Runtime LOD 축이다.

BlockMovementIntent로 frame은 줄지만 gameplay 상태가 크게 변하면,
최적화 적용안은 movement disable이 아니라 path update interval / active movement budget / distant movement simplification 쪽으로 설계한다.

CharacterMovement tick disable이 큰 효과를 보이면,
far / dormant enemy에서 movement simulation을 proxy 또는 lightweight actor로 대체하는 후속 작업 후보가 된다.
```

## 구현 전 체크리스트

```text
1. BT asset에서 MoveTo 노드 위치와 조건을 확인한다.
2. AIPerf Movement/Nav 전용 맵을 만들지, 기존 AnimationLOD 맵을 복제할지 결정한다.
3. EnemyMovementMode CVar 제어 위치를 결정한다.
4. MovementComponent tick disable과 MovementIntent block이 각각 의도한 축만 제어하는지 확인한다.
5. BlockMovementIntent 측정 시 AI state가 계속 Engage/Alert를 유지하는지 확인한다.
```

## 권장 커밋 분리

```text
docs(ai): plan movement nav lod measurement
feat(ai): add movement nav profiling control
chore(ai): prepare movement nav profiling maps
docs(ai): record movement nav profiling results
```

## 종료 조건

```text
40 / 80 Enemy 기준 MovementBaseline과 최소 1개 reduced 조건을 비교한다.
CharacterMovement / PathFollowing 비용이 frame budget에 미치는 영향을 확인한다.
Movement / Nav가 주요 병목이면 후속 Runtime LOD 구현 후보를 정의한다.
효과가 제한적이면 BT Update Interval 또는 Collision / Overlap 축으로 넘어간다.
```
