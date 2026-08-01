# Debug Overlay P0.5 Evidence Package Round1

## 목적

이 문서는 P0.5 debug overlay의 1차 evidence 패키지를 정리한다.

현재 패키지는 최종 제출 후보가 아니다. 이후 debug overlay 리팩터링, target component 기반 enemy selection, capture preset 정리, 최종 재촬영이 남아 있으므로 현 시점에서는 동작 확인 및 기술문서 초안용 근거로만 사용한다.

## 패키지 전제

- 브랜치: `feature/debug-overlay-evidence-plan`
- 원본 위치: `C:\Users\starb\Videos\Bandicam\Debug`
- 패키지 위치: `Docs/98_Evidence/01_Screenshot/DebugOverlay/Round1`
- Stagger Count 보강 패키지 위치: `Docs/98_Evidence/01_Screenshot/DebugOverlay/Round1_StaggerCount`
- 대상 overlay: `[Debug Overlay P0.5]`
- 대상 맵: `TestRoom`
- 전역 `GlobalDefaultGameMode` 변경 없음
- `.umap`, `.uasset`, config, `Build.cs` 변경 없음
- EventLog 추가 축약은 이번 패키지 기준에서 제외
- Enemy 선택은 현재 `WorldScanFallback`이며, target component 기반 선택은 P1 후보

## 포함 파일

| 패키지 파일 | 원본 파일 | 장면 목적 | 확인 가능한 evidence | 주의 |
| --- | --- | --- | --- | --- |
| `debug_overlay_p0_5_round1_move_walk.png` | `Move_Walk.png` | 플레이어 걷기 상태 | Player `Movement: Gait=Walk`, speed/current state | 이동 getter 기반 현재값 evidence |
| `debug_overlay_p0_5_round1_move_run.png` | `Move_Run.png` | 플레이어 달리기 상태 | Player `Movement: Gait=Run`, speed/current state | 걷기/달리기 비교용 1차 후보 |
| `debug_overlay_p0_5_round1_guard_in.png` | `GuardIn_Guard.png` | Guard In 입력/상태 | `Action: Guard In`, guard state, recent execution | Guard action index는 화면에 노출하지 않는 정책 확인 |
| `debug_overlay_p0_5_round1_guard_out_reguard_lock.png` | `GuardOut_ReguardLock.png` | Guard Out 및 재가드 흐름 | `Action: Guard Out`, enemy action, guard 관련 combat summary | source 파일명은 `ReguardLock` 의미로 해석 |
| `debug_overlay_p0_5_round1_enemy_hit.png` | `EnemyHit.png` | 플레이어 공격으로 enemy hit 발생 | Player action, Enemy reaction `Hit`, recent execution/combat | Enemy current value가 world scan fallback 기반임을 명시 필요 |
| `debug_overlay_p0_5_round1_player_hit.png` | `PlayerHit.png` | 플레이어 피격 | Player reaction `Hit`, Enemy action, damage commit | 피격/데미지 commit evidence 초안 |
| `debug_overlay_p0_5_round1_parry.png` | `ParryHit.png` | Parry 성공 | Player reaction `Parry`, combat outcome `Parry`, commit `0.000` | 방어 결과 evidence 초안 |
| `debug_overlay_p0_5_round1_enemy_stagger.png` | `EnemyStagger.png` | Parry 이후 enemy stagger | Enemy reaction `Stagger`, parry outcome, event log | parry 결과와 enemy reaction 연결 설명용 |
| `debug_overlay_p0_5_round1_block_hit.png` | `BlockHit.png` | Guard block hit | Player reaction `BlockHit`, guard defense outcome, reduced commit | block/parry 차이 설명용 |

## Stagger Count 보강 파일

다음 파일은 Stagger Count getter 기반 panel 표시 구현 이후 추가한 Round1 보강 evidence다.

이 보강 패키지도 최종 제출 후보가 아니다. 현재 목적은 `Stagger:` 라인이 Player/Enemy panel에 같은 위치로 표시되고, enemy parry stack이 `0/3 -> 1/3 -> 2/3 -> 0/3 reset` 흐름으로 확인되는지를 1차 검증하는 것이다.

