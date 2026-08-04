# Debug Overlay ViewData / Renderer Separation Design

## 1. 목적

이 문서는 Debug Overlay runtime HUD를 단순 파일 분리 대상으로 보지 않고, 장기적으로 Blueprint/UMG 디자인 override가 가능한 구조로 분리하기 위한 책임 경계와 구현 PR 순서를 정의한다.

최근 `refactor(debug): clean overlay hud and snapshot store`에서 `CDebugOverlayHUD.cpp`와 `FDebugOverlaySnapshotStore.cpp`의 low-risk cleanup은 완료했다. 다음 단계에서는 Canvas HUD 중심의 helper 분리를 넘어서 `Store -> ViewDataBuilder -> Renderer` 구조를 설계한다.

이번 작업은 조사/설계 작업이다. 코드 구현, 파일 이동, `.umap`, `.uasset`, config, `Build.cs`, `Portfolio.uproject` 변경은 하지 않는다.

## 2. 현재 상태

| 항목 | 확인 결과 |
| --- | --- |
| 현재 브랜치 | `main` |
| HEAD | `8a33801b01b964431652a6b332ee905f3eaf85cb` |
| origin/main | `87230feb3f99b66cec850bfac4e559851ee46dbc` |
| 관계 | `main...origin/main [ahead 2]` |
| 최근 문서 커밋 | `ce4bd522 docs(debug): investigate overlay code cleanup` |
| 최근 cleanup 커밋 | `8a33801b refactor(debug): clean overlay hud and snapshot store` |

현재 Debug Overlay는 다음 책임으로 구성되어 있다.

| 영역 | 현재 파일 | 현재 책임 |
| --- | --- | --- |
| Store | `FDebugOverlaySnapshotStore.*`, `FDebugOverlaySnapshotTypes.h` | runtime evidence 원천 데이터, EventLog ring, CVar gate, filter/query |
| HUD | `CDebugOverlayHUD.cpp` | Store query, actor state read, AI blackboard read, line buffer 생성, Canvas layout/draw |
| Target bridge | `CDebugOverlayTargetComponent.*`, `CPlayerController.cpp` | selected enemy target/source/summary 저장 및 Exec command |
| Editor Tooling | `PortfolioDebugOverlayEditorModule.cpp` | Editor-only CVar UI, target command button, menu/toolbar/tab |

## 3. 현재 HUD 책임 분류

`CDebugOverlayHUD.cpp`에는 다음 책임이 아직 함께 있다.

| 책임 | 현재 위치/형태 | 장기 분리 방향 |
| --- | --- | --- |
| Store query | `DrawHUD()`에서 snapshot, EventLog filter/limit, recent events 조회 | ViewDataBuilder 입력 수집 단계로 이동 |
| actor state read | `FormatExecutionState`, `FormatActiveAction`, `FormatGuardOverlay`, `FormatActorMovement`, `FormatActorHealth` | ViewDataBuilder 또는 ActorState provider 후보 |
| AI blackboard read | `AppendEnemyCurrentAIBlock` 내부에서 `UBlackboardComponent` 직접 조회 | 별도 AI ViewData provider 후보 |
| snapshot/event formatting | `AppendSummaryLines`, `FormatEventLogEntryLine`, `AppendEventLogBlock` | TextFormatter + ViewDataBuilder |
| ViewData/line buffer 생성 | `AppendMainActorPanelLines`, `AppendInteractionPanelLines`, actor/event block append | 1차 ViewDataBuilder 추출 대상 |
| Canvas panel layout | `CalculateRightPanelGeometry`, line height/visible lines helper | Canvas renderer 소유 |
| Canvas draw | `DrawOverlayLines`, `DrawHUD()` | Canvas fallback renderer 소유 |
| enemy target resolve | `ResolveDisplayEnemy`, `ResolveTargetComponentEnemy` | target selection view source 또는 HUD adapter 후보 |
| fallback/diagnostic helper | `ResolveRecentCombatEnemy`, `ResolveWorldScanFallbackEnemy` | 별도 결정 전 유지/보류 |

현재 구조는 P52/P53 evidence tooling으로는 동작하지만, UI 디자인을 Blueprint/UMG에서 override하려면 line buffer 생성과 Canvas draw가 분리되어야 한다.

## 4. 문제 정의

지금 `CDebugOverlayHUD.cpp`를 단순히 여러 cpp로 나누면 Canvas HUD 중심 구조가 고착될 수 있다. Blueprint/UMG override를 고려하면 분리 기준은 파일 크기가 아니라 데이터 흐름이어야 한다.

문제는 다음과 같다.

