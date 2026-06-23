# UE5 Portfolio Bug Report

## 제목

**B13: Native Component Rename 후 C++ 멤버 참조가 유효하지 않아 Target-side Hit 처리가 누락되는 문제**

## 날짜

**2026.06.23**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-signal-component-rename`

---

## 요약

- `ApplyDamageComponent` / `TakeDamageComponent`를 `CombatSignalSourceComponent` / `CombatSignalTargetComponent`로 리네임한 뒤, Player -> Enemy 공격에서 Enemy `TakeDamage()`는 호출되지만 target-side damage 처리 결과가 출력되지 않는 문제가 발생했다.

- 원인은 Blueprint 인스턴스에 renamed native component는 존재하지만, C++ 멤버 포인터 `CombatSignalTargetComponent`가 해당 component instance를 즉시 가리키지 못하는 상태였다.

- `FindComponentByClass<UCCombatSignalTargetComponent>()`로 Actor에 이미 붙어 있는 component instance를 찾아 멤버 포인터에 다시 연결하면 정상 동작하는 것을 확인했다.

- 최종 수정은 `ACPlayer` / `ACEnemy`의 `BeginPlay()`에서 `ResolveComponentReferences()`를 호출해 rename 대상 component reference를 한 번 검증 / 복구하는 방식으로 처리했다.

---

## 영향 범위

- Player -> Enemy hit 처리

- Enemy -> Player hit 처리

- Native component rename 이후 Blueprint 인스턴스의 C++ member reference 복구

- `CombatSignalSourceComponent` / `CombatSignalTargetComponent` 참조 안정성

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 브랜치:
  - `refactor/combat-signal-component-rename`

- 관련 코드:
  - `Source/Portfolio/Character/Player/CPlayer.cpp`
  - `Source/Portfolio/Character/Player/CPlayer.h`
  - `Source/Portfolio/Character/Enemy/CEnemy.cpp`
  - `Source/Portfolio/Character/Enemy/CEnemy.h`

- 관련 에셋:
  - `Content/01_Character/01_Player/BP_CPlayer.uasset`
  - `Content/01_Character/02_Enemy/BP_CEnemy.uasset`

---

## 발생 조건

- Native component의 class name, C++ UPROPERTY field name, `CreateDefaultSubobject` name을 함께 변경했다.

```text
UCApplyDamageComponent
-> UCCombatSignalSourceComponent

UCTakeDamageComponent
-> UCCombatSignalTargetComponent

ApplyDamageComponent
-> CombatSignalSourceComponent

TakeDamageComponent
-> CombatSignalTargetComponent

"ApplyDamage"
-> "CombatSignalSource"

"TakeDamage"
-> "CombatSignalTarget"
```

- 기존 Blueprint asset이 이전 native component template / serialized reference를 가지고 있었다.

- ClassRedirect / PropertyRedirect는 추가했지만, 런타임에서 일부 C++ member pointer가 renamed component instance를 바로 가리키지 못했다.

---

## 재현 방법

1. `refactor/combat-signal-component-rename` 브랜치에서 Player / Enemy Blueprint를 로드한다.

2. PIE에서 Player가 Enemy를 공격한다.

3. Enemy `TakeDamage()` 진입 여부를 확인한다.

4. `CombatSignalTargetOutcome` 로그와 Enemy HP commit 여부를 확인한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- Player 공격 시 Enemy `TakeDamage()`가 호출된다.

- Enemy `CombatSignalTargetComponent`가 target-side damage flow를 처리한다.

- `CombatSignalTargetOutcome` 로그가 출력된다.

- Enemy HP commit / reaction / feedback 흐름이 이어진다.

**실제 결과**

- Player `CombatSignalSourceComponent`는 Enemy `TakeDamage()`까지 정상 호출했다.

- 하지만 Enemy의 C++ member pointer `CombatSignalTargetComponent`가 유효하지 않은 경우 fallback 경로로 빠질 수 있었다.

- 이 경우 source-side에서는 `TakeDamage()` 반환값 때문에 damage가 commit된 것처럼 보일 수 있지만, target-side HP commit / reaction / feedback flow는 실행되지 않는다.

---

## 관측 로그

정상화 후 Player -> Enemy hit 경로:

```text
[CombatSignalSource] Request | Owner=BP_CPlayer_C_0 | Source=BP_CPlayer_C_0 | Target=BP_CEnemy_C_1
[Enemy] TakeDamage Entry | Target=BP_CEnemy_C_1 | Damage=5.000
[CombatSignalTargetOutcome] Outcome=EDamageDefenseOutcome::None | Commit=true | Damage=5.000 | HP=50.000->45.000
```

---

## 원인

- Actor에는 renamed native component instance가 존재했다.

- 그러나 C++ UPROPERTY member pointer가 해당 component instance를 가리키지 못하는 상태가 발생했다.

- 따라서 문제는 damage 계산이나 collision flow가 아니라, native component rename 이후 Blueprint serialized reference와 C++ member reference가 어긋난 migration 문제였다.

- `FindComponentByClass<UCCombatSignalTargetComponent>()`는 새 component를 생성하지 않고, Actor에 이미 붙어 있는 component instance를 찾아 C++ member pointer에 다시 연결했다.

---

## 수정 방향

이번 브랜치에서는 rename 대상 component만 좁게 복구한다.

```text
BeginPlay
-> ResolveComponentReferences
   -> CombatSignalSourceComponent 유효성 확인
   -> invalid면 FindComponentByClass로 기존 component instance 재연결
   -> CombatSignalTargetComponent 유효성 확인
   -> invalid면 FindComponentByClass로 기존 component instance 재연결
```

전체 character component cache validation은 이번 브랜치 범위를 넘어서므로 후속 브랜치 후보로 분리한다.

---

## 수정

- `ACPlayer::ResolveComponentReferences()` 추가

- `ACEnemy::ResolveComponentReferences()` 추가

- `ACPlayer::BeginPlay()`에서 `ResolveComponentReferences()` 호출

- `ACEnemy::BeginPlay()`에서 `ResolveComponentReferences()` 호출

- `TakeDamage()` fallback 경로에 invalid target component 로그 추가

---

## 검증 결과

- `git diff --check` 통과

- `PortfolioEditor Win64 Development` 빌드 성공

- Player -> Enemy hit 시 `CombatSignalTargetOutcome` 로그와 HP commit 확인

- Enemy -> Player hit 기존 흐름 유지 확인

---

## 회귀 방지 기준

- Native component rename 시 class redirect / property redirect만으로 충분하다고 보지 않는다.

- Blueprint Details panel에 component가 보이더라도 C++ member pointer 유효성을 별도로 확인한다.

- 리네임 대상 native component는 `BeginPlay()` 단계에서 reference validation / recovery 필요성을 검토한다.

- 전체 component cache validation 정책은 별도 작업으로 분리한다.

---

## 관련 PR / 문서

- PR: `Docs/04_Pull_Request/P24_UE5_Portfolio_Pull_Request.md`

- Task Brief: `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_05_Combat_Signal_Component_Rename.md`

- Note: `Docs/06_notes/N07_Unreal_Native_Component_Rename_And_Blueprint_Reference_Note.md`

---

## 비고

- 이 Bug Report는 현상 관측, 원인 판단, 현재 브랜치의 해결 방식을 기록한다.

- Unreal Engine의 native component 생성, Blueprint serialization, C++ member reference 복구 방식은 N07에서 별도로 정리한다.

---
