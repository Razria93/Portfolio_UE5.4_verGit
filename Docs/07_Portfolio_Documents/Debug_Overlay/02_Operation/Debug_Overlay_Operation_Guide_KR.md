# Debug Overlay 운영 가이드

## TestRoom 수동 연결 절차

### 전제

- 전역 `GlobalDefaultGameMode`는 변경하지 않는다.
- TestRoom 경로는 `/Game/00_UnitTest/TestRoom`이다.
- 기존 테스트용 GameMode asset `Content/00_UnitTest/GM_Test.uasset`가 존재한다.
- `ACDebugOverlayGameMode`는 C++ class로 존재하며, non-shipping에서 `ACDebugOverlayHUD`를 `HUDClass`로 사용한다.
- `.umap`, `.uasset` binary asset은 자동 수정하지 않는다. 에디터에서 의도적으로 저장할 때만 변경한다.

### 권장 연결 방식

권장 방식은 TestRoom의 World Settings에서 GameMode Override를 `ACDebugOverlayGameMode`로 지정하는 것이다.

절차:

1. Unreal Editor에서 `/Game/00_UnitTest/TestRoom`을 연다.
2. `World Settings` 패널을 연다.
3. `GameMode Override`를 확인한다.
4. P0 overlay 촬영용으로 `ACDebugOverlayGameMode`를 지정한다.
5. 변경 저장 전 TestRoom만 변경되는지 확인한다.
6. 전역 `DefaultEngine.ini`의 `GlobalDefaultGameMode`는 변경하지 않는다.

대안:

- 기존 `GM_Test` BP를 유지해야 한다면 `GM_Test`의 `HUDClass`만 `ACDebugOverlayHUD`로 지정한다.
- 이 경우에도 변경 저장 전 `GM_Test.uasset`만 변경되는지 확인한다.

### 실행 CVar

PIE 실행 전 또는 실행 중 콘솔에서 다음 값을 설정한다.

```text
Portfolio.DebugOverlay.Enabled 1
Portfolio.DebugOverlay.Collect 1
Portfolio.DebugOverlay.EventLogLimit 5
```

의미:

- `Portfolio.DebugOverlay.Enabled`: Canvas HUD 표시 여부
- `Portfolio.DebugOverlay.Collect`: 기존 debug hook에서 SnapshotStore에 최근 evidence를 기록할지 여부
- `Portfolio.DebugOverlay.EventLogLimit`: 화면에 표시할 최근 event line 수

### Main Panel 섹션 표시 제어

`Panel_01`의 Player/Enemy 정보량은 수집 여부와 분리된 표시 전용 CVar로 제어한다. 모든 CVar의 기본값은 `1`이며, Editor Debug Overlay 패널의 `Main Panel Sections`에서도 같은 값을 세션 단위로 변경할 수 있다.

```text
Portfolio.DebugOverlay.Player.Enabled
Portfolio.DebugOverlay.Player.Status.Enabled
Portfolio.DebugOverlay.Player.Targeting.Enabled
Portfolio.DebugOverlay.Player.Locomotion.Enabled
Portfolio.DebugOverlay.Player.RecentExecution.Enabled

Portfolio.DebugOverlay.Enemy.Enabled
Portfolio.DebugOverlay.Enemy.Focus.Enabled
Portfolio.DebugOverlay.Enemy.Status.Enabled
Portfolio.DebugOverlay.Enemy.CombatParticipation.Enabled
Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled
Portfolio.DebugOverlay.Enemy.RecentExecution.Enabled
Portfolio.DebugOverlay.Enemy.CurrentAI.Enabled
Portfolio.DebugOverlay.Enemy.RecentAIEvent.Enabled
```

