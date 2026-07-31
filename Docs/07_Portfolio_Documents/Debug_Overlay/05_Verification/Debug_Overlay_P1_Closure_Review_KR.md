# Debug Overlay P1 Closure Review

## 목적

이 문서는 Debug Overlay P1 작업을 FinalCandidate 촬영 단계로 넘길 수 있는지 최종 점검한다.

P1의 목적은 최종 제출 캡처를 만드는 것이 아니라, runtime evidence tooling이 실제 PIE에서 설명 가능한 형태로 동작하는지 확인하는 것이다. 따라서 이 문서는 완료 항목, 보류 항목, evidence claim 가능 범위, 금지 claim을 분리해서 고정한다.

코드클린 구현은 이번 브랜치 마감 조건에서 제외한다. 현재 브랜치에서는 구조 리뷰와 후속 cleanup 후보를 문서화한 상태로 두고, 파일 분리나 schema/API 변경이 필요한 cleanup은 후속 브랜치에서 다룬다.

## 결론

P1은 `Runtime LOD actual` 표시를 제외하면 FinalCandidate 촬영 전 기능 검증 단계로 닫을 수 있다.

FinalCandidate 촬영은 P1 기능 closure 이후 별도 패키지로 진행한다. 현재 PIE 캡처와 검증 문서는 기능 확인 evidence이며, 최종 제출 후보로 승격하지 않는다.

## P1 완료 항목

| 영역 | 상태 | 마감 판단 |
| --- | --- | --- |
| TargetComponent 기반 Enemy 선택 | 완료 | `DebugOverlaySelectNearestTarget`, `DebugOverlayClearTarget`, `EnemySource: TargetComponent.Nearest`, `EnemySource: None` 기준으로 claim 가능 |
| Nearest target 운용성 | 완료 | nearest radius `3000` 기준으로 PIE 운용 가능 |
| Player / Enemy current state panel | 완료 | actor current state 비교 claim 가능 |
| Player / Enemy Recent Execution | 완료 | actor별 recent execution 분리 claim 가능 |
| Enemy Current AI | 완료 | selected Enemy의 현재 AI context 표시 claim 가능 |
| Enemy Recent AI Event | 완료 | 최근 AI task event 표시 claim 가능. current AI state로 주장하지 않음 |
| Interaction panel | 완료 | `Pannel_03`에서 world-level recent execution/combat summary 표시 |
| EventLog separate panel | 완료 | `Pannel_02`에서 EventLog를 별도 panel로 표시 |
| EventLog category filter | 완료 | `All / Execution / Combat / AI` 표시 filter claim 가능 |
| EventLog limit | 완료 | `0~32` 표시 limit claim 가능 |
| Reject / Ignore noise filter | 완료 | 표시 noise filter claim 가능 |
| Collision window display filter | 완료 | EventLog 표시 제어 claim 가능 |
| Recent Combat collision lifecycle 제외 | 완료 | collision lifecycle event가 Recent Combat을 덮어쓰지 않는 claim 가능 |
| Recent Combat damage breakdown | 완료 | `Request / Mitigated / Final / Commit` 표시 claim 가능 |
| 3-panel layout | 완료 | `Pannel_01 / Pannel_02 / Pannel_03` 구조 claim 가능 |

## P1 보류 / 후속 브랜치 항목

다음 항목은 P1 closure의 실패 조건으로 보지 않는다. 후속 브랜치 또는 P1 이후 작업으로 분리한다.

- `Runtime LOD actual` 표시
- Behavior Tree active node 전체 추적
- EventLog wrapping / compact redesign
- `CollisionDisabledIgnored` event 자체 발생 원인 제거
- HUD helper section / naming cleanup
- `FDebugOverlaySnapshotStore.cpp` helper section cleanup
- HUD 파일 분리
- Store role matcher 분리
- Store schema/API 변경
- 범용 target component 전환
- FinalCandidate 촬영 / 패키징
- 포트폴리오 본문 evidence claim 연결

## Evidence Claim 가능 범위

