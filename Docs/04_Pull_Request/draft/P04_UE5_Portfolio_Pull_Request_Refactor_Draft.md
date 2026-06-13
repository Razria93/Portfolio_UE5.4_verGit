# UE5 Portfolio Pull Request

## 제목

**P04: Action Object 및 Action Execution Pipeline 기본 구성**

## 날짜

**2025.10.21**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-light-attack`

---

## 요약

이번 PR에서는 **Player의 공격 입력이 단일 LightAttack montage 실행으로 이어지는 기본 action 실행 흐름을 구현했다.**

입력 처리, action 실행 객체, montage 실행 데이터, notify timing의 책임을 나누어 이후 ComboAttack과 hit collision으로 확장할 수 있게 했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Action input 전달 흐름 구성**: PlayerController의 공격 입력이 Player와 WeaponComponent를 거쳐 action 실행 객체까지 전달되도록 구성했다.

- **LightAttack 실행 객체 구성**: sword 장착 상태와 Idle state를 확인한 뒤 LightAttack montage를 실행하는 `UCAction_LightAttack`을 추가했다.

- **Action montage 실행 데이터 구성**: montage, play rate, 이동 허용 여부를 `FActionData`로 분리해 action 실행에 필요한 값을 관리하도록 구성했다.

### Refactoring

- **입력과 실행 책임 분리**: Player 입력 계층은 공격 의도를 전달하고, 실제 montage 실행과 state 전환은 action 실행 객체가 처리하도록 역할을 나눴다.

- **montage 실행 정책 분리**: montage 재생과 이동 제한 / 복구 기준을 `FActionData`에 두어 action class가 montage 세부 설정을 직접 들고 있지 않도록 정리했다.

- **notify timing 책임 정리**: `UCAnimNotify_Action`이 montage의 Begin / End timing을 action으로 전달하고, action이 시작 / 종료 상태를 처리하도록 구성했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

```text
Action Object(action 실행 객체)
-> Player 입력을 받아 실제 action state 전환과 montage 실행을 처리하는 UObject 기반 실행 단위
-> 코드에서는 `UCAction`과 파생 action class가 이 역할을 담당함
```

```text
Action Execution Pipeline(action 실행 흐름)
-> Player action input이 action object로 전달되고, montage 실행 / notify timing / 종료 복구로 이어지는 실행 흐름
-> P04에서는 LightAttack을 기준으로 이 흐름의 기본 골격을 구성함
```

```text
LightAttack(라이트 공격)
-> P04에서 구현한 첫 번째 단일 공격 action
-> sword 장착 상태와 Idle state에서만 실행됨
```

```text
FActionData(action 실행 데이터)
-> action montage, play rate, movement 허용 여부를 담는 실행 데이터
```

```text
Action Notify(action notify)
-> montage timing에서 action의 Begin / End를 알려주는 notify
-> 코드에서는 `UCAnimNotify_Action`이 이 역할을 담당함
```

---

## 변경 배경

이 섹션은 장비 장착 흐름 이후, Player 입력을 실제 공격 montage 실행으로 연결해야 했던 이유를 정리한다.

### Player 공격 입력 실행 경로 필요성

이전 단계에서는 character movement와 weapon equip 흐름을 구성했다.

다음 단계에서는 Player가 공격 입력을 눌렀을 때, 그 입력이 weapon component를 거쳐 실제 공격 action 실행으로 이어지는 경로가 필요했다.

### Action 실행 단위 분리 필요성

공격 입력을 Player나 WeaponComponent 안에서 바로 montage 재생으로 처리하면, 이후 ComboAttack, collision, damage 처리로 확장할 때 책임이 쉽게 섞일 수 있었다.

따라서 공격 실행을 담당하는 Action Object를 분리하고, Player 입력 계층은 실행 요청만 전달하도록 구조를 나눌 필요가 있었다.

### Montage 실행 데이터 분리 필요성

LightAttack은 단일 montage 실행이지만, montage, play rate, 이동 허용 여부는 action 로직과 분리되어야 했다.

이 값을 `FActionData`로 분리해두면 이후 ComboAttack에서 단계별 action data로 확장할 수 있다.

### Notify timing 기반 종료 처리 필요성

Action state와 movement 제한은 montage 재생 시작과 종료 timing에 맞춰 바뀌어야 했다.

따라서 montage notify가 action의 Begin / End timing을 전달하고, action이 state와 movement 복구를 처리하는 연결이 필요했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. Player action input 전달 흐름 구성

- **왜**:
  Player의 공격 입력이 실제 LightAttack 실행 객체까지 도달해야 했다.
  또한 sword를 장착하지 않은 상태에서는 공격 실행으로 이어지지 않아야 했다.

- **어떻게**:
  `ACPlayerController::PressAction()`이 `ACPlayer::HandleAction()`을 호출하고, `ACPlayer`는 현재 weapon type이 `Sword`인지 확인한 뒤 `UCWeaponComponent::PlayAction()`을 호출하도록 구성했다.

- **결과**:
  Player action input은 `ACPlayerController -> ACPlayer -> UCWeaponComponent` 경로로 전달되고, sword 장착 상태에서만 action 실행으로 이어진다.

### 2. UCAction base와 LightAttack 실행 객체 구성

- **왜**:
  공격 실행은 state 전환, montage 실행, notify timing 처리 같은 공통 생명주기를 가져야 했다.
  이를 Player나 WeaponComponent가 직접 처리하면 이후 action 종류가 늘어날 때 확장하기 어렵다.

- **어떻게**:
  `UCAction` base class를 추가하고 owner character, `FActionData`, `UCStateComponent`를 주입 / cache하도록 구성했다.
  `UCAction_LightAttack`은 Idle state와 montage 유효성을 확인한 뒤 `UCAction::PlayAction()`과 `FActionData::Begin_PlayMontage()`를 호출하도록 구현했다.

- **결과**:
  LightAttack 실행은 `UCAction_LightAttack` 안에서 시작 조건 확인, action state 진입, montage 재생 순서로 처리된다.

### 3. WeaponComponent의 Action 생성 / 초기화 구성

- **왜**:
  WeaponComponent는 현재 장착된 weapon과 연결된 action 실행 객체를 소유하고, Player 입력을 해당 action으로 전달할 진입점이 필요했다.

- **어떻게**:
  `UCWeaponComponent::BeginPlay()`에서 `CreateAction()`으로 `UCAction` instance를 생성하고, owner character와 `FActionData`를 `InitializeAction()`으로 주입하도록 구성했다.
  `UCWeaponComponent::PlayAction()`은 생성된 action instance의 `PlayAction()`을 호출하는 진입점으로 두었다.

- **결과**:
  WeaponComponent는 action instance를 소유하고, Player 입력을 자신이 소유한 action 실행 객체로 전달할 수 있다.

### 4. FActionData 기반 montage 실행 정책 분리

- **왜**:
  LightAttack 실행에는 montage, play rate, 이동 허용 여부가 필요했다.
  이 값을 action 로직에 직접 섞으면 montage 설정과 실행 판단이 같은 위치에 묶인다.

- **어떻게**:
  `FActionData`에 `Montage`, `PlayRate`, `bCanMove`를 두고, `Begin_PlayMontage()`와 `End_PlayMontage()`에서 montage 재생과 movement 제한 / 복구를 처리하도록 구성했다.

- **결과**:
  Action은 어떤 action을 실행할지 판단하고, montage 재생 설정과 movement policy 적용은 `FActionData`가 담당한다.

### 5. AnimNotify 기반 action timing 연결

- **왜**:
  Action 시작과 종료 상태는 montage의 실제 notify timing과 연결되어야 했다.
  특히 montage 종료 이후 state와 movement가 복구되어야 다음 입력 흐름이 정상적으로 이어질 수 있었다.

- **어떻게**:
  `UCAnimNotify_Action`을 추가하고, notify의 `Begin` / `End` flow에 따라 `UCWeaponComponent`가 소유한 action instance에 `Begin_PlayAction()` 또는 `End_PlayAction()`을 호출하도록 구성했다.

- **결과**:
  Montage notify timing은 action lifecycle로 전달되고, LightAttack 종료 시 state와 movement policy가 복구된다.

---

## 주요 처리 흐름

이 섹션은 Player 입력이 LightAttack montage 실행과 종료 처리로 이어지는 대표 흐름을 정리한다.

### Action input 전달 흐름

```text
Player action input
-> ACPlayerController::PressAction
-> ACPlayer::HandleAction
-> current weapon type 확인
   - Sword 아님 -> 실행하지 않음
   - Sword     -> UCWeaponComponent::PlayAction
