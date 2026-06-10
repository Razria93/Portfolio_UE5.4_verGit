# UE5 Portfolio Pull Request

## 제목

**P06: Hit Collision 시스템 구현 및 Collision Window 도입**

## 날짜

**2025.12.25**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-hit-collision`

---

## 요약

### 작업 요약

본 PR은 montage timing에 맞춰 무기 collision을 열고 닫으며, overlap event를 action 계층까지 전달하는 첫 번째 hit collision pipeline을 구성한 작업이다.

```yaml
Action Montage
-> UCAnimNotify_Collision 호출
-> ACAttachment collision 활성화 / 비활성화
-> UShapeComponent overlap 발생
-> ACAttachment overlap event 구성
-> UCWeaponComponent delegate binding
-> UCAction overlap callback 전달
```

### 작업 배경

전투 공격은 montage의 특정 timing에서만 hit 판정이 발생해야 한다.

이를 위해 attack montage와 collision을 직접 연결할 수 있는 collision window가 필요했다.

 그리고 collision을 발생시키는 객체인 `ACAttachment`가 실제 overlap event를 수신하도록 구성할 필요가 있었다.

또한 후속 damage pipeline에서 overlap 결과를 사용할 수 있도록, attachment에서 발생한 overlap을 action 계층으로 전달하는 event routing 경계를 먼저 마련해야 했다.

```yaml
필요한 기준
- montage timing 기준 collision window 제어
- attachment 내부 UShapeComponent 자동 탐색 / 저장
- owner self-overlap 차단
- attachment overlap event를 UCAction까지 전달
- 후속 damage 처리로 확장 가능한 callback 경계 확보
```

### 구현 방향

```yaml
1. Dummy Target 구성
- hit collision 테스트용 ACEnemy 추가

2. Collision Window 구성
- UCAnimNotify_Collision에서 montage timing 기준으로 collision enable / disable 호출

3. Attachment Overlap 처리
- ACAttachment가 UShapeComponent overlap event를 수신하고 유효한 hit event로 정리

4. Action Routing 구성
- UCWeaponComponent가 ACAttachment delegate를 UCAction callback으로 연결
```

---
## 변경 범위

### Hit Collision Pipeline

#### A. Dummy Target 추가

- 테스트 레벨에서 overlap 기반 hit collision을 확인할 수 있도록 최소 enemy target을 추가했다.

**Structure**
```yaml
ACEnemy
- ACharacter 기반 dummy target
- UCStateComponent 보유
- mesh / capsule / movement 기본값 구성
- 테스트 레벨 배치용 target 역할
```

#### B. Attachment Collision 저장 및 기본 비활성화

- `ACAttachment`가 root 하위 component 중 `UShapeComponent`를 찾아 저장하고, 시작 시 collision을 비활성화하도록 구성했다.

**Flow**
```yaml
ACAttachment::BeginPlay
-> OwnerCharacter_Cached 저장
-> Root 하위 component 순회
-> UShapeComponent 캐스팅
-> overlap delegate binding
-> SetCollisionEnabled(NoCollision) 호출
-> Collisions_Cached에 저장
```

**Structure**
```yaml
ACAttachment
- OwnerCharacter_Cached : attachment owner character
- Collisions_Cached     : attack collision으로 사용할 UShapeComponent 목록
```

#### C. AnimNotify 기반 Collision Window 제어

- attack montage의 notify timing에 따라 attachment collision을 열고 닫는 `UCAnimNotify_Collision`을 추가했다.

**Flow**
```yaml
Action Montage Notify
-> UCAnimNotify_Collision::Notify 호출
-> GetWeaponComponent로 UCWeaponComponent 조회
-> UCWeaponComponent::GetAttachment 호출
-> FlowType에 따라 collision 제어
```

**Structure**
```yaml
UCAnimNotify_Collision
- FlowType      : Begin / End 구분
- CollisionName : 특정 collision 선택 활성화용 이름

EAnimNotifyFlow::Begin
- ACAttachment::CollisionEnabled 호출

EAnimNotifyFlow::End
- ACAttachment::CollisionDisabled 호출
```

#### D. CollisionName 기반 선택 활성화

- 하나의 attachment에 여러 collision shape가 존재할 수 있으므로, notify에서 지정한 `CollisionName`과 일치하는 collision만 활성화할 수 있도록 구성했다.

**Flow**
```yaml
ACAttachment::CollisionEnabled
-> CollisionName 존재 여부 확인
-> CollisionName이 있으면 이름이 같은 UShapeComponent만 활성화
-> CollisionName이 없으면 Collisions_Cached 전체 활성화
-> OnAttachmentCollisionEnabled broadcast
```

#### E. Attachment Overlap Event 구성

- collision window 중 발생한 overlap을 attachment 기준 event로 정리하고, owner self-overlap은 무시하도록 구성했다.

**Flow**
```yaml
UShapeComponent::OnComponentBeginOverlap
-> attacker actor 구성
-> damage causer 구성
-> attack collision 구성
-> target actor 구성
-> hit component 구성
-> owner self-overlap 검증
-> OnAttachmentBeginOverlap broadcast
```

**Structure**
```yaml
OnAttachmentBeginOverlap
- InAttackerActor  : 공격 주체
- InDamageCauser   : damage 원인 actor
- InAttackCollision: overlap이 발생한 attack collision
- InTargetActor    : overlap 대상 actor
- InHitComponent   : overlap 대상 component

