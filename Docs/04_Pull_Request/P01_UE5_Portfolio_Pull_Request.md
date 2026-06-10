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

### 작업 요약

본 PR은 3인칭 액션 RPG 프로젝트의 가장 기본이 되는 플레이 환경을 구성한 작업이다.

`TestRoom` 레벨을 만들고, `ACPlayer`, `ACPlayerController`, third-person camera rig를 추가하여 레벨 안에서 플레이어 캐릭터를 배치하고 카메라를 조작할 수 있는 최소 구성을 마련했다.

### 작업 배경

3D 3인칭 게임을 구성하려면 먼저 플레이어가 존재할 레벨, 조작 대상이 되는 캐릭터, 시점을 담당하는 카메라, 그리고 입력을 받아 카메라를 회전시키는 컨트롤러가 필요하다.

따라서 test level setup, player pawn setup, camera rig setup, controller input binding을 분리하여 이후 movement / action / combat 기능을 얹을 수 있는 기본 골격을 구성했다.

### 구현 방향

```yaml
1. TestRoom 세팅
- 기능 검증을 위한 기본 테스트 레벨 구성

2. Player 기본 구성
- ACPlayer C++ class와 BP_CPlayer 구성

3. Camera rig 구성
- SpringArmComp / CameraComp 기반 third-person camera 구성

4. Camera input binding
- ACPlayerController에서 LookYaw / LookPitch axis input 처리
```

---
## 변경 범위

### Test Environment

#### A. TestRoom 레벨 구성

- 기능 검증을 위한 `TestRoom` 레벨과 기본 테스트 asset을 구성했다.

**Structure**
```yaml
Test Assets
- TestRoom : 기능 검증용 테스트 레벨
- GM_Test  : 테스트 레벨용 GameMode asset
- Label_Test : 테스트 레벨 식별용 label asset
```

#### B. Editor Startup Map 설정

- 에디터 실행 시 `TestRoom`을 바로 확인할 수 있도록 startup map을 설정했다.

**Flow**
```yaml
Editor 실행
-> EditorStartupMap 조회
-> /Game/00_UnitTest/TestRoom 로드
-> Player / Camera 기본 구성 확인
```

**Structure**
```yaml
DefaultEngine.ini
- EditorStartupMap : /Game/00_UnitTest/TestRoom.TestRoom
```

### Player / Camera Core

#### A. ACPlayer 기본 구성

- `ACPlayer`를 추가하고, 캐릭터 capsule / mesh / camera component 구성을 초기화했다.

**Flow**
```yaml
ACPlayer::ACPlayer
-> CapsuleComp size 설정
-> MeshComp relative transform 설정
-> SpringArmComp 생성 및 capsule에 attach
-> CameraComp 생성 및 SpringArmComp에 attach
```

**Structure**
```yaml
ACPlayer
- CapsuleComp   : collision 기준 크기 설정
- MeshComp      : visual mesh 위치 / 회전 보정
- SpringArmComp : third-person camera 거리와 회전 기준
- CameraComp    : 실제 camera view component
```

#### B. Third-Person Camera Rig 구성

- `USpringArmComponent`와 `UCameraComponent`를 기반으로 기본 third-person camera rig를 구성했다.

**Structure**
```yaml
SpringArmComp
- Attach Target        : CapsuleComp
- Relative Location    : (0, 0, 55)
- Target Arm Length    : 300
- Use Pawn Control Rot : true

CameraComp
- Attach Target        : SpringArmComp
- Relative Location    : (0, 40, 0)
- Use Pawn Control Rot : false
```

#### C. Player / Controller Blueprint 구성

- C++ class를 기반으로 `BP_CPlayer`, `BP_CPlayerController` asset을 구성했다.

**Structure**
```yaml
Blueprint Assets
- BP_CPlayer           : ACPlayer 기반 player blueprint
- BP_CPlayerController : ACPlayerController 기반 controller blueprint
```

