# UE5 Portfolio Pull Request

## 제목

**P07: Action Execution Pipeline 및 ApplyDamage Pipeline 구현**

## 날짜

**2026.01.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-apply-damage`

---

## 요약

### 작업 요약

본 PR은 action 실행 중 발생한 attachment overlap을 target actor의 Unreal `TakeDamage()` 호출로 연결하는 ApplyDamage 송신 pipeline을 구성한 작업이다.

```yaml
Action Montage
-> AnimNotify timing 도달
-> Attachment에 context 저장
-> Attachment overlap 발생
-> FHitContext 구성
-> UCApplyDamageComponent::RequestApplyDamage 호출
-> Target->TakeDamage 호출
```

### 작업 배경

action damage 송신 흐름에서는 collision 감지, damage context 수집, damage 계산, target `TakeDamage()` 호출 경계를 함께 정리해야 한다.

이 처리를 character나 attachment 내부에 직접 구현하면 한 객체가 apply damage pipeline 전체를 소유하게 되어 책임이 비대해질 수 있다.

따라서 다음과 같이 책임을 분리하고자 한다.

```yaml
UCAction
- montage timing 처리
- action context 구성

UCWeaponComponent
- attachment 접근 경계
- attachment / equipment context 구성

ACAttachment
- collision overlap 수신
- FOverlapContext와 cached action context 결합
- hit context 구성

UCApplyDamageComponent
- apply damage request 검증
- damage spec 조회
- damage result 계산
- target TakeDamage 호출
```

collision은 weapon attachment의 collision component를 montage notify 구간에서 켜고, overlap event를 기준으로 감지함.

다만 attachment가 action / equipment / character 상태를 직접 조회하면 결합도가 높아지므로, action notify timing에서 필요한 context를 `UCWeaponComponent`를 통해 attachment에 미리 push하도록 구성했다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Collision 감지 기준 정리
- montage notify 구간에서 attachment collision을 켜고 overlap event를 사용

2. Context Push 구조 구성
- action timing에 필요한 context를 attachment에 저장

3. Attachment overlap 기반 HitContext 구성
- FOverlapContext와 cached action context를 결합하여 FHitContext 구성

4. TakeDamage 호출 경계 고정
- UCApplyDamageComponent::ApplyDamageToTarget에서만 Target->TakeDamage 호출
```

---
## 변경 범위

### Action Execution / ApplyDamage Pipeline

#### A. AnimNotify 기반 Action Context Push

- action montage notify timing에서 ApplyDamage에 필요한 action context를 attachment에 미리 push하도록 구성했다.

**Flow**
```yaml
Action Montage
-> UCAnimNotify_Action
-> UCActionComponent에서 active action 조회
-> active action의 BeginPlayAction / NextPlayAction 호출
-> UCAction_ComboAttack에서 FActionContext 구성
-> UCWeaponComponent::PushContextToAttachment 호출
-> ACAttachment에 Attachment / Equipment / Action context 저장
```

**Structure**
```yaml
FActionContext
- CurrentActionType : 현재 action type
- ActionIndex       : 현재 action index

Cached action context in ACAttachment
- FAttachmentContext : 현재 attachment type
- FEquipmentContext  : 현재 equipment type
- FActionContext     : 현재 action type / action index
```

#### B. Attachment Overlap / HitContext 구성

- attachment collision overlap이 발생하면 `FOverlapContext`와 cached action context를 결합하여 `FHitContext`를 구성했다.

**Flow**
```yaml
Attachment Collision Overlap
-> ACAttachment::BuildOverlapContext 호출
-> FHitContext 구성
-> UCApplyDamageComponent::RequestApplyDamage 호출
```

**Structure**
```yaml
FOverlapContext
- OwnerActor          : 공격자 actor
- DamageCauser        : damage causer attachment
- OverlappedComponent : overlap을 발생시킨 attachment collision
- OverlapShape        : shape collision cast 결과
- OtherActor          : 피격 대상 actor
- OtherComponent      : 피격 component
- OtherBodyIndex      : overlap body index
- bFromSweep          : sweep 기반 overlap 여부
- SweepResult         : sweep hit result

FHitContext
- OverlapContext
- AttachmentContext
- EquipmentContext
- ActionContext
```

#### C. UCApplyDamageComponent 진입점 구성

