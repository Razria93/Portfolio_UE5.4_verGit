# TB W05-04 Player Target Lock Assist v1

## 작업명

Player Target Lock Assist v1

## 브랜치

```text
feat/player-targeting-component
```

## 상태

```text
구현 완료 (PIE 검증 대기)
```

## 목적

Player CurrentTarget이 존재하는 동안 카메라와 캐릭터 정면을 타겟 방향으로 유지하고, 전투 및 Guard 상태에서 타겟을 바라보며 8Way 이동할 수 있게 한다.

타겟 선택·전환·유효성은 기존 `UCTargetingComponent`가 유지하며, 락온에 반응하는 카메라·이동 정책은 별도의 `UCTargetLockAssistComponent`가 담당한다.

## 책임 경계

```text
UCTargetingComponent
- CurrentTarget 선택 / 전환 / 해제
- 타겟 생존 / 거리 유효성

UCTargetLockAssistComponent
- OnTargetChanged 관찰
- 락온 카메라 추적
- 락온 여부에 따른 Player 회전 정책 연결
- 락온 중 자유 시점 입력 억제 정책 제공

UCMovementComponent
- 카메라 기준 이동 입력 적용
- 지속 Rotation Mode 적용
- 일시적 Gait Override 적용 및 복구

UCDefenseComponent
- Guard 상태와 Guard Walk Gait만 관리
- 캐릭터 정면 정책을 소유하지 않음
```

## 이동 및 정면 정책

### 락온 없음

```text
Normal / Combat / Guard
-> 이동 입력 방향과 캐릭터 정면 일치
-> EMovementRotationMode::OrientToMovement
```

Guard 중에도 별도 FixedFacing을 적용하지 않는다. 정지 중에는 마지막 정면을 유지하고, 이동 입력이 들어오면 이동 방향으로 회전한다.

### 락온 있음

```text
Normal / Combat / Guard
-> 카메라가 CurrentTarget 추적
-> 캐릭터 정면은 CurrentTarget 방향
-> EMovementRotationMode::ControllerDesired
-> 전후좌우 8Way 이동
```

기존 `UCMovementComponent::CalculateDirection()`이 캐릭터 정면과 속도 방향의 각도를 계산하므로 별도 8Way 방향 계산은 추가하지 않는다.

## Movement 책임 분리

기존 `ApplyMovementOverride(Gait, Rotation)`는 Gait와 Rotation 소유권을 결합한다. 현재 실제 호출자는 Guard뿐이므로 다음처럼 분리한다.

```cpp
void ApplyMovementGaitOverride(EMovementGait InGait);
void ClearMovementGaitOverride();
void SetMovementRotationMode(EMovementRotationMode InRotationMode);
```

Guard는 `ApplyMovementGaitOverride(Walk)`만 사용한다. 락온 여부는 `SetMovementRotationMode()`를 통해 회전 정책을 변경한다.

현재 필요한 Rotation Override 사용처가 없으므로 Base / Override 이중 회전 계층은 추가하지 않는다.

## 카메라 정책

- 락온 중 PlayerController의 자유 Look Yaw / Pitch 입력을 억제한다.
- 카메라 목표 회전은 Player ViewPoint에서 `TargetLocation + TargetFocusOffset`을 바라보는 회전으로 계산한다.
- 현재 ControlRotation에서 목표 회전까지 `RInterpTo()`로 보간한다.
- 타겟 전환 시 새 타겟 방향으로 연속 보간한다.
- 락온 해제 시 자동 추적만 종료하고 마지막 카메라 각도는 유지한다.
- 락온 해제 직후 자유 Look 입력을 복구한다.
- 별도의 자유 시점 Offset 또는 락온 중 수동 카메라 조작은 후속 확장하지 않는다.

초기 튜닝:

```cpp
USTRUCT(BlueprintType)
struct FTargetLockAssistTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Targeting|LockAssist", meta = (ClampMin = "0.0"))
    float CameraRotationInterpSpeed = 8.f;

    UPROPERTY(EditAnywhere, Category = "Targeting|LockAssist")
    FVector TargetFocusOffset = FVector(0.f, 0.f, 80.f);
};
```

## Target 변경 처리

```text
nullptr -> Enemy
-> ControllerDesired
-> Camera Tracking 시작
-> Look Input 억제

EnemyA -> EnemyB
-> ControllerDesired 유지
-> Camera Tracking 대상만 변경

Enemy -> nullptr
-> OrientToMovement
-> Camera Tracking 종료
-> Look Input 복구
```

