# UE5 Portfolio Pull Request

## 제목

**P03: Attachment Object 및 Equipment 실행 흐름 구현**

## 날짜

**2025.12.17**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/character-weapon-equip`

---

## 요약

이번 PR에서는 **Player의 sword 입력으로 무기를 장착 / 해제하고, montage timing에 맞춰 weapon attachment를 hand socket과 holster socket 사이에서 전환하는 흐름을 구현했다.**

입력 전달, weapon type 변경, equip / unequip montage 실행, attachment socket 전환 책임을 나누어 이후 공격 실행에서 무기 장착 상태를 기준으로 사용할 수 있게 했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Sword 입력 기반 장착 / 해제 흐름 구성**: Player의 sword 입력이 현재 weapon type에 따라 장착 또는 해제 실행으로 이어지도록 구성했다.

- **Equipment montage 실행 구성**: 장착 / 해제 상태로 전환한 뒤 각각의 montage를 재생하고, montage 종료 후 Idle state로 복귀하도록 구성했다.

- **Attachment socket 전환 구성**: 장착 begin timing에는 weapon attachment를 hand socket으로 옮기고, 해제 begin timing에는 holster socket으로 되돌리도록 구성했다.

### Refactoring

- **입력과 장착 실행 책임 분리**: Player 입력 계층은 장착 / 해제 의도를 전달하고, 실제 montage 실행과 state / movement 제어는 `UCEquipment`가 처리하도록 역할을 나눴다.

- **weapon type 관리와 장착 실행 분리**: `UCWeaponComponent`는 현재 weapon type과 attachment / equipment 생성 및 delegate binding을 관리하고, `UCEquipment`는 equip / unequip montage 실행과 state / movement lifecycle을 담당하도록 정리했다.

- **socket 전환 timing 분리**: `UCEquipment`가 notify timing에 맞춰 delegate를 broadcast하고, `ACAttachment`가 socket 재부착을 처리하도록 연결했다.

### Troubleshooting

- **Equipment data editor 안정화**: editor details panel에서 equipment montage 설정을 안전하게 편집할 수 있도록 `FEquipmentData`를 reflection 친화적인 struct로 정리했다. 자세한 원인과 검증은 `B02_UE5_Portfolio_Bug_Report.md`에 분리했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

```text
Equipment(장착 실행 객체)
-> weapon equip / unequip montage 재생, state 전환, movement 제어를 담당하는 UObject 기반 실행 객체
-> 코드에서는 `UCEquipment`가 이 역할을 담당함
```

```text
Attachment(무기 부착 actor)
-> character mesh의 hand / holster socket에 붙는 weapon actor
-> 코드에서는 `ACAttachment`가 이 역할을 담당함
```

```text
Weapon Type(무기 상태)
-> 현재 character가 무기를 들고 있는지 나타내는 상태
-> 코드에서는 `EWeaponType::Unarmed`, `EWeaponType::Sword`를 사용함
```

```text
FEquipmentData(equipment 실행 데이터)
-> equip / unequip montage, play rate, movement 허용 여부를 담는 실행 데이터
```

```text
Equipment Notify(equipment notify)
-> montage timing에서 equip / unequip의 Begin / End를 알려주는 notify
-> 코드에서는 `UCAnimNotify_Equip`, `UCAnimNotify_Unequip`이 이 역할을 담당함
```

---

## 변경 배경

이 섹션은 weapon equip / unequip 기능이 필요했던 이유와 socket 전환을 montage timing에 맞춰 분리해야 했던 이유를 정리한다.

### 무기 장착 상태 전환 필요성

Player는 sword 입력으로 무기를 장착하거나 해제할 수 있어야 했다.

이 상태는 단순히 flag만 바꾸는 값이 아니라, 이후 공격 실행에서 sword 장착 여부를 판단하는 기준으로 사용된다.

### Montage timing 기반 socket 전환 필요성

무기 장착 / 해제는 입력 즉시 weapon mesh 위치만 바꾸는 기능이 아니다.

장착 montage가 진행되는 중 특정 timing에 weapon attachment가 holster socket에서 hand socket으로 이동해야 하고, 해제 montage에서는 다시 hand socket에서 holster socket으로 돌아가야 했다.

### Equipment 실행 책임 분리 필요성

장착 / 해제 중에는 state가 Equip / Unequip으로 바뀌고, 설정에 따라 movement도 제한되어야 했다.

따라서 Player나 WeaponComponent가 montage 재생과 state / movement 제어를 직접 처리하지 않고, `UCEquipment`가 equipment lifecycle을 관리하는 구조가 필요했다.

### Equipment data editor 안정성 필요성

Equip / Unequip montage와 movement policy는 editor에서 설정해야 하는 값이다.

따라서 `FEquipmentData`는 details panel과 Blueprint compile / refresh 과정에서 안정적으로 다뤄질 수 있는 editor-facing struct 형태로 정리할 필요가 있었다.

---

## 변경 범위

이 섹션은 weapon equip / unequip 흐름을 어떤 책임으로 나눠 구성했고, 그 결과 장착 동작이 어떻게 정리됐는지 설명한다.

### 1. Sword input 기반 장착 / 해제 진입 흐름 구성

- **왜**:
  Player의 sword 입력이 현재 weapon 상태에 따라 equip 또는 unequip 실행으로 이어져야 했다.
  또한 장착 / 해제는 Idle state에서만 시작되어야 했다.

- **어떻게**:
  `ACPlayerController::PressSword()`는 `ACPlayer::HandleSword()`를 호출한다.
  `ACPlayer`는 Idle state와 현재 weapon type을 확인한다.
  현재 weapon type에 따라 `UCWeaponComponent::SetSwordMode()` 또는 `SetUnarmedMode()`를 호출한다.

- **결과**:
  Sword input은 Idle state에서만 equip / unequip 실행으로 이어지고, 현재 weapon type에 따라 장착과 해제가 토글된다.

### 2. WeaponComponent의 attachment / equipment 생성과 연결 구성

- **왜**:
  WeaponComponent는 현재 weapon type을 관리하면서, 장착 실행 객체와 실제 socket 전환을 담당할 attachment를 함께 소유해야 했다.

- **어떻게**:
  `UCWeaponComponent::BeginPlay()`에서 actor인 `ACAttachment`는 spawn하고, UObject인 `UCEquipment`는 `NewObject`로 생성했다.
  이후 `UCEquipment`의 begin delegate를 `ACAttachment` callback에 binding해, equipment timing이 attachment socket 전환으로 이어지도록 연결했다.

- **결과**:
  WeaponComponent는 weapon type, equipment 실행 객체, attachment actor, delegate binding 지점을 관리하는 중심 component가 된다.

### 3. Equipment montage 실행과 state / movement 제어 구성

- **왜**:
  Equip / Unequip은 montage 재생과 함께 character state, movement 제한 / 복구를 처리해야 했다.

- **어떻게**:
  `UCEquipment::Equip()`은 `SetEquipMode()`를 호출해 장착 상태로 전환한다.
  필요한 경우 movement를 제한한 뒤 equip montage를 재생한다.
  `UCEquipment::Unequip()`도 같은 흐름으로 해제 상태 전환, movement 제한, unequip montage 재생을 처리한다.

- **결과**:
  Equip / Unequip 실행 중 character state와 movement policy가 montage 실행 흐름에 맞춰 적용된다.

### 4. Equipment Notify 기반 begin / end timing 연결

- **왜**:
  Socket 전환과 종료 복구는 montage의 실제 timing에 맞춰 실행되어야 했다.

- **어떻게**:
  `UCAnimNotify_Equip`과 `UCAnimNotify_Unequip`을 추가했다.
  `UCAnimNotify_Equip`은 `Begin` / `End` flow에 따라 `UCEquipment::Begin_Equip()` 또는 `End_Equip()`을 호출한다.
  `UCAnimNotify_Unequip`은 `Begin` / `End` flow에 따라 `UCEquipment::Begin_Unequip()` 또는 `End_Unequip()`을 호출한다.

- **결과**:
  Equip / Unequip montage timing이 equipment lifecycle 처리로 전달된다.

### 5. Attachment socket 전환 구성

- **왜**:
  Weapon attachment는 장착 전에는 holster socket에 있고, 장착 중에는 hand socket으로 이동해야 했다.

- **어떻게**:
  `ACAttachment::InitializeAttachment()`는 weapon attachment를 holster socket에 먼저 부착한다.
  `OnEquipmentBeginEquip()`은 weapon attachment를 hand socket으로 재부착한다.
  `OnEquipmentBeginUnequip()`은 weapon attachment를 holster socket으로 재부착한다.

- **결과**:
  Weapon attachment는 equipment begin timing에 맞춰 hand / holster socket 사이를 전환한다.

### 6. Equipment 종료 처리와 복구 구성

- **왜**:
  장착 / 해제가 끝나면 movement 제한을 풀고 character state를 Idle로 되돌려야 했다.
  또한 장착 완료 여부를 내부 상태로 기록해야 했다.

- **어떻게**:
  `End_Equip()`은 `bEquipped`를 `true`로 설정한다.
  필요한 경우 movement를 복구한 뒤 Idle state로 복귀한다.
  `End_Unequip()`은 `bEquipped`를 `false`로 설정한다.
  같은 방식으로 movement 복구와 Idle state 복귀를 처리한다.

- **결과**:
  Equip / Unequip 종료 이후 character는 다시 Idle state로 돌아가고, 장착 완료 상태가 갱신된다.

### 7. FEquipmentData editor 안정성 보완

- **왜**:
  Equipment montage, play rate, movement policy는 editor details panel에서 설정되어야 했다.
  기존 struct 노출 방식이 불안정하면 Blueprint compile 또는 details refresh 과정에서 crash로 이어질 수 있었다.

- **어떻게**:
  `FEquipmentData`를 `USTRUCT(BlueprintType)` / `UPROPERTY(EditAnywhere)` 기반으로 정리했다.
  `UCEquipment::InitializeEquipment()`에서 `FEquipmentData` 값을 전달받아 cache하도록 구성했다.

- **결과**:
  Equipment 실행 데이터는 editor에서 편집 가능하고, 값 전달 방식으로 equipment object에 안정적으로 주입된다.

---

## 주요 처리 흐름

이 섹션은 sword 입력이 equip / unequip montage 실행과 attachment socket 전환으로 이어지는 대표 흐름을 정리한다.

### Sword input 흐름

```text
Player sword input
-> ACPlayerController::PressSword
-> ACPlayer::HandleSword
-> Idle state 확인
   - Idle 아님 -> 실행하지 않음
   - Idle     -> current weapon type 확인
                - Unarmed -> UCWeaponComponent::SetSwordMode
                - Sword   -> UCWeaponComponent::SetUnarmedMode
