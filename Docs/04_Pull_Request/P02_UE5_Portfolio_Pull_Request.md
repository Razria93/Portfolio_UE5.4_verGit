# UE5 Portfolio Pull Request

## 제목

**P02: Movement Component 및 기본 Locomotion AnimBP 구현**

## 날짜

**2025.12.05**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/character-move-core`

---

## 요약

### 작업 요약

본 PR은 Player movement input을 `UCMovementComponent`로 전달하고, movement runtime 값을 AnimBP에 공급하여 기본 locomotion과 jump animation을 구동하는 movement / locomotion pipeline을 구성한 작업이다.

```yaml
Movement input
-> ACPlayerController input binding
-> ACPlayer movement handler 호출
-> UCMovementComponent movement API 호출
-> CharacterMovementComponent movement 적용
-> UCMovementComponent runtime 값 갱신
-> UCAnimInstance parameter 갱신
-> Locomotion / Jump AnimBP 구동
```

### 작업 배경

Player 캐릭터가 테스트 레벨에서 기본 이동, 점프, 카메라 조작을 수행하려면 input handling, movement calculation, character movement apply, animation parameter update, locomotion state transition이 분리되어야 했다.

특히 movement 값을 character 내부에서 직접 계산하기보다 `UCMovementComponent`로 분리하여, 이후 walk / run / sprint, action 중 movement policy, AI movement 확장과 연결할 수 있는 기준을 만들 필요가 있었다.

```yaml
필요한 기준
- Controller input에서 movement component까지의 호출 경로 구성
- movement direction / speed / falling 상태 계산
- walk / run / sprint speed type 적용
- AnimBP에서 사용할 Speed / Direction / bIsInAir 갱신
- Idle / Walk / Run / Jump locomotion state 구성
```

### 구현 방향

```yaml
1. Movement Input Routing 구성
- ACPlayerController -> ACPlayer -> UCMovementComponent로 movement input 전달

2. Movement Component 분리
- UCMovementComponent에서 movement input 적용과 runtime movement 값 계산

3. Locomotion Parameter 연결
- UCAnimInstance가 UCMovementComponent 값을 읽어 AnimBP parameter 갱신

4. Locomotion AnimBP 구성
- Speed / bIsInAir 기반 Idle / Move / Jump state 구성
```

---
## 변경 범위

### Movement / Locomotion Pipeline

#### A. Movement Input Routing 구성

- Player movement input을 controller, player character, movement component 순서로 전달하도록 구성했다.

**Flow**
```yaml
MoveForward / MoveRight input
-> ACPlayerController::InputMoveForward / InputMoveRight 호출
-> ACPlayer::HandleMoveForward / HandleMoveRight 호출
-> UCMovementComponent::OnMoveForward / OnMoveRight 호출
```

**Structure**
```yaml
ACPlayerController
- MoveForward / MoveRight axis binding
- Walk / Jump action binding
- pawn으로 input 전달

ACPlayer
- movement input handler 제공
- UCMovementComponent로 movement request 전달

UCMovementComponent
- movement input 적용
- speed type 전환
- movement runtime 값 계산
```

#### B. UCMovementComponent Movement 처리

- `UCMovementComponent`가 owner character와 `CharacterMovementComponent`를 저장하고, movement input과 speed type을 처리하도록 구성했다.

**Flow**
```yaml
UCMovementComponent::BeginPlay
-> owner character 저장
-> CharacterMovementComponent 저장
```

```yaml
UCMovementComponent::OnMoveForward / OnMoveRight
-> owner character 유효성 확인
-> bCanMove 확인
-> input value 확인
-> controller yaw 기준 movement direction 계산
-> OwnerCharacter_Cached->AddMovementInput 호출
```

**Structure**
```yaml
UCMovementComponent
- SpeedMap                    : ESpeedType별 movement speed
- OwnerCharacter_Cached       : movement owner
- CharacterMovementComp_Cached: Unreal movement component
- CurrentSpeed                : AnimBP에 전달할 현재 속도
- CurrentDirection            : AnimBP에 전달할 현재 방향
- bCanMove                    : movement 허용 여부
- bIsFalling                  : falling 상태
```

#### C. Walk / Run / Sprint Speed Type 구성

- `ESpeedType`과 `SpeedMap`을 통해 movement speed를 변경할 수 있도록 구성했다.

**Flow**
```yaml
Walk input
-> ACPlayerController::Press_Walk 호출
-> ACPlayer::HandleWalk 호출
-> UCMovementComponent::OnWalk 호출
-> SetSpeedType(ESpeedType::Walk) 호출
-> CharacterMovementComp_Cached->MaxWalkSpeed 갱신
```

```yaml
Walk release
-> ACPlayerController::Release_Walk 호출
-> ACPlayer::HandleRun 호출
-> UCMovementComponent::OnRun 호출
-> SetSpeedType(ESpeedType::Run) 호출
```

**Structure**
```yaml
ESpeedType
- Walk
- Run
- Sprint
```

#### D. Jump Input 연결

- Jump input을 Unreal `ACharacter::Jump / StopJumping` API로 연결했다.

**Flow**
```yaml
Jump pressed
-> ACPlayerController::Press_Jump 호출
-> ACPlayer::HandleJump 호출
-> ACharacter::Jump 호출

