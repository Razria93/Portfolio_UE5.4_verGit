# Debug Overlay P1 FinalCandidate Evidence Package

## 1. 목적

이 문서는 P1 debug overlay FinalCandidate 캡처 후보 중 현재 패키징한 파일과 각 파일에 매핑할 수 있는 evidence claim을 정리한다.

P1 Closure Review 이후 촬영한 clean viewport 캡처만 `FinalCandidate` 폴더에 복사한다. PIE 검증 캡처, Round1 캡처, editor console/window/taskbar/tooltip이 노출된 캡처는 FinalCandidate로 승격하지 않는다.

## 2. 패키지 전제

- 브랜치: `feature/debug-overlay-evidence-plan`
- 저장 위치:
  - `Docs/98_Evidence/01_Screenshot/DebugOverlay/FinalCandidate`
- 기준 문서:
  - `05_Verification/Debug_Overlay_P1_Closure_Review_KR.md`
  - `06_Evidence_Package/Debug_Overlay_P1_Final_Candidate_Capture_Checklist_KR.md`
- runtime panel 표시:
  - `[Debug Overlay Pannel_01]`: Player / Enemy actor state
  - `[Debug Overlay Pannel_02]`: EventLog
  - `[Debug Overlay Pannel_03]`: Interaction
- `Pannel` 표기는 현재 runtime style-lock 값이므로 임의로 `Panel`로 교정하지 않는다.
- `Runtime LOD: N/A`는 Runtime LOD actual 구현 성공 evidence로 사용하지 않는다.
- `BT_Default.uasset` patrol tuning은 overlay 기능 claim으로 사용하지 않는다.

## 3. 채택 파일

| 패키지 파일 | 원본 파일 | 대상 장면 | visible evidence | claim |
| --- | --- | --- | --- | --- |
| `debug_overlay_p1_final_idle.png` | `bandicam 2026-08-01 00-11-22-419.jpg` | Idle baseline | `State: Idle`, `Action: None`, `Reaction: None`, 3-panel layout | Player/Enemy current state baseline과 3-panel layout 확인 |
| `debug_overlay_p1_final_target_nearest.png` | `bandicam 2026-08-01 00-11-22-419.jpg` | TargetComponent.Nearest | `EnemySource: TargetComponent.Nearest`, `EnemyTarget: Selected=BP_CEnemy_C_1`, `Radius: 3000` | 명시 target 기반 Enemy panel 표시 |
| `debug_overlay_p1_final_move_run.png` | `bandicam 2026-08-01 05-44-17-600.jpg` | Run movement | `Movement: Gait: Run`, `Speed: 600.0`, `CanMove: true`, `Falling: false` | Player movement state가 current state panel에 표시됨 |
| `debug_overlay_p1_final_guard_in.png` | `bandicam 2026-08-01 00-11-48-970.jpg` | Guard / BlockHit | `Guard: Wants: true`, `Pose: true`, `CanGuard: true`, `Reaction: BlockHit` | Guard 상태와 BlockHit 반응 표시 |
| `debug_overlay_p1_final_guard_out.png` | `bandicam 2026-08-01 05-30-45-910.jpg` | Guard Out | Player `Action: Guard Out`, Player `[Recent Execution] Subject: Guard Out`, EventLog `Subject: Guard Out` | Player Action과 Player Recent Execution 분리 표시 |
| `debug_overlay_p1_final_block_hit.png` | `bandicam 2026-08-01 00-11-48-970.jpg` | Block Hit | `Reaction: BlockHit`, `Outcome: Guard`, `Commit: 2.500` | 방어 판정과 damage commit 감소 확인 |
| `debug_overlay_p1_final_parry.png` | `bandicam 2026-08-01 05-40-49-138.jpg` | Parry | Player `Reaction: Parry`, Interaction `Outcome: Parry`, `Final: 0.000`, `Commit: 0.000` | Parry 판정과 최종 damage commit 0 확인 |
| `debug_overlay_p1_final_player_hit.png` | `bandicam 2026-08-01 00-12-10-592.jpg` | Player Hit | `Reaction: Hit`, Player HP 감소, `Commit: 10.000` | Player 피격 current state와 combat result 표시 |
| `debug_overlay_p1_final_enemy_hit.png` | `bandicam 2026-08-01 00-50-10-176.jpg` | Enemy damage evidence | Enemy HP 감소, EventLog의 Player -> Enemy combat result line | Enemy 측 damage evidence 후보. claim은 화면 visible line 기준으로 제한 |
| `debug_overlay_p1_final_enemy_recent_execution.png` | `bandicam 2026-08-01 00-50-10-176.jpg` | Enemy Recent Execution | `[Recent Execution]`, `Owner: BP_CEnemy_C_1`, `Subject: ComboAttack[2]` | Player/Enemy Recent Execution 분리 |
| `debug_overlay_p1_final_interaction_combat_damage.png` | `bandicam 2026-08-01 05-40-52-777.jpg` | Interaction Recent Combat damage | `Request: 15.000`, `Mitigated: 15.000`, `Final: 0.000`, `Commit: 0.000`, `Accepted: true` | Recent Combat damage breakdown 표시 |
| `debug_overlay_p1_final_eventlog_all.png` | `bandicam 2026-08-01 00-50-10-176.jpg` | EventLog All | `[Event Log: All]`, execution/combat/AI lines | EventLog separate panel과 `All` filter 표시 |
| `debug_overlay_p1_final_enemy_current_ai.png` | `bandicam 2026-08-01 05-40-50-842.jpg` | Enemy Current AI | `[Current AI]`, `Controller`, `Pawn`, `Target`, `IntentState: Engage`, `HasLOS`, `DistanceToTarget`, `IsCombatAction` | Enemy Current AI snapshot 표시 |
| `debug_overlay_p1_final_enemy_recent_ai_event.png` | `bandicam 2026-08-01 05-40-50-842.jpg` | Enemy Recent AI Event | `[Recent AI Event]`, `Task: ComboAttack`, `Result: Started`, `Age`, `RejectReason` | Current AI와 Recent AI Event 분리 표시 |
| `debug_overlay_p1_final_stagger_stack_2.png` | `bandicam 2026-08-01 05-40-50-842.jpg` | Stagger stack 2 | Enemy `Stagger: 2/3`, combat action context | Stagger stack count 2/3 표시 |
| `debug_overlay_p1_final_stagger_stack_3.png` | `bandicam 2026-08-01 05-40-52-777.jpg` | Enemy Stagger / stack 3 | Enemy `Reaction: Stagger`, `Stagger: 3/3`, EventLog `Subject: Stagger` | Enemy stagger reaction과 stack count 3/3 표시 |

