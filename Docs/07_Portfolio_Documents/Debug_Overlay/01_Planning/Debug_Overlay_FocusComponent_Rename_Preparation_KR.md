# Debug Overlay FocusComponent Rename Preparation

## 1. 조사 목적

이 문서는 `UCDebugOverlayTargetComponent`를 장기적으로 `UCDebugOverlayFocusComponent`로 전환하기 전에 남아 있는 `Target` 명명 사용처를 분류하고, 안전한 전환 순서를 정리하기 위한 계획 문서다.

이번 단계는 조사/계획 작업이다. 코드 구현, class/file rename, public API 제거, console command rename, Editor Tooling 변경은 하지 않는다.

## 2. 현재 Focus/Target 책임 상태

최근 Debug Overlay cleanup 이후 runtime focus 경로는 다음 상태다.

| 영역 | 현재 상태 |
| --- | --- |
| HUD 표시 | `EnemyFocusMode`, `EnemyFocusActor`, `EnemyFocusCommand` 표시로 전환됨 |
| HUD 조회 | `UCDebugOverlayTargetComponent`의 Focus getter API를 사용함 |
| Controller 적용 | `SetDebugOverlayFocus`, `ClearDebugOverlayFocus`, `SetDebugOverlayFocusCommandResult`를 사용함 |
| Resolver | `FDebugOverlayFocusResolver`가 nearest/editor-selection 탐색 결과만 반환함 |
| TargetComponent | 실제 책임은 focus actor/source/command result 저장소에 가까움 |
| Editor Tooling | PIE `PlayerController->ConsoleCommand(...)`를 호출하는 command sender 역할만 수행함 |

즉 현재 `TargetComponent`라는 이름은 남아 있지만, runtime 의미는 이미 `Focus`에 가깝다. 다만 class/file name, 기존 public Target API, console command 이름, Editor Tooling label에는 compatibility를 위해 Target 용어가 남아 있다.

## 3. 남은 Target 명명 사용처 요약

조사 기준:

```text
DebugOverlayTarget
TargetComponent
GetDebugOverlayTarget
SetDebugOverlayTarget
ClearDebugOverlayTarget
DebugOverlaySelection
EnemyTarget
EnemySource
EnemySelect
DebugOverlaySelectNearestTarget
DebugOverlaySelectActorTarget
DebugOverlayClearTarget
```

확인된 주요 사용처:

| 파일 | 남은 명칭 | 성격 |
| --- | --- | --- |
| `CDebugOverlayTargetComponent.h/.cpp` | `UCDebugOverlayTargetComponent` | class/file rename 후보 |
| `CDebugOverlayTargetComponent.h/.cpp` | `DebugOverlayTargetActor`, `DebugOverlayTargetSource`, `DebugOverlaySelectionSummary` | 내부 필드 rename 후보 |
| `CDebugOverlayTargetComponent.h/.cpp` | `GetDebugOverlayTarget*`, `SetDebugOverlayTarget`, `ClearDebugOverlayTarget`, `*SelectionSummary` | compatibility wrapper |
| `CDebugOverlayTargetComponent.cpp` | `FormatDebugOverlayTargetSource` | 내부 helper rename 후보 |
| `CPlayerController.h/.cpp` | `DebugOverlayTargetComponent` member | 내부 member rename 후보 |
| `CPlayerController.h/.cpp` | `DebugOverlaySelectNearestTarget`, `DebugOverlaySelectActorTarget`, `DebugOverlayClearTarget` | command-facing compatibility |
| `CPlayerController.h/.cpp` | `ClearDebugOverlayTarget()` | command-facing/internal bridge rename 후보 |
| `CDebugOverlayHUD.cpp/.h` | `ResolveTargetComponentEnemy` | 내부 helper rename 후보 |
| `CDebugOverlayHUD.cpp` | `EnemySource:` legacy diagnostic strings | RecentCombat/WorldScanFallback diagnostic path |
| `FDebugOverlayFocusResolver.*` | `EDebugOverlayTargetSource` | enum rename 후보, 후순위 |
| `PortfolioDebugOverlayEditorModule.cpp` | command string constants | command sender compatibility |
| `PortfolioDebugOverlayEditorModule.cpp` | `Target` section/button/status labels | Editor UI terminology rename 후보 |

## 4. 사용처 분류

### 4.1 Compatibility Wrapper

기존 public Target API는 바로 제거하지 않는다. 외부 호출 가능성이 있고, 현재도 Focus API wrapper와 호환 계층 역할을 한다.

유지 대상:

```cpp
bool HasDebugOverlayTarget() const;
bool HasDebugOverlaySelectionSummary() const;
AActor* GetDebugOverlayTargetActor() const;
FString GetDebugOverlayTargetSummary() const;
FString GetDebugOverlayTargetSource() const;
FString GetDebugOverlaySelectionSummary() const;

void SetDebugOverlayTarget(AActor* InTargetActor, EDebugOverlayTargetSource InSource);
void ClearDebugOverlayTarget();
void SetDebugOverlaySelectionSummary(const FString& InSummary);
void ClearDebugOverlaySelectionSummary();
```

