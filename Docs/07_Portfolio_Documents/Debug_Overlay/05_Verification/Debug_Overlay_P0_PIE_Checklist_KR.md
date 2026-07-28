# Debug Overlay P0 PIE 확인 체크리스트

## 1. 목적

이 문서는 TestRoom PIE에서 P0 debug overlay가 정상적으로 표시되고, debug hook을 통해 수집된 snapshot/event 값이 화면에서 갱신되는지 확인하기 위한 검증표다.

검증 목적은 제출 영상 및 기술문서 evidence 확보 전 사전 확인이다. 이 문서는 성능 성공 주장, Shipping HUD 검증, 완성형 UI 품질 검증을 목적으로 하지 않는다.

## 2. 사전 조건

| 항목 | 기준 |
| --- | --- |
| 브랜치 | `feature/debug-overlay-evidence-plan` |
| 테스트 맵 | `/Game/00_UnitTest/TestRoom` |
| HUD 연결 | `ACDebugOverlayGameMode`가 TestRoom `GameMode Override`로 지정되어 있거나, 기존 `GM_Test` 경유로 `ACDebugOverlayHUD`가 연결되어 있어야 한다. |
| 실행 환경 | Non-shipping PIE |
| asset 저장 | `.umap`, `.uasset` 저장은 의도적으로만 수행한다. |
| 전역 설정 | `GlobalDefaultGameMode`, `DefaultEngine.ini`, `Build.cs`는 변경하지 않는다. |

## 3. CVar 세팅

PIE 실행 전 또는 PIE 콘솔에서 다음 값을 설정한다.

```text
Portfolio.DebugOverlay.Enabled 1
Portfolio.DebugOverlay.Collect 1
Portfolio.DebugOverlay.EventLogLimit 5
```

기존 audit log CVar는 필요할 때 별도로 켤 수 있다. 단, overlay collect CVar와 audit log CVar는 독립이다. Audit log가 꺼져 있어도 `Portfolio.DebugOverlay.Collect=1`이면 overlay Store 기록은 가능해야 한다.

## 4. 기본 화면 확인

PIE 시작 직후 좌상단 text block에서 다음 label이 보이는지 확인한다.

| 확인 항목 | 기대 상태 |
| --- | --- |
| `[Debug Overlay P0]` | 표시됨 |
| `ExecutionState` | 현재값 또는 `N/A` |
| `ActiveAction` | 현재값, `None`, 또는 `N/A` |
| `ActiveReaction` | 현재값, `None`, 또는 `N/A` |
| `GuardOverlay` | 현재값 또는 `N/A` |
| `RuntimeLODTier` | 현재값 또는 `N/A` |
| `Recent Execution` | 최근 기록 또는 `NotCaptured` |
| `Recent Combat` | 최근 기록 또는 `NotCaptured` |
| `Recent AI` | 최근 기록 또는 `NotCaptured` |
| `Event Log` | 최근 event 0~5 lines |

## 5. 표시값 기준

| 표시값 | 의미 | Evidence 사용 기준 |
| --- | --- | --- |
| `N/A` | 현재 대상, component, getter를 찾지 못해 조회할 수 없음 | 성공 evidence로 사용하지 않는다. |
| `NotCaptured` | 해당 event가 아직 Store에 기록되지 않음 | capture 전 상태 설명에는 사용 가능하지만 성공 evidence로 사용하지 않는다. |
| `None` | 현재 active action/reaction 등 활성 상태가 없음 | Idle/비활성 상태 evidence로 사용 가능하다. |
| `Pending` | hook 또는 event 흐름상 아직 최종 결과가 확정되지 않음 | 전이 상태 설명에만 사용한다. |

`RuntimeLODTier`는 대상 AI 선택 또는 조회 대상 확정 전까지 `N/A`일 수 있다. `FinalTakenDamage`는 실제 combat result capture 전까지 `NotCaptured`일 수 있다.

## 6. 이벤트별 체크리스트

