# UE5 Portfolio Pull Request

## 제목

**P07: ApplyDamage Pipeline 및 TakeDamage Boundary 구현**

## 날짜

**2026.01.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-apply-damage`

---

## 요약

이번 PR에서는 **공격 action montage의 collision window에서 target overlap이 발생했을 때, hit가 발생한 조건을 damage 요청으로 전달하는 흐름을 구현했다.**

damage 요청 생성 흐름과 damage 계산 / 전달 흐름의 책임을 나누고, 계산된 damage 결과는 target의 `TakeDamage()` 경계로 넘기도록 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **collision window 기반 damage 요청 연결**: 공격 montage의 특정 시점에 collision window를 열고, 해당 구간에서 target overlap이 발생하면 damage 요청으로 이어지도록 구성했다.

- **FHitContext 구성**: target overlap이 발생하면 `ACAttachment`에 저장된 cached context를 합쳐 어떤 조건에서 hit가 발생했는지 설명하는 `FHitContext`를 구성했다.

- **ApplyDamage 처리 흐름 구현**: damage 요청을 검증하고, 현재 공격 action에 맞는 damage 설정을 찾은 뒤, 계산된 damage 결과를 target의 `TakeDamage()`로 전달하도록 구성했다.

### Refactoring

- **Context 구성 책임 분리**:
  - `UCAction`은 공격 action 정보를 `FActionContext`로 만들고 `UCWeaponComponent`에 전달한다.
  - `UCWeaponComponent`는 현재 attachment / equipment 상태를 `FAttachmentContext` / `FEquipmentContext`로 만들어 `ACAttachment`에 cache한다.
  - `ACAttachment`는 target overlap이 발생하면 `FOverlapContext`를 만들고 cached context와 결합해 `FHitContext`를 구성한다.

- **damage 계산 경계 분리**: damage 설정 조회, 결과 계산, target 전달을 `UCApplyDamageComponent` 안에서 처리하도록 정리했다.

- **TakeDamage 호출 지점 고정**: target damage 수신은 여러 곳에서 직접 호출하지 않고, `ApplyDamageToTarget()` 경계를 통해서만 전달하도록 정리했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

```text
ApplyDamage Pipeline(damage 송신 흐름)
-> 공격 실행 중 발생한 overlap을 damage 요청으로 바꾸고 target의 `TakeDamage()` 경계까지 전달하는 흐름
```

```text
FAttachmentContext(attachment context)
-> attachment는 부착물(무기)를 의미함
-> 현재 공격에 사용 중인 attachment type을 담는 context
-> damage 설정을 찾을 때 attachment type 기준으로 사용됨
```

```text
FEquipmentContext(equipment context)
-> 현재 공격에 연결된 equipment type을 담는 context
-> P07 당시 damage 설정을 찾을 때 equipment type 기준으로 사용됨
```

```text
FActionContext(action context)
-> 현재 실행 중인 action type과 action index를 담는 context
-> combo attack의 단계별 damage 설정을 찾을 때 action type / action index 기준으로 사용됨
```

```text
FOverlapContext(overlap context)
-> target overlap이 발생한 순간의 attacker / damage causer / target 정보를 담는 context
```

```text
FHitContext(damage 요청에 필요한 타격 정보)
-> target overlap 결과와 현재 attachment / equipment / action context를 합쳐 만든 damage 요청 정보
```

```text
FDamageSpec(damage 설정)
-> attachment, equipment, action type, action index 조합에 따라 찾는 기본 damage 설정
```

```text
FDamageResult(damage 결과)
-> target에게 전달할 최종 damage 값과 attacker / damage causer / target 정보를 담은 결과
```

```text
TakeDamage Boundary(damage 수신 경계)
-> 계산된 damage 결과를 Unreal `TakeDamage()` 호출로 넘기는 마지막 전달 지점
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 구조에서 분리해야 했던 책임을 정리한다.