권장 처리:

- 당장 제거하지 않는다.
- Focus API로 위임하는 wrapper 상태를 유지한다.
- 호출부가 충분히 Focus API로 전환된 뒤 후반부 cleanup에서 제거 여부를 다시 판단한다.

### 4.2 Command-Facing Compatibility

Console command 이름은 Editor Tooling과 사용자 workflow가 의존하는 외부 인터페이스다. 이번 전환 작업에서 변경하지 않는다.

유지 대상:

```cpp
UFUNCTION(Exec)
void DebugOverlaySelectNearestTarget();

UFUNCTION(Exec)
void DebugOverlayClearTarget();

UFUNCTION(Exec)
void DebugOverlaySelectActorTarget(const FString& ActorName);
```

Editor plugin command sender 문자열:

```cpp
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
DebugOverlaySelectActorTarget
```

권장 처리:

- command 이름은 유지한다.
- 필요하면 후반부에 `Focus` command alias를 추가하고 기존 `Target` command를 alias로 남기는 방식만 검토한다.
- 기존 command 삭제 또는 rename은 하지 않는다.

### 4.3 내부 구현명 Rename 후보

다음 항목은 외부 command/API보다 영향 범위가 좁으므로 class/file rename 전에 정리할 수 있다.

| 현재 이름 | 권장 방향 |
| --- | --- |
| `DebugOverlayTargetComponent` member | `DebugOverlayFocusComponent` |
| `DebugOverlayTargetActor` | `DebugOverlayFocusActor` |
| `DebugOverlayTargetSource` | `DebugOverlayFocusSource` |
| `DebugOverlaySelectionSummary` | `DebugOverlayFocusCommandResult` |
| `FormatDebugOverlayTargetSource` | `FormatDebugOverlayFocusSource` 또는 `FormatDebugOverlayFocusModeText` |
| `ResolveTargetComponentEnemy` | `ResolveFocusComponentEnemy` |
| `ClearDebugOverlayTarget()` private bridge | command-facing wrapper 유지 여부를 보고 후순위 검토 |

주의:

- `TEXT("DebugOverlayTarget")` subobject name은 단순 내부 변수보다 영향이 클 수 있다. asset/reference 영향 확인 전 변경하지 않는다.
- `EDebugOverlayTargetSource` enum은 `FocusResolver` result와 `TargetComponent` storage가 공유하므로 class/file rename 뒤 또는 별도 작업으로 분리한다.

### 4.4 Class/File Rename 후보

가장 후순위로 분리해야 하는 항목:

```text
UCDebugOverlayTargetComponent -> UCDebugOverlayFocusComponent
CDebugOverlayTargetComponent.h/.cpp -> CDebugOverlayFocusComponent.h/.cpp
CDebugOverlayTargetComponent.generated.h -> CDebugOverlayFocusComponent.generated.h
```

이 작업은 Unreal reflection, generated header, include path, serialized component/subobject reference, Blueprint/asset reference 영향을 가질 수 있다.

권장 처리:

- 내부 member/field/helper rename을 먼저 끝낸다.
- public compatibility API와 command 이름이 안정화된 뒤 별도 작업으로 처리한다.
- 필요하면 Core Redirect 또는 asset migration 필요 여부를 별도로 조사한다.

### 4.5 유지해야 하는 명칭

다음 명칭은 당장 유지한다.

| 명칭 | 유지 이유 |
| --- | --- |
| `DebugOverlaySelectNearestTarget` | console command compatibility |
| `DebugOverlaySelectActorTarget` | console command compatibility 및 Editor command sender 의존 |
| `DebugOverlayClearTarget` | console command compatibility 및 Editor command sender 의존 |
| `GetDebugOverlayTarget*` 계열 | public compatibility wrapper |
| `SetDebugOverlayTarget`, `ClearDebugOverlayTarget` | public compatibility wrapper |
| `EnemySource:` legacy diagnostic strings | RecentCombat/WorldScanFallback diagnostic helper, Focus display main path 아님 |

### 4.6 이미 Focus로 전환 완료된 항목

완료된 항목:

- HUD는 Focus getter API를 사용한다.
- Controller는 Focus setter/result API를 사용한다.
- FocusResolver는 `TargetComponent` 저장소에 직접 접근하지 않는다.
- FocusResolver는 Store/HUD/ViewData/Formatter/Renderer를 직접 호출하지 않는다.
- Enemy panel 표시는 `EnemyFocusMode`, `EnemyFocusActor`, `EnemyFocusCommand`를 사용한다.

## 5. Editor Tooling 경계

`Plugins/PortfolioDebugOverlayEditor`는 Editor-only plugin이며, runtime focus state를 직접 읽거나 저장하지 않는다.

현재 역할:

