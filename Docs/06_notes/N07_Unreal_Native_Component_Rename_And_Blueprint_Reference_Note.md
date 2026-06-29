# N07 Unreal Native Component Rename And Blueprint Reference Note

## 목적

Unreal Engine에서 C++ native component를 생성하고 Blueprint asset이 이를 상속 / 저장 / 복구하는 흐름을 정리한다.

특히 native component rename 이후 다음 문제가 왜 발생할 수 있는지 이해하는 것을 목표로 한다.

```text
- Blueprint Details panel에서 component가 사라짐
- C++ 수정 후 Blueprint component data가 초기화됨
- Blueprint에는 component가 보이지만 C++ UPROPERTY member pointer가 nullptr 또는 invalid 상태가 됨
- class / property redirect를 넣었는데도 일부 data나 reference가 기대대로 복구되지 않음
```

---

## 시작 배경

W04-05 `Combat Signal Component Rename`에서 다음 리네임을 수행했다.

```text
UCApplyDamageComponent
-> UCCombatSignalSourceComponent

UCTakeDamageComponent
-> UCCombatSignalTargetComponent

ApplyDamageComponent
-> CombatSignalSourceComponent

TakeDamageComponent
-> CombatSignalTargetComponent

"ApplyDamage"
-> "CombatSignalSource"

"TakeDamage"
-> "CombatSignalTarget"
```

리네임 이후 Player -> Enemy 공격에서 Enemy `TakeDamage()`는 호출되었지만 target-side `CombatSignalTargetOutcome`이 출력되지 않는 현상이 있었다.

디버깅 결과 Actor에는 `UCCombatSignalTargetComponent` instance가 존재했지만, `ACEnemy::CombatSignalTargetComponent` C++ member pointer가 해당 instance를 즉시 가리키지 못하는 상태로 판단했다.

---

## 기본 개념

### Native Class

C++에서 정의한 `UCLASS`다.

예:

```cpp
UCLASS()
class PORTFOLIO_API ACEnemy : public ACharacter
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, Category = "CombatSignal")
    class UCCombatSignalTargetComponent* CombatSignalTargetComponent;
};
```

Native class의 constructor는 class default object, Blueprint generated class, runtime actor instance 구성에 모두 영향을 준다.

### CDO

CDO는 Class Default Object다.

Unreal은 class마다 기본 객체를 만들고, 이 객체를 기준으로 기본 property 값과 native default subobject를 관리한다.

```text
Native CDO
-> C++ constructor 실행 결과를 가진 기본 객체

Blueprint Generated Class CDO
-> Native CDO를 기반으로 Blueprint override / template data를 반영한 기본 객체

Runtime Actor Instance
-> CDO와 construction 흐름을 기반으로 만들어지는 실제 actor
```

### Native Default Subobject

`CreateDefaultSubobject`로 생성한 subobject다.

```cpp
CombatSignalTargetComponent =
    CreateDefaultSubobject<UCCombatSignalTargetComponent>(TEXT("CombatSignalTarget"));
```

여기서 중요한 식별자는 두 가지다.

```text
1. C++ member property
   CombatSignalTargetComponent

2. Subobject name
   "CombatSignalTarget"
```

C++ member property는 native class의 UPROPERTY field다.

Subobject name은 actor / CDO 안에서 subobject를 식별하는 `FName`이다.

둘은 비슷해 보이지만 같은 것이 아니다.

---

## Blueprint가 Native Component를 보관하는 방식

Blueprint는 C++ class를 parent로 삼을 수 있다.

이때 parent C++ class가 가진 native component는 Blueprint에서 inherited component처럼 보인다.

Blueprint asset은 다음 성격의 정보를 저장할 수 있다.

```text
- parent class 정보
- inherited native component에 대한 override data
- component template reference
- exposed property override
- editor graph / node / pin reference
- serialized object path / name reference
```

따라서 C++ constructor에서 component를 만들었다고 해서 Blueprint가 매번 완전히 새로 읽기만 하는 것은 아니다.

Blueprint는 기존 component template과 override data를 자기 asset 안에 저장하고, 다음 load 때 parent class의 native component와 다시 맞춰야 한다.

---

## Rename이 위험한 이유

