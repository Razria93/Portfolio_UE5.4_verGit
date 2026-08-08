# Debug Overlay P1 EventLog Separate Panel Design

## 1. 목적

이 문서는 P1 debug overlay에서 EventLog를 왼쪽 Interaction 패널에서 분리해 별도 패널로 표시하는 정책을 고정한다.

현재 overlay는 Player, Enemy, Interaction, Recent, EventLog가 모두 왼쪽 패널에 쌓인다. 이 구조는 `EventLogLimit`이 작을 때는 동작하지만, Recent block이 여러 줄로 확장되고 EventLog line 수가 늘어나면 화면 하단을 넘어가거나 긴 log line이 좌측 패널 폭에 잘린다.

이번 설계의 목표는 EventLog를 흐름 추적 전용 패널로 분리하고, 왼쪽 패널은 actor current state와 recent summary를 빠르게 읽는 영역으로 유지하는 것이다.

## 2. 문제 배경

현재 구조의 문제는 다음과 같다.

- Player / Enemy / Interaction / Recent / EventLog가 모두 왼쪽 패널에 쌓이면 세로 공간이 부족하다.
- `Portfolio.DebugOverlay.EventLogLimit`을 늘리면 EventLog가 화면 밖으로 밀린다.
- EventLog line은 길어서 좌측 좁은 패널에서는 가독성이 떨어진다.
- EventLog는 world interaction flow를 시간 순서로 보는 용도이고, Recent block은 빠른 상태 확인용이다.
- 두 기능을 같은 좁은 column에 둘 경우 둘 다 읽기 어려워진다.

따라서 EventLog는 Interaction의 의미를 유지하되, 화면 배치는 Interaction 패널 내부가 아니라 별도 EventLog 패널로 분리한다.

이 문서는 기존 `Debug_Overlay_P1_Interaction_Event_Flow_Redesign_Plan_KR.md`의 의미론을 유지하되, EventLog 표시 위치 정책만 대체한다. 즉, EventLog는 여전히 Interaction timeline을 설명하는 world-level log이지만, HUD 배치상으로는 Interaction block 안에 포함하지 않는다.

## 3. 최종 방향

P1의 권장 방향은 다음과 같다.

- 왼쪽 패널은 Player / Enemy / Interaction recent summary 중심으로 유지한다.
- EventLog는 별도 패널로 분리한다.
- EventLog 패널은 상단 중앙부터 우측 영역까지 사용한다.
- EventLog는 최종 촬영용 HUD가 아니라 debug evidence용 overlay로 유지한다.
- EventLog filter, noise filter, collision window filter의 의미는 유지한다.
- EventLog record path와 Store ring buffer 구조는 변경하지 않는다.

## 4. 권장 화면 구조

```text
[Left Panel]
[Debug Overlay P0.5]

[Player]
State:
Action:
Reaction:
HP:
Stagger:
Guard:
Movement:
Runtime LOD:

[Recent Execution]
...

[Enemy]
EnemyFocusMode:
EnemyFocusActor:
EnemyFocusCommand:

State:
Action:
Reaction:
HP:
Stagger:
Guard:
Movement:
Runtime LOD:

[Recent Execution]
...
[Recent AI]
...

[Interaction]
[Recent Execution]
...
[Recent Combat]
...
```

```text
[EventLog Panel]
[Event Log: All]
...
```

Interaction 패널은 world-level recent summary를 유지한다. EventLog는 Interaction timeline의 일부라는 의미를 유지하지만, 실제 표시 위치는 별도 EventLog 패널로 이동한다.

## 5. EventLog 패널 배치 기준

EventLog 패널은 다음 기준으로 배치한다.

- X 시작점은 left panel 오른쪽 여백 이후로 둔다.
- Y 시작점은 화면 상단 여백 이후로 둔다.
- Width는 화면 오른쪽 끝 여백까지 최대한 사용한다.
- Height는 표시 line 수에 따라 증가하되 viewport 하단을 넘지 않도록 max height를 둔다.
- editor console/output log가 켜져 있어도 overlay 자체의 배치가 incoherent하게 겹치지 않아야 한다.
- viewport 폭이 줄어들면 EventLog 패널 width를 줄이되, 왼쪽 패널과 겹치지 않도록 한다.

구현 기준은 다음처럼 고정한다.

