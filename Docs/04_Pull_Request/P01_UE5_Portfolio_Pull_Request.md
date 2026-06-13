# UE5 Portfolio Pull Request

## 제목

**P01: Player / Camera Core 및 TestRoom 세팅 구현**

## 날짜

**2025.12.03**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/character-camera-core`

---

## 요약

이번 PR에서는 **3인칭 플레이 확인을 위한 테스트 레벨, Player 캐릭터, 카메라, PlayerController 기본 구성을 구현했다.**

아직 이동 / 전투 기능을 넣기 전 단계에서, 레벨 안에 Player를 배치하고 마우스 입력으로 카메라를 돌려볼 수 있는 최소 플레이 환경을 마련했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **TestRoom 기본 환경 구성**: 기능 검증에 사용할 테스트 레벨과 startup map 설정을 추가했다.

- **Player 기본 actor 구성**: `ACPlayer`에 capsule, mesh, SpringArm, camera component를 구성해 3인칭 캐릭터 기준을 만들었다.

- **Camera look input 연결**: `LookYaw` / `LookPitch` 입력을 `ACPlayerController`에 binding하고, controller rotation으로 전달하도록 구성했다.

### Refactoring

- **Player와 Controller 책임 분리**: Player는 캐릭터와 camera rig를 소유하고, PlayerController는 look input을 받아 controller rotation으로 전달하도록 역할을 나눴다.

- **Camera rig 기준 분리**: camera를 Player mesh에 직접 붙이지 않고, SpringArm을 기준으로 거리 / 높이 / 회전 추종을 관리하도록 구성했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

```text
TestRoom(테스트 레벨)
-> 기능을 추가할 때 Player와 camera 동작을 바로 확인하기 위한 기본 테스트 레벨
```

```text
Camera Rig(3인칭 camera 구성)
-> 3인칭 시점을 만들기 위해 SpringArm과 CameraComponent를 함께 구성한 camera 구조
```

```text
Look Input(camera 회전 입력)
-> mouse X / Y 입력을 controller yaw / pitch 회전으로 전달하는 입력 흐름
```

---

## 변경 배경

이 섹션은 프로젝트 초기에 Player와 camera 기반을 먼저 구성해야 했던 이유를 정리한다.

### 테스트 레벨 필요성

이후 movement, equipment, action, combat 기능을 검증하려면 기능을 바로 실행해볼 수 있는 기본 테스트 레벨이 필요했다.

따라서 `TestRoom`을 만들고 editor startup map으로 설정해, 프로젝트를 열었을 때 기본 플레이 환경을 확인할 수 있도록 해야 했다.

### Player / Camera 기본 구성 필요성

3인칭 게임은 조작 대상이 되는 Player 캐릭터와 그 캐릭터를 따라가는 camera 구성이 먼저 필요하다.

따라서 Player의 capsule / mesh 기준을 잡고, SpringArm과 CameraComponent로 3인칭 camera rig를 구성해야 했다.

### Camera input 분리 필요성

마우스 입력은 Player actor가 직접 처리하기보다 PlayerController가 받아 controller rotation으로 전달하는 편이 기본 입력 흐름에 맞다.

따라서 `ACPlayerController`에서 `LookYaw` / `LookPitch` axis input을 binding하고, `AddYawInput()` / `AddPitchInput()`으로 넘기는 구조가 필요했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. TestRoom과 startup map 구성

- **왜**:
  이후 기능을 검증할 기본 레벨이 필요했고, editor 실행 시 바로 확인할 수 있어야 했다.

- **어떻게**:
  기능 검증용 `TestRoom` level을 구성하고, `DefaultEngine.ini`의 `EditorStartupMap`을 `/Game/00_UnitTest/TestRoom.TestRoom`으로 설정했다.

- **결과**:
  Editor 실행 시 `TestRoom`을 바로 열어 Player / camera 기본 구성을 확인할 수 있다.

### 2. ACPlayer 기본 actor 구성

- **왜**:
  3인칭 플레이를 확인하려면 collision 기준이 되는 capsule, visual mesh, camera rig를 가진 Player actor가 필요했다.

- **어떻게**:
  `ACPlayer` 생성자에서 capsule size를 설정하고, mesh relative location / rotation을 보정했다.
  이후 `USpringArmComponent`와 `UCameraComponent`를 생성해 Player capsule 기준으로 attach하도록 구성했다.

- **결과**:
  `ACPlayer`는 3인칭 캐릭터로 배치할 수 있는 capsule / mesh / camera rig 기본 구성을 가진다.

### 3. Third-person camera rig 구성

- **왜**:
  Camera는 Player 뒤쪽 일정 거리에서 따라가고, controller rotation을 기준으로 회전해야 했다.

- **어떻게**:
  `SpringArmComp`를 capsule에 attach하고, 높이와 arm length를 설정했다.
  `SpringArmComp->bUsePawnControlRotation`을 `true`로 두고, `CameraComp`는 SpringArm에 attach하되 pawn control rotation을 직접 사용하지 않도록 구성했다.

- **결과**:
  Mouse look input으로 controller rotation이 바뀌면 SpringArm이 회전하고, camera는 SpringArm 기준의 3인칭 시점을 제공한다.

### 4. PlayerController camera input binding 구성

- **왜**:
  Mouse X / Y input을 camera yaw / pitch 회전으로 전달해야 했다.

- **어떻게**:
  `DefaultInput.ini`에 `LookYaw`, `LookPitch` axis mapping을 추가하고, `ACPlayerController::SetupInputComponent()`에서 각각 `OnLookYaw()` / `OnLookPitch()`에 binding했다.
  `OnLookYaw()`는 `AddYawInput()`, `OnLookPitch()`는 `AddPitchInput()`을 호출하도록 구성했다.

- **결과**:
  Mouse X / Y 입력으로 Player camera의 좌우 / 상하 회전을 조작할 수 있다.

### 5. Player / Controller Blueprint 구성

- **왜**:
  C++ class를 테스트 레벨에서 실제로 사용할 수 있는 Blueprint asset으로 연결해야 했다.

- **어떻게**:
  `ACPlayer` 기반 `BP_CPlayer`와 `ACPlayerController` 기반 `BP_CPlayerController`를 구성하고, 테스트 레벨용 GameMode에서 사용할 수 있도록 준비했다.

- **결과**:
  `TestRoom`에서 Player와 PlayerController 기반 플레이 환경을 확인할 수 있다.

---

## 주요 처리 흐름

이 섹션은 테스트 레벨에서 Player camera가 구성되고, mouse input으로 camera가 회전하는 대표 흐름을 정리한다.

### Player camera setup 흐름

```text
ACPlayer 생성
-> capsule size 설정
-> mesh 위치 / 회전 보정
-> SpringArm 생성 후 capsule에 attach
-> CameraComponent 생성 후 SpringArm에 attach
-> third-person camera 기준 구성
```

이 흐름은 Player actor가 3인칭 camera를 사용할 수 있는 기본 component 구조를 갖추는 과정을 의미한다.

### Camera input 흐름

```text
Mouse X / Mouse Y
-> LookYaw / LookPitch axis mapping
-> ACPlayerController input binding
-> OnLookYaw / OnLookPitch
-> AddYawInput / AddPitchInput
-> controller rotation 변경
-> SpringArm이 controller rotation 기준으로 camera 회전
```

이 흐름은 mouse input이 controller rotation으로 전달되고, SpringArm을 통해 camera 시점 변화로 이어지는 과정을 의미한다.

### TestRoom startup 흐름

```text
Editor 실행
-> DefaultEngine.ini EditorStartupMap 확인
-> TestRoom 로드
-> BP_CPlayer / BP_CPlayerController 기반 플레이 환경 확인
```

이 흐름은 editor 실행 시 기본 테스트 레벨에서 Player / camera 구성을 확인하는 과정을 의미한다.

---

## 구현 결과

- `TestRoom`이 editor startup map으로 설정되어 기본 테스트 환경을 바로 열 수 있다.

- `ACPlayer`는 capsule / mesh / SpringArm / CameraComponent 기본 구성을 가진다.

- SpringArm은 controller rotation을 따르고, CameraComponent는 SpringArm 기준으로 3인칭 시점을 제공한다.

- `LookYaw` / `LookPitch` axis input은 `ACPlayerController`에서 controller yaw / pitch input으로 전달된다.

- `BP_CPlayer` / `BP_CPlayerController`를 통해 테스트 레벨에서 Player와 camera 구성을 확인할 수 있다.

---

## 테스트 방법

### TestRoom

- Editor 실행 시 `TestRoom`이 startup map으로 로드되는지 확인한다.

- 테스트 레벨에서 Player / camera 확인이 가능한지 확인한다.

### Player / Camera

- `BP_CPlayer`가 `ACPlayer` 기반으로 구성되어 있는지 확인한다.

- Capsule 크기와 mesh 위치 / 회전이 캐릭터 기준에 맞게 보정되어 있는지 확인한다.

- `SpringArmComp`와 `CameraComp`가 정상적으로 attach되어 있는지 확인한다.

- Third-person camera 거리와 높이가 플레이 확인에 적절한지 확인한다.

### Camera Input

- `LookYaw` 입력 시 좌우 camera rotation이 동작하는지 확인한다.

- `LookPitch` 입력 시 상하 camera rotation이 동작하는지 확인한다.

- Mouse Y input이 의도한 방향으로 반전되어 적용되는지 확인한다.

---

## 검증 결과

- `TestRoom` 레벨이 startup map으로 로드되는 것을 확인했다.

- `ACPlayer` / `ACPlayerController` 기반 player / controller class 구성을 확인했다.

- `BP_CPlayer` / `BP_CPlayerController` asset 구성을 확인했다.

- `SpringArmComp` / `CameraComp` 기반 third-person camera 동작을 확인했다.

- `LookYaw` / `LookPitch` axis input 기반 camera rotation 동작을 확인했다.

---

## 비범위

- P01에서는 movement input, locomotion animation, weapon equip, combat action을 구현하지 않는다.

- Player 이동과 jump는 후속 movement / locomotion 범위로 남는다.

- 장착, 공격, damage 처리는 후속 gameplay 기능 범위로 남는다.

---

## 관련 문서

- Issue Checklist: `D02_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

- P01은 3인칭 액션 RPG 프로젝트에서 기능을 검증할 기본 레벨, Player actor, PlayerController, camera rig를 구성한 첫 플레이 환경 PR이다.

- `ACPlayer`는 capsule / mesh / SpringArm / CameraComponent를 소유하고, `ACPlayerController`는 camera look input을 controller rotation으로 전달한다.

- 이 브랜치의 완료 범위는 테스트 레벨과 Player / camera 기본 구성까지이며, movement와 combat 기능은 후속 브랜치에서 확장한다.
