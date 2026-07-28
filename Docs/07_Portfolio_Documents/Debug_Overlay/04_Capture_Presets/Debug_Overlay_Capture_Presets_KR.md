# Debug Overlay Capture Presets

## 공통 레이아웃

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

## Preset 1: Action / Reaction 실행 흐름

표시 항목:

- ActionState
- ReactionState
- CurrentMontage
- ApplyMode
- OverlayHandling
- Event Log

## Preset 2: CombatSignal / Damage

표시 항목:

- HitWindow
- DamageSpecKey
- DefenseOutcome
- FinalDamage
- DamageCommit
- Event Log

## Preset 3: Guard / Parry

표시 항목:

- Guard overlay snapshot
- DefenseOutcome
- DamageCommit
- FinalDamage
- ReactionType
- CurrentMontage
- Event Log

## Preset 4: Enemy AI

표시 항목:

- BT State
- Blackboard Intent
- AI Request
- RuntimeLODTier
- CurrentMontage
- Event Log

## Preset 5: Runtime LOD / CSV Profiler 보조

표시 항목:

- EnemyCount
- RuntimeLODTier
- BT Interval
- DistanceToPlayer
- Visible
- CSV Capture 상태

## 주의

Runtime LOD preset은 최적화 성공 주장용 화면이 아니다. tier, interval, profiler capture의 관측 보조 정보로만 사용한다.

