# UE5 Portfolio Pull Request

## 제목

**P04: Action Object 및 LightAttack Action Execution Pipeline 구현**

## 날짜

**2025.10.21**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-light-attack`

---

## 요약

### 작업 요약

본 PR은 Player action input을 weapon component의 action object로 전달한 뒤, action object가 montage 기반 LightAttack을 실행하는 첫 번째 action execution pipeline을 구성한 작업이다.

```yaml
Action input
-> ACPlayerController::PressAction 호출
-> ACPlayer::HandleAction 호출
-> UCWeaponComponent::PlayAction 호출
-> UCAction_LightAttack::PlayAction 호출
-> FActionData::Begin_PlayMontage 호출
-> UCAnimNotify_Action으로 Begin / End timing 전달
-> FActionData::End_PlayMontage 호출
```

### 작업 배경

기존 movement / equipment 흐름에 이어, combat action을 실행할 action object가 필요했다.

LightAttack은 단일 montage 실행이지만, 이후 ComboAttack, hit collision, damage pipeline으로 확장될 기준점이 되기 때문에 input handling, action object, montage data, notify timing을 분리해두는 것이 중요했다.

따라서 player input은 action 의도만 전달하고, `UCWeaponComponent`가 action object를 소유하며, `UCAction`이 state 전환과 montage 실행을 담당하는 구조로 정리했다.

```yaml
필요한 기준
- Player input에서 action execution까지의 호출 경로 구성
- action execution 단위를 UObject 기반 UCAction으로 분리
- montage / play rate / movement policy를 FActionData로 분리
- montage notify timing에서 action begin / end 처리
- action 종료 후 state / movement policy 복구
```

### 구현 방향

```yaml
1. Input Routing 구성
- ACPlayerController -> ACPlayer -> UCWeaponComponent로 action input 전달

2. Action Object 도입
- UCAction base와 UCAction_LightAttack 구현

3. Action Data 구성
- FActionData로 montage와 movement policy 관리

4. Notify Timing 연결
- UCAnimNotify_Action으로 action begin / end timing 전달
```

---
## 변경 범위

### LightAttack Execution Pipeline

#### A. Action Input Routing 구성

- Player action input을 player character와 weapon component를 거쳐 action object로 전달하도록 구성했다.

**Flow**
```yaml
InputComponent Action input
-> ACPlayerController::PressAction 호출
-> ACPlayer::HandleAction 호출
-> weapon type이 Sword인지 확인
-> UCWeaponComponent::PlayAction 호출
-> UCAction::PlayAction 호출
```

**Structure**
```yaml
ACPlayerController
- Action input binding
- PressAction에서 pawn으로 input 전달

ACPlayer
- HandleAction에서 weapon type 확인
- UCWeaponComponent로 action execution 요청

UCWeaponComponent
- UCAction instance 소유
- PlayAction 진입점 제공
```

#### B. FActionData 추가

- action montage 실행에 필요한 montage data와 movement policy를 `FActionData`로 분리했다.

**Structure**
```yaml
FActionData
- Montage  : 실행할 action montage
- PlayRate : montage 재생 속도
- bCanMove : action 중 movement 허용 여부
```

**Flow**
```yaml
FActionData::Begin_PlayMontage
-> UCMovementComponent 조회
-> bCanMove가 false이면 SetStop 호출
-> Montage가 유효하면 PlayAnimMontage 호출

FActionData::End_PlayMontage
-> UCMovementComponent 조회
-> bCanMove가 false이면 SetMove 호출
```

#### C. UCAction Base 추가

- action execution에 필요한 공용 state 전환과 lifecycle hook을 제공하는 `UCAction` base class를 추가했다.

**Structure**
```yaml
UCAction
- OwnerCharacter_Injected : action owner
- ActionData_Injected     : action execution data
- StateComp_Cached        : state 전환 대상
- bBeginAction            : action begin notify 수신 여부
- bIsAction               : action active 여부
```

**Flow**
```yaml
UCAction::InitializeAction
-> owner character 저장
-> FActionData 주입
-> UCStateComponent 조회 / 저장

UCAction::PlayAction
-> bIsAction true
-> StateComp_Cached->SetActionMode 호출

