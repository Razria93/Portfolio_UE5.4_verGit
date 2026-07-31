# Debug Overlay P1 FinalCandidate Capture Checklist

## 1. 목적

이 문서는 P1 debug overlay의 FinalCandidate 촬영 전에 확인해야 할 장면, claim 범위, 제외 기준, CVar / command 준비사항을 고정한다.

이번 문서는 촬영 실행이나 패키징 결과가 아니라 촬영 전 준비 기준이다.

P1 Closure Review는 완료된 상태로 본다. FinalCandidate 촬영은 P1 기능 closure 이후 새로 촬영한 파일만 대상으로 한다.

Round1, Round1_StaggerCount, PIE 검증 캡처는 FinalCandidate로 승격하지 않는다. 최종 후보는 P1 마감 기준을 통과한 뒤 새로 촬영한 파일만 사용한다.

## 2. 현재 P1 HUD 기준

P1 최종 촬영은 다음 3-panel layout을 기준으로 한다.

`Pannel` 표기는 현재 runtime style-lock 표시값이므로 FinalCandidate 문서와 파일 설명에서 임의로 `Panel`로 정정하지 않는다.

### 2.1 `Pannel_01`

역할:

- Player current state
- Player Recent Execution
- Enemy current state
- Enemy Recent Execution
- Enemy Current AI
- Enemy Recent AI Event

필수 확인:

- `[Debug Overlay Pannel_01]`
- `[Player]`
- `[Enemy]`
- Player / Enemy의 `State`, `Action`, `Reaction`, `HP`, `Stagger`, `Guard`, `Movement`
- Enemy target source와 selected target
- Enemy `[Current AI]`, `[Recent AI Event]`

주의:

- `Runtime LOD: N/A`는 표시될 수 있으나 성공 evidence로 사용하지 않는다.
- `Recent AI Event`는 current AI state가 아니라 최근 AI task event다.

### 2.2 `Pannel_02`

역할:

- EventLog separate panel
- `All / Execution / Combat / AI` filter 결과 표시
- `EventLogLimit 0~32`
- noise / collision display filter 결과 표시

필수 확인:

- `[Debug Overlay Pannel_02]`
- `[Event Log: All]`
- `[Event Log: Execution]`
- `[Event Log: Combat]`
- `[Event Log: AI]`

주의:

- EventLog display filter는 화면 표시 제어다.
- filter로 숨겨진 event를 “발생하지 않음”으로 주장하지 않는다.

### 2.3 `Pannel_03`

역할:

- Interaction panel
- world-level Recent Execution
- world-level Recent Combat

필수 확인:

- `[Debug Overlay Pannel_03]`
- `[Interaction]`
- `[Recent Execution]`
- `[Recent Combat]`
- Recent Combat의 `Request`, `Mitigated`, `Final`, `Commit`

주의:

- `CollisionEnabled`, `CollisionDisabled`, `CollisionDisabledIgnored`는 Recent Combat 대표값이 아니라 EventLog diagnostic이다.

## 3. 촬영 전 기본 CVar / Command

촬영 전 기본 설정은 다음을 권장한다.

```text
Portfolio.DebugOverlay.Enabled 1
Portfolio.DebugOverlay.Collect 1
Portfolio.DebugOverlay.EventLogFilter All
Portfolio.DebugOverlay.EventLogLimit 5
Portfolio.DebugOverlay.HideNoiseEvents 1
Portfolio.DebugOverlay.HideCollisionWindowEvents 1
DebugOverlaySelectNearestTarget
DebugOverlayClearTarget
```

장면에 따라 변경 가능한 값:

| 목적 | 권장 명령 |
| --- | --- |
| EventLog 전체 확인 | `Portfolio.DebugOverlay.EventLogFilter All` |
| Execution log 확인 | `Portfolio.DebugOverlay.EventLogFilter Execution` |
| Combat log 확인 | `Portfolio.DebugOverlay.EventLogFilter Combat` |
| AI log 확인 | `Portfolio.DebugOverlay.EventLogFilter AI` |
| EventLog 줄 수 확대 | `Portfolio.DebugOverlay.EventLogLimit 10` 또는 필요 시 `32` 이하 |
| EventLog 숨김 상태 확인 | `Portfolio.DebugOverlay.EventLogLimit 0` |
| collision diagnostic 표시 확인 | `Portfolio.DebugOverlay.HideCollisionWindowEvents 0` |
| collision diagnostic 숨김 | `Portfolio.DebugOverlay.HideCollisionWindowEvents 1` |
| noise diagnostic 표시 확인 | `Portfolio.DebugOverlay.HideNoiseEvents 0` |
| noise diagnostic 숨김 | `Portfolio.DebugOverlay.HideNoiseEvents 1` |
| selected Enemy 지정 | `DebugOverlaySelectNearestTarget` |
| selected Enemy 해제 | `DebugOverlayClearTarget` |
| editor viewport 집중 | `LevelEditor.ToggleImmersive` |

운영 기준:

- Live Coding은 끈 상태를 유지한다.
- hot reload 직후 상태가 의심되면 editor 재시작 후 확인한다.
- console command 입력창 또는 Output Log 창이 최종 캡처 claim을 가리면 final 후보에서 제외한다.

## 4. 필수 촬영 장면

P1 FinalCandidate 필수 장면은 다음을 기준으로 한다.

| 순서 | 장면 | 핵심 확인 | 권장 EventLog filter |
| --- | --- | --- | --- |
| 1 | Idle baseline | 3-panel baseline, Player/Enemy current state | `All` |
| 2 | Walk | Player `Movement: Gait: Walk` | `All` |
| 3 | Run | Player `Movement: Gait: Run` | `All` |
| 4 | Guard In | Player guard start / Recent Execution | `Execution` |
| 5 | Guard Out | Player guard end / Recent Execution | `Execution` |
| 6 | Player Hit | Player `Reaction: Hit`, HP 변화 | `Combat` |
| 7 | Enemy Hit | Enemy `Reaction: Hit`, Enemy HP 변화 | `Combat` |
| 8 | Block Hit | Player `Reaction: BlockHit`, guard outcome | `Combat` |
| 9 | Parry | Player `Reaction: Parry`, commit cancel | `Combat` |
| 10 | Enemy Stagger | Enemy `Reaction: Stagger` | `Combat` |
| 11 | Stagger Count stack 1 | Enemy `Stagger: 1/3` | `Combat` |
| 12 | Stagger Count stack 2 | Enemy `Stagger: 2/3` | `Combat` |
| 13 | Stagger Count reset | Enemy stagger 이후 `0/3` reset | `Combat` |
| 14 | TargetComponent.Nearest selected | `EnemySource: TargetComponent.Nearest` | `All` |
| 15 | TargetComponent clear / no target | `DebugOverlayClearTarget` 후 `EnemySource: None` | `All` |
| 16 | Enemy Recent Execution | Enemy panel의 actor-local `[Recent Execution]` | `Execution` |
| 17 | Interaction Recent Combat damage breakdown | `Request / Mitigated / Final / Commit` | `Combat` |
| 18 | EventLog All | `[Event Log: All]` | `All` |
| 19 | EventLog Execution | `[Event Log: Execution]` | `Execution` |
| 20 | EventLog Combat | `[Event Log: Combat]` | `Combat` |
| 21 | EventLog AI | `[Event Log: AI]` | `AI` |
| 22 | Enemy Current AI visible | `[Current AI]` blackboard 값 | `AI` |
| 23 | Enemy Recent AI Event visible 또는 stale | `[Recent AI Event]` event/stale 상태 | `AI` |

## 5. 장면별 Claim 기준

