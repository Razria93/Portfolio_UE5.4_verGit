# UE5 Portfolio Bug Report (KR)

## 제목

**M05-B10: Action trail / collision runtime effect가 hit 또는 dead interrupt 이후 정리되지 않는 문제**

### Date

- **2026.06.04**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/orchestration-refactor`

---

## 요약

- 공격 액션 중 trail, collision, hit context가 열린 상태에서 hit reaction 또는 dead reaction으로 action이 interrupt되면 trail이 꺼지지 않는 문제가 발생함.

- 원인은 runtime effect 정리가 animation notify end 또는 feedback end 흐름에 의존하고 있었기 때문임.

- montage가 interrupt되면 notify end가 항상 의도한 순서로 실행된다고 보장할 수 없으므로, executor 종료 경로에서 외부 runtime state를 명시적으로 정리해야 함.

- 이를 위해 `CleanupRuntimeEffects()`를 추가하고, feedback request 생성과 실행을 `BuildFeedbackRequest()` / `PlayFeedbackRequest()`로 분리함.

---

## 환경

- Engine: Unreal Engine 5.4

- Branch:
  - `feature/orchestration-refactor`

### Related Code

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

1. sword combo attack 또는 trail feedback이 켜지는 action을 실행함.

2. action trail 또는 collision window가 열린 상태에서 캐릭터가 hit reaction으로 interrupt되도록 함.

3. 같은 조건에서 dead reaction으로 interrupt되는 케이스도 확인함.

4. interrupt 이후 weapon trail, collision, hit context가 정상적으로 닫히는지 확인함.

---

## 기대 결과

- active action이 hit 또는 dead reaction으로 interrupt되면 action runtime effect가 즉시 정리되어야 함.

- weapon trail은 꺼져야 함.

- weapon collision은 disabled 상태로 돌아가야 함.

- apply damage hit window는 closed 처리되어야 함.

- pushed hit context는 clear되어야 함.

- interrupt / complete feedback은 cleanup에 의해 제거되지 않고 정상 재생되어야 함.

---

## 실제 결과

- action trail이 켜진 상태에서 hit 또는 dead interrupt가 발생하면 trail이 꺼지지 않는 현상이 발생함.

- collision notify end 또는 hit context notify end가 interrupt 경로에서 정상 실행되지 않을 경우, collision / hit context도 stale state로 남을 수 있는 구조였음.

- 기존 `ClearRuntime()`은 executor 내부 cache만 정리하고 외부 component / weapon actor runtime state는 정리하지 않았음.

---

## 원인

- trail off, collision disabled, hit context clear가 notify end 또는 feedback data에 의존하고 있었음.

- `UCAction::Stop()`과 `UCReaction::Stop()`은 montage stop 이후 runtime effect cleanup을 명시적으로 수행하지 않았음.

- 기존 `RequestFeedback()`는 feedback request 생성과 실행을 한 함수 안에서 처리했기 때문에, terminal feedback을 `ClearRuntime()` 이후로 내리기 어려웠음.

- 그 결과 cleanup 순서와 terminal feedback 실행 순서가 명확하게 분리되지 않았음.

---

## 수정

다음 구조로 정리함.

```text
Capture terminal feedback request / event payload
-> StopMontage
-> CleanupRuntimeEffects
-> ClearRuntime
-> PlayFeedbackRequest
-> EmitEvent / HandleFinished
```

### Action

- `UCAction::CleanupRuntimeEffects()` 추가함.

- `UCWeaponComponent::ClearRuntimeWeaponState()`를 통해 hit context와 collision을 정리함.

- `UCActionFeedbackComponent::ClearRuntimeFeedback()`을 통해 trail을 강제 OFF함.

- `RequestFeedback()`를 제거하고 모든 action feedback 호출을 `BuildFeedbackRequest()` + `PlayFeedbackRequest()`로 통일함.

- `Stop()` / `Complete()`는 feedback request와 action index를 먼저 캡처한 뒤 cleanup / clear 이후 feedback과 event를 실행함.

### Reaction

- `UCReaction::CleanupRuntimeEffects()` 추가함.

- `UCReactionFeedbackComponent::ClearRuntimeFeedback()` 추가함.

- 현재 reaction feedback cleanup은 no-op이지만, 향후 loop VFX/SFX가 추가될 때 동일한 종료 지점에서 정리할 수 있도록 hook을 맞춤.

- `RequestFeedback()`를 제거하고 reaction feedback 호출도 `BuildFeedbackRequest()` + `PlayFeedbackRequest()`로 통일함.

---

## 컴파일 검증 결과

- `RequestFeedback` 잔여 참조 없음 확인함.

- `git diff --check` 통과함.

- `PortfolioEditor Win64 Development` 빌드 성공함.

```text
Target is up to date
Total execution time: 0.63 seconds
```

---

## 런타임 검증 목록

- [x] 공격 trail ON 중 hit reaction interrupt 시 trail OFF 확인 필요함.

- [x] 공격 trail ON 중 dead reaction interrupt 시 trail OFF 확인 필요함.

- [x] collision window 중 interrupt 시 collision disabled 및 hit window closed 확인 필요함.

- [x] hit context notify begin 후 interrupt 시 context clear 확인 필요함.

- [x] 정상 combo attack 0-1-2에서 trail / collision / feedback 타이밍 회귀 확인 필요함.

- [x] reaction interrupt / complete feedback이 snapshot 기반 실행 경로에서 정상 재생되는지 확인 필요함.

---

## Notes

- `ClearRuntime()`은 executor 내부 cache/state 정리만 담당함.

- `CleanupRuntimeEffects()`는 외부 component / weapon actor에 남아 있을 수 있는 runtime side effect 정리만 담당함.

- terminal feedback은 cleanup 이후 실행되며, 필요한 request 정보는 cleanup 전에 snapshot으로 캡처함.

---
