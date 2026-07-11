# AI Component Tick Audit Plan

## 목적

`Component Tick` 축이 AI Runtime LOD v1의 실질적인 최적화 후보인지 확인한다.

이번 문서는 Unreal Engine 기본 tick 전체를 줄이는 계획이 아니다. 프로젝트 코드가 직접 가진 Enemy-side actor / component tick 중 Runtime LOD로 줄일 가치가 있는 항목을 분리한다.

## 배경

이전 Runtime LOD 측정에서 다음 축은 이미 별도 문서로 분리했다.

| 축 | 관련 문서 |
| --- | --- |
| Mesh / pose / render | `Enemy_Mesh_Runtime_LOD_Measurements.md` |
| Animation parameter refresh | `AI_Animation_Pose_LOD_Measurement_Plan.md` |
| Movement / nav / path following | `AI_Movement_Nav_LOD_Measurement_Plan.md` |
| BT service interval | `AI_BT_Update_Interval_LOD_Result_Note.md` |
| Combat collision / hit window | `AI_Combat_Collision_HitWindow_Measurement_Plan.md` |
| Combat feedback presentation | `AI_Combat_Feedback_Presentation_Measurement_Plan.md` |

따라서 이 문서의 `Component Tick`은 다음처럼 좁게 정의한다.

```text
ACEnemy::Tick
UCMovementComponent::TickComponent
UCActionComponent::TickComponent
```

`CharacterMovementComponent`, `SkeletalMeshComponent`, `BehaviorTreeComponent`는 엔진 또는 시스템 축이므로 이 문서에서 직접 off 후보로 다루지 않는다. 해당 비용은 Movement / Animation / BT 문서에서 해석한다.

## 제어 CVar

```text
Portfolio.AI.RuntimeLOD.EnemyComponentTickMode
```

| Mode | 이름 | 제어 내용 | 목적 |
| ---: | --- | --- | --- |
| 0 | Default | 기본 tick 유지 | baseline |
| 1 | EnemyActorTickDisabled | `ACEnemy::Tick` 비활성화 | Enemy actor-level polling tick 비용 분리 |

현재 구현 범위는 Mode 1까지다.

주의:

- Mode 1은 `ACEnemy::Tick` 자체를 끄므로 PIE 중 CVar를 1에서 0으로 되돌리는 runtime restore 검증에는 적합하지 않다.
- 정규 측정은 PIE 시작 전 CVar를 설정한 뒤 진행한다.
- Mode 1은 `CActionComponent`, `CMovementComponent`, `CharacterMovementComponent`, `BehaviorTreeComponent`, `SkeletalMeshComponent` tick을 끄지 않는다.

## 측정 조건

