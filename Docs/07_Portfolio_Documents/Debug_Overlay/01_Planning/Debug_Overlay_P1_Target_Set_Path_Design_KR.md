# Debug Overlay P1 Target Set Path Design

## 1. 목적

이 문서는 `UCDebugOverlayTargetComponent`에 실제 target 값을 세팅하기 위한 debug-only 호출 경로를 확정한다.

현재 `UCDebugOverlayTargetComponent`는 `ACPlayerController` 소유 component로 구현되어 있다. P1 최종 source 정책은 `Debug_Overlay_P1_Target_Selection_Decision_KR.md`를 우선하며, HUD는 명시 target이 있을 때만 `TargetComponent.Trace` 또는 `TargetComponent.Nearest`를 표시한다.

P1의 목적은 범용 lock-on 또는 gameplay targeting system을 만드는 것이 아니다. 이번 경로는 포트폴리오 evidence capture를 위한 개발 전용 target provider 입력 경로다.

## 2. 현재 상태

| 항목 | 상태 |
| --- | --- |
| Target component | `UCDebugOverlayTargetComponent` 구현 완료 |
| Owner | `ACPlayerController` |
| 저장 방식 | `TWeakObjectPtr<AActor>` |
| public API | `HasDebugOverlayTarget`, `GetDebugOverlayTargetActor`, `GetDebugOverlayTargetSummary`, `GetDebugOverlayTargetSource`, `SetDebugOverlayTarget`, `ClearDebugOverlayTarget` |
| HUD source policy | `TargetComponent.Trace/Nearest -> None` |
| 누락 지점 | `SetDebugOverlayTarget()` 호출 경로 |

현재 상태에서 `EnemySource: TargetComponent.Trace/Nearest`는 component target과 source type이 외부에서 세팅될 때만 표시된다. 따라서 다음 구현 단계에서는 `ACPlayerController`에서 debug-only helper를 통해 target component에 Enemy와 source type을 넣는 경로가 필요하다.

## 3. 후보 방식 비교

| 후보 | 장점 | 리스크 | P1 판단 |
| --- | --- | --- | --- |
| PlayerController debug console command | asset/config 변경 없이 호출 가능 | 프로젝트 내 console command 선례가 적음 | 권장 |
| PlayerController debug-only input binding | 기존 `SetupInputComponent()` 패턴과 맞음 | input mapping 추가가 필요하면 config/asset 변경으로 번질 수 있음 | 보조 후보 |
| camera forward trace | "보고 있는 Enemy 선택"이라는 evidence 설명이 명확함 | trace channel/distance 결정 필요 | 1순위 선택 알고리즘 |
| nearest enemy scan | 구현 확실성이 높고 명시 command 기반 보조 선택으로 설명 가능 | 다중 enemy에서 선택 근거가 약함 | 별도 command |
| mouse cursor hit result | 화면 클릭 기반 선택 가능 | 현재 mouse interaction 패턴이 없고 PIE capture와 충돌 가능 | P1 보류 |
| RecentCombatTarget 자동 승격 | 실제 전투 상대 기반 | "명시 선택 target"과 "최근 전투 상대" 의미가 섞임 | 보류 |
| gameplay target/lock-on 연동 | 장기적으로 자연스러운 구조 | P1 debug overlay 범위를 초과함 | 제외 |

## 4. 권장 방식

P1에서는 `ACPlayerController`에 debug-only helper를 추가하고, 호출 경로는 console command 또는 exec command 후보로 둔다.

권장 선택 순서:

1. camera forward trace로 `ACEnemy` 선택
2. 별도 nearest command로 nearest `ACEnemy` 선택
3. 실패 또는 clear command로 target 해제

이 방식은 다음 이유로 적합하다.

- target 선택 책임이 `ACPlayerController` 내부 debug-only 경로에 머문다.
- `UCDebugOverlayTargetComponent` API를 변경하지 않아도 된다.
- HUD와 Store 구조를 변경하지 않아도 된다.
- target 없음 상태를 `EnemySource: None`으로 명확히 보여줄 수 있다.
- RecentCombatTarget은 자동 승격하지 않아 의미 혼동을 줄인다.

## 5. 구현 후보 API

`ACPlayerController`에 다음 debug-only helper를 후보로 둔다.

```cpp
#if !UE_BUILD_SHIPPING
bool SelectDebugOverlayTargetFromView();
bool SelectDebugOverlayNearestEnemy();
void ClearDebugOverlayTarget();
#endif
```

console 또는 exec command 후보:

