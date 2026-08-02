# Debug Overlay Code Cleanup Investigation

## 1. 조사 목적

이번 조사의 목적은 P52/P53에서 완성한 Debug Overlay runtime HUD, Snapshot Store, Editor Tooling 구조를 기준으로 다음 cleanup PR의 안전한 범위를 확정하는 것이다.

이번 단계는 조사/계획 작업이며 코드 구현, 파일 이동, `.umap`, `.uasset`, config, `Build.cs`, `Portfolio.uproject` 변경은 하지 않는다. Store schema/API, CVar 이름/의미, HUD 표시 정책은 다음 cleanup PR에서도 기본적으로 유지한다.

## 2. 현재 상태

| 항목 | 확인 결과 |
| --- | --- |
| 현재 브랜치 | `main` |
| HEAD | `87230feb3f99b66cec850bfac4e559851ee46dbc` |
| origin/main | `87230feb3f99b66cec850bfac4e559851ee46dbc` |
| 작업트리 | clean |
| P52 merge | `c283ffd7 Merge pull request #107 from Razria93/feature/debug-overlay-evidence-plan` |
| P53 merge | `aa53a10a Merge pull request #108 from Razria93/feature/debug-overlay-editor-tooling` |
| P54 merge | `87230feb Merge pull request #109 from Razria93/feature/asset-reference-inspector-plugin` |

## 3. 현재 Debug Overlay 구조 요약

