# N10. Component Reference Validation Policy Note

## 목적

이 노트는 `Source/Portfolio`의 component reference 초기화와 검증 기준을 정리한다.

목표는 모든 `check()`를 제거하는 것이 아니다. 컴포넌트 참조가 실패했을 때 어떤 경로에서 crash해야 하고, 어떤 경로에서 원인을 남기고 safe return해야 하는지 기준을 분리하는 것이다.

---

## 1. 기본 원칙

컴포넌트 참조는 다음 네 가지로 분류한다.

```text
필수 owner
필수 component dependency
선택 component dependency
일시 lookup dependency
```

필수 참조는 초기화 시점에 명시적으로 검증한다. 선택 참조는 기능 사용 시점에 `IsValid`로 확인하고, 없으면 해당 기능만 skip한다.

---

## 2. check 사용 기준

`check()`는 "절대 깨지면 안 되는 C++ 불변식"에만 사용한다.

UE build configuration은 대략 다음처럼 구분한다.

```text
Debug / DebugGame
-> 디버깅 정보가 많고 상대적으로 느림

Development
-> 개발 중 일반적으로 사용하는 빌드

Test
-> Shipping에 가깝지만 일부 테스트 기능 유지

Shipping
-> 최종 배포용
-> debug code / assert / log 일부가 제거되거나 비활성화될 수 있음
-> 성능과 패키징 기준에 가까움
```

따라서 runtime 구성 오류를 `check()`에만 의존하면 Development에서는 즉시 crash로 드러나고, Shipping에서는 검증 시점이 흐려질 수 있다.

native subobject는 C++ 생성자에서 `CreateDefaultSubobject` API 호출 직후 생성되는 UObject / Component를 뜻한다. 이 객체들은 C++ class의 기본 구성 일부이며, Blueprint 자식에서도 native component로 이어진다.

허용 후보는 다음과 같다.

```text
constructor에서 CreateDefaultSubobject 직후 native subobject 생성 확인
switch / enum 처리에서 도달하면 안 되는 internal invariant
테스트 중 즉시 중단해야 하는 순수 코드 계약
```

주의할 점:

```text
runtime asset / Blueprint 구성 / level 배치 / component 누락 검증에는 check를 기본으로 쓰지 않는다.
```

이런 경우 `check`는 원인 추적보다 즉시 crash에 가까워지고, Shipping 구성에서는 검증 시점이 흐려질 수 있다.

---

## 3. ensure 사용 기준

`ensureMsgf()`는 "잘못된 구성은 알려야 하지만, 이후 코드가 safe return으로 방어할 수 있는 경우"에 사용한다.

좋은 후보는 다음과 같다.

```text
BeginPlay에서 필수 component dependency 검증
Initialize 함수의 필수 owner / owning component 검증
런타임에서 복구 가능한 잘못된 호출 경로
```

프로젝트 기준:

```text
필수 component 누락
-> ensureMsgf로 어떤 컴포넌트가 누락됐는지 남김
-> public request 경로에서는 InvalidComponent / safe return으로 방어
```

---

## 4. 필수 component

필수 component는 해당 클래스의 핵심 책임을 수행하는 데 없으면 안 되는 dependency다.

예시는 다음과 같다.

```text
ActionOrchestrator
-> MovementComponent
-> WeaponComponent
-> StateComponent
-> HealthComponent
-> ActionComponent
-> ReactionComponent

ReactionOrchestrator
-> StateComponent
-> HealthComponent
-> ActionComponent
-> ReactionComponent
```

필수 component는 owner가 알고 있는 native subobject를 명시적으로 주입하고, component는 `BeginPlay()`에서 주입 결과를 검증한다.

```cpp
const FCharacterComponentReferences references = BuildComponentReferences();
ActionOrchestratorComponent->InitializeReferences(references);
```

`FindComponentByClass`는 필수 dependency wiring의 기본 방식으로 쓰지 않는다. 필수 dependency를 소유자가 이미 알고 있다면 owner-side explicit injection을 우선한다.