### Action timing과 damage 요청 연결 필요성

공격 damage는 action montage 전체가 아니라, action montage의 특정 notify timing에서 열린 collision window 안에서만 발생해야 했다.

따라서 notify timing은 collision이 열리는 구간을 만들고, attachment overlap은 실제 hit 발생 여부를 감지하며, damage 처리 component는 overlap 결과를 damage 요청으로 해석하는 구조가 필요했다.

### FHitContext 구성 책임 분리 필요성

Target overlap 결과만으로는 hit가 발생한 시점의 attachment, equipment, action type, action index 같은 overlap 이외의 context를 알 수 없었다.

이를 해결하려면 collision이 발생하기 전에 현재 attachment / equipment / action context를 cache하고, target overlap 발생 시 이 정보와 overlap 결과를 결합해 `FHitContext`를 구성해야 했다.

### ApplyDamage 경계 고정 필요성

Damage 계산과 target의 `TakeDamage()` 호출이 action이나 attachment에 섞이면, 공격 실행 흐름이 target damage 전달 책임까지 직접 가지게 된다.

이번 PR에서는 damage 요청 검증, damage 설정 조회, 결과 계산, target 전달에 대한 책임을 `UCApplyDamageComponent`로 모아 ApplyDamage Pipeline을 분리했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. Action timing 기준 context cache 구성

- **왜**:
  collision window에서 overlap이 발생하기 전에 현재 attachment, equipment, action type / action index를 cache해야 했다.
  이 context가 없으면 overlap 시점에 어떤 공격 조건으로 hit가 발생했는지 알 수 없어 damage 설정을 찾기 어렵다.

- **어떻게**:
  공격 action이 시작되거나 다음 combo 단계로 진입할 때 `UCAction_ComboAttack`이 `FActionContext`를 만들고, `UCWeaponComponent::PushContextToAttachment()`를 호출하도록 구성했다.
  `UCWeaponComponent`는 현재 `FAttachmentContext`, `FEquipmentContext`를 만든 뒤 `FActionContext`와 함께 `ACAttachment`에 cache한다.

- **결과**:
  `ACAttachment`는 target overlap이 발생한 시점에 cached context를 사용해 hit 처리에 필요한 `FHitContext`를 만들 수 있다.

### 2. attachment overlap 기준 FHitContext 구성

- **왜**:
  Target overlap은 타깃 접촉만 알려주므로, damage 요청에는 attacker, damage causer, target 정보와 cached attachment / equipment / action context를 함께 담아야 했다.

- **어떻게**:
  `ACAttachment`가 overlap 발생 시 `FOverlapContext`를 만들고, cached `FAttachmentContext`, `FEquipmentContext`, `FActionContext`와 결합해 `FHitContext`를 구성하도록 했다.

- **결과**:
  damage 요청은 단순 overlap event가 아니라, damage 설정 조회와 결과 계산에 필요한 context를 가진 요청으로 전달된다.

### 3. ApplyDamage Component 진입점 구성

- **왜**:
  damage 요청 검증, damage 설정 조회, 결과 계산, target 전달을 한 곳에서 처리해야 action과 attachment가 damage 내부 처리까지 알 필요가 없었다.

- **어떻게**:
  `UCApplyDamageComponent::RequestApplyDamage()`를 단일 진입점으로 두고, 요청 유효성 검증, hit rule 확인, damage 설정 조회, damage 결과 계산, target 전달을 순서대로 처리하도록 구성했다.

- **결과**:
  action과 attachment는 damage 요청을 만들고 전달하는 역할에 집중하고, 실제 ApplyDamage Pipeline은 `UCApplyDamageComponent`가 소유한다.

### 4. DamageSpec 조회와 DamageResult 계산 분리

- **왜**:
  Damage 값은 단순 overlap에서 바로 정할 수 없고, hit가 발생한 timing의 attachment / equipment / action context 조합에 따라 달라져야 했다.