Native component rename은 보통 하나의 이름만 바뀌지 않는다.

이번 작업처럼 다음 세 가지가 동시에 바뀌면 위험이 커진다.

```text
1. Component class name
   UCApplyDamageComponent
   -> UCCombatSignalSourceComponent

2. C++ UPROPERTY field name
   ApplyDamageComponent
   -> CombatSignalSourceComponent

3. CreateDefaultSubobject name
   "ApplyDamage"
   -> "CombatSignalSource"
```

Unreal은 load 과정에서 redirect를 적용할 수 있지만, redirect마다 복구하는 대상이 다르다.

```text
ClassRedirect
-> class path rename 복구

PropertyRedirect
-> reflected property name rename 복구

StructRedirect
-> reflected struct type rename 복구

EnumRedirect
-> reflected enum type rename 복구
```

하지만 native default subobject의 instance identity는 class name 하나만으로 결정되지 않는다.

Subobject name, outer path, archetype/template 관계, Blueprint가 저장한 inherited component override가 함께 얽힌다.

즉 class redirect와 property redirect가 있어도 다음이 항상 보장되는 것은 아니다.

```text
Blueprint가 보관하던 old component template
-> new native subobject
-> new C++ UPROPERTY member pointer
```

이 세 연결이 모두 완벽하게 복구되어야 한다.

---

## 이번 현상의 가능한 흐름

관측된 현상:

```text
Enemy Actor에는 UCCombatSignalTargetComponent instance가 존재한다.
하지만 ACEnemy::CombatSignalTargetComponent member pointer는 유효하지 않다.
FindComponentByClass<UCCombatSignalTargetComponent>()로 찾으면 정상 instance가 반환된다.
```

가능한 흐름은 다음과 같다.

```text
1. C++ constructor는 새 이름의 native subobject를 생성한다.

2. Blueprint asset은 이전 이름 / 이전 component template / 이전 property override를 가지고 있다.

3. Load 과정에서 class redirect와 property redirect가 일부 reference를 새 이름으로 복구한다.

4. Actor의 component list에는 renamed component instance가 존재한다.

5. 그러나 C++ UPROPERTY member pointer가 그 component instance를 가리키는 연결은 깨져 있을 수 있다.

6. Runtime에서 CombatSignalTargetComponent를 직접 사용하면 invalid가 된다.

7. Actor에 실제 component는 있으므로 FindComponentByClass로 다시 찾으면 복구된다.
```

핵심은 다음이다.

```text
Component가 Actor에 존재하는 것
!=
C++ UPROPERTY member pointer가 그 component를 가리키는 것
```

Blueprint Details panel에 component가 보이는 것도 충분조건이 아니다.

Editor UI는 component tree / template 정보를 보여줄 수 있고, C++ member pointer 유효성은 runtime object reference 문제다.

---

## 어느 단계에서 어긋났는가

이번 문제는 collision, damage spec, `TakeDamage()` 호출, HP 계산 단계에서 발생한 문제가 아니다.

로그상 Player 공격은 다음 지점까지 정상 도달했다.

```text
Source overlap
-> CombatSignalSource request
-> Enemy TakeDamage entry
```

어긋난 지점은 `TakeDamage()` 내부에서 target-side 처리를 위임할 C++ member pointer를 사용하는 순간이다.

```text
Runtime Actor
-> component list에는 UCCombatSignalTargetComponent instance가 있음
-> ACEnemy::CombatSignalTargetComponent member pointer는 invalid
-> member pointer 기반 호출 실패
```

즉 Unreal의 큰 흐름으로 보면 다음 구간의 불일치다.

```text
Blueprint asset load / native component reconstruction
-> runtime actor component instance 구성
-> reflected UPROPERTY member pointer 연결
```

이 중 runtime actor component instance 구성은 성공했지만, C++ 멤버 포인터가 해당 instance를 안정적으로 가리키는 단계가 어긋난 것으로 본다.

`FindComponentByClass`가 문제를 해결한 이유도 여기에 있다.

```text
FindComponentByClass
-> Actor의 component list를 조회
-> 이미 존재하는 UCCombatSignalTargetComponent instance 반환
-> 새 component를 생성하지 않음
-> C++ member pointer만 다시 연결
```

