# UE5 Portfolio Pull Request

## 제목

**P15: Action 공용 실행 Pipeline 구성**

## 날짜

**2026.04.29**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/action-orchestration`

---

## 요약

### 작업 요약

본 PR은 Player와 AI가 서로 다른 절차로 처리하던 action 실행을 공통 action execution pipeline으로 정리한 작업이다.

### 이전 구조의 한계점

기존 구조에서는 Player와 AI가 같은 combat action을 사용하더라도 실행 진입점과 실행 절차가 달랐다.

Player는 input에서 `UCAction`으로 이어지는 흐름을 사용했고, AI는 BehaviorTree / Blackboard 내부에서 action 실행과 상태 관리를 별도로 구성하는 흐름에 가까웠음.

그 결과 같은 action이라도 Player와 AI가 서로 다른 lifecycle, montage, state update 기준을 가지게 되었고, combo chain이나 reaction takeover 같은 상황에서 유지보수 비용이 커졌다.

### 현재 구조의 보완사항

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Player / AI 실행 진입점 통일
- Player input과 AI combat intent를 모두 action request로 변환
- 실제 action 실행은 UCActionOrchestratorComponent로 진입

2. Intent와 lifecycle 분리
- Player input / AI BT는 실행 의도를 만드는 계층으로 정리
- UCActionComponent / UCAction은 공통 action lifecycle을 처리

3. Combo chain 경로 통일
- Player와 AI가 같은 combat request 경로를 통해 다음 combo를 요청
- chain timing은 notify event를 기준으로 연결

4. Runtime 안정성 보완
- reaction takeover 시 active action cleanup 추가
- request 실패 시 observable state change가 남지 않도록 rollback 기준 정리
```

---
## 변경 범위

### Action Execution Pipeline

#### A. Player / AI 실행 흐름 차이 정리

- Player와 AI가 같은 action을 서로 다른 절차로 실행하던 문제를 정리했다.

**Before**
```yaml
Player
-> Input
-> UCActionComponent / UCAction
-> State 변경
-> Montage 실행

AI
-> BehaviorTree / Blackboard
-> AI 상태 판단
-> 별도 combat action 실행 흐름
-> Montage 실행
```

**After**
```yaml
Player
-> Input
-> Action Request
-> UCActionOrchestratorComponent
-> UCActionComponent
-> UCAction

AI
-> BehaviorTree / Blackboard
-> Action Request
-> UCActionOrchestratorComponent
-> UCActionComponent
-> UCAction
```

**Result**
```yaml
공통화 기준
- Player와 AI는 request source만 다름
- action 실행 판단과 lifecycle은 공통 pipeline 사용
- montage 실행과 notify 처리는 UCAction이 담당
```

#### B. UCActionOrchestratorComponent 진입점 추가

- Player input과 AI combat intent를 action request로 변환한 뒤 `UCActionOrchestratorComponent`로 전달하도록 구성했다.

**Flow**
```yaml
Player Input / AI BT
-> Action Request
-> UCActionOrchestratorComponent
-> UCActionComponent
-> UCAction
```

**Structure**
```yaml
UCActionOrchestratorComponent
- RequestMovementAction  : movement intent 처리 진입점
- RequestEquipmentAction : equip / unequip intent 처리 진입점
- RequestCombatAction    : combat action intent 처리 진입점
- CanAcceptActionRequest : 공통 request gate
```

#### C. BT / Blackboard 역할 재정의

- BT / Blackboard를 `UCAction` 대체 구조가 아니라, AI intent와 execution context를 구성하는 계층으로 재정의했다.

**Before**
```yaml
BT / Blackboard
-> AI 상태 판단
-> action 실행 상태 일부 관리
-> combo / cooldown / combat state 직접 처리
```

**After**
```yaml
BT / Blackboard
-> AI 판단 context 구성
-> 실행할 action intent 선택
-> UCActionOrchestratorComponent에 request 전달
-> request result를 기준으로 cooldown / wait 처리
```

