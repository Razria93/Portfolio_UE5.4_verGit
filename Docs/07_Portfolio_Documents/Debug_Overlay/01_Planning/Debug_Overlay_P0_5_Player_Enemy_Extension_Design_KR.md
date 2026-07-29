# Debug Overlay P0.5 Player/Enemy 확장 설계

## 1. 목적

P0.5 확장은 P0 debug overlay가 TestRoom PIE에서 정상 동작하는 것을 전제로 한다. 목적은 제출 영상 및 기술문서 evidence의 가독성과 설명력을 높이는 것이다.

이 확장은 완성형 HUD가 아니다. 개발 전용 debug overlay이며, Shipping HUD처럼 보이거나 제품 UI처럼 설명하지 않는다.

## 2. 확장 범위 요약

| 구분 | 결정 |
| --- | --- |
| Overlay 구조 | Player / Enemy 2개 패널로 분리 |
| Player tab 색상 | Blue |
| Enemy tab 색상 | Red |
| 본문 배경 | 기존 반투명 black 유지 |
| 표시 방식 | Canvas Draw 유지 |
| 데이터 조회 | 가능한 항목은 getter polling 우선 |
| EventLog | Player / Enemy 분리 설계 필요 |
| Enemy 선택 | combat 연관 actor 우선, world scan은 fallback |

## 3. 패널 구조

P0.5 overlay는 Player와 Enemy를 같은 section 순서로 표시한다. 구조는 대칭으로 유지하고, 대상별로 의미가 없거나 아직 조회할 수 없는 값은 `N/A`, `None`, `NotCaptured`로 표시한다.

```text
[Debug Overlay P0.5 - Player]
State:
Action:
Reaction:
Guard:
Movement:
HP:
Runtime LOD:
AI:
Recent Execution:
Recent Combat:
Event Log:

[Debug Overlay P0.5 - Enemy]
State:
Action:
Reaction:
Guard:
Movement:
HP:
Runtime LOD:
AI:
Recent Execution:
Recent AI:
Recent Combat:
Event Log:
```

Player tab은 blue 계열, Enemy tab은 red 계열의 작은 header block으로 표시한다. Header 색상은 대상 구분용이며, gameplay 상태나 위험도 의미를 부여하지 않는다.

본문은 기존 반투명 black background를 유지한다. 제출 캡처에서 읽히는 것이 목적이므로 장식적 UI 요소는 추가하지 않는다.

대칭 구조를 유지하는 이유:

- 캡처 이미지를 보는 사람이 Player/Enemy 정보를 같은 순서로 비교할 수 있다.
- 특정 항목이 없을 때도 누락인지 미수집인지 구분할 수 있다.
- 후속 구현에서 section 추가/삭제로 인한 layout 흔들림을 줄일 수 있다.

대상별 예외:

- Player의 `Runtime LOD`와 `AI`는 P0.5에서 기본적으로 `N/A` 또는 `NotCaptured`가 가능하다.
- Enemy의 `Guard`는 component가 없거나 의미가 낮으면 `N/A`가 가능하다.
- Enemy의 `Recent Execution`은 enemy action/reaction hook 귀속이 확정되기 전까지 `NotCaptured`가 가능하다.

## 4. Player 표시 항목

| 항목 | 표시값 | 데이터 소스 | 상태 |
| --- | --- | --- | --- |
| ExecutionState | 현재 execution state | `UCStateComponent::GetCurrentExecutionState` | Ready |
| ActiveAction | 현재 active action 또는 `None` | `UCActionComponent` getter | Ready |
| ActiveReaction | 현재 active reaction 또는 `None` | `UCReactionComponent` getter | Ready |
| GuardOverlay | guard intent/pose/capability | `UCDefenseComponent` getter | Ready |
| Movement.Gait | 현재 movement gait | `UCMovementComponent::GetCurrentMovementGait` | Ready |
| Movement.Speed | 현재 2D speed | `UCMovementComponent::GetCurrentSpeed` | Ready |
| Movement.Direction | 현재 movement direction | `UCMovementComponent::GetCurrentDirection` | Ready |
| Movement.CanMove | 이동 입력 수락 가능 여부 | `UCMovementComponent::CanMove` 또는 `CanAcceptMoveInput` | Ready |
| Movement.IsFalling | 낙하 상태 | `UCMovementComponent::IsFalling` | Ready |
| HP.CurrentHP | 현재 HP | `UCHealthComponent::GetCurrentHP` | Ready |
| HP.MaxHP | 최대 HP | `UCHealthComponent::GetMaxHP` | Ready |
| HP.DeadState | 생존/사망 상태 | `UCHealthComponent::GetDeadState` | Ready |
| RuntimeLODTier | Player 기준 runtime LOD | 현재 P0.5에서는 기본 `N/A` | ReviewNeeded |
| AI | Player 기준 AI summary | Player에는 일반적으로 없음 | ReviewNeeded |
| Recent Execution | Player 기준 최근 execution summary | Store snapshot 또는 subject 분리 Store | Ready/HookNeeded |
| Recent Combat | Player 기준 최근 combat summary | Store snapshot 또는 subject 분리 Store | Ready/HookNeeded |
| EventLog | Player 기준 최근 event | Subject 분리 Store | HookNeeded |

