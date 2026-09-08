# 포트폴리오 런타임 검증 · 영상 촬영 런북

## 1. 목적과 사용법

이 문서는 포트폴리오의 런타임 검증과 YouTube 영상 촬영 세션에서 사용하는 단일 운영 기준이다.

카테고리 영상의 Shot 선택, 편집 순서, 자막 및 페이지별 증거 추적은 [Portfolio Gameplay Video Evidence Plan](30_Portfolio_Gameplay_Video_Evidence_Plan%20%28KR%29.md)을 따른다. 이 문서는 PIE 환경·Overlay preset·PASS·재촬영 기준만 소유한다.

- 촬영 중에는 본 문서의 **순서**, **Overlay preset**, **성공 조건**을 따른다.
- 각 장면이 끝난 직후에는 영상 타임스탬프와 관측값을 기록한다.
- 촬영이 끝난 뒤에만 A4 문서의 placeholder를 실제 evidence로 교체한다.

이 문서는 기존 Debug Overlay 조작 문서와 A4 결과물을 연결한다. CVar의 상세 의미와 Editor panel 조작은 [Debug Overlay Operation Guide](../Debug_Overlay/02_Operation/Debug_Overlay_Operation_Guide_KR.md), 기존 preset의 기준은 [Debug Overlay Capture Presets](../Debug_Overlay/04_Capture_Presets/Debug_Overlay_Capture_Presets_KR.md)를 따른다.

## 2. 산출물 분리 원칙

한 개의 녹화가 모든 목적을 충족한다고 가정하지 않는다. 동일 시나리오를 다음 두 산출물로 나눈다.

| 산출물 | Overlay | 용도 | 편집 기준 |
| --- | --- | --- | --- |
| 개발용 검증 영상 | ON | 실제 상태, Event Log, world debug, 데이터 검증 | 판정에 필요한 패널·텍스트가 읽혀야 한다. |
| 포트폴리오/YouTube Hero 영상 | OFF | 전투의 시각적 결과와 조작 감각 전달 | A4 Hero와 영상의 주 화면으로 사용한다. |

현재 제출 원본인 `염동섭_UE5_Portfolio_Project_Stellar.html`의 표지 및 각 시스템 Hero는 Debug Overlay 없는 게임플레이 프레임을 요구한다. 반대로 런타임 검증은 Overlay/Event Log를 켠 개발용 검증 영상에서만 확정한다. YouTube 편집본은 clean gameplay를 기본으로 하고, 판단을 설명할 때만 짧은 Overlay 컷을 삽입한다.

## 3. A4 런타임 검증 표현 규칙

### 3.1 사진이 아닌 검증 카드

A4의 `런타임 검증` 영역에는 화면 crop 대신 **관측 데이터 카드**를 넣는다. 영상이 실제 동작을 보이고, 카드는 관측 결과와 그 의미를 빠르게 읽게 한다.

```text
01. Block Hit
Defense Outcome     Guard
Reaction Outcome    BlockHit
Final Damage        2.500
Commit Damage       2.500

판정 의미
Guard는 감쇠 피해를 commit하며 BlockHit 반응으로 처리된다.

Evidence · Debug video 01:24
```

각 카드의 구성은 아래 순서를 고정한다.

1. **시나리오명**: 입력 또는 상태 전이를 포함한 짧은 이름
2. **관측 데이터**: Overlay 또는 Event Log에서 실제 확인한 3~5개 값
3. **판정 의미**: 데이터가 검증하는 계약을 한 문장으로 설명
4. **Evidence**: 최종 편집 영상의 타임스탬프 또는 원본 파일명

### 3.2 값 표기 규칙

