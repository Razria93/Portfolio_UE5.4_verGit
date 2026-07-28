# Debug Overlay P0 Final Decision

## 1. 문서 목적

이 문서는 P0 debug overlay 구현 직전에 더 이상 흔들리면 안 되는 결정 사항을 고정한다.

P0 overlay의 목적은 제출 영상과 기술문서에서 사용할 evidence를 화면에 함께 노출하는 것이다. 완성형 플레이 HUD, 운영 HUD, 성능 계측 UI가 아니다. 모든 구현은 개발 전용 gate를 전제로 하며, Shipping 노출을 방지한다.

이 문서에 없는 항목은 P0 구현 중 새로 추가하지 않는다. 추가 필요가 발견되면 P1 후보로 분리해 문서화한다.

## 2. P0 구현 범위

### 2.1 P0에서 구현한다

- 개발 전용 debug overlay snapshot type
- 개발 전용 debug overlay snapshot store
- Canvas Draw 기반 최소 HUD renderer
- P0 evidence 항목의 현재값 조회 또는 최근 event 조회
- EventLog 3~5 lines 표시
- CVar 기반 표시/수집 gate
- 테스트/촬영 맵 한정 HUD 연결

### 2.2 P0에서 제외한다

- 전역 `GlobalDefaultGameMode` 변경
- UMG/Slate 기반 UI
- Shipping build용 표시 경로
- CSV profiler counter 추가
- 성능 성공 주장처럼 보이는 FPS/최적화 수치 표시
- 기존 gameplay 흐름을 바꾸는 상태 전이 추가
- debug overlay 전용 gameplay state 복제
- 장기 actor raw pointer 보관

### 2.3 P1 이후로 넘긴다

- Runtime LOD interval selection 상세 표시
- AI blackboard intent 상세 표시
- AI request/distance/LOS 상세 표시
- BT state 상세 표시
- preset별 레이아웃 확장
- 복수 actor 선택/필터링
- 캡처 자동화와 overlay preset 연동
- 전역 GameMode/HUD 연결 여부 결정

## 3. 표시 항목 최종 확정

| 표시 항목 | P0 표시 정책 | 값 없을 때 |
| --- | --- | --- |
| ExecutionState | Action/Reaction 실행 상태를 현재값 또는 최근 decision 기준으로 표시 | `N/A` |
| ActiveAction | 현재 action component getter 또는 최근 action decision 기준 표시 | `None` |
| ActiveReaction | 현재 reaction component getter 또는 최근 reaction decision 기준 표시 | `None` |
| GuardOverlay | observable/defense getter 기반 현재 guard overlay 표시 | `N/A` |
| HitWindow | combat signal hook에서 기록한 최근 collision window 표시 | `NotCaptured` |
| DefenseOutcome | target accepted/rejected 또는 defense result 최근값 표시 | `NotCaptured` |
| FinalTakenDamage | combat result receive/dispatch 최근값 표시 | `NotCaptured` |
| DamageCommit | damage commit 발생 여부와 최근 commit 결과 표시 | `NotCaptured` |
| RuntimeLODTier | runtime LOD component 또는 profiling helper에서 확인 가능한 tier 표시 | `N/A` |
| EventLog | 최근 주요 debug event 3~5 lines 표시 | empty |

표시 문구는 성공 evidence처럼 과장하지 않는다. 현재 조회값과 최근 event값은 혼동되지 않도록 label 또는 prefix로 구분한다.

## 4. 파일명 최종 결정

P0 구현 파일은 `Source/Portfolio/Core/Debug` 아래에 둔다.

- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h`
- `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp`
- `Source/Portfolio/Core/Debug/CDebugOverlayHUD.h`
- `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp`

판단 근거:

- 기존 audit/debug helper가 `Core/Debug`에 모여 있다.
- P0 overlay는 gameplay feature가 아니라 evidence/debug feature다.
- UMG/Slate 의존성을 추가하지 않고 `Engine`의 Canvas Draw만 사용한다.
- HUD renderer도 Shipping 전용 UI가 아니므로 `UI`보다 `Core/Debug`가 더 안전하다.

## 5. HUD 연결 방식 최종 결정

P0에서는 전역 `GlobalDefaultGameMode` 변경을 피한다.

최종 정책:

- 테스트/촬영 맵 한정 GameMode 또는 World Settings 연결을 우선한다.
- 기존 전역 `GlobalDefaultGameMode=/Script/Engine.GameMode` 설정은 P0에서 유지한다.
- 전역 GameMode 변경은 P1 이후 별도 결정으로 둔다.
- overlay가 필요한 capture map에서만 `CDebugOverlayHUD`가 활성화되도록 한다.

이 결정은 제출 evidence 촬영에 필요한 최소 범위를 충족하면서, 기존 gameplay map의 HUD/GameMode 동작을 오염시키지 않기 위한 것이다.

## 6. CVar 이름 최종 결정

P0 CVar는 기존 debug audit CVar와 분리해 `Portfolio.DebugOverlay.*` 네임스페이스를 사용한다.

- `Portfolio.DebugOverlay.Enabled`
- `Portfolio.DebugOverlay.Collect`
- `Portfolio.DebugOverlay.Preset`
- `Portfolio.DebugOverlay.EventLogLimit`

정책:

- `Enabled`: HUD 표시 여부
- `Collect`: hook/store 기록 여부
- `Preset`: 표시 preset 선택
- `EventLogLimit`: event log 표시/보관 상한

`Enabled`가 꺼져도 `Collect`가 켜져 있으면 최근 evidence 수집은 가능하다. 다만 Shipping에서는 모든 경로가 no-op 또는 false로 동작해야 한다.

## 7. Store 정책 최종 결정

Snapshot store는 gameplay state의 소유자가 아니다. overlay에 필요한 최근 debug evidence를 보관하는 개발 전용 보조 저장소다.

최종 정책:

- getter 기반 현재값은 HUD draw 시점에 대상 component에서 직접 조회한다.
- packet/result/event 기반 값은 store에 최근값으로 기록한다.
- actor raw pointer를 장기 보관하지 않는다.
- 필요 시 actor 식별자는 name, weak reference, frame/time 정보로 제한한다.
- event log는 고정 크기 ring buffer로 관리한다.
- snapshot 조회 API는 copy를 반환한다.
- Shipping API는 no-op 또는 false를 반환한다.
- PIE/world 전환 시 stale snapshot을 피할 reset 경로를 둔다.

현재값과 최근값의 의미가 다르므로 snapshot field 이름과 HUD label에서 이를 분리한다.

## 8. Hook 최종 후보

### 8.1 P0 Hook

- Action / Reaction decision
- CombatSignal target accepted/rejected
- CombatResult dispatch/receive
- AI combat task success/reject

### 8.2 P0 보조 Hook

- weapon collision window open/close
- defense outcome 결정 지점
- damage commit 직후 결과 기록

### 8.3 P1 또는 보류

- Runtime LOD interval selection 상세 hook
- blackboard intent 상세 hook
- distance/LOS 상세 hook
- BT state 상세 hook

RuntimeLODTier는 P0 표시 항목에 포함하지만, interval selection 세부 값은 P1 또는 보조 hook으로 분류한다.

## 9. Shipping / Build 정책

최종 정책:

- 모든 debug overlay 저장/표시 코드는 `#if !UE_BUILD_SHIPPING`로 보호한다.
- Shipping build에서 public API가 필요하면 no-op 또는 false를 반환한다.
- UMG/Slate dependency를 추가하지 않는다.
- `Engine` 의존성 내 Canvas Draw만 사용한다.
- CSV counter를 추가하지 않는다.
- overlay 표시가 성능 계측 결과처럼 보이지 않도록 FPS/성능 성공 수치를 P0에 넣지 않는다.

P0 overlay는 개발 전용 evidence renderer다. 제품 HUD처럼 보이거나 Shipping HUD처럼 노출되면 안 된다.

## 10. 구현 순서 최종 결정

1. `FDebugOverlaySnapshotTypes.h` 작성
2. `FDebugOverlaySnapshotStore.h/.cpp` 작성
3. 기존 debug hook에서 store 기록 연결
4. `CDebugOverlayHUD.h/.cpp` Canvas Draw 구현
5. 테스트/촬영 맵 GameMode 또는 World Settings 연결
6. build 검증
7. capture preset 기준 표시 확인

각 단계는 독립적으로 검증 가능한 범위로 나눈다. 구현 중 새 표시 항목을 추가하지 않는다.

## 11. 구현 전 체크리스트

- [ ] 이 문서를 기준으로 P0 구현 범위를 잠근다.
- [ ] 문서 외 항목은 구현 중 추가하지 않는다.
- [ ] 추가 항목이 필요하면 P1 후보로 문서화한다.
- [ ] 전역 `GlobalDefaultGameMode`를 변경하지 않는다.
- [ ] UMG/Slate dependency를 추가하지 않는다.
- [ ] actor raw pointer를 store에 장기 보관하지 않는다.
- [ ] 현재값과 최근 event값 label을 분리한다.
- [ ] Shipping 경로가 no-op 또는 false인지 확인한다.
- [ ] capture map에서만 overlay 연결을 확인한다.
- [ ] build 검증 후 capture preset으로 화면 노출을 확인한다.

## 12. 최종 결론

P0 debug overlay는 `Core/Debug` 기반의 개발 전용 snapshot store와 Canvas Draw HUD로 구현한다. 전역 GameMode는 변경하지 않고, 테스트/촬영 맵 한정 연결로 evidence capture 범위를 제한한다.

P0의 핵심은 구현 범위를 작게 유지하면서도 Action/Reaction, Guard, HitWindow, DefenseOutcome, Damage, RuntimeLODTier, EventLog evidence를 한 화면에서 확인 가능하게 만드는 것이다. 구현 중 이 문서의 범위를 벗어나는 항목은 P1로 넘긴다.
