# N24. Debug Log Cleanup Inventory Note

## 목적

이 문서는 `refactor/debug-log-policy-v1` 브랜치에서 진행한 debug log / diagnostic code 정리 항목을 한 곳에 모은다.

정리 기준은 `N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md`를 따른다.

책임 범위:

```text
이 문서는 cleanup 이력 / inventory 문서다.
제거됨, 보완/유지, 정리 후보, 처리 커밋 또는 pending 상태를 기록한다.
Debug helper 설계 상세와 CVar 카테고리 설계는 N25에서 관리한다.
전수조사 우선순위와 다음 작업 판단은 N26에서 관리한다.
```

핵심 분류는 다음과 같다.

```text
1. Temporary trace: 제거
2. Dead debug dump: 제거
3. Diagnostic log: 유지하되 메시지 보강
4. Runtime flow log: 기본 출력 제거 또는 audit 후보로 분리
5. Profiling / audit counter: 유지
```

각 섹션은 상태 판단이 쉽도록 다음 기준으로 나눈다.

```text
제거됨: 임시 trace, 주석 처리된 호출, 호출처 없는 Print... dump, dead helper
보완/유지: 실제 실패 원인을 식별하는 diagnostic log, CVar 기반 audit/profiling hook
정리 후보: 아직 코드에 남아 있으며 다음 cleanup 대상인 항목
```

---

## 1. AI BehaviorTree Task / Service

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CBTTask_AdvanceInvestigateIndex.cpp` | `[Index Done]` 임시 trace 로그 | 제거 | `c24f92d1` |
| `CBTService_UpdateInvestigateContext.cpp` | `[Investigate Time out]` 임시 trace 로그 | 제거 | `c24f92d1` |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| Blackboard 갱신 / investigate index 처리 | 기능 상태 | 유지 | `c24f92d1` |
| EndInvestigate 플래그 흐름 | 기능 상태 | 유지 | `c24f92d1` |

### 판단

두 로그 모두 상태 전환 결과를 짧게 확인하기 위한 임시 trace였다. 기능 흐름은 유지하고 로그만 제거했다.

---

## 2. Action / Reaction Runtime Executor

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CReaction.cpp` | 주석 처리된 `PrintExecutionParticipant(...)` | 제거 | `ff5b1a58` |
| `CReaction.cpp` | 주석 처리된 `PrintExecutionInterventionParticipantFilter(...)` | 제거 | `ff5b1a58` |
| `CReaction.cpp/.h` | 호출처 없는 `PrintExecutionParticipant(...)` | 선언/정의 제거 | `ff5b1a58` |
| `CReaction.cpp/.h` | 호출처 없는 `PrintExecutionInterventionParticipantFilter(...)` | 선언/정의 제거 | `ff5b1a58` |
| `CReaction.cpp/.h` | 호출처 없는 `PrintStopReasonInfo(...)` | 선언/정의 제거 | `ff5b1a58` |
| `CReaction.cpp/.h` | 호출처 없는 `PrintIgnoredStopReasonInfo()` | 선언/정의 제거 | `ff5b1a58` |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CAction.cpp` | `[Action] Unexpected montage interruption.` | 유지하되 `Action`, `Montage`, `Serial` 포함하도록 메시지 보강 | `ff5b1a58` |
| `CReaction.cpp` | `[Reaction] Unexpected montage interruption.` | 유지하되 `Reaction`, `Montage`, `Serial` 포함하도록 메시지 보강 | `ff5b1a58` |

### 판단

`Unexpected montage interruption`은 정상 흐름 로그가 아니라 비정상 runtime 상태를 알려주는 diagnostic log다. 따라서 제거하지 않고 원인 분석에 필요한 context를 추가했다.

반면 `Stopped`, `Ignored`, intervention participant dump는 호출처가 없거나 주석 처리된 debug dump였으므로 제거했다.

---

## 3. Action / Reaction Component Data Diagnostic

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 이 섹션은 실패 진단 로그 보강이 목적 | - |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CActionComponent.cpp` | `// [Debug] ActionData is Valid; but Find and Add Failed` 주석 | 실제 실패 로그로 승격 | `65c716b9` |
| `CReactionComponent.cpp` | `// [Debug] ReactionData is Valid; but Find and Add Failed` 주석 | 실제 실패 로그로 승격 | `65c716b9` |
| `CActionComponent.cpp` | `[Duplicate key] Overwrite Value` | `ActionType`, `ActionIndex`, `Owner` 포함하도록 메시지 보강 | `65c716b9` |
| `CReactionComponent.cpp` | `[Duplicate key] Overwrite Value` | `ReactionType`, `DamageSpecKey`, `Owner` 포함하도록 메시지 보강 | `65c716b9` |
| `CActionComponent.cpp` | `BuildActionExecutorMap` 실패 로그 | `ActionDataKey`, `ExecutorKey`, `Owner` 포함하도록 메시지 보강 | `65c716b9` |
| `CReactionComponent.cpp` | `BuildReactionExecutorMap` 실패 로그 | `ReactionDataKey`, `DamageSpecKey`, `ExecutorKey`, `Owner` 포함하도록 메시지 보강 | `65c716b9` |

