# N09. Unreal Reference Safety Policy Note

## 목적

이 노트는 `Source/Portfolio`에서 UObject 참조를 어떤 기준으로 보관할지 정리한다.

목표는 모든 raw pointer를 한 번에 교체하는 것이 아니다. 각 UObject 참조가 아래 의미 중 무엇인지 코드와 문서에서 설명 가능하게 만드는 것이다.

```text
소유 / 유지 참조
약한 관찰 참조
런타임 캐시
일시적 payload
함수 내부 접근
에디터 설정 asset / class 참조
```

---

## 1. 기본 원칙

멤버 필드로 보관하는 UObject 계열 포인터는 명시적인 수명 정책을 가져야 한다.

```text
보관되는 UObject 참조
-> UPROPERTY, TObjectPtr, TWeakObjectPtr, TSoftObjectPtr, 또는 의도적인 제외 사유
```

raw `UObject*`, `AActor*`, Component pointer가 항상 잘못된 것은 아니다. 다만 의미가 모호하게 남으면 안 된다.

---

## 2. UPROPERTY

`UCLASS` 또는 reflected `USTRUCT` 안에 UObject 참조를 보관하고, 해당 참조가 Unreal reflection / serialization / editor / GC tracking 시스템과 연결되어야 한다면 `UPROPERTY`를 사용한다.

```cpp
UPROPERTY(Transient)
AActor* OwnerActor_Cached = nullptr;
```

프로젝트 기준은 다음과 같다.

```text
런타임 cached UObject 멤버
-> UPROPERTY(Transient)

CreateDefaultSubobject로 생성하는 native component
-> UPROPERTY(VisibleAnywhere)

에디터에서 설정하는 asset / tuning data
-> UPROPERTY(EditAnywhere 또는 EditDefaultsOnly)

저장되지 않아야 하는 runtime state
-> UPROPERTY(Transient)
```

`UPROPERTY`는 단순 장식으로 붙이지 않는다. specifier는 Unreal이 해당 필드를 알아야 하는 이유를 설명해야 한다.

---

## 3. TObjectPtr

오브젝트 필드로 유지되는 UObject 참조이고 UE5 스타일 reflected object tracking에 참여하는 것이 자연스럽다면 `TObjectPtr<T>`를 우선 고려한다.

좋은 후보는 다음과 같다.

```text
USTRUCT data 안의 asset reference
Actor가 소유하는 component field
Component가 생성 / 유지하는 executor instance
UCLASS instance가 오래 보관하는 UObject reference
```

현재 프로젝트 기준은 다음과 같다.

```text
새로 추가하거나 수정 중인 retained UObject field는 주변 코드가 이미 TObjectPtr를 쓰거나 변경 위험이 낮을 때 TObjectPtr를 사용할 수 있다.
기존 raw UPROPERTY field를 한 번에 전부 migration하지 않는다.
```

이 기준은 코드 안정성을 유지하면서 UE5 pointer style로 점진 이동하기 위한 것이다.

---

## 4. TWeakObjectPtr

참조 대상 UObject를 관찰하지만, 그 대상을 살려두거나 소유한다는 의미를 주면 안 되는 경우 `TWeakObjectPtr<T>`를 사용한다.

좋은 후보는 다음과 같다.

```text
timer restore target
hit-window duplicate target cache
temporary actor tracking map
AI perception memory처럼 actor 파괴를 막으면 안 되는 추적 데이터
lookup 비용을 줄이기 위한 optional external actor / component cache
```

예시는 다음과 같다.

```cpp
TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveHitStopMap;
TSet<TWeakObjectPtr<AActor>> DamagedTargets;
```

접근 기준은 다음과 같다.

```cpp
AActor* Actor = ActorKey.Get();
if (!IsValid(Actor)) return;
```

같은 actor가 소유하는 필수 subobject에는 특별한 이유가 없으면 `TWeakObjectPtr`를 우선하지 않는다. 필수 component는 보통 초기화 시점에 검증하는 쪽이 더 명확하다.

---

## 5. Raw Pointer

UObject raw pointer는 보관하지 않거나 Unreal API 경계에 해당하는 경우 허용한다.

