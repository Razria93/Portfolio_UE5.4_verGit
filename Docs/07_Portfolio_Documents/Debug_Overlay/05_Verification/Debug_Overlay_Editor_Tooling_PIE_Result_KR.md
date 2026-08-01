# Debug Overlay Editor Tooling PIE 검증 결과

## 1. 목적

이 문서는 `PortfolioDebugOverlayEditor` Editor-only plugin이 P52 Debug Overlay runtime 기능을 Editor UI에서 조작할 수 있는지 확인한 PIE 검증 결과를 기록한다.

검증 대상은 UE Editor Tooling 역량을 보여주는 Level Editor 진입점, Nomad 설정 패널, CVar read/write UI, 기존 target command 호출 흐름이다. Shipping HUD나 runtime 기능 확장 claim으로 사용하지 않는다.

## 2. 검증 전제

| 항목 | 내용 |
| --- | --- |
| 브랜치 | `main` |
| Plugin | `Plugins/PortfolioDebugOverlayEditor` |
| Plugin 성격 | Editor-only plugin |
| Runtime Debug Overlay | P52에서 구현된 기존 runtime 기능 사용 |
| Runtime 코드 변경 | 없음 |
| 제외 변경 | `Portfolio.Build.cs`, `.uproject`, `.umap`, `.uasset`, config 변경 없음 |
| 확인 방식 | 사용자가 PIE에서 Nomad 패널 조작 및 target command 동작을 수동 확인 |

## 3. Editor Tooling 구조

- Level Editor 메뉴 진입점에서 `Debug Overlay` Nomad tab을 연다.
- Level Editor 상단 toolbar 버튼에서도 동일한 `Debug Overlay` Nomad tab을 연다.
- Nomad 패널은 `IConsoleManager`를 통해 Debug Overlay CVar를 읽고 쓴다.
- 설정은 session-only이며 config 저장을 하지 않는다.
- Target 조작 버튼은 기존 console command를 PIE PlayerController 경로로 호출한다.
- Editor plugin이 `TargetComponent`나 runtime HUD/Store를 직접 조작하지 않는다.

## 4. Toolbar / Menu 진입점 검증 결과

| 진입점 | 검증 결과 |
| --- | --- |
| `창 > Portfolio Tools > Debug Overlay` | 프로젝트 전용 menu entry가 유지되고 Nomad panel이 열린다. |
| Level Editor toolbar button | 상단 toolbar에 `Debug Overlay` 버튼이 표시된다. |
| Toolbar click | 버튼 클릭 시 기존 `Debug Overlay` Nomad panel이 열린다. |
| Nomad panel | CVar UI와 Target command 버튼이 유지된다. |

toolbar button은 panel open 진입점이다. overlay enable/disable direct toggle, target command direct button, preset 저장 기능으로 해석하지 않는다.

## 5. 대표 스크린샷

| 파일 | 확인 내용 |
| --- | --- |
| [debug_overlay_editor_tooling_01_toolbar_button.jpg](../../../98_Evidence/01_Screenshot/DebugOverlay/EditorTooling/debug_overlay_editor_tooling_01_toolbar_button.jpg) | Level Editor toolbar의 `Debug Overlay` 버튼과 tooltip, Nomad panel open 상태 |
| [debug_overlay_editor_tooling_02_nomad_panel_target_select.jpg](../../../98_Evidence/01_Screenshot/DebugOverlay/EditorTooling/debug_overlay_editor_tooling_02_nomad_panel_target_select.jpg) | Nomad panel의 CVar UI / Target command 섹션과 `Select Nearest Target` 실행 확인 |
| [debug_overlay_editor_tooling_03_target_clear_filters.jpg](../../../98_Evidence/01_Screenshot/DebugOverlay/EditorTooling/debug_overlay_editor_tooling_03_target_clear_filters.jpg) | EventLog filter 관련 UI와 `Clear Target` 실행 상태 확인 |

대표 스크린샷은 Editor Tooling 검증용이다. Shipping HUD, config 저장, preset 저장, runtime target system 변경 claim으로 사용하지 않는다.

## 6. CVar 조작 검증 결과

| UI 항목 | 대상 CVar | 검증 결과 |
| --- | --- | --- |
| Enabled | `Portfolio.DebugOverlay.Enabled` | PIE 중 HUD 표시 on/off 반영 확인 |
| Collect | `Portfolio.DebugOverlay.Collect` | PIE 중 수집 on/off 반영 확인 |
| EventLogFilter | `Portfolio.DebugOverlay.EventLogFilter` | `All`, `Execution`, `Combat`, `AI` 선택 및 HUD 반영 확인 |
| EventLogLimit | `Portfolio.DebugOverlay.EventLogLimit` | `0~32` 범위 조작 및 `Limit=0` 표시 정책 확인 |
| HideNoiseEvents | `Portfolio.DebugOverlay.HideNoiseEvents` | noise event 표시 제어 반영 확인 |
| HideCollisionWindowEvents | `Portfolio.DebugOverlay.HideCollisionWindowEvents` | collision window event 표시 제어 반영 확인 |

## 7. Target Command 버튼 검증 결과

### Select Nearest Target

- 호출 command: `DebugOverlaySelectNearestTarget`
- PIE 중 기존 runtime command 경로로 실행된다.
- HUD에서 `EnemySource: TargetComponent.Nearest` 또는 `EnemySelect` nearest diagnostic으로 결과를 확인한다.
- Editor plugin은 nearest 탐색 로직을 직접 구현하지 않는다.

### Clear Target

- 호출 command: `DebugOverlayClearTarget`
- PIE 중 기존 runtime command 경로로 실행된다.
- HUD에서 `EnemySource: None`으로 target clear 결과를 확인한다.
- Editor plugin은 `TargetComponent`를 직접 clear하지 않는다.

### 제외 command

- line trace 기반 `DebugOverlaySelectTarget`은 복구하지 않는다.
- Target selection 성공 조건은 P52 runtime 정책을 그대로 따른다.

## 8. Refresh 동작

`Refresh`는 현재 CVar 값을 UI에 다시 읽어오는 보조 UI 동기화 기능이다.

이 항목은 runtime 기능 성공 evidence가 아니며, 검증 결과에서도 Debug Overlay 기능 claim으로 과장하지 않는다.

## 9. 실패/안전 동작

PIE world 또는 PlayerController를 찾지 못하면 command를 실행하지 않고 상태 메시지를 표시해야 한다.

- PIE world 없음: `PIE world not available`
- PlayerController 없음: `PlayerController not available`

이 경로는 Editor button의 안전 실패 처리이며, target selection 성공 evidence로 사용하지 않는다.

## 10. 성공 판단

- `창 > Portfolio Tools > Debug Overlay` menu entry와 Level Editor toolbar button 양쪽에서 Nomad panel을 열 수 있다.
- Editor Nomad 패널에서 Debug Overlay 주요 CVar를 조작할 수 있다.
- PIE 중 CVar 변경이 runtime HUD에 반영된다.
- Target command 버튼은 기존 runtime console command 경로로 동작한다.
- Editor-only plugin과 runtime Debug Overlay의 책임 경계가 유지된다.
- runtime Debug Overlay 코드, `Portfolio.Build.cs`, `.uproject`, asset/config 변경 없이 Editor Tooling을 추가했다.

## 11. 보류 항목

- target command 추가 확장
- preset 저장
- config 저장
- Runtime LOD actual 표시
- BT active node tracking
- runtime Debug Overlay 코드 클린
- Editor Tooling PR 정리
