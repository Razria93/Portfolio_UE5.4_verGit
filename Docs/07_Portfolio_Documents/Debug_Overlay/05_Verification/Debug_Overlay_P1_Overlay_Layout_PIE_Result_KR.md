# Debug Overlay P1 Overlay Layout PIE Result

## 1. 목적

이 문서는 P1 debug overlay의 현재 HUD layout이 PIE에서 의도대로 표시되는지 확인한 결과를 기록한다.

이번 결과는 최종 제출용 캡처가 아니라, P1 기능 구현 중 layout / panel role / readability 확인을 위한 수동 검증 기록이다.

## 2. 검증 전제

| 항목 | 값 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 맵 | TestRoom PIE |
| Overlay | `Portfolio.DebugOverlay.Enabled 1` |
| Collect | `Portfolio.DebugOverlay.Collect 1` |
| EventLog | separate panel 적용 |
| Interaction | right-top separate panel 적용 |

## 3. 참고 캡처

| 파일 | 확인 내용 |
| --- | --- |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-08-01 00-11-22-419.jpg` | idle / target selected 상태에서 3-panel layout 확인 |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-08-01 00-11-48-970.jpg` | combat 중 Player / Enemy / EventLog / Interaction panel 동시 표시 확인 |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-08-01 00-12-10-592.jpg` | Recent AI Event stale 문구와 right-top Interaction panel 표시 확인 |

위 캡처는 최종 제출 후보가 아니며, P1 layout 검증 참고 자료로만 사용한다.

## 4. 확인된 Panel 역할

### 4.1 `Pannel_01`

왼쪽 panel은 actor current state와 actor-local recent summary를 담당한다.

확인된 표시 범위:

- `[Player]`
- Player current state
- Player `[Recent Execution]`
- `[Enemy]`
- Enemy target source / selected target / nearest diagnostic
- Enemy current state
- Enemy `[Recent Execution]`
- Enemy `[Current AI]`
- Enemy `[Recent AI Event]`

`Pannel_01`에는 더 이상 world-level `[Interaction]` block을 넣지 않는다.

### 4.2 `Pannel_02`

상단 중앙에서 우측 방향으로 확장되는 panel은 EventLog 전용 panel이다.

확인된 표시 범위:

- `[Debug Overlay Pannel_02]`
- `[Event Log: All]`
- EventLog filter / limit / noise / collision filter 결과

EventLog는 Interaction flow를 설명하는 world-level log이지만, HUD layout에서는 Interaction panel 안에 넣지 않고 별도 panel로 유지한다.

### 4.3 `Pannel_03`

오른쪽 상단 panel은 Interaction recent summary 전용 panel이다.

확인된 표시 범위:

- `[Debug Overlay Pannel_03]`
- `[Interaction]`
- `[Recent Execution]`
- `[Recent Combat]`

`[Recent AI Event]`는 Interaction panel로 이동하지 않고 Enemy panel에 유지한다. AI는 selected Enemy 기준 current / recent event evidence로 보는 편이 더 명확하기 때문이다.

## 5. Layout 확인 결과

| 항목 | 기대 | 실제 | 결과 |
| --- | --- | --- | --- |
| Left panel 역할 | Player / Enemy actor state 중심 | Player / Enemy actor state와 actor recent 표시 | 통과 |
| EventLog 분리 | `Pannel_02`에만 EventLog 표시 | `Pannel_02`에 `[Event Log: All]` 표시 | 통과 |
| Interaction 분리 | `Pannel_03`에 Interaction recent 표시 | 오른쪽 상단 `Pannel_03`에 `[Interaction]` 표시 | 통과 |
| Panel overlap | `Pannel_02`와 `Pannel_03`이 겹치지 않음 | 두 panel이 좌우 분리되어 표시 | 통과 |
| Enemy Recent AI Event | Enemy panel에 유지 | Enemy panel 하단에 유지 | 통과 |
| Stale wording | stale 시간을 명확히 표시 | `Stale Time: ...s`, `Last Pawn: ...` 표시 | 통과 |

## 6. 현재 고정할 표시 기준

현재 PIE 확인 결과를 기준으로 다음 layout을 P1 후속 작업의 기본값으로 본다.

```text
[Debug Overlay Pannel_01]
[Player]
...
[Recent Execution]

[Enemy]
...
[Recent Execution]
[Current AI]
[Recent AI Event]

[Debug Overlay Pannel_02]
[Event Log: All]
...

[Debug Overlay Pannel_03]
[Interaction]
[Recent Execution]
[Recent Combat]
```

## 7. 주의 사항

- `Pannel` spelling은 현재 표시값을 유지한다.
- `Pannel_03`는 Interaction recent summary 전용이며, full EventLog를 포함하지 않는다.
- EventLog line wrapping / compact 재작업은 이번 검증 범위가 아니다.
- Runtime LOD actual 표시와 CollisionDisabledIgnored EventLog noise 문제는 별도 후속 작업으로 둔다.
- 현재 캡처는 editor/output log 또는 검증용 환경이 포함될 수 있으므로 최종 제출 evidence로 승격하지 않는다.

## 8. 완료 판단

이번 PIE 확인으로 다음 기준을 충족했다.

- Player / Enemy / EventLog / Interaction panel 역할이 분리되어 있다.
- Interaction block은 left panel에서 제거되고 right-top `Pannel_03`으로 이동했다.
- EventLog는 `Pannel_02`에 독립 표시된다.
- Enemy `Recent AI Event` stale 상태는 `Stale Time` 기준으로 읽힌다.

따라서 P1 overlay layout은 현재 구조를 기준으로 후속 기능 검증을 진행한다.

## 9. 다음 작업

1. Recent AI Event 의미를 추가로 보강할지 결정한다.
2. CollisionDisabledIgnored EventLog noise 잔존 이슈를 별도 검토한다.
3. P1 통합 PIE 체크리스트를 현재 3-panel layout 기준으로 갱신한다.