### 판단

이 영역은 단순 debug trace가 아니라 asset / data configuration 문제를 식별하는 diagnostic log다. 제거하지 않고 실패 원인을 찾을 수 있는 context를 보강했다.

---

## 4. Action / Reaction Feedback Component

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CActionFeedbackComponent.cpp` | 주석 처리된 request / VFX / SFX / trail print 호출 | 제거 | `65c716b9` |
| `CActionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintActionFeedbackRequestInfo` | 선언/정의 제거 | `65c716b9` |
| `CActionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintActionVFXInfo` | 선언/정의 제거 | `65c716b9` |
| `CActionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintActionSFXInfo` | 선언/정의 제거 | `65c716b9` |
| `CActionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintTrailInfo` | 선언/정의 제거 | `65c716b9` |
| `CActionFeedbackComponent.cpp` | `Duplicate VFX/SFX execution key skipped` | 정상 방어 흐름이라 기본 로그 제거 | `65c716b9` |
| `CReactionFeedbackComponent.cpp` | 주석 처리된 request / VFX / SFX print 호출 | 제거 | `65c716b9` |
| `CReactionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintReactionFeedbackRequestInfo` | 선언/정의 제거 | `65c716b9` |
| `CReactionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintReactionVFXInfo` | 선언/정의 제거 | `65c716b9` |
| `CReactionFeedbackComponent.cpp/.h` | 호출처 없는 `PrintReactionSFXInfo` | 선언/정의 제거 | `65c716b9` |
| `CReactionFeedbackComponent.cpp` | `Duplicate VFX/SFX execution key skipped` | 정상 방어 흐름이라 기본 로그 제거 | `65c716b9` |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CActionFeedbackComponent.cpp` | `Duplicate highest-priority trail feedback matches` | 유지하되 Action key, Timing, TriggerKey, Owner 포함하도록 메시지 보강 | `65c716b9` |

### 판단

Feedback execution key 중복 skip은 오류가 아니라 중복 재생 방지 정책이다. 반복 runtime flow로 출력될 가능성이 있으므로 기본 로그에서 제거했다.

Trail feedback 최고 우선순위 중복은 데이터 설계 충돌일 수 있어 유지하고 context를 보강했다.

---

## 5. Component Debug Dump Removal

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CReactionComponent.cpp/.h` | `PrintReactionInfoSummary` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintReactionDataMap` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintComponentStateInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintDamageSpecKeyInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintReactionDataKeyInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintReactionDataInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintReactionExcutorInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReactionComponent.cpp/.h` | `PrintReactionExecutorRuntimeInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CReaction.cpp/.h` | `PrintReactionExecutorRuntimeInfo_Public` | component dump 제거 후 미사용이라 제거 | `b28f6ae9` |
| `CReaction.cpp/.h` | `PrintReactionExecutorRuntimeInfo` | component dump 제거 후 미사용이라 제거 | `b28f6ae9` |
| `CHitFeedbackComponent.cpp/.h` | `PrintHitStopRequestInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CHitFeedbackComponent.cpp/.h` | `PrintHitVFXRequestInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CHitFeedbackComponent.cpp/.h` | `PrintHitSFXRequestInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CHitFeedbackComponent.cpp/.h` | `PrintCameraShakeRequestInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CHitFeedbackComponent.cpp/.h` | `PrintHitInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CPlayerFeedbackComponent.cpp/.h` | `PrintCameraShakeConsumeInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |
| `CStateComponent.cpp/.h` | `PrintExecutionStateChangedInfo` | 호출처 없는 debug dump 제거 | `b28f6ae9` |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 이 섹션은 dead dump 제거가 목적 | - |