| 장면 | 주장할 수 있는 것 | 확인할 overlay line | 주장하지 않을 것 |
| --- | --- | --- | --- |
| Idle baseline | P1 3-panel overlay baseline | `Pannel_01`, `Pannel_02`, `Pannel_03` | 최종 gameplay HUD 완성 |
| Walk / Run | movement current state 표시 | `Movement: Gait`, `Speed`, `CanMove` | 물리/animation 완전 검증 |
| Guard In / Out | execution / guard transition 확인 | Player `Recent Execution`, `Guard` | guard balance 또는 frame-perfect 성능 |
| Player Hit | player reaction / HP 변화 | Player `Reaction`, `HP` | damage formula 전체 검증 |
| Enemy Hit | selected Enemy reaction / HP 변화 | Enemy `Reaction`, `HP`, target lines | 자동 fallback enemy evidence |
| Block Hit | guard outcome과 damage 감소 흐름 | `Reaction: BlockHit`, Recent Combat | parry 성공과 혼동 |
| Parry | parry outcome과 commit cancel | `Reaction: Parry`, `Commit: 0.000` | timing 성능 주장 |
| Enemy Stagger | parry stack threshold 후 enemy reaction | Enemy `Reaction: Stagger` | 장기 stagger 통계 |
| Stagger stack 1 / 2 | 현재 parry stagger stack | Enemy `Stagger: 1/3`, `2/3` | 누적 총량 |
| Stagger reset | stagger 이후 stack reset 상태 | Enemy `Stagger: 0/3` | reset 원인 전체 검증 |
| Target selected | 명시 target source 기반 enemy evidence | `EnemySource: TargetComponent.Nearest`, `EnemySelect` | 범용 target system / lock-on / combat target flow 변경 |
| Target clear | 명시 target 해제 후 no target 상태 | `EnemySource: None` | fallback enemy 자동 선택 성공 |
| Enemy Recent Execution | selected Enemy 기준 최근 execution 표시 | Enemy panel `[Recent Execution]` | Interaction recent execution과 동일한 의미라고 주장 |
| Interaction damage | combat result breakdown | `Request`, `Mitigated`, `Final`, `Commit` | 모든 damage formula 성공 |
| EventLog filters | EventLog 표시 범위 제어 | `[Event Log: ...]` | event 미발생 증명 |
| Current AI | selected Enemy current AI state | `Controller`, `Pawn`, `Target`, `IntentState` | BT active node 전체 추적 |
| Recent AI Event | 최근 AI task event | `Task`, `Result`, `Age` 또는 `Stale Time` | current AI state |

## 6. 성공 Claim에서 제외할 항목

다음 항목은 P1 FinalCandidate 성공 claim에서 제외한다.

- `Runtime LOD: N/A`
- Behavior Tree active node 전체 추적
- EventLog display filter를 event 미발생 증명처럼 설명하는 것
- Recent AI Event를 current AI처럼 설명하는 것
- `BT_Default.uasset` Patrol range 변경을 HUD 기능처럼 설명하는 것
- PIE 검증 캡처를 FinalCandidate로 승격하는 것
- `EnemySource: None` 상태에서 Enemy current state를 성공 evidence처럼 설명하는 것
- 범용 target system / lock-on / combat target flow 변경처럼 설명하는 것
- Shipping HUD / gameplay HUD / UMG / Slate HUD처럼 보이는 설명

## 7. 제외 / 보류할 캡처 기준

다음 조건에 해당하는 이미지는 FinalCandidate에서 제외하거나 보류한다.

| 조건 | 처리 |
| --- | --- |
| editor console window가 overlay 주요 claim을 가림 | 제외 |
| Output Log 창이 EventLog / Interaction claim을 가림 | 제외 |
| Windows taskbar / tooltip / notification 노출 | 제외 |
| EventLog panel이 화면 밖으로 잘림 | 재촬영 |
| Player / Enemy / Interaction / EventLog panel overlap | 재촬영 |
| `EnemySource: None`인데 Enemy current state를 성공 evidence처럼 보임 | 제외 또는 claim 수정 |
| `Runtime LOD: N/A`가 Runtime LOD 성공처럼 보임 | 제외 또는 claim 수정 |
| text clipping 또는 blur가 심함 | 재촬영 |
| claim과 무관한 EventLog noise가 장면 해석을 방해함 | filter 조정 후 재촬영 |
| Recent AI Event stale을 Current AI evidence처럼 해석해야만 의미가 생김 | 제외 |
| `Request / Mitigated / Final / Commit` 중 핵심 값이 읽히지 않음 | damage breakdown evidence에서 제외 |

