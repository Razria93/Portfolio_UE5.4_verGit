# Debug Overlay P1 EventLog Filter PIE Result

## 1. 목적

이 문서는 P1 debug overlay의 `Portfolio.DebugOverlay.EventLogFilter`가 PIE에서 의도대로 동작했는지 기록한다.

이번 결과는 최종 촬영/패키징 후보가 아니다. EventLog category filter가 `All / Execution / Combat / AI` 기준으로 표시를 제어하고, empty state와 invalid value fallback이 문서 기준과 일치하는지 확인한 기능 검증 결과다.

## 2. 검증 전제

| 항목 | 내용 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 맵 | TestRoom PIE |
| Overlay 표시 | `Portfolio.DebugOverlay.Enabled 1` |
| Overlay 수집 | `Portfolio.DebugOverlay.Collect 1` |
| EventLog line | `Portfolio.DebugOverlay.EventLogLimit 5` |
| EventLog filter | `Portfolio.DebugOverlay.EventLogFilter` |
| 관련 target command | `DebugOverlaySelectNearestTarget`, `DebugOverlayClearTarget` |

EventLog filter는 표시 제어 기능이다. filter로 숨겨진 event를 "발생하지 않았다"는 evidence로 해석하지 않는다.

## 3. 확인 명령

PIE에서 다음 명령 조합을 확인했다.

```text
Portfolio.DebugOverlay.EventLogFilter All
Portfolio.DebugOverlay.EventLogFilter Execution
Portfolio.DebugOverlay.EventLogFilter Combat
Portfolio.DebugOverlay.EventLogFilter AI
Portfolio.DebugOverlay.EventLogFilter InvalidValue
Portfolio.DebugOverlay.EventLogLimit 0
Portfolio.DebugOverlay.EventLogLimit 5
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
```

## 4. 검증 결과 요약

| 시나리오 | 기대값 | 실제값 | 판단 |
| --- | --- | --- | --- |
| `EventLogFilter All` | 전체 category 표시 | 정상 동작 | 통과 |
| `EventLogFilter Execution` | `Execution/...` 기준 표시 | 정상 동작 | 통과 |
| `EventLogFilter Combat` | `Combat/...`, `CombatResult/...` 기준 표시 | 정상 동작 | 통과 |
| `EventLogFilter AI` | `AI/...` 기준 표시 또는 `NoEvents(Filter=AI)` | 정상 동작 | 통과 |
| filter 결과 없음 | `NoEvents(Filter=...)` 표시 | 정상 동작 | 통과 |
| `EventLogLimit 0` | `NoEvents(Filter=... Limit=0)` 표시 | NoEvents 상태 확인 | 통과 |
| invalid filter value | `All` fallback | fallback 정상 동작 | 통과 |
| `DebugOverlaySelectNearestTarget` | nearest target 선택 | 정상 동작 | 통과 |
| `DebugOverlayClearTarget` | target clear | 정상 동작 | 통과 |

## 5. 성공 판단

이번 PIE 확인에서 다음 정책이 충족되었다.

- EventLog header와 표시 line이 `All / Execution / Combat / AI` filter 기준으로 전환된다.
- filter 결과가 없을 때 `NoEvents(Filter=...)` 계열 상태로 표시된다.
- `EventLogLimit 0` 상태에서 event line이 표시되지 않고 NoEvents 상태가 표시된다.
- 잘못된 filter 값은 `All`로 fallback된다.
- target command인 `DebugOverlaySelectNearestTarget`과 `DebugOverlayClearTarget`도 현재 P1 명시 target 정책 기준으로 정상 동작한다.

따라서 P1 EventLog category filter는 현재 구두 검증 기준으로 동작 확인된 것으로 본다.

## 6. 주의점

- 이번 결과는 사용자의 PIE 수동 확인을 기준으로 기록한 검증 결과다.
- 최종 제출용 캡처 후보가 아니며, FinalCandidate 패키징으로 승격하지 않는다.
- filter는 표시 제어일 뿐 event 발생 여부 자체를 증명하지 않는다.
- `AI` filter에서 `NoEvents(Filter=AI)`가 표시되는 것은 해당 시점의 AI category event가 없다는 뜻이며, AI 시스템 성공 evidence가 아니다.
- Player/Enemy별 EventLog 분리는 아직 구현하지 않았다.
- EventLog 추가 compact는 이번 검증 범위가 아니다.

## 7. Evidence Claim 범위

이번 결과로 주장 가능한 범위:

- P1 debug overlay는 EventLog를 category별로 표시 제어할 수 있다.
- `All / Execution / Combat / AI` filter가 PIE에서 정상 동작한다.
- EventLog empty state와 limit 0 state가 구분되어 표시된다.
- invalid filter value는 `All` fallback으로 처리된다.

이번 결과로 주장하지 않는 범위:

- Player/Enemy별 EventLog 분리
- EventLog category별 event 발생 보장
- AI detail 성공 표시
- Runtime LOD 성공 표시
- 최종 제출용 evidence 품질
- capture automation

## 8. 보류/실패 항목

현재 구두 검증 기준으로 명확한 실패 항목은 없다.

보류 항목:

- 캡처 파일 기반 최종 증빙은 아직 수행하지 않는다.
- Player/Enemy별 EventLog 분리는 Store subject 분리 설계 이후 판단한다.
- Runtime LOD / AI 표시 보강은 후속 검토로 남긴다.

## 9. 다음 단계

다음 작업은 `Store Subject 분리 설계`로 진행한다.

현재 EventLog는 world 단위 공통 로그이므로, Player/Enemy별 EventLog 분리를 결정하기 전에 source/target/owner 기준을 먼저 확정해야 한다.
