# Combat Interaction Code & Documentation Concordance Audit

> 상태: **Static Code·Document Audit Complete / 신규 PIE 캡처 미수행**  
> 감사일: 2026-09-06  
> 범위: Execution Transition·Intervention, 단방향 Combat Signal·Target Outcome Resolution, 양방향 Execution Pair.  
> 기준: 현행 C++ 코드를 우선한다. 기존 문서는 리팩터링 의도·과거 문제·검증 기록으로만 인용한다.

---

## 1. 결론 요약

| 구조 | 코드와 문서의 일치 | 포트폴리오 판단 | 제출 전 부족한 근거 |
| --- | --- | --- | --- |
| Execution Transition / Intervention | 높음 | 11p 구조, 12p B09 정책 사례로 연결 가능 | Action→Reaction, Reaction→Reaction의 제출용 전환 캡처 |
| Combat Signal Source/Target | 높음 | 13p Source 전달, 14p Target Outcome으로 분리 가능 | Target Packet 기준 Normal/Guard/Parry 비교 캡처 |
| Execution Pair | 높음 | Combat Signal과 별도 협업 구조로 15p를 둘 근거가 있음 | Pair 성공·사전 거절·Commit 전 취소/Release 캡처 |
| 기존 p14 Data-driven Resolve | 코드·문서 일치 | Target Outcome과는 별도 사례다 | 25p 전환 시 별도 배치 결정 필요 |

이번 감사로 다음을 확정한다.

1. Action과 Reaction은 하나의 Orchestrator나 하나의 실행 객체를 공유하지 않는다. 각 도메인은 별도 Orchestrator·Execution Result를 가지며, 공통 `FExecutionDecisionQuery` / `FExecutionDecisionResult` 계약을 사용한다.
2. 11p와 12p는 독립 사례가 아니라 Intervention 문제를 해결하기 위한 구조·정책의 연속 페이지다.
3. 13p와 14p는 하나의 **단방향 Combat Signal**을 Source 전달과 Target Outcome 단계로 나눈 연속 페이지다.
4. Execution Pair는 Source/Target이 한 방향으로 Packet을 소비하는 Combat Signal과 다르다. Pair는 상호 조건·Reservation·Commit·Release를 소유하는 별도 거래다.
5. Pair의 Commit 뒤 피해 적용은 `UCCombatSignalTargetComponent`의 `RequestExecutionOutcomeTarget()`이라는 **별도 Execution Outcome 진입점**에 위임된다. 따라서 Pair는 일반 `HandleDefaultDamageEvent()` / `FCombatSignalTargetPacket` 흐름의 하위 분기가 아니다.

---

## 2. 감사 입력과 방법

### 2.1. 현행 코드

| 영역 | 확인 파일 |
| --- | --- |
| Execution 판단·적용 | `CActionOrchestratorComponent.cpp`, `CReactionOrchestratorComponent.cpp`, `CExecutionTypes.h` |
| Executor terminal | `CAction.cpp`, `CReaction.cpp` |
| Combat Signal | `CCombatSignalSourceComponent.cpp`, `CCombatSignalTargetComponent.cpp`, `FCombatSignalDebug.cpp` |
| Pair / Balance | `CExecutionCollaborationComponent.cpp`, `CBalanceComponent.cpp` |

### 2.2. 기존 문서

| 목적 | 대조 문서 |
| --- | --- |
| Execution 경계·전환 | S18, S20, S21, S22, S23, B09, B10 |
| Combat Signal 경계 | J01, N05, N06, B12 |
| Pair 계약·증거 | S36, `20_P08_Balance_Collapse_P09_Execution_Composition_Plan` |
| 포트폴리오 기존 배정 | p11~p14 Audit Plan, Page Spec Index, Evidence Ledger, Claim Freeze Log |

### 2.3. 상태 표기

| 표기 | 의미 |
| --- | --- |
| Code Confirmed | 현행 코드의 타입·함수·호출 조건으로 확인 |
| Documented | 설계 문서 또는 Bug Report에 문제·의도·검증 기록 존재 |
| Runtime Recorded | 기존 Bug Report에 런타임 검증 기록 존재 |
| Needs Capture | 제출 가능한 신규 PIE/Overlay/로그 캡처는 아직 없음 |

이번 감사에서는 코드·문서를 읽고 대조했으며 PIE를 새로 실행하지 않았다. `Runtime Recorded`는 제출용 캡처가 있음을 뜻하지 않는다.

