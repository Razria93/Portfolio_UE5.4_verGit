# UE5 Portfolio Pull Request

## 제목

**P03: Attachment Object 및 Equipment Pipeline 구현**

## 날짜

**2025.12.17**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/character-weapon-equip`

---

## 요약

### 작업 요약

본 PR은 Player의 sword input을 weapon component로 전달하고, montage notify timing에 맞춰 weapon attachment를 hand / holster socket 사이에서 전환하는 equipment pipeline을 구성한 작업이다.

```yaml
Sword input
-> ACPlayerController::PressSword 호출
-> ACPlayer::HandleSword 호출
-> UCWeaponComponent::SetSwordMode / SetUnarmedMode 호출
-> UCEquipment::Equip / Unequip 호출
-> equipment montage 재생
-> UCAnimNotify_Equip / UCAnimNotify_Unequip 호출
-> UCEquipment begin / end timing 처리
-> ACAttachment socket 전환
```

### 작업 배경

무기 장착 / 해제는 단순히 weapon type flag만 바꾸는 기능이 아니라, montage timing에 맞춰 무기 mesh가 hand socket과 holster socket 사이에서 전환되어야 하는 기능이다.

따라서 input handling, weapon mode transition, equipment lifecycle, attachment socket transition, animation notify timing을 각각 분리해서 구성할 필요가 있었다.

또한 장착 / 해제 중 movement policy와 character state도 함께 제어해야 하므로, `UCEquipment`가 montage 재생과 state / movement 전환을 함께 관리하도록 정리했다.

```yaml
필요한 기준
- Sword input에서 equipment execution까지의 호출 경로 구성
- weapon type과 equipment montage 실행 분리
- hand / holster socket 기반 attachment 전환
- montage notify timing 기준 begin / end 처리
- equip / unequip 중 state / movement policy 제어
```

### 구현 방향

```yaml
1. Weapon Mode 구성
- UCWeaponComponent에서 Unarmed / Sword mode 전환 관리

2. Equipment Lifecycle 구성
- UCEquipment에서 Equip / Unequip montage와 state / movement policy 처리

3. Attachment Socket 전환
- ACAttachment가 hand / holster socket으로 재부착

4. Notify Timing 연결
- UCAnimNotify_Equip / UCAnimNotify_Unequip으로 begin / end timing 전달
```

---
## 변경 범위

### Equipment Pipeline

#### A. Sword Input Routing 구성

- Player sword input을 player character와 weapon component를 거쳐 equipment execution으로 전달하도록 구성했다.

**Flow**
```yaml
InputComponent Sword input
-> ACPlayerController::PressSword 호출
-> ACPlayer::HandleSword 호출
-> Idle state 확인
-> 현재 weapon type 확인
-> UCWeaponComponent::SetSwordMode / SetUnarmedMode 호출
```

**Structure**
```yaml
ACPlayerController
- Sword input binding
- PressSword에서 pawn으로 input 전달

ACPlayer
- HandleSword에서 Idle state 확인
- weapon type 기준으로 equip / unequip 요청

UCWeaponComponent
- CurrentWeaponType 관리
- SetSwordMode / SetUnarmedMode entry 제공
```

#### B. Weapon Type / State Type 변경 흐름

- weapon type과 character state 변경을 component와 delegate 기준으로 관리하도록 구성했다.

**Flow**
```yaml
UCWeaponComponent::ChangeWeaponType
-> PreviousWeaponType 저장
-> CurrentWeaponType 갱신
-> OnWeaponTypeChanged broadcast
-> UCAnimInstance::OnWeaponTypeChanged에서 AnimBP 변수 갱신
```

```yaml
UCStateComponent::ChangeStateType
-> PreviousStateType 저장
-> CurrentStateType 갱신
-> OnStateTypeChanged broadcast
```

**Structure**
```yaml
EWeaponType
- Unarmed
- Sword