**Structure**
```yaml
AI Execution Responsibility
- BehaviorTree : AI 의사결정과 task 흐름 담당
- Blackboard   : AI 판단 context와 request 결과 관찰값 저장
- Orchestrator : action request 해석과 실행 진입 담당
- UCAction      : 실제 montage 기반 action lifecycle 담당
```

#### D. Combat Start / Cooldown Ownership 정리

- combat action이 실제로 시작된 경우에만 cooldown을 commit하도록 정리했다.

**Flow**
```yaml
UCBTTask_StartCombatAction
-> combat action request
-> request result 확인
-> Started일 때만 NextCombatActionTime commit
```

**Rule**
```yaml
Request Result
- Started : cooldown commit 허용
- Rejected / Ignored : cooldown commit 금지
```

#### E. Player / AI Combo Chain 경로 통일

- AI가 BT 내부에서 combo를 직접 이어가지 않고, Player와 같은 combat request 경로를 다시 사용하도록 정리했다.

**Flow**
```yaml
ComboAttack
-> ChainWindowOpened notify event
-> Player input 또는 AI follow-up request
-> RequestCombatAction
-> UCAction_ComboAttack
-> 다음 combo 단계 실행
```

**Structure**
```yaml
Combo Chain
- ChainWindowOpened : 다음 combo 요청이 가능한 timing event
- Combat Request    : Player / AI 공통 follow-up 요청 경로
- UCAction_ComboAttack : 실제 chain 가능 여부와 다음 action 처리
```

#### F. Action Event Bridge 구성

- action execution event를 AI가 후속 판단에 사용할 수 있도록 `Action -> UCActionComponent -> Enemy` callback 경로를 구성했다.

**Flow**
```yaml
UCAction
-> UCActionComponent
-> Enemy callback
-> AI follow-up 판단
```

**Structure**
```yaml
Action Event Bridge
- OnActionEvent      : action event 수신 지점
- ChainWindowOpened : combo follow-up request 허용 event
- Enemy callback    : AI behavior와 action event 연결
```

#### G. Failure Handling / Rollback 기준 정리

- action request 실패와 성공 결과에 따라 state commit 여부를 분리했다.

**Rule**
```yaml
Reject / Ignore
- observable state change를 남기지 않음

Started / Chained
- 실제 실행이 확정된 결과에서만 state commit 허용

Abort / Complete
- action lifecycle endpoint에서 cleanup 수행
```

---
## 안정성 보완

### Weapon Blueprint Parent Class Load 복구 (B06 보완)

#### A. C++ class rename 이후 Blueprint reference 복구

- C++ class rename으로 기존 Weapon Blueprint의 parent class path가 끊기는 문제를 복구했다.
- 자세한 재현 조건과 원인은 `B06` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
ACAttachment rename
-> ACWeaponActor
-> 기존 Blueprint parent class path 단절
-> CoreRedirects 추가
-> 기존 Weapon Blueprint load 복구
```

### AI ComboAttack Chain 안정화 (B07 보완)

#### A. AI follow-up combo request 경로 보정

- AI가 BT 내부에서 combo를 직접 이어가지 않고, Player와 같은 combat request 경로로 다음 combo를 요청하도록 정리했다.
- 자세한 재현 조건과 원인은 `B07` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
UCAction_ComboAttack
-> ChainWindowOpened notify event
-> Enemy callback
-> AI follow-up 판단
-> RequestCombatAction 재호출
-> 다음 ComboAttack 단계 실행
```

**Rule**
```yaml
AI ComboAttack Chain
- BT가 montage를 직접 이어서 재생하지 않음
- ChainWindowOpened 이후 combat request를 다시 발행
- UCAction_ComboAttack이 chain 가능 여부와 다음 단계 실행을 판단
```

### Reaction Takeover 이후 상태 복구 안정화 (B08 보완)

#### A. Reaction 진입 전 active action cleanup

- active action 중 reaction이 시작될 때 action context와 state가 남지 않도록 cleanup 경로를 보완했다.
- 자세한 재현 조건과 원인은 `B08` Bug Report에서 분리하여 정리했다.