- Store raw schema를 UI가 직접 소비하면 EventLog filter, subject role, noise/collision 의미가 UI 계층으로 새어나간다.
- Blueprint가 Store를 직접 조회하면 C++ Canvas와 BP Widget 양쪽에 표시 정책이 중복된다.
- `CDebugOverlayHUD.cpp` 내부 line buffer가 현재 사실상의 ViewData 역할을 하지만, `TArray<FString>` 중심이라 BP에서 구조적으로 활용하기 어렵다.
- Canvas renderer API를 중심 계약으로 만들면 UMG에서 색상, 상태, stale/diagnostic 여부를 문자열에서 다시 파싱해야 한다.
- Canvas layout helper는 UMG layout과 관심사가 다르므로 ViewDataBuilder에 섞이면 안 된다.
- Current AI는 HUD에서 Blackboard를 직접 읽고 있어 display-only renderer로 분리하기 어렵다.

따라서 장기 구조는 raw Store를 바로 BP에 노출하는 방향이 아니라, display-safe ViewData를 생성한 뒤 Canvas와 UMG가 같은 ViewData를 소비하는 방향이어야 한다.

## 5. 장기 목표 구조

목표 구조:

```text
Debug Hooks
  -> FDebugOverlaySnapshotStore
  -> FDebugOverlayViewDataBuilder
  -> ACDebugOverlayHUD Canvas Renderer
  -> UMG / Blueprint Widget Renderer
```

책임:

| 구성요소 | 책임 |
| --- | --- |
| `FDebugOverlaySnapshotStore` | runtime evidence 원천 데이터 저장/query. CVar gate와 EventLog filter 의미 유지. |
| `FDebugOverlayTextFormatter` | bool/enum/reason/age/capture state 등 기본 문자열 format. C++ 기본 표시값 제공. |
| `FDebugOverlayViewDataBuilder` | Store, actor state, target source를 읽어 display-safe panel/row data 생성. |
| `ACDebugOverlayHUD` | Canvas fallback renderer. ViewData를 Canvas 위치/색상/텍스트로 그린다. |
| `UUserWidget` / Widget Blueprint | ViewData를 입력으로 받아 layout, 색상, typography, visibility, wrapping을 override. |
| optional `UDebugOverlayViewModel`, `UActorComponent`, `UBlueprintFunctionLibrary` facade | Blueprint binding-friendly API 제공. Store 정책을 직접 노출하지 않는 adapter. |
| `PortfolioDebugOverlayEditor` | Editor-only CVar/command tooling. runtime UI rendering 책임 없음. |

## 6. 책임 분리안

### 6.1 Store

Store는 원천 데이터와 query contract를 유지한다.

- `FDebugOverlaySnapshotStore` public API/schema를 이번 계열 리팩터링의 기반으로 둔다.
- EventLog filter는 display filter 의미로 유지한다.
- `TryGetSnapshotCopy().RecentEvents`의 raw 성격은 별도 설계 전에는 바꾸지 않는다.
- `FDebugOverlaySnapshotTypes.h`를 바로 `BlueprintType`으로 바꾸지 않는다.

### 6.2 ViewDataBuilder

ViewDataBuilder는 현재 HUD의 line buffer 생성 책임을 가져간다.

포함 후보:

- Player / Enemy panel data 구성.
- Actor current state row 생성.
- Actor별 Recent Execution row 생성.
- Interaction panel Recent Execution / Recent Combat row 생성.
- EventLog display row 생성.
- Enemy Current AI / Recent AI Event row 생성.
- `NoTarget`, `NotCaptured`, `NotMatched`, `Stale`, `NoEvents(...)` 같은 표시 상태 유지.

제외:

- Canvas 좌표, width, height, clipping 계산.
- UMG widget 생성.
- CVar UI.
- config 저장.

권장 호출 흐름:

```text
ResolveContext -> BuildViewData -> FormatTextPanels -> CanvasRenderer.Draw
```

Blueprint/UMG 경로도 `BuildViewData` 결과를 소비해야 하며, Canvas line 출력 결과를 다시 파싱하는 구조로 만들지 않는다.

### 6.3 TextFormatter

TextFormatter는 현재 HUD/Store에 흩어진 표시 문자열 기본값을 한곳에 모으는 후보이다.

포함 후보:

- `BoolText`
- `MissingText`
- `FormatAgeSeconds`
- `CompactEnumText`
- `CompactReasonText`
- `CaptureStateText`
- summary split policy

주의:

- `Pannel_01/02/03` 표시 문자열은 style-lock 상태이므로 TextFormatter로 옮기더라도 값을 바꾸지 않는다.
- `Runtime LOD: N/A`를 actual 구현처럼 바꾸지 않는다.

