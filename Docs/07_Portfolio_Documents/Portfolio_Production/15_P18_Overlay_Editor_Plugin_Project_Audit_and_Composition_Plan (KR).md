# 문서상 p.21 Overlay Editor Plugin 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.21에 적용한다.

## 1. 페이지 목적

이 페이지는 p.20 Runtime Debug의 표시·관찰 기능을 반복하지 않는다. `PortfolioDebugOverlayEditor`가 p.20 관찰에 필요한 표시 조건과 Focus 요청을 한 Editor Panel에서 준비하되, 런타임 상태의 소유권을 가져가지 않는 **Editor-only bridge**임을 설명한다.

## 2. 코드·문서 감사 결과

1. `PortfolioDebugOverlayEditor.uplugin`의 module type은 `Editor`이며 description도 Debug Overlay CVar의 Editor-only control로 한정한다.
2. Toolbar와 Level Editor Menu는 동일한 Nomad tab을 여는 진입점이다. 직접 overlay toggle이나 focus command 실행 버튼으로 과장하지 않는다.
3. `SPortfolioDebugOverlayEditorWidget`은 `IConsoleManager`를 통해 CVar를 읽고 쓴다. 설정은 session-only이며 config에 저장하지 않는다.
4. Nearest·Outliner·Clear action은 기존 PIE PlayerController console command를 호출한다. Editor plugin은 FocusComponent, runtime HUD, Snapshot Store를 직접 조작하지 않는다.
5. PIE world 또는 PlayerController가 없을 때 command를 실행하지 않고 상태 메시지를 남기는 안전 실패 경로가 있다.

## 3. FinalCandidate 증거

| 증거 | 확인 가능한 내용 | 페이지 사용 범위 |
| --- | --- | --- |
| `debug_overlay_editor_tooling_01_toolbar_button.jpg` | Toolbar button, Nomad panel open | Editor 진입점 |
| `debug_overlay_editor_tooling_02_nomad_panel_target_select.jpg` | CVar UI, focus command, Select Nearest Target | session UI와 기존 command 실행 |
| `debug_overlay_editor_tooling_04_outliner_target_selection.jpg` | Outliner actor 선택, OutlinerFocus 결과 | 선택 actor 전달과 기존 command 위임 |

## 4. 금지 표현

- Gameplay UI, Shipping HUD, runtime 기능 확장처럼 소개하지 않는다.
- Toolbar가 overlay enable/direct toggle 또는 preset 저장 기능이라고 쓰지 않는다.
- Editor plugin이 target 탐색·FocusComponent·HUD·Store를 직접 소유하거나 갱신한다고 쓰지 않는다.
- config/preset 영속 저장을 주장하지 않는다.

## 5. 페이지 구성

1. **요구조건·의도**: 표시 조건과 Focus 대상이 모두 있어야 p.20의 Character Details·Focused Enemy Event Scope를 같은 대상 기준으로 읽을 수 있음을 먼저 둔다. 반복 콘솔 입력의 정량 절감은 주장하지 않는다.
2. **구현 구조**: `Toolbar / Menu → Nomad Panel`에서 `등록된 CVar → IConsoleManager`와 `Focus Actions → PIE PlayerController Console Command → 기존 Runtime command`로 갈라지는 Editor Control Bridge 다이어그램 슬롯을 둔다. Plugin에서 HUD·Snapshot Store·FocusComponent로 직접 향하는 화살표는 두지 않는다.
3. **실사용 검증**: Nomad Panel의 session-only CVar 설정 화면 한 장을 중심 증거로 사용한다. 사진 주변에는 session-only 설정, Overlay Options, Event Log 범위를 설명한다. Focus command는 구현 구조 다이어그램에서 기존 Runtime command 위임으로만 다룬다.
4. **사용 범위**: Editor의 요청 역할과 Runtime의 Focus 적용·Snapshot 기록·Overlay 표시를 분리하고, session-only·runtime 상태 미소유·Shipping 비기능화를 고정한다.

## 6. 독립 사전 검토 반영

- p.20의 Runtime Overlay 화면·관찰 값을 반복하지 않는다.
- `PIE fail-safe`는 PIE world/PlayerController가 없을 때 command를 실행하지 않고 실패 상태를 표시하는 실제 동작으로 구체화한다.
- Focus는 항상 기존 command 위임으로만 표기한다.

## 7. 검수 상태

- 코드·문서·증거 범위 감사: 완료
- HTML 구성 반영: 완료 — Tooling 공통 형식(요구조건·의도 → 구현 구조 → 실사용 검증)으로 전환
- 증거 상태: Ready — Nomad Panel session-only CVar UI와 Editor Control Bridge 다이어그램 반영
- PDF 레이아웃 검증: 보완 후 별도 수행 필요