---

## 3. Execution Transition / Intervention

### 3.1. 현행 코드 흐름 — Code Confirmed

Action과 Reaction은 각자 다음 흐름을 갖는다.

```text
Action Candidate / Reaction Candidate
  → Resolve Action/Reaction Context
  → DataKey로 Data resolve
  → Executor resolve
  → FExecutionDecisionQuery
       [Snapshot + Incoming Participant + Active Participant]
  → Executor의 FExecutionDecisionResult
  → Relationship별 ApplyMode·Directive 확정
  → 각 Component가 Action/Reaction Execution Result 적용
```

근거:

- `UCActionOrchestratorComponent::ProcessActionCandidate()`
- `UCReactionOrchestratorComponent::ProcessReactionCandidate()`
- 두 `Resolve*Context()`는 Data와 Executor가 모두 resolve되어야 Context를 만든다.
- `FExecutionDecisionQuery`는 Snapshot, IncomingPart, ActivePart를 보유한다.
- `FExecutionDecisionResult`는 Decision과 Relationship을 보유한다.
- Action/Reaction은 별도 `FActionExecutionResult` / `FReactionExecutionResult`를 만들고 각 Component에 dispatch한다.

### 3.2. 정책·전환 책임 — Code Confirmed

| 관계 | Action | Reaction | 표현 제한 |
| --- | --- | --- | --- |
| Independent | Idle + active 없음이면 Start | 동일 | 단순 시작 |
| Sequential | Action만 Reserve 가능 | 즉시 Reject | Reaction Reserve를 주장하지 않음 |
| Exclusive | Want + active Allow 통과 시 Directive 후 Intervene | 동일, 단 Dead는 강제 예외 | 모든 개입을 Want∧Allow로 단정하지 않음 |

`ResolveInterventionDirective()`는 incoming executor의 `WantIntervention()`과 active executor의 `AllowIntervention()`을 확인하고, 통과 시 `FExecutionInterventionDirective`를 만든다. Directive에는 StopSource, SourceDomain, TargetDomain, StopReason, AfterStopAction이 들어간다.

Dead Reaction은 `UCReactionOrchestratorComponent`에서 participant permission을 조회하지 않고 강제 Intervention하는 별도 예외다.

### 3.3. terminal 정리 — Code Confirmed / B10 Documented

Interrupt 경로에서 Action과 Reaction은 모두 다음 순서를 따른다.

```text
StopMontage
  → CleanupRuntimeEffects
  → ClearRuntime
  → PlayFeedbackRequest
  → Event / HandleApply*Finished
```

- `UCAction::CleanupRuntimeEffects()`는 Weapon runtime state와 Action feedback runtime state를 정리한다.
- `UCReaction::CleanupRuntimeEffects()`는 Reaction feedback runtime state를 정리한다.
- `Complete()` 경로는 Montage가 이미 terminal에 도달한 뒤 호출되므로 `StopMontage` 없이 Cleanup → Clear → Feedback/Event 순서를 사용한다.

따라서 B10의 대표 문제는 Combat Signal 자체보다 **Execution terminal path의 runtime effect 정리**다. B10을 13p Combat Signal의 대표 사례로 두면 책임 범위가 섞인다. 11p 또는 12p의 전환 검증 보조 근거로 이동하는 것이 코드와 맞다.

### 3.4. 문서 대조

| 문서 | 일치한 내용 | 처리 |
| --- | --- | --- |
| S18 | Decision / Relationship / ApplyMode / Directive 전환 | 현행 구조 근거로 사용 |
| S20 | Decision, Component Apply, Executor Lifecycle, Notify 책임 분리 | 현행 구조 근거로 사용 |
| S21 | Snapshot/Participant와 Key/Context의 유효성 경계 | Context 설명 보조 근거로 사용 |
| S22 / S23 | Relationship별 적용 방식과 Directive 의미 | 정책 설명 근거로 사용 |
| B09 | Action↔Reaction 개입 실패, Want/Allow 분리, Notify timing gate | 12p 대표 문제 해결 근거 |
| B10 | Interrupt 뒤 Trail/Collision/Hit Context 잔존 | 11~12p 전환·terminal 정리 보조 근거 |

### 3.5. 포트폴리오 판정