### 6.4 Canvas Renderer

`ACDebugOverlayHUD`는 장기적으로 Canvas fallback renderer가 된다.

남길 책임:

- Canvas availability 확인.
- ViewData를 Canvas line으로 그리기.
- Canvas 전용 panel geometry 계산.
- Canvas clipping.
- Canvas header/background draw.

빼야 할 책임:

- Store query policy.
- EventLog subject/filter 의미 판단.
- Actor component/Blackboard direct formatting.
- BP override와 공유해야 할 display-safe row 생성.

### 6.5 Blueprint Widget / ViewModel

Blueprint/UMG는 ViewData를 받아 디자인만 override한다.

허용:

- panel 배치.
- 색상, font, spacing, typography.
- row visibility.
- wrapping/scrolling.
- preset별 layout.
- icon/label 표시 방식.

금지:

- Store를 직접 순회하며 EventLog filter를 재구현.
- subject role matching 재구현.
- noise/collision display filter 재구현.
- raw `FDebugOverlaySnapshot` schema를 gameplay truth처럼 해석.
- Runtime LOD actual이나 BT active node를 아직 구현된 것처럼 표시.

## 7. ViewData 후보 타입

초기 후보는 raw snapshot type과 분리된 display-safe 타입이다.

```text
FDebugOverlayViewData
  TArray<FDebugOverlayPanelViewData> Panels
  FString PresetName
  bool bHasSnapshot

FDebugOverlayPanelViewData
  FName PanelId
  FString Title
  TArray<FDebugOverlayRowViewData> Rows
  EDebugOverlayPanelKind PanelKind

FDebugOverlayRowViewData
  FName RowId
  FString Label
  FString Value
  FString RawLine
  EDebugOverlayRowState State
  EDebugOverlayEventCategoryDisplay Category
```

초기 구현에서는 `BlueprintType`으로 바로 공개할지 결정하지 않는다. 먼저 C++ Canvas HUD가 같은 ViewData를 소비하게 만든 뒤, BP 노출은 별도 PR에서 검증한다.

Blueprint-friendly 타입으로 확장할 경우 고려할 점:

- `FString`/`FName` 중심의 안정적인 표시 데이터.
- raw actor pointer 최소화.
- category/row state는 UI decoration 용도이며 Store schema 대체가 아니다.
- Store의 `FDebugOverlayEventEntry`를 그대로 BP에 넘기지 않는다.
- `FDebugOverlayRecentCombatPair`의 `TWeakObjectPtr<AActor>`도 BP에 직접 넘기지 않고 `SourceName`, `TargetName`, `AgeSeconds`, `bStale`, `EventName` 같은 표시 안전 값으로 변환한다.

## 8. Blueprint/UMG override 가능 범위

Blueprint에서 override 가능한 범위:

- 3-panel을 2-column, tab, scroll panel 등으로 재배치.
- Player/Enemy/Interaction/EventLog panel 색상과 header style 변경.
- EventLog row wrapping과 scroll behavior 변경.
- capture preset별 표시 row 선택.
- empty/stale/not captured 상태의 visual treatment 변경.
- font, line spacing, background opacity, icon 등 presentation 변경.

Blueprint에서 override하면 안 되는 범위:

- `Portfolio.DebugOverlay.*` CVar 의미.
- EventLog filter 결과 의미.
- EventLog hidden event를 “발생하지 않음”으로 해석하는 문구.
- actor별 Recent Execution subject matching.
- Enemy Current AI와 Recent AI Event의 claim 구분.
- Runtime LOD actual 표시.
- BT active node tracking claim.

## 9. Canvas HUD fallback 유지 기준

Canvas HUD는 다음 이유로 유지한다.

- P52/P53 evidence claim의 기준 화면이다.
- UMG/Blueprint override가 없어도 개발용 overlay가 동작해야 한다.
- Shipping HUD가 아니라 non-shipping debug evidence renderer다.
- ViewDataBuilder 추출 후 회귀 비교 기준이 된다.

Canvas fallback 유지 조건:

- `Portfolio.DebugOverlay.Enabled`는 Canvas fallback 표시 gate로 유지.
- `Portfolio.DebugOverlay.Collect`는 Store record gate로 유지.
- `Pannel_01/02/03` 표시 문자열은 별도 표시 정책 PR 전까지 유지.
- 기존 line order, `NoTarget`, `NotCaptured`, `NotMatched`, `Stale`, `NoEvents(...)` 문구 유지.

## 10. Store와 ViewData 경계

