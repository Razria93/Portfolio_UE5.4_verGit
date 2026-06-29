# UE5 Portfolio Pull Request Fix

## 제목

**F05: WeaponActor Trail Lifecycle 선언 보정**

## 날짜

**2026.06.30**

## 상태

- [x] **완료**

---

## 브랜치

- `fix/weapon-actor-runtime-lifecycle`

---

## 요약

이번 Fix PR에서는 `ACWeaponActor`의 runtime lifecycle helper 선언과 정의가 어긋난 부분을 보정한다.

`BeginPlay()` / `EndPlay()`에서 collision과 trail의 초기화 / 해제 흐름을 명시적으로 읽을 수 있도록 `Initialize*` / `Clear*` 계열 API를 사용한다.

기능 동작을 새로 추가하지 않고, WeaponActor의 runtime state 정리 의도를 헤더와 소스에서 일관되게 드러내는 것이 목표다.

---

## 변경 사항

- `ACWeaponActor::BeginPlay()`에서 collision / trail 초기화 호출을 명시적인 lifecycle helper 이름으로 정리했다.

```text
InitializeCollisionComponents
InitializeTrailState
```

- `ACWeaponActor::EndPlay()`에서 trail 해제 처리를 직접 `ToggleTrailActive(false)`로 호출하지 않고 `ClearTrailState()`로 분리했다.

- `CWeaponActor.h`의 private helper 선언을 `CWeaponActor.cpp` 정의와 일치시켰다.

```text
InitializeCollisionComponents
ClearCollisionComponents
InitializeTrailState
ClearTrailState
```

---

## 변경하지 않은 것

- WeaponActor 생성 / 파괴 흐름

- collision enable / disable gameplay 동작

- trail 활성화 / 비활성화 실제 처리 로직

- WeaponComponent와 WeaponActor 사이의 reference 주입 흐름

- montage notify / collision window routing 정책

---

## 검증 결과

- `git diff --check` 통과

- 헤더 선언과 소스 정의 이름 일치 확인

- 기존 `ConfigureCollisionComponents`, `ConfigureTrailInitialState` 잔여 사용처 없음 확인

- 이번 Fix PR은 WeaponActor helper 선언 / 정의 보정이므로 별도 기능 검증은 기존 P30 runtime lookup 검증 범위에 의존한다.

---

## 관련 문서

- `Docs/04_Pull_Request/P30_UE5_Portfolio_Pull_Request.md`

- `Docs/06_notes/N12_Runtime_Component_Lookup_Policy_Note.md`

---