EStateType
- Idle
- Equip
- Unequip
```

#### C. FEquipmentData 추가

- equipment montage 실행에 필요한 montage data와 movement policy를 `FEquipmentData`로 분리했다.

**Structure**
```yaml
FEquipmentData
- Montage  : equip / unequip montage
- PlayRate : montage 재생 속도
- bCanMove : equipment execution 중 movement 허용 여부
```

#### D. UCEquipment Lifecycle 구성

- `UCEquipment`가 equip / unequip montage 재생, state 전환, movement policy 적용을 담당하도록 구성했다.

**Flow**
```yaml
UCEquipment::Equip
-> StateComp_Cached->SetEquipMode 호출
-> EquipmentData_Cached.bCanMove 확인
-> 필요한 경우 MovementComp_Cached->SetStop 호출
-> EquipmentData_Cached.Montage 재생
```

```yaml
UCEquipment::Unequip
-> StateComp_Cached->SetUnequipMode 호출
-> UnquipmentData_Cached.bCanMove 확인
-> 필요한 경우 MovementComp_Cached->SetStop 호출
-> UnquipmentData_Cached.Montage 재생
```

**Structure**
```yaml
UCEquipment
- OwnerCharacter_Cached  : equipment owner
- MovementComp_Cached    : movement policy 적용 대상
- StateComp_Cached       : equipment state 전환 대상
- EquipmentData_Cached   : equip montage data
- UnquipmentData_Cached  : unequip montage data
- bBeginEquip            : equip begin notify 수신 여부
- bBeginUnequip          : unequip begin notify 수신 여부
- bEquipped              : 장착 완료 여부
```

#### E. ACAttachment Socket 전환 구성

- `ACAttachment`가 owner mesh의 hand / holster socket으로 재부착되도록 구성했다.

**Flow**
```yaml
ACAttachment::InitializeAttachment
-> SocketName_Holster로 AttachToOwnerSocket 호출

UCEquipment::Begin_Equip
-> OnEquipmentBeginEquip broadcast
-> ACAttachment::OnEquipmentBeginEquip 호출
-> SocketName_Hand로 AttachToOwnerSocket 호출

UCEquipment::Begin_Unequip
-> OnEquipmentBeginUnequip broadcast
-> ACAttachment::OnEquipmentBeginUnequip 호출
-> SocketName_Holster로 AttachToOwnerSocket 호출
```

**Structure**
```yaml
ACAttachment
- SocketName_Holster     : 보관 위치 socket
- SocketName_Hand        : 장착 위치 socket
- OwnerCharacter_Cached  : attachment owner
- AttachToOwnerSocket    : owner mesh socket 재부착 API
```

#### F. AnimNotify 기반 Equip / Unequip Timing 연결

- montage notify timing에서 equipment begin / end를 전달하는 notify를 추가했다.

**Flow**
```yaml
UCAnimNotify_Equip::Notify
-> GetWeaponComponent로 UCWeaponComponent 조회
-> UCWeaponComponent::GetEquipment 호출
-> FlowType에 따라 equip timing 전달
```

```yaml
UCAnimNotify_Unequip::Notify
-> GetWeaponComponent로 UCWeaponComponent 조회
-> UCWeaponComponent::GetEquipment 호출
-> FlowType에 따라 unequip timing 전달
```

**Structure**
```yaml
EAnimNotifyFlow::Begin
- UCEquipment::Begin_Equip / Begin_Unequip 호출

EAnimNotifyFlow::End
- UCEquipment::End_Equip / End_Unequip 호출
```

#### G. Equipment 종료 처리

- montage end notify에서 movement policy를 복구하고 state를 Idle로 되돌리도록 구성했다.

**Flow**
```yaml
UCEquipment::End_Equip
-> bBeginEquip false로 갱신
-> bEquipped true로 갱신
-> 필요한 경우 MovementComp_Cached->SetMove 호출
-> OnEquipmentEndEquip broadcast
-> StateComp_Cached->SetIdleMode 호출
```

```yaml
UCEquipment::End_Unequip
-> bBeginUnequip false로 갱신
-> bEquipped false로 갱신
-> 필요한 경우 MovementComp_Cached->SetMove 호출
-> OnEquipmentEndUnequip broadcast
-> StateComp_Cached->SetIdleMode 호출
```

---
## 안정성 보완

### FEquipmentData Editor 안정성 보완 (B02 보완)

#### A. Editor-Facing Struct 정리

- `FEquipmentData`를 editor details panel에서 안전하게 편집할 수 있도록 `USTRUCT(BlueprintType)` / `UPROPERTY` 기준으로 정리했다.
- 자세한 원인과 검증 내용은 `B02` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
FEquipmentData
-> USTRUCT(BlueprintType) 적용
-> field를 UPROPERTY로 노출
-> InitializeEquipment에서 값 전달 방식 사용
-> details panel / reflection 경로의 reference lifetime 문제 회피
```

**Structure**
```yaml
FEquipmentData
- Montage  : editor에서 지정할 equipment montage
- PlayRate : editor에서 지정할 montage 재생 속도
- bCanMove : editor에서 지정할 movement policy
```

---
## 주요 Pipeline

### Equipment Input Pipeline

```yaml
Sword input
-> ACPlayerController::PressSword
-> ACPlayer::HandleSword
-> Idle state 확인
-> UCWeaponComponent::SetSwordMode / SetUnarmedMode
```

### Equip Pipeline

