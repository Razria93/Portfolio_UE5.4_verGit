# S37. Execution Transition & Intervention Architecture

> 상태: **현행 코드 기준 확정 / 제출용 전환 캡처 대기**  
> 작성일: 2026-09-06  
> 근거 감사: `Docs/07_Portfolio_Documents/Portfolio_Production/26_Combat_Interaction_Code_Document_Concordance_Audit (KR).md`

---

## 1. 목적과 범위

이 문서는 Action과 Reaction 실행 중 새 실행이 들어올 때, 기존 실행을 어떤 조건으로 중단하고 새 실행을 어떻게 시작하는지의 구조를 정의한다.

핵심 목적은 **Intervention을 개별 실행 객체의 예외 처리로 누적하지 않고, 공통 전환 계약으로 처리하는 것**이다.

포함 범위:

- Action / Reaction Candidate와 resolved Context
- 공통 Decision Query / Result
- Relationship, ApplyMode, Intervention Directive
- Start / Interrupt / Complete terminal과 Runtime Effect 정리

제외 범위:

- Damage 결과의 Guard / Parry 판정
- Source/Target Combat Signal Packet 전달
- 두 Actor의 Pair reservation·commit 거래

---

## 2. 문제와 설계 요구

### 2.1. 문제

피격 반응을 기존 실행 위에 덧씌우는 방식에서는 새 Reaction이 기존 Action 또는 Reaction이 남긴 Trail, Collision, Hit Context 등의 소유 정보를 알기 어렵다. 이 경우 특정 값만 안전하게 정리하기보다 광범위한 초기화에 의존하게 된다.

또한 다음 요구가 함께 존재한다.

- Action 내부의 연속 실행
- Reaction 내부의 재피격 개입
- Action 실행 중 Reaction 개입
- Reaction 중 새 Action 또는 Reaction 개입

실행 객체가 이 전환을 각각 처리하면, 새 상태와 새 예외가 추가될 때마다 "무엇이 무엇 위에 덧씌워지는가"를 해당 객체들이 알아야 한다.

### 2.2. 설계 요구

1. Action과 Reaction은 별도 도메인 실행으로 유지한다.
2. 두 도메인은 incoming/active 실행을 같은 형식으로 비교할 수 있어야 한다.
3. 실행 가능 여부와 적용 방식은 분리한다.
4. 기존 실행을 Interrupt할 때 Runtime Effect 정리가 terminal path에서 보장되어야 한다.
5. Notify는 정책 소유자가 아니라 timing gate여야 한다.

---

## 3. 책임 경계

| 주체 | 책임 | 하지 않는 일 |
| --- | --- | --- |
| `UCAction` / `UCReaction` | 고유 실행 규칙, Want/Allow, montage lifecycle, terminal cleanup | cross-domain 전환 순서 전체를 직접 소유하지 않음 |
| Action / Reaction Orchestrator | Candidate→Context, Decision Query 구성, Relationship·ApplyMode·Directive 확정 | Damage Outcome 판정, Pair transaction 소유 |
| Action / Reaction Component | Execution Result 적용, active 실행 상태와 완료 통지 | Source/Target Packet 판정 |
| Notify | 실행 중 시점 이벤트와 Allow Window key 전달 | Intervention 정책 자체 결정 |

Action과 Reaction은 각각 `UCActionOrchestratorComponent`, `UCReactionOrchestratorComponent`, `FActionExecutionResult`, `FReactionExecutionResult`를 사용한다. 하나의 공통 Orchestrator나 하나의 공통 실행 객체가 존재한다고 표현하지 않는다.

---

## 4. 공통 전환 계약

```text
Action Candidate / Reaction Candidate
  → Resolve Context
      [DataKey → resolved Data → Executor]
  → FExecutionDecisionQuery
      [Snapshot + Incoming Participant + Active Participant]
  → FExecutionDecisionResult
      [Decision + Relationship]
  → ApplyMode / InterventionDirective
  → Component Apply
  → Executor Start / Interrupt / Complete
```

### 4.1. Context와 Query

- `ResolveActionContext()` / `ResolveReactionContext()`는 Data와 Executor를 모두 resolve한 뒤에만 Context를 만든다.
- `FExecutionDecisionQuery`는 현재 실행 상태를 가진 Snapshot과 incoming/active Participant를 분리한다.
- `FExecutionParticipant`는 Action Context 또는 Reaction Context와 executor, priority를 표현한다.

따라서 lookup용 DataKey와 실제 실행 가능한 Context를 같은 의미로 쓰지 않는다. wildcard나 global match는 lookup 단계의 규칙일 수 있지만, runtime Context는 resolved Data와 Executor가 있어야 한다.

### 4.2. Relationship과 ApplyMode