| 패키지 파일 | 원본 파일 | 장면 목적 | 확인 가능한 evidence | 주의 |
| --- | --- | --- | --- | --- |
| `debug_overlay_p0_5_round1_stagger_count_idle.png` | `bandicam 2026-07-29 16-09-11-2652026-07-29 16-12-39-159-1.png` | Stagger Count baseline | Player `Stagger: 0/3`, Enemy `Stagger: 0/3` | Enemy 쪽 일부 텍스트 가독성이 낮아 보조 baseline으로만 사용 |
| `debug_overlay_p0_5_round1_stagger_count_stack_1.png` | `bandicam 2026-07-29 16-09-11-2652026-07-29 16-10-03-003-1.png` | parry stack 1회 | Player `Stagger: 0/3`, Enemy `Stagger: 1/3` | Enemy fallback 대상 기준 현재 parry stack |
| `debug_overlay_p0_5_round1_stagger_count_stack_2.png` | `bandicam 2026-07-29 16-09-11-2652026-07-29 16-10-11-011-2.png` | parry stack 2회 | Player `Stagger: 0/3`, Enemy `Stagger: 2/3` | threshold 도달 전 누적 상태 |
| `debug_overlay_p0_5_round1_stagger_count_reset.png` | `bandicam 2026-07-29 16-09-11-2652026-07-29 16-10-18-018-3.png` | threshold 도달 후 reset | Enemy `Reaction: Stagger`, Enemy `Stagger: 0/3` | Stagger 반응 확인 후 count reset 상태 |

Stagger Count 표시 확인 기준:

- Player/Enemy panel 양쪽에 `Stagger:` 라인이 표시된다.
- `Stagger:` 라인은 `Reaction:` 다음, `Guard:` 앞에 위치한다.
- 값은 getter 기반 현재 parry stack이며 누적 총량이 아니다.
- Enemy 값은 현재 P0.5의 `WorldScanFallback` 선택 대상 기준이다.
- `Stagger: 0/3` reset은 Stagger 반응 확인 후 count가 초기화된 상태로 해석한다.

## 보류 파일

다음 파일은 1차 패키지에 포함하지 않았지만, 후속 캡처 또는 보조 evidence로 검토할 수 있다.

- `Equip.png`
- `Unequip.png`
- `Dodge_ActionCancel.png`
- `Dodge_ReactionCancel.png`
- `GuardIn_Parry.png`
- `GuardOut_RegaurdUnlock.png`
- `Idle_Combat.png`
- `Idle_Guard.png`
- `CutVideo.mp4`
- `RawVideo.mp4`

보류 사유:

- 일부 이미지는 mouse control tooltip 또는 editor/taskbar 노출이 있어 최종 제출용으로는 재촬영이 필요하다.
- `RawVideo.mp4`는 원본 기록 성격이 강하므로 최종 evidence 패키지에는 편집본 중심으로 선별하는 것이 낫다.
- equip/unequip/dodge는 현재 debug overlay의 핵심 P0.5 evidence보다 보조 성격이 강하다.

## 현재 패키지 해석 기준

이 패키지에서 주장 가능한 내용:

- P0.5 overlay가 PIE에서 표시된다.
- Player/Enemy panel이 분리되어 표시된다.
- Movement, HP, Action, Reaction, Guard, AI, Runtime LOD 항목이 같은 구조로 배치된다.
- Recent Execution / Combat / AI / Event Log가 HUD에 표시된다.
- Guard In / Guard Out은 runtime index 대신 화면용 subject로 표시된다.
- Combat result의 hit, block, parry, stagger 흐름이 캡처된다.
- Stagger Count 보강 캡처에서는 Player/Enemy panel의 `Stagger:` 현재값과 enemy parry stack 진행을 확인할 수 있다.

이 패키지에서 주장하지 말아야 할 내용:

- 최종 제출 후보 확정
- Shipping HUD 동작
- 성능 최적화 성공
- target component 기반 enemy selection
- Player/Enemy별 EventLog 분리
- EventLog category filter
- Runtime LOD 최종 evidence
- Stagger Count의 누적 총량 또는 장기 통계

## 후속 처리

최종 제출 전에는 다음 기준으로 재패키징한다.

1. debug overlay 리팩터링 완료 후 동일 장면을 재촬영한다.
2. mouse control tooltip, editor panel, taskbar 노출 여부를 캡처 목적에 맞게 정리한다.
3. target component가 구현되면 `WorldScanFallback` 문구가 아닌 실제 target source를 캡처한다.
4. final candidate 폴더를 별도로 만들고 Round1 파일과 섞지 않는다.
5. 최종 문서에는 `Round1`이 아니라 final candidate 경로만 인용한다.
