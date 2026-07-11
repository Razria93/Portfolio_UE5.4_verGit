# AI Component Tick Audit Plan

## 목적

`Component Tick` 축이 AI Runtime LOD v1의 실제 최적화 후보인지 확인한다.

이번 측정은 Unreal Engine 기본 tick 비용 전체를 보는 것이 아니라, 프로젝트가 직접 정의한 Enemy-side actor / component tick 중 Runtime LOD로 줄일 수 있는 부분을 분리한다.

## 배경

이전 측정에서 다음 축은 이미 별도로 다뤘다.

- `CharacterMovementComponent`
- `BehaviorTreeComponent`
- `SkeletalMeshComponent`
- AnimInstance parameter refresh
- Movement / Nav intent blocking

따라서 이번 문서에서 말하는 `Component Tick`은 위 엔진 컴포넌트나 BT service tick이 아니라, 프로젝트 코드가 직접 가진 tick entry를 의미한다.

## 현재 Tick 후보

코드 스캔 기준 직접 tick을 가진 주요 대상은 다음과 같다.

| 대상 | Tick entry | 현재 역할 | 이번 축 포함 여부 |
| --- | --- | --- | --- |
| `ACEnemy` | `ACEnemy::Tick` | Runtime LOD mesh / movement mode 갱신 | 포함 |
| `UCMovementComponent` | `UCMovementComponent::TickComponent` | speed / direction / falling 등 movement parameter refresh | 포함 |
| `UCActionComponent` | `UCActionComponent::TickComponent` | active action executor tick 전달 | 포함 |
| `CharacterMovementComponent` | engine tick | 실제 movement simulation / path following 영향 | 제외, Movement / Nav 축에서 해석 |
| `SkeletalMeshComponent` | engine tick | pose / animation update | 제외, Mesh / Animation 축에서 해석 |
| `BehaviorTreeComponent` | engine tick / service scheduling | AI decision update | 제외, BT Update Interval 축에서 해석 |

## 측정 질문

1. `ACEnemy::Tick`은 Runtime LOD CVar 감시 외에 지속 tick이 필요한가?
2. `UCMovementComponent::TickComponent`를 줄이면 비용 이득보다 locomotion 표현 깨짐이 더 큰가?
3. `UCActionComponent::TickComponent`는 combat action이 active인 동안 의미 있는 비용을 갖는가?
4. tick off가 montage / notify / hit window / combat result를 깨는가?
5. 이 축이 실제 Runtime LOD 후보인지, 아니면 측정 결과만 남기고 닫을 축인지 판단한다.

## 제어안

측정 CVar 후보:

```text
Portfolio.AI.RuntimeLOD.EnemyComponentTickMode
```

| Mode | 이름 | 제어 내용 | 의도 |
| ---: | --- | --- | --- |
| 0 | Default | 기본 tick 유지 | 기준값 |
| 1 | DisableEnemyActorTick | `ACEnemy::Tick` 비활성화 | actor-level tick 비용 분리 |
| 2 | DisableMovementComponentTick | `UCMovementComponent::TickComponent` 비활성화 | movement parameter refresh 비용 분리 |
| 3 | DisableActionComponentTick | `UCActionComponent::TickComponent` 비활성화 | active action executor tick 비용 분리 |

현재 구현 범위:

```text
Mode 1: DisableEnemyActorTick
```

`ACEnemy::Tick`은 현재 Runtime LOD mesh / movement / component tick mode polling을 수행한다.
따라서 Mode 1은 gameplay action 자체가 아니라 Enemy actor-level polling tick 비용을 분리하는 측정이다.

주의:

- `Mode 2`는 이전 Movement / Nav 측정의 `MovementComponent tick off`와 겹친다.
- 따라서 새 결과가 기존 결론을 바꾸지 않는다면, `Mode 2`는 재측정보다 기존 결과 인용으로 닫아도 된다.
- `Mode 3`은 combat action lifecycle을 깨뜨릴 수 있으므로, 기능 smoke가 우선이다.
- Mode 1은 actor tick 자체를 끄므로 PIE 중 CVar를 1에서 0으로 되돌리는 runtime restore 검증에는 적합하지 않다.
- 정규 측정은 PIE 실행 전 CVar 설정을 기준으로 한다.

