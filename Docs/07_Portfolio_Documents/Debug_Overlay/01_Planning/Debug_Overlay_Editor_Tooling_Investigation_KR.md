# Debug Overlay Editor Tooling 조사 및 구현 계획

## 1. 목적

P52에서 완성한 runtime Debug Overlay를 기반으로, UE Editor 편집/툴 제작 역량을 보여줄 수 있는 Editor 전용 tooling 구현 가능성을 조사한다.

목표는 다음 두 가지다.

- Level Editor toolbar/menu에서 Debug Overlay tooling에 접근할 수 있게 한다.
- Nomad 설정 창에서 Debug Overlay CVar를 읽고 변경할 수 있게 한다.

이번 문서는 구현 전 조사/설계 기준이다. 코드 구현, asset/config/Build.cs 변경, `.uasset`/`.umap` 변경은 포함하지 않는다.

## 2. 현재 상태 요약

조사 시점 기준 현재 브랜치는 `main`이며, HEAD는 P52 merge 이후 상태다.

- 현재 프로젝트 module은 `Portfolio` Runtime module 1개다.
- `Portfolio.uproject`에는 별도 Editor module 선언이 없다.
- `Source/Portfolio/Portfolio.Build.cs`는 runtime 의존성 중심이다.
- `Slate`, `SlateCore`, `ToolMenus`, `UnrealEd`, `LevelEditor` 계열 의존성은 runtime module에 활성화되어 있지 않다.
- 루트 `Plugins/`에는 `AssetReferenceInspector` editor plugin 작업물이 있으나 현재 untracked 상태다.

따라서 Editor toolbar, Nomad tab, Slate widget 구현은 runtime module에 직접 넣지 않는 것이 안전하다.

## 3. 프로젝트 모듈 구조 조사

`Portfolio.uproject`의 module 구성은 다음과 같다.

- `Portfolio`
  - Type: `Runtime`
  - LoadingPhase: `Default`
  - Dependencies: `Engine`, `CoreUObject`, `AIModule`

`Source/Portfolio/Portfolio.Build.cs`의 주요 dependency는 다음과 같다.

- `Core`
- `CoreUObject`
- `Engine`
- `InputCore`
- `EnhancedInput`
- `AIModule`
- `Niagara`

Editor UI 구현에 필요한 `Slate`, `SlateCore`, `ToolMenus`, `UnrealEd`, `LevelEditor`를 runtime module에 추가하면 packaged/runtime 경계가 흐려진다.

권장 구조는 새 Editor 전용 plugin이다.

## 4. Plugins 조사 결과

루트 `Plugins/AssetReferenceInspector`는 Editor plugin 구조의 좋은 참고 자료다.

확인된 패턴은 다음과 같다.

- `.uplugin`
  - Type: `Editor`
  - LoadingPhase: `Default`
  - CanContainContent: `false`
- Module lifecycle
  - `StartupModule`
  - `ShutdownModule`
- Editor UI 진입점
  - `UToolMenus::RegisterStartupCallback`
  - Level Editor menu 확장
  - `FGlobalTabmanager::RegisterNomadTabSpawner`
- Slate UI
  - `SCompoundWidget`
  - `SButton`
  - `SCheckBox`
  - `SEditableTextBox`
  - `SGridPanel`
  - `SVerticalBox`
  - `SBorder`
- Command pattern
  - `TCommands`
  - `UI_COMMAND`
  - `FUICommandList`

다만 현재 plugin은 untracked이므로 이번 작업에서 add/commit하지 않는다. Debug Overlay tooling은 이 plugin을 직접 확장하기보다 별도 plugin으로 만드는 것이 적절하다.

## 5. Debug Overlay Runtime 제어점

현재 runtime Debug Overlay는 CVar와 Exec command를 통해 제어된다.

### 5.1 CVar 후보

Nomad settings panel에 1차로 노출하기 좋은 CVar는 다음이다.

| CVar | 용도 | Editor UI 후보 |
| --- | --- | --- |
| `Portfolio.DebugOverlay.Enabled` | overlay 표시 on/off | checkbox |
| `Portfolio.DebugOverlay.Collect` | snapshot/event 수집 on/off | checkbox |
| `Portfolio.DebugOverlay.EventLogFilter` | EventLog category filter | combo box |
| `Portfolio.DebugOverlay.EventLogLimit` | EventLog 표시 줄 수 | numeric input/slider |
| `Portfolio.DebugOverlay.HideNoiseEvents` | reject/ignore noise 숨김 | checkbox |
| `Portfolio.DebugOverlay.HideCollisionWindowEvents` | collision lifecycle event 숨김 | checkbox |