| 영역 | 현재 책임 | cleanup 판단 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp` | Canvas 기반 3-panel runtime evidence HUD, actor state formatting, EventLog/Interaction line buffer, selected enemy 표시, Current AI/Recent AI Event 표시 | 가장 큰 low-risk cleanup 후보 |
| `FDebugOverlaySnapshotStore.cpp` | world별 snapshot store, EventLog ring buffer, CVar gate, display filter, recent execution/combat/AI record/query | helper section 정리와 중복 축소 후보 |
| `FDebugOverlaySnapshotTypes.h` | snapshot/event/recent summary data-only schema | 변경하지 않음 |
| `CDebugOverlayTargetComponent.*` | debug overlay target actor/source/selection summary 저장 | 현재 책임 유지 |
| `CPlayerController.cpp` | debug overlay Exec command bridge, nearest/editor-selected target 판정 | command flow 유지, 로그/summary 중복만 별도 후보 |
| `PortfolioDebugOverlayEditor` | Editor-only Nomad tab, CVar UI, target command button, menu/toolbar registration | module cpp 분리 후보 |

`Source/Portfolio/Portfolio.Build.cs`에는 `UnrealEd`, `Slate`, `ToolMenus` 같은 Editor dependency가 섞이지 않았다. `Portfolio.uproject`에도 `PortfolioDebugOverlayEditor` plugin entry가 추가되어 있지 않다. Editor dependency는 `Plugins/PortfolioDebugOverlayEditor` 내부에 격리되어 있다.

## 4. HUD cleanup 후보

### 바로 해도 안전한 것

- `DrawHUD()`의 3-panel layout 산식을 작은 geometry helper로 추출한다.
  - 유지해야 하는 값: left origin, panel gap, right margin, EventLog 최소폭, Interaction 고정폭, line clipping 정책.
  - 표시 정책은 `Pannel_01`, `Pannel_02`, `Pannel_03` 구조 그대로 둔다.
- EventLog/Interaction visible line copy 중복을 공통 helper로 줄인다.
- `CalculateOverlayLinesHeight()`와 `CalculateVisibleOverlayLineCount()`가 공유하는 line height + header padding 계산을 내부 helper로 분리한다.
- `AppendSnapshotLines()`는 실제 역할이 Interaction panel 구성에 가까우므로, 동작 변경 없이 `AppendInteractionPanelLines` 계열로 이름 정리를 검토한다.
- `AppendOverlayLine(FString::Printf(...))` 반복은 `AppendLabelValueLine` 또는 `AppendFormattedLine` 같은 내부 helper로 축약할 수 있다.
- `AppendSnapshotLines()`의 Recent Execution / Recent Combat 반복 구조는 `AppendSnapshotSummaryBlock()` 계열 helper로 분리 가능하다.

### 별도 설계가 필요한 것

- HUD 파일 분리: formatting, layout, target resolve, AI blackboard read가 한 파일에 모여 있으나 파일/헤더 증가와 build 영향이 있다.
- `ResolveRecentCombatEnemy()`, `ResolveWorldScanFallbackEnemy()`는 현재 기본 draw path에서 사용되지 않는다. 삭제, diagnostic mode 복구, fallback 정책 재도입 중 하나를 결정해야 한다.
- Recent AI Event stale/match 정책을 formatter로만 둘지, Store/provider 책임으로 옮길지 결정이 필요하다.
- Current AI를 HUD에서 Blackboard 직접 조회로 유지할지, Store snapshot/provider로 이전할지 별도 설계가 필요하다.

### 하지 않는 것이 나은 것

- `[Debug Overlay Pannel_01/02/03]`의 `Pannel` 표기를 `Panel`로 고치는 것. 현재 runtime style-lock 표시값이므로 문구 변경이다.
- Player / Enemy / Interaction / EventLog 표시 순서 변경.
- `NotCaptured`, `NoTarget`, `NotMatched`, `Stale`, `NoEvents(...)` 같은 evidence 문구 변경.
- Interaction panel 표시 조건, EventLog 최소폭, Interaction 고정폭, AI stale seconds 변경.
- Runtime LOD actual 값을 연결하거나 `Runtime LOD: N/A`를 성공 evidence처럼 보이게 하는 것.

## 5. Snapshot Store cleanup 후보

### 바로 해도 안전한 것

- `Reset()`에서 직접 `StoresByWorld.Remove()`를 호출하는 대신 기존 `RemoveStoreForWorld()` helper를 재사용한다.
- `Record...()` 함수들의 frame/time/world resolve 반복을 내부 helper로 축소한다.
- `GetRecentEventsCopyFromStore()`와 subject query에서 filter normalization이 반복되지 않도록 내부 helper 의미를 명확히 한다.
- `CompactStoreEnumText()`, `CompactStoreReasonText()`, `ToSafeReason()`의 역할을 섹션으로 묶어 문자열 compact policy를 읽기 쉽게 만든다.
- EventLog record path, recent summary update path, query/filter path를 anonymous namespace 내부 섹션으로 재배치한다.

### 별도 설계가 필요한 것

- `RecordWeaponCollisionWindow()`가 EventLog만 기록하고 `LastCombat.HitWindowId/HitWindowState`를 갱신하지 않는 현재 정책을 바꿀지 여부.
- `TryGetSnapshotCopy().RecentEvents`는 raw recent snapshot 의미에 가깝고 display filter/noise filter가 적용되지 않는다. 여기에 display filter를 적용할지 여부는 API 의미 변경이다.
- summary 문자열 파싱 기반 noise 판정을 structured field/schema 기반으로 바꾸는 작업.
- `FDebugOverlayRecentCombatPair`를 `FDebugOverlaySnapshotTypes.h`로 이동할지 여부.
- `CVarDebugOverlayPreset`은 현재 명확한 runtime preset 동작 claim이 없으므로 노출/제거/보류 정책을 별도 결정한다.

### 하지 않는 것이 나은 것

- `FDebugOverlayEventEntry`, `FDebugOverlaySnapshot`, `FDebugOverlaySnapshotStore` public API 시그니처 변경.
- `CombatResult` category를 `Combat`으로 합치거나 EventLog filter 문자열을 공개 enum API로 바꾸는 것.
- `CollisionDisableIgnored` / `CollisionDisabledIgnored` 호환 방어 중 하나를 단순 삭제하는 것.
- EventLog filter를 “event 미기록/삭제” 의미로 바꾸는 것. 현재는 display filter로 유지해야 한다.

## 6. Target command / Controller bridge cleanup 후보

### 바로 해도 안전한 것

- debug overlay command는 `UFUNCTION(Exec)` 3개로 노출되어 있다.
  - `DebugOverlaySelectNearestTarget`
  - `DebugOverlayClearTarget`
  - `DebugOverlaySelectActorTarget`
- `CPlayerController.cpp`의 nearest/editor-selected 실패 처리에서 clear + summary + log 반복을 작은 내부 helper로 줄일 수 있다.
- `RecordDebugOverlayNearestSelectionResult()`와 `RecordDebugOverlayEditorSelectionResult()`는 현재 동일 동작이므로 helper 의미 정리를 검토할 수 있다.
- `CDebugOverlayTargetComponent`는 weak actor, source, selection summary만 저장하므로 구조 유지가 안전하다.

### 별도 설계가 필요한 것

- editor-selected actor command가 런타임에서 `GetName()`과 `WITH_EDITOR` label을 모두 허용하는데, Editor UI는 현재 `GetName()`만 전달한다. label 기반 선택을 공식 UX로 볼지 결정이 필요하다.
- `GetSingleSelectedEditorActor()`는 여러 선택 중 첫 actor를 반환한다. 단일 선택만 허용할지, 첫 선택 사용을 명시할지 UX 결정이 필요하다.
- runtime target component를 범용 target/lock-on system으로 확장하는 것은 별도 기능 설계다.

### 하지 않는 것이 나은 것

- Editor plugin이 `CDebugOverlayTargetComponent`를 직접 include해 상태를 직접 변경하는 것.
- nearest radius, selected enemy 판정, non-enemy reject 정책 변경.
- Shipping에서 동작하는 gameplay target command처럼 포장하는 것. Shipping에서는 구현 본문이 no-op 성격임을 유지한다.

## 7. Editor Tooling cleanup 후보

### 바로 해도 안전한 것

`PortfolioDebugOverlayEditorModule.cpp`는 약 678 lines로 커졌고 다음 역할이 한 파일에 모여 있다.

- CVar 접근 helper: `Find/Get/SetDebugOverlay*CVar`, filter 검증.
- PIE command helper: PIE world 찾기, PlayerController `ConsoleCommand`, Outliner 선택 actor 처리.
- Slate widget: `SPortfolioDebugOverlayEditorWidget`.
- menu/toolbar/tab registration.

다음 분리는 Editor-only plugin 내부에서만 수행하면 안전하다.

- `Private/DebugOverlayEditorCVarHelpers.*`
- `Private/DebugOverlayEditorTargetCommands.*`
- `Private/UI/SPortfolioDebugOverlayEditorWidget.*`
- module cpp는 tab/menu/toolbar registration만 유지.

### 별도 설계가 필요한 것

- Editor UI에서 `Portfolio.DebugOverlay.Preset`을 노출할지 여부. 현재 runtime claim이 명확하지 않으므로 보류가 안전하다.
- Outliner 다중 선택 UX.
- command status 문구/UX 개선.

### 하지 않는 것이 나은 것

- runtime `Portfolio.Build.cs`에 Slate/UnrealEd/ToolMenus 의존성을 넣는 것.
- `Portfolio.uproject`나 config 저장을 cleanup PR에 섞는 것.
- Editor Tooling을 Shipping HUD 또는 runtime 기능 확장 evidence로 주장하는 것.

## 8. 리팩터링 금지/보류 항목

| 분류 | 항목 | 이유 |
| --- | --- | --- |
| 금지 | Debug Overlay HUD 표시 정책 변경 | P52/P53 evidence claim 기준이 깨진다. |
| 금지 | Store public API/schema 변경 | 호출부와 evidence 의미 재검증이 필요하다. |
| 금지 | CVar 이름/의미 변경 | Editor Tooling과 운영 문서의 command contract다. |
| 금지 | EventLog filter 의미 변경 | 현재는 display filter이며 event 발생 여부 claim이 아니다. |
| 금지 | `.umap`, `.uasset`, config, `Build.cs`, `Portfolio.uproject` 변경 | 이번 작업 범위 밖이다. |
| 보류 | Runtime LOD actual 구현 | cleanup이 아니라 feature 작업이다. |
| 보류 | BT active node tracking | 현재는 recent AI task event 수준이며 별도 instrumentation 설계가 필요하다. |
| 보류 | EventLog compact/wrapping redesign | 표시 정책 변경 가능성이 있다. |
| 보류 | HUD 파일 대분리 | build 영향과 책임 경계 설계가 필요하다. |
| 보류 | Store role matcher 분리 | subject match 정책과 schema 의미 검토가 필요하다. |

## 9. 권장 cleanup PR 분리안

1. `DebugOverlay runtime low-risk cleanup`
   - 대상: `CDebugOverlayHUD.cpp`, `FDebugOverlaySnapshotStore.cpp`
   - 범위: 내부 helper 추출, 섹션 정리, 반복 제거, naming 보정.
   - 금지: 표시 문자열/순서, Store schema/API, CVar, filter 의미 변경.

2. `DebugOverlay editor module cleanup`
   - 대상: `Plugins/PortfolioDebugOverlayEditor` 내부 source.
   - 범위: CVar helper, target command helper, Slate widget 파일 분리.
   - 금지: runtime module dependency 추가, config 저장, `.uproject` 변경.

3. `DebugOverlay design follow-up`
   - 대상: HUD 파일 분리, Store role matcher 분리, Current AI provider 여부, recent combat fallback helper 처리.
   - 산출물: 구현 전 설계 문서 또는 작은 spike PR.

4. `Runtime LOD overlay feature`
   - 대상: 실제 runtime LOD source 연결.
   - 전제: HUD `N/A` 표시가 실제 값을 읽을 수 있는 경로가 확인되어야 한다.

5. `BT evidence feature`
   - 대상: Behavior Tree active node tracking.
   - 전제: instrumentation 위치와 evidence claim 범위 별도 설계.

## 10. 1차 구현 우선순위

1차 구현은 `DebugOverlay runtime low-risk cleanup`을 권장한다.

우선순위는 다음 순서가 안전하다.

1. HUD visible line copy, line height 계산, Interaction summary block helper 추출.
2. HUD `DrawHUD()` 우측 panel geometry helper 추출.
3. HUD line append 반복 축약.
4. Store helper section 정리와 `Reset()`의 cleanup helper 재사용.
5. Store record path의 frame/time/world resolve 반복 축약.

Editor module 분리는 별도 PR로 두는 것이 좋다. runtime HUD/Store cleanup과 Editor-only file split을 한 PR에 섞으면 검증 포인트가 늘어난다.

## 11. 검증 기준

문서 조사 PR 검증:

- `git diff --check` 통과.
- 변경 파일이 `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_Code_Cleanup_Investigation_KR.md` 하나인지 확인.

다음 runtime cleanup PR 검증:

- `git diff --check` 통과.
- `PortfolioEditor Win64 Development` build 통과.
- TestRoom PIE에서 기존 P52/P53 evidence 흐름 유지.
- `Portfolio.DebugOverlay.Enabled`는 HUD 표시 gate로 유지.
- `Portfolio.DebugOverlay.Collect`는 snapshot/event record gate로 유지.
- `Portfolio.DebugOverlay.EventLogFilter`, `EventLogLimit`, `HideNoiseEvents`, `HideCollisionWindowEvents`는 display semantics 유지.
- Player / Enemy / Interaction / EventLog 표시 순서와 문구 유지.
- `Pannel_01 / Pannel_02 / Pannel_03` 표시 유지.
- EventLog filter로 숨겨진 event를 “발생하지 않음”으로 claim하지 않음.
- `Runtime LOD: N/A`를 actual 표시 성공으로 claim하지 않음.
- Shipping HUD, gameplay HUD, UMG/Slate runtime HUD처럼 표현하지 않음.
- `.umap`, `.uasset`, `Content/*`, `Config/*`, `Portfolio.Build.cs`, `Portfolio.uproject` 변경 없음.

## 12. 사용자 결정이 필요한 보류 질문

다음 항목은 cleanup PR에 넣기 전에 별도 결정이 필요하다.

- 사용되지 않는 `ResolveRecentCombatEnemy()` / `ResolveWorldScanFallbackEnemy()`를 diagnostic 후보로 유지할지, 별도 PR에서 제거할지.
- `GetSingleSelectedEditorActor()`의 다중 선택 동작을 첫 actor 사용으로 유지할지, 단일 선택 강제로 바꿀지.
- Current AI source를 HUD Blackboard read로 유지할지, Store/provider로 이동할지.
- `TryGetSnapshotCopy().RecentEvents`를 raw snapshot으로 유지할지, display filter 적용 snapshot으로 바꿀지.
