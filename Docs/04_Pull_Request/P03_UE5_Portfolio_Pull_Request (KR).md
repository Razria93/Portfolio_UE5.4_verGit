# 무기 장착/해제 시스템 및 애니 노티파이 기반 플로우 구현

## 제목

✨ feat: 무기 장착/해제 시스템 및 애니 노티파이 기반 플로우 구현 (#9, #10)

## 요약

- 이 PR은 **무기 장착/해제 플로우**를 **Montage + AnimNotify 타이밍**으로 구동하고, 이를 `CStateComponent`(상태), `CMovementComponent`(이동 제어), `CAttachment`(소켓 부착물)과 연동함
  
- 또한 `FEquipmentData`를 `USTRUCT(BlueprintType)`으로 지정하고 USTRUCT를 `const&`로 전달하던 부분을 **값 전달로 변경**하여, **Property Editor 평가 과정에서 발생하던 USTRUCT 참조 수명(lifetime) 문제로 인한 에디터 크래시를 방지**함

---

## 완료 항목

### 1. weapon type / state type 변경 플로우 (기반)

- `CWeaponComponent`에 weapon type/mode 전환 로직 구현
  
    - `FWeaponTypeChanged` 델리게이트로 변경 알림 브로드캐스트
        
- `CStateComponent`에 state type/mode 관리 로직 구현
  
    - `FStateTypeChanged` 델리게이트로 변경 알림 브로드캐스트
        
- `CPlayer` 입력 라우팅 연결
  
    - `HandleSword()` → `CWeaponComponent::SetSwordMode`
        
- `CAnimInstance`에서 델리게이트 바인딩을 통해 weapon type 변경 반응 구현
  
    - `OnWeaponTypeChanged` 바인딩 후 AnimBP 변수 업데이트
        

---

### 2. equipment 시스템 (장착/해제 라이프사이클)

- `CEquipment` 라이프사이클 구현

    - `Equip`, `Begin_Equip`, `End_Equip`

    - `Unequip`, `Begin_Unequip`, `End_Unequip`
        
- 장착/해제 상태에서 이동 제어 지원

    - `CMovementComponent::SetStop` / `SetMove` 구현

    - `FEquipmentData::bCanMove` 값으로 이동 제한 여부 제어
        
- 장착 상태 정의 확장

    - `CStateStructure`에 `Equip`(및 필요 시 `Unequip`) 상태 추가
        
- 무기 장착 데이터 정의 확장

    - `CWeaponStructure`에 `FEquipmentData` 추가

    - `CWeaponComponent`에 `EquipmentData`및 `UnequipmentData` 노출

    - `CWeaponComponent`에서 `CEquipment` 생성/초기화 및 `GetEquipment()` 접근자 제공
        

---

### 3. attachment + 소켓 구성 (hand/holster)

- `CAttachment` 소켓 부착 로직 확장

    - hand/holster 소켓 지원

    - 에디터/BP에서 소켓 이름 데이터 지정 가능하도록 구성
        
- `CEquipment` 이벤트와 `CAttachment`를 바인딩하여 장착/해제 타이밍에 소켓 전환 처리

    - `OnEquipmentBeginEquip` → hand 부착

    - `OnEquipmentBeginUnequip` → holster 부착


---

### 4. 애니 타이밍 기반 장착/해제 (anim notify)

- AnimNotify 연동 구현

    - `CAnimNotify_Equip` → `CEquipment::Begin_Equip` / `End_Equip`

    - `CAnimNotify_Unequip` → `CEquipment::Begin_Unequip` / `End_Unequip`
        
- 관련 몽타주/블렌드 에셋 추가/수정

    - draw/sheath 몽타주 구성

    - upper/full body 레이어 블렌드 구성(현재 프로젝트 세팅 기준)


---

### 5. 에디터 크래시 수정 (property editor 안정성)

- `FEquipmentData`에 `USTRUCT(BlueprintType)` 적용 및 `UPROPERTY` 기반으로 노출
    
- 초기화 매개변수의 `const FEquipmentData&` 제거, 값 타입 전달로 변경
    
- Property Editor에서 `USTRUCT`를 평가/표시/복사하는 과정에서 발생하던 **참조 수명(lifetime) 문제(댕글링 참조 가능성)**를 제거하여 크래시 방지


---

## 테스트 방법

1. 프로젝트 실행 후 테스트 레벨에서 플레이
    
2. 무기 키 입력(예: `PressSword` 바인딩) 수행
    
3. 장착 플로우 확인

    - draw 몽타주 재생

    - state가 equip 상태로 전환

    - 데이터에 따라 이동 stop/move 동작 확인

    - 노티파이 타이밍에 attachment가 hand 소켓으로 이동
        
4. 해제 플로우 확인

    - sheath 몽타주 재생

    - 노티파이 타이밍에 attachment가 holster 소켓으로 복귀

    - state가 idle로 복귀
        
5. BP/디테일 패널에서 `EquipmentData` 편집 시

    - 에디터 크래시가 발생하지 않는지 확인
        

---

## 관련 이슈 / 브랜치

- 브랜치: `feature/character-weapon-equip`
    
- 이슈:

    - #9 (장착/해제 시스템 통합)

    - #10 (`USTRUCT`의 매개변수에 대해 const& 사용으로 인한 문제 해결)


---

## 노트

- 책임 분리

    - `CPlayerController / CPlayer`: 입력 처리 및 라우팅

    - `CWeaponComponent`: `CAttachment` / `CEquipment` 생성 및 소유

    - `CEquipment`: 상태 전환, 이동 제어, 몽타주 기반 타이밍 처리

    - `AnimNotify`: 장착/해제 begin/end 타이밍 트리거
        
- `FEquipmentData`는 에디터 안전한 형태로 정리되어 다른 무기에도 재사용 가능


---