Jump released
-> ACPlayerController::Release_Jump 호출
-> ACPlayer::HandleStopJump 호출
-> ACharacter::StopJumping 호출
```

#### E. Movement Runtime 값 계산

- AnimBP에서 사용할 movement parameter를 매 tick 계산하도록 구성했다.

**Flow**
```yaml
UCMovementComponent::TickComponent
-> CalculateSpeed 호출
-> CalculateDirection 호출
-> CharacterMovementComp_Cached->IsFalling 조회
-> bIsFalling 갱신
```

**Structure**
```yaml
Runtime Movement Parameter
- CurrentSpeed     : OwnerCharacter velocity Size2D
- CurrentDirection : actor forward와 velocity 사이의 signed angle
- bIsFalling       : CharacterMovementComponent falling 상태
```

#### F. AnimInstance Parameter 연결

- `UCAnimInstance`가 `UCMovementComponent`에서 locomotion parameter를 읽어 AnimBP에 전달하도록 구성했다.

**Flow**
```yaml
UCAnimInstance::NativeBeginPlay
-> owner character 저장
-> UCMovementComponent 조회 / 저장

UCAnimInstance::NativeUpdateAnimation
-> Speed = MovementComp_Cached->GetCurrentSpeed()
-> Direction = MovementComp_Cached->GetCurrentDirection()
-> bIsInAir = MovementComp_Cached->IsFalling()
```

**Structure**
```yaml
UCAnimInstance
- Speed     : BlendSpace 구동 속도
- Direction : movement 방향
- bIsInAir  : jump state 전환 기준
```

#### G. Locomotion / Jump AnimBP 구성

- Speed와 falling 상태를 기준으로 기본 locomotion과 jump state를 구성했다.

**Structure**
```yaml
Locomotion
- Idle
- Walk
- Run
- Speed 기반 BlendSpace

Jump
- Jump Start
- Jump Loop
- Jump End
- bIsInAir 기반 state transition
```

---
## 안정성 보완

### Animation Retarget 안정화 (B01 보완)

#### A. Mannequin -> Quinn Retarget 구성

- UE4 Mannequin 기준 animation을 UE5 Quinn skeleton에 직접 연결하면서 발생한 pose distortion 문제를 retarget pipeline으로 해결했다.
- 자세한 원인과 검증 내용은 `B01` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
UE4 Mannequin animation
-> Mannequin skeleton 기준 정상 재생 확인
-> IK Rig / IK Retargeter 구성
-> Quinn skeleton 기준 animation 생성
-> Locomotion AnimBP에 retarget animation 적용
```

**Structure**
```yaml
Retarget Assets
- IK_AutoGeneratedSource : Mannequin source rig
- IK_AutoGeneratedTarget : Quinn target rig
- RTG_AutoGenerated      : IK Retargeter
```

---
## 주요 Pipeline

### Movement Input Pipeline

```yaml
Movement input
-> ACPlayerController input binding
-> ACPlayer movement handler
-> UCMovementComponent movement API
-> AddMovementInput 호출
```

### Speed Type Pipeline

```yaml
Walk / Run input
-> ACPlayer movement handler
-> UCMovementComponent::OnWalk / OnRun
-> SetSpeedType 호출
-> CharacterMovementComponent MaxWalkSpeed 갱신
```