- 촬영에서 확인한 값만 일반 텍스트로 확정한다.
- 아직 촬영하지 않은 값은 `Needs Verification`으로 남긴다. 기대값을 관측값처럼 쓰지 않는다.
- 밸런스 조정에 따라 달라지는 수치는 촬영 당시 값임을 `Observed`로 표시하거나, `Commit = 0`, `HP Decreased` 같은 관계형 표현으로 작성한다.
- `N/A`, `NotCaptured`, 잘린 Event Log, 흐린 글자는 최종 증거가 아니다.
- 카드의 `판정 의미`는 성공을 과장하지 않는다. 예: `Runtime LOD: N/A`는 LOD 성공 근거가 아니다.

### 3.3 공통 카드 템플릿

```text
0N. <Scenario>
<Field A>            <Observed value>
<Field B>            <Observed value>
<Field C>            <Observed value>

판정 의미
<이 결과가 검증하는 상태 전이 또는 책임 경계 한 문장>

Evidence · <Video ID / timestamp 또는 source filename>
```

## 4. 촬영 전 공통 준비

### 4.1 맵·녹화 환경

- TestRoom(`/Game/00_UnitTest/TestRoom`)에서 PIE를 시작한다.
- 녹화 시작 전에 콘솔, Editor panel, mouse tooltip이 게임 화면을 가리지 않게 한다.
- 각 장면의 입력 직전과 결과 직후에 약 2초의 여유를 둔다. 이 구간은 카드 값 판독과 영상 편집의 기준점이다.
- 사망, Destroy, terminal 결과 뒤에는 다음 장면 전에 맵/대상을 리셋한다.

### 4.2 첫 번째 단계: 전체 표시 범위(coverage) baseline

첫 단계는 최종 evidence take가 아닌 **사전 점검**이다. 이미 구현한 모든 Character Details block이 실제 PIE에서 표시 가능한지만 확인한다. 이 단계에서만 아래 항목을 모두 켠다. 값이 `None`, `N/A`, `NotCaptured`인 것은 초기 상태에서는 정상일 수 있으나, block 자체가 보이지 않으면 해당 시각화의 검증 실패다.

| Player | Enemy |
| --- | --- |
| Status | Focus |
| Locomotion | Status |
| Targeting | Balance / Collapse |
| Execution Session | Execution Session |
| Recent Action / Reaction | Combat Participation |
|  | Death Lifecycle |
|  | Recent Action / Reaction |
|  | Current AI |
|  | Recent AI Event |
|  | Facing Visible |

`World Summary > Combat Participation`도 함께 켠다. world visualization domain은 첫 단계에서 모두 켤 필요가 없고, Targeting/Balance/Execution/Participation의 월드 표현은 해당 시나리오 단계에서 각각 확인한다.

전체 표시 baseline은 녹화 후보로 저장하지 않는다. 이후 장면에서는 이번 장면의 판정과 무관한 block을 끈다. 이는 구현 누락을 숨기기 위한 것이 아니라, 검증 영상의 핵심 값과 Event Log가 읽히게 하기 위한 preset 전환이다.

### 4.3 공통 Overlay command baseline

```text
Portfolio.DebugOverlay.HUDVisible 1
Portfolio.DebugOverlay.CaptureEnabled 1
Portfolio.DebugOverlay.EventLogLimit 5
Portfolio.DebugOverlay.EventLogFilter All
Portfolio.DebugOverlay.EventLogScope World
Portfolio.DebugOverlay.HideNoiseEvents 1
Portfolio.DebugOverlay.HideCollisionWindowEvents 1
DebugOverlaySelectNearestFocus
```

시작 후 `[Player]`, `[Enemy]`, `[Event Log: ...]`, `[World Summary]`가 읽히는지 확인한다. coverage baseline에서는 위 표의 모든 block label이 보이는지, Focus가 필요한 장면은 `FocusActor`가 의도한 Enemy인지 반드시 확인한다.

### 4.4 재촬영 기준

다음 하나라도 해당하면 해당 장면을 재촬영한다.