| 페이지 | 허용 주장 | 증거 상태 |
| --- | --- | --- |
| 11p | Action/Reaction은 별도 도메인 실행이지만 공통 판단·전환 계약을 사용한다. | Code Confirmed / Needs Capture |
| 12p | 일반 개입은 incoming Want와 active Allow, 필요한 경우 timing gate로 결정한다. | Code Confirmed / Documented / Runtime Recorded(B09) |

11p의 런타임 증거는 Action과 Reaction이 각각 동일 형식의 Query/Result를 거친다는 직접 캡처가 필요하다. B07 AI Combo는 Action 경로의 연속 실행 근거일 뿐, Action·Reaction 공통 계약 전체의 런타임 증거로 확대하지 않는다.

---

## 4. 단방향 Combat Signal과 Target Outcome Resolution

### 4.1. Source 전달 흐름 — Code Confirmed

일반 hit 진입은 다음과 같다.

```text
FHitContext
  → Source ValidateRequest
  → Source Payload / Context
  → Source ValidateContext / CanSend
  → DamageSpec resolve / request damage 계산
  → TargetActor::TakeDamage(FDefaultDamageEvent)
  → UCCombatSignalTargetComponent::ProcessCombatDamageTarget
```

`UCCombatSignalSourceComponent`는 Source Actor, DamageCauser, Target Actor, Hit Impact, HitWindowKey, DamageSpecKey를 source-side payload/context로 구성한다. Target의 Guard/Parry/Reaction 종류는 Source에서 판정하지 않는다.

Timing Cue는 별도 진입을 가진다.

```text
RequestCombatSignalCue
  → FCombatSignal(ECombatSignalType::TimingCue)
  → Target::RequestCombatSignalTarget
  → ProcessCombatSignalTarget / HandleTimingCueSignal
```

현재 `HandleTimingCueSignal()`은 Blink·Repulse Tag의 수용 여부를 기록하고 `true`를 반환한다. 이 함수 안에서 실제 Blink/Repulse gameplay 결과를 적용하는 코드는 확인되지 않았다. 따라서 cue 전달·수용과 실제 행동 구현을 동일하게 표현하지 않는다.

### 4.2. Target Outcome 흐름 — Code Confirmed

`HandleDefaultDamageEvent()`의 일반 Damage 흐름은 다음 순서다.

```text
Engine Damage Input
  → Target Payload / Context
  → ValidateContext
  → Target pre-state snapshot
  → CanReceiveCombatSignal
  → ComputeTargetDamage
  → CommitCombatSignalTarget (Health)
  → ResolveDamageReactionOutcome
  → FCombatSignalTargetPacket
  → Reaction / Feedback / Parry Result dispatch / Debug
```

Target은 다음 기준으로 Outcome을 결정한다.

| 조건 | Target 처리 |
| --- | --- |
| 이미 Dead | Reject |
| `RejectAll` external policy | Reject |
| `DamageOnly` external policy | Damage만 commit, DefenseOutcome=None |
| Parry 가능 | Damage commit 없음, Parry Outcome |
| Guard 가능 | Guard multiplier 적용, BlockHit Outcome |
| 그 외 | 일반 Hit 또는 CollapseHit / Dead Outcome |

따라서 14p의 실제 경계는 `Packet + Target State + Target Rule → Outcome`이다. Damage 감쇠와 Reaction 결과를 Action/Reaction Orchestrator로 보내 먼저 결정하지 않고, Target Component가 확정한 뒤 필요한 Reaction Request를 보낸다.

### 4.3. 중요한 표현 제한 — Code Confirmed

`UCCombatSignalSourceComponent::CommitCombatSignalSource()`는 `SendDamageToTarget()` 반환값이 0 이하이면 Source Context를 `CommitFailed`로 기록한다. Target이 Parry를 수용해 Damage commit을 하지 않는 경우 Target은 accepted Parry packet을 dispatch하면서도 반환 Damage는 0일 수 있다.

따라서 다음 표현은 사용하지 않는다.

- "Source Accepted는 Target의 모든 성공 Outcome을 뜻한다."
- "Parry는 Source에서 실패한 Signal이다."

Parry 검증은 Source Context가 아니라 **Target Packet의 Accepted, DefenseOutcome, ReactionOutcome, HP 전후와 Result dispatch**를 기준으로 읽어야 한다.

### 4.4. 문서 대조