동일 원본 파일이 여러 패키지 파일로 사용된 경우가 있다. 이는 한 캡처가 여러 claim을 동시에 보여주기 때문이며, 각 패키지 파일의 claim은 표에 기록된 visible evidence 범위로만 제한한다.

## 4. 보류 / 제외 파일

| 원본 파일 | 상태 | 이유 |
| --- | --- | --- |
| `bandicam 2026-08-01 00-49-57-941.jpg` | Excluded | taskbar와 대화/창 테두리가 노출되어 FinalCandidate 제외 기준에 해당 |
| `bandicam 2026-08-01 00-49-57-893.jpg` | Excluded | `00-49-57-941`과 동일 후보이며 taskbar와 대화/창 테두리 노출 |
| `bandicam 2026-08-01 05-39-21-877.jpg` | Excluded | mouse capture tooltip 노출 |
| `bandicam 2026-08-01 05-23-38-428.jpg` | Excluded | `Subject: Stagger`는 보이나 Stagger Count evidence가 약해 후속 후보로 보류 |
| 2026-07-31 이전 PIE 검증 캡처 | Excluded | P1 최종 layout 이전 검증 자료이므로 FinalCandidate로 승격하지 않음 |
| Round1 / Round1_StaggerCount | Excluded | P0.5 임시 검증 evidence로 유지하며 FinalCandidate와 분리 |

## 5. Claim Matrix

| claim | 상태 | 근거 파일 | 주의 |
| --- | --- | --- | --- |
| 3-panel layout | Packaged | `debug_overlay_p1_final_idle.png`, `debug_overlay_p1_final_eventlog_all.png` | `Pannel_01/02/03` 표시 유지 |
| TargetComponent.Nearest | Packaged | `debug_overlay_p1_final_target_nearest.png` | generic target system / lock-on claim 금지 |
| EnemySource None | Packaged | `debug_overlay_p1_final_guard_out.png`, `debug_overlay_p1_final_parry.png` | clear/target 없음 상태 claim에 사용 가능 |
| Player movement Run | Packaged | `debug_overlay_p1_final_move_run.png` | movement state 표시 claim으로만 제한 |
| Player Recent Execution | Packaged | `debug_overlay_p1_final_guard_out.png`, `debug_overlay_p1_final_parry.png` | actor-specific recent execution으로만 주장 |
| Enemy Recent Execution | Packaged | `debug_overlay_p1_final_enemy_recent_execution.png` | Interaction recent와 역할 분리 |
| Interaction Recent Combat | Packaged | `debug_overlay_p1_final_interaction_combat_damage.png`, `debug_overlay_p1_final_parry.png` | collision lifecycle이 Recent Combat을 덮지 않는 claim은 별도 문서 기준과 함께 사용 |
| EventLog separate panel | Packaged | `debug_overlay_p1_final_eventlog_all.png`, `debug_overlay_p1_final_parry.png` | line wrapping / compact claim 금지 |
| EventLog `All` filter | Packaged | `debug_overlay_p1_final_eventlog_all.png` | filter로 숨겨진 event를 미발생으로 주장 금지 |
| EventLog `Execution` filter | Packaged | `debug_overlay_p1_final_guard_out.png`, `debug_overlay_p1_final_parry.png` | Execution category 표시 claim |
| EventLog `Combat` filter | NotPackaged | N/A | `NoEvents(Filter: Combat)` 캡처는 empty-state 증거로만 별도 패키징 가능 |
| EventLog `AI` filter | NotPackaged | N/A | `NoEvents(Filter: AI)` 캡처는 empty-state 증거로만 별도 패키징 가능 |
| Guard / BlockHit | Packaged | `debug_overlay_p1_final_guard_in.png`, `debug_overlay_p1_final_block_hit.png` | Guard outcome과 BlockHit visible line 기준 |
| Guard Out | Packaged | `debug_overlay_p1_final_guard_out.png` | action/recent execution claim 기준 |
| Parry | Packaged | `debug_overlay_p1_final_parry.png` | `Final: 0.000`, `Commit: 0.000` 기준 |
| Player Hit | Packaged | `debug_overlay_p1_final_player_hit.png` | HP/Reaction visible line 기준 |
| Enemy Hit | Packaged | `debug_overlay_p1_final_enemy_hit.png` | HP/EventLog visible line 기준, 영상 흐름 claim 금지 |
| Stagger stack 2 | Packaged | `debug_overlay_p1_final_stagger_stack_2.png` | count 2/3 claim |
| Stagger stack 3 / Enemy Stagger | Packaged | `debug_overlay_p1_final_stagger_stack_3.png` | `Reaction: Stagger`, count 3/3 claim |
| Enemy Current AI | Packaged | `debug_overlay_p1_final_enemy_current_ai.png` | BT active node tracking claim 금지 |
| Enemy Recent AI Event | Packaged | `debug_overlay_p1_final_enemy_recent_ai_event.png` | Recent event이며 Current AI evidence와 구분 |
| Runtime LOD actual | NotClaimed | N/A | P1 보류 항목 |