- 핵심 값이 `N/A`, `NotCaptured`, `None`으로 남아 있어 claim을 지지하지 못함
- Event Log가 원하는 category/scope가 아니거나, 핵심 행이 화면 밖으로 밀림
- Focused Enemy가 의도한 Actor가 아님
- Overlay 패널끼리 겹치거나 텍스트가 잘림/흐림
- 콘솔 또는 editor 창이 게임·Overlay를 가림

## 5. 장면별 최소 Overlay preset과 카드 기준

이 표에서 기재하지 않은 Character Details / World Debug는 **OFF**가 기본이다. 공통으로 `HUDVisible=1`, `CaptureEnabled=1`, `EventLogLimit=5`를 적용한다. 성공·일반 전이 take는 `HideNoiseEvents=1`, Combo의 Hit Window take만 `HideCollisionWindowEvents=0`으로 바꾼다.

| A4 구간 / 개별 take | Character Details에서 켤 항목 | World Debug에서 켤 항목 | Event Log | 카드에 확정할 값 |
| --- | --- | --- | --- | --- |
| p.4 Target 획득·전환·해제 | `Player: Status`, `Player: Targeting` | Targeting Diagnostics, Selected Target Sphere, View Line, World Debug Text. Range Sphere는 거리 경계 전용 take에서만 ON | Event Log Visible=OFF, World Summary Visible=OFF | Runtime Target, Distance, Dot, Final Score, In Range/View Cone, 해제 뒤 Rotation |
| p.5 Combo 예약 | `Player: Status`, `Player: Recent Action / Reaction` | 없음 | `ActionReaction`, `World`, Noise=1, Collision=1 | Action subject, Decision, Apply, RejectReason. Active/Reserved key·serial은 실제 출력 여부를 확인한 뒤에만 기록 |
| p.5 Hit Window | `Player: Status` | 없음 | `Combat`, `World`, Noise=1, **Collision=0** | Active Action, HitWindowId, Collision state/name/event |
| p.5 Hit 결과 | `Enemy: Status`, `Enemy: Recent Action / Reaction` | 없음 | `Combat`, `World`, Noise=1, Collision=1 | Source/Target, Defense/Reaction, damage/commit, Enemy reaction |
| p.6 Block / Parry / Player Hit | `Player: Status` | 없음 | `Combat`, `World`, Noise=1, Collision=1 | Defense Outcome, Reaction Outcome, Final Damage, Commit Damage, Player HP |
| p.7 Idle→Dodge·Guard→Dodge 성공 | `Player: Status`, `Player: Recent Action / Reaction` | 없음 | `ActionReaction`, `World`, Noise=1, Collision=1 | Incoming action subject, Decision, Apply, Guard state / ClearGuardState |
| p.7 Active→Dodge 거절 | `Player: Status`, `Player: Recent Action / Reaction` | 없음 | `ActionReaction`, `World`, **Noise=0**, Collision=1 | Active state, Decision, Apply 또는 RejectReason |
| p.8 Balance / Collapse | `Enemy: Focus`, `Enemy: Balance / Collapse` | Balance Diagnostics, World Text, Count Segments | `Balance`, `FocusedEnemy`, Noise=1, Collision=1 | Count/Threshold, Lifecycle/Serial, Loop Remaining, Reset/Recovery |
| p.9 Execution Session | `Enemy: Focus`, `Player: Execution Session`, `Enemy: Execution Session` | Execution Session Diagnostics, Pair Link, World Text. Start Geometry는 Activate take에서만 ON | `ExecutionSession`, `FocusedEnemy`, Noise=1, Collision=1 | Role/State/Outcome, Partner/Session, Reservation, Terminal, Geometry |
| p.10 Dead Entry / Finalize | `Enemy: Focus`, `Enemy: Death Lifecycle` | 없음 | `Death`, `FocusedEnemy`, Noise=1, Collision=1 | Health State, Lifecycle, Death Entry, Presentation, Fallback, Finalization |
| p.10 Facing Suppressed | `Enemy: Focus`, `Enemy Facing Visible` | Combat Target Facing Diagnostics | `Facing`, `FocusedEnemy`, Noise=1, Collision=1 | Gameplay Focus, Rotation, Deferred, Last Facing Decision |
| p.16 AI Intent | `Enemy: Focus`, `Enemy: Current AI`, `Enemy: Recent AI Event` | 없음 | `AI`, `FocusedEnemy`, Noise=1, Collision=1 | Target, IntentState, HasLOS, DistanceToTarget, IsCombatAction, latest AI event |
| p.18 Participation | `Enemy: Focus`, `Enemy: Combat Participation` | World Summary: Combat Participation, Combat Participation Diagnostics, World Text, World Ring | 사용하지 않음 | Role, Admission, Evidence/lifetime, Target, Assignment Revision, Protection, target slot summary |
| p.20 Runtime Debug | `Enemy: Focus`, `Enemy: Current AI`, `Enemy: Recent AI Event` | 없음 | `AI`, `FocusedEnemy`, Noise=1, Collision=1 | FocusActor, current AI context, same Actor의 recent AI event |