- `Player.Enabled` 또는 `Enemy.Enabled`가 `0`이면 해당 Actor 섹션의 모든 하위 블록을 숨긴다. 하위 CVar 값은 변경하지 않으므로 부모를 다시 켜면 이전 선택이 복원된다.
- 하위 CVar는 해당 블록만 숨긴다. Snapshot/EventLog 수집, focus 선택, 타게팅·이동·Combat Participation 월드 디버그에는 영향을 주지 않는다.
- Player와 Enemy를 모두 숨기면 `Panel_01` 자체를 그리지 않고 Event Log/World Summary 패널을 좌측 기준으로 재배치한다.
- Editor의 표시 순서는 실제 Panel_01과 같게 `Player: Status → Locomotion → Targeting → Recent Execution`, `Enemy: Focus → Status → Combat Participation → Death Lifecycle → Recent Execution → Current AI → Recent AI Event`로 유지한다.
- 기존 `Targeting.ShowOverlayDetails`, `Movement.ShowOverlayDetails`, `CombatParticipation.ShowOverlayDetails`는 제거한다. Panel_01 상세 생성은 각 블록의 표시 CVar가 단독으로 결정한다.
- Targeting/Movement/Combat Participation의 기존 도메인 패널에는 도메인 활성화와 월드 디버그 옵션만 남긴다.

### World Summary 섹션 표시 제어

`Panel_03`의 Combat Participation 전역 요약은 Panel_01의 focused Enemy 상세와 독립적으로 제어한다.

```text
Portfolio.DebugOverlay.WorldSummary.CombatParticipation.Enabled
```

- `Enemy.CombatParticipation.Enabled`: focused Enemy 한 명의 개인 상세를 Panel_01에 표시한다.
- `WorldSummary.CombatParticipation.Enabled`: Target별 전체 참여·슬롯 요약을 Panel_03에 표시한다.
- 두 CVar는 독립적이다. 한쪽을 꺼도 다른 위치의 Combat Participation 출력에는 영향을 주지 않는다.
- Target별 World Summary는 폭을 넘는 pipe 한 줄 대신 아래와 같이 의미 단위 행으로 출력한다.

```text
Target: BP_Player_C_0
Engage: 1 / 5
  - Base: 1 / 2
  - HitReactive Extra: 0 / 3
Alert: 0 / 6
Observe: 0 / 6
```

### Combat Participation Evidence 수명 표시

`Enemy.CombatParticipation.Enabled`가 켜진 Panel_01 상세와 `Portfolio.DebugOverlay.CombatParticipation.DrawWorldText`의 Enemy 머리 위 World Text는 현재 활성 Evidence의 수명 상태를 함께 표시한다. 이 표시는 Debug Snapshot 전용이며 Evidence 등록·철회, allocator, Investigate 정책을 바꾸지 않는다.

- Perception Evidence가 LOS를 유지하면 `Perception: LOS`로 표시한다. LOS가 유지되는 동안에는 Evidence가 계속 갱신되므로 카운트다운을 표시하지 않는다.
- LOS가 끊기면 `Perception: Memory 12.3s`처럼 `TargetMemoryTimeout`의 남은 시간을 표시한다.
- HitReactive Evidence가 Reaction 종료를 기다리는 중이면 `HitReactive: Awaiting reaction`으로 표시한다.
- Reaction이 종료되어 post-reaction TTL이 시작되면 `HitReactive: TTL 58.7s`처럼 남은 시간을 표시한다.
- 해당 source의 Active Evidence가 없으면 그 source의 수명 행을 표시하지 않는다.
- 한 Enemy가 여러 Target에 대한 Evidence를 동시에 보유하면, 머리 위 World Text는 Target별 블록을 위쪽으로 누적 배치해 서로 겹치지 않게 한다.

### HitReactive Evidence anchor 월드 표시

`Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor`는 live HitReactive Evidence마다 다음 세 가지를 한 묶음으로 표시한다. 기본값은 `0`이며, Combat Participation Debug 패널의 `HitReactive Evidence Anchor` 체크박스로 변경할 수 있다.

- Cyan point: 유효 hit 수용 시점의 Enemy 위치인 `HitReactiveEvidenceAnchorLocation`
- Cyan line: 현재 Target 위치에서 anchor로 연결되는 leash
- Cyan 2D circle: `HitReactiveEvidenceAnchorRadius`의 유효 반경