```text
Portfolio.DebugOverlay.SelectTarget
Portfolio.DebugOverlay.SelectNearestTarget
Portfolio.DebugOverlay.ClearTarget
```

최종 구현 시 프로젝트 내 선례를 다시 확인한다. `FAutoConsoleCommand`가 새 패턴으로 과하다고 판단되면 `UFUNCTION(Exec)` 또는 `ACPlayerController` 내부 debug helper 호출 경로로 축소한다.

## 6. Target 선택 기준

| 기준 | P1 권장값 | 비고 |
| --- | --- | --- |
| 대상 class | `ACEnemy` | player/other actor 제외 |
| trace channel | `ECC_Visibility` 후보 | 구현 전 충돌/가시성 반응 확인 필요 |
| trace distance | `5000.f` 후보 | TestRoom capture 거리 기준 |
| nearest command radius | `1500.f` 후보 | 명시 nearest command 선택 범위 |
| dead enemy 제외 | 구현 전 결정 필요 | HP/DeadState 기준이 실제 capture에 필요한지 확인 |
| 다중 enemy tie-break | 거리 우선 | deterministic selection 필요 |

`ACEnemy`는 현재 movement, health, action/reaction, parry stagger getter를 제공하므로 overlay panel 표시 대상과 잘 맞는다.

## 7. 표시 기대 결과

선택 성공:

```text
EnemySource: TargetComponent.Trace
EnemyTarget: Selected=BP_CEnemy_C_1
```

선택 실패:

```text
EnemySource: TargetComponent.Nearest
EnemyTarget: Selected=BP_CEnemy_C_1
```

또는

```text
EnemySource: None
```

clear 이후:

```text
EnemySource: None
```

또는

```text
EnemySource: None
```

clear는 target component의 명시 target을 해제하고 Enemy panel을 `EnemySource: None` 상태로 만든다. Store recent combat pair나 world scan diagnostic 데이터를 지우는 기능은 아니다.

## 8. 사용자 결정이 필요한 항목

구현 전 다음 항목은 사용자 확인이 필요하다.

| 결정 항목 | 권장 | 이유 |
| --- | --- | --- |
| 호출 방식 | console/exec command 우선 | config/asset 변경 없이 PIE에서 호출 가능 |
| 선택 알고리즘 | camera trace command와 nearest command 분리 | evidence 설명과 구현 안정성 균형 |
| clear command | 포함 | 명시 target 없음 상태 확인 가능 |
| dead enemy 제외 | 우선 제외하지 않음 | capture 대상이 죽은 상태인지 애매하면 과도한 필터가 될 수 있음 |
| trace distance | `5000.f` | TestRoom 기본 거리 대응 |
| nearest radius | `1500.f` | fallback이 너무 넓어지는 문제 방지 |

결정이 바뀌면 구현 프롬프트에서 명시한다.

## 9. 제외 범위

이번 설계와 다음 최소 구현에서는 다음을 제외한다.

- 범용 `UCTargetSelectionComponent`
- lock-on system
- target cycling UI
- camera/aim assist
- combat action target 강제
- AI target selection 변경
- 기존 `ITargetContextProvider` 확장
- Blueprint asset 수정
- `.umap`, `.uasset`, config, `Build.cs` 변경
- RecentCombatTarget 자동 승격
- Player/Enemy EventLog 분리
- EventLog category filter
- 최종 촬영/패키징

## 10. 구현 단계 기준

다음 구현 단계는 다음 기준을 따른다.

1. `ACPlayerController`에 debug-only helper를 추가한다.
2. `UCDebugOverlayTargetComponent` API는 가능하면 변경하지 않는다.
3. HUD/Store 구조는 변경하지 않는다.
4. camera forward trace와 nearest command는 `ACEnemy`만 반환한다.
5. 명시 target clear 경로를 포함한다.
6. Shipping에서는 no-op 또는 컴파일 제외한다.
7. PIE에서 `EnemySource: TargetComponent.Trace`와 `EnemySource: TargetComponent.Nearest`를 확인한다.
8. 실패/clear 후 `EnemySource: None`을 확인한다.

## 11. 완료 기준

이 설계 문서가 완료되면 다음이 확정된 것으로 본다.

- P1 target set 경로는 debug-only다.
- 범용 target/lock-on system으로 확장하지 않는다.
- 선택 알고리즘은 camera trace command와 nearest command를 분리한다.
- RecentCombatTarget은 자동 승격하지 않는다.
- clear command/helper는 target component를 비우고 `EnemySource: None` 상태로 만든다.
- 다음 작업은 `P1 Debug Target Set 경로 실제 구현`이다.
