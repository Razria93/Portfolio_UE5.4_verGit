# UE5 Portfolio Pull Request

## 제목

**P05: Action Execution Pipeline의 ComboAttack 확장**

## 날짜

**2025.12.22**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-combo-attack`

---

## 요약

이번 PR에서는 **Player가 공격 입력을 반복했을 때, 정해진 입력 허용 구간 안에서만 다음 공격으로 이어지는 연속 공격(ComboAttack) 흐름을 구현했다.**

P04에서 구성한 기본 action 실행 흐름을 3단계 ComboAttack으로 확장하고, 공격 montage의 notify timing에 맞춰 입력을 저장한 뒤 다음 공격 단계에서 소비하도록 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **ComboAttack 실행 흐름 구성**: 공격 입력이 들어오면 첫 번째 공격 montage를 실행하고, 이후 입력 허용 구간 안에서 들어온 재입력만 다음 공격으로 이어지도록 구성했다.

- **Combo step별 montage 실행**: 공격 단계마다 다른 montage, 재생 속도, 이동 허용 여부를 사용할 수 있도록 `FActionData`를 배열로 확장했다.

- **PreInput Window 구성**: montage notify timing으로 입력 허용 구간을 열고 닫아, combo 입력을 받을 수 있는 시점을 제어했다.

### Refactoring

- **공격 입력 처리 기준 분리**: 최초 입력은 공격 시작으로 처리하고, 공격 중 재입력은 즉시 실행하지 않고 다음 combo step 후보로 저장하도록 나눴다.

- **combo 상태 관리 책임 분리**: `UCAction_ComboAttack`이 현재 combo step과 저장된 재입력 상태를 관리하도록 정리했다.

- **notify timing 책임 정리**: `UCAnimNotify_PreInput`은 입력 허용 구간을 제어하고, `UCAnimNotify_Action`의 `Next` timing은 저장된 입력을 다음 공격 실행으로 연결하도록 역할을 나눴다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

```text
Action Execution Pipeline(action 실행 흐름)
-> Player action input이 action object로 전달되고, montage 실행 / notify timing / 종료 복구로 이어지는 실행 흐름
-> P05에서는 이 흐름 위에 ComboAttack의 단계별 실행 규칙을 추가함
```

```text
ComboAttack(연속 공격)
-> 하나의 공격 입력 흐름 안에서 1타, 2타, 3타처럼 단계별 공격 montage로 이어지는 action
```

```text
Combo Step(연속 공격 단계)
-> 현재 ComboAttack이 몇 번째 공격 montage를 실행 중인지 나타내는 단계
-> 코드에서는 `UCAction_ComboAttack::Index`가 이 역할을 담당함
```

```text
PreInput Window(선입력 허용 구간)
-> 현재 공격 montage 중 다음 공격 입력을 받아도 되는 구간
-> 이 구간 안의 재입력만 다음 Combo Step으로 이어질 수 있음
```

```text
PreInput(선입력)
-> PreInput Window 안에서 들어온 재입력
-> 즉시 다음 montage를 재생하지 않고, `Next` timing에서 소비할 값으로 저장됨
```

```text
FActionData(action 실행 데이터)
-> Combo Step별 montage, play rate, movement 허용 여부를 담는 실행 데이터
```

```text
Action Notify Next(action next timing)
-> 저장된 PreInput이 있을 때 다음 Combo Step montage를 실행하는 notify timing
```

---

## 변경 배경

이 섹션은 P04의 Action Execution Pipeline 기본 골격 이후, 공격 입력을 단계별 ComboAttack으로 확장해야 했던 이유를 정리한다.

### 단일 공격에서 연속 공격으로 확장 필요성

P04에서는 Player 입력이 action object로 전달되고, LightAttack montage 1개를 재생하는 Action Execution Pipeline의 기본 골격을 구성했다.

하지만 3D action combat에서는 공격 입력이 한 번의 montage 재생으로 끝나지 않고, 입력 timing에 따라 2타 / 3타로 이어지는 연속 공격 흐름이 필요했다.

### 입력 허용 구간 기준 필요성

ComboAttack은 재입력이 들어올 때마다 즉시 다음 공격을 실행하면 안 된다.

현재 montage의 특정 구간 안에서 들어온 입력만 다음 공격 단계로 인정해야 하므로, montage notify timing을 기준으로 입력 허용 구간을 열고 닫는 구조가 필요했다.

### 다음 공격 실행 timing 분리 필요성

PreInput Window 안에서 재입력이 들어와도 현재 montage를 바로 끊고 다음 montage를 재생하는 구조는 아니다.

따라서 재입력은 먼저 저장하고, montage의 `Next` timing에서 저장된 입력을 소비해 다음 Combo Step을 실행하는 구조가 필요했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. Combo Step별 FActionData 배열 구성

- **왜**:
  기존 LightAttack은 하나의 action data로 하나의 montage만 실행했다.
  ComboAttack은 1타 / 2타 / 3타가 서로 다른 montage와 실행 조건을 가질 수 있어 단계별 실행 데이터가 필요했다.

- **어떻게**:
  `UCWeaponComponent`가 `TArray<FActionData>`를 가지고, `UCAction::InitializeAction()`을 통해 action에 주입하도록 확장했다.
  `UCAction_ComboAttack`은 현재 `Index`에 해당하는 `FActionData`를 사용해 montage를 시작하고 종료한다.

- **결과**:
  ComboAttack은 하나의 action 안에서 Combo Step별 montage, play rate, movement policy를 구분해 사용할 수 있다.

### 2. ComboAttack 실행 객체 추가

- **왜**:
  ComboAttack은 현재 몇 번째 공격인지, 선입력 구간이 열려 있는지, 다음 단계로 소비할 입력이 저장되어 있는지를 action 내부에서 관리해야 했다.

- **어떻게**:
  `UCAction_ComboAttack`을 추가하고 `Index`, `bEnablePreInput`, `bExistPreInput`을 관리하도록 구성했다.
  최초 실행 시에는 실행 조건을 검증한 뒤 현재 `Index`의 montage를 재생하고, 종료 시에는 combo 상태를 초기화한다.

- **결과**:
  ComboAttack의 현재 단계와 선입력 상태가 `UCAction_ComboAttack` 안에서 관리된다.

### 3. 최초 입력과 재입력 처리 분리

- **왜**:
  최초 입력은 ComboAttack 시작으로 처리해야 하지만, 공격 중 재입력은 현재 montage를 즉시 바꾸지 않고 다음 공격 후보로 저장해야 했다.

- **어떻게**:
  `UCAction_ComboAttack::PlayAction()`에서 `bEnablePreInput`을 먼저 확인하도록 구성했다.
  PreInput Window가 열려 있으면 `bExistPreInput`을 `true`로 저장하고, 열려 있지 않은 최초 입력은 Idle 상태와 weapon type 등을 검증한 뒤 첫 번째 montage를 실행한다.

- **결과**:
  같은 action input이라도 현재 timing에 따라 ComboAttack 시작 또는 다음 단계 선입력으로 구분된다.

### 4. PreInput Window 제어 notify 추가

- **왜**:
  combo 입력은 montage의 특정 구간에서만 인정되어야 하므로, 입력 허용 구간을 animation timing에 맞춰 제어해야 했다.

- **어떻게**:
  `UCAnimNotify_PreInput`을 추가하고, notify의 `Begin` / `End` timing에서 현재 `UCAction_ComboAttack`의 `OnEnablePreInput()` / `OffEnablePreInput()`을 호출하도록 구성했다.

- **결과**:
  montage가 지정한 timing에 맞춰 PreInput Window를 열고 닫을 수 있다.

### 5. Action Notify Next 기반 Combo Step 소비

- **왜**:
  저장된 PreInput은 입력 시점에 즉시 실행되는 것이 아니라, 다음 공격으로 넘어갈 수 있는 montage timing에서 소비되어야 했다.

- **어떻게**:
  `UCAnimNotify_Action`의 `Next` flow에서 `UCAction::Next_PlayAction()`을 호출하도록 확장했다.
  `UCAction_ComboAttack::Next_PlayAction()`은 `bExistPreInput`이 있을 때만 `Index`를 증가시키고 다음 `FActionData` montage를 재생한다.

- **결과**:
  PreInput Window 안에서 저장된 입력은 `Next` timing에만 다음 Combo Step으로 이어진다.

### 6. ComboAttack 종료 초기화

- **왜**:
  ComboAttack이 끝난 뒤 이전 step이나 선입력 상태가 남으면 다음 공격 시작에 영향을 줄 수 있었다.

- **어떻게**:
  `UCAction_ComboAttack::End_PlayAction()`에서 현재 montage 종료 처리를 수행한 뒤 `Index`, `bEnablePreInput`, `bExistPreInput`을 초기화하도록 구성했다.

- **결과**:
  ComboAttack 종료 이후 다음 공격은 다시 첫 번째 step에서 시작할 수 있다.

---

## 주요 처리 흐름

이 섹션은 공격 입력이 ComboAttack 시작, 선입력 저장, 다음 공격 step 실행으로 이어지는 대표 흐름을 정리한다.

### ComboAttack 시작 흐름

```text
Player action input
-> UCWeaponComponent::PlayAction
-> UCAction_ComboAttack::PlayAction
-> owner / state / weapon type / action data 확인
-> UCAction::PlayAction
-> 현재 Index의 FActionData 확인
-> 첫 번째 combo montage 재생
```

이 흐름은 Player의 공격 입력이 ComboAttack action으로 전달되고, 실행 조건을 통과한 뒤 첫 번째 공격 montage가 재생되는 과정을 의미한다.

### PreInput 저장 흐름

```text
UCAnimNotify_PreInput Begin
-> PreInput Window open
-> Player action input 재입력
-> UCAction_ComboAttack::PlayAction
-> bEnablePreInput 확인
-> bExistPreInput 저장
-> 현재 montage는 그대로 유지
-> UCAnimNotify_PreInput End
-> PreInput Window close
```

이 흐름은 공격 중 재입력이 들어왔을 때 즉시 다음 montage를 실행하지 않고, 다음 Combo Step으로 이어질 입력만 저장하는 과정을 의미한다.

### Combo Step 전환 흐름

```text
UCAnimNotify_Action Next
-> UCAction_ComboAttack::Next_PlayAction
-> bExistPreInput 확인
   - false -> 현재 combo 유지
   - true  -> bExistPreInput 초기화
           -> Index 증가
           -> 다음 FActionData 확인
           -> 다음 combo montage 재생