### 판단

이 영역은 실제 runtime diagnostic log가 아니라 호출처가 주석뿐이거나 전혀 없는 debug dump 함수들이다. 필요하면 기존 dump를 되살리기보다 정책에 맞는 audit/debug hook으로 새로 설계한다.

---

## 6. Combat Signal Source / Target

### 제거됨

| 대상 | 기존 상태 | 판단 | 처리 |
| --- | --- | --- | --- |
| `CCombatSignalSourceComponent.cpp` `NotifyHitWindowOpened` trace | 주석 처리된 `FLog::Log` | 단순 flow trace | 제거 |
| `CCombatSignalSourceComponent.cpp` `NotifyHitWindowClosed` trace | 주석 처리된 `FLog::Log` | 단순 flow trace | 제거 |
| `CCombatSignalSourceComponent.cpp` rejected summary 호출 | 주석 처리된 `PrintCombatSignalSourceRejectedSummaryInfo(...)` | 죽은 debug hook | 주석 호출 제거 |
| `CCombatSignalSourceComponent.cpp` success summary 호출 | 주석 처리된 `PrintCombatSignalSourceSummaryInfo(...)` | 죽은 debug hook | 주석 호출 제거 |
| `CCombatSignalSourceComponent.cpp/.h` source summary/context/rejected dump 묶음 | 호출처 없음 | 현재 죽은 dump 코드 | 제거. 필요하면 `CombatSignalAudit`로 재설계 |
| `CCombatSignalTargetComponent.cpp` rejected summary 호출 | 주석 처리된 `PrintCombatSignalTargetSummaryInfo(...)` | 죽은 debug hook | 주석 호출 제거 |
| `CCombatSignalTargetComponent.cpp` success/outcome 호출 | 주석 처리된 summary/outcome dump | 죽은 debug hook | 주석 호출 제거 |
| `CCombatSignalTargetComponent.cpp` timing cue trace | 주석 처리된 Blink / Repulse / Rejected 로그 | 단순 flow trace | 제거 |
| `CCombatSignalTargetComponent.cpp/.h` target summary/context/outcome dump 묶음 | 호출처 없음 | 현재 죽은 dump 코드 | 제거. 필요하면 `CombatSignalAudit`로 재설계 |

### 보완/유지

| 대상 | 기존 상태 | 판단 | 처리 |
| --- | --- | --- | --- |
| `CCombatSignalTargetComponent.cpp` `DispatchCombatResultToReceiver` 실제 runtime `FLog::Log` 4곳 | active log | target signal / combat result dispatch diagnostic 후보 | `FCombatSignalDebug::RecordCombatResultDispatchForAudit(...)`로 이동. 기본 출력 제거, `Portfolio.Debug.CombatSignalAudit`로 gate |
| `CCombatSignalSourceComponent.cpp` source reject / accept 경계 | no active log | hit pipeline drop 관측 후보 | `FCombatSignalDebug::RecordSource...ForAudit(...)` hook 추가. 기본 출력 Off |
| `CCombatSignalSourceComponent.cpp` source context dump | no active dump | source context 상세 확인 후보 | `FCombatSignalDebug::PrintSourceContextDebug(...)` hook 추가. `Portfolio.Debug.CombatSignalDump`로 gate |
| `CCombatSignalTargetComponent.cpp` target reject / accept 경계 | 일부 rejected dispatch 비어 있음 | target damage / defense / result 관측 후보 | `FCombatSignalDebug::RecordTarget...ForAudit(...)` hook 추가. 기본 출력 Off |
| `CCombatSignalTargetComponent.cpp` target packet dump | no active dump | target packet 상세 확인 후보 | `FCombatSignalDebug::PrintTargetPacketDebug(...)` hook 추가. `Portfolio.Debug.CombatSignalDump`로 gate |

