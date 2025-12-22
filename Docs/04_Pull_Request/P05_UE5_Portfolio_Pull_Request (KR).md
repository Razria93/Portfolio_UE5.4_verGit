# 콤보 어택 시스템 구현 및 PreInput 기반 입력 윈도우 도입

## 제목

✨ feat: PreInput 버퍼링 및 AnimNotify 기반 입력 윈도우를 사용하는 콤보 어택 액션 구현 (#14)

## 요약

- 기존 단일 `FActionData` 구조를 `TArray<FActionData>`로 확장하여  
  다단 히트(Combo) 액션을 데이터 기반으로 처리할 수 있도록 구조를 개선함

- `UCAction_ComboAttack` 클래스를 신규 구현하여  
  콤보 단계(Index) 및 PreInput 버퍼링 기반의 연속 공격 흐름을 구성함

- `PlayAction()` 재호출 시점을 최초 호출 / 재호출(Re-entry)로 명확히 분리하고, 재호출 입력을 즉시 실행하지 않고 PreInput으로 저장하는 구조를 도입함

- AnimNotify를 통해 Combo Input Window(Input Window)를 명시적으로 제어하기 위해 `UCAnimNotify_PreInput`을 신규 추가함

- 기존 `UCAction / UCAnimNotify_Action` 구조를 콤보 액션에 맞게 일부 보완함


---

## 완료 항목

### 1. FActionData 배열화 (Combo 대응 구조 개선)

- `FActionData`를 단일 데이터에서 `TArray<FActionData>`로 변경

- 각 콤보 단계별로 개별 Montage / PlayRate / 이동 제어를 설정 가능하도록 확장

- `UCAction::InitializeAction()`에서 다수의 `FActionData`를 주입받아 관리하도록 구조 수정


---

### 2. UCAction_ComboAttack 구현

- `UCAction_ComboAttack` 신규 구현

  - 콤보 단계 관리를 위한 `Index` 변수 도입

  - PreInput 상태 관리를 위한 플래그 추가
    - `bEnablePreInput` : Input Window 활성 여부
    - `bExistPreInput` : 입력 버퍼 저장 여부

- `PlayAction()` 흐름 분리

  - **재호출(Re invocation)**  
    - Input Window 활성 상태에서 `PlayAction()`이 재호출되면  
      → 즉시 실행하지 않고 PreInput으로 저장

  - **최초 호출(First invocation)**  
    - Idle 상태 및 무기/상태 조건 검증 후  
      → 첫 번째 콤보 몽타주 실행

- `Next_PlayAction()`

  - `bExistPreInput == true`일 경우에만 다음 콤보 단계로 진행

  - Index 증가 후 다음 `FActionData`의 몽타주 실행

- `End_PlayAction()`

  - 콤보 종료 시 Index 및 PreInput 상태 초기화


---

### 3. AnimNotify 기반 Combo Input Window 제어

- `UCAnimNotify_PreInput` 신규 구현

- 콤보 몽타주 상에서 입력 허용 구간(Input Window)을 명시적으로 정의

- `FlowType(Begin / End)` 기반 동작

  - `Begin` : `UCAction_ComboAttack::OnEnablePreInput()` 호출  
  - `End`   : `UCAction_ComboAttack::OffEnablePreInput()` 호출

- 이를 통해 콤보 구간 내 입력만 PreInput으로 인정되도록 구성


---

### 4. 기존 Action / AnimNotify 구조 보완

- `UCAction` / `UCAnimNotify_Action` 구조 일부 조정

- Action 확장(ComboAttack)에 대응할 수 있도록 인터페이스 정리

- Action 실행 책임을 Action 클래스 내부로 더 명확히 이관


---

## 테스트 방법

1. 프로젝트 실행 후 테스트 레벨 진입

2. Sword 무기 장착 상태 확인

3. Action 키 입력하여 1타 공격 정상 실행 확인

4. 콤보 테스트

   - 공격 몽타주 재생 중  
     `UCAnimNotify_PreInput(Begin ~ End)` 구간 내에서 Action 키 재입력

   - 다음 콤보 몽타주가 정상적으로 이어지는지 확인

5. Input Window 외 입력 테스트

   - 콤보 구간이 아닌 프레임에서 입력 시  
     콤보가 진행되지 않는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-combo-attack`

- 이슈: #14


---

## 노트

- **Input Window**
  - 입력을 의미 있게 받아들이는 시간 구간
  - AnimNotify로 열고 닫힘

- **PreInput Buffering**
  - 입력을 즉시 실행하지 않고 저장한 뒤,
    다음 콤보 실행 시점에 소비하는 구조

- 책임 분리

  - `CPlayerController / CPlayer`  
    → 입력 전달 및 의도 처리

  - `CWeaponComponent`  
    → Action 소유 및 실행 진입점

  - `UCAction_ComboAttack`  
    → 콤보 흐름, PreInput 처리, 단계 관리

  - `UCAnimNotify_PreInput`  
    → 애니메이션 타이밍 기반 Input Window 제어


---