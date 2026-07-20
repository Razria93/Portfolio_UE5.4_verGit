# N26. Diagnostic Log Full Audit Inventory Note

## 목적

`refactor/debug-log-policy-v1`의 1단계 전수조사 결과를 정리한다.

이 문서는 프로젝트 안에 남아 있는 runtime diagnostic log, CVar audit, CSV counter, 그리고 아직 로그는 없지만 관측 후보가 되는 실패/거절/경계 지점을 분류한다.

책임 범위:

```text
이 문서는 전수조사 / 우선순위 문서다.
파일별 후보, 현재 상태, 권장 처리, 처리 완료 여부, 다음 탐색 순서를 관리한다.
정책 원칙은 N23, cleanup 이력은 N24, helper 설계는 N25에서 관리한다.
```

기준은 다음과 같다.

```text
1. 기본 출력으로 남기면 안 되는 runtime flow log는 CVar audit helper로 분리한다.
2. asset / data / required reference 문제를 찾는 low frequency diagnostic log는 유지하되 context를 보강한다.
3. 정상 정책 reject는 기본 로그로 출력하지 않는다.
4. 반복 가능성이 큰 reject / branch / boundary는 CVar audit 또는 CSV counter 후보로만 둔다.
5. Shipping 빌드에는 debug output 구현이 포함되지 않도록 별도 helper + #if !UE_BUILD_SHIPPING 구조를 우선한다.
```

---

## 분류 요약

| 분류                                     | 상태                   | 판단                                                               |
| -------------------------------------- | -------------------- | ---------------------------------------------------------------- |
| CombatResult runtime flow              | helper gated             | `FCombatResultDebug` + `CombatResultAudit` 처리 완료                     |
| CombatResult dispatch flow             | helper gated         | `FCombatSignalDebug::RecordCombatResultDispatchForAudit` 처리 완료          |
| Action / Reaction data diagnostic      | active 기본 출력         | 유지. asset/data 문제 진단용. 추후 non-shipping helper화 후보                |
| Action / Reaction montage interruption | active 기본 출력         | 유지. 비정상 interruption 진단용. 추후 non-shipping helper화 후보             |
| Feedback data diagnostic               | active 기본 출력         | 유지. 중복/invalid feedback data 진단용                                 |
| Notify invalid trigger                 | active 기본 출력         | 유지. notify asset 설정 오류 진단용. owner/notify context 보강 후보           |
| Movement invalid gait                  | helper gated            | `FMovementDebug` + `MovementAudit` 처리 완료                                  |
| Overlay handling failure               | active 기본 출력         | 유지 후보. 정책 실패 진단용. owner/context 보강 후보                            |
| AI / Engage audit                      | CVar gated           | 유지. 이미 감사 구조로 분리됨                                                |
| CSV profiling counters                 | CSV only             | 유지. 성능 측정용이며 출력 로그가 아님                                           |
| Action / Reaction request reject       | no active log        | 기본 로그 추가 금지. 필요 시 CVar audit/counter 후보                          |
| CombatSignal reject reason             | helper gated         | `FCombatSignalDebug` 처리 완료. 기본 출력 Off                                |

---

## 1. 기본 출력에서 분리해야 하는 후보

### 1-1. CombatResult

| 파일 | 위치 | 현재 상태 | 후보 이벤트 | 빈도 | 권장 처리 |
| --- | --- | --- | --- | --- | --- |
| `Source/Portfolio/Character/Enemy/CEnemy.cpp` | `ReceiveCombatResultPacket` | helper gated | combat result packet 수신/내용 | combat event마다 | `FCombatResultDebug` + `CombatResultAudit` 처리 완료 |
| `Source/Portfolio/Character/Enemy/CEnemy.cpp` | `HandleParryCombatResult` | helper gated | parry stack / stagger threshold | parry result마다 | `FCombatResultDebug` + `CombatResultAudit` 처리 완료 |
| `Source/Portfolio/Character/Enemy/CEnemy.cpp` | `TryRequestParryStaggerReaction` | helper gated | stagger reaction request result | parry threshold 도달 시 | `FCombatResultDebug` + `CombatResultAudit` 처리 완료 |
| `Source/Portfolio/Character/Player/CPlayer.cpp` | `ReceiveCombatResultPacket` | helper gated | combat result packet 수신/내용 | combat event마다 | `FCombatResultDebug` + `CombatResultAudit` 처리 완료 |
| `Source/Portfolio/Character/Player/CPlayer.cpp` | `HandleParryCombatResult` | helper gated | parry stack / stagger threshold | parry result마다 | `FCombatResultDebug` + `CombatResultAudit` 처리 완료 |
| `Source/Portfolio/Character/Player/CPlayer.cpp` | `TryRequestParryStaggerReaction` | helper gated | stagger reaction request result | parry threshold 도달 시 | `FCombatResultDebug` + `CombatResultAudit` 처리 완료 |

