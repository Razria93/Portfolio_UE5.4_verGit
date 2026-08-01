# Debug Overlay Evidence Map

## 목적

Debug overlay에 표시할 항목이 실제 코드 어디에서 나오는지, 바로 조회 가능한지, 최근값 저장 hook이 필요한지, 또는 제외해야 하는지 기록한다.

이번 문서는 구현 전 정밀분석 결과이며, 아직 overlay 구현을 시작하지 않는다.

## 상태 분류

- `Ready`: 현재 코드에서 read-only 조회가 가능하다.
- `HookNeeded`: 값은 코드 흐름 안에 있지만 overlay가 지속 표시하려면 최근값 저장 또는 event hook이 필요하다.
- `ReviewNeeded`: 의미 정의 또는 표시 대상 선정이 먼저 필요하다.
- `Exclude`: 제출 evidence로 부적합하거나 오해 가능성이 크다.

## 전체 판정 요약

| 항목 | 도메인 | 상태 | 표시 가능 값 | 조회 위치 / hook 후보 | 신뢰도 | 주의점 |
| --- | --- | --- | --- | --- | --- | --- |
| ActionState | Action / Reaction | Ready | `ExecutionState`, active action type/index | `UCStateComponent::GetCurrentExecutionState()`, `UCActionComponent::IsActive()`, `GetActiveActionType()`, `GetActiveActionIndex()` | 높음 | 별도 `EActionState`는 없다. 표시명은 `ExecutionState + ActiveAction`이 정확하다. |
| ReactionState | Action / Reaction | Ready | `ExecutionState`, active reaction type | `UCStateComponent::GetCurrentExecutionState()`, `UCReactionComponent::IsActive()`, `GetActiveReactionType()` | 높음 | 별도 `EReactionState`는 없다. 표시명은 `ExecutionState + ActiveReaction`이 정확하다. |
| CurrentMontage | Animation | ReviewNeeded | active data의 montage 또는 실제 playing montage | `UCActionComponent::GetActiveActionData()`, `UCReactionComponent::GetActiveReactionData()` | 중간 | executor의 `ActiveMontage_Cached`는 public getter가 없다. 정확한 현재 재생 montage는 getter/hook 보강 필요. |
| ApplyMode | Action / Reaction | HookNeeded | `Start`, `Reserve`, `Intervene` | `FActionExecutionResult::ApplyMode`, `FReactionExecutionResult::ApplyMode` | 높음 | decision 순간의 지역/결과 값이라 최근 decision 저장소가 필요하다. |
| OverlayHandling | Observable Overlay | HookNeeded | `ClearGuardState`, `ClearGuardOverlay` 등 | `ResolveObservableOverlayGate()`, `ApplyOverlayHandlings()` | 높음 | 정상 적용/실패를 구분하려면 decision hook과 apply hook 중 어느 지점을 표시할지 정해야 한다. |
| Guard overlay snapshot | Observable Overlay / Guard | Ready | wants/pose/canGuard/canParry/canStart | `UCObservableOverlayComponent::WriteOverlaySnapshot()`, `UCDefenseComponent::WriteOverlaySnapshot()` | 높음 | P0 Guard/Parry evidence로 적합하다. |
| HitWindow | CombatSignal | Ready / HookNeeded | current hit window id, opened 여부, hit event id | `ACWeaponActor::IsHitWindowOpened()`, `GetCurrentHitWindowId()`, `FHitContext.OverlapContext.HitWindowId` | 높음 | 현재 weapon 기준 조회는 Ready. 최근 hit event log는 hook 필요. |
| DamageSpecKey | CombatSignal / Damage | HookNeeded | weapon/action/index | `FHitContext`, `FCombatSignalSourceContext`, `FCombatSignalTargetPacket`, `FCombatResultPacket` | 높음 | packet/context가 흐름 중 지역값으로 지나가므로 최근값 저장이 필요하다. |
| DefenseOutcome | CombatSignal / Damage | HookNeeded | `None`, `Guard`, `Parry` 등 | `FCombatSignalTargetPacket.Result.DefenseOutcome`, `FCombatResultPacket.DefenseOutcome` | 높음 | target 처리 순간의 결과값이다. overlay에는 최근 target packet 기준으로 표시한다. |
| DamageCommit | CombatSignal / Damage | HookNeeded | commit 여부, committed damage | `FCombatSignalTargetResult::CommittedDamage`, `FCombatResultPacket::bDamageCommitted` | 높음 | `bShouldCommitDamage`와 실제 `CommittedDamage`를 혼동하지 않는다. |
| FinalDamage | CombatSignal / Damage | HookNeeded | `FinalTakenDamage` | `UCCombatSignalTargetComponent::ComputeFinalTakenDamage()`, `FCombatSignalTargetResult::FinalTakenDamage` | 높음 | overlay 명칭은 `FinalTakenDamage`가 정확하다. |
| ReactionType | Reaction / Damage | Ready / HookNeeded | active reaction type 또는 damage로 산출된 reaction type | `UCReactionComponent::GetActiveReactionType()`, `UCReactionOrchestratorComponent::ResolveDamageReactionType()` | 중간 | 현재 활성 반응이면 Ready. damage 산출 reaction이면 hook 필요. |
| BT State | Enemy AI | ReviewNeeded | UE BT 실행 상태 또는 현재 노드명 | `ACAIController::StartBehaviorTreeRuntime()`, `StopBehaviorTreeRuntime()` | 낮음 | 프로젝트 wrapper에 현재 BT 노드 저장 구조가 없다. P0에서 제외 권장. |
| Blackboard Intent | Enemy AI | Ready | `EAIIntentState` | `CAIKey::State::AIIntentState`, `UCBTService_UpdateAIIntentState::ChangeAIIntentState()` | 높음 | `BT State` 대신 P0/P1 AI 상태 evidence로 쓰기 좋다. |
| AI Request | Enemy AI | HookNeeded | combat action intent, request result | `UCBTTask_StartCombatAction::ExecuteTask()`, `FAICombatBTDebug` | 높음 | request result는 지역값이다. 최근 AI request 저장 hook 필요. |
| RuntimeLODTier | Runtime LOD | Ready | `CombatCritical`, `CombatSupport`, `Awareness`, `Background`, `Dormant` | `ACAIController::GetCurrentRuntimeLODTier()` | 높음 | `RefreshRuntimeLODTierFromBlackboard()` 호출 시점 기준 snapshot이다. |
| BT Interval | Runtime LOD | HookNeeded | AI intent interval preset/value | `CBTServiceIntervalHelper::GetAIIntentStateInterval()` | 중간 | polling으로 계산하면 CSV counter를 오염시킬 수 있다. 최근 선택값 저장 hook 권장. |
| DistanceToPlayer | Enemy AI | Ready / ReviewNeeded | `DistanceToTarget` | `CAIKey::Metric::DistanceToTarget` | 중간 | 실제 key는 player 전용이 아니라 target 거리다. 표시명은 `DistanceToTarget` 권장. |
| Visible | Enemy AI | Ready | LOS 여부 | `CAIKey::Perception::bHasLOS` | 높음 | 표시명은 `Visible`보다 `HasLOS`가 정확하다. |
| EnemyCount | Runtime LOD / Profiling | HookNeeded | enemy count 또는 engage request count | CombatEngage request/debug 흐름 | 낮음 | 전역 enemy count getter는 확인하지 못했다. P0 제외 권장. |
| CSV Capture | Profiling | HookNeeded | capture 중 여부, profiling gate 상태 | `Core/Profiling/*`, UE CSV profiler helper 후보 | 낮음 | 현재 코드는 CSV stat emit 중심이다. 성공 주장처럼 보이면 안 된다. |