| 문서 | 일치한 내용 | 현행 코드 기준 보정 |
| --- | --- | --- |
| J01 / N05 / N06 | Source가 사실을 전달하고 Target이 자기 상태로 판단, 기존 Apply/Take 책임 재정의 | 현재 클래스명은 `UCCombatSignalSourceComponent` / `UCCombatSignalTargetComponent`를 사용 |
| N05 | Target 내부 메서드 단계로 먼저 나누고 과도한 component 일반화는 보류 | 현행 Target에 Validate/CanReceive/Compute/Commit/Dispatch 함수 경계 존재 |
| B12 | Guard Out → Hit 전환의 overlay cleanup 문제 | p6/Execution 전환 보조 근거이며 Target Outcome Packet 성공 증거와는 별도 |

과거 `ApplyDamageComponent` / `TakeDamageComponent` 명칭은 리팩터링 전 문제를 설명할 때만 사용한다. 현행 구조 설명에서는 Source/Target 명칭으로 통일한다.

### 4.5. 포트폴리오 판정

| 페이지 | 허용 주장 | 증거 상태 |
| --- | --- | --- |
| 13p | Source는 hit/cue의 사실을 구성·전달하고, Target을 직접 조작하지 않는다. | Code Confirmed / Documented / Needs Capture |
| 14p | Target은 incoming packet과 자기 상태·방어 규칙으로 Outcome을 확정한다. | Code Confirmed / Documented / Needs Capture |

13p와 14p는 하나의 단방향 Combat Signal의 연속 페이지다. 14p는 독립 상호작용 모델이 아니다.

---

## 5. 양방향 Execution Pair

### 5.1. Pair 시작·예약 — Code Confirmed

`UCExecutionCollaborationComponent::RequestCombatExecution()`은 Source 상태, Target Snapshot/Revision, 시작 거리·각도, Target Collaboration Component를 확인한다. 이후 Target의 `AcceptExecutionReservation()`을 호출한다.

Target은 다음을 확인한다.

```text
Target alive / idle
→ Collapse opportunity available
→ target Reaction data·Executor resolve
→ Target Outcome Policy(Standard/Lethal) resolve
→ TryResolveExecutionAppliedDamage
→ Balance opportunity reservation
```

그 뒤 Source는 Target Reaction과 Source Action을 각각의 Orchestrator에 요청하고, 양쪽이 Reserved 상태인지 확인한 뒤 `ActivateExecutionPair()`로 Pair를 Active로 만든다.

```text
Source Request
  → Target AcceptExecutionReservation
  → Target Balance Reservation
  → Target Execution Reaction request
  → Source Execution Action request
  → Pair Active
```

### 5.2. Commit·취소·완료 — Code Confirmed

Source Action의 Commit Notify는 `HandleSourceExecutionCommit()`으로 들어온다. Session, Target Snapshot, opportunity가 아직 유효한지 확인한 뒤 Target의 `RequestExecutionOutcomeTarget(FExecutionOutcomePacket)`을 호출한다.

`UCCombatSignalTargetComponent::ProcessExecutionOutcomeTarget()`은 일반 Damage handler와 다른 경로다.

```text
Execution Outcome Packet
  → TryResolveExecutionAppliedDamage
  → Target Collaboration CommitExecutionOutcome
  → Health::TakeDamage
  → Partner Commit 통지
```

즉 Pair는 별도 협업 거래이며, Target의 Health 적용 경계를 재사용한다. 일반 Combat Signal의 `FCombatSignalTargetPacket`을 소비해 Pair를 성립시키는 구조는 아니다.

Commit 이전 취소에서는 Target의 `ReleaseExecutionOpportunityReservation()`이 예약과 Loop TTL을 복구한다. Commit 이후에는 reservation이 `ExecutionPrimaryCommitted`로 소비되므로 이전 Collapse Loop로 rollback하지 않는다. EndPlay, Death, Target 변경, Action/Reaction 중단은 Session cancel 경로로 들어간다.

### 5.3. 문서 대조

| 문서 | 일치한 내용 | 처리 |
| --- | --- | --- |
| S36 | Source/Target pair, session, reservation, Standard/Lethal, Commit/Release 계약 | 기준 기술 문서로 유지 |
| p8/p9 Composition Plan | p8은 단일 Actor Balance lifecycle, p9는 두 Actor Pair transaction | 9p Gameplay / 15p 구조 설명 분리 유지 |
| Evidence C08 / C06c~e | 구조 구현은 기록됐으나 제출용 Pair capture 부족 | Needs Capture 유지 |

### 5.4. 포트폴리오 판정

