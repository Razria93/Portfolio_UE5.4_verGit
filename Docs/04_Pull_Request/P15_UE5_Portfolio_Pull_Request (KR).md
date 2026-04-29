# Action Orchestration 구조 정리 및 AI Combo / Reaction 연동 보강

## 제목

`♻️ refactor: action orchestration 구조 정리 및 ai combo / reaction 연동 보강`

## 요약

- 본 PR에서는 **Action Orchestration 구조를 기준으로 Player / AI 공통 실행 흐름을 정리**하였고, **AI Combo chain**과 **Reaction takeover**가 기존 액션 실행 흐름 안에서 동작하도록 보강하였음.

- 핵심 방향은 다음과 같음.

	- `Intent -> Request -> Resolve -> Execute` 흐름을 명시적으로 정리함.
	
	- Player와 AI가 같은 **combat request 경로**와 같은 **combo chain 실행 경로**를 재사용하도록 맞춤.
	
	- combat blackboard는 chain window 자체가 아니라 **combat availability / lifecycle state** 중심으로 정리함.
	
	- action 실행 실패 시의 **failure-handling / rollback policy**를 문서화하고 현재 실행 구조와 맞추어 정리함.

- 또한 프로젝트 진행중에 발생한 다음 두 문제를 함께 수정하였음.

	- C++ 클래스 rename 이후 기존 Weapon Blueprint 부모 클래스 로드가 실패하던 문제

	- AI가 `ComboAttack`에서 1타만 반복하고 다음 콤보 단계로 chain되지 않던 문제
	
	- combo action 도중 피격 시 Reaction 이후 action / state / blackboard가 어긋나 전투 흐름이 고장나던 문제


---

## 완료 작업

### 1. AI combat intent / blackboard 구조 정리

- AI 전투 판단 흐름을 **intent-driven state** 기준으로 정리함

- AI 전투 blackboard key를 **CombatAction** 중심으로 정리함

- 기존의 `AIState`를 `AIIntentState` 라고 명시하고 관련 명칭 및 구조를 정리함
  
### 2. combat start / cooldown ownership 정리

- 기존의 cooldown commit task를 제거하고, combat action이 실제로 `Started` 된 시점에만 cooldown이 commit 되도록 정리함

- `StartCombatAction`은 다음 역할만 담당하도록 정리함

	- combat action 시작 요청
	
	- 성공 여부 확인
	
	- 성공 시 `NextCombatActionTime` commit

### 3. action event bridge 구성

- `Action -> ActionComponent -> Enemy callback` 경로를 명시적으로 구성함

- 해당 `callback`을 기반으로 AI는 추가적인 행동을 구성할 수 있음

- AI가 `OnActionEvent(...)` 를 통해 후속  **combo chain**을 받을 수 있도록 정리함

### 4. Player / AI combo chain 실행 흐름 통일

- **combo chain** 의미를 `ChainWindow` 기준으로 정리함

- AI의 combo chain이 기존 combat request 경로를 다시 호출하도록 구성함

- 실제 chain 판정은 기존과 동일하게 `UCAction_ComboAttack` 내부에서 처리하도록 유지함

- 결과적으로 Player와 AI가 같은 combo chain 실행 흐름을 재사용하도록 정리함

### 5. Reaction takeover 안전성 보강

- reaction 시작 시 현재 active action이 존재하면 먼저 abort 하도록 구성함

- reaction 상태를 combat availability 계산에 반영함

- combo action 도중 피격 후에도 reaction 이후 전투 흐름이 다시 이어질 수 있도록 최소 안전 구조를 추가함

### 6. failure-handling / rollback policy 정리

- action orchestration 구조에서 공통으로 적용할 실패 처리 원칙을 문서화함

- 핵심 정책은 다음과 같음

	- `Reject / Ignore`에서는 observable state change를 남기지 않음
	
	- `Start / Chain / Enqueue / Interrupt` 결과에서만 commit을 허용함
	
	- cleanup 책임은 `Abort()` / `Complete()` 같은 action lifecycle endpoint에 둠
	  
	- blackboard 상태값은 실제로 시작/종료된 신호나 계산된 조건을 기준으로 갱신함

### 7. Blueprint parent class migration 안전성 보강

- `ACAttachment`를 `ACWeaponActor`로 rename한 이후 기존 Weapon Blueprint 부모 클래스 로드가 실패하던 문제를 수정함

- 기존 Blueprint가 저장하고 있던 부모 클래스 경로를 새 C++ 부모 클래스로 정상 연결할 수 있도록 `CoreRedirects` class redirect를 추가함

- 에디터 재시작 및 Blueprint 재로드 기준으로 기존 Weapon Blueprint가 정상 복구되는 것을 확인함

### 8. 문서 및 버그 리포트 정리

- `S03`: Action Orchestration State Model 설계

- `S04`: Action Orchestration 구현 계획

- `S05`: AI Action Event Bridge / combo chain 구조

- `B06`: C++ 클래스 rename 이후 기존 Weapon Blueprint 부모 클래스 로드 실패 버그 리포트

- `B07`: AI combo가 1타만 반복되던 문제 버그 리포트

- `B08`: reaction takeover 이후 combat flow가 고장나던 문제 버그 리포트


---

## 테스트 방법

1. Player 입력이 `Orchestrator -> ActionComponent -> Action` 경로로 정상 실행되는지 확인

2. combat action 시작 성공 시에만 `NextCombatActionTime`이 갱신되는지 확인

3. Player `ComboAttack`이 기존과 동일하게 chain되는지 확인

4. AI `ComboAttack`이 1타 이후 다음 combo 단계로 정상 chain되는지 확인

5. 마지막 combo 단계에서 불필요한 chain window가 열리지 않는지 확인

6. combo action 도중 피격 시 reaction takeover 이후 combat flow가 복구되는지 확인

7. action request 실패 시 reject / ignore 경로에서 observable state change가 남지 않는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/action-orchestration`

- 관련 작업:

  - `M05-01: Action Orchestration 구조 정리 및 AI Combo / Reaction 연동 보강`

- 관련 버그 리포트:

  - `B06_UE5_Portfolio_Bug_Report (KR/EN)`

  - `B07_UE5_Portfolio_Bug_Report (KR/EN)`

  - `B08_UE5_Portfolio_Bug_Report (KR/EN)`


---