### 판단

CombatSignal은 전투 파이프라인 분석 가치가 높다. 다만 현재 남은 `PrintCombatSignal...` 함수들은 호출되지 않는 debug dump다.

따라서 죽은 코드는 제거하고, 실제 관측 지점은 `FCombatSignalDebug` helper로 재구성했다.

현재 처리 상태:

```text
1. Diagnostic Hook: Portfolio.Debug.CombatSignalAudit
2. Debug Dump: Portfolio.Debug.CombatSignalDump
3. 본문 직접 FLog / FString::Printf 제거
4. Shipping 빌드에서는 helper 내부 gate가 no-op 처리
```

---

## 7. Weapon / Collision Debug Dump

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CWeaponActor.cpp` `PrintTrailInfo(...)` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CWeaponActor.cpp` `PrintBeginOverlapContextInfo(...)` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CWeaponActor.cpp/.h` `PrintBeginOverlapContextInfo(...)` | 호출처 없음 | 선언/정의 제거 | pending |
| `CWeaponActor.cpp/.h` `PrintEndOverlapContextInfo(...)` | 호출처 없음 | 선언/정의 제거 | pending |
| `CWeaponActor.cpp/.h` `PrintOverlapContextInfo(...)` | dump helper만 남음 | 선언/정의 제거 | pending |
| `CWeaponActor.cpp/.h` `PrintHitContextInfo(...)` | dump helper만 남음 | 선언/정의 제거 | pending |
| `CWeaponActor.cpp/.h` `PrintTrailInfo(...)` | 호출처 없음 | 선언/정의 제거 | pending |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 아직 미적용 | - |

### 판단

Weapon overlap / hit context는 전투 충돌 분석 가치가 있다. 다만 기존 구현은 호출되지 않는 dump 함수였으므로 제거했다.

필요하면 `CombatCollisionProfilingCounters` 또는 별도 `WeaponCollisionAudit`로 다시 설계한다.

---

## 8. Health / Defense Debug Dump

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CHealthComponent.cpp` `PrintTakeDamageContextInfo()` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CHealthComponent.cpp` `PrintTakeHealContextInfo()` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CHealthComponent.cpp/.h` `PrintTakeDamageContextInfo()` | 호출처 없음 | 선언/정의 제거 | pending |
| `CHealthComponent.cpp/.h` `PrintTakeHealContextInfo()` | 호출처 없음 | 선언/정의 제거 | pending |
| `CHealthComponent.cpp/.h` `PrintHealthContextInfo(...)` | dump helper만 남음 | 선언/정의 제거 | pending |
| `CHealthComponent.cpp/.h` `PrintDeadContextInfo(...)` | dump helper만 남음 | 선언/정의 제거 | pending |
| `CDefenseComponent.cpp/.h` `PrintGuardStateInfo()` | 호출처 없음 | 선언/정의 제거 | pending |

### 보완/유지

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 아직 미적용 | - |

### 판단

Health / Defense 상태 변화는 gameplay diagnostic 가치가 있다. 다만 기존 `Print...ContextInfo` 계열은 호출되지 않는 dump 함수였으므로 제거했다.

실제 문제가 필요하면 damage / guard state 변경 지점에서 조건부 diagnostic log로 다시 설계한다.

---

## 9. AI Controller / BT Service Debug Dump

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| `CAIController.cpp` `PrintTargetPerceptionUpdatedSummary(...)` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CAIController.cpp` `PrintAllTargetData()` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CAIController.cpp/.h` `PrintPerceptionUpdatedSummary(...)` | 호출처 없음 | 선언/정의 제거 | pending |
| `CAIController.cpp/.h` `PrintTargetPerceptionUpdatedSummary(...)` | 호출처 없음 | 선언/정의 제거 | pending |
| `CAIController.cpp/.h` `PrintTargetPerceptionForgotten(...)` | 호출처 없음 | 선언/정의 제거 | pending |
| `CAIController.cpp/.h` `PrintAllTargetData()` | 호출처 없음 | 선언/정의 제거 | pending |
| `CAIController.cpp/.h` `PrintTargetData(...)` | dump helper만 남음 | 선언/정의 제거 | pending |
| `CBTService_UpdateEngageContext.cpp` `PrintEngageContext(...)` 호출 | 주석 처리된 호출 | 제거 | pending |
| `CBTService_UpdateEngageContext.cpp/.h` `PrintEngageContext(...)` | 호출처 없음 | 선언/정의 제거 | pending |