| 페이지 | 허용 주장 | 증거 상태 |
| --- | --- | --- |
| 9p | Execution Pair가 게임플레이에서 실행되는 모습과 runtime data를 보여 준다. | Needs Capture |
| 15p 신규 | Pair는 상호 조건 확인·reservation·commit·release를 갖는 양방향 협업 구조다. | Code Confirmed / Documented / Needs Capture |

"프레임 단위 완전 동시 실행"보다 `양측 Reserved → 공동 Active/Commit → terminal 또는 Release` 계약으로 표현한다.

---

## 6. 기존 포트폴리오 배정에서 수정할 사항

| 항목 | 현행 배정 | 감사 결과 | 후속 조치 |
| --- | --- | --- |
| B10 | p13 Combat Signal 대표 사례 | Executor terminal cleanup 사례 | 11p Execution Transition 보조 근거로 이동 확정 |
| p14 Data-driven Resolve / C14 | Action·Reaction Key·Data·Context 경계 | Target Outcome Resolution과 다른 구조 | p11 Context 구성 보조 근거로 흡수 확정; p14 대표 주장에서는 제외 |
| 13p Combat Signal | source/target·packet trace | 유효하나 hit, cue, result ingress 범위를 구분해야 함 | Source 전달 중심으로 좁힘 |
| 14p 신규 방향 | 없음 | Target Outcome Resolution | Normal/Guard/Parry Target Packet 비교를 대표 증거로 추가 |
| Execution Pair | p9 Gameplay만 존재 | 구조를 설명할 독립 범위 존재 | 15p 신규, p9는 Gameplay Hero·runtime evidence 유지 |

### C14의 처리 원칙

기존 C14는 `DataKey → Data → resolved Context → Executor` 경계라는 유효한 Execution 구조 근거다. 다만 Packet과 Target State의 Outcome Resolution을 직접 설명하지는 않는다.

25페이지 전환안에서는 **11p의 Context 설명 보조 근거로 흡수**한다. 이 감사 기록은 기술 문서와 면접 보조로 보존하지만, p14를 "Data-driven Resolve"와 "Outcome Resolution"을 동시에 주장하는 페이지로 만들지 않는다.

---

## 7. 제출용 증거 게이트

| ID | 대상 | 필요한 캡처 | 상태 |
| --- | --- | --- | --- |
| E11-A | 11p | Action→Reaction 전환에서 Decision/Relationship/ApplyMode/Directive가 읽히는 Overlay 또는 Log | Needs Capture |
| E11-B | 11p | Reaction→Reaction 전환에서 동일 항목이 읽히는 Overlay 또는 Log | Needs Capture |
| E12 | 12p | B09 범위의 Want/Allow·timing gate·Interrupt 결과 | Needs Capture |
| E13 | 13p | Source Context와 Target Packet을 같은 사건으로 대조한 trace | Needs Capture |
| E14 | 14p | 동일 계열 hit의 Normal / Guard / Parry Target Packet 비교 | Needs Capture |
| E15-A | 15p | Pair 성공: Reservation → Active → Commit → terminal | Needs Capture |
| E15-B | 15p | Pair 거절: HP 1 Standard 또는 Target 조건 불충족 | Needs Capture |
| E15-C | 15p | Commit 이전 취소: Reservation Release와 TTL 복구 | Needs Capture |

기존 B09/B10의 런타임 검증 기록은 문서 근거로 보존하되, 위 Capture가 확보되기 전에는 포트폴리오에서 최종 성공 화면처럼 사용하지 않는다.

---

## 8. 다음 문서 작업 순서

1. [S37 Execution Transition & Intervention Architecture](../../05_System_Architecture/S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md)를 작성했다.
2. [S38 Combat Signal Architecture](../../05_System_Architecture/S38_UE5_Portfolio_Combat_Signal_Architecture.md)에 13p Source 전달과 14p Target Outcome을 연속 장으로 작성했다.
3. S36에 Combat Signal과 Pair의 접점(`RequestExecutionOutcomeTarget`)과 비동일성을 보강했다.
4. C14를 p11 Context 구성 보조 근거로 배정한다.
5. Page Spec, Audit Plan, Evidence Ledger, Claim Freeze Log과 HTML을 25페이지 체계로 동기화했다.
6. p.11–p.15 SVG 다이어그램과 목차·footer 순연을 반영하고, PDF 페이지 객체 25개를 확인했다. 이후에는 E11–E15 캡처만 실제 증거로 교체한다.
