# Project Stellar 영상 촬영 디버그 설정

Project Stellar · UE 5.4.4 · 2026-10-23 소스 대조

기준은 PR #133이 병합된 main `aeb9e7ed898cdb6e425c52b48036b92be70bc5c3`다. 아래 명령은 현재 소스에 대조한 **촬영 제안**이다. 이 문서의 프리셋 전체를 이번에 실제 PIE로 실행한 것은 아니다. 사용 전 짧은 시험 녹화로 Actor·패널·도형·이벤트와 내보내기를 확인한다. 콘솔 명령은 한 줄씩 입력한다.

자동 입력과 완료 판정은 [PIE 재현 시나리오](../../06_notes/N_Portfolio_Replay_Scenarios.md), 순찰 맵과 Reverse 절차는 [순찰 전용 재현 장면](../../06_notes/N_Patrol_Replay_Scene.md)을 따른다. 해당 문서의 기존 검증을 새 프리셋 조합 전체의 성공으로 확대하지 않는다.

게임플레이 상한·피해·시간 배율을 바꾸는 명령은 포함하지 않는다. 설정 전 현재 값을 기록하고, 촬영이 끝나면 저장한 값으로 복구하거나 새 세션에서 다시 확인한다. 프리셋 전환마다 공통 설정을 적용해 이전 기능의 도형·로그가 섞이지 않게 한다.

## 촬영 직전 이벤트 로그 초기화

PIE 실행 중 Debug Overlay 에디터 패널 상단 `Event Log Actions → Clear Event Log` 버튼을 누른다. 런타임 콘솔에서는 다음 명령을 사용한다. On/Off 값은 붙이지 않는다.

```text
Portfolio.DebugOverlay.ClearEventLog
```

- 현재 월드의 전체 이벤트 링과 캐릭터별 이벤트 기록을 함께 비운다. `World` / `FocusedEnemy` 전환 후에도 이전 이벤트가 다시 표시되지 않는다.
- Capture Enabled, 필터, 표시 줄 수, Focus 선택을 유지한다. `CaptureEnabled 1`이면 삭제 이후 발생한 이벤트부터 계속 수집한다.
- 캐릭터의 실제 상태, 별도 진단 스냅샷, 다른 월드의 기록, Output Log 및 디스크 로그는 초기화하지 않는다. 캐릭터별 이벤트 기록을 사용하는 최근 Action/Reaction 표시도 다음 이벤트까지 비워진다.
- 기록이 없거나 수집이 꺼진 상태에서도 실행할 수 있다. 삭제 동작 자체는 이벤트 로그에 추가하지 않는다.
- 촬영 순서: 전체 설정 적용 → Focus 선택 → 녹화 준비 → Clear Event Log → 콘솔/에디터 패널에서 게임 화면으로 복귀 → 시연. 삭제 후 자연 발생하는 AI 등 이벤트는 정상적으로 추가되므로 필요한 Scope/Filter를 촬영 전에 지정한다.

현재 소스의 ClearEventLog 구현과 자동 테스트 정의를 대조했다. 이번 문서 작업에서는 빌드·테스트·버튼 클릭을 새로 실행하지 않았다. 선행 PR #128의 검증 범위와 이번 촬영 프리셋의 시험 녹화를 구분한다.

## 공통 설정

양측 사건을 다루는 1대1 전투가 기준이다. 다수 적의 개별 AI를 볼 때는 아래 AI 프리셋에서 범위를 변경한다. `CaptureEnabled 1`은 사건 발생 전에 설정한다.