### 보완/유지

| 대상 | 기존 상태 | 판단 | 권장 처리 |
| --- | --- | --- | --- |
| `PerceptionCandidateAudit` | CVar 기반 audit | 대량 perception 후보 검증에 사용 | 유지 |
| `BlackboardEngageLatencyAudit` | CVar 기반 audit | engage latency 분석에 사용 | 유지 |

### 판단

CVar 기반 audit로 승격된 관측은 유지한다. 수동으로 주석을 풀어야 동작하던 `Print...` dump는 기본 코드에 남길 이유가 약하므로 제거했다.

---

## 10. Combat Engage / Feedback Audit Logs

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 아직 미적용 | - |

### 보완/유지

| 대상 | 기존 상태 | 판단 | 권장 처리 |
| --- | --- | --- | --- |
| `CWorldSubsystem_CombatEngage.cpp/.h` `PrintAssignmentWarmupDelay(...)` | audit CVar 경로에서 호출 | assignment bootstrap 분석용 audit | 유지 |
| `CWorldSubsystem_CombatEngage.cpp/.h` `PrintEngageRequestSnapshot(...)` | verbose audit CVar 경로에서 호출 | request bucket 분석용 audit | 유지 |
| `CWorldSubsystem_CombatEngage.cpp/.h` `PrintEngageAssignmentRebuildSummary(...)` | audit CVar 경로에서 호출 | assignment 결과 summary | 유지 |

### 정리 후보

| 대상 | 기존 상태 | 판단 | 권장 처리 |
| --- | --- | --- | --- |
| `CWorldSubsystem_CombatEngage.cpp/.h` `PrintAppliedFreshEngageAssignment(...)` | 호출부 주석 처리 | verbose assignment dump | verbose audit CVar로 통합하거나 제거 |
| `CWorldSubsystem_CombatEngage.cpp/.h` `PrintPromotedEngageAssignment(...)` | 호출부 주석 처리 | verbose assignment dump | verbose audit CVar로 통합하거나 제거 |
| `CWorldSubsystem_CombatEngage.cpp/.h` `PrintPreservedAssignment(...)` | 호출부 주석 처리 | verbose assignment dump | verbose audit CVar로 통합하거나 제거 |
| `CWorldSubsystem_CombatFeedback.cpp` hit stop trace 3곳 | 주석 처리된 `FLog::Log` | 단순 flow trace | 제거 |
| `CWorldSubsystem_CombatFeedback.cpp/.h` `PrintHitStopConsumeInfo(...)` | 호출부 주석 처리 | hit stop dump hook | 선언/정의 제거 후보 |

### 판단

CombatEngage의 summary / snapshot 계열은 실제 CVar audit 경로에 연결되어 있고 측정 과정에서 사용했다.

반면 fresh / promoted / preserved 상세 dump는 호출부가 다시 주석 처리되어 있어, 남길 거라면 `EngageAssignmentVerboseAudit`에 명확히 연결해야 한다.

---

## 11. Notify / Combo / Character Trace

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 아직 미적용 | - |

### 보완/유지

| 대상 | 기존 상태 | 판단 | 권장 처리 |
| --- | --- | --- | --- |
| `CEnemy.cpp`, `CPlayer.cpp` possession / lifecycle runtime 로그 | active `FLog::Log` | runtime flow log 가능성 | 유지 필요성 재검토 |

### 정리 후보

