# N12. Runtime Component Lookup Policy Note

## 목적

P29에서 Character가 소유한 component reference는 owner 쪽에서 구성하고 주입하는 흐름으로 정리했다.

하지만 프로젝트에는 여전히 runtime에서 component나 actor를 조회해야 하는 경로가 남아 있다.

이 문서는 다음 질문에 대한 기준을 고정한다.

```text
이 참조는 Character DI 대상인가?
아니면 runtime lookup으로 남겨야 하는가?
조회 결과를 cache해야 하는가?
조회 실패 시 reject / ignore / safe return 중 무엇이 맞는가?
```

---

## 핵심 결론

모든 `FindComponentByClass`, `GetOwner`, `TryGetPawnOwner`, Blackboard 조회를 제거하는 것이 목표가 아니다.

참조 확보 방식은 객체의 소유권과 lifecycle에 따라 나눈다.

```text
Character-owned required component
-> Character가 BuildReferences / InjectReferences로 주입

Runtime target / external actor
-> request 시점에 조회

Animation owner / mesh-bound object
-> AnimInstance lifecycle에서 owner를 확인하고 cache

AI perception / blackboard state
-> AIController / Blackboard 기준 runtime query 유지

Blueprint stale native component reference recovery
-> 예외적 방어선으로만 FindComponentByClass 사용
```

---

## 1. Character Component DI 대상

다음 조건을 만족하면 DI 대상이다.

```text
- Character가 생성자에서 만들거나 소유하는 component다.
- 동일 Character 내부 sibling component가 반복적으로 참조한다.
- gameplay 실행에 필수 dependency다.
- 참조 누락이 구조 오류에 가깝다.
```

예시:

```text
MovementComponent
WeaponComponent
StateComponent
HealthComponent
DefenseComponent
CombatSignalSourceComponent
CombatSignalTargetComponent
ActionComponent
ReactionComponent
FeedbackComponent
```

정책:

```text
ACPlayer / ACEnemy
-> RecoverReferences
-> BuildReferences
-> InjectReferences
-> component InitializeReferences
-> ValidateRequiredComponentReferences
```

component 내부에서 sibling component를 다시 `FindComponentByClass`로 찾지 않는다.

### Component Order

Character-owned component를 나열할 때는 다음 순서를 기본값으로 둔다.

```text
OwnerCharacter

Movement
Weapon
State
Health
Defense
ObservableOverlay

CombatSignalSource
CombatSignalTarget

ActionOrchestrator
ReactionOrchestrator

Action
Reaction

HitFeedback
ActionFeedback
ReactionFeedback
```

이 순서는 다음 영역에 우선 적용한다.

```text
Character field declaration
Character constructor CreateDefaultSubobject
RecoverReferences
BuildReferences
InjectReferences
FCharacterComponentReferences
InitializeReferences 내부 대입
ValidateRequiredReferences 배열
component getter 나열
```

단, runtime gameplay flow는 component order보다 domain 처리 순서를 우선한다.

예외:

```text
ACEnemy
-> 현재 DefenseComponent를 보유하지 않는다.
-> 적 방어 기능이 필요해지는 시점에 추가한다.
-> CombatSignalTarget의 Defense reference는 현재 optional dependency로 취급한다.

WeaponActor
-> Character-owned component가 아니라 WeaponComponent가 spawn하는 runtime actor다.
-> Character component order 대상이 아니다.

AnimInstance
-> ActorComponent DI가 아니라 TryGetPawnOwner 기반 animation lifecycle cache다.

Notify / AI BT / CombatSignal target resolve
-> runtime context에서 owner / target / Blackboard / payload를 해석하는 경로다.
-> Character component order를 강제하지 않는다.
```

---

## 2. Runtime Lookup 유지 대상

다음 조건이면 runtime lookup을 허용한다.

```text
- 대상이 현재 Character의 고정 component가 아니다.
- request / overlap / AI 판단 시점마다 target이 달라질 수 있다.
- 외부 actor 또는 외부 actor의 component를 resolve하는 흐름이다.
- lookup 자체가 domain policy의 일부다.
```