판단:

```text
이 로그들은 전투 결과 흐름을 따라가기에는 유용하지만 기본 출력으로 두기에는 runtime flow log 성격이 강하다.
특히 대량 전투 / 피드백 측정 / 충돌 측정 중 출력 로그 노이즈가 될 수 있으므로 `FCombatResultDebug` + `CombatResultAudit`으로 gate 처리했다.
```

권장 API 형태:

```cpp
FCombatResultDebug::RecordCombatResultReceivedForAudit(Receiver, Packet);
FCombatResultDebug::RecordParryStackUpdatedForAudit(Receiver, Packet, Count, Threshold, bStaggerReady);
FCombatResultDebug::RecordParryStaggerReactionRequestedForAudit(Receiver, Packet, Result);
FCombatResultDebug::RecordParryStaggerReactionRejectedForAudit(Receiver, Packet, Reason);
```

### 1-2. CombatResult Dispatch / CombatSignal Target

| 파일 | 위치 | 현재 상태 | 후보 이벤트 | 빈도 | 권장 처리 |
| --- | --- | --- | --- | --- | --- |
| `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp` | `DispatchCombatResultToReceiver` | helper gated | receiver 없음 | combat signal result마다 | `FCombatSignalDebug::RecordCombatResultDispatchForAudit(..., "NoReceiver")` 처리 완료 |
| `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp` | `DispatchCombatResultToReceiver` | helper gated | receiver interface 없음 | combat signal result마다 | `FCombatSignalDebug::RecordCombatResultDispatchForAudit(..., "MissingReceiverInterface")` 처리 완료 |
| `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp` | `DispatchCombatResultToReceiver` | helper gated | delivering | combat signal result마다 | `FCombatSignalDebug::RecordCombatResultDispatchForAudit(..., "Delivering")` 처리 완료 |
| `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp` | `DispatchCombatResultToReceiver` | helper gated | delivered | combat signal result마다 | `FCombatSignalDebug::RecordCombatResultDispatchForAudit(..., "Delivered")` 처리 완료 |

판단:

```text
dispatch 실패는 diagnostic 가치가 높지만 delivering/delivered는 기본 출력으로 두기에는 flow trace에 가깝다.
수신부 CombatResult 로그와 함께 켜고 끌 수 있어야 전투 결과 라우팅을 한 번에 추적할 수 있다.
```

처리 결과:

```text
FCombatSignalDebug로 분리했다.
Portfolio.Debug.CombatSignalAudit으로 dispatch 경계를 gate한다.
기존 active FLog::Log 4곳은 본문 helper 호출로 대체했다.
```

현재 프로젝트 흐름상 `CombatSignalTarget -> CombatResultReceiver` 경계이므로, dispatch는 CombatSignalAudit에 포함하고 CombatResult receive / parry stack / stagger request 쪽은 별도 CombatResultAudit으로 분리했다.

---

## 2. 유지하되 context 보강 후보

### 2-1. Action / Reaction Data Diagnostic

| 파일 | 위치 | 현재 상태 | 후보 이벤트 | 권장 처리 |
| --- | --- | --- | --- | --- |
| `Source/Portfolio/Component/CActionComponent.cpp` | `ResolveActionData`, `ResolveActionExecutor` | helper gated | ActionData/ActionExecutor resolve 실패 | `FActionComponentDebug` + `ActionComponentAudit` 처리 완료 |
| `Source/Portfolio/Component/CActionComponent.cpp` | `BuildActionDataMap`, `BuildActionExecutorMap` | helper gated | duplicate key overwrite / add 실패 | `FActionComponentDebug` + `ActionComponentAudit` 처리 완료 |
| `Source/Portfolio/Component/CReactionComponent.cpp` | `ResolveReactionData`, `ResolveReactionExecutor` | helper gated | ReactionData fallback miss / ReactionExecutor resolve 실패 | `FReactionComponentDebug` + `ReactionComponentAudit` 처리 완료 |
| `Source/Portfolio/Component/CReactionComponent.cpp` | `BuildReactionDataMap`, `BuildReactionExecutorMap` | helper gated | duplicate key overwrite / add 실패 | `FReactionComponentDebug` + `ReactionComponentAudit` 처리 완료 |

