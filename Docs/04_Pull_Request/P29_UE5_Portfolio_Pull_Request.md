# UE5 Portfolio Pull Request

## 제목

**P29: Character Component 참조 주입 / 복구**

## 날짜

**2026.06.29**

## 상태

- [x] 완료

---

## 브랜치

- `refactor/character-component-reference-di`

---

## 커밋

```text
46d1d793 refactor(feedback): inject required references for player feedback
41ef737b refactor(action): inject required references for action executors
7e086873 refactor(reaction): inject required references for reaction executors
7c0719aa refactor(core): share required reference validation entry
3e18b352 refactor(character): simplify component reference injection calls
35b9495c refactor(character): recover component references before injection
49a38a81 docs(unreal): document blueprint component reference recovery
1ac69be3 fix(asset): refresh enemy blueprint and test map
```

---

## 요약

이번 PR은 Character가 소유한 component reference를 owner 쪽에서 명시적으로 구성하고, 각 component / executor / weapon actor에 주입하는 기준을 정리한다.

핵심 흐름은 다음과 같다.

```text
ACPlayer / ACEnemy
-> RecoverReferences
-> BuildReferences
-> InjectReferences
-> InitializeReferences
-> ValidateRequiredComponentReferences
```

기존에는 여러 component가 owner나 다른 component를 직접 찾거나, 생성 / 초기화 타이밍에 따라 암시적으로 참조를 확보했다. 이번 PR에서는 `FCharacterComponentReferences`를 Character 기준 참조 묶음으로 두고, 필요한 객체가 이 묶음에서 자기 책임에 필요한 reference만 선택해 저장하도록 정리했다.

또한 Blueprint asset의 stale native component reference 문제를 런타임에서 감지 / 복구하는 방어선을 추가했다. 이 recovery는 일반 dependency wiring 방식이 아니라, Blueprint serialized component reference와 실제 Actor component list가 어긋난 경우를 위한 안전장치다.

---

## 변경 배경

Component dependency가 늘어나면서 다음 문제가 있었다.

```text
- component 내부에서 owner / sibling component를 직접 lookup하는 책임이 섞임
- required reference 누락 시 어떤 dependency가 빠졌는지 설명이 부족함
- Action / Reaction executor와 Feedback / WeaponActor가 각자 다른 방식으로 reference를 확보함
- native component rename / Blueprint stale mapping 상황에서 C++ UPROPERTY component field가 invalid가 될 수 있음
```

이번 PR은 “프로젝트 전체 lookup 제거”가 아니라, Character가 소유하고 조립할 수 있는 reference를 명시적으로 구성하는 데 집중한다.

---

## 변경 범위

### 1. Character 기준 참조 초기화 흐름

`ACPlayer` / `ACEnemy`의 component reference 초기화 흐름을 다음 순서로 정리했다.

```cpp
void ACEnemy::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    RecoverReferences();

    FCharacterComponentReferences references;
    BuildReferences(references);
    InjectReferences(references);
}
```

역할:

```text
RecoverReferences
-> invalid component reference field를 actual component list 기준으로 복구

BuildReferences
-> 현재 Character field 기준으로 FCharacterComponentReferences 구성

InjectReferences
-> 동일한 참조 묶음을 각 component에 주입
```

### 2. Component 참조 묶음

`FCharacterComponentReferences`를 Character가 소유한 component reference 묶음으로 사용한다.

기본 배열 순서는 다음 기준을 따른다.

```text
Movement
Weapon
State
Health / Resource
Defense
Overlay
CombatSignal
Orchestrator
Execution / Action / Reaction
Feedback
```

Player와 Enemy의 component field, build, inject 흐름도 가능한 한 이 순서로 정렬했다.

### 3. 필수 참조 검증

공용 validation entry를 추가해 필수 reference 누락 로그 형식을 통일했다.

```text
Missing required <ReferenceLabel> | Owner=<Owner> | This=<Context>
```

대상 component는 `InitializeReferences(...)`에서 필요한 reference를 저장한 뒤 `ValidateRequiredComponentReferences()`로 필수 dependency를 검증한다.

### 4. Action / Reaction executor 참조 주입

Action / Reaction executor도 component와 같은 reference 묶음을 통해 필요한 owner / component reference를 주입받도록 정리했다.

```text
ActionComponent
-> BuildActionExecutorReferences
-> UCAction::InitializeReferences

ReactionComponent
-> BuildReactionExecutorReferences
-> UCReaction::InitializeReferences
```

이를 통해 executor 내부에서 owner component를 직접 찾는 흐름을 줄이고, active executor가 사용하는 reference source를 component DI 흐름과 맞췄다.

### 5. Feedback component 참조 주입

