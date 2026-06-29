# N10. Component Reference Validation Policy Note

## 목적

Character가 소유한 component reference를 어떤 기준으로 주입하고 검증할지 정리한다.

이 문서는 프로젝트 전체의 모든 runtime lookup을 제거하는 문서가 아니다. 이번 기준은 `ACPlayer` / `ACEnemy`가 소유한 native component reference를 명시적으로 구성하고, Character component / executor / weapon actor에 안정적으로 전달하는 데 집중한다.

---

## 핵심 원칙

```text
Character-owned component reference
-> owner-side에서 구성한다.

Required dependency
-> InitializeReferences에서 주입받고 ValidateRequiredComponentReferences에서 검증한다.

Optional dependency
-> 사용 시점에 IsValid로 방어한다.

Recovery lookup
-> 일반 wiring이 아니라 stale Blueprint/native component reference 방어선으로만 사용한다.
```

---

## 초기화 흐름

Character 초기화 흐름은 다음 순서를 따른다.

```text
PostInitializeComponents
-> RecoverReferences
-> BuildReferences
-> InjectReferences
```

예시:

```cpp
void ACPlayer::PostInitializeComponents()
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
-> invalid component reference field를 actual component list 기준으로 복구한다.

BuildReferences
-> 현재 Character field 기준으로 FCharacterComponentReferences를 구성한다.

InjectReferences
-> 각 component에 동일한 참조 묶음을 전달한다.
```

---

## FCharacterComponentReferences

`FCharacterComponentReferences`는 Character가 소유한 component reference 묶음이다.

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

Player / Enemy의 field 배치, build, inject 흐름은 가능한 한 이 순서를 따른다.

각 component는 bundle 전체를 받아도 자기 책임에 필요한 reference만 `_Injected` field에 저장한다.

---

## Required Reference Validation

필수 reference는 `InitializeReferences(...)` 직후 검증한다.

```cpp
void UCActionComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
    OwnerCharacter_Injected = InReferences.OwnerCharacter;
    WeaponComp_Injected = InReferences.WeaponComponent;

    ValidateRequiredComponentReferences();
}
```

검증 메시지는 공용 helper를 사용해 다음 형식을 유지한다.

```text
Missing required <ReferenceLabel> | Owner=<Owner> | This=<Context>
```

필수 reference 누락은 개발 중 즉시 드러나야 하므로 `ensureMsgf` 기반으로 기록한다. 단 public request 경로는 기존처럼 `InvalidComponent` / reject result로 방어한다.

---

## Injected / Cached Naming

주입받은 장기 reference는 `_Injected` suffix를 사용한다.

```text
OwnerCharacter_Injected
WeaponComp_Injected
CombatSignalSourceComp_Injected
```

런타임 상태나 마지막 처리 결과처럼 계산 / 이벤트 흐름에서 갱신되는 값은 `_Cached` suffix를 사용한다.

```text
ActiveData_Cached
ActiveMontage_Cached
LastWeaponContext_Cached
```

---

## Recovery Lookup

`FComponentReferenceHelper::RecoverIfInvalid(...)`는 일반 dependency wiring 방식이 아니다.

용도:

```text
Blueprint asset에는 native component가 존재하지만
C++ UPROPERTY component pointer field가 null 또는 stale 상태일 때
actual component list 기준으로 field를 복구한다.
```

recovery가 발생하면 다음 로그를 남긴다.

```text
[ComponentReferenceRecovery] Recovered | Owner=... | Component=... | Resolved=...
```

이 로그가 보이면 runtime 방어선은 작동한 것이지만, 관련 Blueprint asset의 serialized mapping 상태는 별도로 정리해야 한다.

권장 에셋 조치:

```text
1. Blueprint asset 열기
2. Compile / Save
3. 필요 시 Blueprint asset rename
4. Rebuild
5. Editor restart
6. PIE에서 recovery 로그 반복 여부 확인
```

---

## FindComponentByClass 기준

`FindComponentByClass` / `GetComponentByClass`는 Character-owned required dependency wiring의 기본 방식으로 사용하지 않는다.

허용되는 경우:

```text
- stale Blueprint/native component reference recovery
- dynamic target actor에서 target component resolve
- Notify / AnimInstance / AI 같은 runtime boundary에서 후속 정책으로 허용된 경우
```

이번 PR에서는 Notify / AnimInstance / AI lookup 정책을 확정하지 않는다. 해당 범위는 `runtime component lookup policy` 후속 작업에서 분류한다.

---

## 현재 적용 범위

이번 기준은 다음 축에 적용됐다.

```text
ACPlayer / ACEnemy
ActionOrchestratorComponent
ReactionOrchestratorComponent
ActionComponent / ReactionComponent
Action executor / Reaction executor
WeaponComponent / WeaponActor
ActionFeedbackComponent
ReactionFeedbackComponent
HitFeedbackComponent
PlayerFeedbackComponent
CombatSignalSourceComponent
CombatSignalTargetComponent
Movement / State / Health / Defense / Overlay component
```

---

## 제외 범위

다음 항목은 현재 PR의 DI 전환 완료 범위가 아니다.

```text
Notify / NotifyState component lookup
AnimInstance component cache
BehaviorTree Service / Decorator component query
CombatSignalSource dynamic target resolution policy
```

이 항목들은 같은 상위 주제인 runtime component lookup policy로 묶어 후속 브랜치에서 정리한다.

---

## 관련 문서

- `Docs/02_Bug_Report/B14_UE5_Portfolio_Bug_Report.md`
- `Docs/06_notes/N11_Unreal_Blueprint_Native_Component_Reference_Mismatch_Note.md`
- `Docs/04_Pull_Request/P29_UE5_Portfolio_Pull_Request.md`
