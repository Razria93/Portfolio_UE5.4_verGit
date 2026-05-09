# Combat Feedback and Damage Impact Architecture Decision

## 1. 목적

본 문서는 combat feedback 계층과 damage impact metadata 흐름을 정리하기 위한 architecture decision 문서임.

archive 문서 `A06`, `A08`의 내용을 하나의 흐름으로 재구성함.

핵심은 `DamageFeedback`과 `ReactionFeedback`을 분리하고, `FDamageImpactInfo`가 어디서 생성되고 어디서 소비되는지 명확히 하는 것임.

---

## 2. 배경

기존 feedback 구조에서는 피격 상황에서 발생하는 여러 표현이 하나의 feedback 흐름처럼 보이기 쉬웠음.

예시는 다음과 같음.

```text
피격 순간의 hit VFX / hit SFX
reaction montage 중 실행되는 posture dust / stagger feedback
hit stop
camera shake
reaction start / completed / interrupted feedback
```

이들은 모두 combat 중에 발생할 수 있지만, 기준 context가 다름.

예를 들어 hit VFX는 damage event의 충돌 위치를 기준으로 해석하는 것이 자연스럽고, reaction montage 중 posture dust는 reaction lifecycle timing을 기준으로 해석하는 것이 자연스러움.

따라서 feedback을 하나의 component에 섞기보다, 어떤 event/context를 기준으로 실행되는지에 따라 분리하는 것이 적절함.

이번 문서의 핵심 쟁점은 다음 두 가지임.

```text
DamageFeedback과 ReactionFeedback을 어떤 기준으로 분리할 것인가
DamageFeedback이 사용할 hit 위치 metadata를 어디서 만들고 어떻게 전달할 것인가
```

---

## 3. 결정 사항

Combat feedback은 다음 계층으로 분리함.

```text
ActionFeedback
-> action execution context 기반 feedback

ReactionFeedback
-> reaction execution context 기반 feedback

DamageFeedback
-> take damage packet / damage impact metadata 기반 feedback

PlayerFeedback
-> local player presentation 기반 feedback

CombatFeedbackSubsystem
-> world-level feedback execution support
```

특히 이번 reaction orchestration 범위에서는 `ReactionFeedback`과 `DamageFeedback`의 분리가 중요함.

```text
DamageFeedback
-> damage event가 실제로 accepted / committed 되었는지
-> 어디에 맞았는지
-> hit VFX / hit SFX / hit stop / camera shake를 어떻게 실행할지

ReactionFeedback
-> 어떤 reaction이 실행 중인지
-> reaction이 어떤 timing에 도달했는지
-> reaction type / damage spec / trigger key에 맞는 표현이 무엇인지
```

따라서 sword hit VFX는 `DamageFeedback`에 가깝고, hit reaction montage 중 발생하는 posture break dust는 `ReactionFeedback`에 가까움.

정리하면 다음과 같음.

```text
DamageFeedback
-> damage event가 발생한 순간의 표현임
-> hit impact point / damage accepted result / hit stop 같은 정보가 중요함

ReactionFeedback
-> reaction executor가 실행 중일 때의 표현임
-> reaction type / timing / trigger key / montage notify가 중요함
```

두 feedback은 모두 피격과 관련될 수 있지만, 같은 책임이 아님.

---

## 4. DamageImpactInfo

Hit feedback 위치를 개선하기 위해 `FDamageImpactInfo`를 추가함.

`FDamageImpactInfo`는 damage result가 아니라 damage event에 동반되는 impact metadata임.

구조의 목적은 다음과 같음.

```text
damage event에 hit impact metadata를 실어 보냄
DamageFeedback이 actor location이 아니라 impact point 기반으로 VFX / SFX를 실행할 수 있게 함
TakeDamagePacket 안에서 damage result와 impact metadata를 함께 사용할 수 있게 함
```

현재 전달 흐름은 다음과 같음.

```text
ACWeaponActor
-> FHitContext.DamageImpactInfo
-> FApplyDamagePayload
-> FApplyDamageContext
-> FDefaultDamageEvent
-> FTakeDamagePayload
-> FTakeDamageContext
-> FTakeDamagePacket.Context.DamageImpactInfo
-> UCDamageFeedbackComponent
```

책임은 다음처럼 나눔.

```text
ACWeaponActor
-> overlap / sweep 정보를 기반으로 DamageImpactInfo를 생성함

ApplyDamage / TakeDamage pipeline
-> DamageImpactInfo를 계산하지 않고 payload/context/packet으로 전달함

UCDamageFeedbackComponent
-> TakeDamagePacket.Context.DamageImpactInfo를 소비해 feedback 위치와 방향을 결정함
```

이렇게 두는 이유는 damage 처리 계층이 collision metadata를 새로 계산하기 시작하면 ApplyDamage / TakeDamage의 책임이 불필요하게 커지기 때문임.

DamageImpactInfo는 hit detection에 가까운 계층에서 만들고, damage pipeline은 전달만 담당하는 것이 적절함.

---

## 5. 충돌 지점 계산

현재 melee hit detection은 weapon collision overlap 기반임.