### 5.1 Event Log가 근거가 아닌 장면

Targeting과 Participation에는 전용 Event Log filter가 없다. Targeting은 `Player: Targeting`과 Targeting World Debug로, Participation은 `Enemy: Combat Participation`과 World Summary/World Text/Ring으로만 주장한다. 이 장면에서 `All` 로그의 우연한 행을 증거로 인용하지 않는다.

### 5.2 Event Log filter 전환 규칙

- **Combo Hit Window**: Collision event가 핵심이므로 `HideCollisionWindowEvents=0`이 필수다. 나머지 Combo take는 `1`로 되돌린다.
- **Dodge Active→Dodge 및 p.11/p.12의 거절 근거**: Reject/Ignore가 핵심이므로 `HideNoiseEvents=0`이 필수다. 성공 take는 `1`로 되돌린다.
- **FocusedEnemy scope**: `Enemy: Focus`를 반드시 켜고, `FocusActor`가 대상 Enemy와 일치하는지 화면에서 확인한다.

### 5.3 현재 Overlay가 직접 증명하지 않는 값

- Targeting의 획득·전환·해제는 Event Log category로 기록되지 않는다.
- Combo의 `Request Serial`, Execution의 `Snapshot Revision`은 현재 Character Details의 직접 출력값이 아니다. 화면에서 실제 표시가 확인되기 전에는 A4 카드의 확정 필드로 쓰지 않는다.
- HP 1 Standard Execution의 pre-check 거절은 일반 Overlay lifecycle event가 아닌 Output Log audit 경로다. 이 take만 `Portfolio.Debug.ExecutionSessionAudit 1`과 Output Log 기록을 별도로 수집한다.
- Combat Target의 Revision/Change Reason과 Participation의 event history는 현 Overlay의 직접 출력 범위를 넘어선다. 현재 표시되는 consumer/current snapshot 이상으로 claim하지 않는다.

## 6. 검증 영상 촬영 순서

아래 순서는 리셋 비용과 상태 의존성을 고려한 기본 순서다. 각 단계에서 성공 조건을 만족할 때만 다음 단계로 이동한다.

