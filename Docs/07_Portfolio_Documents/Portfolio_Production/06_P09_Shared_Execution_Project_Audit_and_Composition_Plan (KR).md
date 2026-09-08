# 현행 p.11 Execution Transition 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.11에 적용한다.

> 상태: **코드·설계 문서 감사 완료 / Before·After PNG HTML 적용 완료 / 런타임 캡처 대기**  
> 목적: p.11은 Action과 Reaction을 하나의 실행기로 표현하지 않고, 허용된 `Directive` 뒤 기존 executor가 자기 Runtime Effect를 `Interrupt → Cleanup → Next Start` 순서로 전환하는 경계를 설명한다.

## 1. 확인 근거

### 코드

- `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
  - `ProcessActionCandidate()`는 `ActionCandidate → ActionContext → FExecutionDecisionQuery → FExecutionDecisionResult → FActionExecutionResult → DispatchActionDecision()` 흐름을 사용한다.
  - `BuildDecisionQuery()`는 Snapshot, Incoming Action Participant, Active Execution Participant를 구성한다.
- `Source/Portfolio/Component/CReactionOrchestratorComponent.cpp`
  - `ProcessReactionCandidate()`는 `ReactionCandidate → ReactionContext → FExecutionDecisionQuery → FExecutionDecisionResult → FReactionExecutionResult → DispatchReactionDecision()` 흐름을 사용한다.
  - Damage, Balance Lifecycle, Execution의 반응 요청은 각 Candidate로 정규화된 뒤 같은 판단 형식으로 진입한다.
- `Source/Portfolio/Type/CExecutionTypes.h`
  - `FExecutionDecisionQuery`, `FExecutionDecisionResult`, `EExecutionApplyMode`, `FExecutionInterventionDirective`가 공통 계약을 정의한다.

### 설계 문서

- `Docs/05_System_Architecture/S18_UE5_Portfolio_System_Architecture.md`: Decision / Relationship / ApplyMode / Intervention 중심으로 책임을 분리한 전환 근거.
- `Docs/05_System_Architecture/S20_UE5_Portfolio_System_Architecture.md`: Orchestrator, Component, Executor의 실행 계층 책임 분리.
- `Docs/05_System_Architecture/S21_UE5_Portfolio_System_Architecture.md`: Snapshot, Participant, Context, DataKey의 입력 책임 분리.

## 2. 확정 주장 범위

1. Action과 Reaction은 **각각의 Orchestrator와 각 도메인별 ExecutionResult**를 가진다. 하나의 Orchestrator 인스턴스나 하나의 결과 객체를 공유한다고 표현하지 않는다.
2. 두 경로는 Candidate·Context로 요청을 정규화한 뒤, 동일한 `FExecutionDecisionQuery`와 `FExecutionDecisionResult` 계약으로 판단한다.
3. Component는 Start·Reserve·Intervene 같은 결과를 적용하고, Executor는 개별 규칙과 실행 수명주기를 담당한다.
4. p.11은 공통 전환 계약과 terminal cleanup의 구조 페이지다. Intervention 정책은 p.12, Combat Signal Source/Target은 p.13–p.14, Pair 협업은 p.15로 한정한다.

## 3. 페이지 구성

### 중심 시각 자료 — 전환 계약과 terminal cleanup

Mermaid 원본: `Assets/Diagrams/p11_execution_transition_contract.mmd`. 이는 After의 요청·분기·Directive·정리·시작 순서를 관리하는 기준 원본이다.

삽입 시안은 같은 프레임에 위·아래로 배치한다.

- **Before** — `Assets/Diagrams/p11_execution_transition_before.png`는 `Diagram/PP/ExecutionTransition_Before.png`를 그대로 복사한 설명 도식이다. 새 Hit Reaction이 암시적 `Montage End` callback을 통해 종료·정리되던 흐름, 그리고 종료 실행과 새 실행의 관계를 명시적으로 보존하지 못하는 위험을 보여 준다.
- **After** — `Assets/Diagrams/p11_execution_transition_300.png`는 현재 `Diagram/PP/ExecutionTransition_After.png`와 동일한 PNG다. `중단할 Active Execution 존재? → 전환 Directive → Stop Montage → Cleanup Runtime → 새 Execution 시작`의 시각 순서를 사용한다. Want·Allow·Timing Gate의 세부 정책은 p.12에 한정한다.

두 도식은 리팩터링의 문제와 구조적 해결을 설명하는 시각 자료이며, 전환 성공을 증명하는 런타임 캡처가 아니다.

페이지 읽기 순서는 **구조 문제 → Before/After → 구조 해결**으로 고정한다. 구조 해결은 Orchestrator의 Directive 생성, 기존 Executor의 terminal cleanup, 다음 Executor의 lifecycle 시작이라는 책임 3칸으로 표현한다. p.11에는 `런타임 검증` 카드·`Needs Capture` 목업·별도 문제 해결 경험 카드를 배치하지 않는다. E11a/E11b와 B10은 Evidence Ledger에서만 추적한다.

```text
새 Execution 요청 (Action 또는 Reaction)
  → Active 실행 존재?
      ├─ 아니오 → 새 Executor Start
      └─ 예 → 전환 Directive
                 → 기존 Active Executor
                     StopMontage → Cleanup Runtime
                 → 새 Executor Start
```

- Candidate·Context, Want·Allow, Relationship과 timing gate는 p.12의 정책 판단 시각 자료로 한정한다.
- p.11은 새 실행이 이전 실행의 Trail·Collision·Hit Context를 직접 초기화하지 않고, 기존 executor가 자기 terminal path에서 정리한다는 경계를 중심에 둔다.

### 보조 자료 1 — 요청 사례

- Action: Player Input / AI Intent → Candidate / Context → Action ExecutionResult
- Reaction: Damage Result / Balance Lifecycle → Candidate / Context → Reaction ExecutionResult

### 보조 자료 2 — 책임 분리

- Orchestrator: 요청 해석, 판단 query 구성, 결과 조립
- Component: 결과 적용, active 상태 및 cross-domain 전달
- Executor: 개별 실행 규칙, 애니메이션 수명주기

### 설계 위험 메모

과거 구조를 사실처럼 단정하지 않는다. 대신 **직접 실행 연결을 피한 이유**로 표현한다.

- Candidate·Context가 없으면 요청 의미와 상태 의존성이 판단 단계에 섞일 수 있다.
- Directive가 없으면 중단 주체·사유·후속 동작이 결과에 남지 않는다.

## 4. 증거·표현 제한

- 이 페이지의 핵심 근거는 코드와 구조 문서다. 신규 런타임 캡처를 완료된 증거처럼 요구하지 않는다.
- 추후 Debug Overlay 캡처를 추가한다면 Decision·Relationship·ApplyMode를 보조 근거로만 사용하고, p.10의 정책 사례와 중복하지 않는다.
- 25페이지 HTML에 Before·After 전환 도식을 반영했다. 제출 전에는 E11a/E11b 전환 캡처를 확보하거나 Code Confirmed 구조 설명으로만 한정한다.

## 5. 25페이지 전환 기준 (2026-09-06)

- 이 문서는 `S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md`의 p.11 구성 감사로 사용한다.
- B10은 Combat Signal 사례가 아니라 executor terminal cleanup의 구현 근거이므로 p.11 보조 근거로 재배치한다.
- E11a/E11b는 Action→Reaction과 Reaction→Reaction 전환을 각각 읽을 수 있어야 한다. p.11 본문에는 넣지 않으며, 캡처 전에는 전환 성공을 포괄적으로 단정하지 않는다.