- `UCApplyDamageComponent`를 추가하고, apply damage 요청을 `RequestApplyDamage()` 한 지점으로 받도록 구성했다.

**Flow**
```yaml
UCApplyDamageComponent::RequestApplyDamage
-> ProcessApplyDamage
-> ValidateRequest
-> CheckHitRule
-> ResolveDamageSpec
-> ComputeDamageResult
-> ApplyDamageToTarget
```

**Structure**
```yaml
UCApplyDamageComponent
- RequestApplyDamage : apply damage 외부 진입점
- ProcessApplyDamage : 검증 / spec 조회 / 계산 / 적용 처리 흐름
- RequestStopDamage  : overlap end 기반 지속 효과 처리를 위한 확장 지점
```

#### D. ApplyDamage Request Validation

- `ValidateRequest()`에서 hit context의 actor / component / ownership 관계를 검증하고 invalid request를 조기 반환하도록 구성했다.

**Validation Gate**
```yaml
ValidateRequest
- overlap context 최소 유효성 검증
- component owner와 overlap owner 일치 여부 확인
- self-hit 차단
- overlapped component / other component 유효성 확인
- overlap shape 유효성 확인
- damage causer ownership 확인
- overlapped component ownership 확인
- target component ownership 확인
```

#### E. DamageSpec Resolve

- hit context를 기반으로 `FDamageSpecKey`를 구성하고, `DamageSpecMap`에서 damage spec을 조회했다.

**Flow**
```yaml
FHitContext
-> BuildSpecKey로 FDamageSpecKey 구성
-> DamageSpecMap에서 FDamageSpec 조회
```

**Structure**
```yaml
FDamageSpecKey
- AttachmentType
- EquipmentType
- ActionType
- ActionIndex

FDamageSpec
- BaseDamage
```

#### F. DamageResult 연산

- damage 계산은 target 상태를 직접 변경하지 않고, `FDamageResult`만 구성하는 단계로 분리했다.

**Flow**
```yaml
FHitContext + FDamageSpec
-> ComputeDamageResult 호출
-> OverlapContext에서 Attacker / DamageCauser / Target 조회
-> actor 유효성 확인
-> FDamageSpec.BaseDamage를 FinalDamage로 계산
-> FDamageResult 구성
```

**Structure**
```yaml
FDamageResult
- FinalDamage  : target의 TakeDamage에 전달할 최종 damage amount
- Attacker     : 공격을 수행한 actor
- DamageCauser : damage를 발생시킨 attachment actor
- Target       : damage를 받을 actor
```

#### G. Target TakeDamage 호출 경계

- 최종 damage 적용은 `ApplyDamageToTarget()` 한 지점에서만 수행하고, target actor의 Unreal `TakeDamage()` entry로 전달했다.

**Flow**
```yaml
ApplyDamageToTarget
-> attacker pawn controller 조회
-> FCustomDamageEvent 구성
-> Target->TakeDamage(FinalDamage, FCustomDamageEvent, InstigatorController, DamageCauser) 호출
```

**Structure**
```yaml
FCustomDamageEvent
- DamageResult : target TakeDamage로 전달할 damage result
```

#### H. Collision End / StopDamage Extension Point

- attachment overlap end 시점에는 `RequestStopDamage()`를 호출하도록 경로만 열어두고, 지속 damage / overlap 유지형 효과 처리는 후속 확장 지점으로 남겼다.

**Flow**
```yaml
Attachment Collision EndOverlap
-> FHitContext 구성
-> UCApplyDamageComponent::RequestStopDamage
```

**Extension Point**
```yaml
RequestStopDamage
- active overlap set 제거
- repeated hit timer 정리
- sustained effect 해제
```

---
## 주요 Pipeline

### Action Context Push Pipeline

```yaml
Action Montage
-> UCAnimNotify_Action
-> active action의 BeginPlayAction / NextPlayAction
-> FActionContext 구성
-> UCWeaponComponent::PushContextToAttachment
-> ACAttachment cached action context 갱신
```

### Overlap to ApplyDamage Pipeline

```yaml
Collision Notify
-> ACAttachment::CollisionEnabled
-> Attachment Collision Overlap 발생
-> FOverlapContext 구성
-> FHitContext 구성
-> UCApplyDamageComponent::RequestApplyDamage
```

### ApplyDamage Processing Pipeline