### Jump Pipeline

```yaml
Jump input
-> ACPlayerController::Press_Jump / Release_Jump
-> ACPlayer::HandleJump / HandleStopJump
-> ACharacter::Jump / StopJumping
-> CharacterMovementComponent falling 상태 갱신
```

### Locomotion Parameter Pipeline

```yaml
Character velocity / movement state
-> UCMovementComponent::TickComponent
-> CurrentSpeed / CurrentDirection / bIsFalling 갱신
-> UCAnimInstance::NativeUpdateAnimation
-> Speed / Direction / bIsInAir 갱신
-> Locomotion / Jump AnimBP 구동
```

### Animation Retarget Pipeline

```yaml
UE4 Mannequin animation
-> IK Rig / IK Retargeter
-> UE5 Quinn animation
-> Locomotion AnimBP 적용
```

---
## 테스트 방법

### Movement Input

- WASD 입력 시 `ACPlayerController -> ACPlayer -> UCMovementComponent` 경로로 movement input이 전달되는지 확인
- Forward / Right input이 controller yaw 기준 방향으로 적용되는지 확인
- input value가 0에 가까운 경우 movement input이 적용되지 않는지 확인

### Speed Type

- Walk input press / release에 따라 Walk / Run speed type이 전환되는지 확인
- `SpeedMap` 값이 `CharacterMovementComponent::MaxWalkSpeed`에 반영되는지 확인

### Jump

- Jump input press 시 `Jump()`가 호출되는지 확인
- Jump input release 시 `StopJumping()`이 호출되는지 확인
- 공중 상태에서 `bIsInAir`가 true로 갱신되는지 확인

### Locomotion AnimBP

- Idle / Walk / Run BlendSpace가 Speed 기준으로 자연스럽게 전환되는지 확인
- Jump Start / Jump Loop / Jump End state가 falling 상태 기준으로 전환되는지 확인
- 착지 후 Idle / Move state로 자연스럽게 복귀하는지 확인

### Animation Retarget

- Quinn mesh에서 retarget animation 재생 시 팔 / 다리 pose distortion이 발생하지 않는지 확인
- Idle / Walk / Run / Jump animation이 Quinn skeleton 기준으로 정상 재생되는지 확인

---
## 검증 결과

- WASD movement input이 player movement로 적용되는 동작 확인
- Walk / Run speed type 전환 확인
- Jump / StopJumping input 동작 확인
- `UCMovementComponent`에서 Speed / Direction / bIsFalling 값 갱신 확인
- `UCAnimInstance`에서 Speed / Direction / bIsInAir 값으로 locomotion AnimBP 구동 확인
- IK Rig / IK Retargeter 기반 retarget 후 Quinn pose distortion 문제 해결 확인

---
## 관련 문서

- Issue Checklist: `D03_UE5_Portfolio_Issue_Checklist.md`

- Bug Report: `B01_UE5_Portfolio_Bug_Report.md`

---
## 정리

이 PR의 핵심은 Player movement input을 `UCMovementComponent`로 분리하고, movement runtime 값을 `UCAnimInstance`와 AnimBP에 연결하여 기본 movement / locomotion pipeline을 만든 것이다.

변경 후에는 input handling, movement calculation, speed type control, jump input, locomotion parameter update, animation retarget의 책임이 분리되어 이후 equipment, action, combat 시스템을 얹을 수 있는 character movement 기반이 마련됐다.

```yaml
ACPlayerController
- movement / walk / jump input binding
- pawn으로 input 전달

ACPlayer
- movement / jump handler 제공
- UCMovementComponent 또는 ACharacter jump API 호출

UCMovementComponent
- movement input 적용
- speed type control
- CurrentSpeed / CurrentDirection / bIsFalling 계산

UCAnimInstance
- UCMovementComponent runtime 값 조회
- Speed / Direction / bIsInAir 갱신

AnimBP
- Speed 기반 locomotion BlendSpace
- bIsInAir 기반 jump state transition
```

이 브랜치에서는 기본 movement / jump / locomotion을 우선 구현했고, equipment, combat action, advanced locomotion은 후속 브랜치에서 확장할 수 있는 경계로 남겼다.

---
