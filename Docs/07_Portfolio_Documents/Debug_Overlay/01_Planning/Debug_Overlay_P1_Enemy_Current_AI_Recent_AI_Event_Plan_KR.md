# Debug Overlay P1 Enemy Current AI / Recent AI Event 분리 계획

## 1. 목적

P1 debug overlay의 Enemy AI 표시는 현재 `Recent AI` 안에 live Blackboard 값과 최근 AI task event 값이 함께 섞여 있다.

이 문서는 Enemy AI 표시를 다음 두 evidence로 분리하기 위한 기준을 고정한다.

- `Enemy Current AI`: 선택된 Enemy의 현재 Blackboard / Controller / Pawn 상태
- `Recent AI Event`: debug hook 또는 event log에 기록된 최근 AI task event

이번 단계는 설계 문서 작성이다. 코드 구현, asset/config 변경, 최종 촬영은 하지 않는다.

## 2. 현재 문제

현재 HUD의 AI 표시는 다음 문제가 있다.

- `IntentState`는 Blackboard에서 읽은 현재 상태인데, 같은 block 안의 `SubState` / `RecentTask`는 과거 task event 값이다.
- `SubState=ComboAttack`은 Engage 상태 아래의 combat action event에 가깝지만, 현재 AI 상태처럼 보일 수 있다.
- Engage가 끝난 뒤에도 마지막 AI event가 남아 `ComboAttack`이 stale 상태로 보일 수 있다.
- Idle/Patrol/ReturnHome 흐름은 Behavior Tree에 존재하지만, 현재 `Recent AI` event hook은 combat action task 중심이라 충분히 설명하지 못한다.
- `AI: NotCaptured` 같은 actor current line은 실제 current AI state를 읽는 값이 아니라 빈 자리처럼 보일 수 있다.

따라서 P1에서는 현재 AI 상태와 최근 AI event를 UI와 문서 claim에서 분리한다.

## 3. 최종 분리 정책

### Enemy Current AI

`Enemy Current AI`는 선택된 Enemy의 현재 Blackboard 값을 직접 읽어 표시한다.

- 현재 frame 기준 값이다.
- stale 개념을 적용하지 않는다.
- 읽을 수 없는 값은 `N/A` 또는 `NotCaptured`로 표시한다.
- Behavior Tree active node를 추정해서 표시하지 않는다.
- `SubState`라는 이름을 current AI에 사용하지 않는다.

### Recent AI Event

`Recent AI Event`는 최근 AI debug hook/event에서 기록된 과거 event를 표시한다.

- 현재 AI 상태가 아니다.
- event time 기준 `Age`를 표시한다.
- 일정 시간이 지나면 `Stale`로 표시한다.
- 선택된 Enemy와 event pawn이 다르면 `NotMatched`로 표시한다.
- `ComboAttack` 같은 combat action 값은 `Task` 또는 `RecentTask`로 표시한다.

## 4. Blackboard Key 조사 결과

아래 key는 `CAIKey.h`와 AI controller / BT 흐름 기준으로 확인한 후보이다.

| 표시 항목 | 실제 source | 읽기 방식 | P1 판단 | 비고 |
| --- | --- | --- | --- | --- |
| Controller | `Enemy->GetController()` | `Cast<ACAIController>` | 표시 | selected enemy 기준 |
| Pawn | selected `ACEnemy` | actor name | 표시 | current target panel과 연결 |
| Target | `CAIKey::Targeting::TargetActor` | `GetValueAsObject` | 표시 | 없으면 `N/A` |
| IntentState | `CAIKey::State::AIIntentState` | `GetValueAsEnum` | 표시 | `Idle`, `Patrol`, `Observe`, `Investigate`, `Chase`, `Alert`, `Engage`, `HitReact`, `Dead` |
| HasLOS | `CAIKey::Perception::bHasLOS` | `GetValueAsBool` | 표시 후보 | target context와 함께 볼 때 의미 있음 |
| DistanceToTarget | `CAIKey::Metric::DistanceToTarget` | `GetValueAsFloat` | 표시 후보 | target 없으면 `N/A` |
| DistanceToHome | `CAIKey::Metric::DistanceToHome` | `GetValueAsFloat` | 표시 후보 | ReturnHome 해석 보조 |
| ReturnHome | `CAIKey::Navigation::bReturnHome` | `GetValueAsBool` | 표시 | `ReturnHome`은 enum state가 아니라 derived 상태 |
| UsePatrol | `CAIKey::Patrol::bUsePatrol` | `GetValueAsBool` | 표시 | Patrol 가능 여부 |
| PatrolMode | `CAIKey::Patrol::PatrolMode` | `GetValueAsEnum` | 표시 후보 | Patrol branch 해석 보조 |
| PatrolIndex / PatrolLocation | `CAIKey::Patrol::*` | int/vector | 보류 | Patrol branch 안에서만 의미가 강함 |
| CombatRole | `CAIKey::Engage::CombatRole` | `GetValueAsEnum` | 표시 후보 | Engage 상태 보강 |
| ShouldEngage | `CAIKey::Engage::bShouldEngage` | `GetValueAsBool` | 표시 후보 | engage 판단 보조 |
| InEngageRange | `CAIKey::Engage::bInEngageRange` | `GetValueAsBool` | 표시 후보 | target distance와 함께 의미 있음 |
| CanCombatAction | `CAIKey::Engage::bCanCombatAction` | `GetValueAsBool` | 표시 후보 | combat action 가능 여부 |
| IsCombatAction | `CAIKey::Engage::bIsCombatAction` | `GetValueAsBool` | 표시 후보 | 현재 combat action 중인지 여부 |
| LastSeenTime / LastKnownLocation | `CAIKey::Perception::*` | float/vector | 보류 | current target처럼 보이면 오해 가능 |