- **어떻게**:
  `FHitContext`에서 `FDamageSpecKey`를 만들고, `DamageSpecMap`에서 `FDamageSpec`을 조회한 뒤 `FDamageResult`를 계산하도록 분리했다.

- **결과**:
  Damage 설정 조회와 최종 damage 결과 생성이 분리되어, 이후 damage 공식 확장이나 data asset 분리로 이어질 수 있는 기준이 생겼다.

### 5. TakeDamage 호출 경계 고정

- **왜**:
  Target damage 수신은 Unreal `TakeDamage()` 경계를 통해 들어가야 하고, 호출 지점이 흩어지면 수신 측 구현과 연결하기 어려워진다.

- **어떻게**:
  `ApplyDamageToTarget()`에서 `FCustomDamageEvent`에 `FDamageResult`를 담고, `Target->TakeDamage()`를 호출하도록 고정했다.

- **결과**:
  ApplyDamage Pipeline은 송신 측에서 계산한 damage 결과를 target 수신 경계로 전달하고, 실제 HP / reaction / feedback 처리는 이후 수신 측 흐름에서 확장할 수 있게 됐다.

---

## 주요 처리 흐름

이 섹션은 공격 overlap이 damage 요청과 target의 `TakeDamage()` 호출로 이어지는 대표 흐름을 정리한다.

### action context cache 흐름

```text
action montage notify timing
-> 현재 action type / action index 확인
-> FActionContext 생성
-> UCWeaponComponent가 현재 attachment / equipment 상태 확인
-> FAttachmentContext / FEquipmentContext 생성
-> FAttachmentContext / FEquipmentContext / FActionContext를 ACAttachment에 cache
```

이 흐름은 notify timing에서 현재 `FActionContext`와 attachment / equipment context를 기록해, 이후 attachment overlap이 발생했을 때 같은 기준으로 damage 요청을 만들 수 있게 준비하는 과정을 의미한다.

### overlap 기반 damage 요청 흐름

```text
attachment collision enabled
-> target overlap 발생
-> FOverlapContext 생성
-> cached FAttachmentContext / FEquipmentContext / FActionContext 결합
-> FHitContext 생성
-> UCApplyDamageComponent::RequestApplyDamage 호출
```

이 흐름은 target overlap 결과를 damage 요청에 필요한 `FHitContext`로 바꿔 ApplyDamage Pipeline에 전달하는 과정을 의미한다.

### ApplyDamage 처리 흐름

```text
RequestApplyDamage
-> 요청 context 유효성 검증
-> hit rule 확장 지점 통과
-> DamageSpec 조회
-> DamageResult 계산
-> FCustomDamageEvent 구성
-> Target->TakeDamage 호출
```

이 흐름은 damage 요청을 검증하고, 설정된 damage 값을 계산한 뒤 target 수신 경계로 넘기는 과정을 의미한다.

---

## 구현 결과

- action montage notify timing에서 현재 `FActionContext`와 attachment / equipment context를 `ACAttachment`에 cache할 수 있다.

- attachment overlap은 target overlap 결과와 cached `FAttachmentContext` / `FEquipmentContext` / `FActionContext`를 결합해 `FHitContext`를 만든다.

- `UCApplyDamageComponent`는 `FHitContext`를 기준으로 요청을 검증하고, `DamageSpecMap`에서 `FDamageSpec`을 찾는다.

- `FDamageSpec.BaseDamage`는 최소 damage 계산 결과인 `FDamageResult.FinalDamage`로 전달된다.

- Target damage 수신은 `ApplyDamageToTarget()`에서 `FCustomDamageEvent`와 함께 `Target->TakeDamage()`를 호출하는 경계로 고정됐다.

---

## 테스트 방법

### Component / Data 설정

- character에 `UCWeaponComponent`, `UCActionComponent`, `UCApplyDamageComponent`가 구성되어 있는지 확인한다.

