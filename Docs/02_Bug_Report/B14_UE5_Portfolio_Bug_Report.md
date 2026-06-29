# UE5 Portfolio Bug Report

## 제목

**B14: Blueprint 기반 Character의 UPROPERTY component reference field가 실제 native component와 불일치하는 문제**

## 날짜

**2026.06.29**

## 상태

- [x] 완료

---

## 브랜치

- `refactor/character-component-reference-di`

---

## 요약

Character component reference DI 정리 중 `BP_CEnemy_C_1`에서 `UCCombatSignalSourceComponent`, `UCCombatSignalTargetComponent` 참조가 invalid로 판정되는 문제가 발생했다.

실제 Actor component list에는 해당 component가 존재했지만, `ACEnemy`의 UPROPERTY component pointer field가 null 또는 stale 상태로 남아 일부 코드 경로가 fallback으로 빠졌다.

이번 수정에서는 Character 초기화 흐름을 다음 순서로 정리했다.

```text
PostInitializeComponents
-> RecoverReferences
-> BuildReferences
-> InjectReferences
```

`RecoverReferences()`는 invalid component reference field만 실제 Actor component list 기준으로 복구한다. 이후 `BuildReferences()`가 복구된 field를 `FCharacterComponentReferences` bundle로 묶고, `InjectReferences()`가 각 component에 동일한 reference set을 주입한다.

---

## 영향 범위

- `ACPlayer` / `ACEnemy` component reference 초기화
- `UCActionComponent` / `UCWeaponComponent` / `ACWeaponActor` 참조 주입
- `ACEnemy::TakeDamage()`의 `CombatSignalTargetComponent` 접근 경로
- Blueprint 기반 Character asset의 stale native component reference 방어

---

## 관측 로그

### WeaponComponent validation ensure

```text
Missing required UCCombatSignalSourceComponent | Owner=BP_CEnemy_C_1 | This=Weapon
```

### ActionComponent validation ensure

```text
Missing required UCCombatSignalSourceComponent | Owner=BP_CEnemy_C_1 | This=Action
```

### TakeDamage fallback

```text
[Enemy] TakeDamage Fallback | Target=BP_CEnemy_C_1 | Damage=5.000 | Reason=InvalidCombatSignalTargetComponent
```

### Recovery log

```text
[ComponentReferenceRecovery] Recovered | Owner=BP_CEnemy_C_1 | Component=CCombatSignalSourceComponent | Resolved=CombatSignalSource
[ComponentReferenceRecovery] Recovered | Owner=BP_CEnemy_C_1 | Component=CCombatSignalTargetComponent | Resolved=CombatSignalTarget
```

---

## 원인

원인은 component object 생성 실패가 아니다.

`CreateDefaultSubobject()`로 생성된 native component는 실제 Actor component list에 존재했다. 문제는 Blueprint generated class / serialized default data / inherited component metadata 적용 이후 C++ UPROPERTY component pointer field가 실제 component instance를 안정적으로 가리키지 못한 것이다.

```text
Actor actual component list
-> UCCombatSignalTargetComponent instance exists

ACEnemy::CombatSignalTargetComponent field
-> null or stale
```

이는 native component rename, UPROPERTY field rename, subobject name change, Live Coding / Hot Reload 중 구조 변경, Blueprint asset 미저장 또는 stale 상태가 겹칠 때 발생할 수 있다.

---

## 수정

- `FComponentReferenceHelper` 추가
  - `RecoverIfInvalid(...)`
  - `InjectIfValid(...)`
- `ACPlayer` / `ACEnemy`에서 local anonymous namespace helper 제거
- Character 초기화 흐름을 다음 순서로 분리

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

- `RecoverReferences()`에서 invalid field만 actual component list 기준으로 복구
- `BuildReferences()`에서 현재 field 기준으로 참조 묶음 구성
- `InjectReferences()`에서 각 component에 참조 묶음 주입
- `UCWeaponComponent`가 weapon actor 생성 후 필요한 reference를 명시적으로 전달
- `ACWeaponActor`가 `BeginPlay()`에서 owner/component를 직접 lookup하지 않고 injected reference를 사용

---

## 에셋 조치

코드 방어선은 runtime crash / invalid route를 막기 위한 것이며, stale Blueprint asset 자체를 근본적으로 정리하는 조치는 별도다.

이번 확인에서는 `BP_CEnemy`의 컴포넌트가 Details / Components 패널에 정상 표시됐지만 recovery 로그가 출력됐다. 이후 Blueprint 이름 변경 후 rebuild / compile / save를 수행하자 recovery 로그가 사라졌다.

권장 조치 순서:

```text
1. 관련 Blueprint asset 열기
2. Compile / Save
3. 필요 시 Blueprint asset rename
4. Rebuild
5. Editor restart
6. PIE에서 [ComponentReferenceRecovery] 로그 재확인
7. 그래도 반복되면 새 Blueprint로 재생성 후 설정 이관
```

---

## 검증 결과

- `git diff --check` 통과
- `PortfolioEditor Win64 Development` 빌드 성공
- PIE 전투 사이클 정상 동작 확인
- 기존 `BP_CEnemy`는 에셋 리네임 / 리빌드 후 recovery 로그 미출력 확인
- 새 임시 Enemy BP에서 recovery 로그가 발생하지 않아 C++ native subobject 선언 자체는 정상으로 판단

---

## 후속 방지 기준

- native component reference를 담는 UPROPERTY field와 actual component list가 항상 일치한다고 가정하지 않는다.
- `FindComponentByClass`는 일반 dependency wiring 방식이 아니라 stale Blueprint/native component reference recovery 방어선으로 제한한다.
- native component rename / type change / subobject name change 후에는 Blueprint asset load / compile / save / PIE validation을 함께 수행한다.
- `[ComponentReferenceRecovery]` 로그가 반복 출력되면 코드 복구는 성공한 것이지만, 관련 Blueprint asset의 serialized mapping 상태를 별도로 정리한다.

---

## 관련 문서

- `Docs/02_Bug_Report/B13_UE5_Portfolio_Bug_Report.md`
- `Docs/06_notes/N07_Unreal_Native_Component_Rename_And_Blueprint_Reference_Note.md`
- `Docs/06_notes/N10_Component_Reference_Validation_Policy_Note.md`
- `Docs/06_notes/N11_Unreal_Blueprint_Native_Component_Reference_Mismatch_Note.md`