P1 구현은 과도하게 많은 값을 한 번에 넣지 않는다. 1차 권장 표시는 `Controller`, `Pawn`, `Target`, `IntentState`, `ReturnHome`, `UsePatrol`, `HasLOS`, `DistanceToTarget`, `IsCombatAction` 정도로 제한한다.

## 5. Intent / Branch 해석 기준

`EAIIntentState`는 상위 AI 의도 상태이다.

- `Idle`
- `Patrol`
- `Observe`
- `Investigate`
- `Chase`
- `Alert`
- `Engage`
- `HitReact`
- `Dead`

`ReturnHome`은 `EAIIntentState` 값이 아니다. `bReturnHome`, `DistanceToHome`, `HomeLocation` 등으로 해석되는 derived 상태로 다룬다.

`ComboAttack`은 current AI state가 아니다. 현재 코드 기준으로는 `FAICombatBTDebug`가 기록하는 combat action task event의 `SubState` 또는 `Task`에 가깝다.

따라서 다음처럼 분리해서 해석한다.

- Current AI: `IntentState=Engage`, `IsCombatAction=true`
- Recent AI Event: `Task=ComboAttack`, `Result=Started`

Behavior Tree active node를 직접 추적하지 않는 한, `Patrol`, `ReturnHome`, `CombatAttack`을 모두 같은 의미의 `SubState`로 표시하지 않는다.

## 6. Recent AI Event 조사 결과

현재 최근 AI event는 `FDebugOverlaySnapshotStore::RecordAICombatTask`에서 기록된다.

기록 source는 다음 debug hook이다.

- `FAICombatBTDebug::RecordCombatActionTaskSucceededForAudit`
- `FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit`

현재 `FDebugOverlayAISummary`에는 다음 값이 있다.

- `CaptureState`
- `FrameNumber`
- `WorldTimeSeconds`
- `ControllerName`
- `PawnName`
- `TargetName`
- `IntentState`
- `SubState`
- `RequestResult`
- `RejectReason`
- `RuntimeLODTier`
- `Summary`

`RuntimeLODTier` field는 존재하지만 현재 `RecordAICombatTask`에서 실제 tier로 채우지 않는다. 따라서 P1 evidence로 주장하지 않는다.

Store schema/API를 바꾸지 않고도 1차 구현은 가능하다.

- Current AI는 HUD에서 selected Enemy의 Blackboard를 직접 읽는다.
- Recent AI Event는 기존 `LastAI` 또는 `GetRecentEventsForSubjectCopy(..., "AI", EnemyName)`를 사용한다.
- selected Enemy와 event pawn이 다르면 `NotMatched`로 표시한다.
- event age가 threshold를 넘으면 `Stale`로 표시한다.

## 7. 표시 정책

Enemy panel에는 AI를 다음 두 block으로 표시한다.

```text
[Current AI]
Controller: BP_CAIController_C_0
Pawn: BP_CEnemy_C_1
Target: BP_CPlayer_C_0
IntentState: Engage
ReturnHome: false
UsePatrol: true
HasLOS: true
DistanceToTarget: 2150.0
IsCombatAction: true

[Recent AI Event]
Task: ComboAttack
Result: Started
Age: 0.42s
RejectReason: None
```

