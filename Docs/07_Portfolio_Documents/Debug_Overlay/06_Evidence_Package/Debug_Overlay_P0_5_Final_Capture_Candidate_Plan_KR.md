# Debug Overlay P0.5 Final Capture Candidate Plan

## 1. 목적

이 문서는 P0.5 debug overlay의 최종 재촬영 기준을 고정한다.

Round1과 Round1_StaggerCount는 1차 evidence 패키지이며 최종 제출 후보가 아니다. 최종 후보는 후속 재촬영을 통해 별도 폴더에 패키징한다.

이 문서의 목표:

- 최종 문서/영상에 사용할 필수 캡처 장면을 고정한다.
- 장면별 evidence claim을 과장 없이 정의한다.
- 제외/보류 기준과 파일명 규칙을 확정한다.
- 다음 단계에서 실제 재촬영과 최종 패키징을 바로 진행할 수 있게 한다.

## 2. 기준 패키지

현재 1차 기준 패키지는 다음 두 묶음이다.

| 구분 | 경로 | 용도 |
| --- | --- | --- |
| Round1 | `Docs/98_Evidence/01_Screenshot/DebugOverlay/Round1` | 이동, 가드, 피격, 패리, enemy reaction 1차 확인 |
| Round1_StaggerCount | `Docs/98_Evidence/01_Screenshot/DebugOverlay/Round1_StaggerCount` | Stagger Count 표시와 reset 흐름 1차 확인 |

최종 후보 저장 위치:

```text
Docs/98_Evidence/01_Screenshot/DebugOverlay/FinalCandidate
```

Round1 파일을 final candidate로 승격하지 않는다. 최종 후보는 위 경로에 새로 재촬영한 파일만 둔다.

## 3. 최종 캡처 필수 장면

최종 재촬영 필수 장면은 다음 13개다.

| 순서 | 장면 | 최종 파일명 | 핵심 확인 |
| --- | --- | --- | --- |
| 1 | Idle baseline | `debug_overlay_p0_5_final_idle.png` | Player/Enemy panel, baseline current state |
| 2 | Walk | `debug_overlay_p0_5_final_move_walk.png` | Player movement gait/speed |
| 3 | Run | `debug_overlay_p0_5_final_move_run.png` | Player movement gait/speed |
| 4 | Guard In | `debug_overlay_p0_5_final_guard_in.png` | `Action: Guard In`, guard state |
| 5 | Guard Out | `debug_overlay_p0_5_final_guard_out.png` | `Action: Guard Out`, guard transition |
| 6 | Player Hit | `debug_overlay_p0_5_final_player_hit.png` | Player `Reaction: Hit`, damage commit |
| 7 | Enemy Hit | `debug_overlay_p0_5_final_enemy_hit.png` | Enemy `Reaction: Hit`, player attack |
| 8 | Block Hit | `debug_overlay_p0_5_final_block_hit.png` | block/guard defense outcome |
| 9 | Parry | `debug_overlay_p0_5_final_parry.png` | parry defense outcome, commit `0.000` |
| 10 | Enemy Stagger | `debug_overlay_p0_5_final_enemy_stagger.png` | Enemy `Reaction: Stagger` |
| 11 | Stagger Count stack 1 | `debug_overlay_p0_5_final_stagger_stack_1.png` | Enemy `Stagger: 1/3` |
| 12 | Stagger Count stack 2 | `debug_overlay_p0_5_final_stagger_stack_2.png` | Enemy `Stagger: 2/3` |
| 13 | Stagger Count reset | `debug_overlay_p0_5_final_stagger_reset.png` | Enemy `Reaction: Stagger`, `Stagger: 0/3` |

## 4. 선택 보강 장면

다음 장면은 최종 후보 필수 13개에는 넣지 않지만, 포트폴리오 문서 연결 과정에서 필요하면 추가 재촬영한다.

| 장면 | 권장 파일명 | 목적 | 포함 조건 |
| --- | --- | --- | --- |
| Clean ComboAttack | `debug_overlay_p0_5_final_combo_attack.png` | Player `Action: ComboAttack[n]` 단독 action evidence | attack action 자체를 PF03에서 별도 설명할 때 |
| Enemy Action clear frame | `debug_overlay_p0_5_final_enemy_action.png` | Enemy active action과 AI combat task 연결 보조 | enemy `Action:` 값이 명확히 읽히는 프레임이 있을 때 |

