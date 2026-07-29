# Debug Overlay P1 TargetComponent PIE Checklist

## 1. 목적

이 문서는 P1 debug overlay의 TargetComponent 기반 Enemy source가 PIE에서 실제로 표시되는지 확인하기 위한 검증 체크리스트다.

검증 대상은 다음 Exec command다.

```text
DebugOverlaySelectTarget
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
```

이 문서는 최종 촬영 후보를 고르는 문서가 아니다. 기능 검증과 failure branch 확인을 위한 문서이며, 최종 촬영/패키징은 P1 검증 이후 별도 단계에서 진행한다.

## 2. 사전 조건

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

## 3. 실행 명령

PIE console에서 다음 명령을 실행한다.

```text
DebugOverlaySelectTarget
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
```

명령 의미:

| 명령 | 의미 |
| --- | --- |
| `DebugOverlaySelectTarget` | camera forward trace로 `ACEnemy`를 찾고, 실패하면 nearest enemy fallback을 사용한다. 둘 다 실패하면 explicit target을 clear한다. |
| `DebugOverlaySelectNearestTarget` | player pawn 기준 nearest `ACEnemy`를 찾는다. 실패하면 explicit target을 clear한다. |
| `DebugOverlayClearTarget` | `UCDebugOverlayTargetComponent`의 explicit target만 clear한다. Store recent combat pair나 world scan fallback은 지우지 않는다. |

## 4. 기본 표시 확인

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

P1 TargetComponent 검증에서는 Enemy panel의 source 관련 line을 우선 확인한다.

## 5. 검증 시나리오

| 순서 | 테스트 액션 | 기대 결과 | 실패 시 확인 |
| --- | --- | --- | --- |
| 1 | Enemy를 화면 중앙에 두고 `DebugOverlaySelectTarget` 실행 | `EnemySource: TargetComponent`, `EnemyTarget: Selected=...` | trace channel, Enemy collision, command 호출 여부 |
| 2 | Enemy를 바라보지 않고 `DebugOverlaySelectTarget` 실행 | trace 실패 후 nearest fallback이 성공하면 `EnemySource: TargetComponent` | nearest radius `1500.f`, TestRoom enemy 거리 |
| 3 | 근처 Enemy 기준 `DebugOverlaySelectNearestTarget` 실행 | nearest enemy가 `TargetComponent` target으로 표시 | 다중 enemy일 때 거리 기준 선택 여부 |
| 4 | `DebugOverlayClearTarget` 실행 | `TargetComponent` source가 사라지고 `RecentCombatTarget` 또는 `WorldScanFallback`으로 내려감 | explicit target clear 여부 |
| 5 | combat event 발생 후 clear 실행 | 최근 전투 상대가 유효하면 `EnemySource: RecentCombatTarget` | `Portfolio.DebugOverlay.Collect`, combat pair 기록 여부 |
| 6 | recent combat pair가 stale인 상태에서 clear 실행 | `EnemyRecentCombat: Stale ...` 보조 표시 후 fallback으로 내려감 | stale 시간, weak pointer validity |
| 7 | Enemy가 없거나 탐색 실패 | `EnemySource: None` | TestRoom enemy 배치 |
| 8 | WorldScanFallback 후보가 여러 개 | `EnemySource: Ambiguous(Count=N)` | 다중 enemy 상태, fallback claim 제외 |

## 6. 기대 표시

### 6.1 TargetComponent 선택 성공

```text
EnemySource: TargetComponent
EnemyTarget: Selected=BP_CEnemy_C_1
```

해석:

- 사용자가 명시적으로 debug target을 선택한 상태다.
- 이 상태만 TargetComponent 기반 Enemy source evidence로 사용한다.

### 6.2 RecentCombatTarget fallback

```text
EnemySource: RecentCombatTarget
EnemyRecentCombat: Source=BP_CPlayer_0 Target=BP_CEnemy_C_1 Age=0.42
```

해석:

- explicit TargetComponent target은 없지만 최근 combat pair로 Enemy를 resolve한 상태다.
- 이 값은 "선택 target"이 아니라 "최근 전투 상대" evidence다.

### 6.3 RecentCombatTarget stale/not matched

```text
EnemyRecentCombat: Stale Source=... Target=... Age=...
EnemyRecentCombat: NotMatched Source=... Target=... Age=...
```

해석:

- 최근 combat pair는 존재하지만 현재 player 기준 Enemy source로 사용하기 어렵다.
- 이 line이 보이면 다음 source는 `WorldScanFallback`, `None`, `Ambiguous`, `Stale` 중 하나로 내려가는지 확인한다.

### 6.4 WorldScanFallback

```text
EnemySource: WorldScanFallback
EnemyFallback: Selected=BP_CEnemy_C_1 Policy=FirstValid Count=1
```

해석:

- TargetComponent와 RecentCombatTarget 모두 유효하지 않을 때 사용하는 최후 fallback이다.
- final evidence에서 TargetComponent 기반 선택처럼 설명하지 않는다.

### 6.5 실패/보류 상태

```text
EnemySource: None
EnemySource: Ambiguous(Count=N)
EnemySource: Stale
```

해석:

- `None`: Enemy를 찾지 못했다.
- `Ambiguous(Count=N)`: WorldScanFallback 후보가 여러 개라 단일 Enemy source로 주장하지 않는다.
- `Stale`: cached fallback target이 더 이상 유효하지 않다.

## 7. 실패 분기

### 7.1 Exec command가 호출되지 않음

확인:

1. PIE console에서 명령을 정확히 입력했는지 확인한다.
2. 현재 player controller가 `ACPlayerController`인지 확인한다.
3. 실행 환경이 shipping이 아닌지 확인한다.

### 7.2 `DebugOverlaySelectTarget` 후 TargetComponent가 표시되지 않음

확인:

1. Enemy가 camera forward trace 경로에 있는지 확인한다.
2. `ECC_Visibility` trace가 Enemy collision에 막히는지 확인한다.
3. trace 실패 후 nearest fallback 범위 `1500.f` 안에 Enemy가 있는지 확인한다.
4. 둘 다 실패하면 explicit target은 clear되고 fallback chain으로 내려가는 것이 정상이다.

### 7.3 nearest fallback이 기대와 다른 Enemy를 선택함

확인:

1. nearest fallback은 player pawn 위치 기준 거리 우선이다.
2. 다중 enemy에서는 카메라 방향보다 거리가 우선될 수 있다.
3. 이 경우 TargetComponent evidence로는 "nearest fallback으로 선택된 target"이라고 설명한다.

### 7.4 clear 이후에도 TargetComponent가 남아 보임

확인:

1. `DebugOverlayClearTarget`을 실행했는지 확인한다.
2. `EnemySource: TargetComponent`가 사라졌는지 확인한다.
3. clear 이후 `RecentCombatTarget` 또는 `WorldScanFallback`으로 내려가는지 확인한다.

### 7.5 fallback chain이 WorldScanFallback으로 내려가지 않음

확인:

1. recent combat pair가 아직 유효하면 `RecentCombatTarget`이 먼저 표시될 수 있다.
2. recent combat pair가 stale/not matched이면 보조 line이 표시된 뒤 fallback으로 내려가야 한다.
3. Enemy가 0개면 `None`, 여러 개면 `Ambiguous(Count=N)`이 정상이다.

## 8. 확인 기준

- `TargetComponent`는 명시 target source로만 주장한다.
- `RecentCombatTarget`은 최근 전투 상대 source로만 주장한다.
- `WorldScanFallback`은 최후 fallback source로만 주장한다.
- `DebugOverlaySelectTarget`은 camera trace 우선, nearest fallback 보조로 설명한다.
- dead enemy 제외는 아직 적용하지 않았으므로 성공 evidence처럼 말하지 않는다.
- trace channel, trace distance, nearest radius는 P1 debug helper 기본값으로만 설명한다.
- 실제 코드에서 읽지 못한 값을 성공 evidence처럼 표시하지 않는다.

## 9. 완료 기준

| 완료 항목 | 기준 |
| --- | --- |
| TargetComponent source | `DebugOverlaySelectTarget` 또는 `DebugOverlaySelectNearestTarget` 후 `EnemySource: TargetComponent` 표시 |
| Target summary | `EnemyTarget: Selected=...` 표시 |
| Clear | `DebugOverlayClearTarget` 후 `EnemySource: TargetComponent` 제거 |
| Recent fallback | combat event 이후 clear 시 `RecentCombatTarget` 표시 가능 |
| WorldScan fallback | explicit target/recent target이 없을 때 `WorldScanFallback` 또는 실패 상태 표시 |
| 실패 상태 | `None`, `Ambiguous(Count=N)`, `Stale`, `NotMatched`를 성공 evidence처럼 사용하지 않음 |

## 10. 결과 기록 템플릿

```text
날짜:
브랜치:
맵:
GameMode/HUD 연결:
CVar:

DebugOverlaySelectTarget:
DebugOverlaySelectNearestTarget:
DebugOverlayClearTarget:

TargetComponent 표시:
RecentCombatTarget fallback:
WorldScanFallback fallback:
None/Ambiguous/Stale 상태:

Evidence 사용 가능 항목:
Evidence 제외 항목:
후속 조치:
```

## 11. 다음 단계

이 체크리스트를 통과하면 다음 작업은 `P1 EventLog category filter 설계` 또는 `P1 TargetComponent PIE 수동 검증 결과 정리` 중 하나로 진행한다.

실제 촬영/패키징은 P1 기능 검증이 닫힌 뒤 진행한다.