Action / Reaction / Hit / Player feedback 계열의 reference 주입 기준을 정리했다.

```text
ActionFeedbackComponent
ReactionFeedbackComponent
HitFeedbackComponent
PlayerFeedbackComponent
```

Player feedback은 PlayerController 소유 component이므로 `APlayerController` 기준 `InitializeReferences(...)`를 유지한다.

### 6. WeaponActor 참조 주입

`UCWeaponComponent`가 weapon actor를 생성한 뒤 필요한 reference를 명시적으로 전달한다.

```text
UCWeaponComponent
-> BuildWeaponActorReferences
-> ACWeaponActor::InitializeReferences
```

`ACWeaponActor`는 owner / combat signal source를 `BeginPlay()`에서 다시 lookup하지 않고 injected reference를 사용한다.

### 7. Blueprint stale component 참조 복구

Blueprint 기반 Character에서 실제 Actor component list에는 component가 존재하지만, C++ UPROPERTY component pointer field가 null 또는 stale 상태가 되는 문제를 확인했다.

이를 위해 `FComponentReferenceHelper`를 추가했다.

```text
FComponentReferenceHelper::RecoverIfInvalid
FComponentReferenceHelper::InjectIfValid
```

recovery가 발생하면 다음 로그를 남긴다.

```text
[ComponentReferenceRecovery] Recovered | Owner=... | Component=... | Resolved=...
```

이 로그는 정상 흐름 로그가 아니라 stale Blueprint/native component reference mismatch가 복구됐다는 신호다.

### 8. Blueprint asset 갱신

`BP_CEnemy`에서 recovery 로그가 반복 출력되는 상황을 확인했고, Blueprint 이름 변경 후 rebuild / compile / save를 통해 stale mapping이 해소되는 것을 확인했다.

관련 에셋 갱신을 반영했다.

```text
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/00_UnitTest/TestRoom.umap
```

---

## 주요 파일

```text
Source/Portfolio/Type/CCharacterComponentReferenceStructure.h
Source/Portfolio/Core/Debug/FReferenceValidation.h
Source/Portfolio/Core/Debug/FComponentReferenceHelper.h
Source/Portfolio/Character/Player/CPlayer.h
Source/Portfolio/Character/Player/CPlayer.cpp
Source/Portfolio/Character/Enemy/CEnemy.h
Source/Portfolio/Character/Enemy/CEnemy.cpp
Source/Portfolio/Component/CActionComponent.h
Source/Portfolio/Component/CActionComponent.cpp
Source/Portfolio/Component/CReactionComponent.h
Source/Portfolio/Component/CReactionComponent.cpp
Source/Portfolio/Component/CActionOrchestratorComponent.h
Source/Portfolio/Component/CActionOrchestratorComponent.cpp
Source/Portfolio/Component/CReactionOrchestratorComponent.h
Source/Portfolio/Component/CReactionOrchestratorComponent.cpp
Source/Portfolio/Component/CWeaponComponent.h
Source/Portfolio/Component/CWeaponComponent.cpp
Source/Portfolio/Weapon/CWeaponActor.h
Source/Portfolio/Weapon/CWeaponActor.cpp
Docs/02_Bug_Report/B14_UE5_Portfolio_Bug_Report.md
Docs/06_notes/N10_Component_Reference_Validation_Policy_Note.md
Docs/06_notes/N11_Unreal_Blueprint_Native_Component_Reference_Mismatch_Note.md
```

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

### PIE

```text
TestRoom PIE
```

확인 내용:

```text
- 전투 사이클 정상 동작
- CombatSignalSource / CombatSignalTarget invalid fallback 해소
- BP_CEnemy asset 갱신 이후 ComponentReferenceRecovery 로그 미출력 확인
```

---

## 제외 범위

이번 PR은 Character가 소유한 component reference의 DI / recovery를 다룬다. 다음 runtime lookup 영역은 별도 정책 작업으로 분리한다.

```text
- Notify / NotifyState의 FindComponentByClass 경로
- AnimInstance의 owner component cache 정책
- BehaviorTree Service / Decorator의 pawn component query 정책
- CombatSignalSource의 동적 target component resolve 정책
```

남은 lookup은 모두 제거 대상이라고 단정하지 않는다. 다음 작업에서 runtime boundary별로 허용 lookup, DI 전환 대상, cache 유지 대상을 분류한다.

---

## 후속 작업

권장 후속 브랜치:

```text
refactor/runtime-component-lookup-policy
```

권장 범위:

```text
1. Notify / NotifyState component lookup 정책 정리
2. AnimInstance component cache 기준 정리
3. AI BehaviorTree component query 기준 정리
4. CombatSignal 동적 target lookup 허용 기준 문서화
5. Runtime component lookup 정책 문서 추가
```
