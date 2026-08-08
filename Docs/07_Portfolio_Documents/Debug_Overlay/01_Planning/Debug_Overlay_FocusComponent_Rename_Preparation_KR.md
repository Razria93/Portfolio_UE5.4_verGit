# Debug Overlay FocusComponent Migration Record

## 1. 목적

이 문서는 Debug Overlay의 과거 `Target` 명명이 현재 `Focus` 명명으로 이주된 기록을 보존한다.

여기서 old side에 남아 있는 `Target` 식별자는 stale naming이 아니라 migration/debugging을 위한 historical identifier다. serialized reference, Core Redirect, 문서 링크, 리뷰 분석 시 실제로 어떤 이름에서 어떤 이름으로 이동했는지 확인하기 위해 보존한다.

이번 문서는 기능 확장 계획이 아니다. Canvas ellipsis, Blueprint/UMG, Runtime LOD actual, BT active node tracking은 포함하지 않는다.

## 2. 이주 결과 요약

| 항목 | Pre-migration | Current |
| --- | --- | --- |
| class rename | `UCDebugOverlayTargetComponent` | `UCDebugOverlayFocusComponent` |
| file rename | `CDebugOverlayTargetComponent.h/.cpp` | `CDebugOverlayFocusComponent.h/.cpp` |
| generated header | `CDebugOverlayTargetComponent.generated.h` | `CDebugOverlayFocusComponent.generated.h` |
| Controller member | `DebugOverlayTargetComponent` | `DebugOverlayFocusComponent` |
| HUD component lookup | `FindComponentByClass<UCDebugOverlayTargetComponent>()` | `FindComponentByClass<UCDebugOverlayFocusComponent>()` |
| source enum | `EDebugOverlayTargetSource` | `EDebugOverlayFocusSource` |
| component subobject name | `TEXT("DebugOverlayTarget")` | `TEXT("DebugOverlayFocus")` |
| public command naming | `DebugOverlay*Target` | `DebugOverlay*Focus` |
| HUD actor label | `FocusTarget:` | `FocusActor:` |
| panel title typo | `Pannel_01/02/03` | `Panel_01/02/03` |
| Store schema/API | unchanged | unchanged |

## 3. P57 최종 정책

P57 기준으로 Debug Overlay focus 선택/표시/에디터 조작 경로는 `Focus` 명명만 사용한다.

현재 command surface:

```cpp
DebugOverlaySelectNearestFocus
DebugOverlaySelectOutlinerFocus
DebugOverlaySelectRecentCombatFocus
DebugOverlayClearFocus
```

P57에서 제거된 legacy command:

```cpp
DebugOverlaySelectNearestTarget
DebugOverlaySelectOutlinerTarget
DebugOverlaySelectActorTarget
DebugOverlaySelectRecentCombatTarget
DebugOverlayClearTarget
```

현재 CVar:

```cpp
Portfolio.DebugOverlay.NearestFocusRadius
```

P57에서 제거된 legacy CVar:

```cpp
Portfolio.DebugOverlay.NearestTargetRadius
```

현재 subobject name:

```cpp
TEXT("DebugOverlayFocus")
```

`TEXT("DebugOverlayTarget")`는 historical pre-migration name으로만 남긴다. P57 수동 검수에서 editor loading, BP compile, PIE 진입, component/class reference 깨짐 없음이 확인되었으므로 현재 브랜치에서는 별도 Core Redirect를 추가하지 않는다.

## 4. 현재 책임 분리

| 영역 | 현재 책임 |
| --- | --- |
| `UCDebugOverlayFocusComponent` | debug-only focus actor/source/command result 저장 |
| `FDebugOverlayFocusResolver` | nearest/outliner/recent-combat focus 후보 탐색과 resolve result 생성 |
| `ACPlayerController` | console command entry, resolver 호출, focus result 적용 |
| `ACDebugOverlayHUD` | FocusComponent에서 표시 대상 enemy와 마지막 command result 조회 |
| `FDebugOverlayViewDataBuilder` | Store/actor/focus context를 ViewData로 구성 |
| `FDebugOverlayTextFormatter` | ViewData를 표시 문자열 panel로 변환 |
| `FDebugOverlaySnapshotStore` | snapshot/event/recent summary 저장 및 copy API 제공 |
| Editor plugin | PIE controller에 focus console command 전송 |

FocusComponent는 Store, World scan, resolver 책임을 갖지 않는다. Store도 focus selection owner가 아니다.

