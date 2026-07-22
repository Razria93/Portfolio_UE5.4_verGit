# UE5 Portfolio Pull Request

## 제목

**P42: Debug Log Policy v1**

## 날짜

**2026.07.21**

## 상태

- [x] Debug log 분류 정책 정리
- [x] runtime debug dump 제거 / helper 분리
- [x] Diagnostic Hook / Debug Dump / Profiling Audit / Error / Warning 기준 문서화
- [x] CombatSignal / CombatResult diagnostic helper 추가
- [x] Action / Reaction / Movement / Feedback / Notify / Overlay diagnostic helper 추가
- [x] ComponentReference recovery 성공 로그 CVar gate 처리
- [x] 기존 AI / CombatEngage audit CVar gate 유지 및 보완
- [x] diagnostic helper API ordering 기준 정리
- [x] `PortfolioEditor Win64 Development` build 통과

## 브랜치

- `refactor/debug-log-policy-v1`

## 요약

이번 PR은 기본 출력으로 남아 있던 debug log / dump를 정리하고, 반복 가능성이 있는 runtime 관측점을 CVar 기반 diagnostic helper로 분리한다.

기존에는 전투, 액션, 리액션, 피드백, AI 경계에서 디버그용 출력이 기능 코드 안에 직접 남아 있거나, 주석 처리된 dump 함수로 남아 있었다. 이 방식은 Output Log 노이즈가 커지고, 어떤 로그가 Error / Warning / Debug Dump / Audit인지 구분하기 어려웠다.

이번 PR에서는 로그를 다음 기준으로 재분류했다.

```text
Error
-> 필수 component / asset 없음, 계속 진행하면 잘못된 상태
-> ensureMsgf / UE_LOG(Error) 유지 가능

Warning
-> fallback 가능한 data/config 문제
-> 필요하면 once / validation gate 적용

Debug Dump
-> context / payload / result 상세 출력
-> 기본 제거 또는 helper + CVar

Diagnostic Hook
-> reject / result / dispatch 관측 지점
-> gameplay 본문은 hook 호출만 남기고 출력은 helper가 담당

Profiling Audit
-> 측정 / 카운터 / CSV / CVar 기반 관측
-> 기존 RuntimeLOD / profiling audit은 유지
```

## 주요 변경

```text
1. 정책 / 인벤토리 문서 추가
   - N22: Debug Log Policy 작업 계획
   - N23: Debug Log / Diagnostic Code 정책
   - N24: Debug Log Cleanup Inventory
   - N25: Diagnostic Log Gating / Audit Category 설계
   - N26: Diagnostic Log Full Audit Inventory + PR final rescan

2. CombatSignal / CombatResult 진단 분리
   - FCombatSignalDebug 추가
   - FCombatResultDebug 추가
   - Source / Target / Weapon / Cue / Dispatch 경계 CVar audit 처리
   - DamageRequest / CombatSignal reject naming을 Rejected 기준으로 통일

3. Action / Reaction 진단 분리
   - FActionComponentDebug 추가
   - FReactionComponentDebug 추가
   - FExecutionOrchestratorDebug 추가
   - request result, execution decision, notify, montage lifecycle, runtime dump를 helper로 이동

4. Movement / Feedback / Notify / Overlay 진단 분리
   - FMovementDebug 추가
   - FCombatFeedbackDebug 추가
   - FAnimNotifyDebug 추가
   - FObservableOverlayDebug 추가
   - invalid notify trigger는 once warning으로 제한
   - overlay policy failure는 기본 출력하지 않고 CVar audit 처리

5. ComponentReference recovery 진단 분리
   - FComponentReferenceDebug 추가
   - 실패 ensureMsgf는 contract validation으로 유지
   - 성공 recovery 로그는 ComponentReferenceAudit CVar gate 뒤로 이동

6. 기존 AI / CombatEngage audit 유지
   - RuntimeLOD / CombatEngage 측정 목적 CVar는 유지
   - Shipping no-op / helper 내부 gate 보완

7. diagnostic helper API ordering 통일
   - request/gate reject는 먼저 배치
   - 같은 레벨 result 쌍은 positive-first
   - Accepted / Applied / Succeeded / Started / Played / Resolved / Allowed
   - Rejected / Ignored / Failed / Blocked
   - 단독 reject/warning API는 억지로 재배치하지 않음
```

## 주요 CVar

```text
Portfolio.Debug.CombatSignalAudit
Portfolio.Debug.CombatSignalDump
Portfolio.Debug.CombatResultAudit
Portfolio.Debug.ActionComponentAudit
Portfolio.Debug.ActionComponentDump
Portfolio.Debug.ReactionComponentAudit
Portfolio.Debug.ReactionComponentDump
Portfolio.Debug.MovementAudit
Portfolio.Debug.FeedbackAudit
Portfolio.Debug.ObservableOverlayAudit
Portfolio.Debug.ComponentReferenceAudit

Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit
```

