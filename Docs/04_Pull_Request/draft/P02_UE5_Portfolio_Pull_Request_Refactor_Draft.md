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

이번 PR에서는 **Player가 이동 / 점프 입력에 따라 움직이고, 그 움직임이 기본 locomotion animation으로 이어지는 흐름을 구현했다.**

입력으로 캐릭터를 움직이고, 현재 이동 상태 값을 animation parameter로 전달해 기본 locomotion animation을 구동할 수 있도록 이동 흐름을 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Movement input 전달 흐름 구성**: PlayerController의 이동 입력이 Player를 거쳐 MovementComponent로 전달되도록 구성했다.

- **Walk / Run / Jump 동작 연결**: Walk 입력으로 이동 속도를 전환하고, Jump 입력은 Unreal character jump API로 연결했다.

- **Locomotion AnimBP 구동**: movement runtime 값을 AnimInstance로 전달해 Idle / Walk / Run / Jump animation이 상태에 따라 전환되도록 구성했다.

### Refactoring

- **이동 책임 분리**: Player는 입력을 전달하고, 이동 방향 계산과 speed type 적용은 `UCMovementComponent`가 담당하도록 역할을 나눴다.

- **AnimBP parameter 갱신 책임 정리**: AnimBP가 직접 movement를 계산하지 않고, `UCAnimInstance`가 `UCMovementComponent`의 runtime 값을 읽어 animation parameter를 갱신하도록 정리했다.

- **후속 gameplay 확장 기준 마련**: 이후 장착 / 공격 중 movement 제한, speed type 확장, AI movement 확장에 사용할 movement 제어 지점을 분리했다.

### Troubleshooting

- **Animation retarget 안정화**: UE4 Mannequin 기준 animation을 UE5 Quinn skeleton에 맞게 retarget해 pose distortion 문제를 보완했다. 자세한 원인과 검증은 `B01_UE5_Portfolio_Bug_Report.md`에 분리했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

