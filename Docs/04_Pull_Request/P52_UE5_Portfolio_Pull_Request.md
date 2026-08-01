# UE5 Portfolio Pull Request

## 제목

**P52: Debug Overlay Evidence Tooling**

## 날짜

**2026.07.30 ~ 2026.08.01**

## 상태

- [x] P0 SnapshotTypes / SnapshotStore 구현
- [x] 기존 debug hook에서 SnapshotStore 기록 연결
- [x] Canvas 기반 `ACDebugOverlayHUD` 구현
- [x] TestRoom 수동 연결용 `ACDebugOverlayGameMode` 추가
- [x] P0.5 Player / Enemy panel 분리
- [x] Movement / HP / Stagger Count 표시
- [x] P1 TargetComponent 기반 명시 Enemy target 표시
- [x] `DebugOverlaySelectNearestTarget` / `DebugOverlayClearTarget` 운용 경로 정리
- [x] EventLog category filter 구현
- [x] EventLog noise / collision window display filter 구현
- [x] EventLog separate panel 구현
- [x] Interaction separate panel 구현
- [x] Player / Enemy Recent Execution 분리
- [x] Recent Combat damage breakdown 표시
- [x] Recent Combat collision lifecycle overwrite 방지
- [x] Enemy Current AI / Recent AI Event 분리
- [x] W05 Code Quality Review 작성
- [x] W05 cleanup 후보 구조 리뷰 및 후속 브랜치 이관
- [x] P1 통합 PIE 결과 문서화
- [x] P1 Closure Review 작성
- [x] P1 FinalCandidate evidence package 작성
- [x] P52 evidence claim 연결
- [ ] Runtime LOD actual 표시 보강
- [ ] 포트폴리오 본문 연결

## 브랜치

- `feature/debug-overlay-evidence-plan`

## 요약

이번 PR 후보는 resume / 기술문서 / 제출 영상에서 runtime evidence를 설명하기 위한 개발 전용 Debug Overlay를 추가한다.

완성형 gameplay HUD가 아니라, Action / Reaction, CombatSignal / Damage, Enemy AI, Runtime LOD 관련 현재값과 최근 event를 TestRoom PIE에서 확인하기 위한 evidence tooling이다. Overlay는 shipping product UI처럼 보이지 않게 유지하며, 실제 코드에서 읽을 수 없는 값은 `N/A`, `NotCaptured`, `Pending`으로 표시한다.

P1 마감 기준에서는 `Runtime LOD actual` 표시만 의도적으로 보류하고, Target / EventLog / Interaction / Recent summary / AI evidence 표시를 대부분 구현 완료 상태로 둔다. FinalCandidate evidence package는 별도 폴더와 문서에 정리했으며, 포트폴리오 본문 연결은 후속 단계로 분리한다.

```text
P0
-> SnapshotStore / debug hook / Canvas HUD

P0.5
-> Player/Enemy panel, Movement, HP, Stagger Count, compact 표시

P1
-> TargetComponent.Nearest
-> 3-panel layout
-> EventLog filter / separate panel
-> Interaction recent summary
-> Player/Enemy Recent Execution
-> Enemy Current AI / Recent AI Event
-> Recent Combat damage breakdown
```

## 변경 배경

기술문서와 포트폴리오 문서에서 gameplay 구조를 설명하려면, 코드 경로만이 아니라 실제 runtime에서 어떤 판단과 결과가 발생했는지 보여주는 evidence가 필요하다.

기존 audit log는 Output Log 기반이라 제출 영상이나 스크린샷에 바로 쓰기 어렵고, 기존 gameplay HUD로 만들면 shipping HUD처럼 오해될 수 있다. 따라서 debug-only CVar gate 뒤에서만 동작하는 overlay를 별도 evidence tooling으로 구성했다.

또한 이 브랜치에서 추가된 debug overlay 코드는 P42~P51의 W05 코드 클린 기준과 충돌하지 않아야 한다. 특히 debug log policy, CVar ownership, include order, naming, section, tuning constant, shipping guard 경계를 함께 검토했다.

## 주요 변경

### 1. SnapshotTypes / SnapshotStore

대상:

- `FDebugOverlaySnapshotTypes.h`
- `FDebugOverlaySnapshotStore.h/.cpp`

역할:

- World별 debug overlay snapshot cache를 제공한다.
- 현재 gameplay state의 owner가 아니라 evidence 표시를 위한 최근값 저장소다.
- event log는 고정 크기 ring buffer로 유지한다.
- snapshot 조회는 copy 반환 방식으로 처리한다.

주요 정책:

```text
World별 store
-> TObjectKey<UWorld> 기반 분리

Actor 장기 보관
-> raw pointer 금지
-> weak pair / actor name / frame / time 정보만 저장

Shipping
-> CVar 미선언 참조 없음
-> Record API no-op
-> Query API false 또는 empty
```