판단:

```text
asset/data 설정 오류를 찾는 저빈도 진단 로그다.
기본 출력에서 바로 제거하면 문제 원인 추적성이 떨어진다.
다만 최종 정책에서는 FActionReactionDataDebug 같은 non-shipping helper로 묶는 것이 좋다.
```

### 2-2. Action / Reaction Runtime Invariant

| 파일 | 위치 | 현재 상태 | 후보 이벤트 | 권장 처리 |
| --- | --- | --- | --- | --- |
| `Source/Portfolio/Action/CAction.cpp` | action executor runtime / montage lifecycle | helper gated | start/stop reject, montage play/bind failure, unexpected interruption, stale montage end | `FActionComponentDebug` + `ActionComponentAudit/Dump` 처리 완료 |
| `Source/Portfolio/Reaction/CReaction.cpp` | reaction executor runtime / montage lifecycle | helper gated | start/stop reject, montage play/bind failure, unexpected interruption, stale montage end | `FReactionComponentDebug` + `ReactionComponentAudit/Dump` 처리 완료 |
| `Source/Portfolio/Component/CActionOrchestratorComponent.cpp` | execution participant resolve | helper gated | action/reaction 동시 active | `FExecutionOrchestratorDebug::RecordInvalidActiveParticipantsForAudit` 처리 완료 |
| `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp` | execution participant resolve | helper gated | action/reaction 동시 active | `FExecutionOrchestratorDebug::RecordInvalidActiveParticipantsForAudit` 처리 완료 |

판단:

```text
정상 flow trace가 아니라 runtime invariant 위반 또는 예외적 흐름이다.
기본 제거 대상이 아니다.
```

### 2-3. Feedback Diagnostic

| 파일 | 위치 | 현재 상태 | 후보 이벤트 | 권장 처리 |
| --- | --- | --- | --- | --- |
| `Source/Portfolio/Component/CActionFeedbackComponent.cpp` | `ExecuteTrailFeedbacks` | helper gated | 최고 우선순위 trail feedback 중복 | `FCombatFeedbackDebug` + `FeedbackAudit` 처리 완료 |
| `Source/Portfolio/Component/CHitFeedbackComponent.cpp` | `PlayHitVFX` | helper gated | HitVFX invalid | `FCombatFeedbackDebug` + `FeedbackAudit` 처리 완료 |
| `Source/Portfolio/Component/CHitFeedbackComponent.cpp` | `PlayHitSFX` | helper gated | HitSFX invalid | `FCombatFeedbackDebug` + `FeedbackAudit` 처리 완료 |

판단:

```text
feedback presentation은 Runtime LOD에서 비용 측정을 마쳤지만, invalid asset / 중복 매칭은 데이터 설정 문제다.
기본 제거 대상이 아니라 진단 로그로 유지한다.
```

### 2-4. Notify / Movement / Overlay