Player actor는 HUD의 `GetOwningPawn()`을 기준으로 한다. Component를 찾지 못하면 기존 P0 정책처럼 `N/A`, 아직 event가 없으면 `NotCaptured`, active 상태가 없으면 `None`을 사용한다.

## 5. Enemy 표시 항목

| 항목 | 표시값 | 데이터 소스 | 상태 |
| --- | --- | --- | --- |
| ExecutionState | enemy 현재 execution state | `UCStateComponent::GetCurrentExecutionState` | Ready |
| ActiveAction | enemy 현재 active action 또는 `None` | `UCActionComponent` getter | Ready |
| ActiveReaction | enemy 현재 active reaction 또는 `None` | `UCReactionComponent` getter | Ready |
| GuardOverlay | enemy guard intent/pose/capability | `UCDefenseComponent` getter 또는 `N/A` | ReviewNeeded |
| Movement.Gait | enemy movement gait | `UCMovementComponent::GetCurrentMovementGait` | Ready |
| Movement.Speed | enemy 2D speed | `UCMovementComponent::GetCurrentSpeed` | Ready |
| Movement.Direction | enemy movement direction | `UCMovementComponent::GetCurrentDirection` | Ready |
| Movement.CanMove | enemy 이동 입력 수락 가능 여부 | `UCMovementComponent::CanMove` 또는 `CanAcceptMoveInput` | Ready |
| Movement.IsFalling | enemy 낙하 상태 | `UCMovementComponent::IsFalling` | Ready |
| HP.CurrentHP | enemy 현재 HP | `UCHealthComponent::GetCurrentHP` | Ready |
| HP.MaxHP | enemy 최대 HP | `UCHealthComponent::GetMaxHP` | Ready |
| HP.DeadState | enemy 생존/사망 상태 | `UCHealthComponent::GetDeadState` | Ready |
| RuntimeLODTier | enemy runtime LOD tier | 기존 Runtime LOD source 또는 AI summary | ReviewNeeded |
| AI | enemy AI 상태/summary | `FDebugOverlaySnapshotStore` AI summary | Ready |
| Recent Execution | enemy 기준 최근 execution summary | Store subject 분리 이후 안정화 | HookNeeded |
| Recent AI | 최근 AI combat task summary | `FDebugOverlaySnapshotStore` AI summary | Ready |
| Recent Combat | enemy 기준 최근 combat summary | Store snapshot 또는 subject 분리 Store | Ready/HookNeeded |
| EventLog | Enemy 기준 최근 event | Subject 분리 Store | HookNeeded |

Enemy 패널은 선택된 enemy가 없으면 header는 유지하되 주요 값은 `N/A` 또는 `NotCaptured`로 표시한다. 이 상태를 enemy system 성공 evidence로 사용하지 않는다.

## 6. 실제 코드 근거

| 근거 | 의미 |
| --- | --- |
| `ACPlayer::GetMovementComp` | Player movement component 접근 가능 |
| `ACPlayer::GetHealthComp` | Player health component 접근 가능 |
| `ACEnemy::GetMovementComp` | Enemy movement component 접근 가능 |
| `ACEnemy::GetHealthComp` | Enemy health component 접근 가능 |
| `UCMovementComponent::GetCurrentMovementGait` | movement gait 조회 가능 |
| `UCMovementComponent::GetCurrentSpeed` | speed 조회 가능 |
| `UCMovementComponent::GetCurrentDirection` | direction 조회 가능 |
| `UCMovementComponent::CanMove` | move 가능 상태 조회 가능 |
| `UCMovementComponent::IsFalling` | falling 상태 조회 가능 |
| `UCHealthComponent::GetCurrentHP` | 현재 HP 조회 가능 |
| `UCHealthComponent::GetMaxHP` | 최대 HP 조회 가능 |
| `UCHealthComponent::GetDeadState` | 생존/사망 상태 조회 가능 |

`UCMovementComponent`의 runtime LOD movement 내부 상태, movement override cache, AI intent state는 현재 공개 getter만으로 충분히 읽기 어렵다. 해당 항목은 P1 getter 또는 hook 대상으로 둔다.

