# Debug Overlay P1 EventLog Noise Filter Design

## 1. 목적

이 문서는 P1 debug overlay의 Interaction EventLog에서 Reject / Ignore / Collision window 계열 로그를 어떻게 표시 제어할지 결정한다.

Interaction panel은 `Action -> Window Open -> Hit -> 판정 -> Result` 흐름을 보는 영역이다. 따라서 EventLog 자체는 유지해야 하지만, 캡처나 검증 중에는 일부 diagnostic event가 너무 많이 표시되어 핵심 흐름을 가릴 수 있다.

이번 설계의 목표는 다음과 같다.

- EventLog의 interaction 흐름 추적 능력은 유지한다.
- Reject / Ignore 계열 노이즈를 표시 단계에서 숨길 수 있게 한다.
- Collision window event를 표시 단계에서 켜고 끌 수 있게 한다.
- Store 원본 event 수집은 줄이지 않는다.

이번 단계는 설계 문서 작업이다. 코드 구현, 촬영, 패키징, asset/config/Build.cs 변경은 하지 않는다.

## 2. 현재 문제

현재 P1 EventLog는 category filter를 통해 `All / Execution / Combat / AI` 단위로 표시를 제어한다.

이 방식은 큰 category를 나누는 데에는 충분하지만, 다음과 같은 세부 노이즈를 제어하지 못한다.

| Event | 문제 |
| --- | --- |
| `Execution/DecisionResolved` + `Decision: Reject` | 입력 타이밍 실패나 실행자 거절이 반복되면 EventLog 대부분을 차지할 수 있음 |
| `Execution/DecisionResolved` + `Decision: Ignore` | Guard Out / parry timing 등에서 정상적인 무시 흐름이 많아질 수 있음 |
| `Combat/CollisionEnabled` | window open 흐름 확인에는 유용하지만 일반 캡처에서는 노이즈가 될 수 있음 |
| `Combat/CollisionDisabled` | window close 흐름 확인에는 유용하지만 일반 캡처에서는 노이즈가 될 수 있음 |
| `Combat/CollisionDisabledIgnored` | 실패 원인 확인에는 유용하지만 반복되면 핵심 combat result를 가림 |

따라서 category filter와 별도로 display-only 세부 필터가 필요하다.

## 3. 필터 계층

P1 EventLog 표시 필터는 다음 계층으로 나눈다.

| 계층 | CVar | 역할 |
| --- | --- | --- |
| Category filter | `Portfolio.DebugOverlay.EventLogFilter` | `All / Execution / Combat / AI` category 선택 |
| Noise filter | `Portfolio.DebugOverlay.HideNoiseEvents` | Reject / Ignore 계열 event 숨김 |
| Collision window filter | `Portfolio.DebugOverlay.HideCollisionWindowEvents` | Collision window event 숨김 여부 |

정책:

- category filter는 기존 정책을 유지한다.
- noise/collision filter는 category filter 이후에 적용한다.
- Store record path에서는 어떤 event도 버리지 않는다.
- 숨겨진 event를 "발생하지 않았다"는 evidence로 해석하지 않는다.

## 4. CVar 설계

### 4.1 HideNoiseEvents

```text
Portfolio.DebugOverlay.HideNoiseEvents
```

권장 타입:

```text
int32 / bool style CVar
```

권장 기본값:

```text
0
```

의미:

| 값 | 의미 |
| --- | --- |
| `0` | Reject / Ignore 계열 event를 숨기지 않음 |
| `1` | Reject / Ignore 계열 event를 EventLog 표시에서 숨김 |

기본값을 `0`으로 두는 이유:

- Debug overlay는 기본적으로 발생한 event를 숨기지 않는 편이 안전하다.
- 검증 중 실패 원인을 놓치지 않는다.
- 캡처 정리나 노이즈가 심한 상황에서만 사용자가 명시적으로 켠다.

### 4.2 HideCollisionWindowEvents

```text
Portfolio.DebugOverlay.HideCollisionWindowEvents
```

권장 타입:

```text
int32 / bool style CVar
```

권장 기본값:

```text
0
```

의미:

| 값 | 의미 |
| --- | --- |
| `0` | Collision window event를 EventLog에 표시 |
| `1` | Collision window event를 EventLog 표시에서 숨김 |

기본값을 `0`으로 두는 이유:

- P1 Interaction panel은 flow 확인이 목적이다.
- `CollisionEnabled -> TargetAccepted -> CombatResult` 순서는 combat evidence에서 중요하다.
- 사용자가 필요할 때만 숨기는 방식이 원본 흐름 확인에 더 안전하다.

