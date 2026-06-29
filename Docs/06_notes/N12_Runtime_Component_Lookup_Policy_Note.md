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