Movement Runtime Value(이동 runtime 값)
-> 매 tick 계산되는 현재 이동 속도, 이동 방향, 공중 상태 값
-> 코드에서는 `CurrentSpeed`, `CurrentDirection`, `bIsFalling`을 사용함
```

```text
Locomotion Parameter(locomotion parameter)
-> AnimBP가 Idle / Walk / Run / Jump 상태를 전환할 때 사용하는 animation parameter
-> 코드에서는 `Speed`, `Direction`, `bIsInAir`를 사용함
```

```text
Speed Type(이동 속도 타입)
-> Walk / Run / Sprint처럼 movement speed를 구분하는 값
-> 코드에서는 `ESpeedType`과 `SpeedMap`을 사용함
```

```text
Animation Retarget(animation retarget)
-> 다른 skeleton 기준 animation을 현재 skeleton에서 재생할 수 있도록 변환하는 작업
```

---

## 변경 배경

이 섹션은 character movement와 기본 locomotion animation을 별도 흐름으로 분리해야 했던 이유를 정리한다.

### Player 이동 입력 처리 필요성

Player는 테스트 레벨에서 WASD 이동, Walk / Run 전환, Jump 입력을 수행할 수 있어야 했다.

이 입력들은 Controller에서 들어오지만, 실제 이동 처리와 속도 정책은 character의 movement 흐름으로 전달되어야 했다.

### Movement 계산 책임 분리 필요성

이동 방향, 현재 속도, 공중 상태 같은 값은 animation과 gameplay 양쪽에서 반복적으로 사용될 수 있다.

따라서 Player class가 직접 모든 값을 계산하기보다 `UCMovementComponent`가 이동 입력 적용과 runtime 값 계산을 담당하는 구조가 필요했다.

### Locomotion parameter 연결 필요성

AnimBP는 Idle / Walk / Run / Jump 상태를 전환하기 위해 현재 속도, 이동 방향, 공중 상태를 알아야 했다.

따라서 `UCAnimInstance`가 `UCMovementComponent`에서 값을 읽어 AnimBP parameter로 전달하는 연결이 필요했다.

### Animation retarget 안정성 필요성

기존 animation을 UE5 Quinn mesh에서 사용하려면 skeleton 차이로 인한 pose distortion을 피해야 했다.

따라서 Mannequin 기준 animation을 Quinn skeleton에 맞게 retarget해 기본 locomotion animation으로 사용할 수 있게 정리해야 했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. Movement input 전달 흐름 구성

- **왜**:
  Controller에서 들어온 MoveForward / MoveRight 입력이 Player를 거쳐 실제 movement 처리 component까지 전달되어야 했다.

- **어떻게**:
  `ACPlayerController`에서 MoveForward / MoveRight axis input을 binding하고, `ACPlayer::HandleMoveForward()` / `HandleMoveRight()`를 거쳐 `UCMovementComponent::OnMoveForward()` / `OnMoveRight()`를 호출하도록 구성했다.

- **결과**:
  Movement input은 `ACPlayerController -> ACPlayer -> UCMovementComponent` 경로로 전달된다.

### 2. Controller yaw 기준 이동 방향 계산

- **왜**:
  Player movement는 character의 현재 회전이 아니라 camera / controller yaw 기준으로 앞 / 오른쪽 방향을 계산해야 했다.

- **어떻게**:
  `UCMovementComponent::OnMoveForward()` / `OnMoveRight()`에서 owner character의 control rotation yaw를 기준으로 방향 vector를 구하고 `AddMovementInput()`을 호출하도록 구성했다.

- **결과**:
  WASD 이동은 controller yaw 기준 방향으로 적용된다.

### 3. Walk / Run speed type 구성

- **왜**:
  Walk 입력에 따라 character movement speed가 전환되어야 했다.

- **어떻게**:
  `ACPlayerController`의 Walk press / release를 `ACPlayer::HandleWalk()` / `HandleRun()`으로 전달하고, `UCMovementComponent::OnWalk()` / `OnRun()`이 `SetSpeedType()`을 통해 `CharacterMovementComponent::MaxWalkSpeed`를 갱신하도록 구성했다.

- **결과**:
  Walk 입력 상태에 따라 Walk / Run speed type이 적용된다.

### 4. Jump input 연결

- **왜**:
  Player가 jump input으로 점프를 시작하고, release input으로 jump hold를 종료할 수 있어야 했다.

- **어떻게**:
  `ACPlayerController::Press_Jump()` / `Release_Jump()`가 `ACPlayer::HandleJump()` / `HandleStopJump()`를 호출하고, Player는 Unreal `Jump()` / `StopJumping()` API를 호출하도록 연결했다.

- **결과**:
  Jump input은 Unreal character jump 흐름으로 전달된다.

### 5. Movement runtime 값 계산

- **왜**:
  AnimBP와 후속 gameplay는 현재 speed, direction, falling 상태를 기준으로 동작해야 했다.

- **어떻게**:
  `UCMovementComponent::TickComponent()`에서 owner velocity로 `CurrentSpeed`를 계산하고, actor forward와 velocity 사이 각도로 `CurrentDirection`을 계산하며, `CharacterMovementComponent::IsFalling()`으로 `bIsFalling`을 갱신하도록 구성했다.

- **결과**:
  MovementComponent는 AnimBP와 후속 시스템에 전달할 runtime movement 값을 매 tick 갱신한다.

### 6. AnimInstance parameter 연결

- **왜**:
  AnimBP가 locomotion state를 전환하려면 movement runtime 값을 animation parameter로 받아야 했다.

- **어떻게**:
  `UCAnimInstance::NativeBeginPlay()`에서 owner character와 `UCMovementComponent`를 cache하고, `NativeUpdateAnimation()`에서 `Speed`, `Direction`, `bIsInAir` 값을 갱신하도록 구성했다.

- **결과**:
  AnimBP는 `Speed`, `Direction`, `bIsInAir` 값을 기준으로 locomotion / jump state를 구동할 수 있다.

### 7. Locomotion / Jump AnimBP 구성

- **왜**:
  Player movement가 화면에서 Idle / Walk / Run / Jump animation으로 표현되어야 했다.

- **어떻게**:
  `Speed` 값에 따라 Idle / Walk / Run animation이 전환되도록 locomotion BlendSpace를 구성했다.
  `bIsInAir` 값에 따라 Jump Start / Jump Loop / Jump End state로 전환되도록 구성했다.
  각 state에는 Quinn skeleton 기준으로 retarget한 animation을 적용했다.

- **결과**:
  Player는 이동 입력과 jump 상태에 따라 기본 locomotion animation을 재생한다.

### 8. Animation retarget 안정화

- **왜**:
  UE4 Mannequin 기준 animation을 UE5 Quinn skeleton에 직접 연결하면 pose distortion이 발생할 수 있었다.

- **어떻게**:
  Mannequin animation을 원본 skeleton에서 확인한 뒤 IK Rig / IK Retargeter를 사용해 Quinn skeleton 기준 animation으로 변환했다.

- **결과**:
  Quinn mesh에서 Idle / Walk / Run / Jump animation이 안정적으로 재생된다.

---

## 주요 처리 흐름

이 섹션은 Player 입력이 movement 적용과 locomotion animation으로 이어지는 대표 흐름을 정리한다.

### Movement input 흐름

```text
MoveForward / MoveRight input
-> ACPlayerController input binding
-> ACPlayer movement handler
-> UCMovementComponent movement API
-> controller yaw 기준 direction 계산
-> AddMovementInput 호출
```

이 흐름은 Player movement input이 controller yaw 기준 이동 방향으로 변환되어 character movement에 적용되는 과정을 의미한다.

### Speed type 흐름

```text
Walk input press / release
-> ACPlayerController
-> ACPlayer walk / run handler
-> UCMovementComponent::OnWalk / OnRun
-> SetSpeedType
-> CharacterMovementComponent MaxWalkSpeed 갱신
```

이 흐름은 Walk 입력 상태에 따라 movement speed가 전환되는 과정을 의미한다.

### Jump 흐름

```text
Jump input press
-> ACPlayerController::Press_Jump
-> ACPlayer::HandleJump
-> Jump