## P0 확정 후보

P0는 바로 evidence로 쓸 수 있거나, 최소 hook으로 정확하게 표시 가능한 항목만 둔다.

- `ExecutionState + ActiveAction`
- `ExecutionState + ActiveReaction`
- `Guard overlay snapshot`
- `RuntimeLODTier`
- `HitWindow`
- `DefenseOutcome`
- `FinalTakenDamage`
- `DamageCommit`
- recent event log 3-5 lines

## P1 후보

- `ApplyMode`
- `OverlayHandling`
- `DamageSpecKey`
- `ReactionType`
- `Blackboard Intent`
- `AI Request`
- `AIIntent interval preset/value`
- `DistanceToTarget`
- `HasLOS`

## 제외 또는 보류

- `BT State`
  - 현재 프로젝트 코드에 현재 BT 노드/상태를 안정적으로 저장하는 wrapper가 없다.
- `EnemyCount`
  - 전역 enemy count getter가 확인되지 않았다.
- `CSV Capture`
  - 현재 프로젝트 profiling 코드는 stat emit 중심이다. overlay에 넣는다면 UE CSV profiler 상태 helper를 별도로 검토해야 한다.
- `FPS 최적화 성공`류 문구
  - 이 overlay의 목적은 성능 성공 주장이 아니라 실행 흐름 evidence다.

## 도메인별 코드 근거

### Action / Reaction

- `UCStateComponent::GetCurrentExecutionState()`
  - 현재 실행 상태 `Idle / Action / Reaction / Dead`를 반환한다.
