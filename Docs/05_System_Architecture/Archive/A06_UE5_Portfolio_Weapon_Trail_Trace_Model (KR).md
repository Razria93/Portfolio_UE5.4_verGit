# Weapon Trail Trace 모델

## 1. 목적

본 문서는 근접 무기 hit 위치를 계산하기 위한 `weapon trail trace` 방식의 의미와 비용을 정리하기 위한 문서임.

현재 근접 hit 판정은 weapon collision overlap을 기반으로 target을 감지하는 구조임.

다만 overlap 기반 구조에서는 `OnComponentBeginOverlap()`의 `SweepResult`가 항상 의미 있는 hit 위치를 제공하지 않음.

특히 weapon actor가 character mesh socket에 attached 된 상태에서 animation transform을 따라 움직이는 경우 `bFromSweep == false`가 자연스럽게 발생할 수 있음.

따라서 hit VFX / hit SFX / hit direction / guard / parry surface 등을 더 정확하게 처리하려면 hit 위치 계산 방식을 별도로 검토해야 함.

---

## 2. 현재 Overlap 기반 방식

현재 구조는 weapon actor의 collision component가 overlap event를 발생시키고, `ACWeaponActor`가 이를 `FHitContext`로 변환하는 구조임.

기본 흐름은 다음과 같음.

```text
Weapon collision overlap
-> ACWeaponActor::OnComponentBeginOverlap()
-> FOverlapContext
-> FHitContext
-> UCApplyDamageComponent::RequestApplyDamage()
-> ApplyDamage / TakeDamage / Reaction / Feedback
```

이 방식의 핵심 역할은 target detection임.

즉, weapon collision과 target collision이 겹쳤는지 판단하는 데 적합함.

하지만 hit point 계산에는 한계가 있음.

`OnComponentBeginOverlap()`의 `SweepResult`는 overlap이 sweep movement 결과로 발생했을 때 의미 있는 값을 가질 수 있음.

현재 weapon collision은 보통 `MoveComponent(..., bSweep=true)`로 직접 이동하는 것이 아니라 socket / attachment / animation transform에 의해 위치가 갱신됨.

따라서 overlap이 발생해도 `bFromSweep == false`가 될 수 있고, 이 경우 `SweepResult.ImpactPoint`를 신뢰하기 어려움.

---

## 3. bFromSweep가 false인 이유

`bFromSweep == false`는 overlap이 sweep 이동의 결과가 아니라는 의미임.

현재 근접 무기 구조에서는 다음 경우가 흔함.

- weapon actor가 character mesh socket에 attached 되어 animation pose를 따라 이동함
- hit window begin 시 collision이 켜지면서 이미 겹쳐 있던 target과 overlap이 발생함
- transform update 이후 physics overlap pair가 새로 감지됨
- collision component가 sweep movement API로 이동한 것이 아님

이 경우 overlap은 정상적으로 발생하지만, engine이 sweep hit result를 생성한 상황은 아님.

따라서 `SweepResult.ImpactPoint`가 비어 있거나 부정확할 수 있음.

이는 버그라기보다 현재 overlap 기반 melee hit detection 방식의 특성으로 보는 것이 맞음.

---

## 4. Closest Point fallback

1차 보완책은 `GetClosestPointOnCollision()`을 사용해 target collision 표면의 근사 hit point를 계산하는 것임.

예시는 다음과 같음.

```cpp
const FVector queryLocation = InOverlapContext.OverlappedComponent->GetComponentLocation();

FVector closestPoint = FVector::ZeroVector;
const float distance = InOverlapContext.OtherComponent->GetClosestPointOnCollision(
	queryLocation,
	closestPoint
);
```

이 방식은 weapon collision 위치에서 target collision 표면까지 가장 가까운 지점을 계산함.

즉 다음 의미를 가짐.

```text
Weapon collision center
-> Target collision surface closest point
```

이 방식은 정확한 blade contact point는 아니지만, actor location에 VFX를 띄우는 것보다 자연스러운 hit position fallback을 제공함.

단, target component가 capsule이면 결과도 capsule surface 기준으로 계산됨.

따라서 skeletal mesh의 실제 몸 표면이나 특정 bone 위치를 정확히 맞추는 방식은 아님.

---

## 5. Weapon Trail Trace 방식

`weapon trail trace`는 engine의 별도 단일 기능명이 아니라, trace / sweep API를 조합해 만드는 melee hit detection 패턴임.

핵심은 weapon 위의 여러 sample point를 정하고, 각 sample point의 이전 위치와 현재 위치 사이를 trace하는 것임.

예시는 다음과 같음.

```text
Blade_Base previous position -> Blade_Base current position
Blade_Mid previous position  -> Blade_Mid current position
Blade_Tip previous position  -> Blade_Tip current position
```

각 구간에 대해 `SphereTraceMulti()` 같은 trace를 수행함.

`SphereTrace`는 이름상 sphere trace지만 실제 의미는 다음과 같음.

```text
Radius R sphere swept from Start to End
-> capsule-like swept volume
-> find objects overlapping or blocking that swept volume
```

즉 두 점 사이에 반지름을 가진 캡슐형 검사 영역을 만들고, 해당 영역과 충돌하는 객체를 찾는 방식으로 이해할 수 있음.

---

## 6. Overlap 방식과 Trail Trace 방식의 차이

Overlap 방식은 weapon shape를 감싸는 collision volume이 target collision과 겹쳤는지를 확인함.

Trail trace 방식은 weapon 위의 여러 sample point가 시간에 따라 이동한 궤적을 검사함.

차이는 다음과 같음.

```text
Overlap
-> weapon collision volume itself detects overlap
-> target detection에 적합함
-> 정확한 ImpactPoint를 얻기 어려울 수 있음

Trail Trace
-> several sample points trace their previous-current paths
-> hit point / hit normal / bone / surface data를 얻기 쉬움
-> weapon shape를 sample point들의 swept volume으로 근사함
```

