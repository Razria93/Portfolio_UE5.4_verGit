# Debug Overlay P1 EventLog Filter Design

## 1. 목적

이 문서는 P1 debug overlay에서 EventLog category filter를 어떻게 설계할지 고정한다.

P0.5 EventLog는 world 단위 최근 event를 그대로 보여준다. 이 구조는 동작 검증에는 충분하지만, Action/Reaction, Combat, AI evidence를 따로 확인할 때 불필요한 event가 섞인다. P1에서는 최종 촬영 전 필요한 category만 확인할 수 있도록 EventLog 표시 제어를 추가한다.

이 작업은 EventLog를 더 축약하거나 Player/Enemy별로 분리하는 작업이 아니다.

## 2. 현재 구조

현재 EventLog 구조는 다음과 같다.

| 항목 | 현재 상태 |
| --- | --- |
| 저장 단위 | `World`별 `FDebugOverlayWorldStore` |
| 저장 구조 | 고정 capacity `32`의 ring buffer |
| 표시 line 수 | `Portfolio.DebugOverlay.EventLogLimit` |
| 표시 limit 기본값 | `5` |
| 표시 limit clamp | `0~5` |
| 표시 순서 | 최신 event 우선 |
| HUD 출력 | `[Event Log]` 아래 `Category/EventName: Summary` |

`FDebugOverlayEventEntry`는 이미 `Category`, `EventName`, `OwnerName`, `SourceName`, `TargetName`, `Summary`를 가진다. 따라서 P1 filter는 새 event schema 없이 `Category` 기반으로 구현할 수 있다.

## 3. P1 필터 범위

P1 필수 category는 다음으로 고정한다.

| Filter | 표시 대상 |
| --- | --- |
| `All` | 모든 EventLog |
| `Execution` | Action / Reaction decision event |
| `Combat` | CombatSignal / CombatResult 계열 event |
| `AI` | AI combat task event |

`CombatResult` category는 Combat 계열로 취급한다. 현재 Store에는 `Combat`과 `CombatResult`가 모두 들어올 수 있으므로, `Combat` filter는 두 category를 함께 포함해야 한다.

## 4. CVar 설계

신규 CVar는 다음으로 둔다.

```text
Portfolio.DebugOverlay.EventLogFilter
```

권장 기본값:

```text
All
```

권장 입력값:

```text
All
Execution
Combat
AI
```

문자열 CVar를 우선한다. 콘솔에서 직접 운용하는 debug evidence 도구이므로 `0/1/2/3`보다 현재 filter 의미가 명확하다. 구현 시에는 대소문자 차이를 허용하되, HUD 표시에는 canonical name을 사용한다.

잘못된 값이 들어오면 `All`로 fallback하고, 성공 evidence처럼 보이지 않도록 필요 시 `EventLogFilter=Invalid->All` 같은 진단 문구를 검토한다.

## 5. 적용 위치 결정

필터는 Store의 record 경로가 아니라 조회/display 경로에 적용한다.

이유:

- Store는 debug evidence의 원본 event cache 역할을 유지해야 한다.
- filter를 바꿔도 기존 ring buffer event가 사라지면 안 된다.
- `All -> Combat -> Execution`처럼 런타임에서 filter를 바꿔도 같은 수집 데이터를 다시 볼 수 있어야 한다.
- `Portfolio.DebugOverlay.Collect`와 filter는 서로 다른 책임이다.

권장 구현은 Store에 filter-aware query를 추가하는 것이다.

```text
Record path:
Debug hook -> SnapshotStore ring buffer에 모든 event 기록

Display path:
HUD -> SnapshotStore에서 filter 기준 recent events copy 조회 -> Canvas 출력
```

HUD가 `Snapshot.RecentEvents`만 후처리하는 방식은 P1 권장안이 아니다. 현재 `Snapshot.RecentEvents`는 이미 `EventLogLimit`이 적용된 최대 5줄 copy이므로, HUD에서 그 뒤에 filter를 적용하면 원하는 category event가 ring buffer 안에 있어도 표시되지 않을 수 있다.

## 6. EventLogLimit 적용 순서

P1 filter 적용 순서는 다음으로 고정한다.

```text
1. ring buffer 최신순 순회
2. EventLogFilter 기준 category match
3. match된 event만 EventLogLimit 개수까지 수집
4. HUD에 최신순으로 표시
```

예시:

```text
EventLogFilter=Combat
EventLogLimit=5
```