선택 보강 장면은 다음 조건을 만족할 때만 final package에 포함한다.

- 필수 13개 장면으로 설명이 부족하다.
- overlay line이 선명하게 읽힌다.
- EventLog/Recent block이 장면 claim과 충돌하지 않는다.
- `WorldScanFallback` 기반 enemy 표시라는 제한을 문서에 함께 남긴다.

## 5. 장면별 Evidence Claim

| 장면 | 보여줄 시스템 | 확인할 overlay line | 기대 값 | 주장 범위 | 주의 |
| --- | --- | --- | --- | --- | --- |
| Idle baseline | Overlay 기본 상태 | `[Debug Overlay P0.5]`, `[Player]`, `[Enemy]` | 양쪽 panel 표시 | P0.5 overlay가 개발 전용 화면으로 표시됨 | 최종 시스템 성공 주장 아님 |
| Walk | Movement getter | Player `Movement` | `Gait=Walk`, speed 값 | movement component의 현재 debug state 표시 | 물리 truth 전체 검증 아님 |
| Run | Movement getter | Player `Movement` | `Gait=Run`, speed 값 | walk/run 상태 차이 표시 | 캡처 순간 speed가 0일 수 있으면 설명 필요 |
| Guard In | Action/Guard | Player `Action`, `Guard`, Recent Execution | `Guard In`, guard flags | guard 입력과 execution summary 표시 | Guard index를 노출하지 않음 |
| Guard Out | Action/Guard | Player `Action`, `Guard`, Recent Execution | `Guard Out`, guard flags | guard 해제/전환 evidence | Reguard lock/unlock은 장면 문맥으로만 설명 |
| Player Hit | Reaction/Damage | Player `Reaction`, Recent Combat | `Hit`, `FinalTakenDamage`, `DamageCommit` | player 피격과 damage commit 표시 | damage 수치의 밸런스 주장 아님 |
| Enemy Hit | Action/Reaction | Player `Action`, Enemy `Reaction`, Recent Execution | player attack, enemy `Hit` | 공격 결과로 enemy reaction 발생 표시 | Enemy는 `WorldScanFallback` 선택 대상 |
| Block Hit | Guard/CombatResult | Player `Reaction`, Recent Combat | `BlockHit` 또는 guard outcome | block/guard 결과와 감소 damage 표시 | parry 성공과 혼동 금지 |
| Parry | DefenseOutcome | Player `Reaction`, Recent Combat | `Parry`, commit `0.000` | parry outcome과 damage cancel 표시 | timing 완성도/성능 주장 아님 |
| Enemy Stagger | Reaction | Enemy `Reaction`, Recent Execution | `Stagger` | parry stack threshold 이후 enemy stagger 표시 | stack count 총량 주장 아님 |
| Stagger Count stack 1 | Stagger Count getter | Enemy `Stagger` | `1/3` | 현재 parry stack 1단계 표시 | 현재값이며 장기 통계 아님 |
| Stagger Count stack 2 | Stagger Count getter | Enemy `Stagger` | `2/3` | threshold 직전 현재 stack 표시 | Enemy fallback 대상 기준 |
| Stagger Count reset | Reaction/Stagger Count | Enemy `Reaction`, Enemy `Stagger` | `Stagger`, `0/3` | Stagger 반응 확인 후 count reset 상태 | reaction request 내부 성공 로그 주장 아님 |

## 6. 공통 캡처 체크리스트

모든 최종 후보 캡처는 다음을 만족해야 한다.

- `[Debug Overlay P0.5]`가 보인다.
- Player/Enemy panel이 모두 보인다.
- `Stagger:` 라인이 Player/Enemy panel에서 `Reaction:` 다음, `Guard:` 앞에 있다.
- 관련 장면의 핵심 overlay line이 읽힌다.
- EventLog는 3~5 lines 범위로 표시된다.
- relevant Recent block이 장면과 충돌하지 않는다.
- `WorldScanFallback` 상태를 target component evidence처럼 설명하지 않는다.
- `N/A`, `NotCaptured`는 성공 evidence로 사용하지 않는다.
- Runtime LOD가 `N/A`이면 Runtime LOD 성공 evidence로 사용하지 않는다.
- 캡처 목적과 무관한 editor panel, mouse tooltip, taskbar 노출을 피한다.

## 7. 재촬영 주의 사항