### 2. 기존 debug hook 연결

대상:

- `FExecutionOrchestratorDebug.cpp`
- `FCombatSignalDebug.cpp`
- `FCombatResultDebug.cpp`
- `FAICombatBTDebug.cpp`

역할:

- 기존 audit log 출력 흐름을 바꾸지 않고 SnapshotStore에 최근 event를 기록한다.
- 기존 `ShouldAudit...()` CVar와 overlay collect CVar를 결합하지 않는다.

연결 항목:

```text
Execution
-> Action / Reaction decision

CombatSignal
-> HitWindow
-> Target accepted / rejected
-> CombatResult dispatch

CombatResult
-> CombatResult received

AI
-> Combat task success / reject
```

### 3. Canvas 기반 Debug Overlay HUD

대상:

- `ACDebugOverlayHUD`
- `ACDebugOverlayGameMode`

역할:

- UMG / Slate 없이 `AHUD::DrawHUD()`와 Canvas Draw로 개발 전용 overlay를 표시한다.
- 전역 `GlobalDefaultGameMode`를 변경하지 않고 TestRoom 수동 연결을 기준으로 둔다.

P1 현재 표시 구조:

```text
[Debug Overlay Pannel_01]
[Player]
Current State
[Recent Execution]

[Enemy]
Target Source / Current State
[Recent Execution]
[Current AI]
[Recent AI Event]

[Debug Overlay Pannel_02]
[Event Log: All|Execution|Combat|AI]

[Debug Overlay Pannel_03]
[Interaction]
[Recent Execution]
[Recent Combat]
```

### 4. P0.5 표시 보강

대상:

- Player / Enemy panel 분리
- blue / red tab
- 배경 박스와 글자 크기 조정
- Movement, HP, Stagger Count 추가
- enum prefix 제거
- multi-field line `|` 구분
- Guard action 의미 중심 표시

결과:

- Player와 Enemy의 현재 상태를 같은 순서로 비교할 수 있다.
- Stagger Count는 현재 parry stack으로만 표시하며 누적 통계처럼 주장하지 않는다.
- P0.5 Round1 캡처는 임시 검증 evidence로 유지하고 FinalCandidate로 승격하지 않는다.

### 5. P1 TargetComponent 명시 target 표시

대상:

- `UCDebugOverlayTargetComponent`
- `ACPlayerController` debug Exec command
- HUD enemy source 표시

현재 정책:

```text
DebugOverlaySelectNearestTarget
-> TargetComponent.Nearest

DebugOverlayClearTarget
-> EnemySource: None

target 없음
-> EnemySource: None
```

`RecentCombatTarget` / `WorldScanFallback`은 기본 Enemy panel source로 자동 사용하지 않는다. P1 evidence에서는 명시 target이 있을 때만 Enemy panel 값을 성공 evidence로 주장한다.

Nearest radius는 `3000` 기준으로 운용한다.

### 6. EventLog filter / separate panel

대상:

- `FDebugOverlaySnapshotStore`
- `ACDebugOverlayHUD`

구현 상태:

- `Portfolio.DebugOverlay.EventLogFilter`
  - `All`
  - `Execution`
  - `Combat`
  - `AI`
- `Portfolio.DebugOverlay.EventLogLimit`
  - `0~32`
- Reject / Ignore noise display filter
- Collision window display filter
- EventLog separate panel `Pannel_02`

주의:

- display filter는 화면 표시 제어다.
- filter로 숨겨진 event를 “발생하지 않았다”고 주장하지 않는다.
- collision lifecycle event는 EventLog diagnostic으로 유지될 수 있다.

### 7. Interaction / Recent summary

Interaction panel은 world-level recent summary를 담당한다.

표시 항목:

- `[Recent Execution]`
- `[Recent Combat]`

Player / Enemy panel에는 actor-local Recent Execution을 별도 표시한다.

Recent Combat은 최근 combat evidence summary이며, 마지막 combat log가 아니다. `CollisionEnabled`, `CollisionDisabled`, `CollisionDisabledIgnored` 같은 collision lifecycle event는 Recent Combat 대표값을 덮어쓰지 않는다.

Recent Combat damage breakdown:

```text
Request
Mitigated
Final
Commit
```

`Request`는 요청 damage, `Mitigated`는 방어/감산 이후 값, `Final`은 최종 판정 damage, `Commit`은 실제 commit 값이다. `Raw`라는 표현은 사용하지 않는다.

### 8. Enemy Current AI / Recent AI Event

Enemy AI evidence는 두 영역으로 분리한다.

```text
[Current AI]
Controller
Pawn
Target
IntentState
ReturnHome
UsePatrol
HasLOS
DistanceToTarget
IsCombatAction

[Recent AI Event]
Task
Result
Age 또는 Stale Time
Last Pawn
RejectReason
Note
```

