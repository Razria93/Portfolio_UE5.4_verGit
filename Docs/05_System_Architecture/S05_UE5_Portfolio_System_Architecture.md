# AI 액션 이벤트 브리지 구조

## 1. 목적

본 문서는 AI가 Notify 이름, Montage 타이밍, Action 내부 구현 디테일에 직접 의존하지 않으면서도,  Action의 중요한 실행 타이밍을 참조할 수 있는 현재 구조를 정리함.

핵심 목표는 다음과 같음.

- Action 내부 타이밍 이벤트를 안정적인 이벤트 버스를 통해 외부에 노출함.
- AI combo도 기존 combat request 경로와 chain 실행 경로를 그대로 재사용함.
- Action 내부 구현이 BT와 AI Task로 새지 않도록 함.


---

## 2. 문제 정의

Player는 입력 재진입으로 타이밍 윈도우에 직접 반응할 수 있음.

```text
Player Input
-> Character API
-> ActionOrchestrator
-> ActionComponent
-> Action
```

반면 AI는 특정 프레임의 재입력을 흉내 내는 구조가 아님.

```text
Behavior Tree
-> 판단
-> 요청
-> 대기
-> 다음 판단
```

AI가 Notify 타이밍을 직접 소비하려 하면 다음 문제가 생김.

- BT가 Notify 이름이나 Montage 타이밍을 알아야 함.
- AI Task가 Action 내부 구현에 결합됨.
- Combo 타이밍 정책이 Action 바깥에 중복됨.
- Action 확장 시 BT 쪽 타이밍 처리 비용이 커짐.

현재 구조는 위 문제를 끊기 위해 다음을 분리함.

- 타이밍 소유
- 이벤트 브로드캐스트
- 상위 전투 가능 여부 판단
- combo chain follow-up


---

## 3. 설계 원칙

### 3.1 Action이 타이밍을 소유함

타이밍 윈도우는 Action 내부에 둠.

예:

- `UCAction_ComboAttack::OpenChainWindow()`
- `UCAction_ComboAttack::CloseChainWindow()`
- `UCAction::Begin()`
- `UCAction::Complete()`
- `UCAction::Abort()`

Notify는 Action 메서드만 호출함.  
AI 정책이나 BT 로직을 해석하지 않음.


---

### 3.2 ActionComponent는 안정적인 이벤트 버스임

개별 Action 인스턴스는 실행 단위 객체임.

`UCActionComponent`는 캐릭터에 안정적으로 붙어 있는 관측 지점임.

따라서 Action은 `UCActionComponent`를 통해 타이밍 및 lifecycle 이벤트를 외부에 노출함.

```text
Action
- 타이밍 의미를 소유함
- 이벤트를 발생시킴

ActionComponent
- 현재 action 실행 상태를 관리함
- action 이벤트를 브로드캐스트함
```


---

### 3.3 Combo chain follow-up은 기존 combat request 경로를 재사용함

현재 AI combo chain follow-up은 다음 순서를 따름.

```text
Action event 발생
-> Enemy가 OnActionEvent(...) 수신
-> Enemy가 같은 combat action을 다시 요청
-> Action이 Start / Chain / Reject 판단
```

즉 AI combo chaining도 Player combo chaining과 같은 실행 경로를 재사용함.


---

## 4. 전체 흐름

### 4.1 Player 전투 흐름

```text
Player Input
-> ACPlayer::HandleXXX()
-> UCActionOrchestratorComponent::RequestCombatAction()
-> UCActionComponent::ExecuteAction()
-> UCAction::DecideExecution()
-> Start / Chain / Reject
```

### 4.2 AI 첫 전투 시작 흐름

```text
Behavior Tree
-> ACEnemy::HandleAICombatAction()
-> UCActionOrchestratorComponent::RequestCombatAction()
-> UCActionComponent::ExecuteAction()
-> UCAction::DecideExecution()
-> Start / Chain / Reject
```

### 4.3 AI combo chain follow-up 흐름

```text
AnimNotify
-> UCAction_ComboAttack::OpenChainWindow()
-> UCAction::EmitActionEvent(...)

-> UCActionComponent::OnActionEvent.Broadcast(...)

-> ACEnemy::OnActionEvent(...)
-> ACEnemy::RequestChainCombatAction(...)
-> ACEnemy::HandleAICombatAction(...)

-> UCActionOrchestratorComponent::RequestCombatAction()
-> UCActionComponent::ExecuteAction()
-> UCAction::DecideExecution()

-> Chain
```


---

## 5. 책임 분리

