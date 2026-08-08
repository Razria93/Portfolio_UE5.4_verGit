# Debug Overlay P1 FocusComponent PIE Result

## 1. 목적

이 문서는 P1 debug overlay의 FocusComponent 기반 Enemy panel 표시가 PIE에서 의도대로 동작했는지 기록한다.

이번 결과는 최종 촬영/패키징 후보가 아니다. `DebugOverlaySelectTarget` 제거 이후, `DebugOverlaySelectNearestFocus` / `DebugOverlayClearFocus` 중심의 명시 focus 정책이 실제 화면과 일치하는지 확인한 기능 검증 결과다.

## 2. 검증 전제

| 항목 | 내용 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 맵 | TestRoom PIE |
| Overlay 표시 | `Portfolio.DebugOverlay.Enabled 1` |
| Overlay 수집 | `Portfolio.DebugOverlay.Collect 1` |
| EventLog line | `Portfolio.DebugOverlay.EventLogLimit 5` |
| 제거된 명령 | `DebugOverlaySelectTarget` |
| 사용 명령 | `DebugOverlaySelectNearestFocus`, `DebugOverlayClearFocus` |

P1 FocusComponent 정책은 `FocusComponent.NearestFocus` / `None` 기준이다. `RecentCombatFocus`과 `WorldScanFallback`은 기본 Enemy panel source로 자동 사용하지 않는다.

## 3. 참고 캡처

| 파일 | 확인 목적 | 관찰 결과 |
| --- | --- | --- |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-07-31 01-01-48-567.jpg` | PIE 진입 후 focus 없음 | `[Enemy]` 아래 `EnemyFocusMode: None` 표시 |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-07-31 01-02-50-090.jpg` | nearest target 선택 성공 | `EnemyFocusMode: FocusComponent.NearestFocus`, `EnemyFocusActor: Selected=BP_CEnemy_C_1` 표시 |
| `C:\Users\starb\Videos\Bandicam\bandicam 2026-07-31 01-02-58-350.jpg` | clear 후 focus 없음 | `[Enemy]` 아래 `EnemyFocusMode: None` 표시 |

캡처에는 editor console window가 함께 보인다. 따라서 이 파일들은 최종 제출 후보가 아니라 PIE 기능 검증 참고 자료로만 사용한다.

## 4. 검증 결과 요약

| 시나리오 | 기대값 | 실제값 | 판단 |
| --- | --- | --- | --- |
| PIE 진입 직후 | `EnemyFocusMode: None` | `EnemyFocusMode: None` | 통과 |
| `DebugOverlaySelectNearestFocus` 실행 | `EnemyFocusMode: FocusComponent.NearestFocus` | `EnemyFocusMode: FocusComponent.NearestFocus` | 통과 |
| nearest target summary | `EnemyFocusActor: Selected=...` | `EnemyFocusActor: Selected=BP_CEnemy_C_1` | 통과 |
| `DebugOverlayClearFocus` 실행 | `EnemyFocusMode: None` | `EnemyFocusMode: None` | 통과 |
| focus 없음 상태 자동 fallback | RecentCombatFocus / WorldScanFallback 자동 표시 없음 | 자동 fallback 표시 없음 | 통과 |

## 5. 성공 판단

이번 PIE 확인에서 다음 정책이 충족되었다.

- 기본 상태에서 Enemy panel이 자동 fallback으로 채워지지 않는다.
- nearest command 성공 시 명시 focus source가 `FocusComponent.NearestFocus`로 표시된다.
- target summary가 `Selected=BP_CEnemy_C_1`로 표시된다.
- clear command 이후 focus 없음 상태인 `EnemyFocusMode: None`으로 복귀한다.
- `RecentCombatFocus` / `WorldScanFallback` 자동 표시가 P1 기본 path에 재도입되지 않았다.

따라서 P1 TargetComponent의 명시 focus source 정책은 현재 캡처 기준으로 동작 확인된 것으로 본다.

## 6. 주의점

- `DebugOverlaySelectNearestFocus`은 camera 방향이 아니라 player pawn 위치 기준 거리 우선 선택이다.
- 다중 enemy 상황에서는 선택 근거를 nearest distance로 설명해야 한다.
- dead enemy 제외는 아직 적용하지 않았으므로 성공 evidence처럼 말하지 않는다.
- Runtime LOD / AI는 아직 `N/A` / `NotCaptured` 상태이므로 성공 evidence로 주장하지 않는다.
- 이번 캡처는 editor console window가 보이므로 최종 제출 캡처로 사용하지 않는다.
- 최종 촬영/패키징은 P1 기능 검증과 후속 보강이 닫힌 뒤 별도 단계에서 진행한다.

## 7. Evidence Claim 범위

이번 결과로 주장 가능한 범위:

- P1 debug overlay는 명시 focus이 없을 때 Enemy panel을 `None`으로 표시한다.
- `DebugOverlaySelectNearestFocus` 명령으로 Enemy target을 명시 선택할 수 있다.
- 선택된 Enemy는 FocusComponent source와 target summary로 화면에 표시된다.
- `DebugOverlayClearFocus`으로 명시 focus을 해제할 수 있다.

이번 결과로 주장하지 않는 범위:

- 최종 제출용 evidence 품질
- camera trace 기반 focus selection
- combat gameplay target 연동
- lock-on / target cycling
- Runtime LOD 성공 표시
- AI detail 성공 표시
- Player/Enemy EventLog 분리

## 8. 남은 작업

다음 작업은 P1의 나머지 필수 범위인 EventLog category filter로 넘어간다.

권장 순서:

1. `P1 EventLog Category Filter 설계`
2. `P1 EventLog Category Filter 구현`
3. `P1 통합 검증 체크리스트 갱신`
4. `Runtime LOD / AI 표시 보강 검토`
5. P1 완료 후 FinalCandidate 재촬영