## 7. Enemy 탐색 정책

Enemy 탐색은 debug evidence의 신뢰도를 위해 단순 world scan을 1순위로 사용하지 않는다.

우선순위:

| 순위 | 방식 | 설명 |
| --- | --- | --- |
| 1 | 최근 combat snapshot 기반 추론 | Player가 source이면 target/receiver를 enemy 후보로, Player가 target이면 source를 enemy 후보로 본다. |
| 2 | AIController / Blackboard target actor | 기존 AI runtime truth를 사용할 수 있는지 검토한다. |
| 3 | cached `ACEnemy` world scan fallback | 명확한 combat/AI 대상이 없을 때만 사용한다. |

정책:

- 매 `DrawHUD()`마다 전체 enemy scan을 수행하지 않는다.
- HUD 또는 helper 내부에 `TWeakObjectPtr<AActor>` 기반 cached enemy를 둔다.
- 캐시 갱신 interval은 0.25~0.5초 후보로 둔다.
- cached enemy가 invalid, pending kill, world mismatch, 사망/제외 조건에 걸리면 재탐색한다.
- 다중 enemy 환경에서 “처음 발견된 enemy”를 evidence 대상처럼 표시하지 않는다.
- fallback scan으로 선택한 enemy는 표시상 `EnemySource=WorldScanFallback` 같은 보조 정보를 남기는 것을 검토한다.

## 8. EventLog 분리 정책

현재 P0 Store는 `World` 단위 단일 ring buffer다. Player/Enemy EventLog를 안정적으로 분리하려면 Store key 또는 event entry를 확장해야 한다.

후보 A: `World + SubjectActor`

```text
FDebugOverlaySubjectKey
  TObjectKey<UWorld> WorldKey
  TObjectKey<AActor> SubjectActorKey
```

장점:

- 실제 actor 기준으로 Player/Enemy log를 분리할 수 있다.
- 다중 enemy 확장에 유리하다.
- combat source/target/receiver 기준 event 귀속이 명확해진다.

주의:

- actor raw pointer 장기 보관은 금지한다.
- key에는 `TObjectKey` 또는 weak reference 성격의 식별자를 사용한다.
- actor destroy/world transition 시 cleanup/reset 정책이 필요하다.

후보 B: `World + SubjectRole`

```text
World
  PlayerEvents
  EnemyEvents
```

장점:

- 구현이 단순하다.
- P0.5의 2패널 목적에는 충분할 수 있다.

주의:

- 다중 enemy 환경에서 어느 enemy의 event인지 손실된다.
- 특정 enemy evidence로 설명하기 어렵다.

권장:

P0.5에서는 구현 난이도와 evidence 신뢰도의 균형을 위해 `World + SubjectActor` 설계를 우선 검토한다. 단, 첫 구현에서는 `SubjectRole` 임시 분리를 허용할지 사용자 결정이 필요하다.

`FDebugOverlayEventEntry` 확장 후보:

| 필드 | 목적 |
| --- | --- |
| `SubjectName` | HUD에 표시할 대상 이름 |
| `SubjectRole` | Player / Enemy / World / Unknown 구분 |
| `SubjectSource` | CombatSnapshot / Blackboard / WorldScanFallback 등 선택 근거 |

`OwnerName`, `SourceName`, `TargetName`만으로는 Player/Enemy log를 안정적으로 분리하지 않는다. Combat result에서는 receiver/source/target 중 어느 actor를 subject로 볼지 문맥에 따라 달라지기 때문이다.

## 9. EventLog category filter 설계

EventLog filter는 문자열 비교보다 enum/bitmask 기반을 권장한다.

카테고리 후보:

| Category | 포함 event |
| --- | --- |
| Execution | Action / Reaction decision |
| Combat | HitWindow, target accepted/rejected |
| CombatResult | result dispatch/receive |
| AI | AI combat task, AI decision |
| Movement | movement input/gait/runtime LOD movement |
| Health | HP change, dead/alive transition |

CVar 후보:

```text
Portfolio.DebugOverlay.EventCategories
```

Preset 후보:

| Preset | 목적 | Event category |
| --- | --- | --- |
| `0 Compact` | 제출 대표 캡처 | Execution, Combat |
| `1 Combat` | combat 흐름 evidence | Combat, CombatResult, Health |
| `2 FullTrace` | 개발 검증 | Execution, Combat, CombatResult, AI, Movement, Health |

P0.5에서 category filter를 바로 구현할지, P1로 미룰지는 결정이 필요하다. Player/Enemy EventLog 분리보다 먼저 넣으면 filter 기준은 생기지만 subject 분리 문제가 남는다.

## 10. Compact summary 정책