## 5. HideNoiseEvents 대상

`HideNoiseEvents=1`일 때 숨길 후보는 다음으로 제한한다.

| Category | Event | 조건 | 판단 |
| --- | --- | --- | --- |
| `Execution` | `DecisionResolved` | `Decision: Reject` | 숨김 |
| `Execution` | `DecisionResolved` | `Decision: Ignore` | 숨김 |
| `Execution` | `DecisionResolved` | `RejectReason`이 `None`이 아님 | 숨김 |
| `Combat` | `CollisionDisabledIgnored` | 전체 | 숨김 후보 |

주의:

- `Decision: Accept`는 숨기지 않는다.
- `Combat/TargetAccepted`, `Combat/TargetRejected`는 숨기지 않는다.
- `CombatResult` 계열은 숨기지 않는다.
- Reject/Ignore가 중요한 디버깅 상황에서는 `HideNoiseEvents=0`으로 두어야 한다.

`Combat/CollisionDisabledIgnored`는 noise와 collision window 성격을 모두 가진다. P1 구현에서는 다음 중 하나로 처리한다.

```text
HideNoiseEvents=1이면 숨김
HideCollisionWindowEvents=1이면 숨김
```

즉 둘 중 하나라도 숨김 조건에 걸리면 표시하지 않는다.
구현 기준은 OR 조건이다.

다만 `CollisionDisabledIgnored`는 hit window 실패 원인을 확인할 때 유용하다. 원인 분석이나 hook 검증을 할 때는 `HideNoiseEvents=0`, `HideCollisionWindowEvents=0` 기본 조합으로 되돌린 뒤 확인한다.

## 6. Collision Window 대상

`HideCollisionWindowEvents=1`일 때 숨길 대상은 다음으로 제한한다.

| Category | Event | 표시 정책 |
| --- | --- | --- |
| `Combat` | `CollisionEnabled` | 숨김 |
| `Combat` | `CollisionDisabled` | 숨김 |
| `Combat` | `CollisionDisabledIgnored` | 숨김 |

구현 메모:

- EventLog 표시 문자열은 `Category/EventName` 조합으로 보이지만, 실제 필터는 `FDebugOverlayEventEntry.Category`와 `EventName`을 기준으로 판단한다.
- `Category`와 `EventName`은 표시 직전 `TrimStartAndEnd` 후 대소문자 무시 비교를 적용한다.
- collision window event는 `Combat` category 안에서 `CollisionEnabled*`, `CollisionDisabled*` prefix로 판별한다.
- 이 방식은 `CollisionDisabledIgnored`처럼 `CollisionDisabled` 계열의 세부 event도 함께 숨기기 위한 것이다.
- `TargetAccepted`, `TargetRejected`, `CombatResult/*`는 `Collision*` prefix가 아니므로 `HideCollisionWindowEvents=1`로 숨기지 않는다.
- PIE에서 CVar를 켰는데도 숨김이 반영되지 않으면 Live Coding/Hot Reload 잔존 DLL 가능성을 먼저 확인하고, 에디터 재시작 후 다시 검증한다.

숨기지 않는 대상:

| Category | Event | 이유 |
| --- | --- | --- |
| `Combat` | `TargetAccepted` | 실제 attacker/defender 판정 결과 |
| `Combat` | `TargetRejected` | 실제 target 판정 실패 결과 |
| `CombatResult` | `PacketReceived` | receiver-side result evidence |
| `CombatResult` | `Delivering` / `Delivered` | 별도 result diagnostic이며 window event는 아님 |

Collision window event는 flow 확인에 필요할 수 있으므로 기본 표시를 유지한다. 단, 캡처 화면에서 너무 많은 window open/close가 반복되면 `HideCollisionWindowEvents=1`으로 숨길 수 있다.

## 7. 적용 순서

EventLog 표시 query는 다음 순서로 동작해야 한다.

```text
1. ring buffer 최신순 순회
2. category filter match
3. noise display exclusion check
4. collision window display exclusion check
5. EventLogLimit 개수까지 수집
6. HUD 표시
```

중요한 점은 `EventLogLimit`보다 noise/collision filter를 먼저 적용한다는 것이다.

이유:

- 최신 5개 event가 모두 숨김 대상이면, 그 뒤에 있는 유효 event를 보여줄 수 있어야 한다.
- filter 후 limit을 적용해야 "현재 표시 조건에 맞는 최근 event"가 된다.
- Store ring buffer 원본은 그대로 유지되므로 filter 값을 바꾸면 같은 수집 데이터에서 다른 표시를 볼 수 있다.

## 8. Empty State

noise/collision filter 적용 후 표시할 event가 없으면 기존 empty state 정책을 유지한다.

| 상태 | 표시 |
| --- | --- |
| snapshot/store 없음 | `NotCaptured` |
| limit 0 | `NoEvents(Filter: Combat Limit: 0)` |
| category + noise/collision filter 결과 없음 | `NoEvents(Filter: Combat)` |

P1 구현에서 filter 상태까지 표시할지 여부는 구현 단계에서 검토한다.

후보:

```text
NoEvents(Filter: Combat)
NoEvents(Filter: Combat NoiseHidden: true CollisionWindow: false)
```

권장:

- P1에서는 기존 `NoEvents(Filter: ...)` 형식을 유지한다.
- 이때 `NoEvents`는 category filter, noise filter, collision window filter를 모두 적용한 뒤 표시할 event가 없다는 뜻이다.
- header 또는 별도 diagnostic에 filter 상태를 추가할 필요가 생기면 후속 작업으로 분리한다.

## 9. 표시 예시

### 9.1 기본값

```text
Portfolio.DebugOverlay.EventLogFilter All
Portfolio.DebugOverlay.HideNoiseEvents 0
Portfolio.DebugOverlay.HideCollisionWindowEvents 0
```

예상:

```text
[Event Log: All]
Execution/DecisionResolved:
Owner: BP_CPlayer_C_0
Domain: Action
Subject: Guard Out
Decision: Ignore
Apply: None
RejectReason: None

Combat/CollisionDisabledIgnored:
State: CollisionDisabledIgnored
HitWindow: -1
Collision: None
Reason: HitWindowNotOpened
```

### 9.2 noise 숨김

```text
Portfolio.DebugOverlay.HideNoiseEvents 1
```

예상:

- `Decision: Reject` execution event 숨김
- `Decision: Ignore` execution event 숨김
- `Combat/CollisionDisabledIgnored` 숨김
- `Decision: Accept`, `TargetAccepted`, `CombatResult`는 유지

### 9.3 Collision window 숨김

```text
Portfolio.DebugOverlay.HideCollisionWindowEvents 1
```

예상:

- `Combat/CollisionEnabled` 숨김
- `Combat/CollisionDisabled` 숨김
- `Combat/CollisionDisabledIgnored` 숨김
- `Combat/TargetAccepted`, `CombatResult/PacketReceived`는 유지

### 9.4 category + noise/collision 조합

```text
Portfolio.DebugOverlay.EventLogFilter Combat
Portfolio.DebugOverlay.HideNoiseEvents 1
Portfolio.DebugOverlay.HideCollisionWindowEvents 1
```

예상:

```text
[Event Log: Combat]
Combat/TargetAccepted:
Attacker: BP_CEnemy_C_1
Defender: BP_CPlayer_C_0
Outcome: Parry
Final: 0.000
Commit: 0.000
Accepted: true

CombatResult/PacketReceived:
ResultFrom: BP_CEnemy_C_1
ResultReceiver: BP_CPlayer_C_0
Outcome: Parry
DamageCommitted: false
Commit: 0.000
```

이 조합은 최종 캡처 전 interaction 핵심 흐름만 보고 싶을 때 사용할 수 있다.

## 10. Recent Summary와의 관계

이번 설계는 EventLog 표시 전용이다.

대상:

```text
[Interaction]
[Event Log: ...]
```

기본 비대상:

```text
[Recent Execution]
[Recent AI]
```

이유:

- Recent는 현재 가장 중요한 summary를 빠르게 보는 영역이다.
- Reject/Ignore가 최근 상태 자체를 설명하는 경우도 있으므로 숨기면 오해가 생길 수 있다.
- Recent filtering은 별도 정책과 CVar가 필요하다.

단, `Recent Combat`은 예외 정책을 둔다.

- `Recent Combat`은 최근 Combat 로그가 아니라 최근 전투 판정/결과 evidence summary다.
- `CollisionEnabled`, `CollisionDisabled`, `CollisionDisabledIgnored`는 collision lifecycle diagnostic이므로 `Recent Combat` 갱신 대상에서 제외한다.
- 위 collision lifecycle event는 EventLog에는 기록하되, `TargetAccepted`, `TargetRejected`, `CombatResult/*`가 만든 의미 있는 combat summary를 덮어쓰지 않는다.
- `HideCollisionWindowEvents`는 EventLog 표시 제어용이며, `Recent Combat`은 CVar와 무관하게 collision lifecycle event로 갱신하지 않는다.