FinalCandidate로 적합하게 재촬영 / 패키징한 뒤 아래 범위는 성공 evidence로 사용할 수 있다.

- selected Enemy를 명시 target으로 고정해 Enemy current state를 표시한다.
- Player / Enemy current state를 같은 화면에서 비교한다.
- Player / Enemy Recent Execution을 actor별로 분리해 표시한다.
- Interaction panel에서 world-level Recent Execution / Recent Combat 흐름을 표시한다.
- EventLog를 별도 panel로 분리하고 category filter / limit을 적용한다.
- Enemy Current AI와 Recent AI Event를 분리해 현재 상태와 최근 event를 구분한다.
- Recent Combat에서 Attacker / Defender / Outcome / `Request / Mitigated / Final / Commit`을 확인한다.
- Collision lifecycle event는 EventLog diagnostic으로 유지하되 Recent Combat summary를 덮어쓰지 않는다.

## Claim 금지 / 주의 항목

아래 항목은 P1 evidence claim으로 사용하지 않는다.

- `Runtime LOD: N/A`를 Runtime LOD actual 표시 성공으로 주장하지 않는다.
- Behavior Tree active node tracking을 구현 완료처럼 말하지 않는다.
- Recent AI Event를 Current AI evidence로 해석하지 않는다.
- EventLog filter로 숨겨진 event를 “발생하지 않았다”고 주장하지 않는다.
- 현재 PIE 검증 캡처를 FinalCandidate 또는 최종 제출 evidence로 승격하지 않는다.
- Shipping HUD, gameplay HUD, UMG/Slate HUD처럼 표현하지 않는다.
- 범용 target system, lock-on, combat target flow 변경을 구현한 것처럼 말하지 않는다.
- `Pannel` 표기는 현재 runtime style-lock 표시값이므로 임의로 `Panel`로 정정하지 않는다.
- `BT_Default.uasset` patrol 이동 거리 조정은 PIE 검증 안정화용 tuning이며, debug overlay UI 기능 claim에 포함하지 않는다.

## 문서 / 검증 상태

| 문서 | 상태 | Closure 판단 |
| --- | --- | --- |
| `Debug_Overlay_P1_Closure_Criteria_KR.md` | 작성됨 | Runtime LOD actual 제외 조건부 P1 close 기준과 일치 |
| `Debug_Overlay_P1_Integrated_PIE_Result_KR.md` | 작성됨 | P1 통합 검증 결과와 완료 항목 일치 |
| `Debug_Overlay_P1_Overlay_Layout_PIE_Result_KR.md` | 작성됨 | 3-panel layout 검증 결과와 일치 |
| `Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md` | 작성됨 | cleanup 후보 식별 완료. 구현은 후속 브랜치로 이관 |
| `P52_UE5_Portfolio_Pull_Request.md` | 갱신됨 | P1 closure와 FinalCandidate 전 상태를 PR 후보 문맥에 연결 |

## FinalCandidate 진입 조건

다음 조건을 만족하면 FinalCandidate 촬영 / 패키징으로 넘어간다.

- `Pannel_01 / Pannel_02 / Pannel_03` layout이 유지된다.
- `TargetComponent.Nearest`와 `EnemySource: None` 전환이 확인된다.
- Player / Enemy Recent Execution이 actor별로 분리되어 표시된다.
- EventLog separate panel에서 filter / limit이 동작한다.
- Enemy Current AI와 Recent AI Event가 분리되어 표시된다.
- Recent Combat이 collision lifecycle event로 덮이지 않는다.
- Runtime LOD actual은 제외 항목으로 명확히 기록한다.
- editor console, tooltip, taskbar 등 최종 제출 캡처에 부적합한 요소를 제거하고 다시 촬영한다.

## 다음 작업

1. FinalCandidate 실제 촬영 / 패키징
2. P52 PR 문서와 FinalCandidate evidence claim 연결
3. 포트폴리오 본문 evidence claim 정리
4. 후속 브랜치에서 Debug Overlay code cleanup 진행
