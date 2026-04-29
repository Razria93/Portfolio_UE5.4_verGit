# UE5 Portfolio Bug Report (KR)

## 제목

**M05-B02: AI ComboAttack이 다음 콤보 단계로 체인되지 않고 1타만 반복 실행됨**

### Date

- **2026.04.29**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/action-orchestration`


---

## 요약

- AI가 `ComboAttack`을 시작한 뒤 다음 콤보 단계로 이어지지 못하고 1타만 반복함.

- Player는 기존 `Chain` 경로를 통해 정상적으로 다음 콤보 단계로 진행할 수 있었지만, AI는 같은 흐름을 재사용하지 못해 combo chain이 끊겼음.

- 이를 해결하기 위해 Notify / Action / ActionComponent / Enemy 사이에 action event callback 경로를 구성하고, AI도 동일한 combat request를 다시 호출하여 `Chain` 판정을 받도록 수정함.

- 또한 마지막 콤보 단계에서는 chain window를 열지 않도록 정리하여 불필요한 follow-up 요청을 차단함.


---

## 환경

- Engine: Unreal Engine 5.4

- Branch:

  - `feature/action-orchestration`

- Related Code:

  - `Source/Portfolio/Action/CAction_ComboAttack.cpp`
  - `Source/Portfolio/Action/CAction_ComboAttack.h`
  - `Source/Portfolio/Action/CAction.cpp`
  - `Source/Portfolio/Action/CAction.h`
  - `Source/Portfolio/Component/CActionComponent.cpp`
  - `Source/Portfolio/Component/CActionComponent.h`
  - `Source/Portfolio/Character/Enemy/CEnemy.cpp`
  - `Source/Portfolio/Character/Enemy/CEnemy.h`
  - `Source/Portfolio/Notify/CAnimNotify_ChainWindow.cpp`
  - `Source/Portfolio/Notify/CAnimNotify_ChainWindow.h`

- Related Asset:

  - `Content/02_Controller/02_Enemy/AI/BehaviorTree/State/Sub/SBT_Attack.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_0.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_1.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_2.uasset`


---

## 재현 방법

1. AI가 전투 상태에서 `ComboAttack`을 시작하도록 함.

2. 1타 montage가 재생되는 동안 다음 콤보 입력이 들어가야 하는 timing window까지 진행함.

3. AI가 다음 콤보 단계로 이어지는지 확인함.


---

## 기대 결과

- AI도 Player와 동일하게 기존 `ComboAttack` 흐름 안에서 `Chain` 판정을 받아야 함.

- chain window 시점에 동일 combat request를 다시 넣고, `ApplyChain()`과 `AdvanceCombo()`를 통해 다음 콤보 단계로 진행해야 함.

- 결과적으로 1타, 2타, 3타가 하나의 combo flow로 이어져야 함.


---

## 실제 결과

- AI는 1타를 시작할 수는 있었지만, 다음 콤보 단계로 이어지지 못했음.

- chain timing 이후에도 다음 단계로 진행되지 않고, 다시 1타만 시작하는 형태로 보였음.

- 결과적으로 AI combo가 직선형 chain이 아니라 1타 반복처럼 동작했음.


---

## 원인

- AI combo continuation이 기존 Action 내부의 `Chain` 경로를 재사용하지 못했음.

- Player는 같은 action request를 다시 넣어 `Chain` 판정을 받는 구조였지만, AI는 Notify timing을 동일한 실행 경로로 되돌려 연결하지 못했음.

- 또한 `PreInput` 개념이 남아 있어 Player combo와 AI combo의 의미가 완전히 일치하지 않았음.


---

## 수정

다음과 같이 구조를 정리함.

```text
AnimNotify
-> UCAction_ComboAttack::OpenChainWindow()
-> UCAction::EmitActionEvent(...)
-> UCActionComponent::OnActionEvent.Broadcast(...)
-> ACEnemy::OnActionEvent(...)
-> ACEnemy::RequestChainCombatAction(...)
-> ACEnemy::HandleAICombatAction(...)
-> UCAction::DecideExecution()
-> Chain
```

- `PreInput` 용어를 `ChainWindow` 중심으로 정리함.

- AI가 `ChainWindowOpened` 이벤트를 받으면 기존 `HandleAICombatAction()` 경로로 동일 combat request를 다시 호출하도록 수정함.

- 실제 chain 판정은 `UCAction_ComboAttack::DecideExecution()`이 계속 담당하도록 유지함.

- 마지막 콤보 단계에서는 `CanAdvanceCombo()`를 통해 `OpenChainWindow()` 자체를 열지 않도록 수정함.


---

## 검증 결과

- AI가 `ComboAttack` 시작 후 2타, 3타로 정상적으로 이어지는 것을 확인함.

- Player와 AI가 모두 같은 `Chain` 실행 경로를 사용함을 확인함.

- 마지막 콤보 단계에서 불필요한 chain follow-up 요청이 발생하지 않음을 확인함.


---

## 후속 정리

- 향후 combo branch가 추가되면 `ActionType + ActionIndex` 기준으로 follow-up 정책을 분기할 수 있음.

- 현재 구조는 직선형 combo chain 기준으로 충분하며, branch combo / enqueue / interrupt는 별도 확장 포인트로 남겨둠.


---

## Notes

- 이 수정의 핵심은 AI 전용 combo 시스템을 따로 만드는 것이 아니라, Player와 동일한 Action-driven chain 경로를 재사용하도록 정리한 것임.

- Notify는 Action 메서드만 호출하고, Action은 ActionComponent를 통해 event를 노출하며, Enemy는 기존 combat request 경로를 다시 호출하는 역할만 담당함.


---
