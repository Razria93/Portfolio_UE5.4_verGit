# Debug Overlay Evidence Map

## 작성 목적

각 overlay 항목이 실제 코드 어디에서 읽히는지, 바로 표시 가능한지, 추가 저장 구조가 필요한지 기록한다.

## 상태 분류

- `Ready`: 현재 코드에서 안정적으로 조회 가능
- `HookNeeded`: 기존 이벤트 지점에 최근 값 저장이 필요
- `ReviewNeeded`: 코드 확인 후 결정 필요
- `Exclude`: 제출 evidence로 부적합

## 초기 항목

| 항목 | 분류 | 예상 근거 위치 | 상태 | 메모 |
| --- | --- | --- | --- | --- |
| ActionState | Action / Reaction | `UCActionComponent`, `FExecutionSnapshot` | ReviewNeeded | 실제 공개 getter 확인 필요 |
| ReactionState | Action / Reaction | `UCReactionComponent`, `FExecutionSnapshot` | ReviewNeeded | 실제 공개 getter 확인 필요 |
| CurrentMontage | Animation | AnimInstance / Montage state | ReviewNeeded | Owner mesh 기준 조회 가능성 높음 |
| ApplyMode | Action / Reaction | `FActionExecutionResult`, `FReactionExecutionResult` | HookNeeded | 최근 decision result 저장 필요 |
| OverlayHandling | Observable Overlay | `FExecutionOrchestratorDebug`, `UCObservableOverlayComponent` | HookNeeded | 기존 debug formatter 존재 |
| HitWindow | CombatSignal | `ACWeaponActor`, `FHitContext` | HookNeeded | 최근 hit window event 저장 필요 |
| DamageSpecKey | CombatSignal / Damage | `FHitContext`, `FCombatSignalTargetPacket` | HookNeeded | 기존 formatter 존재 |
| DefenseOutcome | CombatSignal / Damage | `FCombatSignalTargetPacket`, `FCombatResultPacket` | HookNeeded | target packet 결과 사용 |
| DamageCommit | CombatSignal / Damage | `FCombatSignalTargetPacket`, `FCombatResultPacket` | HookNeeded | committed damage / bool 확인 |
| FinalDamage | CombatSignal / Damage | `FCombatSignalTargetPacket` | HookNeeded | `FinalTakenDamage` 후보 |
| BT State | Enemy AI | Blackboard keys | ReviewNeeded | 표시 대상 enemy 선정 필요 |
| Blackboard Intent | Enemy AI | `CAIKey`, Blackboard | ReviewNeeded | intent key 확인 필요 |
| AI Request | Enemy AI | `UCBTTask_StartCombatAction`, `FAICombatBTDebug` | HookNeeded | task result hook 존재 |
| RuntimeLODTier | Runtime LOD | `FAIRuntimeLODTierResolver` | ReviewNeeded | tier context 조회 방식 확인 필요 |
| BT Interval | Runtime LOD | Runtime LOD policy / BT service | ReviewNeeded | 실제 저장 위치 확인 필요 |
| CSV Capture | Profiling | `Core/Profiling/*` | ReviewNeeded | overlay에는 상태 보조 정보만 표시 |

