# UE5 Portfolio Bug Report

## 제목

**B02: USTRUCT와 const& 매개변수로 인한 에디터 크래시 문제**

## 날짜

**2025.12.16**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/character-weapon-equip`

---

## 요약

- `FEquipmentData` 같은 `USTRUCT` 데이터를 `const&` 참조 매개변수로 전달하면서 발생한 에디터 크래시 문제를, 에디터 노출 구조체 구성 정리와 값 전달 방식 적용으로 수정했다.

---

## 영향 범위

- Details Panel / Property Editor 안정성

- `FEquipmentData` 기반 장비 초기화와 Blueprint 노출 흐름

---

## 환경

- 엔진: Unreal Engine 5.4

- 발생 위치: Details Panel / Property Editor (USTRUCT 데이터 편집 구간)

- 대상 데이터: `FEquipmentData` (Montage, PlayRate, bCanMove)

---

## 발생 조건

- editor에 노출되는 USTRUCT 데이터를 `const&` 형태로 전달한 상태에서 Details Panel 선택, compile, reinstance가 발생할 때 재현될 수 있다.

---

## 재현 방법

1. `FEquipmentData`를 `USTRUCT`로 정의하고, 에디터에서 설정 가능한 데이터로 사용한다.

2. `FEquipmentData`를 `UPROPERTY()`로 구현한다.

3. 아래처럼 구조체를 **참조(`const&`)로 받는 함수 시그니처**를 사용한다.
    - `InitializeEquipment(ACharacter* InOwnerCharacter, const FEquipmentData& InEquipData, const FEquipmentData& InUnequipData)`

4. 에디터에서 `FEquipmentData`가 표시되는 Details 패널을 열고 값을 확인/수정한다.

5. 특정 타이밍(패널 갱신/평가/리인스턴스 등)에서 에디터가 간헐적으로 크래시가 발생하는 것을 확인.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- Details 패널에서 `FEquipmentData` 값을 편집해도 에디터가 안정적으로 동작해야 한다.

**실제 결과**

- Property Editor 평가 과정에서 access violation 크래시가 발생했다.

---

## 원인

- `USTRUCT`를 `const&`로 받는 형태가 **리플렉션/에디터 평가 경로**에 걸릴 경우,
  디테일 패널/프로퍼티 시스템이 **임시 구조체(temporary) 생성/복사/평가**를 수행하는 중에, 참조가 가리키는 원본 메모리의 수명이 보장되지 않는 타이밍이 생길 수 있다.

- 그 결과, `const&`가 **유효하지 않은 메모리(댕글링 참조)**를 가리키게 되어 크래시로 이어질 수 있다.

---

## 수정

1. `FEquipmentData`를 에디터 / 블루프린트에서 안전하게 다룰 수 있도록 정리했다.
	- `USTRUCT(BlueprintType)` 적용
	- 멤버를 `UPROPERTY`로 노출

2. 함수 / 초기화 매개변수를 참조 전달에서 값 전달로 변경했다.
    - `const FEquipmentData&` → `FEquipmentData`

3. 변경된 전달 방식에 맞춰 장착 / 해제 초기화 흐름이 동일하게 동작하도록 호출부를 정리했다.

---

## 수정 기준

- editor 노출 데이터는 안정적인 소유권을 가진 값으로 전달한다.

- BlueprintType / UPROPERTY 노출 구조가 Details Panel과 runtime 초기화 양쪽에서 동일하게 유지되어야 한다.

---

## 검증 결과

1. 에디터에서 `FEquipmentData`가 노출된 액터 / 에셋을 선택했다.

2. Details 패널에서 Montage / PlayRate / bCanMove 값을 변경했다.

3. 선택 변경, 패널 리프레시, 블루프린트 컴파일 과정에서 에디터 크래시가 발생하지 않는지 확인했다.

4. 런타임에서 장착 / 해제 흐름이 정상 동작하는지 확인했다.

---

## 회귀 방지 기준

- Details Panel에서 Equipment Data를 선택해도 editor crash가 없어야 한다.

- 장비 equip / unequip runtime 흐름이 기존과 동일하게 동작해야 한다.

---

## 관련 PR / 문서

- Issue Checklist: `D04_UE5_Portfolio_Issue_Checklist.md`

- PR: `P03_UE5_Portfolio_Pull_Request.md`

---

## 비고

- 에디터 / 리플렉션 경로에 걸리는 USTRUCT는 값 전달 또는 `UPROPERTY`로 보관된 안정적인 소유 데이터를 기준으로 다룬다.

- 에디터에서 편집할 구조체는 `USTRUCT(BlueprintType)` + 멤버 `UPROPERTY` 구성을 기본으로 둔다.

---