## 변경 파일 범위

```text
Docs/06_notes/N22_Debug_Log_Policy_Work_Plan_Note.md
Docs/06_notes/N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md
Docs/06_notes/N24_Debug_Log_Cleanup_Inventory_Note.md
Docs/06_notes/N25_Diagnostic_Log_Gating_And_Audit_Category_Plan_Note.md
Docs/06_notes/N26_Diagnostic_Log_Full_Audit_Inventory_Note.md
Docs/06_notes/N27_Debug_Profiling_CVar_Ownership_Final_Note.md

Source/Portfolio/Core/Debug/*Debug.*
Source/Portfolio/Core/Debug/FComponentReferenceHelper.h

Source/Portfolio/Component/*
Source/Portfolio/Action/*
Source/Portfolio/Reaction/*
Source/Portfolio/Weapon/CWeaponActor.*
Source/Portfolio/Notify/*
Source/Portfolio/AI/BehaviorTree/*
Source/Portfolio/System/Combat/*
Source/Portfolio/Character/*
Source/Portfolio/Controller/*
```

## 검증

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Static check

```text
git status --short
Result: clean

git diff --check
Result: Pass
```

### PR final rescan

```text
Action / Reaction / Feedback / Overlay / Notify 직접 FLog::Log 잔여 없음
Core/Debug helper 내부 FLog::Log 허용
CAIController RuntimeLOD audit CVar 유지
CWorldSubsystem_CombatEngage assignment audit CVar + Shipping no-op 유지
CBTDecorator_CanMove decorator audit CVar 유지
check / ensureMsgf는 contract validation으로 유지
```

### Role-based review

```text
Code Safety Reviewer: Pass
Combat Meaning Reviewer: Pass
API Consistency Reviewer: Pass with notes
```

API Consistency notes는 다음 항목으로 반영했다.

```text
RecordReactionDataResolvedForAudit -> RecordReactionDataResolveFailedForAudit 순서 정렬
RecordRuntimeLODMovementIntentAllowedForAudit -> RecordRuntimeLODMovementIntentBlockedForAudit 순서 정렬
N26에 단독 reject/warning API 예외 명시
```

### Runtime smoke

CombatSignal CVar On 상태에서 확인:

```text
[Combat|WeaponActor|BeginOverlapAccepted]
[Combat|WeaponActor|BeginOverlapIgnored] Reason=SelfOverlap
[Combat|SignalSource|Accepted]
[Combat|SignalSource|Rejected] Reason=CommitFailed
[Combat|SignalTarget|Accepted]
[Combat|SignalCue|Accepted]
[Combat|SignalCue|Rejected] Reason=MissingAITarget
[Combat|SignalTargetCue|Accepted]
[Combat|ResultDispatch|Delivering]
[Combat|ResultDispatch|Delivered]
```

Action / Reaction CVar On 상태에서 확인:

```text
[Action|Executor|Started]
[Action|Executor|MontagePlayed]
[Action|Component|StartApplied]
[Action|Component|ReserveApplied]
[Action|Component|ReserveActionRejected]
[Action|Component|DataResolveFailed]
[Action|Executor|MontageIgnored]

[Reaction|Component|DataResolved]
[Reaction|Component|StartAccepted]
[Reaction|Component|InterveneApplied]
[Reaction|Executor|Started]
[Reaction|Executor|MontagePlayed]
[Reaction|Executor|MontageIgnored]
```

첨부 Output Log 기준으로 `Error`, `Warning`, `Accessed None`, `ensure failed` 류는 확인되지 않았다.

## 후속 확인

```text
1. 모든 debug CVar Off 상태에서 기본 debug dump가 출력되지 않는지 최종 smoke
2. Shipping build에서 신규 debug CVar / output 구현 제외 여부 확인
3. 다음 code quality sweep에서 TODO / naming / const consistency로 이동
```

## PR 설명 초안

```md
## Summary

- 정리되지 않은 runtime debug dump를 diagnostic helper / CVar audit 구조로 분리
- CombatSignal, CombatResult, Action, Reaction, Movement, Feedback, Notify, Overlay, ComponentReference 진단 경계를 helper API로 정리
- 반복 가능성이 있는 reject/result/dispatch 관측점은 기본 출력 없이 CVar gate 뒤로 이동
- Error / Warning / Debug Dump / Diagnostic Hook / Profiling Audit 정책을 N22~N26 문서로 정리
- diagnostic helper API ordering 기준을 request/gate reject -> positive result -> negative result로 통일

## Validation

- git status --short clean
- git diff --check 통과
- PortfolioEditor Win64 Development 빌드 통과
- PR 전 잔여 FLog::Log 재스캔 완료
- Code Safety / Combat Meaning / API Consistency 역할 분리 리뷰 완료
- CombatSignal / Action / Reaction runtime smoke log 확인
```