```text
Portfolio.DebugOverlay.HUDVisible 1
Portfolio.DebugOverlay.CaptureEnabled 1
Portfolio.DebugOverlay.EventLog.Visible 1
Portfolio.DebugOverlay.EventLogLimit 10
Portfolio.DebugOverlay.EventLogScope World
Portfolio.DebugOverlay.EventLogFilter All
Portfolio.DebugOverlay.HideNoiseEvents 0
Portfolio.DebugOverlay.HideCollisionWindowEvents 0
Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget 0
Portfolio.DebugOverlay.Player.Enabled 1
Portfolio.DebugOverlay.Player.Status.Enabled 1
Portfolio.DebugOverlay.Player.RecentActionReaction.Enabled 1
Portfolio.DebugOverlay.Player.Targeting.Enabled 0
Portfolio.DebugOverlay.Player.Locomotion.Enabled 0
Portfolio.DebugOverlay.Player.Knockback.Enabled 0
Portfolio.DebugOverlay.Player.AttackFacing.Enabled 0
Portfolio.DebugOverlay.Player.ExecutionSession.Enabled 0
Portfolio.DebugOverlay.Enemy.Enabled 1
Portfolio.DebugOverlay.Enemy.Focus.Enabled 1
Portfolio.DebugOverlay.Enemy.Status.Enabled 1
Portfolio.DebugOverlay.Enemy.RecentActionReaction.Enabled 1
Portfolio.DebugOverlay.Enemy.CurrentAI.Enabled 0
Portfolio.DebugOverlay.Enemy.RecentAIEvent.Enabled 0
Portfolio.DebugOverlay.Enemy.BalanceCollapse.Enabled 0
Portfolio.DebugOverlay.Enemy.CombatParticipation.Enabled 0
Portfolio.DebugOverlay.Enemy.CombatTargetFacing.Enabled 0
Portfolio.DebugOverlay.Enemy.ExecutionSession.Enabled 0
Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled 0
Portfolio.DebugOverlay.Enemy.Knockback.Enabled 0
Portfolio.DebugOverlay.Enemy.AttackFacing.Enabled 0
Portfolio.DebugOverlay.WorldSummary.Visible 0
Portfolio.DebugOverlay.WorldSummary.CombatParticipation.Enabled 0
Portfolio.DebugOverlay.CombatParticipation.Enabled 0
Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor 0
Portfolio.DebugOverlay.Balance.Enabled 0
Portfolio.DebugOverlay.Balance.DrawWorldText 0
Portfolio.DebugOverlay.Balance.DrawLifecycleBar 0
Portfolio.DebugOverlay.ExecutionSession.Enabled 0
Portfolio.DebugOverlay.Targeting.Enabled 0
Portfolio.DebugOverlay.Movement.Enabled 0
Portfolio.DebugOverlay.CombatTargetFacing.Enabled 0
Portfolio.DebugOverlay.AttackFacing.Enabled 0
Portfolio.DebugOverlay.Knockback.Enabled 0
Portfolio.DebugOverlay.Patrol.Enabled 0
Portfolio.DebugOverlay.Patrol.RecordTrail 0
Portfolio.DebugOverlay.Investigate.Enabled 0
Portfolio.DebugOverlay.WeaponPresentation.Enabled 0
Portfolio.DebugOverlay.Feedback.Enabled 0
Portfolio.DebugOverlay.CounterRush.Enabled 0
Portfolio.Debug.CombatSignalAudit 0
Portfolio.Debug.CombatSignalDump 0
Portfolio.Debug.CombatResultAudit 0
Portfolio.Debug.BalanceAudit 0
Portfolio.Debug.ActionRequestAudit 0
Portfolio.Debug.ReactionRequestAudit 0
Portfolio.Debug.ActionComponentAudit 0
Portfolio.Debug.ReactionComponentAudit 0
Portfolio.Debug.ExecutionOrchestratorDump 0
Portfolio.Debug.ExecutionSessionAudit 0
Portfolio.Debug.CombatTargetFacingAudit 0
Portfolio.Debug.AttackFacingAudit 0
Portfolio.Debug.KnockbackAudit 0
Portfolio.Debug.CounterRushAudit 0
```

Focus는 Editor 패널의 `Select Outliner Focus`를 우선한다. 콘솔에서는 `DebugOverlaySelectNearestFocus` 또는 `DebugOverlaySelectPlayerTargetFocus`를 사용할 수 있다. 실행 후 Enemy Actor 이름을 확인한다. 최근 피격자 선택이 필요하면 사건 이후 `DebugOverlaySelectRecentCombatFocus`를 사용한다. 자동 동기화를 껐으므로 Player 타깃을 바꿔도 Focus가 함께 바뀐다고 가정하지 않는다.