| 파일 | 위치 | 현재 상태 | 후보 이벤트 | 권장 처리 |
| --- | --- | --- | --- | --- |
| `Source/Portfolio/Notify/CAnimNotify_ActionBase.cpp` | `IsValidateActionType` | active diagnostic | invalid action notify type | 유지. mesh owner/context 보강 후보 |
| `Source/Portfolio/Notify/CAnimNotifyState_ActionBase.cpp` | `IsValidateActionType` | active diagnostic | invalid action notify state type | 유지. mesh owner/context 보강 후보 |
| `Source/Portfolio/Notify/CAnimNotify_ReactionBase.cpp` | `IsValidateReactionType` | active diagnostic | invalid reaction notify type | 유지. mesh owner/context 보강 후보 |
| `Source/Portfolio/Notify/CAnimNotifyState_ReactionBase.cpp` | `IsValidateReactionType` | active diagnostic | invalid reaction notify state type | 유지. mesh owner/context 보강 후보 |
| `Source/Portfolio/Component/CMovementComponent.cpp` | `ChangeMovementGait` | helper gated | GaitSpeedMap missing | `FMovementDebug` + `MovementAudit` 처리 완료 |
| `Source/Portfolio/Component/CObservableOverlayComponent.cpp` | `ApplyOverlayHandlings` | active diagnostic | overlay handling apply 실패 | 유지. owner/context 보강 후보 |
| `Source/Portfolio/Component/CObservableOverlayComponent.cpp` | `ApplyOverlayHandling` | active diagnostic | policy 미수락 | 유지 후보. 정상 reject인지 추가 확인 필요 |
| `Source/Portfolio/Core/Debug/FComponentReferenceHelper.h` | `RecoverIfInvalid` | active diagnostic | invalid component reference recovery | 유지 후보. non-shipping helper 또는 CVar gate 후보 |

판단:

```text
이 영역은 대부분 asset/configuration 오류 또는 시스템 경계 실패다.
다만 Overlay policy 미수락과 ComponentReferenceRecovery는 정상 복구/정책 흐름일 수도 있으므로 다음 단계에서 빈도와 의미를 확인한 뒤 유지/게이트 여부를 결정한다.
```

---

## 3. 이미 CVar audit 구조로 정리된 로그

| 파일 | CVar | 상태 | 판단 |
| --- | --- | --- | --- |
| `Source/Portfolio/Controller/CAIController.cpp` | `Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit` | gated | 유지 |
| `Source/Portfolio/Controller/CAIController.cpp` | `Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit` | gated | 유지 |
| `Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp` | `Portfolio.AI.RuntimeLOD.EngageAssignmentAudit` | gated + Shipping no-op | 유지 / print helper 내부 gate 보완 완료 |
| `Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp` | `Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit` | gated + Shipping no-op | 유지 / print helper 내부 gate 보완 완료 |
| `Source/Portfolio/AI/BehaviorTree/Decorator/CBTDecorator_CanMove.cpp` | `Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit` | gated | 유지 |

판단:

```text
이 로그들은 이미 측정 목적의 명시적 CVar 뒤에 있다.
기본 출력 노이즈가 아니므로 이번 정리의 제거 대상이 아니다.
```

---

## 4. CSV profiling counter / timing stat

| 영역 | 대표 파일 | 상태 | 판단 |
| --- | --- | --- | --- |
| Animation refresh counter | `Source/Portfolio/Character/CAnimInstance.cpp` | CSV counter | 유지 |
| Combat feedback profiling | `Source/Portfolio/Core/Profiling/CCombatFeedbackProfiling.cpp` | CSV counter | 유지 |
| Combat collision profiling | `Source/Portfolio/Core/Profiling/CCombatCollisionProfilingCounters.cpp` | CSV counter | 유지 |
| CombatEngage timing | `Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp` | CSV timing stat | 유지 |
| BT service timing/count | `Source/Portfolio/AI/BehaviorTree/Service/*` | CSV timing/counter | 유지 |
| State Runtime LOD tier count | `Source/Portfolio/AI/RuntimeLOD/CAIStateRuntimeLODPolicy.cpp` | CSV counter | 유지 |

판단:

```text
CSV stat은 출력 로그가 아니라 성능 측정 데이터다.
Shipping 포함 여부는 별도 빌드 정책에서 다룰 수 있지만, debug log cleanup의 제거 대상은 아니다.
```

---

## 5. 로그는 없지만 관측 후보인 경계

### 5-1. CombatSignal Source / Target reject

| 파일 | 위치 | 후보 이벤트 | 현재 상태 | 권장 처리 |
| --- | --- | --- | --- | --- |
| `Source/Portfolio/Component/CCombatSignalSourceComponent.cpp` | `ValidateRequest`, `CanSendCombatSignal`, `ResolveDamageSpec`, `ComputeDamage`, `CommitSignal`, cue send | invalid hit context, invalid actor/component ownership, self/friendly target, duplicate hit, spec missing, compute failed, commit failed, cue target/component missing | helper gated | `FCombatSignalDebug::RecordSource...ForAudit`, `RecordCue...ForAudit` 처리 완료 |
| `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp` | `ValidateRequest`, `CanReceiveCombatSignal`, `ComputeTargetDamage`, rejected/accepted packet, timing cue | invalid damage event, invalid target/causer/instigator, already dead, zero damage, guard/parry, unknown cue | helper gated | `FCombatSignalDebug::RecordTarget...ForAudit`, `RecordTimingCue...ForAudit` 처리 완료 |