## 6. 필수 파일명 기준 상태

| 파일명 | 상태 |
| --- | --- |
| `debug_overlay_p1_final_idle.png` | Packaged |
| `debug_overlay_p1_final_move_walk.png` | NotPackaged |
| `debug_overlay_p1_final_move_run.png` | Packaged |
| `debug_overlay_p1_final_guard_in.png` | Packaged |
| `debug_overlay_p1_final_guard_out.png` | Packaged |
| `debug_overlay_p1_final_player_hit.png` | Packaged |
| `debug_overlay_p1_final_enemy_hit.png` | Packaged |
| `debug_overlay_p1_final_block_hit.png` | Packaged |
| `debug_overlay_p1_final_parry.png` | Packaged |
| `debug_overlay_p1_final_enemy_stagger.png` | Covered by `debug_overlay_p1_final_stagger_stack_3.png` |
| `debug_overlay_p1_final_stagger_stack_1.png` | NotPackaged |
| `debug_overlay_p1_final_stagger_stack_2.png` | Packaged |
| `debug_overlay_p1_final_stagger_stack_3.png` | Packaged |
| `debug_overlay_p1_final_stagger_reset.png` | NotPackaged |
| `debug_overlay_p1_final_target_nearest.png` | Packaged |
| `debug_overlay_p1_final_target_none.png` | Covered by several `EnemySource: None` captures |
| `debug_overlay_p1_final_enemy_recent_execution.png` | Packaged |
| `debug_overlay_p1_final_interaction_combat_damage.png` | Packaged |
| `debug_overlay_p1_final_eventlog_all.png` | Packaged |
| `debug_overlay_p1_final_eventlog_execution.png` | Covered by Execution-filter captures, dedicated file optional |
| `debug_overlay_p1_final_eventlog_combat.png` | NotPackaged |
| `debug_overlay_p1_final_eventlog_ai.png` | NotPackaged |
| `debug_overlay_p1_final_enemy_current_ai.png` | Packaged |
| `debug_overlay_p1_final_enemy_recent_ai_event.png` | Packaged |

## 7. 금지 Claim

다음 항목은 현재 패키지에서 성공 evidence로 사용하지 않는다.

- `Runtime LOD: N/A`를 Runtime LOD actual 구현 성공으로 주장
- Behavior Tree active node tracking claim
- Shipping HUD / gameplay HUD claim
- generic target system, lock-on, combat action target flow claim
- EventLog filter로 숨겨진 event를 미발생으로 주장
- PIE 검증 캡처, Round1 캡처를 FinalCandidate로 승격
- `BT_Default.uasset` patrol tuning을 overlay 기능 claim으로 사용

## 8. 완료 기준

이번 패키징 단계의 완료 기준은 다음과 같다.

- FinalCandidate 폴더에 추가 캡처 PNG 파일 정리
- 각 파일의 source, visible evidence, claim 범위 기록
- 제외/보류 파일과 이유 기록
- 아직 미확보된 장면을 `NotPackaged` 또는 `Covered by ...`로 분리
- Runtime LOD actual, BT active node tracking, Shipping HUD, generic target system claim 금지 유지

## 9. 다음 작업

1. P52 PR 최종 점검 후 브랜치 마감 가능 여부를 판단한다.
2. 포트폴리오 본문 evidence claim에 연결할 스크린샷과 claim 문장을 선별한다.
3. 필요하면 `NotPackaged`로 남은 Combat/AI empty filter 전용 캡처만 별도 보강한다.
4. 후속 브랜치에서 debug overlay code cleanup과 Runtime LOD actual 표시를 검토한다.