## Clean 일반 플레이

공통 설정 뒤 적용한다. 일반 게임플레이 HUD는 유지한다.

```text
Portfolio.DebugOverlay.HUDVisible 0
Portfolio.DebugOverlay.CaptureEnabled 0
Portfolio.DebugOverlay.Patrol.Enabled 0
Portfolio.DebugOverlay.Patrol.RecordTrail 0
Portfolio.DebugOverlay.Investigate.Enabled 0
Portfolio.DebugOverlay.WeaponPresentation.Enabled 0
Portfolio.DebugOverlay.Feedback.Enabled 0
Portfolio.DebugOverlay.CounterRush.Enabled 0
```

공통 설정에서 순찰·수색·무기/피드백 진단도 끈다. Patrol.Enabled를 끄면 기존 순찰 궤적을 비우므로 필요한 기록은 먼저 내보낸다. 이미 그려진 유지시간 도형이 사라진 뒤 녹화한다. CounterRush는 HUD 밖의 Action 경로에서도 그려지므로 별도로 끈다. 원본 로그를 함께 확보하려면 필요한 Audit와 수집만 다시 켜고 그 상태를 기록한다. 성능 측정에는 이 일반 촬영 프리셋 대신 기존 계측 절차를 사용한다.

## Combat 일반 타격

공통 설정과 다음 Audit를 사용한다. 상태·최근 판단·충돌 이력 외의 드로우는 필요하지 않다.

```text
Portfolio.Debug.CombatSignalAudit 1
Portfolio.Debug.ActionComponentAudit 1
Portfolio.Debug.ReactionComponentAudit 1
```

일반 HUD의 HP 변화, 충돌 이벤트, Enemy의 실제 Reaction 시작을 연결한다. 원본 로그를 저장한다.

## Parry 패링 양측 처리

```text
Portfolio.DebugOverlay.Balance.Enabled 1
Portfolio.DebugOverlay.Enemy.BalanceCollapse.Enabled 1
Portfolio.Debug.CombatSignalAudit 1
Portfolio.Debug.CombatResultAudit 1
Portfolio.Debug.BalanceAudit 1
Portfolio.Debug.ReactionRequestAudit 1
Portfolio.Debug.ReactionComponentAudit 1
```

Scope는 World, Filter는 All을 유지한다. FocusedEnemy만 사용하면 Player 쪽 반응 기록을 놓칠 수 있다. 화면에는 Player HP·패링 반응과 Enemy 균형을 남긴다. 균형 내부 누적값 증가와 HUD 남은 값 감소를 구분한다.

### G5-C1 적 전용 속도 후보의 자동 패링 입력

최종 후보는 공용 공격 몽타주 RateScale 1.0을 유지하고 적의 첫 ComboAttack PlayRate만 0.35로 조정한다. 두 플레이어의 몽타주·공격 배율은 유지한다. 후보 자동 패링은 별도 프로세스 실행 인수 `-Structure02GuardDelay=1.07`을 명시한다. CVar 프리셋에 넣지 않는다. 적 PlayRate 0.5 기준은 기본 0.75초를 사용한다.

2026-10-23 SciFiMap 격리 PIE, 요청 1280×720·실제 녹화 1920×1080, RecordFPS 30·t.MaxFPS 60, 이벤트 수집·표시 켬에서 기준 0.75초와 후보 1.07초의 실제 Parry·정상 export를 확인했다. 후보 기본 0.75초는 실패 대조였다. 단일 120/200ms 패링 시험 결과와 구체적 지연 위치는 [PIE 재현 시나리오](../../06_notes/N_Portfolio_Replay_Scenarios.md)의 G5-C1 항목을 따른다. 모든 프레임 지연에 대한 성공 보장이 아니며 실제 Parry·완료·내보내기 근거로 판정한다. 분리본 검증 기록을 PR 게시·병합 완료로 해석하지 않는다.

