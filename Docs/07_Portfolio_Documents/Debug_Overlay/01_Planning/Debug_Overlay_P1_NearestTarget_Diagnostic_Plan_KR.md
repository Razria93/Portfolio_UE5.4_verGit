# Debug Overlay P1 NearestTarget Diagnostic Plan

## 1. 목적

이 문서는 `DebugOverlaySelectNearestTarget` 운용 중 실패 원인을 구분하기 위한 P1 진단 표시 정책을 고정한다.

최근 PIE 확인 결과 `DebugOverlaySelectNearestTarget` 자체는 정상 동작한다. 다만 현재 nearest radius가 `1500.f`로 고정되어 있어, 반경 밖에서 명령을 실행하면 기존 target이 clear되고 Enemy panel에는 `EnemySource: None`만 표시된다. 이 상태만으로는 명령이 호출되지 않은 것인지, 반경 밖인 것인지, TargetComponent가 없는 것인지 구분하기 어렵다.

이번 문서는 코드 구현이 아니라 후속 구현의 표시 문구, 로그 기준, 검증 기준을 확정하는 설계 문서다.

## 2. 현재 코드 전제

| 항목 | 현재 상태 |
| --- | --- |
| 명령 | `DebugOverlaySelectNearestTarget`, `DebugOverlayClearTarget` |
| 제거된 명령 | `DebugOverlaySelectTarget` |
| 선택 방식 | Player pawn 위치 기준 nearest `ACEnemy` 탐색 |
| 탐색 반경 | `1500.f` |
| 성공 표시 | `EnemySource: TargetComponent.Nearest` |
| 실패 표시 | 기존 target clear 후 `EnemySource: None` |
| 자동 fallback | `RecentCombatTarget`, `WorldScanFallback` 자동 표시 없음 |

nearest 선택은 camera 방향이나 line trace가 아니라 player pawn 위치 기준 거리 우선 선택이다.

## 3. 문제 배경

현재 실패 상황은 모두 같은 화면 결과로 보인다.

```text
EnemySource: None
```

구분이 필요한 실패 조건은 다음과 같다.

| 실패 조건 | 의미 |
| --- | --- |
| `TargetComponentMissing` | PlayerController에 `UCDebugOverlayTargetComponent`가 없음 |
| `InvalidContext` | `World` 또는 `Pawn`이 유효하지 않음 |
| `NoEnemy` | world에 유효한 `ACEnemy`가 없음 |
| `OutOfRange` | Enemy는 있지만 nearest radius `1500.f` 밖에 있음 |

PIE 운용에서는 특히 `OutOfRange`가 자주 헷갈린다. 가까이 접근하면 정상적으로 `TargetComponent.Nearest`가 표시되므로, 실패 원인을 화면 또는 로그에서 짧게 확인할 수 있어야 한다.

## 4. 설계 목표

- command 실행 여부를 확인할 수 있어야 한다.
- 성공/실패 이유를 구분할 수 있어야 한다.
- radius 밖 실패를 명확히 표시해야 한다.
- TargetComponent 명시 target 정책을 유지해야 한다.
- 진단 정보는 최종 evidence claim이 아니라 운용/검증 보조 정보로만 사용한다.
- `RecentCombatTarget` / `WorldScanFallback` 자동 fallback을 재도입하지 않는다.

## 5. 표시 문구 정책

후속 구현에서는 `UCDebugOverlayTargetComponent`가 마지막 selection 결과 summary를 보관하고, HUD Enemy panel에서 target 유무와 별도로 진단 line을 표시하는 방향을 우선한다.

성공:

```text
EnemySource: TargetComponent.Nearest
EnemyTarget: Selected=BP_CEnemy_C_1
EnemySelect: NearestSelected Target=BP_CEnemy_C_1 Distance=820 Radius=1500
```

실패: Enemy 없음

```text
EnemySource: None
EnemySelect: NearestFailed NoEnemy Radius=1500
```

실패: 반경 밖

```text
EnemySource: None
EnemySelect: NearestFailed OutOfRange Closest=2400 Radius=1500
```

실패: TargetComponent 없음

```text
EnemySource: None
EnemySelect: NearestFailed TargetComponentMissing
```

실패: World/Pawn 없음

```text
EnemySource: None
EnemySelect: NearestFailed InvalidContext
```

`EnemySelect` line은 target selection 성공 evidence가 아니라 command 운용 진단 정보다.

## 6. 로그 정책

Output Log에는 명령 호출 시점에만 짧게 기록한다. 별도 CVar는 P1에서는 추가하지 않는다.

예상 로그:

```text
DebugOverlaySelectNearestTarget Result=Selected Target=BP_CEnemy_C_1 Distance=820 Radius=1500
DebugOverlaySelectNearestTarget Result=NoEnemy Radius=1500
DebugOverlaySelectNearestTarget Result=OutOfRange Closest=2400 Radius=1500
DebugOverlaySelectNearestTarget Result=TargetComponentMissing
DebugOverlaySelectNearestTarget Result=InvalidContext
```

