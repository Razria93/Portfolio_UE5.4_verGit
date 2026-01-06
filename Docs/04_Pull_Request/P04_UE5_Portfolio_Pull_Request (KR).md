# 액션키 입력 라우팅 및 AnimNotify 기반 액션 플로우 구현

## 제목

✨ feat: 액션키 입력 라우팅 및 AnimNotify 기반 액션 실행/라이트어택 구현 (#12)

## 요약

- Action 키를 추가하고, 입력을 `CPlayerController → CPlayer → CWeaponComponent → CAction` 순으로 라우팅하여 액션 실행 경로를 구성함

- `FActionData`를 추가하여 액션 몽타주 재생(PlayRate 포함)과 이동 제한(`bCanMove`)을 데이터로 제어하도록 구성함

- `UCAction`을 추상 베이스 클래스로 구현하고, `CWeaponComponent`에서 Action 생성/초기화 및 `PlayAction()` 호출을 통합함

- `UCAnimNotify_Action`를 구현하여 몽타주 타이밍에 맞춰 `Begin_PlayAction / End_PlayAction`을 트리거하도록 구성함

- `UCAction_LightAttack`을 구현하여 Idle 상태에서 몽타주 기반 라이트어택을 실행하고, 종료 시 상태/이동을 정리하도록 구성함


---

## 완료 항목

### 1. 액션 입력 바인딩 및 호출 경로 구성

- `CPlayerController`에서 Action 입력 바인딩 추가

- `PressAction()`에서 `CPlayer::HandleAction()` 호출

- `CPlayer::HandleAction()`에서 무기 타입이 Sword일 때 `CWeaponComponent::PlayAction()` 호출

- `CWeaponComponent::PlayAction()`에서 `UCAction::PlayAction()` 실행


---

### 2. FActionData 구조 추가

- `FActionData` 구현

  - `Montage`, `PlayRate`, `bCanMove`를 UPROPERTY(EditAnywhere)로 구성

  - 기본값 초기화 (예: `Montage = nullptr`, `PlayRate = 1.0f`, `bCanMove = true`)

- `PlayMontage()` / `Begin_PlayMontage()` / `End_PlayMontage()` 구현

  - `bCanMove == false`인 경우 `UCMovementComponent::SetStop/SetMove`로 이동 제한 제어

  - Montage가 유효하면 `Begin_PlayMontage`에서 `PlayAnimMontage(Montage, PlayRate)`로 재생


---

### 3. UCAction 베이스 클래스 구현 및 WeaponComponent 초기화 통합

- `UCAction` 추상 베이스 클래스 구현

  - OwnerCharacter/ActionData DI(Injection) 및 `UCStateComponent` 캐싱

  - `PlayAction / Begin_PlayAction / End_PlayAction` API 제공

  - 기본 상태 전환(예: ActionMode 진입, Idle 복귀) 처리

- `CWeaponComponent`에서 Action 생성/초기화

  - `ActionClass`(TSubclassOf) 기반으로 `NewObject<UCAction>()` 생성

  - `InitializeAction(OwnerCharacter, ActionData)` 호출로 데이터 주입


---

### 4. AnimNotify 기반 Begin/End 액션 타이밍 트리거

- `UCAnimNotify` 베이스 구성

  - `FlowType(Begin/End)`를 에디터에서 지정 가능하도록 구성

  - MeshOwner로부터 `CWeaponComponent` 조회 유틸 제공

  - Notify 이름에 FlowType을 포함하도록 `MakeNotifyName()` 구성

- `UCAnimNotify_Action` 구현

  - 현재 `CWeaponComponent`의 `GetAction()`을 가져와서 다음과 같은 조건으로 호출

    - `FlowType == Begin` → `Begin_PlayAction()`

    - `FlowType == End` → `End_PlayAction()`


---

### 5. UCAction_LightAttack 구현

- `UCAction_LightAttack` 구현

  - `PlayAction()`:

    - `Super::PlayAction()`로 ActionMode 진입

    - `ActionData.Begin_PlayMontage()`로 몽타주 재생 및 이동 제한 적용

  - `End_PlayAction()`:

    - `Super::End_PlayAction()`로 Idle 복귀

    - `ActionData.End_PlayMontage()`로 이동 제한 해제


---

## 테스트 방법

1. 프로젝트 실행 후 테스트 레벨에서 플레이

2. 무기를 Sword 모드로 전환(기존 무기 전환 키 사용)

3. Action 키 입력 수행(예: `Action` 바인딩)

4. 라이트어택 동작 확인

   - 몽타주 재생 여부

   - 액션 중 State가 ActionMode로 전환되는지 확인

   - `bCanMove` 값에 따른 이동 제한 적용 여부 확인

5. 몽타주에 `UCAnimNotify_Action`(Begin/End) 배치한 노티파이 동작 확인

   - 노티파이 타이밍에 `Begin_PlayAction / End_PlayAction` 호출 흐름이 정상 동작하는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-light-attack`

- 이슈: #12


---

## 노트

- 책임 분리

  - `CPlayerController`: 입력 바인딩 및 Pawn 라우팅

  - `CPlayer`: 입력 의도 처리(무기 타입/상태 조건 체크 후 컴포넌트 호출)

  - `CWeaponComponent`: Action 생성/초기화/실행(PlayAction) 소유

  - `UCAction`: 액션 실행 단위(상태 전환 + 데이터 기반 몽타주 실행)

  - `UCAnimNotify_Action`: 애니메이션 타이밍 기반 Begin/End 트리거

- 현재 `HandleAction()`은 Sword 타입일 때만 액션을 실행하도록 제한되어 있음(추후 무기 타입별 확장 가능)