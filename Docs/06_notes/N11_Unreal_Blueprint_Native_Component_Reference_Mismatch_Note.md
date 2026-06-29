# N11. Unreal Blueprint Native Component Reference Mismatch Note

## 목적

Unreal Engine에서 C++ native component와 Blueprint generated class를 함께 사용할 때 다음 상황이 왜 발생할 수 있는지 정리한다.

```text
Actor에는 component가 실제로 존재한다.
하지만 C++ UPROPERTY component pointer field는 null 또는 stale 상태다.
FindComponentByClass로는 component를 찾을 수 있다.
```

이 문서는 특정 기능 구현 문서가 아니라, Unreal object / property / Blueprint asset 구조를 이해하기 위한 기준 문서다.

---

## 핵심 결론

이 문제는 `CreateDefaultSubobject()`가 component 생성을 실패한 문제가 아니다.

서로 다른 두 참조 축이 어긋난 문제다.

```text
1. Actor actual component list
   -> Actor instance가 실제로 소유 / 등록한 component 목록

2. UPROPERTY component pointer field
   -> C++ class에 선언된 reflected component pointer field
```

정상 상태:

```text
Actor component list
-> CombatSignalTarget component exists

ACEnemy::CombatSignalTargetComponent
-> points to that component
```

문제 상태:

```text
Actor component list
-> CombatSignalTarget component exists

ACEnemy::CombatSignalTargetComponent
-> null or stale
```

---

## UPROPERTY Component Pointer Field

다음과 같은 필드를 말한다.

```cpp
UPROPERTY(VisibleAnywhere, Category = "CombatSignal")
class UCCombatSignalTargetComponent* CombatSignalTargetComponent;
```

이 필드는 두 성격을 동시에 가진다.

```text
C++ pointer field
-> UCCombatSignalTargetComponent* 값을 저장한다.

Unreal reflected property
-> UPROPERTY가 붙었기 때문에 reflection / serialization / GC / editor system이 인식한다.
```

따라서 코드에서는 단순한 C++ 포인터처럼 보이지만, Blueprint class default data나 serialized property data 적용 대상이 될 수 있다.

---

## Native Subobject

native subobject는 C++ 생성자에서 `CreateDefaultSubobject()`로 만든 UObject / component다.

```cpp
CombatSignalTargetComponent =
    CreateDefaultSubobject<UCCombatSignalTargetComponent>(TEXT("CombatSignalTarget"));
```

이 호출은 native component object를 만들고 Actor / CDO의 subobject 체계에 등록한 뒤, 해당 UObject를 가리키는 C++ pointer를 반환한다.

반환값은 handle이 아니라 실제 UObject pointer다. 다만 객체 lifetime은 직접 `delete`로 관리하지 않고 Unreal UObject / Actor component lifecycle이 관리한다.

---

## Blueprint Generated Class와 Property 적용

Blueprint class는 C++ class를 상속한 UE class다.

PIE에서 `BP_CEnemy_C_1` 같은 Actor는 최종 Blueprint generated class 기준으로 생성된다. 단순화하면 흐름은 다음과 같다.

```text
C++ native constructor
-> native subobject 생성
-> UPROPERTY component pointer field에 pointer 저장

Blueprint generated class data 적용
-> BP class default data 적용
-> serialized property data 적용
-> inherited component metadata / override 적용

runtime actor instance 준비
-> component register / initialize
-> PostInitializeComponents
```

즉 `PostInitializeComponents()` 시점에는 이미 BP class data 적용이 끝난 상태다. 이 시점에서 C++ field가 null 또는 stale이면, 이후 게임 로직은 그 잘못된 field를 읽을 수 있다.

---

## Field가 Null 또는 Stale이 되는 대표 조건

C++ refactor 중 다음 항목이 바뀌면 Blueprint가 저장한 기존 data와 현재 C++ 구조가 어긋날 수 있다.

```text
UPROPERTY field name
CreateDefaultSubobject FName
component class name
component type
component 삭제 / 재추가
native component 생성 순서 또는 ownership
Hot Reload / Live Coding 중 구조 변경
Blueprint asset 미저장 또는 stale 상태
```

비유하면 다음과 같다.

```text
C++에서 MaxWalkSpeed = 200 설정
Blueprint default에서 MaxWalkSpeed = 400 저장
runtime 최종값은 400이 될 수 있음
```

component pointer field도 property 적용 대상이다. 차이는 값이 float가 아니라 UObject component pointer라는 점이다.

---

