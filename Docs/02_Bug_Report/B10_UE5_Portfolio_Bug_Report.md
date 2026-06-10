# UE5 Portfolio Bug Report

## 제목

**B10: Action trail / collision runtime effect가 hit 또는 dead interrupt 이후 정리되지 않는 문제**

## 날짜

**2026.06.04**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/orchestration-refactor`

---

## 요약

- 공격 액션 중 trail, collision, hit context가 열린 상태에서 hit reaction 또는 dead reaction으로 action이 interrupt되면 trail이 꺼지지 않는 문제가 발생했다.
  
- collision notify end 또는 hit context notify end가 interrupt 경로에서 실행되지 않으면 collision / hit context도 stale state로 남을 수 있었다.
  
- 원인은 runtime effect 정리가 animation notify end 또는 feedback end 흐름에 의존하고 있었기 때문이다.
  
- `CleanupRuntimeEffects()`를 추가하고, feedback request 생성과 실행을 `BuildFeedbackRequest()` / `PlayFeedbackRequest()`로 분리하여 executor 종료 경로에서 runtime side effect를 명시적으로 정리하도록 수정했다.

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 브랜치:
  - `feature/orchestration-refactor`

- 관련 코드:
  - `Source/Portfolio/Action/CAction.cpp`
  - `Source/Portfolio/Action/CAction.h`
  - `Source/Portfolio/Action/CAction_ComboAttack.cpp`
  - `Source/Portfolio/Reaction/CReaction.cpp`
  - `Source/Portfolio/Reaction/CReaction.h`
  - `Source/Portfolio/Component/CActionFeedbackComponent.cpp`
  - `Source/Portfolio/Component/CActionFeedbackComponent.h`
  - `Source/Portfolio/Component/CReactionFeedbackComponent.cpp`
  - `Source/Portfolio/Component/CReactionFeedbackComponent.h`
  - `Source/Portfolio/Component/CWeaponComponent.cpp`
  - `Source/Portfolio/Component/CWeaponComponent.h`

---

## 재현 방법

1. sword combo attack 또는 trail feedback이 켜지는 action을 실행한다.
   
2. action trail 또는 collision window가 열린 상태에서 character가 hit reaction으로 interrupt되도록 한다.
   
3. 같은 조건에서 dead reaction으로 interrupt되는 케이스도 확인한다.
   
4. interrupt 이후 weapon trail, collision, hit context가 정상적으로 정리되는지 확인한다.
   
5. interrupt / complete feedback이 cleanup 이후에도 정상 재생되는지 확인한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- active action이 hit 또는 dead reaction으로 interrupt되면 action runtime effect가 즉시 정리되어야 한다.
  
- weapon trail은 OFF 상태로 돌아가야 한다.
  
- weapon collision은 disabled 상태로 돌아가야 한다.
  
- apply damage hit window는 closed 처리되어야 한다.
  
- pushed hit context는 clear되어야 한다.
  
- interrupt / complete feedback은 cleanup에 의해 제거되지 않고 정상 재생되어야 한다.

**실제 결과**

- action trail이 켜진 상태에서 hit 또는 dead interrupt가 발생하면 trail이 꺼지지 않는 현상이 발생했다.
  
- collision notify end 또는 hit context notify end가 interrupt 경로에서 정상 실행되지 않을 경우, collision / hit context도 stale state로 남을 수 있었다.
  
- 기존 `ClearRuntime()`은 executor 내부 cache만 정리하고 외부 component / weapon actor runtime state는 정리하지 않았다.

---

## 원인

- trail off, collision disabled, hit context clear가 notify end 또는 feedback data에 의존하고 있었다.
  
- `UCAction::Stop()`과 `UCReaction::Stop()`은 montage stop 이후 runtime effect cleanup을 명시적으로 수행하지 않았다.
  
- 기존 `RequestFeedback()`은 feedback request 생성과 실행을 한 함수 안에서 처리했기 때문에 terminal feedback을 `ClearRuntime()` 이후로 내리기 어려웠다.
  
- 그 결과 cleanup 순서와 terminal feedback 실행 순서가 명확하게 분리되지 않았다.

---

## 수정

종료 경로를 다음 순서로 정리했다.

```text
Capture terminal feedback request / event payload
-> StopMontage
-> CleanupRuntimeEffects
-> ClearRuntime
-> PlayFeedbackRequest
-> EmitEvent / HandleFinished
```

### Action

- `UCAction::CleanupRuntimeEffects()`를 추가했다.
  
- `UCWeaponComponent::ClearRuntimeWeaponState()`를 통해 hit context와 collision을 정리했다.
  
- `UCActionFeedbackComponent::ClearRuntimeFeedback()`을 통해 trail을 강제 OFF했다.
  
- `RequestFeedback()`를 제거하고 모든 action feedback 호출을 `BuildFeedbackRequest()` + `PlayFeedbackRequest()`로 통일했다.
  
- `Stop()` / `Complete()`는 feedback request와 action index를 먼저 캡처한 뒤 cleanup / clear 이후 feedback과 event를 실행한다.

### Reaction

- `UCReaction::CleanupRuntimeEffects()`를 추가했다.
  
- `UCReactionFeedbackComponent::ClearRuntimeFeedback()`를 추가했다.
  
- 현재 reaction feedback cleanup은 no-op이지만, 향후 loop VFX / SFX가 추가될 때 동일한 종료 지점에서 정리할 수 있도록 hook을 맞췄다.
  
- `RequestFeedback()`를 제거하고 reaction feedback 호출도 `BuildFeedbackRequest()` + `PlayFeedbackRequest()`로 통일했다.

---

## 검증 결과

- `RequestFeedback` 잔여 참조가 없음을 확인했다.
  
- `git diff --check` 통과를 확인했다.
  
- `PortfolioEditor Win64 Development` 빌드 성공을 확인했다.

```text
Target is up to date
Total execution time: 0.63 seconds
```

---

### 런타임 검증 결과

- [x] 공격 trail ON 중 hit reaction interrupt 시 trail OFF를 확인했다.

- [x] 공격 trail ON 중 dead reaction interrupt 시 trail OFF를 확인했다.

- [x] collision window 중 interrupt 시 collision disabled 및 hit window closed를 확인했다.

- [x] hit context notify begin 후 interrupt 시 context clear를 확인했다.

- [x] 정상 combo attack 0-1-2에서 trail / collision / feedback timing 회귀를 확인했다.

- [x] reaction interrupt / complete feedback이 snapshot 기반 실행 경로에서 정상 재생되는 것을 확인했다.

---

## 비고

- `ClearRuntime()`은 executor 내부 cache / state 정리를 담당한다.
  
- `CleanupRuntimeEffects()`는 외부 component / weapon actor에 남아 있을 수 있는 runtime side effect 정리를 담당한다.
  
- terminal feedback은 cleanup 이후 실행되며, 필요한 request 정보는 cleanup 전에 snapshot으로 캡처한다.

---