- `UCActionComponent::IsActive()`, `GetActiveActionType()`, `GetActiveActionIndex()`, `GetActiveActionData()`
  - active action 상태와 data를 조회할 수 있다.
- `UCReactionComponent::IsActive()`, `GetActiveReactionType()`, `GetActiveReactionData()`
  - active reaction 상태와 data를 조회할 수 있다.
- `FActionExecutionResult::ApplyMode`, `FReactionExecutionResult::ApplyMode`
  - decision 결과에는 존재하지만 overlay용 persistent getter는 없다.
- `FExecutionOrchestratorDebug`
  - decision 순간의 snapshot, apply mode, overlay handlings를 이미 log formatting한다.

### Observable Overlay / Guard

- `UCObservableOverlayComponent::WriteOverlaySnapshot()`
  - overlay policy registry를 통해 snapshot을 구성한다.
- `UCDefenseComponent::WriteOverlaySnapshot()`
  - Guard flags를 `FGuardObservableOverlaySnapshot`에 기록한다.
- `UCDefenseComponent` getter
  - `CanStartGuard()`, `WantsGuarding()`, `IsGuardingPose()`, `CanGuard()`, `CanParry()`가 있다.
- `UCObservableOverlayComponent::ApplyOverlayHandlings()`
  - overlay handling 적용 지점이다. 적용 결과까지 표시하려면 hook 후보가 된다.

### CombatSignal / Damage

- `ACWeaponActor::CollisionEnabled()`, `CollisionDisabled()`
  - hit window open/close와 `CurrentHitWindowId` 갱신 지점이다.
- `ACWeaponActor::BuildOverlapContext()`
  - `FOverlapContext::HitWindowId`를 기록한다.
- `UCCombatSignalSourceComponent::BuildSpecKey()`
  - weapon/action/index 기반 `FDamageSpecKey`를 만든다.
- `UCCombatSignalTargetComponent::BuildResult()`
  - `DefenseOutcome`, `FinalTakenDamage`, `CommittedDamage`를 packet result로 모은다.
- `FCombatSignalDebug::RecordTargetAcceptedForAudit()`
  - target packet 기반으로 overlay event log에 쓰기 좋은 값들을 이미 읽는다.
- `FCombatResultDebug::RecordCombatResultReceivedForAudit()`
  - result receiver evidence hook 후보로 적합하다.

### Enemy AI / Runtime LOD

- `CAIKey::State::AIIntentState`
  - blackboard intent 상태 key다.
- `UCBTService_UpdateAIIntentState::ChangeAIIntentState()`
  - intent 상태 갱신 지점이다.
- `ACAIController::GetCurrentRuntimeLODTier()`
  - runtime LOD tier public getter다.
- `ACAIController::RefreshRuntimeLODTierFromBlackboard()`
  - blackboard 기준 tier snapshot 갱신 지점이다.
- `UCBTTask_StartCombatAction::ExecuteTask()`
  - AI combat action request 결과가 지역값으로 존재한다.
- `CBTServiceIntervalHelper::GetAIIntentStateInterval()`
  - Runtime LOD tier 기반 interval 선택 지점이다.
- `CAIKey::Metric::DistanceToTarget`, `CAIKey::Perception::bHasLOS`
  - AI overlay에 표시 가능한 target distance / LOS evidence다.

## 최근값 저장 hook 후보

최소 구현에서는 별도 gameplay state를 만들지 않고, 개발 전용 debug snapshot 저장소에 최근 이벤트만 기록하는 방향이 안전하다.

- Action / Reaction decision
  - `FExecutionOrchestratorDebug::RecordActionExecutionResultForAudit()`
  - `FExecutionOrchestratorDebug::RecordReactionExecutionResultForAudit()`
- Observable overlay handling
  - `UCObservableOverlayComponent::ApplyOverlayHandlings()`
  - `FObservableOverlayDebug::RecordOverlayHandlingRejectedForAudit()`
- CombatSignal / Damage
  - `FCombatSignalDebug::RecordWeaponCollisionWindowForAudit()`
  - `FCombatSignalDebug::RecordTargetAcceptedForAudit()`
  - `FCombatSignalDebug::RecordTargetRejectedForAudit()`
  - `FCombatSignalDebug::RecordCombatResultDispatchForAudit()`
  - `FCombatResultDebug::RecordCombatResultReceivedForAudit()`
- Enemy AI / Runtime LOD
  - `FAICombatBTDebug::RecordCombatActionTaskSucceededForAudit()`
  - `FAICombatBTDebug::RecordCombatActionTaskRejectedForAudit()`
  - `CBTServiceIntervalHelper::GetAIIntentStateInterval()` 내부의 interval selection 직후