명령 호출 시에만 출력되므로 프레임마다 로그가 쌓이지 않는다.

## 7. 구현 방향

후속 구현 대상은 다음으로 제한한다.

| 파일 | 변경 방향 |
| --- | --- |
| `Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.h` | 마지막 selection result summary getter 추가 |
| `Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.cpp` | summary 저장/clear 구현 |
| `Source/Portfolio/Controller/CPlayerController.h` | nearest query result helper 후보 검토 |
| `Source/Portfolio/Controller/CPlayerController.cpp` | nearest command 결과 계산, summary 기록, 로그 출력 |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp` | Enemy panel에 `EnemySelect:` diagnostic line 표시 |

권장 구조:

1. nearest 탐색 결과를 `Selected`, `NoEnemy`, `OutOfRange`, `InvalidContext`로 분류한다.
2. `TrySelectDebugOverlayNearestEnemy()`에서 성공/실패 모두 TargetComponent에 마지막 결과 summary를 기록한다.
3. 실패 시 기존 target clear 정책을 유지한다.
4. HUD는 target이 없어도 마지막 `EnemySelect:` diagnostic line은 표시할 수 있다.
5. `DebugOverlayClearTarget`은 target과 selection diagnostic을 모두 clear할지 여부를 구현 직전 확인한다.

## 8. 기본 정책

- nearest radius `1500.f`는 이번 단계에서 유지한다.
- radius CVar 추가는 P1 후속 또는 P2 후보로 둔다.
- 실패 시 기존 target clear 정책을 유지한다.
- line trace 기반 `DebugOverlaySelectTarget`은 복구하지 않는다.
- `RecentCombatTarget` / `WorldScanFallback` 자동 fallback은 재도입하지 않는다.
- diagnostic line은 최종 evidence 성공 주장에 사용하지 않는다.

## 9. 결정 필요 후보

다음 항목은 구현 중 판단이 필요하면 사용자에게 질문한다.

| 항목 | 권장안 |
| --- | --- |
| Clear 시 diagnostic 유지 여부 | `DebugOverlayClearTarget` 후에는 selection diagnostic도 clear |
| 가장 가까운 Enemy가 반경 밖일 때 이름 표시 여부 | 이름은 표시하지 않고 closest distance만 표시 |
| 거리 단위 표기 | Unreal unit 기준 숫자만 표시 |
| radius CVar화 | 이번 단계에서는 하지 않음 |

Clear 후 diagnostic을 유지하면 방금 전 실패 이유를 볼 수 있지만, 사용자가 명시적으로 clear한 뒤에도 오래된 선택 결과가 남아 혼동될 수 있다. 따라서 P1 기본값은 clear 시 diagnostic도 함께 제거하는 방향을 권장한다.

## 10. 제외 범위

- nearest radius 튜닝
- radius CVar 추가
- line trace select 복구
- combat target system 연동
- 범용 target component 리팩터링
- EventLog filter 변경
- Player/Enemy EventLog 분리
- Runtime LOD actual 표시 구현
- 최종 촬영/패키징

## 11. 구현 전 체크리스트

- `UCDebugOverlayTargetComponent` API 확장 범위가 작은지 확인한다.
- HUD의 `EnemySelect:` line이 성공 evidence처럼 보이지 않는지 확인한다.
- non-shipping guard가 유지되는지 확인한다.
- gameplay flow가 변경되지 않는지 확인한다.
- PIE checklist와 표시 문구가 일치하는지 확인한다.
- 실패 시 기존 target clear 정책이 유지되는지 확인한다.

## 12. 검증 기준

| 시나리오 | 기대 표시 |
| --- | --- |
| Enemy 반경 내 nearest 실행 | `EnemySource: TargetComponent.Nearest`, `EnemySelect: NearestSelected ...` |
| Enemy 반경 밖 nearest 실행 | `EnemySource: None`, `EnemySelect: NearestFailed OutOfRange ...` |
| Enemy 없는 map 또는 유효 Enemy 없음 | `EnemySource: None`, `EnemySelect: NearestFailed NoEnemy ...` |
| `DebugOverlayClearTarget` 실행 | `EnemySource: None`, selection diagnostic clear |
| target 없음 상태 combat event 발생 | `RecentCombatTarget` 자동 표시 없음 |
| target 없음 상태 Enemy 존재 | `WorldScanFallback` 자동 표시 없음 |

## 13. 다음 작업

다음 작업은 `P1 NearestTarget Diagnostic 구현`이다.

구현에서는 TargetComponent selection result field/API 추가, PlayerController nearest result 기록, HUD diagnostic line 표시, PIE checklist 갱신을 한 번에 처리한다.