Jump input release
-> ACPlayerController::Release_Jump
-> ACPlayer::HandleStopJump
-> StopJumping
```

이 흐름은 Player jump 입력이 Unreal character jump API로 전달되는 과정을 의미한다.

### Locomotion parameter 흐름

```text
Character velocity / falling state
-> UCMovementComponent::TickComponent
-> CurrentSpeed / CurrentDirection / bIsFalling 갱신
-> UCAnimInstance::NativeUpdateAnimation
-> Speed / Direction / bIsInAir 갱신
-> Locomotion / Jump AnimBP state 전환
```

이 흐름은 movement runtime 값이 animation parameter로 전달되어 locomotion animation을 구동하는 과정을 의미한다.

---

## 구현 결과

- Player movement input은 `ACPlayerController -> ACPlayer -> UCMovementComponent` 경로로 전달된다.

- `UCMovementComponent`는 controller yaw 기준으로 movement direction을 계산하고 `AddMovementInput()`을 호출한다.

- Walk / Run speed type은 `SpeedMap` 값을 기준으로 `CharacterMovementComponent::MaxWalkSpeed`에 반영된다.

- Jump input은 `Jump()` / `StopJumping()`으로 전달된다.

- `UCMovementComponent`는 `CurrentSpeed`, `CurrentDirection`, `bIsFalling`을 매 tick 갱신한다.

- `UCAnimInstance`는 movement runtime 값을 `Speed`, `Direction`, `bIsInAir`로 AnimBP에 전달한다.

- Retarget된 Quinn animation으로 기본 Idle / Walk / Run / Jump locomotion이 동작한다.

---

## 테스트 방법

### Movement Input

- WASD 입력이 `ACPlayerController -> ACPlayer -> UCMovementComponent` 경로로 전달되는지 확인한다.

- Forward / Right input이 controller yaw 기준 방향으로 적용되는지 확인한다.

- input value가 0에 가까운 경우 movement input이 적용되지 않는지 확인한다.

### Speed Type

- Walk input press / release에 따라 Walk / Run speed type이 전환되는지 확인한다.

- `SpeedMap` 값이 `CharacterMovementComponent::MaxWalkSpeed`에 반영되는지 확인한다.

### Jump

- Jump input press 시 `Jump()`가 호출되는지 확인한다.

- Jump input release 시 `StopJumping()`이 호출되는지 확인한다.

- 공중 상태에서 `bIsInAir`가 `true`로 갱신되는지 확인한다.

### Locomotion AnimBP

- Idle / Walk / Run BlendSpace가 Speed 기준으로 전환되는지 확인한다.

- Jump Start / Jump Loop / Jump End state가 falling 상태 기준으로 전환되는지 확인한다.

- 착지 후 Idle / Move state로 복귀하는지 확인한다.

### Animation Retarget

- Quinn mesh에서 retarget animation 재생 시 팔 / 다리 pose distortion이 발생하지 않는지 확인한다.

- Idle / Walk / Run / Jump animation이 Quinn skeleton 기준으로 재생되는지 확인한다.

---

## 검증 결과

- WASD movement input이 player movement로 적용되는 것을 확인했다.

- Walk / Run speed type 전환을 확인했다.

- Jump / StopJumping input 동작을 확인했다.

- `UCMovementComponent`에서 `CurrentSpeed`, `CurrentDirection`, `bIsFalling` 값이 갱신되는 것을 확인했다.

- `UCAnimInstance`에서 `Speed`, `Direction`, `bIsInAir` 값으로 locomotion AnimBP가 구동되는 것을 확인했다.

- IK Rig / IK Retargeter 기반 retarget 이후 Quinn pose distortion 문제가 해결된 것을 확인했다.

---

## 비범위

- P02에서는 weapon equip / unequip, combat action, hit collision, damage 처리를 구현하지 않는다.

- 장착 / 공격 중 movement 제한 정책은 후속 equipment / action 범위로 남는다.

- 8-way locomotion, motion warping, foot sync 같은 고급 locomotion 기능은 후속 확장 범위로 남는다.

---

## 관련 문서

- Issue Checklist: `D03_UE5_Portfolio_Issue_Checklist.md`

- Bug Report: `B01_UE5_Portfolio_Bug_Report.md`

---

## 정리

- P02는 Player movement input을 `UCMovementComponent`로 분리하고, movement runtime 값을 AnimBP에 연결한 기본 movement / locomotion PR이다.

- 이동 입력 적용, speed type 전환, jump 입력, locomotion parameter 갱신, animation retarget을 분리해 이후 equipment와 combat action에서 movement 제어 지점을 사용할 수 있게 했다.

- 이 브랜치의 완료 범위는 기본 movement / jump / locomotion까지이며, 장착과 combat action은 후속 브랜치에서 확장한다.