- `Idle baseline`은 전체 overlay baseline이므로 Stagger Count 보강용 idle 이미지를 그대로 대체하지 않는다.
- `Guard Out`은 `ReguardLock` 장면으로 의미가 흐려지지 않게 clean `Guard Out` 프레임을 우선한다.
- `Enemy Action`을 claim으로 사용할 경우 enemy active action line이 선명하게 읽히는 별도 프레임을 확보한다.
- `Stagger Count`는 `0/3 -> 1/3 -> 2/3 -> Reaction: Stagger + 0/3 reset` 흐름이 한 묶음으로 해석되게 연속 장면으로 관리한다.

## 8. 제외/보류 기준

다음 조건에 해당하는 이미지는 final candidate에서 제외하거나 보류한다.

| 조건 | 처리 |
| --- | --- |
| mouse tooltip이 overlay 또는 actor를 가림 | 제외 |
| editor panel/taskbar 노출이 캡처 목적과 맞지 않음 | 제외 또는 별도 editor 설정 evidence로 분리 |
| overlay text가 흐리거나 clipping됨 | 재촬영 |
| EventLog가 장면 claim과 반대로 읽힘 | 보류 |
| `EnemyFallback: Ambiguous(Count=N)` | 특정 enemy evidence로 사용하지 않음 |
| `EnemyFallback: NotCaptured(NoEnemy)` | Enemy evidence로 사용하지 않음 |
| Runtime LOD가 `N/A`인데 Runtime LOD 성공처럼 보임 | 제외 또는 claim 수정 |
| Stagger Count를 누적 총량/장기 통계처럼 설명해야만 의미가 생김 | 제외 |
| `FinalTakenDamage`, `DamageCommit`이 `NotCaptured`인 damage scene | damage evidence로 사용하지 않음 |

## 9. 파일명 규칙

최종 후보 파일명은 다음 규칙을 따른다.

```text
debug_overlay_p0_5_final_<scene>.png
```

확정 파일명:

- `debug_overlay_p0_5_final_idle.png`
- `debug_overlay_p0_5_final_move_walk.png`
- `debug_overlay_p0_5_final_move_run.png`
- `debug_overlay_p0_5_final_guard_in.png`
- `debug_overlay_p0_5_final_guard_out.png`
- `debug_overlay_p0_5_final_player_hit.png`
- `debug_overlay_p0_5_final_enemy_hit.png`
- `debug_overlay_p0_5_final_block_hit.png`
- `debug_overlay_p0_5_final_parry.png`
- `debug_overlay_p0_5_final_enemy_stagger.png`
- `debug_overlay_p0_5_final_stagger_stack_1.png`
- `debug_overlay_p0_5_final_stagger_stack_2.png`
- `debug_overlay_p0_5_final_stagger_reset.png`

선택 보강 파일명:

- `debug_overlay_p0_5_final_combo_attack.png`
- `debug_overlay_p0_5_final_enemy_action.png`

같은 장면의 후보가 여러 장이면 최종 패키지에는 대표 1장만 남긴다. 비교용 파일은 별도 review 폴더에 둔다.

## 10. 최종 후보 패키징 기준

최종 패키징 시 문서는 다음 형태로 별도 작성한다.

권장 문서:

```text
Docs/07_Portfolio_Documents/Debug_Overlay/06_Evidence_Package/Debug_Overlay_P0_5_Evidence_Package_Final_KR.md
```

최종 패키지 문서에는 다음을 포함한다.

- 최종 후보 파일 목록
- 장면별 evidence claim
- 사용 가능한 포트폴리오 문서 섹션
- 사용하지 말아야 할 claim
- `WorldScanFallback`, `N/A`, `NotCaptured` 해석 기준

## 11. 완료 기준

이 문서가 완료되면 다음이 고정된 것으로 본다.

- 최종 재촬영 scene list
- 장면별 evidence claim
- 제외/보류 기준
- 최종 후보 파일명 규칙
- final candidate 저장 경로

## 12. 다음 작업

다음 작업은 실제 최종 재촬영 파일을 받아 `FinalCandidate` 폴더에 패키징하고, 최종 evidence package 문서를 작성하는 것이다.

진행 전 필요 파일:

- 위 13개 장면에 대응하는 새 캡처 PNG
- 캡처 원본 위치
- 최종 후보로 승격할지 여부에 대한 사용자 확인
