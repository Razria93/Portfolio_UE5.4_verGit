# Debug Overlay P1 Overlay Layout Style Lock

## 1. 목적

이 문서는 P1 debug overlay에서 EventLog separate panel 적용 이후 사용자가 수동으로 맞춘 HUD layout/style 값을 이후 작업 기준으로 고정한다.

이후 기능 구현 중 panel 폭, title 문구, header 위치, 색상, padding, left/right panel 역할이 임의로 흔들리지 않게 하는 것이 목적이다.

이번 문서는 layout 정책 고정 문서이며, 코드 구현이나 촬영/패키징 문서가 아니다.

## 2. 현재 Panel 역할

### 2.1 Left Panel

Left panel은 actor current state와 recent summary 확인 영역이다.

현재 역할:

- Player current state
- Player Recent Execution
- Enemy current state
- Enemy Recent Execution
- Interaction Recent Execution
- Interaction Recent Combat
- Interaction Recent AI

Left panel에는 full EventLog를 다시 넣지 않는다. EventLog는 별도 right panel에서만 표시한다.

### 2.2 Right EventLog Panel

Right panel은 EventLog 전용 영역이다.

현재 역할:

- EventLog category filter 결과 표시
- EventLog limit 결과 표시
- noise/collision filter 결과 표시
- world interaction timeline 확인

Right panel은 EventLog 전용 panel이며, Player/Enemy current state나 Recent summary를 넣지 않는다.

## 3. Title Convention

현재 overlay title convention은 다음 값을 기준으로 유지한다.

```text
[Debug Overlay Pannel_01]
[Debug Overlay Pannel_02]
```

- `[Debug Overlay Pannel_01]`: left panel title
- `[Debug Overlay Pannel_02]`: right EventLog panel title

`Pannel` spelling을 포함해 현재 표시값을 그대로 유지한다. title 문구 변경은 별도 사용자 요청이 있을 때만 수행한다.

## 4. Vertical Alignment 기준

두 panel은 같은 top margin 체계를 공유한다.

정렬 기준:

- left panel title line과 right panel title line은 같은 높이에 온다.
- left panel의 `[Player]` header bar와 right panel의 `[Event Log: ...]` header bar는 같은 높이에 온다.
- 두 panel 모두 title line이 위에 있고, 그 아래 header bar가 정렬된다.
- right EventLog panel이 화면 최상단에 붙지 않는다.
- title/header 사이 공백과 header/first content 사이 간격은 현재 사용자 조정값을 기준으로 유지한다.

즉, EventLog panel은 left panel의 title line을 건너뛰는 구조가 아니라, left panel과 같은 title/header 계층을 가진 독립 panel로 본다.

## 5. 현재 코드 기준 Layout 값

문서 작성 시점에 `CDebugOverlayHUD.cpp`에서 확인한 주요 layout/style 값은 다음과 같다.

```cpp
DebugOverlayOriginX = 24.f
DebugOverlayOriginY = 36.f
DebugOverlayLineHeight = 20.f
DebugOverlayFontScale = 1.05f
DebugOverlayBackgroundPadding = 10.f
DebugOverlayBackgroundWidth = 700.f
DebugOverlayHeaderBottomPadding = 5.f
DebugOverlayPanelGap = 24.f
DebugOverlayRightMargin = 24.f
DebugOverlayBottomMargin = 24.f
DebugOverlayMinEventLogPanelWidth = 420.f
```

이 값들은 사용자가 현재 화면을 보며 맞춘 layout 기준으로 취급한다.

향후 기능 구현에서 위 값을 바꾸어야 한다면, 기능 구현에 섞지 말고 별도 layout tuning 작업으로 분리한다.

## 6. Width / Spacing 기준

현재 width/spacing 정책은 다음과 같다.

- left panel width는 현재 사용자 조정값을 기준으로 유지한다.
- EventLog panel X는 left panel 오른쪽 + panel gap 기준으로 둔다.
- EventLog panel Y는 left panel background Y와 맞춘다.
- EventLog panel width는 viewport 오른쪽 여백을 제외한 남은 공간을 사용한다.
- panel gap, top margin, right margin, bottom margin, header bottom padding은 임의 변경하지 않는다.
- EventLog가 별도 panel로 분리되었으므로 left panel 폭을 다시 넓히지 않는다.

기능 추가로 left panel 내용이 늘어나는 경우에도 먼저 표시 범위/line count/summary format을 조정하고, panel width 변경은 마지막 수단으로 둔다.

## 7. Color / Section Style 기준

현재 header color 역할은 다음과 같이 유지한다.

- Player header: blue 계열
- Enemy header: red 계열
- Interaction header: 사용자 조정 purple 계열
- EventLog header: gray 계열
- panel background: dark translucent background

문서 작성 시점 코드 기준 색상:

```cpp
DebugOverlayBackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.72f)
DebugOverlayPlayerHeaderColor = FLinearColor(0.02f, 0.20f, 0.78f, 0.68f)
DebugOverlayEnemyHeaderColor = FLinearColor(0.78f, 0.06f, 0.04f, 0.68f)
DebugOverlayInteractionHeaderColor = FLinearColor(0.2f, 0.08f, 0.35f, 0.68f)
DebugOverlayDefaultHeaderColor = FLinearColor(0.24f, 0.24f, 0.24f, 0.72f)
```

색상 변경은 기능 구현에 포함하지 않는다. 색상 변경이 필요하면 별도 UI tuning 작업으로 분리한다.

## 8. 변경 금지 기준

이후 debug overlay 기능 구현에서 다음 항목은 사용자 명시 요청 없이 변경하지 않는다.

- panel width
- panel title
- header color
- title/header vertical alignment
- panel gap
- header padding
- left/right panel 역할
- EventLog separate panel 구조

기능 추가가 필요한 경우에도 기존 layout 값을 보존하는 방향으로 구현한다.

## 9. 예외

다음 경우에는 layout 값을 변경할 수 있다.

- 사용자가 명시적으로 layout tuning을 요청한 경우
- viewport 크기 때문에 panel이 깨져 최소 표시 폭을 유지할 수 없는 경우
- 새 evidence 요구사항 때문에 기존 panel 역할 자체를 재정의해야 하는 경우

단, 이 경우에도 코드 변경 전에 변경 이유와 대상 값을 먼저 보고한다.

## 10. 후속 작업 연결

다음 작업 후보:

1. EventLog Separate Panel PIE 검증 결과 문서화
2. Recent AI 의미/표시 개선 설계
3. Enemy current `AI: NotCaptured` 제거 또는 대체
4. CollisionDisabledIgnored EventLog noise 이슈 재검토

이후 작업에서는 이 문서의 layout/style lock을 기준으로 삼고, 기능 구현 중 HUD 양식값을 임의로 바꾸지 않는다.
