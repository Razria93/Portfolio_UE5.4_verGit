# Debug Overlay P1 Closure Criteria

## 1. 목적

이 문서는 debug overlay P1을 어떤 기준으로 마감할지 고정한다.

이 문서는 runtime evidence tooling의 기능과 표시 정책을 닫기 위한 기준으로 작성되었고, 현재는 FinalCandidate evidence package 완료 후 P52 closeout 기준까지 반영한다.

## 2. P1 완료로 보는 항목

### 2.1 Target / Enemy panel

P1 완료 기준:

- `TargetComponent.Nearest` 기반 명시 target 표시
- target 없음 상태에서 `EnemySource: None`
- `DebugOverlaySelectNearestTarget` / `DebugOverlayClearTarget` 운용 가능
- nearest radius `3000`
- nearest diagnostic 표시
- `RecentCombatTarget` / `WorldScanFallback` 자동 fallback 제외

성공 claim:

- selected Enemy 기준 current state를 명시 target source와 함께 확인할 수 있다.

주의:

- 이 기능은 범용 target system이 아니다.
- lock-on, target cycling, combat action target 강제는 P1 범위가 아니다.

### 2.2 3-panel HUD layout

P1 완료 기준:

```text
Pannel_01
-> Player / Enemy actor state
-> actor-local Recent Execution
-> Enemy Current AI
-> Enemy Recent AI Event

Pannel_02
-> EventLog separate panel

Pannel_03
-> Interaction recent summary
```

성공 claim:

- actor current state, world event log, interaction recent summary가 서로 다른 panel 역할로 분리되어 있다.

주의:

- `Pannel` spelling은 현재 표시값 기준으로 유지한다.
- 현재 PIE 캡처는 최종 제출 후보가 아니다.

### 2.3 EventLog

P1 완료 기준:

- EventLog separate panel
- `Portfolio.DebugOverlay.EventLogFilter`
  - `All`
  - `Execution`
  - `Combat`
  - `AI`
- `Portfolio.DebugOverlay.EventLogLimit`
  - `0~32`
- Reject / Ignore noise display filter
- Collision window display filter

성공 claim:

- EventLog 표시 범위를 category와 display filter로 조절할 수 있다.

주의:

- display filter는 화면 표시 제어다.
- filter로 숨겨진 event를 “발생하지 않았다”고 주장하지 않는다.
- Collision lifecycle event는 EventLog diagnostic으로 남을 수 있다.

### 2.4 Recent Execution

P1 완료 기준:

- Player Recent Execution 표시
- Enemy Recent Execution 표시
- Interaction Recent Execution 표시
- multi-line `Key: Value` 표시

성공 claim:

- actor-local execution과 world-level recent execution을 분리해서 볼 수 있다.

### 2.5 Recent Combat

P1 완료 기준:

- Interaction panel에서 Recent Combat 표시
- collision lifecycle event가 Recent Combat 대표값을 덮지 않음
- damage breakdown 표시
  - `Request`
  - `Mitigated`
  - `Final`
  - `Commit`

성공 claim:

- 최근 combat 판정의 attacker / defender / outcome / damage breakdown을 확인할 수 있다.

주의:

- `CollisionEnabled`, `CollisionDisabled`, `CollisionDisabledIgnored`는 Recent Combat 대표값이 아니라 EventLog diagnostic으로 본다.

### 2.6 Enemy Current AI / Recent AI Event

P1 완료 기준:

- Enemy Current AI 표시
  - `Controller`
  - `Pawn`
  - `Target`
  - `IntentState`
  - `ReturnHome`
  - `UsePatrol`
  - `HasLOS`
  - `DistanceToTarget`
  - `IsCombatAction`
- Enemy Recent AI Event 표시
  - `Task`
  - `Result`
  - `Age` 또는 `Stale Time`
  - `Last Pawn`
  - `RejectReason`
  - `Note`

성공 claim:

- selected Enemy의 현재 Blackboard 기반 AI 상태와 최근 AI task event를 분리해서 볼 수 있다.

주의:

- Recent AI Event는 current AI evidence가 아니다.
- Behavior Tree active node 전체 추적은 P1 범위가 아니다.

### 2.7 Player / Enemy current state

P1 완료 기준:

- Player / Enemy panel 분리
- State / Action / Reaction 표시
- HP 표시
- Stagger Count 표시
- Guard 표시
- Movement 표시
- Runtime LOD line 유지

성공 claim:

- Player와 selected Enemy의 현재 action/reaction/combat state를 같은 panel 체계에서 비교할 수 있다.

주의:

- Runtime LOD는 현재 `N/A`이며 성공 evidence로 주장하지 않는다.

## 3. P1 보류 항목

다음 항목은 P1 마감 기준에서 제외한다.

| 항목 | 보류 이유 | 후속 후보 |
| --- | --- | --- |
| Runtime LOD actual 표시 | 현재 `N/A`, 실제 tier source hook 필요 | P2 또는 별도 debug pass |
| Behavior Tree active node 전체 추적 | BT node runtime 추적 범위가 큼 | AI evidence 후속 |
| EventLog line wrapping / compact 재작업 | 현재 separate panel로 가독성 확보 | 후속 UI polish |
| CollisionDisabledIgnored event 자체 발생 원인 제거 | Recent Combat overwrite 문제는 해결, event 발생 원인 분석은 별도 | combat diagnostic 후속 |
| 필요 시 FinalCandidate 보강 캡처 | 핵심 claim은 패키징 완료, Combat/AI empty filter 전용 캡처는 선택 사항 | 선택 보강 |
| 포트폴리오 본문 연결 | FinalCandidate evidence package 이후 진행 | 포트폴리오 문서 후속 |

## 4. Evidence claim 주의

P1 문서와 캡처에서 다음 표현은 피한다.

- `Runtime LOD: N/A`를 실제 Runtime LOD 표시 성공처럼 말하지 않는다.
- EventLog display filter를 event 미발생 증명처럼 말하지 않는다.
- Recent AI Event를 현재 AI 상태처럼 말하지 않는다.
- `BT_Default.uasset` patrol range 조정을 debug overlay HUD 기능 구현처럼 말하지 않는다.
- PIE 검증 캡처를 FinalCandidate 또는 최종 제출 evidence처럼 말하지 않는다.
- Shipping HUD처럼 보이게 설명하지 않는다.

## 5. BT_Default 변경 위치

`Content/02_Controller/02_Enemy/AI/BehaviorTree/BT_Default.uasset` 변경은 P1 debug overlay UI 기능이 아니다.

사용자 확인 기준으로 이 변경은 Patrol 이동 가능 거리 조정이며, P1 PIE 검증 중 Enemy patrol / engage 흐름을 안정적으로 확인하기 위한 tuning이다.

따라서 다음처럼 분리한다.

- P52 PR 설명에는 PIE 검증 보조 tuning으로 기록한다.
- HUD 기능 성공 claim에는 포함하지 않는다.
- asset 변경은 문서/코드 변경과 별도 커밋으로 유지한다.

## 6. P1 마감 판단

P1은 다음 조건을 만족하면 마감 가능하다.

- P1 구현 상태가 P52 PR 문서에 반영되어 있다.
- P1 완료/보류 항목이 이 문서에 분리되어 있다.
- `Debug_Overlay_P1_Integrated_PIE_Result_KR.md` 기준으로 통합 PIE 확인 결과가 기록되어 있다.
- Runtime LOD actual 표시가 보류 항목으로 명시되어 있다.
- FinalCandidate evidence package와 P52 evidence claim 연결 상태가 문서에 반영되어 있다.

현재 판단:

- Runtime LOD actual 표시를 제외한 P1 debug overlay 기능은 마감 가능한 상태다.
- FinalCandidate evidence package는 작성 완료 상태다.
- P52 PR 문서에는 FinalCandidate evidence claim 연결을 반영했다.
- 포트폴리오 본문 evidence claim 연결과 후속 code cleanup은 다음 단계로 분리한다.

## 7. 다음 단계

1. P52 PR 최종 점검 후 브랜치 마감 가능 여부를 판단한다.
2. 포트폴리오 본문 evidence claim에 연결할 스크린샷과 claim 문장을 선별한다.
3. 필요하면 `NotPackaged`로 남은 Combat/AI empty filter 전용 캡처만 별도 보강한다.
4. 후속 브랜치에서 debug overlay code cleanup과 Runtime LOD actual 표시를 검토한다.
