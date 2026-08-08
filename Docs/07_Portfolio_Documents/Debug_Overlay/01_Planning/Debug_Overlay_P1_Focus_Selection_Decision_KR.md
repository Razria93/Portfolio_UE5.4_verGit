# Debug Overlay P1 Focus Selection Decision

## 1. 목적

이 문서는 P1 debug overlay의 Enemy panel 표시 정책을 최종 결정한다.

P1의 Enemy panel은 자동 추정된 enemy를 보여주는 화면이 아니라, 사용자가 명시적으로 선택한 target의 상태를 evidence로 보여주는 화면으로 둔다.

## 2. 결정 배경

기존 P1 설계는 다음 source chain을 기준으로 했다.

```text
FocusComponent -> RecentCombatFocus -> WorldScanFallback
```

이 구조는 TargetComponent가 없거나 clear된 상태에서도 RecentCombatFocus 또는 WorldScanFallback이 Enemy panel을 채울 수 있다. 그 결과 사용자가 명시 focus을 잡았는지, 자동 fallback이 표시된 것인지 구분하기 어렵다.

또한 line trace 기반 `DebugOverlaySelectTarget`은 TestRoom PIE에서 실제 운용성이 낮았다. 카메라 방향과 collision hit 조건을 맞추기 어렵고, 향후 TargetComponent도 line trace 추적 방식으로 설계할 가능성이 낮다.

따라서 P1부터는 “명시 focus이 있어야 Enemy panel 정보가 의미 있다”는 정책은 유지하되, 명시 선택 경로는 nearest command 중심으로 단순화한다.

## 3. 최종 결정

| 항목 | 결정 |
| --- | --- |
| 기본 상태 | `EnemyFocusMode: None` |
| 자동 fallback 표시 | Enemy panel을 자동 fallback으로 채우지 않는다. |
| `DebugOverlaySelectTarget` | P1 기본 명령에서 제거한다. |
| line trace focus selection | P1에서 사용하지 않는다. |
| `DebugOverlaySelectNearestFocus` 성공 | `EnemyFocusMode: FocusComponent.NearestFocus` |
| `DebugOverlaySelectNearestFocus` 실패 | 기존 focus clear 후 `EnemyFocusMode: None` |
| `DebugOverlayClearFocus` | focus clear 후 `EnemyFocusMode: None` |

P1 evidence claim은 `FocusComponent.NearestFocus`로 명시 선택된 대상만 신뢰한다.

## 4. Source 의미

| Source | 의미 | Evidence claim |
| --- | --- | --- |
| `FocusComponent.NearestFocus` | 사용자 명령으로 nearest enemy를 명시 선택한 enemy | 명시 command 기반 focus selection evidence |
| `None` | 명시 focus 없음 | Enemy panel target evidence 없음 |
| `RecentCombatFocus` | 최근 combat event에서 확인된 source/target pair | P1 기본 source chain에서 제외. 이후 diagnostic/source 검증 후보 |
| `WorldScanFallback` | world scan으로 찾은 enemy | P1 기본 source chain에서 제외. 이후 diagnostic/debug fallback 후보 |

`Nearest`는 world scan과 유사하게 월드의 enemy를 탐색하지만, 사용자가 command를 실행했을 때만 target으로 저장된다는 점에서 `WorldScanFallback`과 다르다.

## 5. 구현 영향

| 파일/영역 | 영향 |
| --- | --- |
| `UCDebugOverlayFocusComponent` | source type은 `None` / `Nearest`만 유지한다. |
| `ACPlayerController` | `DebugOverlaySelectTarget`과 line trace helper를 제거한다. |
| `ACPlayerController` | `DebugOverlaySelectNearestFocus`과 `DebugOverlayClearFocus`만 유지한다. |
| `CDebugOverlayHUD` | FocusComponent target이 없으면 `EnemyFocusMode: None`을 표시한다. |
| RecentCombatFocus helper | 기본 Enemy panel source에서 제외하고 diagnostic 후보로 유지할 수 있다. |
| WorldScanFallback helper | 기본 Enemy panel source에서 제외하고 diagnostic/debug fallback 후보로 유지할 수 있다. |
| gameplay flow | 기존 combat/action target flow는 변경하지 않는다. |

구현 중 fallback helper를 완전히 삭제할 필요는 없다. 다만 P1 기본 HUD path에서는 자동 표시 source로 사용하지 않는다.

## 6. 비목표

- 범용 target system 구현
- combat targeting 연동
- lock-on / target cycling UI
- camera / aim assist
- line trace focus selection 유지
- RecentCombatFocus 자동 승격
- WorldScanFallback 자동 선택
- 최종 촬영 / 패키징

## 7. 검증 기준

| 시나리오 | 기대 표시 |
| --- | --- |
| PIE 진입 후 focus 없음 | `EnemyFocusMode: None` |
| `DebugOverlaySelectNearestFocus` 성공 | `EnemyFocusMode: FocusComponent.NearestFocus` |
| `DebugOverlaySelectNearestFocus` 실패 | `EnemyFocusMode: None` |
| `DebugOverlayClearFocus` 실행 | `EnemyFocusMode: None` |
| focus 없음 상태에서 combat event 존재 | RecentCombatFocus이 자동으로 Enemy panel을 채우지 않는다. |
| focus 없음 상태에서 enemy가 월드에 1명 존재 | WorldScanFallback이 자동으로 Enemy panel을 채우지 않는다. |

`RecentCombatFocus`, `WorldScanFallback`, `Stale`, `NotMatched`, `Ambiguous`는 P1 기본 focus selection 성공 evidence로 사용하지 않는다.

## 8. 문서 우선순위

이 문서는 이전 P1 문서에 남아 있는 `FocusComponent.Trace`, `DebugOverlaySelectTarget`, `FocusComponent -> RecentCombatFocus -> WorldScanFallback` 자동 fallback chain 설명보다 우선한다.

이후 `Debug_Overlay_P1_Scope_KR.md`, `Debug_Overlay_P1_Work_Order_KR.md`, `Debug_Overlay_P1_FocusComponent_PIE_Checklist_KR.md`는 이 결정 문서 기준으로 갱신한다.

## 9. 다음 구현 단계

1. `UCDebugOverlayFocusComponent`에서 `Trace` source type과 trace summary를 제거한다.
2. `ACPlayerController`에서 `DebugOverlaySelectTarget`과 line trace helper를 제거한다.
3. `CDebugOverlayHUD`에서 trace diagnostic line 표시를 제거한다.
4. RecentCombatFocus / WorldScanFallback 자동 fallback 표시를 P1 기본 path에서 제외한 상태를 유지한다.
5. PIE checklist를 nearest/clear 기준으로 갱신한다.