허용 사례는 다음과 같다.

```text
함수 매개변수
지역 변수
lookup 함수의 반환값
UE callback signature
짧게 소비되는 event payload
즉시 복사 / 소비되는 non-owning USTRUCT context
```

예시는 다음과 같다.

```cpp
void ApplyHitStop(AActor* InActor, float InDuration, float InDilation);
AActor* TargetActor = ResolveCueTargetActor();
```

raw pointer를 멤버 필드로 보관한다면 아래 중 하나가 필요하다.

```text
UPROPERTY(...)
또는 필드 근처에 명시된 의도적인 non-UPROPERTY 사유
```

---

## 6. Class / Asset Reference

에디터에서 설정하는 class reference는 `TSubclassOf<T>`를 사용한다.

```cpp
UPROPERTY(EditAnywhere)
TSubclassOf<UCAction> ActionExecutorKey = nullptr;
```

asset / class 참조가 즉시 로드되지 않아야 하는 경우에만 `TSoftObjectPtr<T>` 또는 `TSoftClassPtr<T>`를 사용한다.

현재 프로젝트 기준은 다음과 같다.

```text
Phase 1에서는 broad soft-reference migration을 하지 않는다.
명확한 loading / asset dependency 이유가 있을 때만 soft reference를 사용한다.
```

---

## 7. Non-UObject Smart Pointer

`TSharedPtr`, `TUniquePtr`, `TSharedRef`는 UObject 소유권 표현에 사용하지 않는다.

이 포인터들은 non-UObject C++ data, Slate-style object, 순수 helper object에는 사용할 수 있다. Actor, Component, Asset, Animation, UObject executor에는 사용하지 않는다.

---

## 8. Container

컨테이너 필드도 일반 필드와 같은 기준을 따른다.

```text
보관되는 UObject element
-> GC / reflection tracking이 필요하면 UPROPERTY container

관찰만 하는 actor / component key 또는 value
-> 소유 의미를 피해야 하면 TWeakObjectPtr element type

함수 내부 temporary container
-> 같은 호출 흐름 안에서 소비된다면 raw pointer element 허용
```

리뷰 질문은 다음과 같다.

```text
이 컨테이너가 clear되기 전에 대상 object가 파괴되면 어떻게 되어야 하는가?
```

답이 "안전하게 skip한다"라면 weak reference가 더 적절하다.

---

## 9. Timer / Delegate

Timer 또는 delegate payload가 외부 actor를 의도치 않게 붙잡으면 안 된다.

프로젝트 기준은 다음과 같다.

```text
UObject가 소유한 timer가 다른 actor의 state를 복구한다
-> timer owner object는 일반 member function binding으로 묶는다
-> 복구 대상 actor는 TWeakObjectPtr로 전달한다
```

예시는 다음과 같다.

```cpp
const TWeakObjectPtr<AActor> ActorKey(InActor);
FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::RestoreHitStop, ActorKey);
```

람다는 named member function보다 코드가 단순해지는 경우에만 사용한다. 리뷰 가능성을 위해 기본적으로는 named member function을 우선한다.

---

## 10. 현재 브랜치 결정

`refactor/unreal-reference-safety-v1`에서는 이 정책의 가장 작고 안전한 일부만 적용한다.

```text
UCHealthComponent::OwnerActor_Cached
-> UPROPERTY(Transient)와 nullptr 초기화 추가

UCWorldSubsystem_CombatFeedback hit-stop maps
-> temporary actor tracking을 weak actor key 기준으로 변경

UCCombatSignalSourceComponent damaged target cache
-> hit-window duplicate tracking target을 weak reference로 변경
```

후속 작업으로 분리할 항목은 다음과 같다.

```text
FCombatSignalHitWindowKey::DamageCauser raw actor key
-> hash / equality 의미가 바뀔 수 있으므로 별도 브랜치

Project-wide TObjectPtr migration
-> diff가 넓고 구체적 안전성 이슈와 묶이지 않으면 스타일 변경에 가까우므로 별도 브랜치

Cached pointer nullptr initialization sweep
-> 별도 quick-win 브랜치
```