```

이 흐름은 Player의 sword 입력이 현재 weapon type에 따라 equip 또는 unequip 실행으로 분기되는 과정을 의미한다.

### Equip 흐름

```text
UCWeaponComponent::SetSwordMode
-> UCEquipment::Equip
-> state를 Equip으로 변경
-> movement policy 적용
-> equip montage 재생
-> UCAnimNotify_Equip Begin
-> OnEquipmentBeginEquip broadcast
-> ACAttachment hand socket 재부착
-> UCAnimNotify_Equip End
-> movement policy 복구
-> state를 Idle로 변경
```

이 흐름은 sword 장착 입력이 equip montage 재생과 hand socket 전환, 종료 복구로 이어지는 과정을 의미한다.

### Unequip 흐름

```text
UCWeaponComponent::SetUnarmedMode
-> UCEquipment::Unequip
-> state를 Unequip으로 변경
-> movement policy 적용
-> unequip montage 재생
-> UCAnimNotify_Unequip Begin
-> OnEquipmentBeginUnequip broadcast
-> ACAttachment holster socket 재부착
-> UCAnimNotify_Unequip End
-> movement policy 복구
-> state를 Idle로 변경
```

이 흐름은 sword 해제 입력이 unequip montage 재생과 holster socket 전환, 종료 복구로 이어지는 과정을 의미한다.

### Attachment socket binding 흐름

```text
UCWeaponComponent::BeginPlay
-> ACAttachment 생성
-> UCEquipment 생성
-> Equipment begin delegate를 Attachment callback에 binding
-> Equipment begin timing에서 delegate broadcast
-> ACAttachment::AttachToOwnerSocket 호출
```

이 흐름은 equipment timing이 delegate binding을 통해 attachment socket 전환으로 전달되는 과정을 의미한다.

---

## 구현 결과

- Player sword input은 `ACPlayerController -> ACPlayer -> UCWeaponComponent` 경로로 전달된다.

- `UCWeaponComponent`는 current weapon type을 기준으로 equip / unequip 실행을 선택한다.

- `UCEquipment`는 equip / unequip montage 재생, state 전환, movement 제한 / 복구를 처리한다.

- `ACAttachment`는 equipment begin timing에 맞춰 hand / holster socket으로 재부착된다.

- `UCAnimNotify_Equip` / `UCAnimNotify_Unequip`은 montage Begin / End timing을 `UCEquipment`로 전달한다.

- `FEquipmentData`는 editor에서 설정 가능한 equipment 실행 데이터로 사용된다.

---

## 테스트 방법

### Input / Entry

- `Sword` input이 `ACPlayerController::PressSword()`로 binding되어 있는지 확인한다.

- `ACPlayer::HandleSword()`에서 Idle state일 때만 equip / unequip으로 이어지는지 확인한다.

- weapon type이 `Unarmed`이면 `SetSwordMode()`, `Sword`이면 `SetUnarmedMode()`가 호출되는지 확인한다.

### Equip / Unequip 실행

- `SetSwordMode()` 호출 시 equip montage가 재생되는지 확인한다.

- `SetUnarmedMode()` 호출 시 unequip montage가 재생되는지 확인한다.

- equip / unequip 중 `EStateType::Equip` / `EStateType::Unequip` state로 전환되는지 확인한다.

- notify end 이후 Idle state로 복귀하는지 확인한다.

### Attachment Socket

- 시작 시 weapon attachment가 holster socket에 부착되는지 확인한다.

- `UCAnimNotify_Equip` `Begin` timing에 hand socket으로 전환되는지 확인한다.

- `UCAnimNotify_Unequip` `Begin` timing에 holster socket으로 전환되는지 확인한다.

### Movement Policy

- `FEquipmentData::bCanMove`가 `false`인 경우 montage 시작 시 movement가 제한되는지 확인한다.

- equip / unequip end timing 이후 movement가 복구되는지 확인한다.

### Editor 안정성

- Details Panel에서 `FEquipmentData`의 `Montage`, `PlayRate`, `bCanMove` 값을 편집할 수 있는지 확인한다.

- Blueprint compile / details refresh 과정에서 editor crash가 발생하지 않는지 확인한다.

---

## 검증 결과

- Sword input이 `ACPlayerController -> ACPlayer -> UCWeaponComponent -> UCEquipment` 경로로 전달되는 것을 확인했다.

- equip montage와 unequip montage가 각각 재생되는 것을 확인했다.

- montage notify timing 기준으로 attachment socket이 hand / holster 사이에서 전환되는 것을 확인했다.

- equipment 실행 중 state / movement policy가 적용되고 종료 후 복구되는 것을 확인했다.

- `FEquipmentData` editor 노출과 값 전달 방식 적용 후 editor crash가 재발하지 않는 것을 확인했다.

---

## 비범위

- P03에서는 combat action 실행, hit collision, damage 계산, hit reaction 처리를 구현하지 않는다.

- Sword 장착 상태를 조건으로 사용하는 실제 공격 실행은 후속 LightAttack 범위로 남는다.

- Attachment collision과 damage 처리는 후속 hit collision / damage 처리 범위로 남는다.

---

## 관련 문서

- Issue Checklist: `D04_UE5_Portfolio_Issue_Checklist.md`

- Bug Report: `B02_UE5_Portfolio_Bug_Report.md`

---

## 정리

- P03는 Player sword input을 weapon equip / unequip 실행으로 연결하고, montage timing에 맞춰 attachment socket을 hand / holster 사이에서 전환하는 equipment 실행 흐름 PR이다.

- `UCWeaponComponent`, `UCEquipment`, `ACAttachment`, equipment notify가 각각 weapon type 관리, equipment lifecycle, socket 재부착, timing 전달 역할을 나눠 갖도록 정리했다.

- 이 브랜치의 완료 범위는 weapon equip / unequip과 attachment socket 전환까지이며, 공격 실행과 hit collision / damage 처리는 후속 브랜치에서 확장한다.