따라서 trail trace는 단순한 hit 위치 보완이 아니라 hit detection metadata를 바꾸는 작업에 가까움.

---

## 7. Trail Trace의 빈 공간 문제

Trail trace는 weapon 자체의 전체 부피를 수학적으로 완벽하게 검사하는 방식이 아님.

weapon 위에 배치한 sample point들의 trace volume을 합쳐 weapon path를 근사하는 방식임.

따라서 sample 간격이 넓고 trace radius가 작으면 sample 사이에 빈 공간이 생길 수 있음.

이론적으로는 작은 target이 sample 사이를 지나가면 감지되지 않을 수 있음.

보완 방법은 다음과 같음.

- sample socket 수를 늘림
- trace radius를 늘림
- sphere trace 대신 box trace / capsule trace를 고려함
- 빠른 공격은 더 짧은 tick interval 또는 sub-step을 고려함
- blade root / mid / tip뿐 아니라 blade length를 따라 여러 sample point를 둠

다만 trace radius를 과하게 늘리면 실제 weapon shape보다 넓게 맞는 문제가 생김.

sample 수를 과하게 늘리면 trace 비용과 중복 hit 처리 비용이 증가함.

따라서 trail trace는 정밀도와 비용 사이의 튜닝이 필요한 방식임.

---

## 8. 비용과 구현 부담

Trail trace는 기존 overlap 기반 구조보다 비용과 구현 부담이 큼.

필요한 추가 요소는 다음과 같음.

- weapon mesh 또는 weapon actor에 trace socket / sample point를 구성해야 함
- hit window 동안 sample point의 previous location을 저장해야 함
- tick 또는 notify tick에서 current location과 previous location 사이를 trace해야 함
- trace result를 `FDamageImpactInfo`로 변환해야 함
- 중복 hit target 처리를 기존 hit window 정책과 통합해야 함
- debug draw / trace channel / ignored actor 정책을 정리해야 함
- 빠른 공격과 낮은 frame rate 상황에서 누락을 줄이기 위한 보정이 필요함

즉 trail trace는 단순히 VFX 위치를 바꾸는 작업이 아니라 melee hit detection 기준을 바꾸는 작업임.

현재 overlap 기반 collision을 유지하면서 trail trace를 추가하면 두 hit detection 방식이 공존하게 됨.

따라서 도입 시에는 overlap을 target detection으로 계속 쓸지, trail trace가 target detection까지 대체할지 결정해야 함.

---

## 9. 현재 프로젝트 기준 권장 방향

현재 단계에서는 trail trace를 즉시 도입하기보다 `FDamageImpactInfo`와 closest point fallback을 먼저 도입하는 것이 적절함.

1차 권장 흐름은 다음과 같음.

```text
Weapon overlap
-> FHitContext
-> FDamageImpactInfo
   - bFromSweep == true이면 SweepResult 사용
   - bFromSweep == false이면 GetClosestPointOnCollision fallback 사용
-> ApplyDamagePayload / Context
-> FDefaultDamageEvent
-> TakeDamagePayload / Context
-> DamageFeedback
```

이 방식은 현재 overlap 기반 hit detection 메타를 유지하면서 hit VFX 위치를 개선할 수 있음.

현재 `ReactionFeedback`은 reaction type / damage spec key / timing / trigger key 기반의 feedback matching 구조임.

따라서 현재 구현 기준으로 `FDamageImpactInfo`를 직접 소비하는 책임은 `DamageFeedback`에 두는 것이 맞음.

반면 trail trace는 다음 요구가 명확해졌을 때 도입하는 것이 적절함.

- weapon path 기반의 정확한 impact point가 필요함
- hit normal / surface / bone data를 안정적으로 얻어야 함
- guard / parry / weapon clash가 hit 위치와 방향에 강하게 의존함
- 빠른 weapon swing에서 overlap 기반 hit point fallback이 부정확함
- weapon collision volume보다 socket path 기반 판정이 더 일관된 gameplay 결과를 제공함

따라서 현재 결론은 다음과 같음.

```text
1차 구현
-> Overlap 유지 + FDamageImpactInfo + ClosestPoint fallback

추후 확장
-> Weapon Trail Trace를 별도 hit detection model로 도입 검토
```

---

## 10. 책임 배치

Hit 위치 계산 책임은 `ACWeaponActor`에 두는 것이 가장 자연스러움.

`ACWeaponActor`는 overlap 원본과 weapon collision 정보를 모두 알고 있음.

따라서 다음 정보에 접근할 수 있음.

- weapon collision component
- target component
- sweep result
- hit window id
- weapon / action context
- socket 또는 sample point transform
- 이전 frame의 weapon sample location

반면 `UCApplyDamageComponent`는 damage pipeline을 처리하는 계층임.

따라서 hit point를 새로 계산하기보다 `FHitContext` 또는 `FDamageImpactInfo`를 받아 payload / context에 복사하는 역할이 더 적절함.

권장 책임은 다음과 같음.

```text
ACWeaponActor
-> hit event 생성
-> hit point / hit normal 계산
-> FDamageImpactInfo 구성

UCApplyDamageComponent
-> DamageImpactInfo를 ApplyDamagePayload / Context로 전달
-> FDefaultDamageEvent에 복사

UCTakeDamageComponent
-> DamageImpactInfo를 TakeDamagePayload / Context로 보존

DamageFeedback
-> TakeDamagePacket.Context.DamageImpactInfo를 사용함
```

이렇게 하면 ApplyDamage와 TakeDamage는 damage calculation domain을 유지하고, hit event metadata는 별도 구조로 전달됨.