따라서 이번 수정은 gameplay rule 변경이 아니라 native component rename 이후 runtime reference recovery에 가깝다.

---

## Details Panel에서 Component가 사라지는 경우

C++ 수정 후 Details panel에서 component가 사라지는 경우는 보통 다음 위험과 관련된다.

```text
- CreateDefaultSubobject name 변경
- UPROPERTY specifier 변경
- UPROPERTY field rename
- component class rename
- parent class 변경
- Hot Reload / Live Coding 후 Blueprint generated class가 stale 상태가 됨
- Blueprint asset이 old template reference를 보유한 상태에서 parent native layout이 크게 바뀜
```

이 경우 Blueprint가 parent native component를 old name / old template 기준으로 찾지 못하면 inherited component 표시나 override data가 어긋날 수 있다.

---

## Data가 날아가는 경우

Blueprint에 입력한 data가 사라지는 경우는 다음과 관련된다.

```text
- property name 변경 후 PropertyRedirect 누락
- struct / enum rename 후 StructRedirect / EnumRedirect 누락
- UPROPERTY type 변경
- container key type 변경
- component template이 새 instance로 인식됨
- old component template override가 new component template에 적용되지 않음
```

특히 `TMap`, `TArray`, custom struct, enum을 Blueprint에서 편집하는 경우 type path와 property path가 모두 중요하다.

이름만 비슷해도 reflected path가 바뀌면 Unreal은 다른 property / 다른 type으로 볼 수 있다.

---

## CoreRedirect의 역할과 한계

CoreRedirect는 asset load 시 old reflected path를 new reflected path로 매핑하기 위한 장치다.

이번 브랜치에서는 다음 redirect를 추가했다.

```ini
+ClassRedirects=(OldName="/Script/Portfolio.CApplyDamageComponent",NewName="/Script/Portfolio.CCombatSignalSourceComponent")
+ClassRedirects=(OldName="/Script/Portfolio.CTakeDamageComponent",NewName="/Script/Portfolio.CCombatSignalTargetComponent")

+PropertyRedirects=(OldName="/Script/Portfolio.CEnemy.ApplyDamageComponent",NewName="/Script/Portfolio.CEnemy.CombatSignalSourceComponent")
+PropertyRedirects=(OldName="/Script/Portfolio.CEnemy.TakeDamageComponent",NewName="/Script/Portfolio.CEnemy.CombatSignalTargetComponent")
+PropertyRedirects=(OldName="/Script/Portfolio.CPlayer.ApplyDamageComponent",NewName="/Script/Portfolio.CPlayer.CombatSignalSourceComponent")
+PropertyRedirects=(OldName="/Script/Portfolio.CPlayer.TakeDamageComponent",NewName="/Script/Portfolio.CPlayer.CombatSignalTargetComponent")
```

이 redirect는 필요하다.

하지만 다음까지 모두 보장한다고 보면 안 된다.

```text
- native subobject FName rename의 모든 영향
- Blueprint inherited component override의 모든 재연결
- stale Blueprint generated class의 즉시 갱신
- C++ member pointer와 component instance의 runtime 연결
- Blueprint editor에 보이는 component tree와 runtime pointer의 일치
```

따라서 redirect 후에도 에디터 load / compile / save, runtime pointer 검증, PIE validation이 필요하다.

---

## 현재 브랜치의 해결 방식

이번 브랜치에서는 문제 범위를 rename 대상 component 두 개로 제한했다.

```cpp
void ACEnemy::ResolveComponentReferences()
{
    if (!IsValid(CombatSignalSourceComponent))
    {
        CombatSignalSourceComponent = FindComponentByClass<UCCombatSignalSourceComponent>();
    }

    if (!IsValid(CombatSignalTargetComponent))
    {
        CombatSignalTargetComponent = FindComponentByClass<UCCombatSignalTargetComponent>();
    }
}
```

이 함수는 `BeginPlay()`에서 한 번 호출한다.

의미:

```text
- 새 component를 생성하지 않는다.
- Actor에 이미 붙어 있는 component instance를 찾는다.
- C++ member pointer를 해당 instance로 다시 연결한다.
- runtime combat flow에서는 member pointer를 계속 사용한다.
```