Anchor는 Last Known Target Context나 Investigate 위치가 아니라 HitReactive Evidence 자체의 유효성 판정 데이터다. Engage ring과 머리 위 텍스트는 Evidence 조합에 따라 `Perception + HitReactive = Red`, `HitReactive only = Magenta`, `Perception only = Orange`로 표시한다. Alert / Observe / None은 기존 역할 색상을 유지한다.

### 확인 절차

1. `/Game/00_UnitTest/TestRoom`에서 PIE를 실행한다.
2. 화면 좌상단에 `[Debug Overlay P0.5]`가 표시되는지 확인한다.
3. player action, reaction, guard, combat event를 발생시킨다.
4. `Recent Execution`, `Recent Combat`, `Recent AI`, `Event Log`가 갱신되는지 확인한다.
5. event가 아직 없으면 `NotCaptured`로 표시되는 것이 정상이다.
6. 대상 AI 선택 로직이 없는 상태에서는 `RuntimeLODTier`가 `N/A`로 표시될 수 있다.
7. 실제 combat result가 capture되기 전에는 `FinalTakenDamage`가 `NotCaptured`로 표시될 수 있다.

## P0.5 운영 기준

P0.5 overlay는 Player/Enemy 상태 패널과 공통 recent block으로 구성한다.

```text
[Debug Overlay P0.5]
[Player]
[Enemy]
[Recent Execution]
[Recent Combat]
[Recent AI]
[Event Log]
```

Player tab은 blue, Enemy tab은 red로 표시한다. 색상은 대상 구분용이며 gameplay 위험도나 성공/실패 의미를 갖지 않는다.

### 표시 정책

P0.5 표시 문자열은 캡처 evidence 가독성을 우선한다.

| 항목 | 표시 정책 | 예시 |
| --- | --- | --- |
| enum prefix | 제거 | `ExecutionState::Idle` -> `Idle` |
| Action subject | type/index compact | `ComboAttack[1]` |
| Reaction subject | type compact | `Hit`, `Parry` |
| Guard action | index 제거 | `Guard In`, `Guard Out` |
| multi-field 상태값 | pipe 문자로 구분 | `Gait=Run`, `Speed=0.0`, `Dir=0.0` |
| Execution summary | subject 포함 | `Action(Guard In)`, `Decision=Accept`, `Apply=Start`, `RejectReason=None` |

Guard Hold / Guard Hit / Guard Parry는 P0.5에서 별도 action label로 표시하지 않는다. 해당 의미는 Guard 현재값, Reaction, Combat outcome에서 설명한다.

### Enemy focus 의미

현재 Enemy 패널은 explicit focus command 결과를 기준으로 표시한다.

명시 focus가 없으면 다음처럼 표시한다.

```text
EnemyFocusMode: None
EnemyFocusActor: None
EnemyFocusCommand: None
```

주의:

- 이 표시는 "현재 명시 focus 대상이 없음"을 보여주는 개발 전용 상태다.
- Enemy actor-derived 값을 성공 evidence처럼 설명하지 않는다.
- Enemy 선택은 `DebugOverlaySelectNearestFocus` 또는 Editor tooling command를 통해 명시적으로 수행한다.

### EventLog 운영 판단

P0.5에서는 EventLog 추가 축약을 하지 않는다.

이유:

- 현재 compact key/value format이 캡처에서 충분히 읽힌다.
- 더 줄이면 action/combat 흐름 설명에 필요한 key가 사라질 수 있다.
- category filter와 Player/Enemy별 EventLog 분리는 Store subject 정책이 필요하므로 P1로 둔다.

### Evidence 파일 규칙

캡처 파일은 목적이 드러나도록 보관한다.

권장 폴더:

```text
Docs/98_Evidence/01_Screenshot/DebugOverlay/
Docs/98_Evidence/02_Video/DebugOverlay/
```