| 순서 | 테스트 액션 | 기대 overlay 항목 | 기대 표시값 | 실패 시 확인할 위치 | Evidence 사용 가능 여부 |
| --- | --- | --- | --- | --- | --- |
| 1 | TestRoom 진입 후 PIE 시작 | `[Debug Overlay P0]`, 기본 label | Overlay 표시, 미수집 항목은 `NotCaptured` 또는 `N/A` | TestRoom `GameMode Override`, `Portfolio.DebugOverlay.Enabled` | 표시 자체는 환경 evidence로 사용 가능 |
| 2 | 입력 없이 Idle 상태 유지 | `ExecutionState`, `ActiveAction`, `ActiveReaction` | Idle 계열 현재값, `None`, 또는 `N/A` | Player pawn component 조회 경로, HUD getter polling | 현재값이 실제 조회된 경우만 사용 |
| 3 | 이동 입력 수행 | `ExecutionState` | 상태 변화 또는 기존 상태 유지 | Action/Execution component 조회, 입력 mapping | 상태 변화가 overlay에 반영되면 사용 가능 |
| 4 | Action 입력 수행 | `ActiveAction`, `Recent Execution`, `Event Log` | action 이름/summary 갱신, event line 추가 | `RecordActionExecutionResultForAudit`, `Portfolio.DebugOverlay.Collect` | 최소 1개 action event가 확인되면 사용 가능 |
| 5 | Reaction 발생 상황 유도 | `ActiveReaction`, `Recent Execution`, `Event Log` | reaction 이름/summary 갱신, event line 추가 | `RecordReactionExecutionResultForAudit`, reaction component 상태 | 실제 reaction 발생 시 사용 가능 |
| 6 | Guard 입력/해제 | `GuardOverlay` | guard 관련 현재값 변화 또는 `N/A` | `UCObservableOverlayComponent`, `UCDefenseComponent` getter | 실제 조회값일 때만 사용 |
| 7 | 공격 hit window 발생 | `Recent Combat`, `Event Log` | HitWindow open/close 또는 weapon collision window summary | `RecordWeaponCollisionWindowForAudit` | hit window event가 기록되면 사용 가능 |
| 8 | Target accepted/rejected 발생 | `Recent Combat`, `Event Log` | target accepted/rejected summary | `RecordTargetAcceptedForAudit`, `RecordTargetRejectedForAudit` | accepted/rejected 결과 구분이 보이면 사용 가능 |
| 9 | CombatResult received 발생 | `Recent Combat`, `FinalTakenDamage`, `DamageCommit`, `Event Log` | result summary, damage value 또는 `NotCaptured` | `RecordCombatResultDispatchForAudit`, `RecordCombatResultReceivedForAudit` | 실제 result capture 후 사용 가능 |
| 10 | 가능 시 AI combat task success/reject 유도 | `Recent AI`, `RuntimeLODTier`, `Event Log` | AI task success/reject summary, LOD는 현재값 또는 `N/A` | `RecordCombatActionTaskSucceededForAudit`, `RecordCombatActionTaskRejectedForAudit` | 보조 evidence로만 사용 |

## 7. 실패 분기

### 7.1 Overlay가 보이지 않음

확인 순서:

1. PIE 대상 map이 `/Game/00_UnitTest/TestRoom`인지 확인한다.
2. TestRoom World Settings의 `GameMode Override`가 `ACDebugOverlayGameMode` 또는 overlay HUD를 포함한 `GM_Test` 경로인지 확인한다.
3. `Portfolio.DebugOverlay.Enabled 1`이 적용되어 있는지 확인한다.
4. 실행 환경이 Shipping이 아닌지 확인한다.

### 7.2 Event Log가 비어 있음

확인 순서:

1. `Portfolio.DebugOverlay.Collect 1`이 적용되어 있는지 확인한다.
2. Action, Combat, AI event가 실제로 발생했는지 확인한다.
3. 기존 audit log CVar 상태와 무관하게 overlay collect가 독립적으로 동작해야 한다는 점을 확인한다.
4. 관련 hook 연결 범위를 확인한다.

### 7.3 Combat summary가 갱신되지 않음

확인 순서:

1. HitWindow가 실제로 열렸는지 확인한다.
2. target accepted/rejected event가 발생했는지 확인한다.
3. combat result dispatch/receive 경로가 호출되었는지 확인한다.
4. Store 기록이 `Portfolio.DebugOverlay.Collect` gate 뒤에서 차단되지 않는지 확인한다.

### 7.4 Action/Reaction 현재값이 바뀌지 않음

확인 순서:

1. Player pawn 기준 component 조회가 가능한지 확인한다.
2. 현재 action/reaction getter가 overlay에서 읽을 수 있는 공개 API인지 확인한다.
3. 현재값이 아닌 최근 execution event summary가 갱신되는지 분리해서 확인한다.

현재값이 바뀌지 않아도 `Recent Execution`과 `Event Log`가 갱신되면 hook 기반 수집은 동작한 것으로 본다.

### 7.5 `FinalTakenDamage`가 계속 `NotCaptured`

확인 순서:

1. 실제 damage가 commit되는 상황인지 확인한다.
2. combat result received hook이 호출되었는지 확인한다.
3. result packet에서 최종 damage 값을 읽을 수 있는지 확인한다.

Damage가 발생하지 않은 장면의 `NotCaptured`는 실패가 아니다. 단, damage evidence로는 사용할 수 없다.

### 7.6 `RuntimeLODTier`가 `N/A`

확인 순서:

1. overlay가 조회할 대상 AI가 있는지 확인한다.
2. Runtime LOD component 또는 profiling source가 현재 HUD 조회 범위에 들어오는지 확인한다.
3. P0 범위에서 Runtime LOD는 대상 선택 전 `N/A`가 허용되는 항목임을 기록한다.

`RuntimeLODTier=N/A` 상태를 Runtime LOD 성공 evidence로 사용하지 않는다.

## 8. 캡처 전 확인

캡처 전에 다음 항목을 확인한다.

| 항목 | 기준 |
| --- | --- |
| 설명 가능성 | overlay가 debug evidence용 표시임을 설명할 수 있어야 한다. |
| 노출 범위 | Shipping HUD처럼 보이거나 제품 UI처럼 설명하지 않는다. |
| 성능 주장 | FPS, frame time, 성능 성공 주장처럼 보이는 문구를 넣지 않는다. |
| 근거성 | 실제 코드에서 읽지 못한 값은 성공 evidence로 사용하지 않는다. |
| 상태 표현 | `N/A`, `NotCaptured`, `None`, `Pending`을 의미에 맞게 해석한다. |

## 9. 완료 기준

P0 PIE 확인은 다음 조건을 만족하면 완료로 본다.

| 완료 항목 | 기준 |
| --- | --- |
| 기본 표시 | `[Debug Overlay P0]`와 기본 label이 보인다. |
| Action/Reaction | 최소 1개 Action 또는 Reaction event가 `Recent Execution` 또는 `Event Log`에 표시된다. |
| Combat | 최소 1개 Combat event가 `Recent Combat` 또는 `Event Log`에 표시된다. |
| EventLog | `Portfolio.DebugOverlay.EventLogLimit 5` 기준으로 3~5 lines 표시를 확인한다. |
| Snapshot | Store snapshot 기반 recent summary가 event 발생 후 갱신된다. |
| 실패/미수집 표현 | 조회 실패와 미수집 상태가 `N/A`, `NotCaptured`, `None`, `Pending` 기준으로 표시된다. |

AI combat task success/reject는 P0 필수 완료 기준이 아니라 가능 시 확인하는 보조 항목이다. P0 완료 판단은 Action/Reaction, Combat, EventLog, Store snapshot 갱신 확인을 기준으로 한다.

## 10. 결과 기록 템플릿

PIE 확인 후 결과를 아래 형식으로 남긴다.

```text
날짜:
브랜치:
맵:
연결 방식:
CVar:

기본 표시:
Action/Reaction event:
Combat event:
AI event:
EventLog lines:
N/A / NotCaptured 허용 항목:

Evidence 사용 가능 항목:
Evidence 제외 항목:
후속 조치:
```