stale 상태는 다음처럼 표시한다.

```text
[Recent AI Event]
Stale: true
Age: 4.83s
LastPawn: BP_CEnemy_C_1
Note: Not current AI evidence
```

selected Enemy와 최근 AI event pawn이 다르면 다음처럼 표시한다.

```text
[Recent AI Event]
NotMatched
Selected: BP_CEnemy_C_2
LastPawn: BP_CEnemy_C_1
```

target이 없으면 다음처럼 표시한다.

```text
[Current AI]
NoTarget

[Recent AI Event]
NoTarget
```

Player panel에는 AI block을 추가하지 않는다. Player는 AI controller 기반 evidence 대상이 아니므로 `AI: NotCaptured` 같은 current line도 제거 대상으로 본다.

## 8. Stale 기준

`Recent AI Event`는 current AI가 아니므로 age/stale 표시가 필요하다.

권장 기준:

- stale threshold: `3.0s`
- 이유: 기존 recent combat target stale 기준과 동일한 감각으로 운용 가능
- stale event는 화면에 남길 수 있지만 current evidence로 주장하지 않는다.

P1 구현에서 threshold를 CVar로 만들 필요는 없다. 필요하면 P2 tuning 후보로 둔다.

## 9. HUD Layout 기준

기존 layout style lock을 유지한다.

- left panel width/title/header/color 임의 변경 금지
- right EventLog panel 임의 변경 금지
- EventLog separate panel은 계속 EventLog 전용으로 유지
- Enemy panel 안에 `[Current AI]`, `[Recent AI Event]`를 배치
- Interaction panel 위치 변경은 후속 작업으로 분리

이 문서는 AI 표시 의미를 고정하기 위한 것이며 panel layout 재조정 작업이 아니다.

## 10. 구현 방향

다음 구현은 가능하면 `CDebugOverlayHUD.cpp` 중심으로 제한한다.

1. 현재 `AppendEnemyRecentAIBlock`을 분리한다.
   - `AppendEnemyCurrentAIBlock`
   - `AppendEnemyRecentAIEventBlock`
2. Current AI helper에서 `ACAIController`와 `UBlackboardComponent`를 읽는다.
3. Recent AI Event helper에서 기존 `LastAI` 또는 subject AI event를 읽는다.
4. `SubState` 표시명은 `Task` 또는 `RecentTask`로 바꾼다.
5. `Age`, `Stale`, `NotMatched`, `NoTarget` 상태를 명확히 표시한다.
6. `AI: NotCaptured` current line은 제거하거나 `[Current AI]` block으로 대체한다.

Store schema/API 변경은 1차 구현에서 하지 않는다.

## 11. 제외 범위

이번 분리 설계 및 1차 구현에서 제외한다.

- Behavior Tree active node 추적
- Blackboard key 추가
- Store schema/API 변경
- Runtime LOD actual 표시
- AI task 전체 taxonomy 재설계
- Interaction panel 위치 변경
- EventLog layout 변경
- CollisionDisabledIgnored noise filter 재검토
- 최종 촬영/패키징

## 12. 검증 기준

구현 후 PIE에서 다음을 확인한다.

- selected Enemy가 없으면 AI block이 `NoTarget` 또는 `N/A`로 표시된다.
- Idle/Patrol 상황에서 Current AI가 `IntentState`, `UsePatrol`, `ReturnHome` 등을 현재 Blackboard 값으로 표시한다.
- Engage/ComboAttack 상황에서 Current AI는 `IntentState=Engage` 쪽으로 보이고, Recent AI Event는 `Task=ComboAttack`으로 분리된다.
- combat action이 끝난 뒤에도 Recent AI Event가 current state처럼 보이지 않고 `Age` 또는 `Stale`로 구분된다.
- selected Enemy와 LastAI pawn이 다르면 `NotMatched`로 표시된다.
- Player panel에 AI current line이 남아 오해를 만들지 않는다.
- EventLog separate panel, layout style lock, EventLog filter 정책은 변경되지 않는다.

## 13. 다음 작업

바로 다음 작업은 `P1 Enemy Current AI / Recent AI Event 구현`이다.

권장 구현 목표:

`Enemy panel에서 Current AI와 Recent AI Event를 분리 표시하고, Recent AI Event에 Age/Stale/NotMatched를 추가해 ComboAttack 같은 과거 AI task가 현재 AI 상태처럼 보이지 않게 만든다.`