UCAction::End_PlayAction
-> bIsAction false
-> bBeginAction false
-> StateComp_Cached->SetIdleMode 호출
```

#### D. UCAction_LightAttack 구현

- Idle state에서만 실행되는 단일 LightAttack action을 구현했다.

**Flow**
```yaml
UCAction_LightAttack::PlayAction
-> OwnerCharacter / StateComp 검증
-> Idle state 확인
-> Montage 유효성 확인
-> UCAction::PlayAction 호출
-> FActionData::Begin_PlayMontage 호출
```

```yaml
UCAction_LightAttack::End_PlayAction
-> OwnerCharacter / StateComp 검증
-> UCAction::End_PlayAction 호출
-> FActionData::End_PlayMontage 호출
```

#### E. UCWeaponComponent Action 생성 / 초기화

- `UCWeaponComponent`가 action class를 생성하고, `FActionData`를 주입하도록 구성했다.

**Flow**
```yaml
UCWeaponComponent::BeginPlay
-> CreateAction 호출
-> NewObject<UCAction>으로 Action 생성
-> Action->InitializeAction 호출
-> OwnerCharacter와 FActionData 주입
```

**Structure**
```yaml
UCWeaponComponent
- ActionClass : 생성할 UCAction class
- ActionData  : action object에 주입할 FActionData
- Action      : 생성된 UCAction instance
```

#### F. AnimNotify 기반 Action Timing 연결

- montage timing에서 action begin / end를 전달하는 `UCAnimNotify_Action`을 추가했다.

**Flow**
```yaml
UCAnimNotify_Action::Notify
-> GetWeaponComponent로 UCWeaponComponent 조회
-> UCWeaponComponent::GetAction 호출
-> FlowType에 따라 action timing 전달
```

**Structure**
```yaml
EAnimNotifyFlow::Begin
- UCAction::Begin_PlayAction 호출

EAnimNotifyFlow::End
- UCAction::End_PlayAction 호출
```

---
## 주요 Pipeline

### Action Input Pipeline

```yaml
Action input
-> ACPlayerController::PressAction
-> ACPlayer::HandleAction
-> UCWeaponComponent::PlayAction
-> UCAction_LightAttack::PlayAction
```

### LightAttack Start Pipeline

```yaml
UCAction_LightAttack::PlayAction
-> Idle state 확인
-> Montage 유효성 확인
-> UCAction::PlayAction
-> StateComp_Cached->SetActionMode
-> FActionData::Begin_PlayMontage
-> PlayAnimMontage
```

### Action Notify Pipeline

```yaml
Action Montage Notify
-> UCAnimNotify_Action
-> UCWeaponComponent::GetAction
-> UCAction::Begin_PlayAction / End_PlayAction
```

### LightAttack End Pipeline

```yaml
UCAnimNotify_Action(End)
-> UCAction_LightAttack::End_PlayAction
-> UCAction::End_PlayAction
-> StateComp_Cached->SetIdleMode
-> FActionData::End_PlayMontage
-> movement policy 복구
```

---
## 테스트 방법

### Input / Entry

- `Action` input이 `ACPlayerController::PressAction`에 binding 되어 있는지 확인
- `ACPlayer::HandleAction`에서 sword weapon type일 때만 action execution으로 이어지는지 확인
- `UCWeaponComponent::PlayAction`을 통해 action object가 호출되는지 확인

### LightAttack Execution

- Idle state에서 action input 시 LightAttack montage가 재생되는지 확인
- Idle state가 아닌 경우 LightAttack이 시작되지 않는지 확인
- `FActionData::PlayRate` 기준으로 montage가 재생되는지 확인

### State / Movement

- action 시작 시 `StateComp_Cached->SetActionMode`가 호출되는지 확인
- `bCanMove == false`인 경우 montage 시작 시 movement가 제한되는지 확인
- montage end notify 이후 state가 Idle로 복귀하는지 확인
- `bCanMove == false`인 경우 action 종료 후 movement가 복구되는지 확인

### Notify Timing

- `UCAnimNotify_Action(Begin)`에서 `Begin_PlayAction`이 호출되는지 확인
- `UCAnimNotify_Action(End)`에서 `End_PlayAction`이 호출되는지 확인

---
## 검증 결과

- Action input이 `ACPlayerController -> ACPlayer -> UCWeaponComponent -> UCAction` 경로로 전달되는 동작 확인
- sword 장착 상태에서 `UCAction_LightAttack` montage 재생 확인
- action 시작 시 Action state 진입 확인
- action 종료 notify 이후 Idle state 복귀 확인
- `FActionData::bCanMove` 기준 movement 제한 / 복구 동작 확인

---
## 관련 문서

- Issue Checklist: `D05_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 Player input에서 시작된 combat action을 `UCWeaponComponent`가 소유한 `UCAction` object로 전달하고, `FActionData`와 montage notify timing을 통해 LightAttack 1타를 실행하는 기본 action execution pipeline을 만든 것이다.

변경 후에는 input routing, action object, action data, notify timing의 책임이 분리되어 이후 ComboAttack과 hit collision을 붙일 수 있는 기준이 마련됐다.

```yaml
ACPlayerController
- Action input binding
- pawn으로 input 전달

ACPlayer
- weapon type 조건 확인
- UCWeaponComponent action entry 호출

UCWeaponComponent
- UCAction instance 생성 / 소유
- FActionData 주입
- PlayAction entry 제공

UCAction_LightAttack
- Idle state 검증
- action state 전환
- FActionData 기반 montage 실행 / 종료

UCAnimNotify_Action
- montage timing 기준 Begin / End 전달
```

이 브랜치에서는 LightAttack 1타 execution pipeline을 우선 구현했고, ComboAttack chaining, hit collision, damage apply, reaction 처리는 후속 브랜치에서 확장할 수 있는 경계로 남겼다.

---