### 5.1 Notify

Notify는 Action 메서드만 호출함.

```text
Notify knows:
- 어떤 Action 메서드를 호출할지

Notify does not know:
- Blackboard
- Behavior Tree
- AI 상태 정책
- combat follow-up 매핑 정책
```


---

### 5.2 Action

Action은 자신의 타이밍 의미를 앎.

예:

- chain window open
- chain window close
- action started
- action completed
- action aborted

Action은 이 시점들에서 `ActionComponent`를 통해 이벤트를 발생시킴.


---

### 5.3 ActionComponent

`UCActionComponent`는 두 역할을 가짐.

- 현재 action 실행 상태 관리
- 공식 action 이벤트 버스 제공

외부 시스템은 개별 Action 인스턴스가 아니라 이 컴포넌트를 통해 Action을 관측함.


---

### 5.4 Enemy / AI 계층

Enemy는 `OnActionEvent`를 구독함.

현재 책임은 다음과 같음.

- 현재 `EActionType`을 적절한 AI combat intent로 매핑함
- chain follow-up을 위해 기존 combat request 경로를 재진입시킴
- combo follow-up 로직이 Notify나 BT Task 내부에 퍼지지 않게 함

현재 예시는 다음과 같음.

```text
ChainWindowOpened
-> RequestChainCombatAction(...)
-> HandleAICombatAction(...)
```


---

### 5.5 Behavior Tree

BT는 combo timing window를 직접 처리하지 않음.

BT는 다음을 담당함.

- Engage 진입
- combat action 가능 여부 판단
- 첫 combat action 시작
- combat action 종료 대기
- 상위 브랜치 전환


---

## 6. 현재 이벤트 모델

현재 Action 이벤트 계층은 다음 수준으로 유지하는 것이 적절함.

```text
ChainWindowOpened
ChainWindowClosed
ActionStarted
ActionCompleted
ActionAborted
```

현재 코드 기준으로:

- `ChainWindowOpened`는 combo chain follow-up의 핵심 이벤트임.
- `ChainWindowClosed`는 타이밍 경계 이벤트임.
- `ActionStarted`, `ActionCompleted`, `ActionAborted`는 lifecycle 이벤트임.

중요한 점은 이 이벤트들을 모두 같은 방식으로 소비하지 않는다는 점임.

- combo follow-up은 `ChainWindowOpened`를 사용함
- lifecycle 이벤트는 디버그, 동기화, 향후 확장용으로 유지함


---

## 7. Combo 처리 원칙

### 7.1 Player Combo

Player combo는 기존 `Chain` 경로를 사용함.

```text
same action request
-> Action decides Chain
-> ApplyChain()
-> AdvanceCombo()
```


---

### 7.2 AI Combo

AI combo도 같은 실행 경로를 따름.

```text
Action emits chain window event
-> Enemy receives action event callback
-> Enemy requests the same combat action again
-> Action decides Chain
-> ApplyChain()
-> AdvanceCombo()
```

즉:

- AI는 Notify 타이밍을 직접 해석하지 않음
- AI는 별도 combo-chain Blackboard 프로토콜을 만들지 않음
- Player와 AI가 Action 내부의 같은 chain 실행 메커니즘을 공유함


---

## 8. 현재 구조의 핵심

현재 구조의 핵심은 다음과 같음.

- Notify는 Action 메서드만 호출함
- Action은 chain timing을 소유함
- ActionComponent는 Action event를 브로드캐스트함
- Enemy는 event를 받아 기존 combat request 경로를 다시 호출함
- 실제 chain 판정은 `UCAction_ComboAttack`이 처리함


---

## 9. 확장 지점

현재 구조는 직선형 combo chain을 기준으로 정리되어 있음.

향후 다음 요구가 추가되면 별도 확장이 필요함.

- combo branch
- enqueue / interrupt / cancel window
- Action / Reaction takeover coordination

예를 들어 combo branch가 생기면  
`ActionType + ActionIndex` 기준으로 follow-up 정책을 분기할 수 있음.


---

## 10. 결론

현재 구조의 핵심은 다음 한 문장으로 정리할 수 있음.

```text
AI combo는 Action event를 계기로 기존 combat request 경로를 다시 호출하고,
실제 chain 판정은 Player와 동일하게 Action 내부에서 처리함.
```

이 구조를 따르면 다음이 성립함.

- Notify는 Action만 앎
- Action은 타이밍을 소유함
- ActionComponent는 타이밍을 외부에 노출함
- Enemy는 chain timing을 기존 combat request 경로로 다시 연결함
