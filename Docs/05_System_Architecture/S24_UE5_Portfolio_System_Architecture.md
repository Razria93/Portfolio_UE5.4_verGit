# Intervention Rule / Interrupt 통합 리팩토링 계획

## Summary

- `Intervention`은 active / incoming 실행 간 개입 판단 구조로 유지한다.

- 실제 중단 결과는 `Interrupted`로 통일한다.

- 필터 설정은 `ActionData` / `ReactionData`가 소유한다.

- `CAction` / `CReaction`은 해당 rule을 런타임 상태와 함께 평가한다.

- Notify는 정책 데이터를 들지 않고 `WindowKey` 기준으로 intervention window의 begin / end 타이밍만 전달한다.

- `Want` / `Allow`는 설정 배열을 분리하되, rule 구조와 평가 로직은 공통화한다.

---

## Key Changes

### 1. 공통 타입 추가 / 정리

- `FExecutionInterventionRule`을 추가한다.

- `FExecutionInterventionRule`의 필드는 다음과 같다.

	- `EExecutionInterventionTiming Timing`: `Always`, `Window`

	- `FName WindowKey`

	- `TArray<FExecutionInterventionParticipantFilter> ParticipantFilters`

- `FActionData`, `FReactionData`에 다음 배열을 추가한다.

	- `WantInterventionRules`

	- `AllowInterventionRules`

- `EExecutionStopReason::Cancelled`는 제거하고 `Interrupted`로 통합한다.

- Action / Reaction stop reason, finish reason, feedback timing의 `Cancel` 계열도 `Interrupt`로 정리한다.

### 2. Notify 책임 축소

- `CAnimNotifyState_ExecutionInterventionWindow`에서 다음 정책 데이터를 제거한다.

	- `OwnerFilter`

	- `StopReason`

	- `WindowRole`

	- `CounterpartFilters`

- Notify는 `WindowKey`만 설정한다.

- Notify begin / end에서는 active executor에 window open / close만 전달한다.

- 기존 구조를 다음과 같이 변경한다.

	- 기존: 필터를 열고 닫는 구조

	- 변경: window key 활성 상태를 열고 닫는 구조

### 3. `CAction` / `CReaction` 평가 구조 변경

- 기존 transient filter 배열을 제거한다.

	- `WantCancelFilters`

	- `WantInterruptFilters`

	- `AllowCancelFilters`

	- `AllowInterruptFilters`

- 대신 active window key set을 transient로 관리한다.

- `WantIntervention()`은 incoming context의 data rule을 평가한다.

- `AllowIntervention()`은 active data의 allow rule을 평가한다.

- `Timing == Always` rule은 window 상태와 무관하게 평가한다.

- `Timing == Window` rule은 해당 `WindowKey`가 열려 있을 때만 평가한다.

- participant matching 기준은 다음과 같이 유지한다.

	- Want: `InQuery.ActivePart`

	- Allow: `InQuery.IncomingPart`

### 4. Exclusive decision hook 추가

- `CAction` / `CReaction`에 `CanResolveExclusiveRelationship(const FExecutionDecisionQuery&) const`를 추가한다.

- 이 hook은 “Exclusive 관계를 선택할 수 있는가”만 판단한다.

- 실제 개입 가능 여부는 기존처럼 `WantIntervention()` / `AllowIntervention()`에서 판단한다.

- `TryResolveIndependentOrExclusiveRelationship()`은 다음 순서로 관계를 결정한다.

	1. `CanResolveIndependentRelationship()`

	2. `CanResolveExclusiveRelationship()`

### 5. Orchestrator 정리

- Action / Reaction orchestrator 모두 intervention query의 `StopReason`을 `EExecutionStopReason::Interrupted`로 생성한다.

- Dead reaction의 force intervention 정책은 유지하되 stop reason은 `Interrupted`로 통일한다.

- `EExecutionApplyMode::Intervene`는 유지한다.

- `Intervene`는 “개입 적용”을 의미하고, 실제 stop reason은 directive의 `Interrupted`가 담당한다.

---

## Test Plan

### 빌드

- `PortfolioEditor Win64 Development` 빌드 성공 확인.

### 에디터 수동 검증

- sword equip / unequip 정상 동작.

- combo attack 0-1-2 chain 정상 동작.

- 무기 미장착 combo attack 거부.

- chain window 밖 입력이 reserve되지 않음.

- dodge가 `Interrupted` intervention으로 active action을 끊고 시작함.

- hit reaction이 허용된 intervention window에서 action을 interrupt함.

- dead reaction은 active action / reaction 여부와 무관하게 우선 실행됨.

- notify window begin / end에 따라 `WindowKey` 기반 rule이 열리고 닫힘.

