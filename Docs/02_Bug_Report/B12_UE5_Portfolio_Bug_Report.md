# UE5 Portfolio Bug Report

## 제목

**B12: Guard Out 중 Hit reaction에서 Overlay handling이 실패하는 문제**

## 날짜

**2026.06.21**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/parry-action`

---

## 요약

- Guard Out 중 Hit reaction이 들어오면 active Guard Out action은 interrupt되지만, Hit reaction이 시작되기 전 요청한 `ClearGuardState` overlay handling이 실패해 Hit montage가 실행되지 않는 문제가 발생했다.

- 문제는 두 층으로 나뉜다.
  - `ClearGuardState`가 이미 정리된 상태에서 다시 요청될 수 있는데, 이를 실패로 보는 조건이 있었다.
  - `UCObservableOverlayComponent`가 long-lived policy registry를 사용하면서 registry 갱신 시점이 명시되어 있지 않았다.

- 최종 수정은 clear 계열 handling을 멱등 cleanup으로 보고, overlay policy registry는 dirty flag 기반으로 갱신하도록 정리했다.

- 이 문제를 통해 overlay registry는 매번 rebuild하는 임시 조회 구조가 아니라, 재사용 가능한 cache로 운용하되 갱신 조건을 명시해야 한다는 기준을 세웠다.

---

## 영향 범위

- Guard Out 중 Hit / Dead reaction 전환

- Observable Overlay policy registry

- Guard state cleanup handling

- Action / Reaction 실행 직전 overlay cleanup 적용

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 브랜치
  - `feature/parry-action`

- 관련 코드
  - `Source/Portfolio/Component/CObservableOverlayComponent.cpp`
  - `Source/Portfolio/Component/CObservableOverlayComponent.h`
  - `Source/Portfolio/Component/CDefenseComponent.cpp`
  - `Source/Portfolio/Component/CDefenseComponent.h`
  - `Source/Portfolio/Action/CAction_Guard.cpp`
  - `Source/Portfolio/Component/CReactionComponent.cpp`

- 관련 문서
  - `Docs/06_notes/N03_Guard_Hold_Overlay_Layer_Design_Note.md`

---

## 발생 조건

- Player가 Guard 상태에서 Guard Out을 실행한다.

- Guard Out 중 enemy attack이 들어와 일반 Hit reaction으로 전환되어야 한다.

- Hit reaction은 시작 전 Guard overlay를 정리하기 위해 `ClearGuardState` handling을 요청한다.

---

## 재현 방법

1. Player가 Guard In / Hold 상태로 enemy attack을 방어한다.

2. Guard release로 Guard Out을 실행한다.

3. Guard Out 중 enemy attack을 맞게 만든다.

4. Hit reaction이 실행되는지 확인한다.

5. 로그에서 overlay handling 실패 여부를 확인한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- Guard Out 중 Hit가 들어오면 Guard Out은 interrupt된다.

- Guard state는 정리된다.

- Hit reaction은 `ClearGuardState` handling을 적용한 뒤 정상 실행된다.

- 이미 Guard state가 정리되어 있어도 `ClearGuardState`는 성공해야 한다.

**실제 결과**

- Guard Out interrupt까지는 진행됐다.

- 이후 Hit reaction이 `ClearGuardState` handling을 요청했지만 overlay policy가 해당 handling을 처리하지 못해 실패했다.

- 결과적으로 Hit reaction이 `ReactionExecutionFailed`로 reject될 수 있었다.

예시 로그:

```text
[TakeDamageOutcome] Outcome=EDamageDefenseOutcome::None | Commit=true
[ReactionDecision] ApplyMode=EExecutionApplyMode::Intervene | ReactionType=EReactionType::Hit
[GuardIntervention] Stage=InterruptGuardOut | ActivePhase=EGuardActionPhase::Out | Incoming=Reaction:EReactionType::Hit | KeepGuardState=false
[OverlayHandling] No policy accepted Handling=EObservableOverlayHandling::ClearGuardState
[OverlayHandling] Failed Handling=EObservableOverlayHandling::ClearGuardState
[ReactionDecision] Overlay handling failed.
```

---

## 원인

### 1. Clear 계열 handling의 의미가 멱등 cleanup으로 정리되지 않음

Guard Out이 Hit reaction으로 interrupt되면 `CAction_Guard` 쪽에서 Guard state가 먼저 정리될 수 있다.

그 직후 incoming Hit reaction이 실행 전 `ClearGuardState` handling을 다시 요청할 수 있다.

이때 `ClearGuardState`는 "Guard state가 있을 때만 성공하는 동작"이 아니라 "Guard state가 없는 상태를 보장하는 동작"이어야 한다. 기존 조건은 이미 정리된 상태를 실패로 볼 수 있어 Hit reaction 실행을 막았다.

### 2. Overlay policy registry의 갱신 기준이 명시되지 않음

`UCObservableOverlayComponent`는 overlay policy 목록을 long-lived registry처럼 들고 있다.

하지만 registry가 현재 actor component 구성과 어긋날 수 있는 시점에 대한 갱신 기준이 없었다.

초기 보강으로는 handling 실패 시 registry를 rebuild하고 재시도하는 방식이 가능했지만, 이는 실패를 cache 갱신 트리거로 쓰는 구조라 원인 구분이 흐려진다.

---

## 검토한 선택지

### 1. 매 요청마다 registry rebuild

- 장점: 항상 현재 component 구성 기준으로 처리한다.

- 단점: cache 전략의 재사용 의미가 약해진다.

- 판단: 구조는 단순하지만 long-lived registry를 둔 의도와 맞지 않는다.

### 2. 실패 시 rebuild 후 재시도

- 장점: 기존 cache를 우선 사용하면서 stale cache 가능성을 복구할 수 있다.

- 단점: 실제 handling 거부와 registry stale 문제가 같은 실패 경로에 섞인다.

- 판단: v1 방어 보강으로는 가능하지만 장기 구조로는 애매하다.

### 3. Dirty flag 기반 registry refresh

- 장점: cache를 재사용하면서 갱신 조건을 명시한다.

- 장점: 실패가 아니라 변경 가능성을 registry 갱신 트리거로 사용한다.

- 판단: long-lived registry / cache 구조에 가장 적합하다.

---

## 수정 방향

### Clear handling 멱등 처리

- `ClearGuardState`
  - Guard input intent, guard start lock, guard pose, guard 판정, parry 판정, movement override를 정리한다.
  - 이미 정리된 상태에서도 성공한다.

- `ClearGuardOverlay`
  - Guard input intent는 유지하고 pose / guard / parry 판정과 movement override만 정리한다.
  - 이미 정리된 상태에서도 성공한다.

### Overlay policy registry dirty flag 적용

`UCObservableOverlayComponent`는 다음 기준으로 registry를 운용한다.

```text
ObservableOverlayPolicies
-> long-lived policy registry