- `DamageSpecMap`에 현재 attachment / equipment / action / action index 조합에 맞는 `FDamageSpecKey`가 등록되어 있는지 확인한다.

### action context cache

- 공격 montage에서 action notify가 호출되는지 확인한다.

- `UCAction_ComboAttack::BeginPlayAction()` 또는 `NextPlayAction()`에서 현재 `FActionContext`가 생성되는지 확인한다.

- `UCWeaponComponent::PushContextToAttachment()`를 통해 `FAttachmentContext`, `FEquipmentContext`, `FActionContext`가 `ACAttachment`에 cache되는지 확인한다.

### overlap / FHitContext

- collision notify 구간에서 attachment collision이 열리고 닫히는지 확인한다.

- target과 overlap 발생 시 `ACAttachment::OnComponentBeginOverlap()`이 호출되는지 확인한다.

- `FHitContext`가 `FOverlapContext`, `FAttachmentContext`, `FEquipmentContext`, `FActionContext` 조합으로 구성되는지 확인한다.

### ApplyDamage 처리

- `RequestApplyDamage -> ValidateRequest -> CheckHitRule -> ResolveDamageSpec -> ComputeDamageResult -> ApplyDamageToTarget` 순서로 처리되는지 확인한다.

- invalid owner, self-hit, invalid component 상황에서 early return되는지 확인한다.

- `FDamageSpecKey` 기준으로 `FDamageSpec`을 조회하고, `BaseDamage`가 `FinalDamage`로 전달되는지 확인한다.

### TakeDamage 경계

- `ApplyDamageToTarget()`에서만 `Target->TakeDamage()`가 호출되는지 확인한다.

- `FCustomDamageEvent`에 `FDamageResult`가 포함되어 target으로 전달되는지 확인한다.

- 로그에서 request damage와 applied damage가 출력되는지 확인한다.

---

## 검증 결과

- action montage notify timing에서 `FActionContext`가 attachment로 전달되는 흐름을 확인했다.

- attachment overlap 발생 시 `FHitContext`가 구성되고 `RequestApplyDamage()`로 전달되는 흐름을 확인했다.

- `DamageSpecMap` 기반 `FDamageSpec` 조회와 `BaseDamage -> FinalDamage` 최소 계산 흐름을 확인했다.

- `ApplyDamageToTarget()` 경유 `Target->TakeDamage()` 호출 흐름을 확인했다.

- overlap end 시점에는 `RequestStopDamage()` 호출 경로만 두고, 지속 damage / 반복 hit 해제 같은 세부 처리는 후속 확장 지점으로 남겼다.

---

## 비범위

- Target 수신 이후의 HP 반영, reaction, feedback 처리는 이번 PR에서 구현하지 않는다.

- `CheckHitRule()`은 already-hit, team check 확장을 위한 판단 지점으로 두었고, 이번 PR에서는 실제 중복 hit 방지 정책을 완성하지 않는다.

- `RequestStopDamage()`는 overlap end 기반 확장 지점으로 두었고, 지속 damage나 repeated hit timer 해제 정책은 이번 PR 범위에 포함하지 않는다.

- DamageSpec의 data asset 분리는 후속 확장 범위로 남긴다.

---

## 관련 문서

- Issue Checklist: `D08_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

- P07은 action montage notify timing과 attachment overlap을 연결해, 공격 실행 중 발생한 hit를 ApplyDamage 요청으로 변환하고 target의 `TakeDamage()` 경계까지 전달하는 PR이다.

- `UCApplyDamageComponent`가 damage 요청 검증, damage 설정 조회, damage 결과 계산, target의 `TakeDamage()` 호출을 담당하도록 정리했다.

- 수신 측 HP / reaction / feedback 처리와 중복 hit 정책은 이후 브랜치에서 확장할 수 있도록 경계를 남겼다.
