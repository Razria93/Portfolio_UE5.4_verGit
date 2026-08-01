# Debug Overlay P1 EventLog Filter PIE Checklist

## 1. 목적

이 문서는 P1 debug overlay의 `Portfolio.DebugOverlay.EventLogFilter`가 PIE에서 의도대로 동작하는지 확인하기 위한 체크리스트다.

검증 대상은 EventLog 표시 제어다. 이 문서는 최종 촬영/패키징 문서가 아니며, filter로 숨겨진 event를 "발생하지 않았다"는 evidence로 해석하지 않는다.

## 2. 사전 조건

| 항목 | 기준 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 맵 | `/Game/00_UnitTest/TestRoom` |
| HUD 연결 | `ACDebugOverlayGameMode` 또는 debug overlay HUD가 연결된 GameMode |
| 실행 환경 | non-shipping PIE |
| asset 저장 | `.umap`, `.uasset` 저장은 의도적으로만 수행 |
| 촬영/패키징 | 이번 단계에서는 수행하지 않음 |

## 3. 기본 CVar

PIE 시작 후 콘솔에서 다음 값을 설정한다.

```text
Portfolio.DebugOverlay.Enabled 1
Portfolio.DebugOverlay.Collect 1
Portfolio.DebugOverlay.EventLogLimit 5
Portfolio.DebugOverlay.EventLogFilter All
```

기존 audit CVar는 overlay collect와 독립이다. EventLog filter 확인에는 `Portfolio.DebugOverlay.*` CVar를 우선 사용한다.

## 4. Filter CVar 시나리오

아래 명령을 순서대로 적용하면서 EventLog header와 표시 line을 확인한다.

```text
Portfolio.DebugOverlay.EventLogFilter All
Portfolio.DebugOverlay.EventLogFilter Execution
Portfolio.DebugOverlay.EventLogFilter Combat
Portfolio.DebugOverlay.EventLogFilter AI
Portfolio.DebugOverlay.EventLogFilter InvalidValue
Portfolio.DebugOverlay.EventLogLimit 0
Portfolio.DebugOverlay.EventLogLimit 5
```

`InvalidValue`는 실제 허용 filter가 아니다. 잘못된 값 입력 시 `All`로 fallback되는지 확인하기 위한 테스트다.

## 5. 기대 표시

| 상태 | 기대 표시 |
| --- | --- |
| 전체 표시 | `[Event Log: All]` |
| Execution filter | `[Event Log: Execution]` |
| Combat filter | `[Event Log: Combat]` |
| AI filter | `[Event Log: AI]` |
| store/snapshot 없음 | `NotCaptured` |
| filter 결과 없음 | `NoEvents(Filter=Execution)` |
| limit 0 | `NoEvents(Filter=Execution Limit=0)` |
| 잘못된 filter 값 | `[Event Log: All]` |

`NoEvents(Filter=...)`는 실패가 아니다. 현재 filter 조건에 맞는 최근 event가 없다는 뜻이다.

## 6. Category별 검증 기준

| Filter | 허용 category | 확인 기준 |
| --- | --- | --- |
| `All` | 전체 | 기존과 동일하게 모든 category가 표시될 수 있다. |
| `Execution` | `Execution` | `Execution/...` line만 표시된다. |
| `Combat` | `Combat`, `CombatResult` | `Combat/...`, `CombatResult/...` line이 표시될 수 있다. |
| `AI` | `AI` | `AI/...` line만 표시된다. |

`CombatResult`는 Combat 계열 event로 취급한다. 따라서 `Portfolio.DebugOverlay.EventLogFilter Combat` 상태에서 `CombatResult/...` line이 보이는 것은 정상이다.

## 7. 테스트 액션 순서

| 순서 | 액션 | 기대 결과 |
| --- | --- | --- |
| 1 | PIE 진입 후 기본 CVar 설정 | `[Debug Overlay P0.5]`, `[Event Log: All]` 표시 |
| 2 | Action 또는 Reaction event 발생 | `Execution/...` event 수집 |
| 3 | `EventLogFilter Execution` 입력 | `Execution/...` line만 표시 |
| 4 | 공격 hit window 또는 target accepted/rejected 유도 | `Combat/...` event 수집 |
| 5 | CombatResult 발생 유도 | `CombatResult/...` event 수집 가능 |
| 6 | `EventLogFilter Combat` 입력 | `Combat/...`, `CombatResult/...` line 표시 |
| 7 | 가능하면 AI combat task event 유도 | `AI/...` event 수집 |
| 8 | `EventLogFilter AI` 입력 | `AI/...` line만 표시 또는 `NoEvents(Filter=AI)` |
| 9 | `EventLogFilter InvalidValue` 입력 | `[Event Log: All]` fallback |
| 10 | `EventLogLimit 0` 입력 | `NoEvents(Filter=... Limit=0)` |
| 11 | `EventLogLimit 5` 입력 | filter 기준 event line 재표시 |