### Camera Input

#### A. Look Axis Mapping 추가

- mouse input을 camera rotation으로 전달하기 위해 `LookYaw`, `LookPitch` axis mapping을 추가했다.

**Structure**
```yaml
DefaultInput.ini
- LookYaw   : MouseX
- LookPitch : MouseY, Scale -1
```

#### B. ACPlayerController Input Binding

- `ACPlayerController`에서 camera look input을 받아 controller rotation에 반영하도록 구성했다.

**Flow**
```yaml
ACPlayerController::SetupInputComponent
-> LookYaw axis binding
-> LookPitch axis binding
-> OnLookYaw에서 AddYawInput 호출
-> OnLookPitch에서 AddPitchInput 호출
```

**Structure**
```yaml
ACPlayerController
- SetupInputComponent : axis input binding
- OnLookYaw           : yaw input 처리
- OnLookPitch         : pitch input 처리
```

---
## 주요 Pipeline

### Player Camera Setup Pipeline

```yaml
ACPlayer 생성
-> CapsuleComp 설정
-> MeshComp transform 보정
-> SpringArmComp attach
-> CameraComp attach
-> third-person camera 기준 구성
```

### Camera Input Pipeline

```yaml
MouseX / MouseY
-> LookYaw / LookPitch axis mapping
-> ACPlayerController::OnLookYaw / OnLookPitch
-> AddYawInput / AddPitchInput 호출
-> SpringArmComp가 controller rotation 기준으로 camera 회전
```

### TestRoom Startup Pipeline

```yaml
Editor 실행
-> EditorStartupMap
-> TestRoom 로드
-> BP_CPlayer / BP_CPlayerController 기반 플레이 환경 확인
```

---
## 테스트 방법

### TestRoom

- 에디터 실행 시 `TestRoom`이 startup map으로 로드되는지 확인
- 테스트 레벨 내에서 player / camera 확인이 가능한지 확인

### Player / Camera

- `BP_CPlayer`가 `ACPlayer` 기반으로 구성되어 있는지 확인
- capsule 크기와 mesh 위치 / 회전이 캐릭터 기준에 맞게 보정되어 있는지 확인
- `SpringArmComp`와 `CameraComp`가 정상적으로 attach되어 있는지 확인
- third-person camera 거리와 높이가 플레이 확인에 적절한지 확인

### Camera Input

- `LookYaw` 입력 시 좌우 camera rotation이 동작하는지 확인
- `LookPitch` 입력 시 상하 camera rotation이 동작하는지 확인
- mouse Y input이 의도한 방향으로 반전되어 적용되는지 확인

---
## 검증 결과

- `TestRoom` 레벨 로드 확인
- `ACPlayer` / `ACPlayerController` 기반 player / controller class 구성 확인
- `BP_CPlayer` / `BP_CPlayerController` asset 구성 확인
- `SpringArmComp` / `CameraComp` 기반 third-person camera 동작 확인
- `LookYaw` / `LookPitch` axis input 기반 camera rotation 동작 확인

---
## 관련 문서

- Issue Checklist: `D02_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 3인칭 액션 RPG의 기본 플레이 단위를 구성하는 것이다.

변경 후에는 `TestRoom` 안에서 `ACPlayer`를 배치하고, `ACPlayer`가 capsule / mesh / camera rig를 소유하며, `ACPlayerController`가 camera look input을 처리하는 구조가 됐다.

이를 통해 레벨, 캐릭터, 카메라, 컨트롤러라는 3D 3인칭 게임의 기본 구성을 먼저 만들고, 이후 movement, locomotion, action, combat 기능을 단계적으로 추가할 수 있는 출발점을 마련했다.

```yaml
ACPlayer
- capsule / mesh / camera rig 구성

ACPlayerController
- LookYaw / LookPitch input binding
- controller rotation 입력 처리

TestRoom
- player / camera 기능 검증용 테스트 환경
```

---