## 구현 범위

1. `ACEnemy`에서 Enemy 전용 component tick profiling gate를 관리한다.
2. 원복을 위해 각 대상의 original tick enabled state를 cache한다.
3. Player actor / player component에는 적용하지 않는다.
4. Runtime 중 CVar 변경 검증을 허용하되, 정규 측정은 PIE 시작 전 CVar 설정을 기준으로 한다.
5. CSV counter는 필요할 때만 추가한다.

카운터 후보:

```text
PortfolioAI_EnemyActorTick_Count
PortfolioAI_MovementComponentTick_Count
PortfolioAI_ActionComponentTick_Count
```

단, tick마다 CSV stat을 직접 찍으면 계측 노이즈가 생길 수 있으므로 필요하면 누적 후 subsystem tick에서 flush하는 방식을 우선한다.

## 측정 조건

공통 조건:

- Capture Duration: 약 36초
- Analysis Window: first 3s / last 3s trimmed, middle 30s used
- Log State: `-noailogging`
- PIE: F11 fullscreen
- fixed camera
- GC 이벤트 없음

CVar:

```text
Portfolio.AI.RuntimeLOD.EnemyComponentTickMode 0 / 1 / 2 / 3

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
- Observe 유지
- attack montage 정상
- hit / guard / parry result 정상
- feedback presentation 정상
- movement / locomotion 표현 깨짐 여부 기록

## 40 Enemy 측정

| Case | Enemy Count | Mode | Capture ID | 비고 |
| --- | ---: | ---: | --- | --- |
| CT01 | 40 | 0 | TBD | Default |
| CT02 | 40 | 1 | TBD | DisableEnemyActorTick |
| CT03 | 40 | 2 | TBD | DisableMovementComponentTick |
| CT04 | 40 | 3 | TBD | DisableActionComponentTick |

## 80 Enemy 측정

40 Enemy에서 기능 smoke가 깨지지 않는 mode만 80 Enemy로 확장한다.

| Case | Enemy Count | Mode | Capture ID | 비고 |
| --- | ---: | ---: | --- | --- |
| CT05 | 80 | 0 | TBD | Default |
| CT06 | 80 | 1 | TBD | DisableEnemyActorTick |
| CT07 | 80 | 2 | TBD | DisableMovementComponentTick |
| CT08 | 80 | 3 | TBD | DisableActionComponentTick |

## 분석 기준

주요 지표:

- `Frame p95`
- `Game p95`
- `Ticks/CEnemy`
- `Ticks/CMovementComponent`
- `Ticks/CActionComponent`
- `Ticks/CharacterMovementComponent`
- `Ticks/BehaviorTreeComponent`
- `Ticks/SkeletalMeshComponent`

해석 기준:

- 대상 tick p95 또는 count가 줄어도 Frame / Game p95가 오차 범위면 핵심 병목으로 보지 않는다.
- 비용 개선보다 gameplay / representation 깨짐이 크면 Runtime LOD 후보에서 제외한다.
- `MovementComponent` tick off가 locomotion 표현을 깨뜨리는 경우, Movement / Nav 기존 결론을 우선한다.
- `ActionComponent` tick off가 combat action lifecycle을 깨뜨리면 combat-capable LOD에서는 금지한다.

## 예상 결론 후보

1. `ACEnemy::Tick` 비용이 작다면 Runtime LOD CVar polling을 더 복잡하게 최적화하지 않는다.
2. `UCMovementComponent` tick off는 비용보다 locomotion 표현 손상이 크면 제외한다.
3. `UCActionComponent` tick off는 combat action 유지 조건과 충돌하면 제외한다.
4. 유의미한 tick 후보가 없으면 Component Tick 축은 측정 결과만 남기고 다음 축으로 넘긴다.

## 후속 작업

이 축이 닫히면 다음 후보는 `Perception Active Budget / Cap`이다.

Component Tick이 큰 병목이 아니라면 Runtime LOD v1은 이미 효과가 확인된 다음 정책을 중심으로 구성한다.

- CombatEngage assignment cap
- Observe / Alert / Engage 계층화
- BT service update precision split
- Mesh / Animation representation overlay