| 대상 | 기존 상태 | 판단 | 권장 처리 |
| --- | --- | --- | --- |
| `CAnimNotify_CombatSignalCue.cpp` invalid cue trace | 주석 처리된 `FLog::Log` | 단순 notify trace | 제거 |
| `CAnimNotify_CombatSignalCue.cpp` accepted cue trace | 주석 처리된 `FLog::Log` | 단순 notify trace | 제거 |
| `CAction_ComboAttack.cpp` chain input trace | 주석 처리된 `FLog::Log` | 단순 combo flow trace | 제거 |
| `CAction_ComboAttack.cpp` failed consume trace | 주석 처리된 `FLog::Log` | combo flow trace 또는 diagnostic 후보 | 제거 또는 diagnostic 승격 판단 |
| `CAction_ComboAttack.cpp` consumed chain trace | 주석 처리된 `FLog::Log` | 단순 combo flow trace | 제거 |
| `CEnemy.cpp` initial equip-action rejected trace | 주석 처리된 `FLog::Log` | startup flow trace | 제거 또는 실패 diagnostic 승격 판단 |

### 판단

Notify / combo trace는 대부분 수동 확인용 임시 로그다. 실패 diagnostic으로 남길 항목은 Owner, ActionType, ActionIndex, Montage 등 원인 추적 context를 포함해야 한다.

---

## 12. Remaining Diagnostic Log Candidates

### 제거됨

| 대상 | 기존 상태 | 처리 | 커밋 |
| --- | --- | --- | --- |
| 없음 | - | 아직 미적용 | - |

### 보완/유지 후보

| 대상 | 기존 상태 | 판단 | 권장 처리 |
| --- | --- | --- | --- |
| `CHitFeedbackComponent.cpp` `Invalid HitVFX` | active log | asset configuration diagnostic | Owner, HitFeedback key, asset context 보강 검토 |
| `CHitFeedbackComponent.cpp` `Invalid HitSFX` | active log | asset configuration diagnostic | Owner, HitFeedback key, asset context 보강 검토 |
| `CActionOrchestratorComponent.cpp` invalid execution state | active log | action/reaction 동시 active diagnostic | Owner, Action state, Reaction state 보강 검토 |
| `CReactionOrchestratorComponent.cpp` invalid execution state | active log | action/reaction 동시 active diagnostic | Owner, Action state, Reaction state 보강 검토 |
| `CMovementComponent.cpp` invalid gait map | active log | configuration diagnostic | Owner, requested gait context 보강 검토 |
| `CAnimNotify_ActionBase.cpp`, `CAnimNotifyState_ActionBase.cpp` invalid trigger action type | active log | notify asset diagnostic | notify name / mesh owner context 보강 검토 |
| `CAnimNotify_ReactionBase.cpp`, `CAnimNotifyState_ReactionBase.cpp` invalid trigger reaction type | active log | notify asset diagnostic | notify name / mesh owner context 보강 검토 |

### 판단

이 섹션은 제거 대상이 아니라 메시지를 diagnostic 기준에 맞출 대상이다.

반복 runtime flow가 아니라 asset 설정 오류, 실행 상태 불일치, notify 설정 오류를 짚는 로그라면 유지하되 원인 식별 context를 보강한다.

`CCombatSignalTargetComponent.cpp`의 runtime `FLog::Log` 4곳은 `Combat Signal Source / Target` 섹션에서 `FCombatSignalDebug` helper 이동 완료로 갱신했다.

---

## 13. 남은 작업

| 우선순위 | 작업 | 비고 |
| ---: | --- | --- |
| 1 | CombatEngage 상세 dump를 verbose audit에 연결하거나 제거 | fresh / promoted / preserved 상세 dump |
| 2 | CombatFeedback hit stop dead dump 제거 | 호출부 주석 처리된 hit stop dump |
| 3 | Notify / Combo / Character 임시 trace 제거 또는 diagnostic 승격 | 주석 처리된 flow trace 중심 |
| 4 | Remaining diagnostic log 메시지 보강 | HitFeedback, Orchestrator, Movement, Notify |
| 5 | `N22`, `N23`, worklist 최신화 | 완료 커밋 반영 |

---

## 14. 커밋 기록

```text
c24f92d1 refactor(debug): remove temporary investigate trace logs
ff5b1a58 refactor(debug): clean up action reaction runtime logs
65c716b9 refactor(debug): clarify combat data diagnostic logs
b28f6ae9 refactor(debug): remove unused component debug dumps
pending refactor(debug): gate combat signal diagnostics behind helper
```
