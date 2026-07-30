# Debug Overlay P1 TargetComponent PIE Checklist

## 1. 목적

이 문서는 P1 debug overlay의 Enemy panel source가 PIE에서 최신 명시 target 정책대로 동작하는지 확인하기 위한 체크리스트다.

검증 대상 command는 다음 2개다.

```text
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
```

`DebugOverlaySelectTarget`과 line trace 기반 target selection은 P1 기본 운용에서 제거한다.

이번 체크리스트는 최종 촬영/패키징 문서가 아니다. P1 TargetComponent source type 구현 이후, 기능 동작과 실패 분기를 확인하기 위한 검증 문서다.

## 2. 현재 코드 전제

| 항목 | 기준 |
| --- | --- |
| TargetComponent source type | `None`, `Nearest` |
| 기본 Enemy source | `TargetComponent.Nearest`, `None` |
| 명시 target 없음 | `EnemySource: None` |
| `RecentCombatTarget` | 기본 Enemy panel source로 자동 사용하지 않음 |
| `WorldScanFallback` | 기본 Enemy panel source로 자동 사용하지 않음 |
| 실패/clear 정책 | 기존 target clear 후 `EnemySource: None` |

Enemy panel은 자동으로 Enemy를 추정해 채우는 화면이 아니다. `DebugOverlaySelectNearestTarget`으로 명시 선택된 target만 P1 target evidence로 취급한다.

## 3. 사전 조건

| 항목 | 기준 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 테스트 맵 | `/Game/00_UnitTest/TestRoom` |
| HUD 연결 | `ACDebugOverlayGameMode` 또는 debug overlay HUD가 연결된 GameMode |
| 실행 환경 | non-shipping PIE |
| Overlay 표시 | `Portfolio.DebugOverlay.Enabled 1` |
| Overlay 수집 | `Portfolio.DebugOverlay.Collect 1` |
| EventLog line | `Portfolio.DebugOverlay.EventLogLimit 5` 권장 |
| asset 저장 | `.umap`, `.uasset` 저장은 의도적으로만 수행 |

## 4. 실행 명령과 기대값

PIE console에서 다음 명령을 실행한다.

| 명령 | 동작 | 성공 기대값 | 실패 기대값 |
| --- | --- | --- | --- |
| `DebugOverlaySelectNearestTarget` | player pawn 기준 nearest `ACEnemy`를 명시 선택 | `EnemySource: TargetComponent.Nearest` | 기존 target clear 후 `EnemySource: None` |
| `DebugOverlayClearTarget` | explicit target clear | `EnemySource: None` | 해당 없음 |

nearest 선택은 camera 방향이 아니라 player pawn 위치 기준 거리 우선이다.

## 5. 기본 화면 확인

PIE 시작 후 overlay에 다음 구조가 보이는지 확인한다.

```text
[Debug Overlay P0.5]
[Player]
[Enemy]
[Recent Execution]
[Recent Combat]
[Recent AI]
[Event Log]
```

P1 TargetComponent 검증에서는 `[Enemy]` 아래 source line을 우선 확인한다.

## 6. 검증 시나리오

| 순서 | 테스트 액션 | 기대 결과 | 실패 시 확인 |
| --- | --- | --- | --- |
| 1 | PIE 진입 직후 command 실행 전 확인 | `EnemySource: None` | 이전 target 또는 fallback source가 남아 있는지 |
| 2 | Enemy 근처에서 `DebugOverlaySelectNearestTarget` 실행 | `EnemySource: TargetComponent.Nearest`, `EnemyTarget: Selected=...` | nearest radius `1500.f`, player pawn 위치, Enemy 거리 |
| 3 | Enemy가 없거나 너무 먼 상태에서 `DebugOverlaySelectNearestTarget` 실행 | `EnemySource: None` | 실패 시 기존 target clear 여부 |
| 4 | `DebugOverlayClearTarget` 실행 | `EnemySource: None` | clear 후 이전 target source가 남아 있는지 |
| 5 | combat event 이후 target 없이 확인 | `RecentCombatTarget`이 Enemy panel을 자동으로 채우지 않음 | TargetComponent decision 적용 여부 |
| 6 | world에 Enemy가 있어도 target 없이 확인 | `WorldScanFallback`이 Enemy panel을 자동으로 채우지 않음 | HUD 기본 source path가 자동 fallback을 호출하는지 |

## 7. 기대 표시 예시

### 7.1 Nearest 선택 성공

```text
EnemySource: TargetComponent.Nearest
EnemyTarget: Selected=BP_CEnemy_C_1
```

의미:

- 사용자가 nearest command로 Enemy를 명시 선택했다.
- camera trace evidence가 아니라, 명시 command 기반 target selection evidence다.

### 7.2 target 없음

```text
EnemySource: None
```

의미:

- 현재 명시 target이 없다.
- Enemy actor-derived line은 `N/A`, `None`, `NotCaptured` 계열로 보일 수 있다.
- target selection 성공 evidence로 사용하지 않는다.

## 8. Diagnostic 후보 분리

아래 항목은 P1 기본 Enemy panel 성공 evidence가 아니다. 후속 diagnostic mode 또는 source 검증 후보로만 다룬다.

| 항목 | 현재 P1 기본 정책 |
| --- | --- |
| `RecentCombatTarget` | target 없음 상태를 자동으로 채우지 않음 |
| `WorldScanFallback` | target 없음 상태를 자동으로 채우지 않음 |
| `Stale` | 기본 성공 evidence 아님 |
| `Ambiguous(Count=N)` | 기본 성공 evidence 아님 |
| `NotMatched` | 기본 성공 evidence 아님 |

기존 helper가 코드에 남아 있더라도, 기본 `DrawHUD` source path에서 자동 호출되지 않는 것이 이번 P1 정책이다.

## 9. 실패 분기

### 9.1 Exec command가 호출되지 않음

확인:

1. PIE console에 명령을 정확히 입력했는지 확인한다.
2. 현재 PlayerController가 `ACPlayerController`인지 확인한다.
3. 실행 환경이 shipping이 아닌지 확인한다.

### 9.2 `DebugOverlaySelectNearestTarget`이 기대와 다른 Enemy를 선택함

확인:

1. nearest command는 player pawn 위치 기준 거리 우선이다.
2. camera 방향보다 거리가 우선될 수 있다.
3. 이 경우 evidence 설명은 "nearest command로 명시 선택한 target"으로 한정한다.

### 9.3 실패 후 이전 target이 남아 보임

실패 조건:

- `DebugOverlaySelectNearestTarget` 실패 후 이전 `TargetComponent.Nearest`가 계속 보임

기대 동작:

```text
EnemySource: None
```

### 9.4 clear 이후 fallback이 자동 표시됨

실패 조건:

- `DebugOverlayClearTarget` 이후 `EnemySource: RecentCombatTarget`이 자동 표시됨
- `DebugOverlayClearTarget` 이후 `EnemySource: WorldScanFallback`이 자동 표시됨

기대 동작:

```text
EnemySource: None
```

### 9.5 target 없음 상태에서 Enemy actor 값이 성공 evidence처럼 보임

확인:

1. `[Enemy]` panel source가 `EnemySource: None`인지 확인한다.
2. target 없음 상태의 Enemy actor-derived 값은 성공 evidence로 사용하지 않는다.
3. 실제 캡처 설명에서는 "명시 target 없음"으로 기록한다.

## 10. 확인 기준

- `TargetComponent.Nearest`는 nearest command 기반 명시 target source로만 주장한다.
- `None`은 target selection 성공 evidence가 아니다.
- `RecentCombatTarget`은 P1 기본 source에서 제외하고 diagnostic 후보로만 다룬다.
- `WorldScanFallback`은 P1 기본 source에서 제외하고 diagnostic 후보로만 다룬다.
- dead enemy 제외는 아직 적용하지 않았으므로 성공 evidence처럼 말하지 않는다.
- nearest radius는 P1 debug helper 기본값으로만 설명한다.
- 실제 코드에서 읽지 못한 값을 성공 evidence처럼 표시하지 않는다.

## 11. 완료 기준

| 완료 항목 | 기준 |
| --- | --- |
| Nearest source | `DebugOverlaySelectNearestTarget` 후 `EnemySource: TargetComponent.Nearest` 표시 |
| Target summary | source 성공 시 `EnemyTarget: Selected=...` 표시 |
| Nearest 실패 | 기존 target clear 후 `EnemySource: None` 표시 |
| Clear | `DebugOverlayClearTarget` 후 `EnemySource: None` 표시 |
| Recent diagnostic | target 없음 상태에서 자동 Enemy source로 승격되지 않음 |
| WorldScan diagnostic | target 없음 상태에서 자동 Enemy source로 승격되지 않음 |
| 문서 claim | 실제 코드 정책과 일치 |

## 12. 결과 기록 템플릿

```text
날짜:
브랜치:
맵:
GameMode/HUD 연결:
CVar:

PIE 진입 직후:
DebugOverlaySelectNearestTarget 성공:
DebugOverlaySelectNearestTarget 실패:
DebugOverlayClearTarget:

RecentCombatTarget 자동 표시 여부:
WorldScanFallback 자동 표시 여부:

Evidence 사용 가능 항목:
Evidence 제외 항목:
후속 조치:
```

## 13. 다음 단계

이 체크리스트를 통과하면 다음 작업은 `P1 TargetComponent PIE 수동 검증 결과 정리` 또는 `P1 EventLog category filter 설계`로 진행한다.

최종 촬영/패키징은 P1 기능 검증이 닫힌 뒤 별도 단계에서 진행한다.