Recent Execution / Recent AI까지 noise filter를 적용할지는 P1 후속 결정으로 둔다.

## 11. 구현 영향

예상 구현 파일:

| 파일 | 영향 |
| --- | --- |
| `FDebugOverlaySnapshotStore.h` | 필요 시 filter query helper 선언 보강 |
| `FDebugOverlaySnapshotStore.cpp` | CVar 추가 예정, display filter helper 구현 후보 |
| `CDebugOverlayHUD.cpp` | 필요 시 header/empty 표시 문구 확인 |

권장 구현 방향:

- CVar는 non-shipping에서만 선언/사용한다.
- Store record path는 변경하지 않는다.
- EventLog query path에만 `ShouldDisplayEventEntry` 계열 helper를 둔다.
- Interaction panel이 사용하는 `GetRecentEventsCopy(...)`에 우선 적용한다.
- `GetRecentEventsForSubjectCopy(...)`는 현재 기본 HUD path에서 사용하지 않으므로, subject-specific EventLog를 다시 사용할 때 같은 helper 적용 여부를 후속으로 판단한다.

권장:

- Interaction EventLog에는 반드시 적용한다.
- subject-specific EventLog가 다시 필요해질 경우 같은 helper를 공유한다.

## 12. 비목표

이번 설계와 후속 1차 구현에서 하지 않을 작업:

- Store schema 변경
- Store record path 변경
- EventLog compact 재작업
- Player/Enemy EventLog 재분리
- Recent Execution / Recent AI summary filtering
- Runtime LOD actual 표시
- AI detail 보강
- final capture packaging
- UMG/Slate 전환
- Shipping HUD화
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 13. 검증 기준

후속 구현 후 PIE에서 다음을 확인한다.

| 시나리오 | 기대 결과 |
| --- | --- |
| 기본값 | 기존 EventLog와 동일하게 표시 |
| `HideNoiseEvents 1` | Reject / Ignore execution event가 표시에서 제외 |
| `HideNoiseEvents 1` | `CollisionDisabledIgnored`가 표시에서 제외 |
| `HideCollisionWindowEvents 1` | `CollisionEnabled/Disabled/DisabledIgnored`가 표시에서 제외 |
| `EventLogFilter Combat` + collision off | `TargetAccepted`, `CombatResult` 중심 표시 |
| `EventLogLimit 0` | 기존 limit 0 empty state 유지 |
| filter 값을 되돌림 | Store에 남아 있던 event가 다시 표시 가능 |

검증 시 주의:

- 숨긴 event를 미발생 evidence로 해석하지 않는다.
- Store ring buffer 원본 수집이 유지되는지 확인한다.
- 최종 촬영 후보가 아니라 기능 검증으로 본다.

## 14. 완료 기준

이번 설계 문서의 완료 기준은 다음과 같다.

- category filter와 noise/collision filter의 역할 차이가 정리되어 있다.
- `HideNoiseEvents`, `HideCollisionWindowEvents` CVar 후보와 기본값이 정리되어 있다.
- 숨길 event와 숨기지 않을 event가 구분되어 있다.
- filter 적용 순서가 `category -> noise/collision -> limit`으로 고정되어 있다.
- Store record path를 변경하지 않는 정책이 명시되어 있다.
- Recent Combat은 collision lifecycle event로 갱신하지 않는 예외 정책을 명시했다.
- Recent Execution / Recent AI summary는 이번 필터 대상이 아님을 명시했다.
- 후속 구현과 PIE 검증 기준이 정리되어 있다.

## 15. 다음 작업

다음 작업은 `P1 EventLog Noise / Collision Window Filter 구현`이다.

권장 구현 범위:

```text
FDebugOverlaySnapshotStore.cpp 중심 display filter helper 추가
Portfolio.DebugOverlay.HideNoiseEvents 추가
Portfolio.DebugOverlay.HideCollisionWindowEvents 추가
EventLog query path에서 category -> noise/collision -> limit 순서 적용
CDebugOverlayHUD.cpp는 필요 시 최소 표시 문구만 확인
```

코드 구현 후에는 PIE checklist를 갱신하고 수동 검증을 진행한다.