AI event는 현재 테스트 상황에 따라 발생하지 않을 수 있다. 이 경우 `NoEvents(Filter=AI)`가 정상일 수 있으며, AI 성공 evidence로 사용하지 않는다.

## 8. 실패 분기

### 8.1 Header가 이전 형식으로 남아 있음

실패 조건:

```text
[Event Log]
```

기대 조건:

```text
[Event Log: All]
[Event Log: Execution]
[Event Log: Combat]
[Event Log: AI]
```

확인 위치:

- `CDebugOverlayHUD.cpp`
- `FDebugOverlaySnapshotStore::GetEventLogFilter()`

### 8.2 Filter 변경 후 category가 섞임

실패 예:

```text
[Event Log: Execution]
Combat/...
```

기대:

- `Execution` filter에서는 `Execution/...`만 표시된다.
- `AI` filter에서는 `AI/...`만 표시된다.
- `Combat` filter에서는 `Combat/...`, `CombatResult/...`만 표시된다.

확인 위치:

- `FDebugOverlaySnapshotStore.cpp` category match helper
- `GetRecentEventsCopy(...)` filter-aware query

### 8.3 Combat filter에서 CombatResult가 누락됨

실패 조건:

- `CombatResult/...` event가 발생했는데 `EventLogFilter Combat`에서 표시되지 않음

기대:

- `Combat` filter는 `Combat`과 `CombatResult` category를 함께 포함한다.

### 8.4 Filter 결과 없음과 NotCaptured가 구분되지 않음

실패 조건:

```text
[Event Log: AI]
NotCaptured
```

단, snapshot/store 자체가 없는 초기 상태는 `NotCaptured`가 가능하다.

기대:

```text
NoEvents(Filter=AI)
```

### 8.5 EventLogLimit 0인데 event line이 표시됨

실패 조건:

- `Portfolio.DebugOverlay.EventLogLimit 0` 상태에서 event line이 계속 표시됨

기대:

```text
NoEvents(Filter=All Limit=0)
```

또는 현재 filter에 맞는:

```text
NoEvents(Filter=Combat Limit=0)
```

### 8.6 Filter로 숨겨진 event를 미발생처럼 해석함

주의:

- `EventLogFilter Execution` 상태에서 Combat event가 보이지 않는 것은 정상이다.
- 이 상태를 "Combat event가 발생하지 않았다"는 evidence로 사용하지 않는다.
- event 발생 여부 확인은 `All` 또는 해당 category filter에서 다시 확인한다.

## 9. 완료 기준

| 완료 항목 | 기준 |
| --- | --- |
| Header | `All / Execution / Combat / AI` header 확인 |
| Execution filter | `Execution/...`만 표시되는지 확인 |
| Combat filter | `Combat/...`, `CombatResult/...` 표시 확인 |
| AI filter | `AI/...` 또는 `NoEvents(Filter=AI)` 확인 |
| Empty state | `NoEvents(Filter=...)` 확인 |
| Limit 0 | `NoEvents(Filter=... Limit=0)` 확인 |
| Invalid value | 잘못된 filter 값이 `All`로 fallback되는지 확인 |
| Scope guard | Player/Enemy별 EventLog 분리나 EventLog 추가 compact를 성공 기준에 넣지 않음 |

## 10. 결과 기록 템플릿

```text
날짜:
브랜치:
맵:
GameMode/HUD 연결:
CVar:

All:
Execution:
Combat:
AI:
InvalidValue fallback:
EventLogLimit 0:

정상 확인:
보류/실패:
후속 조치:
```

## 11. 다음 단계

이 체크리스트를 기준으로 PIE 수동 검증을 수행한다.

검증 후 다음 작업은 `P1 EventLog Filter PIE 검증 결과 문서화`로 진행한다. 이후 Store subject 분리 설계 또는 Player/Enemy EventLog 분리 설계로 넘어간다.