## 5. 변경하지 않는 Target 문맥

다음 `Target` 명명은 Debug Overlay focus rename 대상이 아니다.

| 명명 | 이유 |
| --- | --- |
| `Target:` | AI Blackboard current target 표시 |
| `DistanceToTarget:` | AI Blackboard distance metric 표시 |
| `TargetText` / `DistanceToTargetText` | AI ViewData field |
| `EDebugOverlayRecentAIEventViewState::NoTarget` | AI recent event target 없음 상태 |
| `FDebugOverlayRecentCombatPair::TargetActor` | combat/snapshot schema |
| `FDebugOverlayRecentCombatPair::TargetName` | combat/snapshot schema |
| `CCombatSignalTargetComponent` | gameplay combat signal system |

위 항목은 의미상 gameplay/AI/combat target이며 Debug Overlay focus actor와 다르다.

## 6. Store schema/API 변경 판단

Store schema/API 변경은 필요하지 않다.

근거:

- `FDebugOverlayFocusResolver`는 focus 후보를 해석할 수 있지만 Store schema를 소유하지 않는다.
- `UCDebugOverlayFocusComponent`는 focus actor/source/command result만 저장한다.
- `ACPlayerController`는 resolver result를 FocusComponent에 적용할 뿐 Store를 변경하지 않는다.
- `ACDebugOverlayHUD`는 focus 대상 actor를 resolve한 뒤 기존 ViewData build context에 전달한다.
- `FDebugOverlayViewDataBuilder`는 기존 Store copy API만 사용한다.
- `FDebugOverlaySnapshot`, `FDebugOverlayEventEntry`, `FDebugOverlayRecentCombatPair` 구조 변경이 필요하지 않다.

## 7. Core Redirect / asset reference 판단

P57에서는 다음을 확인했다.

```text
- clean/full rebuild: OK
- editor loading: OK
- BP compile: OK
- PIE entry: OK
- component/class reference broken: none observed
```

따라서 현재 증거 기준으로 Core Redirect는 추가하지 않는다.

Core Redirect 또는 asset migration이 필요한 경우는 다음처럼 실제 reference 문제가 확인될 때다.

```text
- BP inherited component reference missing
- serialized component override lost
- editor loading warning/error from old class path
- asset reference audit에서 old class path 발견
```

## 8. 검증 기준

필수 검색:

```text
rg "DebugOverlaySelectNearestTarget|DebugOverlaySelectOutlinerTarget|DebugOverlaySelectActorTarget|DebugOverlaySelectRecentCombatTarget|DebugOverlayClearTarget|DebugOverlayTarget|NearestTarget|OutlinerTarget|RecentCombatTarget|GameplayTarget|TargetComponentLive|NoTargetFound|TargetIsNotEnemy|InvalidTargetComponent|LogInvalidTargetComponent|FocusTarget|Pannel_" Source/Portfolio Plugins/PortfolioDebugOverlayEditor
rg "Target:|DistanceToTarget:|TargetText|DistanceToTargetText|EDebugOverlayRecentAIEventViewState::NoTarget" Source/Portfolio/Core/Debug
rg "TargetActor|TargetName|CCombatSignalTargetComponent" Source/Portfolio
git diff --check
PortfolioEditor Win64 Development build
```

사용자 검수:

```text
- editor loading
- BP compile
- PIE entry
- DebugOverlaySelectNearestFocus
- DebugOverlaySelectOutlinerFocus
- DebugOverlaySelectRecentCombatFocus
- DebugOverlayClearFocus
- legacy Target command 제거 확인
- Panel_01/02/03 표시 확인
- FocusActor 표시 확인
- AI Target / DistanceToTarget 표시 유지 확인
```

## 9. 이번 작업에서 제외한 항목

- Canvas text overflow / ellipsis
- Blueprint/UMG adapter
- Blueprint/UMG override
- Runtime LOD actual
- BT active node tracking
- gameplay lock-on / targeting system
- 자동 RecentCombat focus fallback
- Store schema/API 실제 변경
- `Build.cs`, `Portfolio.uproject`, `.umap`, `.uasset` 변경

## 10. 후속 작업 후보

이번 migration cleanup 이후 구현성 작업은 별도 판단 대상이다.

우선순위 후보:

1. Canvas text overflow / ellipsis
2. Runtime LOD actual 데이터 소스 조사 및 구현
3. BT active node tracking 설계
4. Blueprint/UMG adapter/override 설계