-> UCAction_LightAttack::PlayAction
```

이 흐름은 Player의 공격 입력이 sword 장착 조건을 통과했을 때만 LightAttack action 실행으로 이어지는 과정을 의미한다.

### LightAttack 시작 흐름

```text
UCAction_LightAttack::PlayAction
-> owner / state component 확인
-> Idle state 확인
   - Idle 아님 -> 실행하지 않음
   - Idle     -> montage 유효성 확인
-> UCAction::PlayAction
-> state를 Action으로 변경
-> FActionData::Begin_PlayMontage
-> movement policy 적용
-> montage 재생
```

이 흐름은 LightAttack 실행 조건을 확인한 뒤 action state로 전환하고 montage를 재생하는 과정을 의미한다.

### Action notify 흐름

```text
Action montage notify
-> UCAnimNotify_Action 실행
-> UCWeaponComponent가 소유한 action instance 확인
-> FlowType 확인
   - Begin -> UCAction::Begin_PlayAction
   - End   -> UCAction_LightAttack::End_PlayAction
```

이 흐름은 montage notify timing이 UCWeaponComponent가 소유한 action instance의 시작 / 종료 처리로 전달되는 과정을 의미한다.

### LightAttack 종료 흐름

```text
UCAnimNotify_Action End
-> UCAction_LightAttack::End_PlayAction
-> UCAction::End_PlayAction
-> state를 Idle로 변경
-> FActionData::End_PlayMontage
-> movement policy 복구
```

이 흐름은 LightAttack montage 종료 timing 이후 action state와 movement policy를 복구하는 과정을 의미한다.

---

## 구현 결과

- Player action input은 `ACPlayerController -> ACPlayer -> UCWeaponComponent -> UCAction_LightAttack` 경로로 전달된다.

- Sword 장착 상태와 Idle state에서만 LightAttack이 시작된다.

- `UCAction`은 action state 진입 / 종료와 lifecycle hook을 제공한다.

- `UCAction_LightAttack`은 `FActionData`를 기준으로 단일 LightAttack montage를 실행한다.

- `FActionData`는 montage 재생과 movement 제한 / 복구를 처리한다.

- `UCAnimNotify_Action`은 montage Begin / End timing을 UCWeaponComponent가 소유한 action instance로 전달한다.

---

## 테스트 방법

### Input / Entry

- `Action` input이 `ACPlayerController::PressAction()`으로 binding되어 있는지 확인한다.

- `ACPlayer::HandleAction()`에서 sword 장착 상태일 때만 `UCWeaponComponent::PlayAction()`이 호출되는지 확인한다.

- `UCWeaponComponent::PlayAction()`이 생성된 action instance의 `PlayAction()`을 호출하는지 확인한다.

### LightAttack 실행

- Idle state에서 action input 시 LightAttack montage가 재생되는지 확인한다.

- Idle state가 아닌 경우 LightAttack이 시작되지 않는지 확인한다.

- `FActionData::PlayRate` 기준으로 montage가 재생되는지 확인한다.

### State / Movement

- action 시작 시 `UCStateComponent::SetActionMode()`가 호출되는지 확인한다.

- `FActionData::bCanMove`가 `false`인 경우 montage 시작 시 movement가 제한되는지 확인한다.

- montage end notify 이후 state가 Idle로 복귀하는지 확인한다.

- `FActionData::bCanMove`가 `false`인 경우 action 종료 후 movement가 복구되는지 확인한다.

### Notify Timing

- `UCAnimNotify_Action` `Begin` timing에서 `Begin_PlayAction()`이 호출되는지 확인한다.

- `UCAnimNotify_Action` `End` timing에서 `End_PlayAction()`이 호출되는지 확인한다.

---

## 검증 결과

- Action input이 `ACPlayerController -> ACPlayer -> UCWeaponComponent -> UCAction_LightAttack` 경로로 전달되는 것을 확인했다.

- sword 장착 상태에서 LightAttack montage가 재생되는 것을 확인했다.

- Idle state가 아닌 경우 LightAttack이 시작되지 않는 것을 확인했다.

- action 시작 시 Action state로 진입하고, action 종료 notify 이후 Idle state로 복귀하는 것을 확인했다.

- `FActionData::bCanMove` 기준으로 movement 제한 / 복구가 처리되는 것을 확인했다.

---

## 비범위

- P04에서는 ComboAttack, hit collision, damage 계산, target damage 수신, hit reaction 처리를 구현하지 않는다.

- 여러 단계 공격 입력과 선입력 구간은 후속 ComboAttack 범위로 남는다.

- 공격 collision과 damage 처리는 후속 hit collision / damage 처리 범위로 남는다.

---

## 관련 문서

- Issue Checklist: `D05_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

- P04는 Player action input이 weapon component를 거쳐 단일 LightAttack montage 실행으로 이어지는 Action Execution Pipeline의 기본 골격을 만든 PR이다.

- 입력 전달, action object, action data, notify timing을 분리해 이후 ComboAttack, hit collision, damage 처리로 확장할 수 있게 했다.

- 이 브랜치의 완료 범위는 LightAttack 1타 실행과 종료 복구까지이며, 연속 공격과 충돌 / damage 처리는 후속 브랜치에서 확장한다.