제출 캡처에서는 긴 enum prefix가 가독성을 해친다. P0.5 또는 후속 작업에서 compact summary를 적용한다.

예시:

```text
Before:
Execution/DecisionResolved: Action EExecutionDecision::Accept Apply=EExecutionApplyMode::Start Reject=EActionRequestRejectReason::None

After:
Execution: Action Accept / Apply=Start / Reject=None
```

정책:

- 내부 enum 값을 숨기지 않고 사람이 읽기 좋은 suffix 중심으로 표시한다.
- 원본 audit log 의미를 바꾸지 않는다.
- compact summary는 overlay 표시 전용으로 둔다.

## 11. 구현 단계 제안

| 단계 | 목표 | 변경 성격 |
| --- | --- | --- |
| 1 | HUD Player/Enemy 패널 분리 + Movement/HP getter polling | HUD 중심 |
| 2 | Enemy cached selection helper 구현 | HUD/helper 중심 |
| 3 | Store subject 분리 설계 확정 | Store 구조 변경 전 의사결정 |
| 4 | EventLog category filter 설계 확정 | Store/CVar 변경 전 의사결정 |
| 5 | compact summary format 설계 | 표시 품질 개선 후보 |

1단계는 Store 구조 변경 없이도 가능하다. 다만 Player/Enemy EventLog 분리는 Store subject 정책이 확정된 뒤 별도 구현 작업으로 진행한다.

## 12. P0 / P0.5 / P1 범위 분리

### P0 유지

- Player current state/action/reaction/guard
- 최근 Execution summary
- 최근 Combat summary
- 최근 AI summary
- World 단위 EventLog 3~5 lines
- 개발 전용 gate와 Shipping no-op 정책

### P0.5 확장

- Player/Enemy 2패널 표시
- Player/Enemy Movement 표시
- Player/Enemy HP 표시
- Enemy cached selection
- Player/Enemy EventLog 분리 설계 확정
- EventLog compact 표시 설계

### P1 후보

- 다중 Enemy cycling UI
- Blackboard 상세 dump
- Perception candidate list
- AIIntentState getter/hook
- RuntimeLOD movement 내부 상태 getter/hook
- Event category filter preset UI
- capture automation
- Store subject 분리 구현
- Event category filter 구현

## 13. 구현 전 결정 필요 항목

다음 항목은 구현 전에 사용자 결정을 받는다.

| 결정 항목 | 선택지 | 권장 |
| --- | --- | --- |
| Enemy 선택 우선순위 | Combat snapshot / Blackboard / World scan | Combat snapshot 우선 |
| Enemy fallback scan 주기 | 0.25초 / 0.5초 / 수동 갱신 | 0.5초 |
| Store subject key | `SubjectActor` / `SubjectRole` | `SubjectActor` |
| Event category filter 시점 | P0.5 포함 / P1 이관 | P1 또는 Store 분리 이후 |
| compact summary 시점 | HUD 패널 분리 전 / 후 | HUD 패널 분리 후 |
| EventLog 분리 시점 | HUD 1단계와 동시 / Store 3단계 | Store 3단계 |

## 14. 위험 요소

| 위험 | 대응 |
| --- | --- |
| 다중 enemy에서 잘못된 대상 표시 | Enemy 선택 근거를 명시하고 world scan은 fallback으로 제한 |
| DrawHUD 비용 증가 | enemy scan interval과 weak cache 사용 |
| Store 구조 변경으로 P0 regression 발생 | P0 world snapshot copy API 호환 유지 |
| EventLog subject 오분류 | `SubjectActor` 또는 `SubjectRole`을 명시적으로 추가 |
| Shipping HUD처럼 보임 | 색상은 tab 구분만 사용하고 장식/제품 UI 표현 금지 |
| 성능 evidence로 오해 | 문서와 caption에서 debug evidence임을 명시 |

## 15. 최종 제안

P0.5는 다음 순서로 진행한다.

1. HUD에 Player/Enemy 패널을 만들고 Movement/HP getter polling을 표시한다.
2. Enemy 선택은 combat snapshot 기반 추론을 우선 검토하고, 구현 가능성이 낮을 때 Blackboard target actor를 검토한다.
3. 두 우선 후보가 확정되지 않은 경우에만 cached world scan fallback을 사용하며, 표시상 fallback임을 드러낸다.
4. Store subject 분리 정책을 별도 작업에서 확정한 뒤 Player/Enemy EventLog 분리 구현 여부를 결정한다.
5. compact summary와 category filter는 Store subject 분리 이후 후속 후보로 둔다.

이 순서는 현재 동작 중인 P0 overlay를 크게 흔들지 않으면서, 캡처 evidence 품질을 단계적으로 높일 수 있다.