- Debug Overlay CVar UI 제공
- PIE world의 first player controller를 찾음
- runtime console command를 `PlayerController->ConsoleCommand(...)`로 전송
- 마지막 command 전송 상태를 Editor UI에 표시

현재 Target 용어가 남은 Editor UI:

```text
Target
Runs existing debug overlay target console commands during PIE...
Select Nearest Target
Select Outliner Actor
Clear Target
Last Command: SelectNearestTarget
Last Command: ClearTarget
Last Command: SelectOutlinerActor
```

분류:

- command string constant는 command-facing compatibility로 유지한다.
- 버튼/섹션 label은 Editor UI terminology rename 후보지만, command rename과 별도 검증이 필요하다.
- Editor plugin은 FocusResolver, TargetComponent, Store, ViewData, Renderer를 직접 호출하지 않는 현재 경계를 유지한다.

## 6. 안전한 전환 순서

권장 순서:

1. 내부 구현명 rename 1차
   - `DebugOverlayTargetComponent` member를 `DebugOverlayFocusComponent`로 rename
   - `ResolveTargetComponentEnemy`를 `ResolveFocusComponentEnemy`로 rename
   - command 이름과 public Target API는 유지

2. `CDebugOverlayTargetComponent` 내부 field/helper rename
   - `DebugOverlayTargetActor` -> `DebugOverlayFocusActor`
   - `DebugOverlayTargetSource` -> `DebugOverlayFocusSource`
   - `DebugOverlaySelectionSummary` -> `DebugOverlayFocusCommandResult`
   - `FormatDebugOverlayTargetSource` -> Focus 명명 helper
   - public Target wrapper 반환 정책은 유지

3. Editor UI terminology 검토
   - button/section label을 Focus 용어로 바꿀지 판단
   - command sender 문자열은 유지
   - UI 문구 변경이므로 별도 검증 필요

4. enum rename 검토
   - `EDebugOverlayTargetSource`를 `EDebugOverlayFocusSource` 또는 `EDebugOverlayFocusMode`로 전환할지 판단
   - FocusResolver result, TargetComponent storage, compatibility wrapper 영향 확인

5. class/file rename 별도 작업
   - `UCDebugOverlayTargetComponent` 및 파일명 변경
   - generated header/include path 반영
   - asset/reference/Core Redirect 필요 여부 확인

6. Target compatibility wrapper 제거 여부 판단
   - 모든 내부 호출부가 Focus API로 전환된 뒤 검토
   - command-facing 이름은 계속 유지하거나 alias 정책을 별도 결정

## 7. 보류/금지 항목

이번 전환 준비 단계 및 다음 초기 구현 단계에서 금지/보류할 항목:

- console command 이름 변경 금지
- Editor Tooling command sender 문자열 변경 금지
- public Target API 제거 금지
- class/file rename은 후순위 별도 작업
- `TEXT("DebugOverlayTarget")` subobject name 변경 보류
- Store public API/schema 변경 금지
- HUD 표시 정책 변경 금지
- ViewData/Formatter/Renderer 변경 금지
- FocusResolver 동작 변경 금지
- RecentCombat focus 연결 금지
- WorldScanFallback 연결 금지
- Runtime LOD actual 구현 금지
- BT active node tracking 구현 금지
- Blueprint/UMG 노출 금지
- `.umap`, `.uasset`, config, `Build.cs`, `Portfolio.uproject` 변경 금지

## 8. 검증 기준

구현 작업별 공통 검증:

```text
rg "DebugOverlayTarget|TargetComponent|DebugOverlaySelection"
rg "DebugOverlaySelectNearestTarget|DebugOverlaySelectActorTarget|DebugOverlayClearTarget"
rg "GetDebugOverlayTarget|SetDebugOverlayTarget|ClearDebugOverlayTarget"
```

확인할 것:

- 남은 Target 명칭이 compatibility wrapper, command-facing 이름, 보류 항목으로 분류되는지 확인한다.
- Editor plugin이 runtime command sender 외 책임을 갖지 않는지 확인한다.
- Store/HUD display policy/ViewData/Formatter/Renderer가 오염되지 않았는지 확인한다.
- `Build.cs`, `Portfolio.uproject`, config, asset 변경이 없는지 확인한다.
- 기본 `PortfolioEditor Win64 Development` 빌드가 통과하는지 확인한다.

## 9. 다음 구현 작업 제안

다음 구현 작업은 class/file rename이 아니라 내부 구현명 rename 1차가 적절하다.

권장 범위:

- `CPlayerController` member `DebugOverlayTargetComponent`를 `DebugOverlayFocusComponent`로 rename
- `CDebugOverlayHUD` helper `ResolveTargetComponentEnemy`를 `ResolveFocusComponentEnemy`로 rename
- public Target API, console command 이름, Editor Tooling command sender 문자열은 유지
- `TEXT("DebugOverlayTarget")` subobject name은 이번 단계에서 유지

권장 커밋 메시지:

```text
refactor(debug): rename overlay focus internals
```