bOverlayPolicyRegistryDirty
-> registry가 현재 component 구성과 어긋날 수 있음을 표시

WriteOverlaySnapshot / ApplyOverlayEvent / ApplyOverlayHandling
-> registry 사용 전 dirty면 refresh
-> dirty가 아니면 기존 registry 재사용
```

적용된 내부 API:

```cpp
MarkPolicyRegistryDirty()
RefreshPolicyRegistry()
RebuildPolicyRegistry()
```

외부 호출 API는 overlay 계층을 드러내고, 내부 helper API는 클래스 문맥에서 반복되는 단어를 줄이는 naming 기준도 함께 정리했다.

---

## 수정

- `UCDefenseComponent::CanApplyOverlayHandling()`에서 `ClearGuardState` / `ClearGuardOverlay`를 멱등 cleanup으로 허용했다.

- `UCObservableOverlayComponent`에 `bOverlayPolicyRegistryDirty`를 추가했다.

- `ApplyOverlayHandling()`의 실패 기반 rebuild 재시도 구조를 제거했다.

- `WriteOverlaySnapshot()`, `ApplyOverlayEvent()`, `ApplyOverlayHandling()` 진입 시 `RefreshPolicyRegistry()`를 호출하도록 정리했다.

- `BuildObservableOverlayPolicies()`를 `RebuildPolicyRegistry()`로 정리했다.

- overlay API명을 `ApplyOverlayEvent` / `ApplyOverlayHandling` 기준으로 단순화했다.

---

## 검증 결과

- `git diff --check` 통과.

- `PortfolioEditor Win64 Development` 빌드 통과.

- PIE에서 Guard Out 중 Hit가 들어왔을 때 `ReactionType=Hit`이 정상적으로 실행되는 것을 확인했다.

- 기존 실패 로그가 더 이상 발생하지 않는 것을 확인했다.

수정 후 기대 로그 흐름:

```text
[TakeDamageOutcome] Outcome=EDamageDefenseOutcome::None | Commit=true
[ReactionDecision] ApplyMode=EExecutionApplyMode::Intervene | ReactionType=EReactionType::Hit
[GuardIntervention] Stage=InterruptGuardOut | ActivePhase=EGuardActionPhase::Out | Incoming=Reaction:EReactionType::Hit | KeepGuardState=false
```

---

## 회귀 방지 기준

- clear 계열 overlay handling은 상태 존재 여부에 의존하지 않는 멱등 cleanup으로 유지한다.

- overlay policy registry는 실패를 트리거로 rebuild하지 않는다.

- registry 갱신은 dirty flag 또는 명시적 register / unregister 같은 변경 신호를 기준으로 수행한다.

- 외부 실행이 overlay 상태 변경을 요청하는 경우, event lifecycle 처리와 handling cleanup 처리를 섞지 않는다.

---

## 관련 PR / 문서

- Work List: `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`

- Note: `Docs/06_notes/N03_Guard_Hold_Overlay_Layer_Design_Note.md`

---

## 비고

- 이 Bug Report는 Guard Out 중 Hit reaction 실패라는 구체 증상을 다룬다.

- overlay layer의 장기 설계 판단과 API naming 기준은 N03에서 관리한다.

- 이후 overlay policy가 런타임에 동적으로 추가 / 제거되는 구조가 생기면 `MarkPolicyRegistryDirty()`를 호출하는 명시적 register / unregister 경로를 추가해야 한다.

---