Store는 “기록된 사실”과 “query contract”를 담당하고, ViewData는 “화면에 안전하게 표시할 형태”를 담당한다.

| 항목 | Store | ViewData |
| --- | --- | --- |
| raw event entry | 보관 | 필요 row로 변환 |
| EventLog filter | query/display filter contract 유지 | 이미 필터된 결과를 표시 |
| noise/collision 숨김 | Store query helper에서 판단 | 숨겨진 event를 claim으로 해석하지 않음 |
| subject role matching | Store query helper에서 판단 | role label이 붙은 display event를 row로 표시 |
| capture state | raw enum 유지 | `NotCaptured`, `Unavailable`, `Stale` 등 표시 상태로 변환 |
| actor pointer | 가능한 Store/query 내부에서 제한 | BP ViewData에는 최소화 |
| Current AI | 현재 HUD 직접 read | 별도 provider 또는 builder 단계로 이전 후보 |

EventLog query semantics는 반드시 유지한다.

```text
ring 최신순 -> category filter -> noise/collision display filter -> limit
```

Blueprint가 `TryGetSnapshotCopy().RecentEvents`를 받은 뒤 자체 필터링하면 `limit`과 display filter 적용 순서가 달라질 수 있다. 따라서 BP는 Store raw snapshot을 직접 후처리하지 않고 ViewDataBuilder가 생성한 EventLog rows를 소비해야 한다.

Blueprint에 직접 노출하지 않을 API:

- `FDebugOverlaySnapshotStore::Record...` 계열.
- `FDebugOverlaySnapshotStore::AddEvent`.
- `FDebugOverlaySnapshotStore::Reset`, `ResetAll`.
- `StoresByWorld`, EventRing, filter/noise/collision anonymous helper.
- raw `FDebugOverlaySnapshot`, raw `FDebugOverlayEventEntry`, raw `FDebugOverlayRecentCombatPair`.

## 11. Editor Tooling과 runtime UI override 경계

`PortfolioDebugOverlayEditor`는 runtime UI renderer가 아니다.

유지할 경계:

- Editor Tooling은 CVar를 읽고 쓰는 session-only control panel이다.
- target command button은 PIE PlayerController의 기존 Exec command를 호출한다.
- Editor plugin이 `TargetComponent`, Store, ViewDataBuilder, Widget BP를 직접 조작하지 않는다.
- Editor Tooling module split은 ViewData/UMG 작업과 별도 PR로 둔다.
- Editor-only plugin에 runtime ViewData/UMG ownership을 넣지 않는다.
- runtime module에 `UnrealEd`, `ToolMenus`, Editor-only dependency를 추가하지 않는다.

향후 Editor Tooling에서 Widget override class를 선택하거나 저장하는 기능은 별도 설계가 필요하다. 이번 설계에서는 config 저장과 `.uproject` 변경을 포함하지 않는다.

## 12. 구현 PR 분리안

### PR 1. ViewData type 설계/도입

목표:

- C++ 내부용 `FDebugOverlayViewData`, `FDebugOverlayPanelViewData`, `FDebugOverlayRowViewData` 후보 도입.
- raw Store schema 변경 없음.
- Blueprint 노출은 보류하거나 최소 `BlueprintType` 여부를 별도 결정.

검증:

- 기존 Canvas HUD 출력 변경 없음.
- Store header/schema 변경 없음 unless 별도 승인.

### PR 2. ViewDataBuilder 1차 추출

목표:

- `CDebugOverlayHUD.cpp`의 line buffer 생성 책임을 builder로 이동.
- Canvas HUD는 builder 결과를 받아 기존 line order로 렌더링.
- Actor state read와 AI blackboard read는 builder 내부 또는 provider 후보로 이동하되 동작 변경 없음.

검증:

- 기존 P52/P53 화면 문자열/순서 유지.
- EventLog filter/noise/collision 의미 유지.

### PR 3. Canvas HUD renderer 축소

목표:

- `ACDebugOverlayHUD`를 Canvas fallback renderer로 축소.
- panel geometry, line clipping, background/header draw만 보유.

검증:

- 3-panel Canvas fallback 유지.
- `Pannel_01/02/03` 유지.

### PR 4. UMG/Blueprint override 설계 또는 spike

목표:

- `UUserWidget`이 소비할 ViewData binding 형태 결정.
- optional `UDebugOverlayViewModel` 또는 component 방식 비교.
- 실제 Widget 구현은 spike로 제한하거나 별도 feature PR로 분리.

검증:

- Store 직접 접근 금지.
- Shipping HUD claim 금지.
- UMG dependency 추가 위치와 `Build.cs` 영향 별도 검토.