```

이 흐름은 montage의 `Next` timing에서 저장된 PreInput을 소비해 다음 Combo Step으로 넘어가는 과정을 의미한다.

### ComboAttack 종료 흐름

```text
UCAnimNotify_Action End
-> UCAction_ComboAttack::End_PlayAction
-> UCAction::End_PlayAction
-> 현재 FActionData montage 종료 처리
-> Index 초기화
-> bEnablePreInput 초기화
-> bExistPreInput 초기화
```

이 흐름은 ComboAttack 종료 시 action state와 combo runtime 값을 초기화하는 과정을 의미한다.

---

## 구현 결과

- Player action input은 `UCWeaponComponent`를 통해 `UCAction_ComboAttack` 실행으로 이어진다.

- ComboAttack은 `TArray<FActionData>`를 사용해 step별 montage와 실행 설정을 가진다.

- `UCAnimNotify_PreInput`은 montage timing에 맞춰 PreInput Window를 열고 닫는다.

- PreInput Window 안에서 들어온 재입력은 `bExistPreInput`에 저장된다.

- `UCAnimNotify_Action`의 `Next` timing은 저장된 PreInput이 있을 때만 다음 Combo Step을 실행한다.

- ComboAttack 종료 시 `Index`, `bEnablePreInput`, `bExistPreInput`이 초기화된다.

---

## 테스트 방법

### ComboAttack 시작

- sword 장착 상태에서 action input 시 첫 번째 ComboAttack montage가 재생되는지 확인한다.

- unarmed 상태에서는 ComboAttack이 시작되지 않는지 확인한다.

- Idle state가 아닌 경우 ComboAttack이 시작되지 않는지 확인한다.

### PreInput Window

- `UCAnimNotify_PreInput` `Begin` 이후 `End` 이전에 action input을 다시 입력하면 `bExistPreInput`이 저장되는지 확인한다.

- PreInput Window 밖에서 action input을 다시 입력하면 다음 Combo Step으로 이어지지 않는지 확인한다.

### Combo Step 전환

- `UCAnimNotify_Action` `Next` timing에서 저장된 PreInput이 있을 때 2타 / 3타 montage로 이어지는지 확인한다.

- 저장된 PreInput이 없으면 `Next` timing에서도 다음 Combo Step이 실행되지 않는지 확인한다.

- 마지막 Combo Step 이후 범위를 벗어난 montage가 재생되지 않는지 확인한다.

### ComboAttack 종료

- `UCAnimNotify_Action` `End` timing에서 action state가 Idle로 복귀하는지 확인한다.

- ComboAttack 종료 후 `Index`, `bEnablePreInput`, `bExistPreInput`이 초기화되는지 확인한다.

- 종료 후 다시 action input을 입력하면 첫 번째 Combo Step부터 시작되는지 확인한다.

---

## 검증 결과

- sword 장착 상태에서 첫 번째 ComboAttack montage가 시작되는 것을 확인했다.

- PreInput Window 안에서 들어온 재입력이 저장되고, `Next` timing에서 다음 Combo Step으로 소비되는 것을 확인했다.

- PreInput Window 밖의 재입력은 다음 Combo Step으로 이어지지 않는 것을 확인했다.

- 1타 / 2타 / 3타가 `FActionData` 배열 기준으로 재생되는 것을 확인했다.

- ComboAttack 종료 후 `Index`, `bEnablePreInput`, `bExistPreInput`이 초기화되는 것을 확인했다.

- action 종료 후 state가 Idle로 복귀하는 것을 확인했다.

---

## 비범위

- P05에서는 hit collision, damage 계산, target damage 수신, hit reaction 처리를 구현하지 않는다.

- P05에서는 Player 기반 ComboAttack 실행만 다루고, AI와 공통 action 실행 경로를 공유하는 구조는 후속 범위로 남는다.

- ComboAttack 중 피격으로 공격이 중단되는 처리는 후속 reaction 처리 범위로 남는다.

---

## 관련 문서

- Issue Checklist: `D06_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

- P05는 P04의 Action Execution Pipeline 위에, Player 재입력이 정해진 PreInput Window 안에서만 다음 Combo Step으로 이어지는 ComboAttack 실행 규칙을 추가한 PR이다.

- `UCAction_ComboAttack`, `UCAnimNotify_PreInput`, `UCAnimNotify_Action`이 각각 combo 상태 관리, 입력 허용 구간 제어, 다음 step 실행 timing 전달 역할을 나눠 갖도록 정리했다.

- 이후 P06의 hit collision, P07의 damage 처리, P15 이후의 공통 action request 구조로 확장할 수 있는 기본 combo action 실행 기준을 남겼다.