Overlap 기반 구조에서는 `OnComponentBeginOverlap()`의 `SweepResult`가 항상 의미 있는 impact point를 제공하지 않음.

특히 weapon actor가 socket / attachment / animation transform에 의해 움직이는 경우 `bFromSweep == false`가 자연스럽게 발생할 수 있음.

따라서 현재 구현에서는 다음 순서로 impact point를 결정함.

```text
1. bFromSweep == true이면 SweepResult를 우선 사용함
2. bFromSweep == false이면 GetClosestPointOnCollision() fallback을 사용함
```

`SweepResult`를 사용할 수 있는 경우에는 엔진이 제공한 hit point / impact normal을 우선 사용함.

`SweepResult`가 유효하지 않은 경우에는 target collision 기준의 closest point fallback을 사용함.

Fallback의 의미는 다음과 같음.

```text
Weapon collision center
-> target collision surface closest point
```

이는 정확한 blade contact point는 아니지만, actor location에 VFX를 띄우는 것보다 자연스러운 hit position fallback을 제공함.

다만 이 fallback은 "현재 overlap 구조 안에서 더 나은 근사값"임.

정확한 weapon contact point, weapon normal, bone / physical material / surface direction까지 보장하는 모델은 아님.

---

## 6. 무기 트레일 트레이스

Weapon Trail Trace는 현재 구현 범위에 포함하지 않음.

Trail trace는 단순히 VFX 위치를 바꾸는 작업이 아니라, melee hit detection model과 hit metadata 생성 방식을 바꾸는 작업에 가까움.

개념은 다음과 같음.

```text
weapon 위의 sample point를 정함
각 sample point의 previous position과 current position 사이를 trace함
trace result에서 impact point / normal / bone / surface data를 얻음
```

도입 시 고려할 비용은 다음과 같음.

```text
sample socket 구성
previous location 저장
tick 또는 notify tick trace
duplicate hit target filtering
trace radius / sample count tuning
debug draw / trace channel 정책
fast swing / low frame rate 보정
```

따라서 현재는 overlap 유지 + `FDamageImpactInfo` + closest point fallback을 1차 구현으로 채택함.

Trail trace는 다음 조건이 필요해질 때 별도 hit detection model로 검토함.

```text
guard / parry / weapon clash가 정확한 weapon contact direction에 의존함
hit normal과 weapon swing direction이 gameplay 판정에 사용됨
bone / physical material / surface 기반 feedback이 필요함
fast swing에서 overlap보다 안정적인 contact point가 필요함
```

---

## 7. 현재 구현

현재 구현 기준은 다음과 같음.

```text
ACWeaponActor::BuildDamageImpactInfo()
-> SweepResult 우선 사용함
-> GetClosestPointOnCollision fallback 사용함

UCDamageFeedbackComponent
-> PlayDamageFeedback()
-> ResolveHitFeedbackLocation()
-> ResolveHitFeedbackRotation()
-> PlayHitVFX()
-> PlayHitSFX()
-> PlayHitStop()
-> PlayCameraShake()

UCReactionFeedbackComponent
-> reaction type / damage spec / timing / trigger key 기반 feedback matching 수행함

UCReaction
-> active reaction context 기반으로 reaction feedback request 생성함
```

`DamageFeedback`은 `TakeDamagePacket.Context.DamageImpactInfo`를 직접 소비함.

`ReactionFeedback`은 `FDamageImpactInfo`를 직접 소비하지 않음.

현재 기준에서 impact metadata는 damage feedback의 입력이고, reaction feedback의 입력이 아님.

Reaction feedback이 hit 위치가 필요해지는 경우에는 reaction context에 impact metadata를 추가로 포함할지 별도로 결정해야 함.

---

## 8. 결과

이 결정의 장점은 다음과 같음.

```text
hit feedback과 reaction execution feedback이 섞이지 않음
DamageFeedback이 damage event metadata를 기준으로 동작함
ReactionFeedback이 reaction lifecycle timing을 기준으로 동작함
actor location 기반 hit VFX보다 자연스러운 impact position을 사용할 수 있음
ApplyDamage / TakeDamage 계층이 collision 계산 책임을 갖지 않음
추후 trail trace 도입 시 전달 구조를 크게 바꾸지 않아도 됨
```

주의할 점은 다음과 같음.

```text
ClosestPoint fallback은 정확한 blade contact point가 아님
ImpactNormal은 fallback 기준의 근사값임
정밀한 surface / bone / weapon clash 판정이 필요하면 trail trace가 필요함
ReactionFeedback에서 hit 위치가 필요해지면 별도 context 확장이 필요함
```

---

## 9. 후속 작업

후속 작업 후보는 다음과 같음.

```text
weapon별 trace profile 설계함
trail trace 기반 hit detection model 도입 검토함
DamageFeedback data asset 분리 검토함
ReactionFeedback과 DamageFeedback authoring workflow 정리함
hit normal 방향 정책을 명확히 정의함
```

---

## 10. 관련 문서

관련 상세 문서는 다음과 같음.

```text
A06_UE5_Portfolio_Weapon_Trail_Trace_Model
A08_UE5_Portfolio_Combat_Feedback_Model
```

---