예시:

```text
CombatSignal target actor에서 target component resolve
AIController에서 Blackboard target actor resolve
DamageCauser owner / instigator fallback resolve
Overlap OtherActor / OtherComp 기반 hit context 구성
```

정책:

```text
runtime target lookup은 DI로 바꾸지 않는다.
lookup 실패는 crash가 아니라 reject / ignore / no-op으로 처리한다.
```

---

## 3. Notify / NotifyState

Notify는 montage timeline의 timing trigger다.

정책:

```text
Notify는 domain 처리를 직접 수행하지 않는다.
Notify base/helper는 owner actor / mesh context에서 routing component를 찾을 수 있다.
Notify 본문은 필요한 경우 component routing entry까지만 호출한다.
실제 판단과 실행은 component / active executor로 넘긴다.
```

허용되는 책임:

```text
- timing key 전달
- command enum 전달
- window begin / end 전달
- cue tag 전달
```

피해야 할 책임:

```text
- target resolve
- combat signal build / send
- action / reaction policy 판단
- damage / feedback / AI domain 처리
```

검토 대상:

```text
Source/Portfolio/Notify
```

현재 정리 기준:

```text
Action Notify
-> UCAnimNotify_ActionBase / UCAnimNotifyState_ActionBase
-> UCActionComponent notify routing API

Reaction Notify
-> UCAnimNotify_ReactionBase / UCAnimNotifyState_ReactionBase
-> UCReactionComponent notify routing API

Health State Notify
-> UCAnimNotify_HealthBase
-> UCHealthComponent notify routing API

Collision Window Notify
-> UCActionComponent collision window routing API
-> UCWeaponComponent
-> ACWeaponActor
```

---

## 4. AnimInstance

AnimInstance는 ActorComponent DI 흐름과 다르다.

이유:

```text
- AnimInstance는 SkeletalMesh / AnimBP lifecycle에 묶인다.
- owner pawn은 TryGetPawnOwner()로 얻는 것이 일반적이다.
- mesh 변경, preview, editor context에서 owner가 없을 수 있다.
```

정책:

```text
NativeInitializeAnimation / NativeBeginPlay 계열에서 owner를 확인한다.
NativeUpdateAnimation에서는 invalid owner / component일 때 safe return한다.
반복 조회가 부담되거나 의미가 고정되면 cache한다.
cache는 owner 변경 가능성을 고려해 갱신 조건을 둔다.
```

AnimInstance의 component cache는 Character DI가 아니다.

현재 정리 기준:

```text
NativeInitializeAnimation
-> 기존 delegate 해제
-> cached reference 초기화
-> TryGetPawnOwner 기반 owner / component cache
-> component event binding
-> 초기 state parameter 갱신

NativeUpdateAnimation
-> cached owner가 invalid면 safe return
-> Movement parameter 갱신
-> State / Guard parameter 갱신

NativeUninitializeAnimation
-> delegate 해제
-> cached reference 초기화
```

`FindComponentByClass<T>()`는 AnimInstance가 mesh owner에서 animation parameter source를 찾기 위한 runtime cache 방식으로 사용한다.

---

## 5. AI BT Service / Decorator / Task

AI BehaviorTree 계층은 runtime decision layer다.

정책:

```text
Blackboard value 조회는 runtime query로 유지한다.
AIController / Pawn / target actor 조회도 runtime query로 유지한다.
다만 같은 service tick 안에서 반복되는 component lookup은 local variable로 줄인다.
필수 key / component 누락은 reject / fail / clear blackboard 중 하나로 명시한다.
```

구분:

```text
Service
-> perception / distance / range / state snapshot 갱신

Decorator
-> 조건 판정

Task
-> 실행 요청 전달
```

AI component lookup을 무조건 Character DI로 밀어 넣지 않는다.

현재 정리 기준:

```text
BT Service
-> OwnerComp에서 Blackboard / AI owner / Pawn을 runtime query로 얻는다.
-> 같은 tick 안에서는 AI owner / Pawn을 local variable로 보관한다.
-> owner component 조회가 필요하면 FindComponentByClass<T>()를 사용한다.
-> 조회 실패 시 관련 blackboard context를 clear한다.

BT Decorator
-> Blackboard / Pawn / component를 읽어 조건만 반환한다.
-> 조회 실패는 false로 처리한다.

BT Task
-> Blackboard / AI owner / Pawn을 runtime query로 얻는다.
-> 실행 필수 값이 없으면 Failed를 반환한다.
```

---

## 6. WeaponActor

WeaponActor는 Character component가 아니라 runtime actor다.

정책:

```text
WeaponComponent가 생성한 뒤 필요한 owner reference를 InitializeReferences로 전달한다.
WeaponActor는 BeginPlay에서 owner component를 다시 찾지 않는다.
Overlap target / hit component는 event payload에서 해석한다.
```

검토 기준:

```text
OwnerCharacter_Injected
CombatSignalSourceComp_Injected
Overlap OtherActor / OtherComp
Socket attach target
```

현재 정리 기준:

```text
WeaponComponent
-> WeaponActor를 runtime actor로 spawn한다.
-> spawn 직후 OwnerCharacter / CombatSignalSource reference를 주입한다.
-> EndPlay에서 WeaponActor runtime state(collision / trail)를 정리하고 spawned actor를 Destroy한다.

WeaponActor
-> BeginPlay에서 collision component delegate를 bind하고 cache한다.
-> EndPlay / runtime state clear 경로에서 collision window와 trail을 명시적으로 닫는다.
-> EndPlay에서 collision delegate를 unbind하고 collision cache / injected reference를 비운다.
-> owner component를 다시 lookup하지 않고 injected reference를 사용한다.
-> overlap target은 event payload의 OtherActor / OtherComp 기준으로 해석한다.
```

후속 보완 대상:

```text
Actor / Component teardown cleanup
-> EndPlay에서 delegate / timer / spawned actor / runtime cache 정리 기준을 별도 점검한다.
-> gameplay cleanup API와 teardown-safe cleanup API를 분리할 필요가 있는지 확인한다.
-> WeaponActor collision window close와 EndPlay cleanup은 별도 refactor에서 다룬다.
```

---

## 7. 실패 처리 기준

```text
필수 owner / component 누락
-> InitializeReferences에서 ensure + validation log

public request entry에서 필수 component 누락
-> reject / false / safe return

runtime target lookup 실패
-> request reject / ignored / no-op

Blueprint stale native component reference
-> recovery log 출력 후 actual component list 기준 복구
```

`ensure`는 진단 도구이지 런타임 흐름을 멈추는 정책이 아니다.

따라서 public request path에서는 필요한 경우 별도의 `IsValid` gate가 필요하다.

---

## 8. 이번 브랜치 검토 순서

```text
1. Notify / NotifyState component routing 확인
2. AnimInstance owner / component cache 확인
3. AI BT Service / Decorator / Task lookup 확인
4. WeaponActor runtime reference 확인
5. FindComponentByClass / TryGetPawnOwner / GetOwner / Blackboard 조회 사용처 분류
6. 필요한 코드 수정만 반영
7. 문서와 PR 기록 업데이트
```

---

## 완료 기준

```text
- Character-owned component lookup과 runtime target lookup이 구분되어 있다.
- Notify가 domain component를 직접 찾는 흐름이 남아 있다면 수정하거나 사유를 기록했다.
- AnimInstance cache는 ActorComponent DI와 다른 기준으로 설명되어 있다.
- AI BehaviorTree lookup은 Service / Decorator / Task 역할에 맞게 분류되어 있다.
- 남겨 둔 lookup은 왜 유지되는지 설명 가능하다.
- 변경 후 Unreal C++ build가 성공한다.
```