OnAttachmentEndOverlap
- InAttackerActor : 공격 주체
- InTargetActor   : overlap 종료 대상 actor
```

#### F. Attachment Event에서 Action Callback으로 Routing

- `UCWeaponComponent`가 attachment delegate를 action callback에 연결하여, overlap 결과를 `UCAction`에서 후속 처리할 수 있도록 구성했다.

**Flow**
```yaml
UCWeaponComponent::BeginPlay
-> ACAttachment 생성 / 초기화
-> UCAction 생성 / 초기화
-> ACAttachment delegate binding
-> UCAction callback 연결
```

**Structure**
```yaml
Collision Window Event
- ACAttachment::OnAttachmentCollisionEnabled  -> UCAction::OnAttachmentCollisionEnabled
- ACAttachment::OnAttachmentCollisionDisabled -> UCAction::OnAttachmentCollisionDisabled

Overlap Event
- ACAttachment::OnAttachmentBeginOverlap -> UCAction::OnAttachmentBeginOverlap
- ACAttachment::OnAttachmentEndOverlap   -> UCAction::OnAttachmentEndOverlap
```

---
## 주요 Pipeline

### Collision Window Pipeline

```yaml
Action Montage
-> UCAnimNotify_Collision
-> UCWeaponComponent::GetAttachment
-> ACAttachment::CollisionEnabled / CollisionDisabled
-> UShapeComponent collision 상태 변경
```

### Attachment Overlap Pipeline

```yaml
UShapeComponent overlap
-> ACAttachment::OnComponentBeginOverlap
-> overlap actor / component 검증
-> owner self-overlap 차단
-> OnAttachmentBeginOverlap broadcast
-> UCAction::OnAttachmentBeginOverlap
```

### Attachment Event Routing Pipeline

```yaml
ACAttachment delegate
-> UCWeaponComponent binding
-> UCAction callback
-> 후속 hit data 구성 / damage 처리 확장 지점
```

---
## 테스트 방법

### Dummy Target

- 테스트 레벨에 `ACEnemy`를 배치하고 overlap 대상 actor로 인식되는지 확인
- enemy mesh / capsule / movement 기본값이 테스트 가능한 상태인지 확인

### Collision Window

- attack montage에서 `UCAnimNotify_Collision` Begin timing에 collision이 활성화되는지 확인
- attack montage에서 `UCAnimNotify_Collision` End timing에 collision이 비활성화되는지 확인
- `CollisionName`이 지정된 경우 해당 이름의 `UShapeComponent`만 활성화되는지 확인
- `CollisionName`이 없는 경우 저장된 collision 전체가 활성화되는지 확인

### Overlap Routing

- collision window 내부에서 target과 overlap 시 `ACAttachment::OnComponentBeginOverlap`이 호출되는지 확인
- owner self-overlap이 무시되는지 확인
- `UCWeaponComponent` binding을 통해 `UCAction::OnAttachmentBeginOverlap`까지 전달되는지 확인
- collision disabled 이후 불필요한 overlap event가 발생하지 않는지 확인

---
## 검증 결과

- `ACEnemy` dummy target을 통해 테스트 레벨에서 overlap 진입 확인
- `UCAnimNotify_Collision` Begin / End timing 기준으로 collision enable / disable 동작 확인
- `ACAttachment` 내부 `UShapeComponent` 자동 탐색 및 overlap delegate binding 확인
- owner self-overlap 차단 확인
- attachment overlap event가 `UCAction` callback까지 전달되는 routing 확인

---
## 관련 문서

- Issue Checklist: `D07_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 montage timing에서만 hit collision이 열리도록 collision window를 구성하고, attachment overlap event를 action 계층까지 전달하는 기반을 마련한 것이다.

변경 후에는 `UCAnimNotify_Collision`이 montage timing을 전달하고, `ACAttachment`가 실제 collision 상태와 overlap event를 관리하며, `UCWeaponComponent`가 attachment event를 `UCAction` callback으로 연결했다.

```yaml
UCAnimNotify_Collision
- montage timing 기준 collision window 제어

ACAttachment
- UShapeComponent 저장
- collision enable / disable 적용
- overlap event 구성
- owner self-overlap 차단

UCWeaponComponent
- ACAttachment delegate와 UCAction callback 연결

UCAction
- attachment event 수신
- 후속 hit data 구성 / damage 처리 확장 지점 제공
```

이 브랜치에서는 실제 damage apply, duplicate hit 방지, target 상태 필터링, reaction 처리는 직접 구현하지 않았고, 해당 기능들은 이후 apply damage / take damage pipeline에서 확장할 수 있는 경계로 남겼다.

---