C1·C2 실제 파일 경계에서 기본 빌드·102개 자동화·에셋 감사와 콤보·플레이어·패링 PIE를 다시 실행했다. 경계별 패링 결과는 [PIE 재현 시나리오](../../06_notes/N_Portfolio_Replay_Scenarios.md)의 커밋 경계 재검증 표를 따른다. 선행 실패 이력과 조건당 1회·지연 시점 차이의 한계를 유지한다.

## Interrupt 공격 중단

```text
Portfolio.DebugOverlay.HideCollisionWindowEvents 0
Portfolio.Debug.ActionRequestAudit 1
Portfolio.Debug.ReactionRequestAudit 1
Portfolio.Debug.ActionComponentAudit 1
Portfolio.Debug.ReactionComponentAudit 1
Portfolio.Debug.ExecutionOrchestratorDump 1
Portfolio.Debug.CombatSignalAudit 1
```

All 필터를 유지해 실행 판단과 충돌 이벤트를 함께 본다. Executor 시작·종료 Audit는 오버레이 Event Log와 별도이므로 Output Log도 보관한다. Reject/Ignore 비교를 촬영할 때 HideNoiseEvents를 켜지 않는다.

## Rush 저스트 회피와 카운터

```text
Portfolio.DebugOverlay.CounterRush.Enabled 1
Portfolio.DebugOverlay.CounterRush.DrawDuration 3
Portfolio.DebugOverlay.CounterRush.DrawForeground 0
Portfolio.Debug.CounterRushAudit 1
Portfolio.Debug.ActionRequestAudit 1
Portfolio.Debug.ReactionRequestAudit 1
Portfolio.Debug.ActionComponentAudit 1
Portfolio.Debug.ReactionComponentAudit 1
Portfolio.Debug.CombatSignalAudit 1
```

반드시 러시 시작 전에 Enabled 또는 Audit를 켠다. 고정 계획의 회색 선·청록 방향/끝점과 자홍색 실제 이동 경로를 구분한다. 실행 거리·방향·기회가 유효한지 원본 로그와 현재 저작 설정을 함께 확인한다. 해당 기회 시간의 전용 패널이 있다고 가정하지 않는다.

## Execution 균형 붕괴와 처형

```text
Portfolio.DebugOverlay.Balance.Enabled 1
Portfolio.DebugOverlay.Enemy.BalanceCollapse.Enabled 1
Portfolio.DebugOverlay.ExecutionSession.Enabled 1
Portfolio.DebugOverlay.ExecutionSession.DrawStartGeometry 1
Portfolio.DebugOverlay.ExecutionSession.DrawPairLink 1
Portfolio.DebugOverlay.ExecutionSession.DrawWorldText 1
Portfolio.DebugOverlay.Player.ExecutionSession.Enabled 1
Portfolio.DebugOverlay.Enemy.ExecutionSession.Enabled 1
Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled 1
Portfolio.Debug.ExecutionSessionAudit 1
Portfolio.Debug.BalanceAudit 1
Portfolio.Debug.ActionComponentAudit 1
Portfolio.Debug.ReactionComponentAudit 1
```

World / All을 유지한다. 세션 정보가 너무 길면 RecentActionReaction 패널을 끄고 양측 Session과 Enemy Balance를 우선한다. 시작 조건 설명이 끝난 뒤 DrawStartGeometry를 0으로 바꾸면 세션 연결선이 잘 보인다. 같은 세션의 Reserved·Active·Committed·종료 결과를 확인하며 실제 출력된 단계만 자막으로 사용한다.

## Roles 다수 적 역할 분포

```text
Portfolio.DebugOverlay.Player.Enabled 0
Portfolio.DebugOverlay.Enemy.Enabled 0
Portfolio.DebugOverlay.EventLog.Visible 0
Portfolio.DebugOverlay.WorldSummary.Visible 1
Portfolio.DebugOverlay.WorldSummary.CombatParticipation.Enabled 1
Portfolio.DebugOverlay.CombatParticipation.Enabled 1
Portfolio.DebugOverlay.CombatParticipation.DrawWorldText 1
Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing 1
Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor 0
```