- left panel의 기존 폭은 이번 단계에서 축소하지 않는다.
- EventLog panel X는 `LeftPanelX + LeftPanelWidth + PanelGap` 기준으로 계산한다.
- EventLog panel Y는 left panel과 같은 top margin을 사용한다.
- EventLog panel Width는 `ViewportWidth - EventLogPanelX - RightMargin` 기준으로 계산한다.
- EventLog panel Height는 `HeaderLine + EventLogLimit` 기준으로 계산하되, `ViewportHeight - TopMargin - BottomMargin`을 넘지 않는다.
- EventLog panel Width가 최소 표시 폭보다 작아지는 viewport에서는 EventLog panel을 그리지 않고 left panel만 유지하는 것을 허용한다.
- EventLog panel background는 left panel background와 별도로 그린다.
- EventLog header highlight는 기존 `[Event Log: Filter]` header 표현을 유지하되, left panel의 Interaction header와 별도 배경 영역에 속한다.

권장 개념 배치는 다음과 같다.

```text
| Left Panel | gap | EventLog Panel ---------------------- |
|            |     | [Event Log: Filter]                   |
|            |     | line 1                                |
|            |     | line 2                                |
|            |     | ...                                   |
```

## 6. EventLogLimit 정책

기존 CVar를 계속 사용한다.

```text
Portfolio.DebugOverlay.EventLogLimit
```

정책:

- 기본값은 현행 유지 또는 `5`로 유지한다.
- 허용 범위는 `0~32`로 확장한다.
- `0`은 event line 숨김 상태로 유지한다.
- `NoEvents(Filter=...)` 표현은 유지한다.
- `NoEvents(Filter=... Limit=0)` 표현은 유지한다.

`EventLogLimit`을 늘리는 것은 EventLog 패널에서만 의미 있게 동작해야 한다. 왼쪽 패널 높이를 밀어내는 방식으로 동작하면 안 된다.

구현 시 CVar clamp와 help text도 같은 기준으로 갱신한다. 기존 코드에 `0~5` 또는 `0-5` 설명이 남아 있으면 `0~32` 또는 `0-32`로 정리한다.

## 7. Text 처리 정책

P1에서는 EventLog line wrapping을 구현하지 않는다.

우선순위:

1. EventLog 패널 폭을 충분히 확보한다.
2. line은 가능한 한 한 줄로 표시한다.
3. 너무 긴 line의 compact/format 재작업은 후속 후보로 둔다.
4. clipping이 필요하면 EventLog 패널 내부에서만 일어나야 하며, left panel layout을 밀어내면 안 된다.

EventLog line 자체를 의미 단위로 재설계하는 작업은 이번 범위가 아니다.

## 8. 유지할 기능

다음 기능은 유지한다.

- `Portfolio.DebugOverlay.EventLogFilter`
  - `All`
  - `Execution`
  - `Combat`
  - `AI`
- `Portfolio.DebugOverlay.HideNoiseEvents`
- `Portfolio.DebugOverlay.HideCollisionWindowEvents`
- Store ring buffer 기준 최신순 조회
- category filter -> noise/collision filter -> limit 적용 순서
- `NoEvents(Filter=...)`
- `NoEvents(Filter=... Limit=0)`

## 9. 제외 범위

이번 설계에서 하지 않는 작업은 다음과 같다.

- 코드 구현
- EventLog schema 변경
- Store public API 변경
- EventLog 의미 재설계
- Player/Enemy EventLog 분리 재도입
- AI snapshot 구현
- Runtime LOD actual 표시
- CollisionDisabledIgnored filter 문제 해결
- `.umap`, `.uasset`, config, `Build.cs` 변경
- 최종 촬영/패키징

## 10. 구현 전 체크리스트

구현 단계에서는 다음을 확인한다.

- left panel과 EventLog panel의 좌표/폭/높이 상수를 분리한다.
- EventLogLimit clamp 범위를 `0~32`로 확장한다.
- 기존 EventLog filter/helper를 재사용한다.
- left panel lines에는 `AppendEventLogBlock` 계열 호출을 넣지 않는다.
- EventLog line은 별도 `eventLogLines` 또는 동등한 별도 line buffer에만 추가한다.
- EventLog가 Interaction panel에 중복 표시되지 않도록 한다.
- EventLog panel background를 별도로 그린다.
- viewport가 작을 때 left panel과 EventLog panel이 incoherent하게 겹치지 않도록 한다.
- `EventLogLimit=0`과 filter 결과 없음 표시가 유지되는지 확인한다.

## 11. 다음 구현 단계

다음 작업은 `P1 EventLog Separate Panel HUD 구현`이다.

권장 구현 범위:

- `CDebugOverlayHUD.cpp`에서 left panel lines와 EventLog panel lines를 분리한다.
- EventLog block을 별도 draw path로 이동한다.
- EventLog panel background를 별도로 그린다.
- `EventLogLimit` clamp를 `0~32`로 확장한다.
- Interaction panel 내부 EventLog 중복 표시를 제거한다.
- 빌드 및 PIE 수동 확인을 수행한다.

권장 커밋 메시지:

```text
feat(debug): split event log into separate panel
```