**Flow**
```yaml
ComboAttack 실행 중 피격
-> active action abort
-> Reaction 실행
-> action / state / blackboard 기준 정리
-> combat flow 복구
```

**Rule**
```yaml
Reaction Takeover
- reaction 진입 전 active action cleanup 수행
- reaction 상태를 combat availability 계산에 반영
- reaction 종료 이후 action context / state / blackboard 기준 재정렬
```

---
## 주요 Pipeline

### Player CombatAction Request Pipeline

```yaml
Player Input (CombatAction)
-> Action Request
-> UCActionOrchestratorComponent
-> UCActionComponent
-> UCAction
-> Montage / Notify
```

### AI CombatAction Request Pipeline

```yaml
BehaviorTree / Blackboard
-> intent 선택 (CombatAction)
-> Action Request
-> UCActionOrchestratorComponent
-> UCActionComponent
-> UCAction
-> Montage / Notify
```

### Combo Chain Pipeline

```yaml
ChainWindowOpened
-> Player input 또는 AI follow-up request
-> RequestCombatAction
-> UCAction_ComboAttack
-> 다음 combo 단계 실행
```

### Reaction Takeover Pipeline

```yaml
Active Action
-> Damage / Reaction 발생
-> Active Action abort
-> Reaction 실행
-> Action / State / Blackboard cleanup
```

---
## 테스트 방법

### Action 실행

- Player 입력이 `UCActionOrchestratorComponent -> UCActionComponent -> UCAction` 경로로 실행되는지 확인
- AI combat task가 직접 montage를 실행하지 않고 action request를 통해 combat action을 실행하는지 확인
- action request 실패 시 observable state change가 남지 않는지 확인

### Combo Chain

- Player `ComboAttack`이 기존 chain window와 action index 기준으로 이어지는지 확인
- AI `ComboAttack`이 1타 이후 같은 combat request 경로로 다음 combo 단계에 진입하는지 확인
- 마지막 combo 단계에서 불필요한 chain request가 발생하지 않는지 확인

### Reaction Takeover

- combo action 도중 피격 시 active action이 정리된 뒤 reaction이 실행되는지 확인
- reaction 종료 이후 combat availability와 blackboard state가 정상 복구되는지 확인

### Asset / Build

- C++ class rename 이후 기존 Weapon Blueprint가 정상 load되는지 확인
- `PortfolioEditor Win64 Development` 빌드가 성공하는지 확인

---
## 검증 결과

- Player combat action 실행 경로 정상 동작 확인
- AI combat action request 경로 정상 동작 확인
- Player / AI combo chain 경로 통일 확인
- reaction takeover 이후 combat flow 복구 확인
- Blueprint parent class load 문제 복구 확인

---
## 관련 문서

- Issue Checklist: `D16_UE5_Portfolio_Issue_Checklist.md`

- Bug Report:
	- `B06_UE5_Portfolio_Bug_Report.md`
	- `B07_UE5_Portfolio_Bug_Report.md`
	- `B08_UE5_Portfolio_Bug_Report.md`

---
## 정리

이 PR의 핵심은 Player와 AI가 서로 다른 방식으로 실행하던 action flow를 공통 action execution pipeline으로 정리한 것이다.

변경 후에는 Player input과 AI BehaviorTree가 모두 action request를 생성하고, 실제 실행 판단과 montage lifecycle은 `UCActionOrchestratorComponent -> UCActionComponent -> UCAction` 경로에서 처리된다.

이를 통해 BT / Blackboard는 `UCAction`을 대체하는 구조가 아니라 AI intent와 context를 만드는 계층으로 정리되었고, Player와 AI가 같은 combat action, combo chain, runtime cleanup 기준을 공유할 수 있게 됐다.

또한 reaction takeover와 failure handling 기준을 함께 정리하여, action 실행 중 다른 execution으로 전환되거나 request가 실패하는 상황에서도 state와 runtime context가 어긋나지 않도록 최소 안전 구조를 추가했다.

---
