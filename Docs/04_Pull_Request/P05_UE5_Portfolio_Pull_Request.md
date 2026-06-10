# UE5 Portfolio Pull Request

## 제목

**P05: ComboAttack Action Execution Pipeline 구현**

## 날짜

**2025.12.22**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-combo-attack`

---

## 요약

### 작업 요약

본 PR은 기존 LightAttack을 3타 ComboAttack으로 확장하고, montage notify timing에 맞춰 다음 ComboAttack step으로 이어질 input을 저장 / 소비하는 ComboAttack execution pipeline을 구성한 작업이다.

```yaml
Action input
-> UCWeaponComponent::PlayAction 호출
-> UCAction_ComboAttack::PlayAction 호출
-> 첫 번째 FActionData montage 재생
-> UCAnimNotify_PreInput으로 input window 제어
-> 재입력 시 bExistPreInput 저장
-> UCAnimNotify_Action(Next)에서 Next_PlayAction 호출
-> 다음 combo montage 재생
```

### 작업 배경

LightAttack 1타만으로는 공격 흐름이 단일 montage 실행에서 끝나기 때문에, 2~3타로 이어지는 ComboAttack 구조가 필요했다.

ComboAttack은 입력 즉시 다음 montage를 재생하는 방식이 아니라, montage의 특정 input window 안에서 들어온 입력만 다음 attack step으로 인정해야 한다.

따라서 action data를 combo step 단위로 확장하고, notify timing을 기준으로 pre-input을 저장한 뒤, 지정된 next timing에서 다음 montage를 재생하는 구조로 정리했다.

```yaml
필요한 기준
- combo step별 montage / play rate / movement policy 설정
- input window 내부 입력만 ComboAttack input으로 인정
- 재입력은 즉시 실행하지 않고 pre-input으로 저장
- next notify timing에서 저장된 입력 소비
- combo 종료 시 Index / pre-input 상태 초기화
```

### 구현 방향

```yaml
1. ActionData 배열화
- TArray<FActionData>로 combo step별 실행 데이터 관리

2. ComboAttack executor 추가
- UCAction_ComboAttack에서 Index와 pre-input 상태 관리

3. PreInput Window 추가
- UCAnimNotify_PreInput으로 combo 입력 가능 구간 제어

4. Action Notify 흐름 확장
- UCAnimNotify_Action의 Next flow로 다음 combo step 소비
```

---
## 변경 범위

### ComboAttack Execution Pipeline

#### A. FActionData 배열 기반 ComboAttack step 구성

- 단일 action data로 처리하던 공격 데이터를 `TArray<FActionData>`로 확장하여 ComboAttack step별 montage를 관리하도록 구성했다.

**Structure**
```yaml
UCAction
- OwnerCharacter_Injected : action owner
- ActionDatas_Injected    : ComboAttack step별 FActionData 배열
- WeaponComp_Cached       : weapon / action 진입점
- StateComp_Cached        : action state 전환 대상

FActionData
- Montage  : step별 action montage
- PlayRate : montage 재생 속도
- bCanMove : montage 중 movement 허용 여부
```

#### B. UCAction_ComboAttack 추가

- `UCAction_ComboAttack`을 추가하여 ComboAttack step index와 pre-input 상태를 관리하도록 구성했다.

**Structure**
```yaml
UCAction_ComboAttack
- Index           : 현재 ComboAttack step
- bEnablePreInput : pre-input window 활성 여부
- bExistPreInput  : 다음 ComboAttack step으로 소비할 입력 저장 여부
```

#### C. 최초 입력과 재입력 분리

- `PlayAction()` 호출을 최초 입력과 input window 내부 재입력으로 분리했다.

**Flow**
```yaml
First invocation
-> OwnerCharacter / StateComp / WeaponComp 검증
-> weapon type 검증
-> Idle state 검증
-> ActionDatas_Injected 유효성 검증
-> UCAction::PlayAction 호출
-> StateComp_Cached->SetActionMode 호출
-> ActionDatas_Injected[Index].Begin_PlayMontage 호출
```

```yaml
Re-invocation
-> bEnablePreInput 확인
-> bEnablePreInput false로 갱신
-> bExistPreInput true로 저장
-> 현재 입력 시점에서는 montage를 즉시 재생하지 않음
-> 이후 UCAnimNotify_Action(Next) timing에서 다음 montage 재생
```

#### D. PreInput Window 제어

- montage notify를 통해 ComboAttack input window를 열고 닫는 `UCAnimNotify_PreInput`을 추가했다.

**Flow**
```yaml
UCAnimNotify_PreInput::Notify
-> GetWeaponComponent로 UCWeaponComponent 조회
-> UCWeaponComponent::GetAction 호출
-> UCAction_ComboAttack으로 캐스팅
-> FlowType에 따라 pre-input window 제어
```

**Structure**
```yaml
EAnimNotifyFlow::Begin
- UCAction_ComboAttack::OnEnablePreInput 호출
- bEnablePreInput true로 갱신

