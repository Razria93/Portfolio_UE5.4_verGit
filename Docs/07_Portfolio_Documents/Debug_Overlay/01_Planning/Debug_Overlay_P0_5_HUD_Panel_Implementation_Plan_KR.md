# Debug Overlay P0.5 HUD Panel 구현 계획

## 1. 목적

이 문서는 P0.5 구현 1단계의 범위를 고정한다. 목표는 `FDebugOverlaySnapshotStore` 구조를 변경하지 않고, `ACDebugOverlayHUD` 표시만 확장해 Player/Enemy 대칭 패널과 Movement/HP getter polling 값을 표시하는 것이다.

이번 단계에서 구현하지 않는 것:

- Player/Enemy EventLog subject 분리
- Store/SnapshotTypes/SnapshotStore 구조 변경
- 새로운 CVar 추가
- GameMode/World Settings 변경
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 2. 구현 대상 파일

| 파일 | 변경 목적 |
| --- | --- |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.h` | enemy cache 필드가 필요한 경우에만 최소 변경 |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp` | Player/Enemy panel line 구성, Movement/HP 표시, Canvas draw 확장 |

Store 관련 파일은 이번 단계에서 변경하지 않는다.

변경 금지:

- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp`
- `Source/Portfolio/Portfolio.Build.cs`
- `Config/*`
- `Content/*.umap`
- `Content/*.uasset`

## 3. 레이아웃 결정

### 3.1 기본 레이아웃

1단계는 stacked layout을 기본으로 한다.

```text
[Debug Overlay P0.5]

[Player]  blue tab
State:
Action:
Reaction:
Guard:
Movement:
HP:
Runtime LOD:
AI:

[Enemy]  red tab
State:
Action:
Reaction:
Guard:
Movement:
HP:
Runtime LOD:
AI:

[Recent Execution]
[Recent Combat]
[Recent AI]
[Event Log]
```

좌우 column layout은 이번 단계에서 사용하지 않는다. 현재 EventLog와 GuardOverlay 문자열이 길고 Canvas text wrap이 자동 처리되지 않으므로, 좌우 column은 작은 viewport나 캡처 화면에서 clipping/겹침 리스크가 크다.

### 3.2 기존 P0 recent block 유지

기존 `AddSnapshotLines()` 출력은 독립 block으로 보존한다.

보존 이유:

- `Recent Execution`, `Recent Combat`, `Recent AI`, `Event Log`의 현재 캡처 상태 표현을 유지한다.
- `ValueOrNotCaptured`, `HasFinalTakenDamageEvidence`, `DamageCommit` fallback 로직 regression을 피한다.
- Store가 아직 world 단위 snapshot이므로 Player/Enemy panel 내부로 억지 분리하지 않는다.

## 4. 공통 section 순서

Player/Enemy 패널은 같은 section 순서를 유지한다.

| 순서 | Section | Player | Enemy |
| --- | --- | --- | --- |
| 1 | State | 표시 | 표시 |
| 2 | Action | 표시 | 표시 |
| 3 | Reaction | 표시 | 표시 |
| 4 | Guard | 표시 | 가능 시 표시, 아니면 `N/A` |
| 5 | Movement | 표시 | 표시 |
| 6 | HP | 표시 | 표시 |
| 7 | Runtime LOD | 기본 `N/A` | 가능 시 표시, 아니면 `N/A` |
| 8 | AI | 기본 `NotCaptured` 또는 `N/A` | 가능 시 표시 |

대칭 layout을 유지하되, 의미가 없거나 조회할 수 없는 값은 `N/A`, 아직 event가 없으면 `NotCaptured`, active 상태가 없으면 `None`을 사용한다.

## 5. Player 표시 정책

Player actor는 `ACDebugOverlayHUD::GetOwningPawn()`을 기준으로 한다.

| 항목 | 조회 방식 | 실패 표시 |
| --- | --- | --- |
| State | `UCStateComponent::GetCurrentExecutionState` | `N/A` |
| Action | `UCActionComponent` active getter | `N/A` / `None` |
| Reaction | `UCReactionComponent` active getter | `N/A` / `None` |
| Guard | `UCDefenseComponent` getter | `N/A` |
| Movement.Gait | `UCMovementComponent::GetCurrentMovementGait` | `N/A` |
| Movement.Speed | `UCMovementComponent::GetCurrentSpeed` | `N/A` |
| Movement.Direction | `UCMovementComponent::GetCurrentDirection` | `N/A` |
| Movement.CanMove | `UCMovementComponent::CanMove` | `N/A` |
| Movement.IsFalling | `UCMovementComponent::IsFalling` | `N/A` |
| HP.CurrentHP | `UCHealthComponent::GetCurrentHP` | `N/A` |
| HP.MaxHP | `UCHealthComponent::GetMaxHP` | `N/A` |
| HP.DeadState | `UCHealthComponent::GetDeadState` | `N/A` |
| Runtime LOD | Player 기준 소스 없음 | `N/A` |
| AI | Player 기준 소스 없음 | `NotCaptured` 또는 `N/A` |

Movement 값은 `UCMovementComponent` tick 또는 Runtime LOD 상태에 따라 마지막 갱신값일 수 있다. 따라서 speed/direction은 “현재 프레임의 물리 truth”가 아니라 movement component가 제공하는 debug state로 설명한다.

## 6. Enemy 표시 정책

이번 단계에서는 Store subject 분리 없이 enemy actor를 찾아 getter polling만 수행한다.

Enemy 선택은 임시 fallback이며, evidence 신뢰도 문구를 명확히 둔다. Fallback으로 선택된 enemy의 Movement/HP는 “선택된 enemy 상태”이지, combat target 성공 evidence가 아니다.

### 6.1 1단계 enemy 선택 후보

우선 구현 후보:

- cached `ACEnemy` world scan fallback
- `TWeakObjectPtr<ACEnemy>` cache
- 0.5초 scan cooldown
- 미발견/다중 enemy 상태 표시

Combat snapshot 기반 enemy 추론은 더 신뢰도가 높지만, Store/API 보강이 필요하면 후속 단계로 둔다.

### 6.2 scan 정책

`DrawHUD()` 매 프레임마다 전체 world scan을 수행하지 않는다.

최소 정책:

1. `CachedEnemy`가 valid면 재사용한다.
2. `CachedEnemy`가 invalid이면 cooldown을 확인한다.
3. cooldown이 지났을 때만 `TActorIterator<ACEnemy>` fallback scan을 수행한다.
4. enemy가 0개면 `EnemyFallback: NotCaptured(NoEnemy)`로 표시한다.
5. enemy가 2개 이상이면 `EnemyFallback: Ambiguous(Count=N)`로 표시하고, HP/Movement 값을 성공 evidence처럼 쓰지 않는다.

선택 정책을 반드시 표시한다.

예시:

```text
EnemySource: WorldScanFallback
EnemyFallback: Selected=BP_CAIController0_Pawn Policy=FirstValid Count=1
```

다중 enemy 상황에서 임의 첫 enemy를 대표 evidence로 과장하지 않는다.

## 7. 기존 Store snapshot 사용 범위

이번 단계에서는 기존 API를 유지한다.

```cpp
FDebugOverlaySnapshotStore::GetSnapshotCopy(GetWorld(), Snapshot)
```

사용 정책:

- P0 world 단위 `Recent Execution`, `Recent Combat`, `Recent AI`, `Event Log`는 공통 recent evidence로 유지한다.
- Player/Enemy panel 내부에 별도 EventLog를 넣지 않는다.
- Player/Enemy EventLog 분리는 Store subject 분리 이후 별도 단계로 진행한다.
- 기존 event log line 수와 `Portfolio.DebugOverlay.EventLogLimit` 정책을 유지한다.

## 8. Helper 구조 후보

`CDebugOverlayHUD.cpp` anonymous namespace 내부 helper를 확장한다.

권장 helper:

| Helper | 역할 |
| --- | --- |
| `FormatActorState` | actor/pawn state line 생성 |
| `FormatActorAction` | active action line 생성 |
| `FormatActorReaction` | active reaction line 생성 |
| `FormatActorGuard` | guard line 생성 |
| `FormatActorMovement` | movement line 생성 |
| `FormatActorHealth` | HP line 생성 |
| `AddPanelHeader` | colored tab/header line 추가 |
| `AddActorPanelLines` | Player/Enemy section line 구성 |
| `DrawOverlayBackground` | 전체 반투명 black background draw |
| `DrawPanelHeader` | Player/Enemy tab color draw |
| `RefreshCachedEnemyIfNeeded` | enemy cache 갱신 |
| `ResolveDisplayEnemy` | 표시할 enemy 반환 |

원칙:

- SnapshotStore 호출은 `DrawHUD()`에서 1회만 유지한다.
- data formatting helper와 Canvas draw helper를 분리한다.
- 기존 `AddSnapshotLines()`는 recent block 전용으로 보존한다.

## 9. Include 후보

`.cpp` include 후보:

```cpp
#include "Character/Enemy/CEnemy.h"
#include "Component/CMovementComponent.h"
#include "Component/CHealthComponent.h"
#include "EngineUtils.h"
```

`.h` forward declaration 후보:

```cpp
class ACEnemy;
```

`CDebugOverlayHUD.h` 캐시 필드 후보:

```cpp
#if !UE_BUILD_SHIPPING
TWeakObjectPtr<ACEnemy> CachedEnemy;
float LastEnemyScanTimeSeconds = -1.f;
int32 LastEnemyScanCount = 0;
#endif
```

Build.cs 변경은 필요하지 않은 것으로 본다. `TActorIterator`, `AHUD`, `Canvas`, actor/component 조회는 기존 Engine 의존성 범위에 포함된다.

## 10. Canvas draw 정책

기존 전체 background는 유지한다.

추가 draw 정책:

- Player header는 blue 계열 `DrawRect`.
- Enemy header는 red 계열 `DrawRect`.
- header rect는 text보다 먼저 그린다.
- 전체 background rect를 먼저 그리고, header rect를 그린 뒤 text를 그린다.
- header alpha는 0.55~0.75 범위 후보로 둔다.
- header height는 `DebugOverlayLineHeight`와 y 계산이 어긋나지 않게 한다.
- Canvas width/height guard를 둔다.

1단계에서는 font scale, line height, 전체 background width를 추가 조정하지 않는다. 필요하면 PIE 캡처 확인 후 별도 styling 작업으로 분리한다.

## 11. Shipping / build 정책

기존 정책을 유지한다.

- `#if !UE_BUILD_SHIPPING` 보호 유지
- Shipping no-op 유지
- UMG/Slate dependency 추가 금지
- Build.cs 변경 금지
- GameMode/World Settings 변경 금지
- Store/SnapshotTypes/SnapshotStore 구조 변경 금지
- CSV profiler counter 추가 금지
- 새로운 CVar 추가 금지

## 12. 검증 계획

구현 단계에서 다음을 확인한다.

| 검증 | 기준 |
| --- | --- |
| `git diff --check` | whitespace 문제 없음 |
| UE build | `PortfolioEditor Win64 Development` 통과 |
| P0 recent block | 기존 `Recent Execution/Combat/AI/Event Log` 유지 |
| Player panel | Movement/HP 표시 |
| Enemy panel | enemy 발견 시 Movement/HP 표시 |
| Enemy 없음 | `N/A` 또는 `NotCaptured(NoEnemy)` 표시 |
| 다중 enemy | `Ambiguous(Count=N)` 표시 또는 evidence 제한 |
| Shipping | overlay 표시/수집 no-op 유지 |

PIE에서 확인할 항목:

- Player `Speed`, `Direction`, `Gait`, `CanMove`, `IsFalling`
- Player `CurrentHP`, `MaxHP`, `DeadState`
- Enemy 발견 상태와 선택 정책
- Enemy `Speed`, `Direction`, `Gait`, `CanMove`, `IsFalling`
- Enemy `CurrentHP`, `MaxHP`, `DeadState`
- 기존 EventLog가 계속 갱신되는지

## 13. 구현 전 결정 필요 항목

| 결정 항목 | 선택지 | 권장 |
| --- | --- | --- |
| Enemy fallback scan 1단계 허용 | 허용 / 보류 | 허용하되 fallback 표시 필수 |
| Layout | stacked / 좌우 column | stacked |
| EventLog 위치 | 공통 recent block 유지 / panel별 임시 중복 | 공통 recent block 유지 |
| Header 색상 | 코드 상수 / CVar | 코드 상수 |
| 다중 enemy 처리 | 첫 enemy 표시 / ambiguous 제한 | ambiguous 제한 |
| enemy scan cooldown | 0.25초 / 0.5초 / 1.0초 | 0.5초 |

위 결정 중 layout, EventLog 위치, 다중 enemy 처리, scan cooldown은 구현 전에 사용자 확인을 받는다.

## 14. 최종 구현 범위

P0.5 HUD 1단계에서 구현할 것:

- Player/Enemy stacked panel
- Player blue header
- Enemy red header
- Player Movement/HP getter polling
- Enemy Movement/HP getter polling
- cached enemy fallback scan
- enemy missing/ambiguous 상태 표시
- 기존 P0 recent block 유지

P0.5 HUD 1단계에서 구현하지 않을 것:

- Player/Enemy EventLog 분리
- Store subject key 확장
- Event category filter
- compact summary format 변경
- combat snapshot 기반 enemy 추론
- Target Component 기반 enemy selection
- Blackboard target actor 보조 검토
- 다중 enemy cycling

이 범위를 기준으로 다음 단계에서 실제 HUD 구현을 진행한다.
