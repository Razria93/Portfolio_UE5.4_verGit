# Debug Overlay P1 Actor Recent Execution / Recent AI Display Plan

## 1. 목적

이 문서는 P1 debug overlay에서 actor별 recent 정보와 AI recent 정보의 표시 의미를 고정한다.

이번 단계의 핵심은 다음과 같다.

- Player / Enemy panel에는 해당 actor가 최근 실행한 Execution을 별도로 표시한다.
- Interaction panel의 Recent Execution은 world-level latest flow로 유지한다.
- Player / Enemy current state의 `AI: NotCaptured` placeholder는 제거한다.
- AI 정보는 current state가 아니라 `[Recent AI]` evidence block에서 표시한다.
- Recent AI는 현재 AI 상태 전체가 아니라 최근 AI task/event evidence임을 명확히 한다.

## 2. Player / Enemy Recent Execution

Player / Enemy panel의 `[Recent Execution]`은 actor owner 기준 recent execution이다.

기준:

- Player panel: `OwnerName == PlayerActorName`
- Enemy panel: `OwnerName == SelectedEnemyActorName`
- Interaction panel: world-level `LastExecution`

표시 예:

```text
[Player]
...
[Recent Execution]
Owner: BP_CPlayer_C_0
Domain: Reaction
Subject: Parry
Decision: Accept
Apply: Intervene
RejectReason: None

[Enemy]
...
[Recent Execution]
Owner: BP_CEnemy_C_1
Domain: Action
Subject: ComboAttack[0]
Decision: Accept
Apply: Start
RejectReason: None

[Interaction]
[Recent Execution]
Owner: BP_CEnemy_C_1
Domain: Action
Subject: ComboAttack[0]
Decision: Accept
Apply: Start
RejectReason: None
```

Player / Enemy panel은 actor-specific detail이고, Interaction panel은 world interaction flow이다.
따라서 세 블록은 중복이 아니라 서로 다른 질문에 답한다.

## 3. AI current line 제거

기존 actor current state의 `AI: NotCaptured`는 제거한다.

이유:

- Player panel의 AI current line은 의미가 약하다.
- Enemy panel의 `AI: NotCaptured`는 실제 current AI state가 아니라 빈 placeholder처럼 보인다.
- AI evidence는 `[Recent AI]`에서 Controller / Pawn / Target / IntentState / SubState 중심으로 보여주는 것이 더 명확하다.

Actor current state는 다음 범위로 유지한다.

```text
State
Action
Reaction
HP
Stagger
Guard
Movement
Runtime LOD
```

## 4. Recent AI 의미

`[Recent AI]`는 현재 AI 상태 snapshot이 아니라 최근 AI task/event evidence이다.

표시 필드:

```text
Controller
Pawn
Target
IntentState
SubState
Result
RejectReason
```

의미:

- `IntentState`: Blackboard `AIIntentState` 기준 상위 AI 상태
- `SubState`: 실제 실행된 하위 task/event 성격
- `Result`: task/request 결과
- `RejectReason`: 실패 또는 reject 이유

예:

```text
[Recent AI]
Controller: BP_CAIController_C_0
Pawn: BP_CEnemy_C_1
Target: BP_CPlayer_C_0
IntentState: Engage
SubState: ComboAttack
Result: Started
RejectReason: None
```

## 5. SubState 확장 기준

이번 단계에서 `SubState`는 기존 AI combat task hook에서 읽을 수 있는 task intent를 사용한다.

Patrol / Idle / ReturnHome / IdleWait 같은 비전투 하위 상태는 현재 hook에서 안정적으로 읽을 수 있을 때만 표시한다.
읽을 수 없는 값은 추정하지 않는다.

후속 후보:

- BT task 실행 hook 확장
- Patrol / Idle / ReturnHome 하위 상태 표시
- Recent AI stale / age 표시
- Enemy panel actor-specific Recent AI 표시

## 6. 비목표

- CollisionDisabledIgnored filter 문제 해결
- EventLog category filter 의미 변경
- Recent Combat 정책 변경
- Runtime LOD actual 표시
- 전체 BT 상태 추적 시스템 구현
- 범용 target system 구현
- 최종 촬영 / 패키징

## 7. 검증 기준

- Player / Enemy current state에 `AI: NotCaptured`가 표시되지 않는다.
- Player panel `[Recent Execution]`은 Player owner 기준으로 표시된다.
- Enemy panel `[Recent Execution]`은 selected Enemy owner 기준으로 표시된다.
- Interaction `[Recent Execution]`은 world latest execution으로 유지된다.
- `[Recent AI]`는 `IntentState`와 `SubState`를 분리해서 표시한다.
- 읽을 수 없는 AI 값은 성공 evidence처럼 표시하지 않는다.