권장 파일명:

```text
debug_overlay_p0_5_idle_YYYYMMDD.png
debug_overlay_p0_5_guard_in_YYYYMMDD.png
debug_overlay_p0_5_combo_attack_YYYYMMDD.png
debug_overlay_p0_5_hit_YYYYMMDD.png
debug_overlay_p0_5_parry_YYYYMMDD.png
```

Bandicam 원본 파일명은 보존할 수 있지만, 문서에서 참조할 채택본은 의미 기반 이름으로 복사하거나 별도 목록에 매핑한다.

### 문제 해결

Overlay가 보이지 않을 때:

- TestRoom의 `GameMode Override`가 `ACDebugOverlayGameMode`인지 확인한다.
- `Portfolio.DebugOverlay.Enabled`가 `1`인지 확인한다.
- PIE 대상 map이 `/Game/00_UnitTest/TestRoom`인지 확인한다.
- Shipping build가 아닌지 확인한다.

Event Log가 비어 있을 때:

- `Portfolio.DebugOverlay.Collect`가 `1`인지 확인한다.
- Action / Reaction / Combat / AI event가 실제로 발생했는지 확인한다.
- 현재 연결된 hook 범위가 P0 대상인지 확인한다.
- 기존 audit log CVar와 overlay collect CVar는 분리되어 있으므로, 기존 `Portfolio.Debug.*Audit` 값이 꺼져 있어도 collect는 가능해야 한다.

### 금지 사항

- 전역 `GlobalDefaultGameMode`를 변경하지 않는다.
- `.umap`, `.uasset` 저장은 의도적으로만 수행한다.
- `DefaultEngine.ini`, `Build.cs`를 이 절차 때문에 변경하지 않는다.
- UMG/Slate dependency를 추가하지 않는다.
- Shipping HUD처럼 사용하지 않는다.
- 실제 코드에서 읽지 못한 값을 성공 evidence처럼 표시하지 않는다.
- EventLog 추가 축약, category filter, Player/Enemy별 EventLog 분리는 P0.5에서 구현하지 않는다.

## 고정 정책

- 이 브랜치의 문서는 기본적으로 한국어(KR)로 작성한다.
- 작업 단위가 끝나면 권장 커밋 메시지 형식으로 자동 커밋한다.
- 작업 종료 시 항상 다음 작업을 짧게 제안한다.
- 진행 프롬프트는 파일로 만들지 않고 채팅에서 직접 제안한다.
- 기존 사용자 변경은 되돌리지 않는다.
- 구현 전에 실제 코드 위치와 표시 가능 여부를 먼저 확인한다.
- 에이전트 활용이 유효하다고 판단되면 적극적으로 사용한다.

## 작업 브랜치

- `feature/debug-overlay-evidence-plan`

## 작업 범위

이 세션은 debug overlay evidence 작업의 계획, 문서, 구현, 검증을 다룬다.

목표는 완성형 게임 HUD가 아니라 이력서, 포트폴리오, 기술문서, 제출 영상에서 사용할 수 있는 개발 전용 evidence overlay를 만드는 것이다.

## 진행 순서

1. 계획 문서 고정
2. evidence map 작성
3. 구현 위치 확정
4. 최소 overlay 구현
5. Editor build 검증
6. 영상 preset별 촬영 검증
7. 포트폴리오/기술문서에서 사용할 evidence 설명 정리

## 검증 기준

- 빌드가 통과해야 한다.
- overlay enable이 꺼져 있을 때 기존 동작이 변하지 않아야 한다.
- 표시 값은 실제 runtime state 또는 최근 event hook에서 온 값이어야 한다.
- 불확실한 값은 `Pending`, `N/A`, `NotCaptured`처럼 표현한다.
- Shipping 기능처럼 보이거나 동작하지 않도록 개발 전용 gate를 둔다.

## Console Variable 원칙

예상 형태:

```text
Portfolio.DebugOverlay.Enabled
Portfolio.DebugOverlay.Collect
Portfolio.DebugOverlay.Preset
Portfolio.DebugOverlay.EventLogLimit
Portfolio.DebugOverlay.Player.*.Enabled
Portfolio.DebugOverlay.Enemy.*.Enabled
Portfolio.DebugOverlay.WorldSummary.*.Enabled
```

기존 debug cvar와 충돌하지 않도록 `Portfolio.DebugOverlay.*` 네임스페이스를 사용한다.

P0.5 실행 확인에서는 `Enabled`, `Collect`, `EventLogLimit`를 필수로 본다. `Preset`은 후속 preset 확장용이며, 현재 P0.5 수동 캡처 절차의 필수 CVar는 아니다. `Player.*.Enabled`, `Enemy.*.Enabled`는 Panel_01, `WorldSummary.*.Enabled`는 Panel_03의 정보 밀도를 조절하는 운영 CVar다.

## 목표모드 사용 기준

목표모드는 작업 범위가 여러 턴에 걸쳐 이어질 때 사용한다.

권장 목표:

```text
Debug overlay evidence workspace 기준으로 evidence map 확정, 최소 overlay 구현, 빌드 검증까지 완료한다.
```

## 에이전트 활용 기준

에이전트는 불필요한 작업에 형식적으로 사용하지 않는다. 다만 다음 조건에 해당하면 적극적으로 사용한다.

- 조사 범위가 Action / Reaction, CombatSignal / Damage, Enemy AI / Runtime LOD처럼 독립적인 도메인으로 나뉠 때
- 메인 작업이 문서 통합, 구현 판단, 코드 변경을 진행하는 동안 병렬 코드 조사가 가능할 때
- 구현 전 근거 수집, 위험 검토, 후보 비교처럼 병렬 검토가 품질을 높일 때
- 빌드 오류 원인 범위가 넓고 여러 후보를 동시에 확인해야 할 때
- 테스트/검증 로그 분석을 구현 작업과 분리해 진행할 수 있을 때

에이전트에 맡기는 작업은 명확한 범위와 산출물을 가져야 한다.

- 읽기 전용 조사인지, 코드 수정 작업인지 명확히 구분한다.
- 코드 수정 작업은 파일 범위를 분리한다.
- 최종 판단과 문서 반영은 메인 에이전트가 직접 수행한다.
- 하위 에이전트 결과는 그대로 확정하지 않고 코드 근거와 함께 검토한다.

## 알림음 운영 정책

Debug overlay evidence 작업에서는 세션 대화 중 임시로 합의한 알림음 규칙보다 이 문서에 기록된 정책을 우선한다.

### 단일 알림음

기존 사용자 호출음 / 작업 완료음 분리 정책은 폐기한다. 이후 알림음은 `Ring02.wav` 하나만 사용한다.

PowerShell 실행 명령:

```powershell
(New-Object Media.SoundPlayer "C:\Windows\Media\Ring02.wav").PlaySync()
```

### 실행 시점

- 사용자가 세션을 확인해야 하는 응답 종료 시점에 실행한다.
- 원칙적으로 `final` 응답 직전에 실행한다.
- commentary 중간 진행 보고에는 실행하지 않는다.
- 목표모드 중 계속 작업하는 commentary 보고에는 실행하지 않는다.

### 적용 대상

- 실제 작업 완료
- 사용자 결정 요청
- 작업 중단 또는 보류
- 계획 제안
- 프롬프트 제안
- 일반 질문 답변
- 목표모드가 사용자 입력을 기다리거나 완료되는 시점

### 예외

- 알림음 정책 자체를 작성, 수정, 논의하는 응답에서는 실행하지 않는다.
- 사용자가 명시적으로 알림음을 끄라고 하면 실행하지 않는다.
- 알림음 실행 실패는 작업 실패로 보지 않는다.

### 변경 절차

이후 알림음 정책 변경은 반드시 이 운영 문서 갱신을 통해 반영한다.