판단:

```text
reject reason이 이미 구조화되어 있으므로 본문에 FLog를 직접 추가할 필요는 낮다.
필요하면 result를 반환/dispatch하는 경계에서 한 번만 CVar audit으로 출력한다.
```

### 5-2. Action / Reaction Orchestrator reject

| 파일 | 위치 | 후보 이벤트 | 현재 상태 | 권장 처리 |
| --- | --- | --- | --- | --- |
| `Source/Portfolio/Component/CActionOrchestratorComponent.cpp` | `RequestMovementAction`, `RequestEquipmentAction`, `RequestCombatAction` | accepted / ignored / rejected / deferred | helper gated | `FExecutionOrchestratorDebug::RecordActionRequestResultForAudit` 처리 완료 |
| `Source/Portfolio/Component/CActionOrchestratorComponent.cpp` | `BuildDecisionResult`, `ResolveExecutionApplyMode`, `ResolveInterventionDirective`, `ResolveObservableOverlayGate` | reject reason 세분화 | helper gated | `FExecutionOrchestratorDebug::RecordActionExecutionResultForAudit` 처리 완료. overlay reject는 `RejectedByOverlay` 보존 |
| `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp` | `RequestDamageReaction`, `RequestCombatResultReaction` | accepted / ignored / rejected | helper gated | `FExecutionOrchestratorDebug::RecordReactionRequestResultForAudit` 처리 완료 |
| `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp` | `BuildDecisionResult`, `ResolveExecutionApplyMode`, `ResolveInterventionDirective`, `ResolveObservableOverlayGate` | reject reason 세분화 | helper gated | `FExecutionOrchestratorDebug::RecordReactionExecutionResultForAudit` 처리 완료. overlay reject는 `RejectedByOverlay` 보존 |

판단:

```text
이 경로는 정상 정책 reject가 많을 수 있다.
따라서 기본 출력은 금지하고, 반복 분석이 필요해질 때 request/result 요약을 CVar audit이나 CSV counter로 추가한다.
```

---

## 6. 권장 CVar 카테고리

처음부터 너무 많은 CVar를 만들지 않는다.

1차 구현 권장 단위:

| CVar | 대상 | 우선순위 |
| --- | --- | --- |
| `Portfolio.Debug.CombatSignalAudit` | CombatSignal source/target reject reason, weapon raw overlap, accepted/rejected result, timing cue, dispatch 요약 | 완료 |
| `Portfolio.Debug.CombatSignalDump` | CombatSignal source context / target packet 상세 dump | 완료 |
| `Portfolio.Debug.ActionRequestAudit` | ActionOrchestrator request result / reject reason | 처리 완료 |
| `Portfolio.Debug.ReactionRequestAudit` | ReactionOrchestrator request result / reject reason | 처리 완료 |
| `Portfolio.Debug.ExecutionOrchestratorDump` | Action/Reaction orchestration query/result dump | 처리 완료 |
| `Portfolio.Debug.ActionComponentAudit` | ActionComponent data/executor/notify/runtime reject | 처리 완료 |
| `Portfolio.Debug.ActionComponentDump` | ActionComponent execution context dump | 처리 완료 |
| `Portfolio.Debug.ReactionComponentAudit` | ReactionComponent data/executor/notify/runtime reject | 처리 완료 |
| `Portfolio.Debug.ReactionComponentDump` | ReactionComponent execution context dump | 처리 완료 |
| `Portfolio.Debug.CombatResultAudit` | CombatResult receive / parry stack / stagger request 경계 | 처리 완료 |
| `Portfolio.Debug.FeedbackAudit` | feedback request/match/invalid data 상세 | 처리 완료 |

판단:

```text
CombatSignalAudit / CombatSignalDump는 source/target/weapon raw overlap 1차 적용 완료 상태다.
다음 탐색은 Weapon raw overlap 또는 Action/Reaction data diagnostic 후보를 검토한다.
CombatResult receive 로그는 dispatch가 CombatSignalAudit에 포함되었으므로, receiver-side 결과 분석용 별도 CombatResultAudit으로 분리했다.
```

---

## 7. 1단계 결론

처리 완료 항목:

```text
1. CCombatSignalSourceComponent source/cue diagnostic hook 추가
2. CCombatSignalTargetComponent target/timing cue diagnostic hook 추가
3. CCombatSignalTargetComponent CombatResultDispatch active FLog 4곳 helper 이동
4. FCombatSignalDebug helper 추가
5. Portfolio.Debug.CombatSignalAudit / Portfolio.Debug.CombatSignalDump 추가
6. 본문 gameplay code에는 문자열 포맷을 두지 않고 helper 호출만 남김
```

추가 검토 항목:

```text
1. Overlay / ComponentReferenceRecovery gate 여부
2. Notify invalid 로그 context 보강
3. Action / Reaction data diagnostic 유지 범위 재확인
4. 전체 잔여 FLog::Log 재스캔
```

현재 단계에서 제거하지 않을 항목:

```text
1. Action / Reaction data diagnostic
2. Unexpected montage interruption
3. invalid notify trigger
4. invalid feedback asset
5. CVar 기반 AI / Engage audit
6. CSV profiling counter
```

---

## 8. 컨텍스트 없는 리뷰 기준 우선 탐색 순서

기존 대화 맥락을 모르는 별도 코드 리뷰 관점에서는 `CombatResult` 수신부보다 한 단계 앞의 전투 판정 파이프라인을 더 높은 우선순위로 보았다.

판단 이유:

```text
CombatResult 로그는 이미 결과가 생성된 뒤의 흐름을 보여준다.
하지만 실제 runtime 문제는 그 이전 단계인 hit overlap, CombatSignal source/target validation,
defense outcome, reaction dispatch 경계에서 조용히 drop될 가능성이 더 높다.
```

CombatSignal source/target 경계를 먼저 확정한 뒤, receiver-side CombatResult 경계는 `FCombatResultDebug` + `CombatResultAudit`으로 분리했다.

