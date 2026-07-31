# Debug Overlay P1 Interaction Event Flow Redesign Plan

## 1. 목적

이 문서는 P1 debug overlay의 EventLog / Recent / Player / Enemy / Interaction 표시 역할을 다시 정리한다.

직전 P1 작업에서는 Player/Enemy panel 안에 subject-specific EventLog를 추가했다. 이 방식은 actor별 관련 event를 분리하는 데에는 유효하지만, 전투 evidence에서 중요한 `Action -> Window Open -> Hit -> 판정` 흐름이 끊겨 보일 수 있다.

따라서 P1의 다음 구현 기준은 다음처럼 재정리한다.

- Player/Enemy panel은 actor current state와 actor별 상세 상태를 보여준다.
- Interaction panel은 world interaction timeline과 최근 실행/전투/AI 흐름을 보여준다.
- EventLog는 actor별 full log가 아니라 Interaction timeline의 일부로 되돌린다.
- combat / execution / AI summary에는 “누가 누구에게 무엇을 했는지”를 명시한다.

이번 단계는 설계 문서 작업이다. 코드 구현, 촬영, 패키징, asset/config/Build.cs 변경은 하지 않는다.

## 2. 결정 배경

### 2.1 Player/Enemy EventLog 분리의 한계

Player/Enemy EventLog를 subject 기준으로 완전히 나누면 다음 문제가 생긴다.

- HitWindow / Collision / TargetAccepted / CombatResult의 순서가 한 눈에 이어지지 않는다.
- Parry / Guard처럼 공격자와 방어자가 함께 관여하는 이벤트는 양쪽 panel에 동시에 표시될 수 있다.
- `Incoming / Outgoing` 같은 방향 label은 packet 방향으로는 맞더라도, “누가 parry/guard를 했는가”라는 evidence 해석에는 혼동을 줄 수 있다.
- `CombatResult/Delivering`, `Delivered`, `PacketReceived`가 함께 표시되면 같은 결과가 여러 번 적용된 것처럼 보일 수 있다.

전투 evidence에서는 actor별 상태보다 interaction timeline이 더 중요한 구간이 있다. 예를 들어 다음 흐름은 한 덩어리로 보여야 한다.

```text
Action request
-> HitWindow open
-> Target accepted/rejected
-> CombatResult received
-> Reaction / Parry / Guard / Damage outcome
```

### 2.2 Interaction panel 필요성

Interaction panel은 world에서 발생한 주요 gameplay interaction을 시간 순서로 설명하는 영역이다.

Player/Enemy panel이 “현재 actor 상태”를 보여준다면, Interaction panel은 “이번 프레임 주변에 어떤 사건이 어떤 순서로 일어났는지”를 보여준다.

이 분리를 통해 다음을 동시에 만족한다.

- Player/Enemy current state는 간결하게 유지한다.
- EventLog는 world timeline으로 읽힌다.
- 전투 관계는 `Attacker -> Defender`, `From -> Receiver`처럼 명시한다.
- 최종 evidence에서 성공 주장 범위를 설명하기 쉬워진다.

## 3. 최종 방향

P1 redesign 이후 overlay는 다음 역할 구분을 따른다.

```text
[Player]
Actor current state + player-specific detail

[Enemy]
Selected enemy current state + enemy-specific detail

[Interaction]
World interaction flow + recent summaries + event timeline
```

하단 공용 EventLog block은 제거한다. 대신 Interaction panel 내부에 EventLog block을 둔다.

기존 Player/Enemy full EventLog 분리는 이번 설계로 대체하거나 보류한다. 단, actor-specific recent execution처럼 명확하게 나눌 수 있는 항목은 Player/Enemy panel에 남길 수 있다.

## 4. Panel 역할 분리

### 4.1 Player panel

Player panel은 player actor의 현재 상태와 player-specific detail을 보여준다.

권장 순서:

```text
[Player]
State:
Action:
Reaction:
HP:
Stagger:
Guard:
Movement:
Runtime LOD:
AI:

[Recent Execution]
...
```

정책:

- HP는 Stagger보다 위에 둔다.
- Player full EventLog block은 기본 표시에서 제거한다.
- Player Recent Execution은 actor owner 기준으로 표시할 수 있다.
- Player의 combat relation timeline은 Interaction panel에서 확인한다.

### 4.2 Enemy panel

Enemy panel은 selected enemy actor의 현재 상태와 enemy-specific detail을 보여준다.

권장 순서:

```text
[Enemy]
EnemySource:
EnemyTarget:
EnemySelect:

State:
Action:
Reaction:
HP:
Stagger:
Guard:
Movement:
Runtime LOD:
AI:

[Recent Execution]
...
```

정책:

- Enemy는 명시 target이 있을 때만 상태 evidence로 사용한다.
- `EnemySource: TargetComponent.Nearest`는 명시 command 기반 selection evidence다.
- `EnemySource: None`이면 Enemy current state는 `N/A`로 유지한다.
- Enemy full EventLog block은 기본 표시에서 제거한다.
- Enemy Recent Execution은 selected enemy owner 기준으로 표시할 수 있다.

### 4.3 Interaction panel

Interaction panel은 world interaction flow를 보여준다.

권장 구조:

```text
[Interaction]

[Recent Execution]
...

[Recent Combat]
...

[Recent AI]
...

[Event Log: All]
...
```

정책:

- Interaction panel의 EventLog는 world ring buffer 기반 timeline이다.
- `Portfolio.DebugOverlay.EventLogFilter`는 계속 사용한다.
- EventLog category filter `All / Execution / Combat / AI`는 유지한다.
- Player/Enemy subject-specific full EventLog는 Interaction redesign 이후 기본 표시에서 제외한다.

## 5. Interaction Summary Direction 정책

Interaction summary는 왼쪽에서 오른쪽으로 “원인/시작 주체 -> 대상/수신자” 방향을 유지한다.

### 5.1 Execution

Execution은 owner 기준으로 표시한다.

권장 format:

```text
Owner=BP_CPlayer_C_0 | Domain=Action | Subject=ComboAttack[0] | Decision=Accept | Apply=Start | RejectReason=None
Owner=BP_CPlayer_C_0 | Domain=Reaction | Subject=Parry | Decision=Accept | Apply=Intervene | RejectReason=None
```

정책:

- Action / Reaction 뒤에는 실제 subject를 표시한다.
- Reject / Ignore는 noise filter 대상이 될 수 있다.
- Execution은 Player/Enemy Recent Execution에도 분리 표시할 수 있다.

### 5.2 Combat Target

Combat Target은 attack packet 판정 결과를 `Attacker -> Defender` 방향으로 표시한다.

권장 format:

```text
Attacker=BP_CEnemy_C_1 | Defender=BP_CPlayer_C_0 | Outcome=Parry | Final=0.000 | Commit=0.000 | Accepted=true
Attacker=BP_CPlayer_C_0 | Defender=BP_CEnemy_C_1 | Outcome=None | Final=15.000 | Commit=15.000 | Accepted=true
```

정책:

- `Incoming / Outgoing`은 P1 최종 interaction evidence 표현으로 사용하지 않는다.
- `Outcome=Parry`, `Outcome=Guard`는 defender 측 방어 결과로 설명한다.
- 양쪽 actor가 관여한 interaction이므로 Interaction panel timeline에 두는 것이 자연스럽다.

### 5.3 Combat Result

Combat Result는 result packet의 전달 방향을 `From -> Receiver`로 표시한다.

권장 format:

```text
From=BP_CEnemy_C_1 | Receiver=BP_CPlayer_C_0 | Outcome=Parry | DamageCommitted=false | Commit=0.000
From=BP_CPlayer_C_0 | Receiver=BP_CEnemy_C_1 | Outcome=None | DamageCommitted=true | Commit=15.000
```

정책:

- direction field 순서는 Combat Target과 동일하게 left-to-right 관계를 유지한다.
- `Receiver -> From` 순서는 사용하지 않는다.
- `CombatResult/PacketReceived`는 receiver-side primary evidence 후보로 둔다.
- `CombatResult/Delivering`, `CombatResult/Delivered`는 dispatch diagnostic 후보로 격하한다.

### 5.4 AI

AI는 controller / pawn / target 관계를 명시한다.

권장 format:

```text
Controller=BP_CAIController_C_0 | Pawn=BP_CEnemy_C_1 | Target=BP_CPlayer_C_0 | Intent=ComboAttack | Result=Started | RejectReason=None
```

정책:

- AI summary에는 controller와 pawn을 함께 표시한다.
- target을 실제 코드에서 읽을 수 없으면 `Target=N/A` 또는 표시 생략을 사용한다.
- 실제로 읽지 못하는 값을 성공 evidence처럼 표시하지 않는다.

## 6. CombatResult Event 의미 정리

CombatResult event는 다음 의미로 구분한다.

| Event | 의미 | Interaction 표시 정책 |
| --- | --- | --- |
| `CombatResult/PacketReceived` | receiver가 실제 combat result packet을 받은 hook | primary evidence 후보 |
| `CombatResult/Delivering` | result dispatch 시점 diagnostic | 기본 timeline에서는 숨기거나 diagnostic 후보 |
| `CombatResult/Delivered` | result dispatch 완료 diagnostic | `PacketReceived`와 중복될 수 있으므로 diagnostic 후보 |