사망, 거리 초과, 직접 Destroy로 `CurrentTarget`이 해제되는 경우도 동일한 `OnTargetChanged` 경로를 사용한다.

## Guard 조합 정책

```text
Guard + 락온 없음
-> Walk Gait
-> OrientToMovement

Guard + 락온 있음
-> Walk Gait
-> ControllerDesired
-> Guard 8Way

Guard 중 락온 해제
-> Walk Gait 유지
-> OrientToMovement

Guard 중 락온 시작
-> Walk Gait 유지
-> ControllerDesired
```

Guard 시작·종료는 현재 락온 회전 정책을 변경하지 않는다.

## Possession 및 종료 계약

- `ACPlayerController`가 Assist Component를 소유한다.
- `PostInitializeComponents()`에서 Controller와 TargetingComponent를 주입한다.
- `OnPossess()`에서 조종 중인 `ACPlayer`를 연결한다.
- `OnUnPossess()`와 `EndPlay()`에서 이전 Player를 `OrientToMovement`로 복구하고 참조를 해제한다.
- 참조 연결 시 이미 CurrentTarget이 존재한다면 즉시 현재 락온 정책을 동기화한다.

## 제외 범위

- 락온 중 자유 시점 Offset
- 카메라 숄더 전환
- 타겟 거리 기반 Boom Length 조절
- 화면 구도 보정 및 다중 타겟 Framing
- 공격 대상을 CurrentTarget으로 강제
- Root Motion Action/Reaction의 별도 회전 Override
- 타겟 마커 및 Enemy Status HUD

## 구현 및 검증 기록

- 기존 Movement의 결합 Override API를 Gait Override와 Rotation Mode 명령으로 분리했다.
- Guard는 Walk Gait만 적용하며 더 이상 `FixedFacing`을 요청하지 않는다.
- `UCTargetLockAssistComponent`가 Target 변경, Possession, 카메라 추적과 회전 정책 연결을 담당한다.
- 락온 중 Look Yaw / Pitch 입력을 억제하고 타겟 방향으로 ControlRotation을 보간한다.
- 타겟 선택·전환·해제는 기존 `UCTargetingComponent` 계약을 그대로 사용한다.
- Debug Overlay Movement 행에 현재 Rotation Mode를 추가했다.
- UHT 및 `PortfolioEditor Win64 Development` 빌드가 성공했다.
- 현재 프로젝트에는 W05-04용 자동화 테스트가 없어 런타임 동작은 PIE 수동 검증을 남겨 둔다.

## 예상 변경 파일

```text
Docs/06_notes/task_briefs/W05_Player_Targeting/README.md
Docs/06_notes/task_briefs/W05_Player_Targeting/TB_W05_04_Player_Target_Lock_Assist_v1.md

Source/Portfolio/Type/CTargetingTypes.h
Source/Portfolio/Component/CMovementComponent.h
Source/Portfolio/Component/CMovementComponent.cpp
Source/Portfolio/Component/CDefenseComponent.h
Source/Portfolio/Component/CDefenseComponent.cpp
Source/Portfolio/Component/CTargetLockAssistComponent.h
Source/Portfolio/Component/CTargetLockAssistComponent.cpp
Source/Portfolio/Controller/CPlayerController.h
Source/Portfolio/Controller/CPlayerController.cpp
Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.cpp
```

## 완료 조건

- 비락온 Normal / Combat / Guard 이동에서 캐릭터 정면이 이동 방향과 일치한다.
- 락온 Normal / Combat / Guard 이동에서 캐릭터가 타겟을 바라보며 8Way로 이동한다.
- Guard는 Walk Gait만 변경하고 Rotation Mode를 덮어쓰지 않는다.
- Guard 중 락온 시작·해제가 Gait를 변경하지 않고 회전 정책만 전환한다.
- 락온 중 자유 Look Yaw / Pitch 입력이 억제된다.
- 락온 해제 즉시 자유 Look 입력이 복구된다.
- 타겟 전환 시 카메라가 새 타겟으로 부드럽게 연결된다.
- 타겟 사망·거리 초과·Destroy 해제 시 `OrientToMovement`로 복구된다.
- Possess / UnPossess / EndPlay에서 이전 Player에 락온 회전 정책이 남지 않는다.
- 기존 W05-01~03 타겟 선택·전환 계약이 유지된다.
- Debug Overlay Movement 행에서 현재 Rotation Mode와 8Way Direction을 확인할 수 있다.
- `PortfolioEditor Win64 Development` 빌드가 성공한다.

## 후속 작업

```text
W05-05: 타겟 마커와 Enemy Status HUD
```