|  순서 | 파일                                                                                                                       | 디버깅 가치                                                             | 관측 후보                                                                                                                                              | 권장 로그 형태                                                   |
| --: | ------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------- |
|   1 | `Source/Portfolio/Component/CCombatSignalSourceComponent.cpp`                                                            | 공격자 쪽 hit pipeline이 여러 gate에서 조용히 drop될 수 있음                       | invalid hit context, self/friendly target, duplicate hit window, missing damage spec, compute/commit failure, cue target resolution                | CVar audit + CSV counter                                   |
|   2 | `Source/Portfolio/Component/CCombatSignalTargetComponent.cpp`                                                            | target-side damage / defense / reaction / CombatResult dispatch 경계 | invalid damage event, already dead, parry/guard/zero damage, committed HP delta, reaction request result, rejected result packet, dispatch failure | CVar audit. 불가능한 dispatch failure는 default diagnostic 후보   |
|   3 | `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`                                                            | action request accept/reject/defer/intervention 중앙 판단              | request source/type, reject reason, resolved data/executor, active participant, execution decision, overlay rejection, intervention failure        | CVar audit                                                 |
|   4 | `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp`                                                          | damage/combat result가 reaction으로 이어지는 중앙 판단                        | reaction type resolution, missing reaction data/executor, active rejection, dead-force intervention, overlay rejection, dispatch failure           | CVar audit                                                 |
|   5 | `Source/Portfolio/Weapon/CWeaponActor.cpp`                                                                               | raw collision window / overlap 시작점                                 | collision window open/close, overlap ignored reason, named collision missing, hit context dump                                                     | `FCombatSignalDebug` + `CombatSignalAudit/Dump` 처리 완료  |
|   6 | `Source/Portfolio/Component/CActionComponent.cpp`                                                                        | action data map, notify routing, executor 상태                       | missing action data, executor add failure, notify ignored, collision/cue notify routing                                                            | `FActionComponentDebug` + `ActionComponentAudit/Dump` 처리 완료 |
|   7 | `Source/Portfolio/Action/CAction.cpp`                                                                                    | montage lifecycle / chain / intervention window                    | start/stop reject, montage play/bind failure, unexpected interruption, stale montage end                                                            | `FActionComponentDebug` + `ActionComponentAudit/Dump` 처리 완료 |
|   8 | `Source/Portfolio/Component/CReactionComponent.cpp`                                                                      | reaction data fallback / active reaction state                     | spec-key fallback miss, executor failure, notify ignored, decision/runtime reject                                                                   | `FReactionComponentDebug` + `ReactionComponentAudit/Dump` 처리 완료 |
|   9 | `Source/Portfolio/Reaction/CReaction.cpp`                                                                                | reaction montage lifecycle                                         | start/stop reject, montage play/bind failure, unexpected interruption, stale montage end                                                            | `FReactionComponentDebug` + `ReactionComponentAudit/Dump` 처리 완료 |
|  10 | `Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.cpp`                                                        | AI combat role allocator                                           | request snapshot, warmup, lease, cap, preserve/promote/fresh assignment                                                                            | 기존 CVar audit 유지 / Shipping no-op + helper 내부 gate 보완 완료 |
|  11 | `Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.cpp`                                                | AI blackboard context builder                                      | target changed/cleared, engage request submitted, assignment missing                                                                               | `FAICombatBTDebug` + `AICombatBTAudit` 처리 완료             |
|  12 | `Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.cpp`                                            | `bCanCombatAction` 계산 경계                                           | range, cooldown, active action/reaction, target missing                                                                                            | `FAICombatBTDebug` + `AICombatBTAudit` 처리 완료             |
|  13 | `Source/Portfolio/AI/BehaviorTree/Task/CBTTask_StartCombatAction.cpp`                                                    | BT에서 combat action으로 들어가는 경계                                       | blackboard/controller/pawn invalid, action request result, cooldown set                                                                            | `FAICombatBTDebug` + `AICombatBTAudit` 처리 완료             |
|  14 | `Source/Portfolio/Component/CMovementComponent.cpp`                                                                      | 최종 movement gate / Runtime LOD movement suppression                | movement intent reject, RuntimeLOD mode change, missing gait speed                                                                                 | `FMovementDebug` + `MovementAudit` 처리 완료                 |
|  15 | `Source/Portfolio/Component/CActionFeedbackComponent.cpp`, `CReactionFeedbackComponent.cpp`, `CHitFeedbackComponent.cpp`, `CWorldSubsystem_CombatFeedback.cpp` | presentation 문제를 combat bug로 오인할 수 있음                              | no matching feedback, duplicate match, invalid VFX/SFX/camera shake, profiling skip                                                                | `FCombatFeedbackDebug` + `FeedbackAudit` 처리 완료 / CSV counter 유지 |

### 우선순위 조정

기존 1단계 결론은 `CombatResult` active log를 가장 먼저 분리하는 방향이었다.

컨텍스트 없는 리뷰 결과를 반영하면 다음 순서가 더 적합하다.

```text
1. CCombatSignalSourceComponent / CCombatSignalTargetComponent를 한 쌍으로 먼저 스캔한다.
2. CombatSignalAudit 범위와 출력 단위를 확정한다.
3. 그 결과를 바탕으로 dispatch 로그는 CombatSignalAudit에 포함하고,
   CombatResult receive / parry stack / stagger request 로그는 별도 CombatResultAudit으로 분리했다.
4. 이후 ActionOrchestrator / ReactionOrchestrator의 request result audit을 검토한다.
```

### 다음 작업 결정

다음 작업은 CombatSignal Source / Target 이후 우선순위에 따라 다음 파일 스캔이다.

```text
1. CActionOrchestratorComponent.cpp
2. CReactionOrchestratorComponent.cpp
```

목표는 다음을 확정하는 것이다.

```text
1. action/reaction request reject가 정상 정책 reject인지 비정상 diagnostic인지
2. accept/reject/defer/intervention 중 어떤 경계만 CVar audit으로 출력할지
3. 기존 active diagnostic log가 Error / Warning / Diagnostic Hook 중 어디에 속하는지
4. CSV counter가 필요한 event volume 지점이 어디인지
5. CombatResult receive 로그 분리 완료 후 전체 잔여 로그를 재스캔할지
```