이 경우 전체 최신 5줄 안에 Combat이 1줄만 있는지 보지 않는다. ring buffer capacity 32 안에서 최신 Combat 계열 event를 최대 5줄까지 찾는다.

## 7. HUD 표시 정책

EventLog header에는 현재 filter를 표시한다.

```text
[Event Log: All]
[Event Log: Execution]
[Event Log: Combat]
[Event Log: AI]
```

event line format은 P0.5 compact format을 유지한다.

```text
Execution/DecisionResolved: Action(ComboAttack[0]) | Decision=Reject | Apply=None | RejectReason=RejectedByExecutor
Combat/CollisionDisabledIgnored: State=CollisionDisabledIgnored | HitWindow=-1 | Collision=None | Reason=HitWindowNotOpened
```

P1에서는 EventLog 추가 compact를 다시 진행하지 않는다.

## 8. Empty / NotCaptured 표현

필터 결과가 비어 있을 때는 원인을 구분한다.

| 상태 | 표시 |
| --- | --- |
| snapshot/store 자체가 없음 | `NotCaptured` |
| event는 있으나 filter 결과가 없음 | `NoEvents(Filter=Execution)` |
| `EventLogLimit=0` | `NoEvents(Filter=Execution Limit=0)` |

`NoEvents`는 실패가 아니라 현재 filter 조건에 맞는 최근 event가 없다는 뜻이다.

## 9. 구현 영향

예상 구현 영향은 다음과 같다.

| 파일 | 영향 |
| --- | --- |
| `FDebugOverlaySnapshotStore.h` | filter 조회 API 또는 filter enum/string helper 추가 가능 |
| `FDebugOverlaySnapshotStore.cpp` | `Portfolio.DebugOverlay.EventLogFilter` CVar, filter-aware event copy helper 추가 |
| `CDebugOverlayHUD.cpp` | EventLog header에 filter 표시, filtered event list 렌더링 |

`FDebugOverlaySnapshotTypes.h` 구조 변경은 우선 피한다. `FDebugOverlayEventEntry.Category`를 그대로 사용한다.

## 10. Shipping / Build 정책

- CVar 선언은 `#if !UE_BUILD_SHIPPING` 내부에 둔다.
- Shipping에서는 filter query가 empty 또는 false/no-op로 동작한다.
- UMG/Slate dependency를 추가하지 않는다.
- `Build.cs`를 변경하지 않는다.
- 기존 audit log format은 변경하지 않는다.
- Store record 경로에서 event를 버리지 않는다.

## 11. 비목표

이번 P1 EventLog filter에서 하지 않는 작업은 다음과 같다.

- Player/Enemy별 EventLog 분리
- Store subject ownership 재설계
- Recent Execution / Recent Combat / Recent AI 분리
- EventLog 추가 compact
- 최종 촬영/패키징
- capture automation
- gameplay flow 변경
- Shipping HUD화

Player/Enemy별 EventLog 분리는 Store subject 분리 설계 이후 별도 작업으로 진행한다.

## 12. 검증 기준

PIE 검증에서는 다음을 확인한다.

| 시나리오 | 기대 결과 |
| --- | --- |
| `EventLogFilter=All` | 기존과 동일하게 모든 category 표시 |
| `EventLogFilter=Execution` | Execution category만 표시 |
| `EventLogFilter=Combat` | Combat, CombatResult category 표시 |
| `EventLogFilter=AI` | AI category만 표시 |
| filter 결과 없음 | `NoEvents(Filter=...)` 표시 |
| 잘못된 filter 값 | `All` fallback 또는 진단 문구 표시 |
| `EventLogLimit=0` | event line 없음 |

검증 시 실제 event가 발생하지 않은 category를 성공 evidence처럼 설명하지 않는다.

## 13. 다음 구현 단계

다음 구현은 아래 순서로 진행한다.

1. Store에 `Portfolio.DebugOverlay.EventLogFilter` CVar 추가
2. filter canonical name / match helper 추가
3. ring buffer에서 filter 후 limit 적용하는 query 추가
4. HUD EventLog block이 filtered query를 사용하도록 변경
5. PIE checklist를 filter 기준으로 갱신
6. 빌드 및 `git diff --check` 검증

## 14. 결론

P1 EventLog category filter는 Store 기록 정책을 바꾸지 않고, 조회/display 단계에서 category별로 보여줄 event를 제한하는 기능으로 구현한다.

핵심 결정은 `All / Execution / Combat / AI` 문자열 CVar, ring buffer 기준 filter-first 조회, `[Event Log: Filter]` header 표시, `NoEvents(Filter=...)` empty 표현이다.
