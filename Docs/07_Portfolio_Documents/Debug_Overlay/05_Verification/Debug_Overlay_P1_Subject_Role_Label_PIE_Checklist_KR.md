# Debug Overlay P1 Subject Role Label PIE Checklist

## 1. 목적

이 문서는 P1 debug overlay의 Player/Enemy subject EventLog에서 target packet 방향성이 `Incoming / Outgoing / Self`로 표시되는지 확인하기 위한 PIE 검증 절차를 고정한다.

`Incoming / Outgoing / Self` label은 성공/실패 판정이 아니라 subject 기준 방향성 보조 정보다. 즉, 같은 `Combat/TargetAccepted` 또는 `Combat/TargetRejected` event가 Player/Enemy 양쪽 EventLog에 표시될 때, 각 panel 기준으로 해당 event가 들어온 판정인지 나간 판정인지 해석하기 위한 표시다.

이번 검증은 최종 촬영/패키징이 아니라 기능 검증 절차다.

## 2. 사전 조건

- 브랜치: `feature/debug-overlay-evidence-plan`
- TestRoom PIE
- debug overlay HUD 연결
- non-shipping PIE
- `.umap`, `.uasset` 저장은 의도적으로만 수행

기본 CVar:

```text
Portfolio.DebugOverlay.Enabled 1
Portfolio.DebugOverlay.Collect 1
Portfolio.DebugOverlay.EventLogLimit 5
Portfolio.DebugOverlay.EventLogFilter Combat
```

Enemy panel 확인을 위해 다음 command로 명시 target을 선택한다.

```text
DebugOverlaySelectNearestTarget
```

선택 성공 시 Enemy panel에 다음과 같은 source가 표시되어야 한다.

```text
EnemySource: TargetComponent.Nearest
EnemyTarget: Selected=...
```

## 3. Label 의미

| Label | 의미 |
| --- | --- |
| `Incoming` | 현재 subject가 target/receiver 쪽이다. |
| `Outgoing` | 현재 subject가 source/dispatch 쪽이다. |
| `Self` | 현재 subject가 source와 target 양쪽에 모두 해당한다. |

Label은 다음 event에만 붙어야 한다.

- `Combat/TargetAccepted`
- `Combat/TargetRejected`

## 4. 기대 표시

### 4.1 Player가 Enemy를 공격하는 경우

Player panel:

```text
[Event Log: Combat]
Combat/TargetAccepted(Outgoing): ...
```

Enemy panel:

```text
[Event Log: Combat]
Combat/TargetAccepted(Incoming): ...
```

해석:

- Player는 source/dispatch 쪽이므로 `Outgoing`
- Enemy는 target/receiver 쪽이므로 `Incoming`

### 4.2 Enemy가 Player를 공격하거나 Player가 방어/피격 판정을 받는 경우

Player panel:

```text
[Event Log: Combat]
Combat/TargetAccepted(Incoming): ...
```

Enemy panel:

```text
[Event Log: Combat]
Combat/TargetAccepted(Outgoing): ...
```

해석:

- Enemy는 source/dispatch 쪽이므로 `Outgoing`
- Player는 target/receiver 쪽이므로 `Incoming`

### 4.3 TargetRejected 발생

`TargetRejected`도 `TargetAccepted`와 같은 방향성 기준을 사용한다.

```text
Combat/TargetRejected(Incoming): ...
Combat/TargetRejected(Outgoing): ...
```

Reject reason 자체는 기존 summary에서 확인한다.

### 4.4 Self 예외

source와 target이 같은 actor로 기록되는 예외 상황에서는 다음처럼 표시될 수 있다.

```text
Combat/TargetAccepted(Self): ...
```

P1 기본 검증에서 `Self`를 반드시 재현할 필요는 없다. 다만 표시 의미는 문서 기준으로 고정한다.

## 5. Label이 붙지 않아야 하는 항목

다음 event에는 `(Incoming)`, `(Outgoing)`, `(Self)` suffix가 붙으면 안 된다.

- `Execution/*`
- `AI/*`
- `Combat/CollisionEnabled`
- `Combat/CollisionDisabled`
- `Combat/CollisionDisabledIgnored`
- `CombatResult/*`

특히 `CombatResult`는 receiver/target 중심으로 subject panel에 표시될 수 있지만, 이번 role label 대상은 아니다.

## 6. 검증 시나리오

| 순서 | 액션 | 기대 |
| --- | --- | --- |
| 1 | PIE 진입 후 overlay 활성화 | `[Debug Overlay P0.5]` 표시 |
| 2 | `DebugOverlaySelectNearestTarget` 실행 | `EnemySource: TargetComponent.Nearest` |
| 3 | `Portfolio.DebugOverlay.EventLogFilter Combat` 입력 | Player/Enemy panel의 `[Event Log: Combat]` 표시 |
| 4 | Player attack으로 target packet 발생 | Player `Outgoing`, Enemy `Incoming` |
| 5 | Enemy attack / block / parry / hit 상황 발생 | Player `Incoming`, Enemy `Outgoing` |
| 6 | 가능하면 TargetRejected 상황 발생 | `TargetRejected(Incoming/Outgoing)` 확인 |
| 7 | `Portfolio.DebugOverlay.EventLogLimit 0` 입력 | `NoEvents(Filter=Combat Limit=0)` 유지 |
| 8 | `Portfolio.DebugOverlay.EventLogFilter All` 입력 | target packet label 유지 |

## 7. 실패 분기

다음 상황은 실패 또는 재확인 대상으로 본다.

- `Combat/TargetAccepted` 또는 `Combat/TargetRejected`에 label이 없음
- `Execution`, `AI`, `CombatResult`, `Collision` event에 label이 붙음
- Player/Enemy 양쪽 label이 모두 같은 방향으로 표시됨
- `Incoming` / `Outgoing`이 source-target 관계와 반대로 보임
- `EventLogLimit 0` 상태에서 event line이 계속 표시됨
- target이 없는데 Enemy panel이 성공 evidence처럼 보임
- `TargetComponent.Nearest` 선택 전 Enemy panel의 EventLog가 target 기반 evidence처럼 보임

## 8. 완료 기준

- `TargetAccepted(Incoming)` 확인
- `TargetAccepted(Outgoing)` 확인
- 가능하면 `TargetRejected(Incoming)` 또는 `TargetRejected(Outgoing)` 확인
- label이 target packet에만 붙는지 확인
- 기존 `EventLogFilter` / `EventLogLimit` 정책 유지 확인
- Player/Enemy EventLog 분리 유지 확인
- label을 성공/실패 판정이 아니라 방향성 보조 정보로 설명할 수 있음

## 9. 다음 작업

다음 작업은 `P1 Subject Role Label PIE 검증 결과 문서화`다.

PIE에서 확인한 실제 결과를 기록하고, `TargetRejected` 재현 여부나 `Self` 미검증 여부가 있으면 보류 항목으로 분리한다.