`Delivering` / `Delivered`는 engine/gameplay 흐름을 추적할 때 유용하지만, 최종 overlay evidence에서는 같은 결과가 여러 번 처리된 것처럼 보일 수 있다.

따라서 Interaction EventLog의 primary combat result line은 `PacketReceived` 중심으로 정리하는 방향을 권장한다.

## 7. HitWindow / Collision 위치

HitWindow / Collision event는 공격자의 weapon window 상태에 가깝다.

하지만 전투 flow를 설명하려면 다음 순서를 함께 볼 수 있어야 한다.

```text
Execution
-> CollisionEnabled
-> TargetAccepted / TargetRejected
-> CombatResult/PacketReceived
```

따라서 HitWindow / Collision event는 Interaction EventLog에 유지할 수 있다.

정책:

- Player/Enemy subject ownership으로 억지 분리하지 않는다.
- Interaction timeline에서는 window state event로 설명한다.
- actor-specific panel에서는 current state 중심으로 유지한다.

## 8. EventLog Noise Filter 정책

Reject / Ignore event와 Collision window event는 디버깅에는 필요하지만 capture readability를 해칠 수 있다.

P1 redesign에서는 표시 단계 noise/collision filter CVar를 설계한다. 최종 설계는 `Debug_Overlay_P1_EventLog_Noise_Filter_Design_KR.md`를 따른다.

채택 CVar:

```text
Portfolio.DebugOverlay.HideNoiseEvents
Portfolio.DebugOverlay.HideCollisionWindowEvents
```

기본값:

```text
HideNoiseEvents=0
HideCollisionWindowEvents=0
```

정책:

- Store ring buffer에는 모든 event를 계속 저장한다.
- noise/collision filter는 record 단계가 아니라 display/query 단계에서만 적용한다.
- EventLog category filter와 결합해서 적용한다.
- 숨겨진 event를 “발생하지 않았다”는 evidence로 해석하지 않는다.
- `Portfolio.DebugOverlay.EventLogNoiseFilter` 문자열형 후보는 폐기한다.
- Reject / Ignore 제어와 Collision window 제어를 분리한다.

예상 적용 순서:

```text
ring buffer 최신순 순회
-> category filter match
-> noise/collision filter match
-> EventLogLimit 수집
-> Interaction panel 표시
```

## 9. Recent Line Count 정책

Interaction panel에서는 Recent Execution을 여러 줄로 확인할 수 있어야 한다.

후보 CVar:

```text
Portfolio.DebugOverlay.RecentExecutionLimit
```

권장 기본값:

```text
1
```

권장 clamp:

```text
1~5
```

정책:

- 기본 capture 화면은 1줄로 유지한다.
- 분석 중에는 2~5줄로 늘릴 수 있다.
- Player/Enemy actor-specific Recent Execution도 같은 limit을 공유할지 여부는 구현 단계에서 검토한다.
- Recent Combat / Recent AI line count 확장은 P1 후속 후보로 둔다.

## 10. UI / 운용 보정

### 10.1 HP / Stagger 순서

Player/Enemy panel의 current state 순서는 HP를 Stagger보다 위에 둔다.

권장 순서:

```text
HP:
Stagger:
Guard:
Movement:
```

이유:

- HP는 actor survivability의 기본 상태다.
- Stagger는 parry stack 또는 전투 reaction 보조 상태다.
- capture 해석에서는 HP를 먼저 읽는 편이 자연스럽다.

### 10.2 Nearest target radius

`DebugOverlaySelectNearestTarget` 기본 탐색거리는 `3000.f`로 조정한다.

이유:

- `1500.f`는 TestRoom 시작 위치에서 enemy 선택이 실패하기 쉽다.
- 실패 원인은 동작 오류가 아니라 반경 제한이지만, 운용 중 혼동을 만든다.
- `3000.f`는 debug overlay target selection 운용 기본값으로 더 적합하다.

정책:

- line trace 기반 `DebugOverlaySelectTarget`은 복구하지 않는다.
- `DebugOverlaySelectNearestTarget`은 명시 target 선택 경로로 유지한다.
- radius CVar화는 P1 후속 또는 P2 후보로 둔다.

## 11. 기존 설계와의 관계

### 11.1 Player/Enemy EventLog Separation

`Debug_Overlay_P1_Player_Enemy_EventLog_Separation_Implementation_Plan_KR.md`의 actor panel full EventLog 방향은 이번 redesign으로 대체하거나 보류한다.

유지할 수 있는 부분:

- Store subject query 경험
- actor-specific recent execution 후보
- category filter와 subject query 결합 검토

후퇴 또는 보류할 부분:

- Player/Enemy panel 내부 full EventLog block
- actor subject match만으로 interaction flow를 분리하려는 방향

### 11.2 Subject Role Label

`Incoming / Outgoing`은 중간 실험으로 정리한다.

P1 최종 interaction 표현은 다음을 우선한다.

```text
Attacker / Defender
From / Receiver
Owner
Controller / Pawn / Target
```

### 11.3 Subject EventLog Role Filter

role-aware subject match는 interaction 중심 재설계 이후 필요 범위를 다시 판단한다.

기본 방향:

- Interaction timeline은 world event flow를 우선한다.
- actor panel은 current state와 actor-specific recent detail을 우선한다.
- subject role filter는 actor-specific recent/event detail이 실제로 필요한 경우에만 사용한다.

### 11.4 EventLog Category Filter

`Portfolio.DebugOverlay.EventLogFilter`는 유지한다.

Interaction panel의 EventLog header는 다음 형식을 유지한다.

```text
[Event Log: All]
[Event Log: Execution]
[Event Log: Combat]
[Event Log: AI]
```

## 12. 구현 단계 제안

### 12.1 1단계: HUD Layout Redesign

범위:

- Interaction panel 추가
- Player/Enemy full EventLog block 제거
- Interaction panel로 Recent Execution / Recent Combat / Recent AI / EventLog 이동
- 하단 공용 EventLog 제거
- HP / Stagger 순서 변경
- Nearest radius `3000.f` 반영

제외:

- Store summary format 대규모 변경
- noise filter CVar
- recent line count CVar

### 12.2 2단계: Interaction Summary Detail Format

범위:

- Execution summary에 `Owner` 추가
- Combat Target summary에 `Attacker / Defender` 추가
- Combat Result summary에 `From / Receiver` 추가
- AI summary에 `Controller / Pawn / Target` 추가 가능한지 검토 후 반영
- `Incoming / Outgoing` 제거

제외:

- 실제 코드에서 읽을 수 없는 target/controller 값을 성공 evidence처럼 표시

### 12.3 3단계: EventLog Noise Filter

범위:

- `Portfolio.DebugOverlay.EventLogNoiseFilter` 추가
- `All / HideReject / HideIgnore / HideRejectAndIgnore` 처리
- Store는 모든 event를 계속 저장
- display/query 단계에서만 filtering

### 12.4 4단계: Recent Execution Line Count

범위:

- `Portfolio.DebugOverlay.RecentExecutionLimit` 추가
- 기본 `1`
- clamp `1~5`
- Interaction Recent Execution과 actor-specific Recent Execution 적용 범위 확정

### 12.5 5단계: PIE Checklist 갱신

범위:

- Interaction panel 기준 수동 검증 절차 갱신
- EventLog category filter와 noise filter 검증 분리
- Nearest radius `3000.f` 확인
- 최종 촬영 전 통합 검증 기준 갱신

## 13. 비목표

이번 redesign 문서와 후속 1차 구현에서 하지 않을 작업:

- 최종 촬영/패키징
- Runtime LOD actual 표시 구현
- AI detail 대규모 보강
- 범용 target system 구현
- combat targeting 연동
- line trace target select 복구
- UMG/Slate 전환
- Shipping HUD화
- `.umap`, `.uasset`, config, `Build.cs` 변경

## 14. 완료 기준

이번 설계 문서의 완료 기준은 다음과 같다.

- Player / Enemy / Interaction panel 역할이 분리되어 있다.
- EventLog가 Interaction timeline으로 돌아가는 기준이 명시되어 있다.
- Combat direction이 `Attacker -> Defender`, `From -> Receiver`로 고정되어 있다.
- `Incoming / Outgoing`을 최종 evidence 표현으로 쓰지 않는 이유가 정리되어 있다.
- Reject / Ignore noise filter CVar 후보와 display-only 정책이 정리되어 있다.
- Recent Execution line count CVar 후보와 기본값/clamp가 정리되어 있다.
- HP / Stagger 순서 변경과 Nearest radius `3000.f`가 후속 구현 항목으로 포함되어 있다.
- 기존 P1 subject EventLog 분리 문서와의 관계가 정리되어 있다.

## 15. 다음 작업

다음 작업은 `P1 Interaction Panel HUD Layout 구현`이다.

권장 구현 범위:

```text
CDebugOverlayHUD.cpp 중심 HUD layout 재배치
Player/Enemy full EventLog block 제거
Interaction panel 추가
Recent Execution / Recent Combat / Recent AI / EventLog를 Interaction panel로 이동
HP를 Stagger 위로 이동
Nearest radius 3000.f 적용
```

Store summary format, noise filter CVar, recent line count CVar는 후속 단계로 분리한다.