```yaml
RequestApplyDamage
-> ValidateRequest
-> CheckHitRule
-> ResolveDamageSpec
-> ComputeDamageResult
-> ApplyDamageToTarget
```

### TakeDamage Boundary Pipeline

```yaml
ApplyDamageToTarget
-> FCustomDamageEvent
-> Target->TakeDamage
```

---
## 테스트 방법

### Component / Data Setup

- 캐릭터에 `UCWeaponComponent`, `UCActionComponent`, `UCApplyDamageComponent`가 구성되어 있는지 확인
- `DamageSpecMap`에 current attachment / equipment / action / index 조합에 맞는 `FDamageSpecKey`가 등록되어 있는지 확인

### Action Context Push

- 공격 montage에서 `UCAnimNotify_Action(Begin / Next)`가 호출되는지 확인
- `UCAction_ComboAttack::BeginPlayAction()` 또는 `NextPlayAction()`에서 `PushContextToAttachment()`가 호출되는지 확인
- attachment에 `LastAttachmentContext / LastEquipmentContext / LastActionContext`가 저장되는지 로그로 확인

### Overlap / HitContext

- collision notify 구간에서 `ACAttachment::CollisionEnabled()`와 `CollisionDisabled()`가 호출되는지 확인
- target과 overlap 발생 시 `ACAttachment::OnComponentBeginOverlap()`이 호출되는지 확인
- `FHitContext`가 `OverlapContext + AttachmentContext + EquipmentContext + ActionContext` 조합으로 구성되는지 확인

### ApplyDamage Processing

- `RequestApplyDamage -> ValidateRequest -> ResolveDamageSpec -> ComputeDamageResult -> ApplyDamageToTarget` 순서로 처리되는지 확인
- invalid owner / self-hit / invalid component 상황에서 early return되는지 확인
- `FDamageSpecKey` 기준으로 `FDamageSpec`이 조회되고, `BaseDamage`가 `FinalDamage`로 전달되는지 확인

### TakeDamage Boundary

- `ApplyDamageToTarget()`에서만 `Target->TakeDamage()`가 호출되는지 확인
- `FCustomDamageEvent`에 `FDamageResult`가 포함되어 target으로 전달되는지 확인
- 로그에서 request damage와 applied damage가 출력되는지 확인

---
## 검증 결과

- action notify timing에서 action context push 동작 확인
- attachment overlap 시 `FHitContext` 구성 확인
- `DamageSpecMap` 기반 `FDamageSpec` 조회 확인
- `BaseDamage -> FinalDamage` 최소 damage 계산 확인
- `ApplyDamageToTarget()` 경유 `Target->TakeDamage()` 호출 확인
- overlap end 시 `RequestStopDamage()` 확장 지점 호출 경로 확인

---
## 관련 문서

- Issue Checklist: `D08_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 action montage timing과 attachment overlap을 결합하여, target `TakeDamage()`까지 이어지는 첫 번째 damage 송신 pipeline을 구성한 것이다.

구성 후에는 `UCAction`이 damage 적용을 직접 수행하지 않고, montage timing에 맞춰 `FActionContext`를 구성하는 역할에 집중했다.

`UCWeaponComponent`는 현재 attachment / equipment 상태를 함께 모아 `ACAttachment`에 context를 저장하고, `ACAttachment`는 `FOverlapContext`와 cached action context를 결합해 `FHitContext`를 구성했다.

`UCApplyDamageComponent`는 이 `FHitContext`를 기준으로 request 검증, damage spec 조회, damage result 계산, target `TakeDamage()` 호출을 담당한다.

```yaml
UCAction
- montage timing
- FActionContext 구성

UCWeaponComponent
- attachment / equipment context 구성
- context push 경계

ACAttachment
- collision overlap 수신
- FOverlapContext와 cached action context 결합
- FHitContext 구성

UCApplyDamageComponent
- apply damage request 검증
- damage spec 조회
- damage result 계산
- Target->TakeDamage 호출
```

이 구조를 통해 action montage에서 시작된 공격 판정이 `FHitContext -> ApplyDamage -> TakeDamage`로 이어지는 기본 송신 흐름을 갖추게 됐다.

수신 측 health commit / reaction / feedback 처리는 이 브랜치에서 직접 구현하지 않고, `Target->TakeDamage()` 이후 단계에서 확장할 수 있는 경계로 남겼다.

---