## 8. FinalCandidate 파일명 규칙

최종 후보 파일명은 다음 규칙을 따른다.

```text
debug_overlay_p1_final_<scene>.png
```

확정 후보 파일명:

- `debug_overlay_p1_final_idle.png`
- `debug_overlay_p1_final_move_walk.png`
- `debug_overlay_p1_final_move_run.png`
- `debug_overlay_p1_final_guard_in.png`
- `debug_overlay_p1_final_guard_out.png`
- `debug_overlay_p1_final_player_hit.png`
- `debug_overlay_p1_final_enemy_hit.png`
- `debug_overlay_p1_final_block_hit.png`
- `debug_overlay_p1_final_parry.png`
- `debug_overlay_p1_final_enemy_stagger.png`
- `debug_overlay_p1_final_stagger_stack_1.png`
- `debug_overlay_p1_final_stagger_stack_2.png`
- `debug_overlay_p1_final_stagger_reset.png`
- `debug_overlay_p1_final_target_nearest.png`
- `debug_overlay_p1_final_target_none.png`
- `debug_overlay_p1_final_enemy_recent_execution.png`
- `debug_overlay_p1_final_interaction_combat_damage.png`
- `debug_overlay_p1_final_eventlog_all.png`
- `debug_overlay_p1_final_eventlog_execution.png`
- `debug_overlay_p1_final_eventlog_combat.png`
- `debug_overlay_p1_final_eventlog_ai.png`
- `debug_overlay_p1_final_enemy_current_ai.png`
- `debug_overlay_p1_final_enemy_recent_ai_event.png`

같은 장면 후보가 여러 장이면 FinalCandidate 패키지에는 대표 1장만 남긴다. 비교/보류 파일은 별도 review 폴더로 분리한다.

## 9. 저장 위치

P1 FinalCandidate 저장 위치:

```text
Docs/98_Evidence/01_Screenshot/DebugOverlay/FinalCandidate
```

분리 기준:

- `Round1`과 섞지 않는다.
- `Round1_StaggerCount`와 섞지 않는다.
- PIE 검증 캡처와 섞지 않는다.
- 최종 후보로 확정되지 않은 이미지는 FinalCandidate 폴더에 넣지 않는다.

## 10. 촬영 전 점검 순서

1. Live Coding이 꺼져 있는지 확인한다.
2. TestRoom PIE 또는 의도한 검증 맵으로 진입한다.
3. `Portfolio.DebugOverlay.Enabled 1`을 적용한다.
4. `Portfolio.DebugOverlay.Collect 1`을 적용한다.
5. 기본 filter / limit / noise CVar를 적용한다.
6. `DebugOverlaySelectNearestTarget`으로 selected Enemy를 지정한다.
7. `Pannel_01 / Pannel_02 / Pannel_03`가 모두 보이는지 확인한다.
8. `EnemySource: TargetComponent.Nearest`가 필요한 장면에서 보이는지 확인한다.
9. 장면별 EventLog filter를 설정한다.
10. editor console / Output Log / tooltip / taskbar가 claim을 가리지 않는지 확인한다.
11. 캡처 후 파일명을 후보 규칙에 맞춘다.
12. 후보 이미지를 FinalCandidate로 승격할지 별도 검토한다.

## 11. 완료 기준

이 문서가 완료되면 다음 기준이 고정된다.

- P1 FinalCandidate 촬영 장면 목록
- 장면별 evidence claim
- 성공 claim에서 제외할 항목
- 제외 / 보류할 캡처 기준
- 촬영 전 CVar / command
- FinalCandidate 파일명 규칙
- FinalCandidate 저장 위치

## 12. 다음 작업

1. 실제 FinalCandidate 촬영을 진행한다.
2. 촬영 결과를 `FinalCandidate` 폴더에 패키징한다.
3. 최종 evidence package 문서를 작성한다.
4. P52 PR과 포트폴리오 본문에 최종 evidence를 연결한다.