정책:

- Current AI는 selected Enemy의 현재 Blackboard / Controller / Pawn 상태다.
- Recent AI Event는 최근 AI task event다.
- Recent AI Event는 current AI evidence가 아니다.
- Behavior Tree active node 전체 추적은 P1 보류다.

### 9. BT_Default tuning

대상:

- `Content/02_Controller/02_Enemy/AI/BehaviorTree/BT_Default.uasset`

변경 목적:

- Patrol 이동 가능 거리 조정
- P1 PIE 검증에서 Enemy patrol / engage 흐름을 안정적으로 확인하기 위한 tuning

주의:

- debug overlay UI 기능 변경이 아니다.
- HUD evidence claim에 포함하지 않는다.
- 별도 커밋으로 분리했다.

## W05 Code Quality 반영

| W05 기준 | 반영 내용 |
| --- | --- |
| P42 Debug Log Policy | audit log와 overlay collect를 분리하고, gameplay hook은 record 호출만 수행한다. |
| P43 CVar Ownership | `Portfolio.DebugOverlay.*` CVar를 기존 debug audit / profiling CVar와 분리했다. |
| P44 Comment / Section Cleanup | HUD / Store helper section을 책임 단위로 정리했다. |
| P45 Naming / API Cleanup | `Try...`, `Append...`, `TryGetSnapshotCopy` naming을 적용했다. |
| P46 Type Header Organization | debug overlay snapshot type을 `Core/Debug` 소유 type으로 분리했다. |
| P48 Include Order Cleanup | HUD / Store / PlayerController `.cpp` include group을 W05 기준으로 정리했다. |
| P49 API Const Consistency | getter/query const 여부와 DecisionNeeded 항목을 문서화했다. |
| P50 Section Comment Consistency | Store API section과 HUD helper section을 파일군 책임 기준으로 정리했다. |
| P51 Tuning Constants Cleanup | nearest radius, stale timeout, event limit을 internal policy constant 또는 CVar contract로 분류했다. |

## 변경 파일 범위

```text
Docs/07_Portfolio_Documents/Debug_Overlay/*
Docs/04_Pull_Request/P52_UE5_Portfolio_Pull_Request.md

Source/Portfolio/Core/Debug/*
Source/Portfolio/Controller/CPlayerController.h/.cpp
Source/Portfolio/Character/Player/CPlayer.h
Source/Portfolio/Character/Enemy/CEnemy.h

Content/02_Controller/02_Enemy/AI/BehaviorTree/BT_Default.uasset
```

`BT_Default.uasset`은 PIE 검증 보조 tuning이며, overlay HUD 기능 구현 범위와 분리해서 설명한다.

## 검증

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Static check

```text
git diff --check
Result: Pass
```

### PIE

```text
Map:
/Game/00_UnitTest/TestRoom

Result:
P1 3-panel overlay 표시 확인
TargetComponent.Nearest 표시 확인
Player / Enemy Recent Execution 분리 확인
EventLog separate panel 확인
Interaction panel 확인
Recent Combat Request / Mitigated / Final / Commit 확인
Enemy Current AI / Recent AI Event 분리 확인
Collision lifecycle event가 Recent Combat을 덮지 않는 것 확인
```

주의:

- PIE 검증 캡처와 P0.5 Round1 캡처는 FinalCandidate로 승격하지 않는다.
- FinalCandidate evidence는 `Docs/98_Evidence/01_Screenshot/DebugOverlay/FinalCandidate`에 별도 패키징한다.
- `Runtime LOD: N/A`는 성공 evidence로 주장하지 않는다.

## FinalCandidate Evidence 연결

FinalCandidate evidence package:

- `Docs/07_Portfolio_Documents/Debug_Overlay/06_Evidence_Package/Debug_Overlay_P1_Final_Candidate_Evidence_Package_KR.md`
- `Docs/98_Evidence/01_Screenshot/DebugOverlay/FinalCandidate`

P52 PR 설명에서 사용할 evidence claim은 아래 파일 기준으로 제한한다. 한 캡처가 여러 claim을 동시에 보여주더라도, claim은 실제 화면에서 읽히는 visible evidence 범위로만 사용한다.