다음 명령은 값을 붙이지 않고 조회한다. 출력된 실행 중 값을 기록한다.

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap
Portfolio.AI.RuntimeLOD.EngageAssignmentHitReactiveExtraCap
Portfolio.AI.RuntimeLOD.EngageAssignmentTotalCap
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap
Portfolio.AI.RuntimeLOD.EngageAssignmentObserveCap
```

현재 소스 초기값은 위 순서대로 **2 / 3 / 5 / 6 / 12**이고 도움말과 일치한다. 보존 작업 트리의 1 / 2 / 3 / 1 / 1은 미채택 AI 튜닝값이다. 현재 런타임 값은 덮어쓸 수 있으므로 조회 결과를 별도로 남긴다.

적 수는 역할 분포를 볼 수 있을 만큼 준비하고 카메라·가독성에 맞춘다. 과거 제안의 9~12명이나 과거 기본값을 필수 조건으로 사용하지 않는다. 링 색상은 역할과 참여 근거를 나타내며 Base/Extra Admission은 Summary·상세 값으로 확인한다.

## AI 개별 행동과 HitReactive

공통 설정 뒤 적용한다. Roles 프리셋에 이어 사용할 때도 공통 설정으로 먼저 되돌린다.

```text
Portfolio.DebugOverlay.Player.Enabled 0
Portfolio.DebugOverlay.Enemy.CurrentAI.Enabled 1
Portfolio.DebugOverlay.Enemy.RecentAIEvent.Enabled 1
Portfolio.DebugOverlay.Enemy.CombatParticipation.Enabled 1
Portfolio.DebugOverlay.Enemy.CombatTargetFacing.Enabled 1
Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled 1
Portfolio.DebugOverlay.CombatParticipation.Enabled 1
Portfolio.DebugOverlay.CombatParticipation.DrawWorldText 0
Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing 1
Portfolio.DebugOverlay.CombatTargetFacing.Enabled 1
Portfolio.DebugOverlay.EventLogScope FocusedEnemy
Portfolio.DebugOverlay.EventLogFilter AI
```

HitReactive·TTL·Facing을 함께 확인하는 경우 EventLogFilter를 All로 바꾸고 다음을 추가한다.

```text
Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor 1
Portfolio.Debug.CombatTargetFacingAudit 1
Portfolio.Debug.ReactionComponentAudit 1
```

반응·균형 붕괴를 자세히 볼 때는 Balance.Enabled와 Enemy.BalanceCollapse.Enabled를 1로 켠다. 화면이 길어지면 별도 take로 분리한다. 모든 적의 앵커가 겹치면 해당 사례의 적 수를 줄인다. 촬영한 실제 TTL·앵커 반경을 기록하고, 과거 재현용 TTL 1초를 현재 기본값처럼 사용하지 않는다.

## Motion 어택 페이싱과 넉백

어택 페이싱은 다음만 추가한다. 아래는 Player 공격 사례다. Enemy 사례이면 대응 Enemy 패널을 켜고 Focus를 지정한다.

```text
Portfolio.DebugOverlay.AttackFacing.Enabled 1
Portfolio.DebugOverlay.AttackFacing.DrawWorld 1
Portfolio.DebugOverlay.AttackFacing.DrawLimits 1
Portfolio.DebugOverlay.AttackFacing.DrawDuration 3
Portfolio.DebugOverlay.AttackFacing.DrawForeground 0
Portfolio.DebugOverlay.Player.AttackFacing.Enabled 1
Portfolio.DebugOverlay.EventLogFilter AttackFacing
Portfolio.Debug.AttackFacingAudit 1
```

적 넉백은 공통 설정으로 되돌린 뒤 다음을 추가한다.

```text
Portfolio.DebugOverlay.Knockback.Enabled 1
Portfolio.DebugOverlay.Knockback.DrawWorld 1
Portfolio.DebugOverlay.Knockback.DrawDuration 3
Portfolio.DebugOverlay.Knockback.DrawForeground 0
Portfolio.DebugOverlay.Enemy.Knockback.Enabled 1
Portfolio.DebugOverlay.EventLogFilter Knockback
Portfolio.Debug.KnockbackAudit 1
```

시각적 비교는 바닥이 보이는 각도를 사용한다. Foreground 1은 가려진 도형을 강제로 보이게 하므로 필요한 경우에만 사용하고 기록한다. 진단을 켜도 저작 데이터에서 해당 기능이 꺼져 있거나 유효 기록이 없으면 도형이 나오지 않을 수 있다.

## Targeting 대상 선택 보조 촬영

```text
Portfolio.DebugOverlay.Player.Targeting.Enabled 1
Portfolio.DebugOverlay.Targeting.Enabled 1
Portfolio.DebugOverlay.Targeting.DrawRangeSphere 1
Portfolio.DebugOverlay.Targeting.DrawSelectedTargetSphere 1
Portfolio.DebugOverlay.Targeting.DrawViewLine 1
Portfolio.DebugOverlay.Targeting.DrawDebugText 1
```

처음에는 일반 화면으로 선택·전환·해제를 보여주고, 선택 기준을 설명할 때만 진단 화면을 추가한다. 대상 유지와 카메라 추적은 실제 플레이 화면에서도 확인한다.

## 도구 시연과 촬영 후 정리

Editor 패널에서 Focus를 바꾸고 Character Details·Event Log·World Summary를 하나씩 켜며 기능을 소개한다. 실제 검증 챕터에서는 해당 프리셋만 남긴다. EventLogFilter의 지원 값은 All, ActionReaction, ExecutionSession, Combat, AI, Balance, Death, Facing, Knockback, AttackFacing이며 여러 이름을 임의로 쉼표 연결하지 않는다.

촬영 후 원본 로그를 복사하고 take 기록을 작성한다. 콘솔 화면은 본편에서 잘라도 원본에는 설정 조회 부분을 남긴다. 공개 편집본에는 필요한 옵션만 짧게 안내하고 전체 명령은 이 문서로 연결한다.

## 자동 재현과 내보내기 확인

- `CaptureEnabled`는 이벤트 수집이며 녹화 시작 명령이 아니다. 수동 재현과 녹화 시작·종료를 구분한다.
- 별도 프로세스에서 한 종류의 `StructureNNMode` / `StructureNNOutput` 조합만 사용한다. 매번 새 출력 경로를 지정한다. 순찰은 `Structure10Output`이 필요한 일회성 무인 실행으로 제한한다.
- 성공 표식과 실패 표식의 배타성, `record_done.txt` / `complete.txt` 계약, 프레임 요청·반환·저장 수, CSV·WAV를 함께 확인한다. 종료 코드 0만으로 합격시키지 않는다.
- 기본 오디오는 `valid_pcm_nonzero`다. 의도적인 무음에만 `-ReplayAllowSilentAudio`를 지정한다. 무음 허용은 효과음·동기화 검증이 아니다.
- 패링 재현의 `Structure02GuardDelay` 기본값은 0.75초다. 현재 fixture에 대한 입력 지연이며 일반 전투 판정창을 바꾸는 설정이 아니다. 다른 공격·환경에서 재확인한다.
- 처형 fixture의 준비 패킷은 자연 패링 누적의 증거가 아니다. `hit`은 최소 한 번의 실제 양수 피해를 요구하며 정확히 4타 성공을 보증하지 않는다.
- 전용 순찰 맵은 `/Game/00_UnitTest/PatrolReplay/L_PatrolReplay`다. 32초 촬영 성공 표식은 실제 BT 지점 도착의 자동 합격 판정이 아니다. Reverse는 별도 복사 맵을 재열고 PIE 시작 전에 설정·저장한 뒤 새 PIE로 확인한다.

세부 실행 인자와 실패 조건은 위의 병합된 안내를 기준으로 한다. 문서에 새 완료 판정이나 새로운 도구를 정의하지 않는다.
