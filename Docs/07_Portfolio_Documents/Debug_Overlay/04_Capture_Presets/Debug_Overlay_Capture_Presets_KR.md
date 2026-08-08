# Debug Overlay Capture Presets

## 사용 우선순위

현재 제출 캡처 기준은 아래 `P0.5 제출 캡처 프리셋` 섹션이다.

상단의 legacy layout과 Preset 1~5는 P0 초기 설계 및 P1 확장 후보를 포함한 참고 섹션으로 유지한다. P0.5 실제 캡처 파일을 선정할 때는 `Idle Baseline`, `Guard In`, `Guard Out`, `ComboAttack`, `Hit Reaction`, `Parry`, `Enemy Action / Reaction` 프리셋을 우선한다.

## Legacy P0/P1 참고 레이아웃

```text
[Player Execution]
ActionState:
ReactionState:
CurrentMontage:
Overlay:

[Target Combat]
DefenseOutcome:
DamageCommit:
FinalDamage:

[Enemy AI / Runtime LOD]
BT State:
Blackboard Intent:
RuntimeLODTier:

[Event Log]
1.
2.
3.
```

## Legacy Preset 1: Action / Reaction 실행 흐름

표시 항목:

- ActionState
- ReactionState
- CurrentMontage
- ApplyMode
- OverlayHandling
- Event Log

## Legacy Preset 2: CombatSignal / Damage

표시 항목:

- HitWindow
- DamageSpecKey
- DefenseOutcome
- FinalDamage
- DamageCommit
- Event Log

## Legacy Preset 3: Guard / Parry

표시 항목:

- Guard overlay snapshot
- DefenseOutcome
- DamageCommit
- FinalDamage
- ReactionType
- CurrentMontage
- Event Log

## Legacy Preset 4: Enemy AI

표시 항목:

- BT State
- Blackboard Intent
- AI Request
- RuntimeLODTier
- CurrentMontage
- Event Log

## Legacy Preset 5: Runtime LOD / CSV Profiler 보조

표시 항목:

- EnemyCount
- RuntimeLODTier
- BT Interval
- DistanceToPlayer
- Visible
- CSV Capture 상태

## 주의

Runtime LOD preset은 최적화 성공 주장용 화면이 아니다. tier, interval, profiler capture의 관측 보조 정보로만 사용한다.

## P0.5 제출 캡처 프리셋

P0.5 제출 캡처는 현재 구현된 Player/Enemy 패널과 공통 recent block을 기준으로 한다.

공통 전제:

- TestRoom: `/Game/00_UnitTest/TestRoom`
- HUD: `[Debug Overlay P0.5]`
- CVar:

```text
Portfolio.DebugOverlay.Enabled 1
Portfolio.DebugOverlay.Collect 1
Portfolio.DebugOverlay.EventLogLimit 5
```

공통 화면 확인:

- `[Player]` blue tab
- `[Enemy]` red tab
- State / Action / Reaction / Guard / Movement / HP / Runtime LOD / AI
- `[Recent Execution]`
- `[Recent Combat]`
- `[Recent AI]`
- `[Event Log]`

### P0.5 Preset A: Idle Baseline

목적:

- overlay 표시 자체와 Player/Enemy getter polling 상태를 보여준다.
- `None`, `N/A`, `NotCaptured`가 실패가 아니라 초기/미수집 상태 표현임을 설명한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| Player State | `Idle` 또는 현재 execution state |
| Player Action | `None` |
| Player Reaction | `None` |
| Player Movement | `Gait`, `Speed`, `Dir` 등이 pipe 구분으로 표시 |
| Player HP | `HP=current/max`, `DeadState=Alive` |
| EnemyFocusMode | `FocusComponent.NearestFocus` 또는 `None` |
| EventLog | 없으면 `NotCaptured`, 있으면 최근 event |

Evidence 의미:

- debug overlay가 TestRoom에서 표시되는지 확인하는 환경 evidence다.
- combat/action 성공 evidence로 사용하지 않는다.

### P0.5 Preset B: Guard In

목적:

- Guard 입력 시작과 Guard 현재값 변화를 캡처한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| Player Action | `Guard In` |
| Player Guard | `Wants=true`, `Pose=...`, `CanGuard=...` |
| Recent Execution | `Action(Guard In)`, `Decision=Accept`, `Apply=Start`, `RejectReason=None` |
| EventLog | `Execution/DecisionResolved` line 포함 |

Evidence 의미:

- Guard action index를 노출하지 않고 의미 중심 label로 표시한다.
- Guard Hold / Hit / Parry 별도 action label은 P0.5 evidence 대상이 아니다.

### P0.5 Preset C: Guard Out

목적:

- Guard 해제 입력과 Guard 상태 종료 흐름을 캡처한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| Player Action | `Guard Out` 또는 active 종료 후 `None` |
| Player Guard | `Wants=false` 또는 pose/capability 변화 |
| Recent Execution | `Action(Guard Out)`, `Decision=Accept`, `Apply=Start`, `RejectReason=None` |
| EventLog | Guard Out decision line 포함 |

Evidence 의미:

- Guard transition이 action execution 흐름을 통해 처리되는지 보여준다.

### P0.5 Preset D: ComboAttack

목적:

- action subject와 combo index가 compact하게 표시되는지 확인한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| Player Action | `ComboAttack[0]`, `ComboAttack[1]` 등 |
| Recent Execution | `Action(ComboAttack[n])`, `Decision=Accept`, `Apply=Start...` |
| Recent Combat | HitWindow 또는 combat target event |
| EventLog | Execution + Combat line |

Evidence 의미:

- Action orchestration과 combat signal이 같은 캡처 안에서 연결되어 보인다.

### P0.5 Preset E: Hit Reaction

목적:

- damage 결과로 player 또는 enemy reaction subject가 표시되는지 확인한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| Reaction | `Hit` |
| Recent Execution | `Reaction(Hit)`, `Decision=Accept`, `Apply=Intervene...` 또는 유사 flow |
| Recent Combat | `Outcome=None`, `Final=...`, `Commit=...` |
| HP | damage 후 current HP 변화 |

Evidence 의미:

- Reaction orchestration과 combat result가 연결되는 장면으로 사용한다.

### P0.5 Preset F: Parry

목적:

- Parry defense outcome과 reaction 흐름을 캡처한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| Reaction | `Parry` 또는 enemy `Stagger` |
| Recent Combat | `Outcome=Parry`, `Commit=0.000` |
| DamageCommit | `false 0.000` 또는 commit 없음 |
| EventLog | Combat target/result line 포함 |

Evidence 의미:

- Parry가 damage commit을 막는 흐름을 보여준다.
- 단, 성능 성공 주장이나 balance 주장으로 사용하지 않는다.

### P0.5 Preset G: Enemy Action / Reaction

목적:

- Enemy 패널의 State/Action/Reaction/Movement/HP가 Player와 같은 구조로 표시되는지 확인한다.

기대 표시:

| 항목 | 기대 |
| --- | --- |
| EnemyFocusMode | `FocusComponent.NearestFocus` |
| EnemyFocusActor | selected enemy actor name |
| Enemy State | `Idle`, `Action`, `Reaction` 등 |
| Enemy Action | `ComboAttack[n]` 또는 `None` |
| Enemy Reaction | `Hit`, `Stagger`, `None` 등 |
| Enemy HP | `HP=current/max`, `DeadState=Alive` |

Evidence 의미:

- 현재 기준 selected enemy는 explicit focus command 결과로 설명한다.
- Focus Component 기반 enemy selection evidence는 `EnemyFocusMode: FocusComponent.NearestFocus`와 `EnemyFocusActor`를 기준으로 설명한다.

## P0.5 제외 프리셋

다음은 P0.5 제출 캡처 프리셋에서 제외한다.

| 제외 항목 | 이유 |
| --- | --- |
| EventLog 추가 축약 비교 | 현재 형식으로 충분하다고 결정 |
| EventLog category filter | P1 Store/filter 작업 필요 |
| Player/Enemy별 EventLog 분리 | Store subject 분리 필요 |
| Runtime LOD 실제 tier 성공 캡처 | 현재 `N/A` 가능, hook 보강 필요 |
| Focus Component 기반 enemy selection | P1 후보 |
| capture automation | P1 후보 |