```yaml
UCWeaponComponent::SetSwordMode
-> UCEquipment::Equip
-> StateComp_Cached->SetEquipMode
-> movement policy 적용
-> equip montage 재생
-> UCAnimNotify_Equip(Begin)
-> ACAttachment hand socket 재부착
-> UCAnimNotify_Equip(End)
-> movement policy 복구
-> StateComp_Cached->SetIdleMode
```

### Unequip Pipeline

```yaml
UCWeaponComponent::SetUnarmedMode
-> UCEquipment::Unequip
-> StateComp_Cached->SetUnequipMode
-> movement policy 적용
-> unequip montage 재생
-> UCAnimNotify_Unequip(Begin)
-> ACAttachment holster socket 재부착
-> UCAnimNotify_Unequip(End)
-> movement policy 복구
-> StateComp_Cached->SetIdleMode
```

### Attachment Socket Pipeline

```yaml
UCEquipment begin notify
-> equipment begin delegate broadcast
-> ACAttachment delegate callback
-> AttachToOwnerSocket 호출
-> owner mesh socket 재부착
```

---
## 테스트 방법

### Input / Entry

- `Sword` input이 `ACPlayerController::PressSword`에 binding 되어 있는지 확인
- `ACPlayer::HandleSword`에서 Idle state일 때만 equip / unequip으로 이어지는지 확인
- weapon type이 `Unarmed`이면 `SetSwordMode`, `Sword`이면 `SetUnarmedMode`가 호출되는지 확인

### Equip / Unequip Execution

- `SetSwordMode` 호출 시 equip montage가 재생되는지 확인
- `SetUnarmedMode` 호출 시 unequip montage가 재생되는지 확인
- equip / unequip 중 `EStateType::Equip / Unequip` state로 전환되는지 확인
- notify end 이후 Idle state로 복귀하는지 확인

### Attachment Socket

- 시작 시 weapon attachment가 holster socket에 부착되는지 확인
- `UCAnimNotify_Equip(Begin)` timing에 hand socket으로 전환되는지 확인
- `UCAnimNotify_Unequip(Begin)` timing에 holster socket으로 전환되는지 확인

### Movement Policy

- `FEquipmentData::bCanMove == false`인 경우 montage 시작 시 movement가 제한되는지 확인
- equip / unequip end timing 이후 movement가 복구되는지 확인

### Editor 안정성

- Details Panel에서 `FEquipmentData`의 Montage / PlayRate / bCanMove 값을 편집할 수 있는지 확인
- Blueprint compile / details refresh 과정에서 editor crash가 발생하지 않는지 확인

---
## 검증 결과

- Sword input이 `ACPlayerController -> ACPlayer -> UCWeaponComponent -> UCEquipment` 경로로 전달되는 동작 확인
- equip montage와 unequip montage 재생 확인
- montage notify timing 기준으로 attachment socket 전환 확인
- equipment execution 중 state / movement policy 적용 및 복구 확인
- `FEquipmentData` editor 노출과 값 전달 방식 적용 후 editor crash 재발 없음 확인

---
## 관련 문서

- Issue Checklist: `D04_UE5_Portfolio_Issue_Checklist.md`

- Bug Report: `B02_UE5_Portfolio_Bug_Report.md`

---
## 정리

이 PR의 핵심은 sword input에서 시작된 equipment request를 `UCWeaponComponent`와 `UCEquipment`로 전달하고, montage notify timing에 맞춰 `ACAttachment`의 socket 위치를 전환하는 equipment pipeline을 만든 것이다.

변경 후에는 input routing, weapon mode, equipment lifecycle, attachment socket, notify timing, editor-facing equipment data의 책임이 분리되어 이후 combat action에서 weapon equipped state를 조건으로 사용할 수 있는 기준이 마련됐다.

```yaml
ACPlayerController
- Sword input binding
- pawn으로 input 전달

ACPlayer
- Idle state 확인
- weapon type 기준 equip / unequip 요청

UCWeaponComponent
- CurrentWeaponType 관리
- UCEquipment / ACAttachment 생성과 소유
- equipment entry 제공

UCEquipment
- equip / unequip montage 재생
- state / movement policy 적용
- begin / end notify timing 처리

ACAttachment
- hand / holster socket 이름 보유
- equipment begin timing에서 socket 재부착

UCAnimNotify_Equip / UCAnimNotify_Unequip
- montage timing 기준 begin / end 전달
```

이 브랜치에서는 weapon equip / unequip과 attachment socket 전환을 우선 구현했고, action execution, hit collision, damage apply 처리는 후속 브랜치에서 확장할 수 있는 경계로 남겼다.

---