EAnimNotifyFlow::End
- UCAction_ComboAttack::OffEnablePreInput 호출
- bEnablePreInput false로 갱신
```

#### E. Next Notify 기반 ComboAttack step 소비

- `UCAnimNotify_Action`의 `Next` timing에서 저장된 pre-input을 확인하고 다음 ComboAttack montage를 재생하도록 구성했다.

**Flow**
```yaml
UCAnimNotify_Action(Next)
-> UCAction::Next_PlayAction 호출
-> UCAction_ComboAttack::Next_PlayAction 실행
-> bExistPreInput 확인
-> Index 증가
-> ActionDatas_Injected[Index] 유효성 확인
-> 다음 montage 재생
```

#### F. ComboAttack 종료 초기화

- ComboAttack 종료 시 action state와 runtime 값을 초기화하도록 구성했다.

**Flow**
```yaml
UCAnimNotify_Action(End)
-> UCAction_ComboAttack::End_PlayAction 호출
-> UCAction::End_PlayAction 호출
-> StateComp_Cached->SetIdleMode 호출
-> 현재 FActionData End_PlayMontage 호출
-> Index 0으로 초기화
-> bEnablePreInput false로 초기화
-> bExistPreInput false로 초기화
```

---
## 주요 Pipeline

### Combo Start Pipeline

```yaml
Action input
-> UCWeaponComponent::PlayAction
-> UCAction_ComboAttack::PlayAction
-> execution precondition 검증
-> UCAction::PlayAction
-> StateComp_Cached->SetActionMode
-> FActionData::Begin_PlayMontage
```

### PreInput Pipeline

```yaml
UCAnimNotify_PreInput(Begin)
-> bEnablePreInput true
-> Action input 재호출
-> bExistPreInput true
-> UCAnimNotify_PreInput(End)
-> bEnablePreInput false
```

### Combo Next Pipeline

```yaml
UCAnimNotify_Action(Next)
-> UCAction_ComboAttack::Next_PlayAction
-> bExistPreInput 확인
-> Index 증가
-> 다음 FActionData montage 재생
```

### Combo End Pipeline

```yaml
UCAnimNotify_Action(End)
-> UCAction_ComboAttack::End_PlayAction
-> StateComp_Cached->SetIdleMode
-> current FActionData::End_PlayMontage
-> Index / pre-input 상태 초기화
```

---
## 테스트 방법

### Combo Start

- sword 장착 상태에서 action input 시 1타 montage가 재생되는지 확인
- unarmed 상태에서는 ComboAttack이 시작되지 않는지 확인
- Idle state가 아닌 경우 ComboAttack이 시작되지 않는지 확인

### PreInput Window

- `UCAnimNotify_PreInput(Begin ~ End)` 구간 안에서 action input을 다시 입력하면 `bExistPreInput`이 저장되는지 확인
- input window 밖에서 action input을 다시 입력하면 다음 ComboAttack step으로 이어지지 않는지 확인

### Combo Step

- `UCAnimNotify_Action(Next)` timing에서 저장된 pre-input이 있을 때 2타 / 3타 montage로 이어지는지 확인
- 마지막 ComboAttack step 이후 `Index`와 pre-input 상태가 초기화되는지 확인
- ComboAttack 중 montage / state / movement policy가 step별 `FActionData` 기준으로 적용되는지 확인

---
## 검증 결과

- sword 장착 상태에서 1타 ComboAttack 시작 확인
- pre-input window 내부 재입력 시 다음 ComboAttack montage 재생 확인
- pre-input window 외부 입력은 ComboAttack step으로 소비되지 않는 동작 확인
- ComboAttack 종료 후 `Index`, `bEnablePreInput`, `bExistPreInput` 초기화 확인
- action 종료 후 state가 Idle로 복귀하는 동작 확인

---
## 관련 문서

- Issue Checklist: `D06_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 단일 LightAttack을 ComboAttack step 기반 action으로 확장하고, montage notify timing에 맞춰 입력 저장과 다음 step 재생을 분리한 것이다.

변경 후에는 `UCAction_ComboAttack`이 ComboAttack index와 pre-input 상태를 관리하고, `UCAnimNotify_PreInput`이 input window를 열고 닫으며, `UCAnimNotify_Action(Next)`가 저장된 입력을 소비해 다음 montage를 재생했다.

```yaml
UCAction_ComboAttack
- ComboAttack step Index 관리
- pre-input 저장 / 초기화
- 현재 step montage 재생
- 다음 step montage 재생

UCAnimNotify_PreInput
- input window begin / end 전달
- bEnablePreInput 제어

UCAnimNotify_Action
- Begin / End / Next timing 전달
- action lifecycle과 ComboAttack next timing 호출

FActionData
- ComboAttack step별 montage / play rate / movement policy 제공
```

이 브랜치에서는 ComboAttack input과 montage 연결을 우선 구현했고, hit collision, damage apply, reaction, duplicate hit 방지 같은 전투 판정 처리는 후속 브랜치에서 확장할 수 있는 경계로 남겼다.

---
