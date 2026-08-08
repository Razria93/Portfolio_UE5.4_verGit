# Debug Overlay P1 Integrated PIE Result

## 1. 목적

이 문서는 P1 debug overlay의 통합 PIE 확인 결과를 기록한다.

이번 결과는 최종 제출용 캡처나 FinalCandidate 패키징이 아니라, P1 마감 전 기능/표시 정책이 실제 PIE 화면에서 의도대로 보이는지 확인한 수동 검증 기록이다.

## 2. 검증 전제

| 항목 | 값 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 맵 | TestRoom PIE |
| Overlay | `Portfolio.DebugOverlay.Enabled 1` |
| Collect | `Portfolio.DebugOverlay.Collect 1` |
| Focus source | `FocusComponent.NearestFocus` |
| EventLog | separate panel |
| Interaction | right-top separate panel |
| Runtime LOD | P1 보류, `N/A` 유지 |

## 3. 참고 캡처

| 파일 | 확인 내용 |
| --- | --- |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-08-01 00-50-10-176.jpg` | P1 통합 layout, FocusComponent nearest, EventLog separate panel, Interaction panel, Current AI, Recent AI Event, damage breakdown 확인 |

위 캡처는 최종 제출 후보가 아니다. P1 기능 검증과 마감 판단을 위한 참고 자료로만 사용한다.

## 4. 통합 표시 결과

### 4.1 `Panel_01`

왼쪽 panel은 Player / Enemy actor 상태와 actor-local recent evidence를 표시한다.

확인된 Player 표시:

- `State: Reaction`
- `Reaction: Hit`
- `HP: 4837.5/5000.0 (DeadState: Alive)`
- `Stagger: 0/3`
- Player `[Recent Execution]`

확인된 Enemy 표시:

- `EnemyFocusMode: FocusComponent.NearestFocus`
- `EnemyFocusActor: Selected: BP_CEnemy_C_1`
- `EnemyFocusCommand: NearestSelected | Target: BP_CEnemy_C_1 | Distance: 1687 | Radius: 3000`
- Enemy current state
- Enemy `[Recent Execution]`
- Enemy `[Current AI]`
- Enemy `[Recent AI Event]`

`Runtime LOD`는 현재도 `N/A`이며, P1 완료 주장 범위에 포함하지 않는다.

### 4.2 `Panel_02`

상단 중앙 panel은 EventLog 전용 panel이다.

확인된 표시:

- `[Debug Overlay Panel_02]`
- `[Event Log: All]`
- `Combat/TargetAccepted`
- `Execution/DecisionResolved`
- `AI/CombatActionTaskSucceeded`

Combat damage line은 다음 순서로 표시된다.

```text
Request -> Mitigated -> Final -> Commit
```

캡처에서 확인된 예:

```text
Combat/TargetAccepted: Attacker: BP_CEnemy_C_1 | Defender: BP_CPlayer_C_0 | Outcome: None | Request: 10.000 | Mitigated: 10.000 | Final: 10.000 | Commit: 10.000 | Accepted: true
```

`Request`는 요청된 damage value, `Mitigated`는 방어/감산 이후 값, `Final`은 최종 판정 damage, `Commit`은 실제 commit 값으로 해석한다.

### 4.3 `Panel_03`

오른쪽 상단 panel은 Interaction recent summary 전용 panel이다.

확인된 표시:

- `[Debug Overlay Panel_03]`
- `[Interaction]`
- `[Recent Execution]`
- `[Recent Combat]`

캡처 기준 `[Recent Combat]`는 다음 핵심 값을 표시한다.

- `Attacker: BP_CEnemy_C_1`
- `Defender: BP_CPlayer_C_0`
- `Outcome: None`
- `Request: 10.000`
- `Mitigated: 10.000`
- `Final: 10.000`
- `Commit: 10.000`
- `Accepted: true`

Collision lifecycle event는 Recent Combat 대표값을 덮어쓰지 않는다.

## 5. Current AI / Recent AI Event 판단

P1에서는 Enemy AI evidence를 다음처럼 분리한다.

- `[Current AI]`: selected Enemy의 현재 controller / pawn / blackboard 상태
- `[Recent AI Event]`: 최근 AI task event

캡처 기준 Current AI는 다음 값을 확인했다.

- `Controller: BP_CAIController_C_0`
- `Pawn: BP_CEnemy_C_1`
- `Target: BP_CPlayer_C_0`
- `IntentState: Engage`
- `ReturnHome: false`
- `UsePatrol: true`
- `HasLOS: true`
- `DistanceToTarget: 131.0`
- `IsCombatAction: true`

Recent AI Event는 다음처럼 event evidence로 표시된다.

- `Task: CombatAttack`
- `Result: Started`
- `Age: 4.53`
- `RejectReason: None`

`Recent AI Event`는 현재 AI 상태가 아니라 최근 event이므로, 오래된 경우 stale 표시가 붙을 수 있다. 현재 P1 기준에서는 Current AI와 Recent AI Event를 분리한 것으로 충분하다고 판단하며, Behavior Tree 하위 node 전체 추적은 후속 후보로 둔다.

## 6. 해결된 항목

이번 통합 검증 기준으로 다음 항목은 P1 구현 완료로 본다.

- Player / Enemy panel 분리
- Enemy explicit focus source
- `DebugOverlaySelectNearestFocus` 기반 `FocusComponent.NearestFocus`
- nearest diagnostic 표시
- Player / Enemy Recent Execution 분리
- Interaction panel 분리
- EventLog separate panel 분리
- EventLog category filter
- EventLog noise / collision window filter
- Recent Combat에서 collision lifecycle overwrite 제거
- Combat damage breakdown `Request / Mitigated / Final / Commit`
- Enemy Current AI / Recent AI Event 분리
- `AI: NotCaptured` current line 제거
- Stagger Count 표시

## 7. 의도적으로 보류한 항목

다음 항목은 P1 최종 제출 claim에 포함하지 않는다.

- Runtime LOD actual 표시
- Behavior Tree active node 전체 추적
- EventLog 최종 촬영/패키징
- FinalCandidate 캡처 패키징
- 포트폴리오 본문 연결

Runtime LOD는 현재 `N/A`로 표시되며, 실제 값 hook과 claim 기준을 별도 작업으로 잡는다.

## 8. BT_Default 변경 기록

`Content/02_Controller/02_Enemy/AI/BehaviorTree/BT_Default.uasset` 변경은 debug overlay HUD 코드 변경이 아니다.

사용자 확인 기준으로, 이 변경은 `BT_Default`의 Patrol 이동 가능 거리 조정이다. P1 검증 중 Enemy patrol / engage 흐름을 안정적으로 확인하기 위한 tuning 변경으로 별도 커밋한다.

`Content/00_UnitTest/TestRoom.umap` 같은 test map 저장 변경은 이번 문서/asset 커밋 범위에 포함하지 않는다.

## 9. 완료 판단

P1 debug overlay는 Runtime LOD actual 표시를 제외하면 처음 계획했던 범위를 대부분 구현 완료한 상태로 판단한다.

특히 다음 claim은 현재 캡처와 구현 기준으로 사용할 수 있다.

- 명시 focus source 기반 Enemy panel
- Player / Enemy / Interaction / EventLog panel 역할 분리
- Player / Enemy recent execution 분리
- Interaction recent combat damage breakdown
- Enemy Current AI와 Recent AI Event 분리

단, 이 문서의 캡처는 최종 제출 evidence가 아니므로 FinalCandidate로 승격하지 않는다.

## 10. 다음 작업

1. P1 마감 기준 문서와 P52 PR 설명을 최신 구현 기준으로 정리한다.
2. Runtime LOD actual 표시를 P2 또는 별도 후속 작업으로 넘길지 확정한다.
3. P1 종료 후 FinalCandidate 촬영/패키징으로 이동한다.