1. **사전 점검** — 모든 Character Details block, Event Log, World Summary가 표시 가능한지만 확인한다. 이 take는 evidence로 저장하지 않는다.
2. **Targeting** — p.4 preset으로 대상 획득 → 좌우 전환 → 거리 초과·사망·파괴 등에 의한 해제를 기록한다.
3. **Combo / Hit** — 예약, Hit Window, Enemy Hit Result를 각각 p.5 preset으로 기록한다. Hit Window take에서만 Collision filter를 연다.
4. **Defense** — p.6 preset으로 Player Hit → Block Hit → Parry 순으로 기록한다. 결과 직후 값이 안정된 구간을 남긴다.
5. **Dodge intervention** — p.7 preset으로 Idle → Dodge, Active → Dodge, Guard → Dodge를 각각 기록한다. 거절 take에서만 Noise filter를 연다.
6. **Balance / Collapse** — p.8 preset으로 Parry 누적 → threshold → CollapseIn → Loop TTL → reset/recovery를 기록한다.
7. **Execution** — p.9 preset으로 Reservation → Activate → Commit 또는 Release/Terminal을 기록한다.
8. **Death lifecycle** — p.10 preset으로 Dead entry / Finalize와 Facing Suppressed를 별도 take로 기록한다.
9. **AI Intent / Runtime Debug** — 맵 또는 대상 리셋 후 p.16/p.20 preset으로, 동일 Focused Enemy의 Character Details와 `FocusedEnemy` Event Log를 한 화면에 고정해 기록한다.
10. **Combat Participation** — p.18 preset으로 다수 Enemy 조건의 role/slot과 World Summary를 기록한다.

단계 7~9처럼 terminal 또는 대상 제거가 일어나는 장면 뒤에는 새 PIE 또는 재배치된 Actor로 시작한다. 한 장면의 최근 이력을 다음 장면의 evidence로 재사용하지 않는다.

## 7. Clean Hero 촬영 순서

검증 영상의 각 단계가 통과한 뒤, Overlay를 끄고 아래 Hero 컷을 별도로 촬영한다.

1. 표지용 대표 전투 컷
2. Target Lock
3. Combo / Hit
4. Guard / Parry / Player Hit 비교
5. Guard → Dodge 또는 Action → Dodge
6. Parry 누적 → Collapse
7. Execution Pair
8. Death Presentation

Hero 컷에서는 개발용 world debug, Canvas Overlay, 콘솔을 모두 숨긴다. 단, UI/타깃 마커처럼 실제 게임플레이 읽기에 필요한 표현은 유지한다.

## 8. 촬영 중 기록표

촬영자는 아래 표를 세션 중 채운다. `PASS`가 아닌 행은 A4에 반영하지 않는다.

| 순서 | 장면 | Take / 파일 | 타임스탬프 | 핵심 관측값 | PASS | 재촬영 사유 |
| ---: | --- | --- | --- | --- | --- | --- |
| 1 | Idle baseline |  |  |  |  |  |
| 2 | Targeting acquire/switch/release |  |  |  |  |  |
| 3 | Combo / Hit |  |  |  |  |  |
| 4 | Block / Parry / Player Hit |  |  |  |  |  |
| 5 | Dodge intervention |  |  |  |  |  |
| 6 | Balance / Collapse |  |  |  |  |  |
| 7 | Execution session |  |  |  |  |  |
| 8 | Death lifecycle |  |  |  |  |  |
| 9 | Runtime Debug / AI focused actor |  |  |  |  |  |
| 10 | Combat participation |  |  |  |  |  |

## 9. 촬영 후 문서 반영 절차

1. 각 PASS 장면에서 영상 타임스탬프와 확정 관측값을 추출한다.
2. A4의 런타임 검증 placeholder를 사진이 아닌 데이터 카드로 교체한다.
3. 카드마다 `판정 의미`와 `Evidence`를 넣는다.
4. A4 Hero placeholder에는 clean 영상에서 선택한 프레임만 넣는다.
5. `Needs Capture`는 성공 evidence가 확보된 경우에만 제거한다.
6. A4의 claim이 영상과 카드의 실제 관측값을 넘지 않는지 마지막으로 대조한다.

## 10. 이번 세션의 완료 기준

- 검증 영상과 clean Hero 영상이 분리되어 저장되었다.
- 필요한 각 PASS 장면에 타임스탬프와 관측값이 있다.
- p.20 Runtime Debug에는 같은 Focused Enemy의 Character Details와 Focused Enemy Event Log가 함께 보이는 영상 근거가 있다.
- A4 런타임 검증은 모든 페이지에서 동일한 데이터 카드 형식을 따른다.
- 미검증 값은 성공 claim으로 문서화되지 않았다.