공통 조건:

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
Camera: fixed camera
GC 이벤트 없음
```

공통 CVar:

```text
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
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 0
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback 0
```

Gameplay smoke:

- Engage 2 유지
- Alert 6 유지
- Observe / Idle 계층 유지
- attack montage 정상
- hit / guard / parry result 정상
- feedback presentation 정상

## 대표 지표

| 지표 | 해석 |
| --- | --- |
| `FrameTime p95` | 최종 체감 frame budget |
| `GameThreadTime p95` | GameThread 전체 변동 |
| `Exclusive/GameThread/TickActors p95` | actor/component tick 묶음 비용 |
| `Exclusive/GameThread/CharacterMovement p95` | movement simulation / path following 영향 |
| `Exclusive/GameThread/Animation p95` | animation / pose update 영향 |
| `Exclusive/GameThread/BehaviorTreeTick p95` | BT update 변동 |
| `Ticks/CEnemy` | Mode 1 적용 여부 확인 |

## 40 Enemy 결과

| Set | Mode | Frame p95 | Game p95 | TickActors p95 | CharacterMovement p95 | Animation p95 | BT Tick p95 | CEnemy Tick |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 40 R1 | 0 | 11.9250ms | 10.8108ms | 1.4724ms | 0.5084ms | 1.9996ms | 0.2198ms | 40 |
| 40 R1 | 1 | 12.0640ms | 11.0778ms | 1.4959ms | 0.5112ms | 2.0839ms | 0.2195ms | missing |
| 40 R2 | 0 | 11.9344ms | 11.0786ms | 1.5314ms | 0.5191ms | 2.1083ms | 0.2191ms | 40 |
| 40 R2 | 1 | 11.9257ms | 11.1269ms | 1.4990ms | 0.5449ms | 2.1380ms | 0.2232ms | missing |

40 Enemy에서는 Mode 1에서 `Ticks/CEnemy`가 사라진다. 즉 `ACEnemy::Tick` 비활성화는 정상 적용됐다.

하지만 `FrameTime`, `GameThreadTime`, `TickActors`는 개선되지 않았다. `BT Tick`은 거의 동일하며, `CharacterMovement`와 `Animation` 쪽 변동이 더 크게 보인다.

## 80 Enemy 결과

| Set | Mode | Frame p95 | Game p95 | TickActors p95 | CharacterMovement p95 | Animation p95 | BT Tick p95 | CEnemy Tick |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 80 R1 | 0 | 17.8866ms | 17.8446ms | 2.9108ms | 0.9470ms | 4.0210ms | 0.4111ms | 80 |
| 80 R1 | 1 | 17.1558ms | 17.1629ms | 2.7634ms | 0.9095ms | 3.8622ms | 0.4075ms | missing |
| 80 R2 | 0 | 16.7917ms | 16.7457ms | 2.7682ms | 0.8442ms | 3.7991ms | 0.4129ms | 80 |
| 80 R2 | 1 | 17.2328ms | 17.1904ms | 2.7561ms | 0.9052ms | 3.9476ms | 0.4171ms | missing |
| 80 R3 | 0 | 19.1129ms | 19.1179ms | 3.0502ms | 0.9456ms | 4.3873ms | 0.4273ms | 80 |
| 80 R3 | 1 | 17.6868ms | 17.7054ms | 2.9069ms | 0.9464ms | 4.1182ms | 0.4171ms | missing |

80 Enemy에서도 Mode 1은 `Ticks/CEnemy`를 제거한다.

다만 Frame / Game p95 개선은 반복 측정에서 안정적으로 재현되지 않았다.

- R1: Mode 1이 좋아 보인다.
- R2: Mode 1이 오히려 나빠 보인다.
- R3: Mode 1이 좋아 보이지만 Mode 0의 Animation / GameThread가 크게 튄다.

따라서 80 Enemy에서도 `ACEnemy::Tick` 제거만으로 안정적인 frame 개선이 있다고 결론내리기 어렵다.

## 해석

`EnemyComponentTickMode 1`은 기능적으로 성공했다.

```text
Mode 0: Ticks/CEnemy = 40 또는 80
Mode 1: Ticks/CEnemy = missing
```

하지만 성능 병목으로 보기는 어렵다.

```text
BT Tick은 반복 측정에서 비교적 안정적이다.
Frame / GameThread 변동은 CharacterMovement와 Animation 변동을 더 크게 따라간다.
TickActors p95 감소폭은 작고, Frame / Game p95 개선으로 안정적으로 이어지지 않는다.
```

즉 이번 측정의 결론은 다음과 같다.

```text
ACEnemy actor tick 제거는 제거 가능한 polling tick 검증으로는 의미가 있다.
하지만 현재 Runtime LOD 성능 병목의 본체는 아니다.
후속 분석은 Movement와 Animation을 독립적으로 통제하는 환경에서 진행해야 한다.
```

## 후속 작업 계획

### 1. Movement Isolation

목적:

```text
CharacterMovement / movement intent / path following이 Frame, GameThread, TickActors에 주는 영향을 분리한다.
```

측정 방향:

- animation 조건은 baseline으로 고정한다.
- `EnemyMovementMode` 또는 별도 movement test map을 사용한다.
- 이동 유지 / 이동 의도 차단 / path following 유지 여부를 분리한다.
- 핵심 지표는 `CharacterMovement p95`, `TickActors p95`, `Frame/Game p95`다.

주의:

```text
MovementComponent tick off는 이전 측정에서 representation 손상을 만들었다.
따라서 단순 tick off보다 active movement 후보 수, movement intent, path following 정책을 중심으로 본다.
```

### 2. Animation Isolation

목적:

```text
Animation / pose update / locomotion visual detail이 Frame, GameThread, TickActors에 주는 영향을 분리한다.
```

측정 방향:

- movement를 고정하거나 최소화한 환경을 만든다.
- mesh visible은 유지한다.
- animation parameter refresh, pose update, locomotion detail을 분리한다.
- 핵심 지표는 `Animation p95`, `SkeletalMeshComponent tick`, `Frame/Game p95`다.

주의:

```text
이전 EnemyAnimationMode 1은 parameter refresh 빈도를 줄인 실험이다.
그 결과만으로 animation graph / pose update / locomotion 전체 비용이 작다고 해석하지 않는다.
```

### 3. Combined Stress

Movement와 Animation을 각각 분리한 뒤, 실제 gameplay stress 조건에서 다시 확인한다.

목적:

```text
독립 측정에서 확인한 비용 축이 실제 Engage / Alert / Observe 조건에서도 의미 있는지 검증한다.
```

## 현재 판단

Component Tick 축은 Runtime LOD v1의 1차 최적화 후보에서 낮은 우선순위로 둔다.

다음 우선순위는 다음과 같다.

```text
1. Movement / Nav active work 분리
2. Animation / Pose / Locomotion detail 분리
3. 두 축을 결합한 gameplay stress 재측정
```