| claim | evidence file | 주의 |
| --- | --- | --- |
| 3-panel layout | `debug_overlay_p1_final_idle.png`, `debug_overlay_p1_final_eventlog_all.png` | runtime 표기 `Pannel_01/02/03` 유지 |
| TargetComponent.Nearest | `debug_overlay_p1_final_target_nearest.png` | generic target system / lock-on claim 금지 |
| EnemySource None | `debug_overlay_p1_final_guard_out.png`, `debug_overlay_p1_final_parry.png` | target 없음 또는 clear 후 상태 claim |
| Player movement Run | `debug_overlay_p1_final_move_run.png` | movement current state 표시 claim |
| Player Recent Execution | `debug_overlay_p1_final_guard_out.png`, `debug_overlay_p1_final_parry.png` | actor-specific recent execution claim |
| Enemy Recent Execution | `debug_overlay_p1_final_enemy_recent_execution.png` | Interaction recent와 역할 분리 |
| Interaction Recent Combat / damage breakdown | `debug_overlay_p1_final_interaction_combat_damage.png`, `debug_overlay_p1_final_parry.png` | `Request / Mitigated / Final / Commit` 기준 |
| EventLog separate panel / All filter | `debug_overlay_p1_final_eventlog_all.png` | line wrapping / compact claim 금지 |
| EventLog Execution filter | `debug_overlay_p1_final_guard_out.png`, `debug_overlay_p1_final_parry.png` | Execution category 표시 claim |
| Guard / BlockHit / Parry | `debug_overlay_p1_final_guard_in.png`, `debug_overlay_p1_final_block_hit.png`, `debug_overlay_p1_final_parry.png` | visible reaction/outcome/damage line 기준 |
| Player Hit / Enemy Hit | `debug_overlay_p1_final_player_hit.png`, `debug_overlay_p1_final_enemy_hit.png` | HP/Reaction/EventLog visible line 기준 |
| Stagger Count | `debug_overlay_p1_final_stagger_stack_2.png`, `debug_overlay_p1_final_stagger_stack_3.png` | 현재 stack count claim, 누적 통계 claim 금지 |
| Enemy Current AI | `debug_overlay_p1_final_enemy_current_ai.png` | Blackboard / Controller / Pawn current snapshot claim |
| Enemy Recent AI Event | `debug_overlay_p1_final_enemy_recent_ai_event.png` | current AI evidence가 아니라 recent task event |

NotPackaged / 제한 claim:

- `EventLog`의 `Combat` / `AI` 전용 filter 성공 캡처는 현재 패키지에 포함하지 않는다.
- `NoEvents(Filter: Combat)` 또는 `NoEvents(Filter: AI)` 장면은 empty-state evidence로만 별도 보강 가능하다.
- `Runtime LOD actual`, Behavior Tree active node tracking, Shipping HUD, generic target system, combat action target flow는 P52 성공 claim으로 사용하지 않는다.
- display filter로 숨겨진 event를 “발생하지 않았다”고 주장하지 않는다.
- collision lifecycle event는 Recent Combat을 덮지 않지만, EventLog diagnostic으로 남을 수 있다.

## 비범위 / 후속 작업

P1에서 의도적으로 보류한 항목:

```text
Runtime LOD actual 표시
Behavior Tree active node 전체 추적
EventLog line wrapping / compact 재작업
CollisionDisabledIgnored event 자체 발생 원인 제거
필요 시 FinalCandidate 보강 캡처
포트폴리오 본문 연결
```

명시적 비목표:

```text
Shipping HUD화
UMG / Slate 전환
범용 lock-on system
combat action target 강제
AI target selection 변경
기존 audit log format 변경
성능 성공 주장
```

## 관련 문서

- `Docs/07_Portfolio_Documents/Debug_Overlay/README.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Work_Order_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Scope_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/01_Planning/Debug_Overlay_P1_Closure_Criteria_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Integrated_PIE_Result_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Overlay_Layout_PIE_Result_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Closure_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Quality_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_W05_PR_Style_Gap_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_P1_Code_Clean_Structure_Review_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/06_Evidence_Package/Debug_Overlay_P1_Final_Candidate_Capture_Checklist_KR.md`
- `Docs/07_Portfolio_Documents/Debug_Overlay/06_Evidence_Package/Debug_Overlay_P1_Final_Candidate_Evidence_Package_KR.md`
- `Docs/04_Pull_Request/P42_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P43_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P44_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P45_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P46_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P48_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P49_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P50_UE5_Portfolio_Pull_Request.md`
- `Docs/04_Pull_Request/P51_UE5_Portfolio_Pull_Request.md`

## 정리

이번 PR 후보는 debug overlay를 제출용 evidence tooling으로 구성하고, P0/P0.5/P1로 단계화된 구현과 W05 code quality 기준의 구조 리뷰를 하나의 설명 가능한 흐름으로 묶는다. 실제 code cleanup 구현은 후속 브랜치로 분리한다.

P1 기준으로 Target / EventLog / Interaction / Recent summary / Enemy AI evidence는 마감 가능한 수준까지 구현되었다. FinalCandidate evidence package까지 정리했으며, Runtime LOD actual 표시와 포트폴리오 본문 연결은 후속 단계로 분리한다.