주입받은 필드는 일반 cache와 구분하기 위해 `_Injected` suffix를 사용한다.

Player / Enemy는 component 주소를 `FCharacterComponentReferences`로 묶어 전달한다. 각 component는 이 구조체에서 필요한 dependency만 꺼내 `_Injected` 필드에 저장한다.

```text
Player / Enemy
-> BuildComponentReferences
-> InjectComponentReferences

Injected component
-> InitializeReferences(const FCharacterComponentReferences&)
-> 필요한 참조만 선택적으로 저장
-> BeginPlay에서 required reference validation
```

이 방식은 component가 늘어날 때 `InitializeReferences`의 parameter list가 계속 길어지는 문제를 줄이고, component 배열 순서를 `FCharacterComponentReferences` 한 곳에서 맞추기 위한 기준점이 된다.

### Rename migration recovery

`P24 Combat Signal Component Rename`의 `ResolveComponentReferences()`는 상시 dependency wiring이 아니라 native component rename 직후의 migration recovery 코드였다.

```text
native component rename 직후
-> Blueprint에는 renamed component instance가 존재
-> C++ UPROPERTY member pointer가 invalid일 수 있음
-> FindComponentByClass로 이미 붙어 있는 component instance를 다시 연결
```

현재 브랜치에서는 rename 안정화 이후의 기준을 정리하므로 이 recovery 경로를 기본 component reference 정책에 포함하지 않는다.

```text
상시 정책
-> native subobject 생성
-> owner-side explicit injection
-> 각 component의 required reference validation

일시 정책
-> native component rename 직후 문제가 확인된 경우에만 recovery hook 검토
```

비슷한 문제가 다시 발생하면 `ResolveComponentReferences` 같은 일반 이름보다 `RecoverRenamedComponentReferences`처럼 일시적 migration 성격이 드러나는 이름을 사용한다. 이 코드는 Blueprint asset load / compile / save, runtime member pointer 검증이 끝난 뒤 제거 여부를 다시 판단한다.

---

## 5. 선택 component

선택 component는 있으면 부가 기능을 수행하고, 없으면 핵심 실행을 막지 않는 dependency다.

예시는 다음과 같다.

```text
ObservableOverlayComponent
-> overlay snapshot / overlay gate 같은 부가 관찰 상태
```

선택 component는 초기화 시점에 필수 검증 대상에 넣지 않는다. 사용 시점에 `IsValid`로 확인하고 없으면 해당 부가 기능만 skip한다.

---

## 6. Public request 경로

외부에서 들어오는 request 함수는 필수 dependency가 없을 때 crash하지 않고 명시적인 reject result를 반환한다.

예시는 다음과 같다.

```text
Action request
-> EActionRequestRejectReason::InvalidComponent

Reaction request
-> EReactionRequestRejectReason::InvalidComponent
```

이 정책은 초기화 검증과 런타임 방어를 함께 사용하기 위한 것이다.

```text
BeginPlay
-> 구성 오류를 ensure로 빠르게 노출

Request path
-> null 접근을 막고 reject result 반환
```

---

## 7. 현재 브랜치 결정

`refactor/component-reference-validation-policy`에서는 가장 영향이 작고 설명력이 높은 두 컴포넌트에 먼저 적용한다.

```text
UCActionOrchestratorComponent
UCReactionOrchestratorComponent
```

적용 내용:

```text
Player / Enemy의 PostInitializeComponents에서 Orchestrator dependency를 명시적으로 주입
Orchestrator BeginPlay에서는 주입된 owner / component reference를 검증
필수 component 검증을 ValidateRequiredComponentReferences로 분리
누락 component 이름이 로그에 드러나도록 ensureMsgf 메시지 작성
ObservableOverlayComponent는 선택 dependency로 유지
```

후속 작업:

```text
ActionComponent / ReactionComponent 초기화 검증
CombatSignalSource / CombatSignalTarget 초기화 검증
Feedback component 초기화 검증
Notify lookup 경로 검증 정책
```