이 방식은 모든 component cache 정책을 바꾸지 않고, rename migration으로 발생한 문제만 좁게 보정한다.

---

## 일반화 기준

이번 브랜치에서 바로 모든 component를 `FindComponentByClass`로 바꾸지는 않는다.

이유:

```text
- CreateDefaultSubobject 기반 native component 소유 구조는 유지하는 것이 맞다.
- C++ UPROPERTY member pointer는 명시적 의존성을 보여준다.
- 모든 component를 BeginPlay에서 재탐색하면 이번 rename 브랜치의 범위를 넘어선다.
- 전체 cache validation 정책은 별도 기준이 필요하다.
```

대신 후속 브랜치에서는 다음을 검토할 수 있다.

```text
- Player / Enemy 공통 component reference validation helper
- 필수 component는 check / ensure / fallback 중 어떤 정책을 쓸지
- optional component와 required component 구분
- Blueprint rename migration 시 일시적 recovery 코드 기준
- resave 후 recovery code를 유지할지 제거할지
```

### Recovery code 유지 / 제거 기준

`ResolveComponentReferences()` 같은 recovery 코드는 기본 dependency wiring 정책이 아니다. native component rename 직후 Blueprint serialized reference와 C++ member pointer가 어긋난 경우를 복구하기 위한 일시적 migration hook이다.

유지할 수 있는 경우:

```text
rename 직후 Blueprint asset load / compile / save가 끝나지 않음
PIE에서 C++ UPROPERTY member pointer invalid가 재현됨
Actor component list에는 renamed component instance가 존재함
대표 flow 검증 전이라 regression safety net이 필요함
```

제거할 수 있는 경우:

```text
관련 Blueprint asset과 map을 load / compile / save함
Details panel에서 inherited component가 정상 표시됨
C++ member pointer runtime 유효성을 확인함
대표 gameplay flow가 정상 동작함
rename migration 목적 외에 상시 wiring 책임으로 쓰이지 않음
```

향후 비슷한 문제가 생기면 `ResolveComponentReferences`처럼 일반 wiring처럼 보이는 이름보다 다음처럼 migration 의도가 드러나는 이름을 우선한다.

```text
RecoverRenamedComponentReferences
RecoverNativeComponentRenameReferences
```

이 hook은 다음 원칙을 따른다.

```text
rename 대상 component만 좁게 복구한다.
새 component를 생성하지 않고 Actor에 이미 붙은 instance만 찾는다.
FindComponentByClass를 상시 dependency injection 방식으로 일반화하지 않는다.
asset migration과 runtime 검증이 끝난 뒤 제거 여부를 다시 판단한다.
```

---

## Native Component Rename 체크리스트

```text
1. class name 변경 여부 확인
2. UPROPERTY field name 변경 여부 확인
3. CreateDefaultSubobject FName 변경 여부 확인
4. Blueprint editable data가 있는 component인지 확인
5. ClassRedirect / PropertyRedirect 필요 여부 확인
6. StructRedirect / EnumRedirect 필요 여부 확인
7. Blueprint asset load / compile / save
8. Details panel component 표시 확인
9. C++ member pointer runtime 유효성 확인
10. Actor component list에 실제 component instance가 존재하는지 확인
11. PIE에서 대표 flow 검증
```

---

## 이번 브랜치에서 확인한 기준

```text
- class / property redirect는 필요하지만 충분조건으로 보지 않는다.
- Blueprint에 component가 표시되어도 C++ member pointer 유효성을 별도로 확인한다.
- rename 대상 component는 BeginPlay에서 기존 component instance를 FindComponentByClass로 재연결할 수 있다.
- 이 복구는 새 component를 생성하는 것이 아니라, 이미 Actor에 붙은 component instance를 C++ 멤버 포인터에 다시 연결하는 것이다.
- rename recovery hook은 migration 안정화 이후 제거할 수 있으며, 기본 component reference 정책으로 일반화하지 않는다.
- 모든 component cache 정책 변경은 별도 브랜치로 분리한다.
```

---

## 관련 문서

- `Docs/02_Bug_Report/B13_UE5_Portfolio_Bug_Report.md`
- `Docs/04_Pull_Request/P24_UE5_Portfolio_Pull_Request.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_05_Combat_Signal_Component_Rename.md`
