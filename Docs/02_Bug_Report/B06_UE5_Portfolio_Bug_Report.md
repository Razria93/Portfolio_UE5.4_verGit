# UE5 Portfolio Bug Report

## 제목

**B06: C++ 클래스 rename 이후 기존 Weapon Blueprint 부모 클래스 로드 실패 문제**

## 날짜

**2026.04.20**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/action-orchestration`

---

## 요약

- `ACAttachment`를 `ACWeaponActor`로 rename한 이후 기존 `BP_CAttachment_Sword` Blueprint 에셋 로드가 실패했다.

- 기존 Blueprint가 저장하고 있던 부모 클래스 경로는 `/Script/Portfolio.CAttachment`였으나, C++ 클래스 rename 이후 해당 클래스가 존재하지 않아서 `BlueprintGeneratedClass` 및 하위 export 로드가 실패했다.

- `Config/DefaultEngine.ini`에 `CoreRedirects` **class redirect**를 추가하여 기존 Blueprint가 새 C++ 부모 클래스인 `ACWeaponActor`를 정상 참조하도록 수정했다.

- 에디터 재실행 및 Blueprint 로드 검증 결과, 기존 Weapon Blueprint 에셋이 정상 복구됨을 확인했다.

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 코드:
	- `Source/Portfolio/Weapon/CAttachment.h`
	- `Source/Portfolio/Weapon/CWeaponActor.h`
	- `Config/DefaultEngine.ini`

- 관련 에셋:
	- `/Game/06_Weapon/BP_CAttachment_Sword`

---

## 재현 방법

1. `ACAttachment` 클래스를 `ACWeaponActor`로 rename한다.

2. 기존 `BP_CAttachment_Sword` 에셋을 열거나 에디터에서 프로젝트를 로드한다.

3. 에디터 로그에서 기존 Blueprint class export 로드 실패 메시지를 확인한다.

```text
CreateExport: 'Capsule_GEN_VARIABLE' 리소스에 대한 BlueprintGeneratedClass /Game/06_Weapon/BP_CAttachment_Sword.BP_CAttachment_Sword_C Outer 로드에 실패했습니다.

CreateExport: 'Trail' 리소스에 대한 BP_CAttachment_Sword_C /Game/06_Weapon/BP_CAttachment_Sword.Default__BP_CAttachment_Sword_C Outer 로드에 실패했습니다.

CreateExport: 'DefaultSceneRoot_GEN_VARIABLE' 리소스에 대한 BlueprintGeneratedClass /Game/06_Weapon/BP_CAttachment_Sword.BP_CAttachment_Sword_C Outer 로드에 실패했습니다.

CreateExport: 'RootScene' 리소스에 대한 BP_CAttachment_Sword_C /Game/06_Weapon/BP_CAttachment_Sword.Default__BP_CAttachment_Sword_C Outer 로드에 실패했습니다.
```

---

## 기대 결과 vs 실제 결과

**기대 결과**

- 기존 `BP_CAttachment_Sword`가 새 C++ 부모 클래스인 `ACWeaponActor`를 정상 참조해야 한다.

- 기존 Blueprint 구성요소와 설정값이 유지된 상태로 에셋이 로드되어야 한다.

- 에셋을 새로 만들지 않고 C++ 클래스 rename만으로 마이그레이션 가능해야 한다.

**실제 결과**

- 기존 Blueprint가 더 이상 존재하지 않는 `CAttachment` 클래스를 참조했다.

- Blueprint class export 및 하위 component export 로드에 실패했다.

- 결과적으로 기존 Weapon Blueprint 에셋을 정상적으로 열 수 없었다.

---

## 원인

- Unreal Blueprint 에셋은 C++ 부모 클래스의 script path를 직렬화하여 저장한다.

- `ACAttachment`를 `ACWeaponActor`로 rename했지만, 기존 클래스 경로에서 새 클래스 경로로 이동시키는 redirect 설정이 없었다.

- 그 결과 `/Script/Portfolio.CAttachment`를 참조하던 기존 에셋이 새 클래스 `/Script/Portfolio.CWeaponActor`를 찾지 못했다.

---

## 수정

`Config/DefaultEngine.ini`에 class redirect를 추가했다.

```ini
[CoreRedirects]
+ClassRedirects=(OldName="/Script/Portfolio.CAttachment",NewName="/Script/Portfolio.CWeaponActor")
```

- 기존 C++ UClass 이름 `CAttachment`를 새 UClass 이름 `CWeaponActor`로 redirect했다.

- 에디터 재시작 시 기존 Blueprint가 새 부모 클래스를 찾을 수 있도록 했다.

---

## 검증 결과

- 에디터를 완전히 종료한 뒤 프로젝트를 다시 빌드하고 에디터를 재실행했다.

- `/Game/06_Weapon/BP_CAttachment_Sword`가 새 C++ 부모 클래스 `ACWeaponActor` 기준으로 정상 로드됨을 확인했다.

- 기존 Blueprint를 새로 만들지 않고 복구 가능함을 확인했다.

- 해당 Blueprint를 다시 저장하여 redirect 적용 이후의 참조 상태를 반영했다.

- 최초 발생했던 `BlueprintGeneratedClass` 및 component export 로드 실패 로그가 재현되지 않는 것을 확인했다.

---

## 비고

- 에셋 이름은 필요 시 에디터에서 `BP_CWeaponActor_Sword`로 rename한다.

- C++ 클래스 rename 이후 Blueprint 에셋을 정상 저장했다면 redirect는 당분간 유지한다.

- 모든 참조가 새 이름으로 저장되고 이전 클래스 참조가 남지 않는 것을 확인한 후 redirect를 제거한다.

- 현재 검증 기준에서는 Blueprint가 새 부모 클래스 기준으로 정상 로드되고 동작하는 것을 확인했다.

- C++ 클래스 rename만 수행하는 경우에도 Blueprint 에셋은 기존 부모 클래스 경로를 계속 참조할 수 있다.

- Unreal에서 UCLASS 이름은 `A` / `U` 접두사를 제외한 이름으로 등록되므로 `ACAttachment`의 UClass 이름은 `CAttachment`, `ACWeaponActor`의 UClass 이름은 `CWeaponActor`이다.

- C++ 클래스 rename이 Blueprint 부모 클래스에 영향을 주는 경우, 에셋 재생성보다 `CoreRedirects`를 통한 마이그레이션을 먼저 시도하는 것이 안전하다.

---
