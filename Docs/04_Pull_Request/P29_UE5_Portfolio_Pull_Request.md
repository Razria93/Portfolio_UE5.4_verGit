# UE5 Portfolio Pull Request

## 제목

**P29: Component 참조 검증 정책 v1**

## 날짜

**2026.06.28**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/component-reference-validation-policy`

---

## 요약

이번 PR은 component dependency 초기화와 검증 기준을 작게 코드에 반영한다.

핵심은 `check()`를 전면 제거하는 것이 아니라, 필수 component 누락을 `ensureMsgf`로 드러내고 public request 경로에서는 기존처럼 `InvalidComponent` reject로 방어하는 패턴을 만드는 것이다.

---

## 변경 배경

W05 코드 품질 정리 계획에서 `check` 기반 component reference 검증은 주요 리스크로 분류되었다.

기존 코드에는 다음 패턴이 넓게 존재한다.

```text
BeginPlay
-> owner / component lookup
-> check()
```

이 패턴은 개발 중 문제를 빠르게 드러내는 장점이 있지만, Blueprint 구성이나 component 누락 같은 runtime 구성 오류에서는 어떤 dependency가 빠졌는지 설명력이 부족할 수 있다.

따라서 이번 PR에서는 필수 dependency가 많은 Orchestrator 계열에 먼저 작은 정책을 적용한다. 필수 dependency는 Orchestrator 내부에서 `FindComponentByClass`로 찾지 않고, owner가 알고 있는 native subobject를 명시적으로 주입한다.

---

## 변경 범위

### 1. ActionOrchestrator dependency 주입

`ACPlayer` / `ACEnemy`가 `PostInitializeComponents()`에서 `UCActionOrchestratorComponent`에 필수 dependency를 주입하도록 정리했다.

```text
InjectComponentReferences
BuildComponentReferences
FCharacterComponentReferences
UCActionOrchestratorComponent::InitializeReferences
ValidateRequiredComponentReferences
```

필수 component는 다음으로 정리했다.

```text
MovementComponent
WeaponComponent
StateComponent
HealthComponent
ActionComponent
ReactionComponent
```

`ObservableOverlayComponent`는 overlay snapshot / gate 보조 기능이므로 선택 dependency로 유지한다.

주입받은 Orchestrator 내부 필드는 일반 cache와 구분되도록 `_Injected` suffix를 사용한다.

`InitializeReferences`의 parameter list가 component 증가와 함께 계속 길어지는 문제를 막기 위해, Player / Enemy는 component 주소 목록을 `FCharacterComponentReferences`로 구성하고 Orchestrator는 이 구조체에서 필요한 참조만 선택적으로 가져간다.

### 2. ReactionOrchestrator dependency 주입

`UCReactionOrchestratorComponent`도 같은 방식으로 owner-side explicit injection을 적용했다.

필수 component는 다음으로 정리했다.

```text
StateComponent
HealthComponent
ActionComponent
ReactionComponent
```

`ObservableOverlayComponent`는 선택 dependency로 유지한다.

ReactionOrchestrator도 동일한 `FCharacterComponentReferences`를 전달받되, reaction orchestration에 필요한 owner / state / health / overlay / action / reaction 참조만 저장한다.

### 3. check 기반 참조 검증을 ensureMsgf 기반 검증으로 변경

두 Orchestrator 모두 주입된 owner / 필수 component가 누락된 경우 `ensureMsgf`로 구성 오류를 남긴다.

```text
Missing required ACharacter Owner | Owner=... | This=...
Missing required UCStateComponent | Owner=... | This=...
```

필수 component 누락 시에는 어떤 component가 누락되었는지 메시지에 남긴다.

### 4. Rename recovery 경로 제거

P24에서 추가했던 `ACPlayer` / `ACEnemy`의 `ResolveComponentReferences()`는 CombatSignal native component rename 직후의 migration recovery 코드였다.

```text
CombatSignalSourceComponent / CombatSignalTargetComponent rename 직후
-> Blueprint actor에는 renamed component instance가 존재
-> C++ member pointer가 invalid일 수 있음
-> FindComponentByClass로 기존 component instance를 재연결
```

현재 브랜치에서는 rename 안정화 이후의 component reference 기준을 정리하므로, 이 recovery 경로를 상시 dependency wiring 정책에서 제거했다.

비슷한 문제가 다시 발생하면 `FindComponentByClass`를 기본 wiring 방식으로 일반화하지 않고, rename 대상 component에 한정된 일시적 recovery hook으로 다시 검토한다.

```text
권장 이름
-> RecoverRenamedComponentReferences

유지 기준
-> Blueprint asset load / compile / save 전 또는 runtime pointer 검증 전

제거 기준
-> asset migration과 대표 flow 검증이 끝난 뒤
```

### 5. Component reference validation 기준 문서 추가

다음 노트를 추가했다.

```text
Docs/06_notes/N10_Component_Reference_Validation_Policy_Note.md
```

문서에는 다음 기준을 정리했다.

```text
- check 사용 기준
- ensureMsgf 사용 기준
- 필수 component / 선택 component 구분
- public request 경로의 InvalidComponent reject 정책
- 이번 브랜치에서 Orchestrator 계열만 먼저 적용한 이유
```

### 6. Code Quality Note 연결

`N08_Code_Quality_Cleanup_Plan_Note`의 component lookup / check 항목에서 세부 기준 문서 `N10`을 참조하도록 갱신했다.

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
```

### 정적 확인

```text
git diff --check
```

결과:

```text
성공
```

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외한다.

```text
- 프로젝트 전체 check 제거
- ActionComponent / ReactionComponent 초기화 검증 변경
- CombatSignalSource / CombatSignalTarget 초기화 검증 변경
- Feedback component 초기화 검증 변경
- Notify lookup 경로 정책 변경
- dependency injection 전면 적용
```

이번 PR의 목적은 적용 범위를 넓히는 것이 아니라, Orchestrator 계열에서 후속 component reference 정리의 기준 패턴을 만드는 것이다.

---

## 후속 작업

권장 후속 순서는 다음과 같다.

```text
1. ActionComponent / ReactionComponent 초기화 검증
2. CombatSignalSource / CombatSignalTarget 초기화 검증
3. Feedback component 초기화 검증
4. Notify lookup 경로 검증 정책 정리
```