`Portfolio.DebugOverlay.Preset`은 CVar가 존재하지만 현재 runtime에서 명확한 preset 동작을 성공 evidence로 주장하기 어렵다. 1차 Editor UI에서는 노출하지 않는 것이 안전하다.

### 5.2 PIE 중 반영성

- `Enabled`는 HUD draw 여부에 즉시 반영된다.
- `EventLogFilter`, `EventLogLimit`, `HideNoiseEvents`, `HideCollisionWindowEvents`는 HUD query/display 단계에서 읽히므로 PIE 중 변경해도 표시가 갱신된다.
- `Collect`는 이후 발생하는 snapshot/event 수집에 영향을 준다. 꺼져 있던 동안의 event가 사후 복구되는 것은 아니다.

### 5.3 Exec command 후보

현재 PlayerController에는 debug overlay target 관련 Exec command가 있다.

- `DebugOverlaySelectNearestFocus`
- `DebugOverlayClearFocus`

이들은 CVar가 아니라 PIE world의 PlayerController를 통해 실행해야 하는 command다. 1차 settings panel 핵심 범위에서는 제외하고, 후속으로 PIE 전용 action button 후보로 둔다.

## 6. Toolbar Button 구현 가능성

UE 5.4 기준으로 Level Editor toolbar/menu 확장은 `UToolMenus` 기반 구현이 적절하다.

1차 권장 동작은 다음이다.

- toolbar/menu button을 눌러 Debug Overlay Nomad settings panel을 연다.
- 선택적으로 작은 quick toggle을 추가해 `Portfolio.DebugOverlay.Enabled`만 즉시 전환한다.

단, toolbar button이 runtime gameplay feature처럼 보이면 안 된다. Editor-only debug tooling 진입점임을 명확히 해야 한다.

## 7. Nomad Settings Panel 구현 가능성

Nomad 창은 다음 구조로 구현 가능하다.

- `FGlobalTabmanager::RegisterNomadTabSpawner`
- `SDockTab`
- `SCompoundWidget` 기반 `SDebugOverlaySettingsWidget`
- `IConsoleManager::Get().FindConsoleVariable(...)` 기반 CVar read/write helper

UI 구성 후보는 다음이다.

- Enabled checkbox
- Collect checkbox
- EventLogFilter combo box
  - `All`
  - `Execution`
  - `Combat`
  - `AI`
- EventLogLimit numeric input
  - 범위: `0~32`
- HideNoiseEvents checkbox
- HideCollisionWindowEvents checkbox
- Refresh button
- Reset to evidence-friendly values button

`ResetAll`, focus clear, select nearest 같은 state-changing action은 후속 범위로 두는 편이 안전하다.

## 8. 권장 아키텍처

1차 권장안은 새 Editor 전용 plugin이다.

예상 위치:

```text
Plugins/PortfolioDebugOverlayEditor/
```

예상 파일:

```text
Plugins/PortfolioDebugOverlayEditor/PortfolioDebugOverlayEditor.uplugin
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/PortfolioDebugOverlayEditor.Build.cs
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Public/PortfolioDebugOverlayEditorModule.h
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/PortfolioDebugOverlayEditorModule.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/DebugOverlayEditorCommands.h
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/DebugOverlayEditorCommands.cpp
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/UI/SDebugOverlaySettingsWidget.h
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/Private/UI/SDebugOverlaySettingsWidget.cpp
```

예상 module dependency:

- Public
  - `Core`
- Private
  - `CoreUObject`
  - `Engine`
  - `InputCore`
  - `Slate`
  - `SlateCore`
  - `ToolMenus`
  - `Projects`
  - 필요 시 `LevelEditor`, `UnrealEd`

Runtime `Portfolio` module dependency는 최소화한다. 단순 CVar 제어만 한다면 `Portfolio` module을 직접 의존하지 않아도 된다.

## 9. 대안 검토

### 9.1 프로젝트 Editor module

예상 위치:

```text
Source/PortfolioEditor/
```

장점:

- 프로젝트 내부 module로 관리된다.
- plugin enable/disable 고려가 줄어든다.

단점:

- `.uproject` module 선언 변경이 필요하다.
- 포트폴리오에서 독립 Editor Tooling을 보여주는 효과는 plugin보다 약하다.

### 9.2 Runtime module에 WITH_EDITOR 코드 추가

비권장한다.

이유:

- Runtime module에 Editor UI dependency가 섞인다.
- Shipping/runtime 경계 설명이 어려워진다.
- P52에서 정리한 debug-only claim과 충돌할 수 있다.

## 10. 1차 구현 계획 후보

다음 PR 또는 작업 단위에서 구현할 수 있는 최소 범위는 다음이다.

1. 새 Editor plugin scaffold 작성
   - `.uplugin`
   - Editor module Build.cs
   - `StartupModule` / `ShutdownModule`
2. Level Editor menu/toolbar 진입점 추가
   - button 또는 menu item
   - Nomad settings panel open
3. Nomad settings panel 작성
   - CVar read/write helper
   - Enabled / Collect checkbox
   - EventLogFilter combo
   - EventLogLimit numeric input
   - HideNoiseEvents / HideCollisionWindowEvents checkbox
4. PIE 수동 검증
   - PIE 전 설정 변경
   - PIE 중 설정 변경
   - HUD 즉시 반영 확인
5. 문서 갱신
   - operation guide
   - editor tooling checklist

속도 우선이면 1~3을 한 작업으로 묶어도 된다. 다만 focus selection button, reset button, preset 저장은 후속 작업으로 분리하는 것이 좋다.

## 11. 검증 기준

1차 구현 시 검증 기준은 다음이다.

- Editor module/plugin이 Editor build에서만 로드된다.
- packaged/runtime build claim을 하지 않는다.
- toolbar/menu button으로 Nomad 창을 열 수 있다.
- Nomad 창에서 CVar 값을 읽을 수 있다.
- Nomad 창에서 CVar 값을 바꾸면 PIE 중 HUD 표시가 반영된다.
- `EventLogFilter`는 `All / Execution / Combat / AI`만 제공한다.
- `EventLogLimit`는 `0~32` 범위를 벗어나지 않는다.
- `Collect` off 상태가 과거 event를 삭제하거나 재생성하는 것으로 설명되지 않는다.
- `Preset`, Runtime LOD actual, BT active node tracking은 성공 claim에서 제외한다.

## 12. 리스크와 결정 필요 항목

구현 전 사용자 결정이 필요한 항목은 다음이다.

- 새 Editor plugin으로 진행할지, 프로젝트 Editor module로 진행할지
- toolbar button의 1차 기능을 창 열기로 둘지, enabled toggle까지 포함할지
- settings 값을 CVar session 상태로만 둘지, config 저장까지 할지
- evidence-friendly preset button을 1차에 넣을지
- target select/clear button을 1차에 넣을지
- `Plugins/AssetReferenceInspector`는 계속 참고용 untracked로 둘지, 별도 PR에서 정리할지

권장 결정:

- 새 Editor plugin으로 진행한다.
- toolbar/menu button은 Nomad 창 열기를 기본으로 한다.
- CVar 설정은 session-only로 시작한다.
- target select/clear, reset, preset 저장은 후속 작업으로 둔다.

## 13. 제외 범위

이번 Editor Tooling 1차 구현 계획에서 제외할 항목은 다음이다.

- Runtime Debug Overlay 동작 변경
- Store schema/API 리팩터링
- Runtime LOD actual 표시 구현
- BT active node tracking 구현
- combat target flow 변경
- Shipping HUD claim
- `.umap`, `.uasset`, config 변경
- 기존 P52 evidence screenshot 재촬영
- Debug Overlay 코드 클린

## 14. 후속 코드 클린 분리 기준

Debug Overlay runtime 코드 클린은 Editor Tooling과 별도 브랜치에서 처리한다.

분리 후보:

- `CDebugOverlayHUD.cpp` panel/layout helper 분리
- EventLog formatting helper 분리
- Store query/format responsibility 정리
- Snapshot schema/API 재검토
- Editor Tooling과 runtime overlay 사이 bridge helper 정리

Editor Tooling 1차 작업은 runtime 구조를 크게 건드리지 않고 CVar bridge 중심으로 구현하는 것이 적절하다.

## 15. 최종 권장안

P52 이후 다음 구현 작업은 `PortfolioDebugOverlayEditor` 새 Editor plugin으로 시작하는 것을 권장한다.

1차 구현 목표는 다음으로 제한한다.

- Level Editor toolbar/menu button
- Debug Overlay Nomad settings panel
- Debug Overlay CVar read/write UI
- PIE 중 설정 반영 검증

이 범위는 editor tooling 역량을 보여주기에 충분하고, runtime debug overlay의 안정성을 흔들지 않는다.