- `Always` rule은 notify window 없이도 평가됨.

- `Window` rule은 window가 닫힌 상태에서 평가되지 않음.

### 회귀 확인

- 기존 `Cancelled` 기반 asset / notify 설정이 남아 있지 않은지 검색한다.

- `git diff --check` 통과.

- Action / Reaction request result 로그에서 Cancel 계열 문구가 남지 않는지 확인한다.

---

## Assumptions

- 설정 UX는 `WantInterventionRules` / `AllowInterventionRules` 분리 배열로 간다.

- 정적 intervention 정책 소유권은 `ActionData` / `ReactionData`에 둔다.

- `CAction` / `CReaction`은 rule 평가와 subclass 특수 조건만 담당한다.

- `Intervention` 네이밍은 유지하고, `Cancel` 네이밍만 `Interrupt`로 통합한다.

- 이번 리팩토링에서는 `Intervene`, `StopOnly`, `StartIncoming` 같은 directive / apply 구조는 유지한다.

---

## Follow-up

- `Reaction_Hit::WantIntervention()`은 data rule로 편입한 뒤 제거할 수 있다.

- `Reaction_Dead::WantIntervention()`은 orchestrator force path와 중복되므로 제거할 수 있다.

- `Reaction_Dead::AllowIntervention()`은 active dead reaction의 terminal invariant guard로 유지할 수 있다.

---

## Prompt History

### 1. Intervention / Interrupt 네이밍 정리

- `Cancel`과 `Interrupt`를 분리해 쓰는 것이 실제 의미 차이를 만들지 못하고 복잡도만 늘린다는 문제를 확인했다.

- `Cancel`은 active execution을 외부 실행이 끊는 의미에서는 `Interrupted`로 통합하기로 결정했다.

- `Intervention`은 active / incoming 실행 사이의 개입 판단 상황과 절차를 총칭하는 이름으로 유지한다.

- `Interrupt`는 intervention 결과로 active execution이 실제 중단되는 stop reason / finish reason을 의미하도록 정리한다.

### 2. Filter 소유권 정리

- Notify가 `OwnerFilter`, `StopReason`, `WindowRole`, `CounterpartFilters`를 들고 있으면 montage asset이 실행 정책 저장소가 되는 문제가 있다고 판단했다.

- 정적 intervention 정책은 `ActionData` / `ReactionData`가 소유하는 것이 더 적절하다고 판단했다.

- `CAction` / `CReaction`은 data rule을 읽고, 현재 owner / runtime state / active window 상태를 반영해 최종 판단하는 역할로 둔다.

### 3. Want / Allow 설정 구조 결정

- `Want`와 `Allow`는 의미가 다르므로 설정 배열은 분리하기로 결정했다.

- 단, rule 구조와 평가 함수는 공통화한다.

- 최종 구조는 다음 방향으로 확정했다.

	- `WantInterventionRules`

	- `AllowInterventionRules`

	- 공통 `FExecutionInterventionRule`

### 4. Global / Window 정책 구분

- 전역 정책과 타이밍 기반 정책을 구분해야 한다고 판단했다.

- 이름은 `Global`보다 의미가 덜 모호한 `Always` / `Window`로 정리했다.

- `Always` rule은 notify window와 무관하게 평가한다.

- `Window` rule은 notify가 열어 둔 `WindowKey`가 활성 상태일 때만 평가한다.

### 5. Notify 책임 축소

- Notify는 더 이상 intervention filter를 직접 주입하지 않는다.

- Notify는 `WindowKey`만 열고 닫는 타이밍 전달자로 축소한다.

- 실제 policy matching은 `ActionData` / `ReactionData` rule을 기준으로 `CAction` / `CReaction`에서 수행한다.

### 6. Exclusive Relationship Hook

- `CAction` / `CReaction`에 `CanResolveExclusiveRelationship()`을 추가하기로 결정했다.

- 이 hook은 “exclusive 관계를 선택할 수 있는가”만 판단한다.

- 실제 개입 의사와 허용 여부는 `WantIntervention()` / `AllowIntervention()`에서 판단한다.

### 7. Dead / Hit Reaction 편입 판단

- `Reaction_Hit::WantIntervention()`은 data rule로 편입 가능한 정책으로 판단했다.

- `Reaction_Dead::WantIntervention()`은 incoming dead를 orchestrator가 force path로 명시 처리하므로 제거 가능하다고 판단했다.

- `Reaction_Dead::AllowIntervention()`은 현재 정상 경로에서는 거의 호출되지 않지만, active dead reaction이 불특정 incoming execution에 의해 끊기지 않도록 막는 terminal invariant guard로 남길 가치가 있다고 판단했다.