## FindComponentByClass가 보는 것

`FindComponentByClass<T>()`는 C++ UPROPERTY field를 읽는 함수가 아니다.

현재 Actor instance가 실제로 소유 / 등록한 component list를 검색한다.

```cpp
FindComponentByClass<UCCombatSignalTargetComponent>()
```

따라서 UPROPERTY field가 null이어도 actual component list에 component가 존재하면 valid component pointer를 반환할 수 있다.

---

## 현재 프로젝트의 대응 패턴

Character 초기화는 다음 순서로 둔다.

```text
PostInitializeComponents
-> RecoverReferences
-> BuildReferences
-> InjectReferences
```

예시:

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
-> 복구된 field 기준으로 FCharacterComponentReferences 구성

InjectReferences
-> 동일한 reference bundle을 각 component에 주입
```

recovery가 실제로 수행되면 다음 로그를 남긴다.

```text
[ComponentReferenceRecovery] Recovered | Owner=BP_CEnemy_C_1 | Component=CCombatSignalTargetComponent | Resolved=CombatSignalTarget
```

이 로그는 정상 흐름 로그가 아니라 recovery path 동작 신호다. 로그가 보이면 방어선은 작동한 것이지만, 관련 Blueprint asset의 stale state를 별도로 확인해야 한다.

---

## 에셋 조치 순서

패널에 component가 정상 표시되는데 recovery 로그가 뜨면, component object가 없는 문제가 아니라 UPROPERTY component pointer field mapping이 stale한 상태일 가능성이 높다.

권장 조치:

```text
1. 관련 Blueprint asset 열기
2. Compile / Save
3. 필요하면 Blueprint asset rename
4. Rebuild
5. Editor restart
6. PIE에서 [ComponentReferenceRecovery] 로그 재확인
7. 계속 반복되면 새 Blueprint 생성 후 설정 이관
```

이번 확인에서는 기존 `BP_CEnemy`의 이름 변경 후 rebuild / compile / save를 수행하자 recovery 로그가 사라졌다. 새 임시 Enemy BP에서도 recovery 로그가 발생하지 않았으므로 C++ native subobject 선언 자체는 정상으로 판단했다.

---

## 설계상 주의점

`FindComponentByClass`를 기본 dependency wiring 방식으로 일반화하지 않는다.

권장 기준:

```text
기본 wiring
-> C++ native subobject field
-> owner-side explicit injection
-> InitializeReferences
-> ValidateRequiredComponentReferences

recovery path
-> Blueprint stale reference 의심 시
-> PostInitializeComponents에서 actual component list 기준으로 resolve
-> field와 bundle을 같은 reference set으로 동기화
```

`FindComponentByClass`는 편리하지만 dependency가 암시화될 수 있다. 따라서 상시 lookup이 아니라 stale Blueprint/native component reference를 복구하는 방어선으로 제한한다.

---

## 체크리스트

native component refactor 후 다음을 확인한다.

```text
1. C++ build 성공
2. 관련 Blueprint asset load / compile / save
3. Details / Components panel에서 inherited component 표시 확인
4. PIE에서 required component validation 로그 확인
5. [ComponentReferenceRecovery] 로그 반복 여부 확인
6. Character field와 actual component list가 같은 component를 가리키는지 확인
7. TakeDamage / BeginPlay / delegate binding처럼 Character field를 직접 읽는 코드 확인
8. DI bundle과 Character field가 같은 resolved reference를 쓰는지 확인
```

---

## 판단 기준

다음이면 component 생성 실패보다 reference mismatch를 먼저 의심한다.

```text
FindComponentByClass로는 찾힘
UPROPERTY field는 invalid
Blueprint Details에는 component가 보임
특정 BP asset에서만 재현됨
native component rename / refactor 이후 발생함
```

반대로 다음이면 실제 component 구성 누락을 의심한다.

```text
FindComponentByClass도 실패
Details panel에도 component가 없음
constructor에서 CreateDefaultSubobject 호출 누락
parent class가 예상과 다름
Blueprint가 다른 parent class를 상속함
```

---

## 관련 문서

- `Docs/02_Bug_Report/B13_UE5_Portfolio_Bug_Report.md`
- `Docs/02_Bug_Report/B14_UE5_Portfolio_Bug_Report.md`
- `Docs/06_notes/N07_Unreal_Native_Component_Rename_And_Blueprint_Reference_Note.md`
- `Docs/06_notes/N10_Component_Reference_Validation_Policy_Note.md`