### PR 5. Editor Tooling module split

목표:

- `PortfolioDebugOverlayEditorModule.cpp`에서 CVar helper, target command helper, Slate widget 분리.
- runtime UI override 구조와 섞지 않는다.

검증:

- Editor-only plugin 경계 유지.
- runtime `Portfolio.Build.cs`, `Portfolio.uproject`, config 변경 없음.

이 PR은 PR 1~4와 독립적으로 진행할 수 있다. 다만 ViewData/UMG runtime presentation 작업과 같은 PR에 섞지 않는다.

### PR 6. Target command bridge polish

목표:

- `CPlayerController.*`, `CDebugOverlayTargetComponent.*`의 debug target command 실패 처리와 summary helper 중복을 정리.
- runtime target command 정책은 유지.

검증:

- nearest radius `3000` 유지.
- enemy-only selection 및 non-enemy reject 유지.
- `DebugOverlaySelectNearestTarget`, `DebugOverlayClearTarget`, `DebugOverlaySelectActorTarget` command 의미 유지.
- Shipping gameplay command처럼 claim하지 않음.

### 별도 feature PR

- Runtime LOD actual 표시.
- BT active node tracking.
- EventLog compact/wrapping redesign.
- Store schema/API 확장.
- generic target/lock-on system 전환.

## 13. 금지/보류 항목

| 분류 | 항목 | 판단 |
| --- | --- | --- |
| 금지 | 이번 설계 작업에서 코드 구현 | 문서 설계만 수행 |
| 금지 | `.umap`, `.uasset`, config, `Build.cs`, `Portfolio.uproject` 변경 | 범위 밖 |
| 금지 | HUD 표시 정책 변경 | evidence claim 기준 유지 |
| 금지 | Store public API/schema 변경 | 별도 PR 필요 |
| 금지 | CVar 이름/의미 변경 | Editor Tooling/운영 contract |
| 금지 | EventLog filter/noise/collision 의미 변경 | display semantics 유지 |
| 금지 | UMG/Blueprint widget 구현 | 이번 작업은 설계만 |
| 금지 | Shipping HUD claim | debug evidence overlay로 유지 |
| 금지 | Editor plugin에 runtime ViewData/UMG ownership 추가 | Editor-only CVar/command tooling 경계 유지 |
| 금지 | runtime module에 Editor-only dependency 추가 | runtime/editor 경계 유지 |
| 보류 | `FDebugOverlaySnapshotTypes.h` `BlueprintType` 전환 | raw schema 노출 위험 |
| 보류 | Current AI provider화 | 설계 후 구현 |
| 보류 | Runtime LOD actual | feature PR |
| 보류 | BT active node tracking | feature PR |
| 보류 | `Pannel` 표기 수정 | 표시 정책 PR 필요 |

## 14. 검증 기준

이번 문서 PR 검증:

- `git diff --check` 통과.
- 변경 파일이 `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_ViewData_Renderer_Separation_Design_KR.md` 하나인지 확인.
- 코드, asset, config, `Build.cs`, `Portfolio.uproject` 변경 없음.

후속 구현 PR 공통 검증:

- `PortfolioEditor Win64 Development` build 통과.
- TestRoom PIE에서 기존 P52/P53 Canvas HUD evidence 흐름 유지.
- `Portfolio.DebugOverlay.Enabled`와 `Collect` 의미 유지.
- EventLog filter/noise/collision semantics 유지.
- Store public API/schema 변경 여부 명시.
- Blueprint/UMG 경로가 생기더라도 Store raw schema를 직접 재해석하지 않음.
- Shipping HUD처럼 표현하지 않음.

## 15. 사용자 결정이 필요한 질문

다음 항목은 구현 전 사용자 결정이 필요하다.

- ViewData 타입을 처음부터 `USTRUCT(BlueprintType)`으로 둘지, C++ 내부 타입으로 먼저 검증할지.
- `UDebugOverlayViewModel`을 둘지, `UActorComponent` adapter를 둘지.
- Canvas fallback과 UMG override를 동시에 띄울 수 있게 할지, 하나만 선택하게 할지.
- Widget class 지정 위치를 GameMode/HUD/Controller 중 어디로 둘지.
- UMG dependency 추가가 필요한 시점에 `Build.cs` 변경을 별도 PR로 허용할지.
- Current AI Blackboard read를 ViewDataBuilder 내부로 옮길지, 별도 AI provider로 분리할지.
- 사용되지 않는 recent combat/world scan fallback helper를 diagnostic ViewData로 유지할지, 별도 cleanup에서 제거할지.