| Relationship | Action Apply | Reaction Apply | 의미 |
| --- | --- | --- | --- |
| Independent | Start | Start | active 실행이 없는 Idle 상태에서 시작 |
| Sequential | Reserve | Reject | Action chain의 예약·Notify consume 경로 |
| Exclusive | Intervene | Intervene | active 실행을 중단하고 incoming 실행 시작 |

`FExecutionDecisionResult`는 executor가 처리 가능한 Decision·Relationship을 반환하는 계약이다. Orchestrator는 Snapshot과 active Participant를 검증한 뒤 ApplyMode를 확정한다.

### 4.3. Intervention Directive

Exclusive 관계에서 일반 개입은 다음 순서로 판단한다.

```text
incoming executor WantIntervention
AND
active executor AllowIntervention
  → FExecutionInterventionDirective
  → active Interrupt
  → incoming Start
```

Directive는 StopSource, SourceDomain, TargetDomain, StopReason, AfterStopAction을 보존한다. Dead Reaction은 participant permission을 조회하지 않는 강제 Intervention 예외다.

---

## 5. Lifecycle과 Runtime Effect 정리

### 5.1. Interrupt

Action과 Reaction의 Interrupt terminal은 다음 순서를 사용한다.

```text
StopMontage
  → CleanupRuntimeEffects
  → ClearRuntime
  → PlayFeedbackRequest
  → Event / HandleApply*Finished
```

- Action cleanup은 Weapon runtime state와 Action feedback runtime state를 정리한다.
- Reaction cleanup은 Reaction feedback runtime state를 정리한다.
- `ClearRuntime()`은 executor 내부 cache·active state·Allow Window를 정리한다.

### 5.2. Complete

Montage가 이미 terminal에 도달한 Complete 경로는 StopMontage 없이 Cleanup → Clear → Feedback/Event 순서를 사용한다. 따라서 "모든 terminal이 StopMontage를 호출한다"고 표현하지 않는다.

### 5.3. B10의 위치

B10은 Notify End 또는 feedback end에 의존하던 Trail·Collision·Hit Context 정리를 executor terminal path로 옮긴 사례다. Combat Signal Packet의 책임이 아니라 **Execution 전환 뒤 남는 Runtime Effect 정리**의 근거로 사용한다.

---

## 6. 불변 조건

| ID | 조건 | 상태 |
| --- | --- | --- |
| EX-01 | Data와 Executor가 resolve되지 않은 Candidate는 Context가 되지 않는다. | Code Confirmed |
| EX-02 | Action과 Reaction은 별도 Orchestrator·Execution Result를 유지한다. | Code Confirmed |
| EX-03 | 일반 Exclusive Intervention은 incoming Want와 active Allow가 모두 필요하다. | Code Confirmed / Documented |
| EX-04 | Dead Reaction은 EX-03의 강제 예외다. | Code Confirmed / Documented |
| EX-05 | Reaction은 Sequential/Reserve를 지원하지 않는다. | Code Confirmed |
| EX-06 | Interrupt terminal은 CleanupRuntimeEffects와 ClearRuntime을 수행한다. | Code Confirmed / Runtime Recorded(B10) |
| EX-07 | Notify는 timing gate이며 policy 규칙의 단독 소유자가 아니다. | Code Confirmed / Documented(B09) |

---

## 7. 증거 상태와 표현 제한

| 주장 | 코드 | 문서 | 런타임 | 제출 상태 |
| --- | --- | --- | --- | --- |
| 공통 Decision Query/Result 계약 | Confirmed | S18/S20/S21 | 없음 | Needs Capture |
| Want/Allow 분리와 timing gate | Confirmed | B09, S22/S23 | B09 기록 | Needs Capture |
| Interrupt cleanup | Confirmed | B10 | B10 기록 | Needs Capture |
| Action chain Reserve | Confirmed | S22 | 별도 사례 | 범위 제한 |

금지 표현:

- Action과 Reaction이 하나의 Orchestrator 또는 하나의 Executor를 공유한다.
- 모든 Intervention이 예외 없이 Want∧Allow로 처리된다.
- B10이 Combat Signal의 Packet 처리 문제다.
- 신규 PIE 캡처 없이 모든 terminal 회귀가 최종 제출 증거로 완료됐다.

---

## 8. 포트폴리오 연결

| 페이지 | 역할 | 런타임 캡처 게이트 |
| --- | --- | --- |
| 11p | 중첩 반응 문제와 `Directive → 기존 executor Interrupt/Cleanup → 새 executor Start` 전환 계약 | Action→Reaction, Reaction→Reaction의 Query/Result/ApplyMode/Directive |
| 12p | Want/Allow·Priority·Timing Gate를 B09 사례로 검증 | Action 중 HitReaction, HitReaction 중 새 HitReaction, Dead 예외 범위 |

11p와 12p는 하나의 구조 리팩터링의 구조·정책 페이지다.
