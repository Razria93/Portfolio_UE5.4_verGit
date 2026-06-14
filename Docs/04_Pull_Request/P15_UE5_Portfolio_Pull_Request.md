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

이번 PR에서는 **Player와 AI가 같은 공격 행동을 서로 다른 절차로 실행하던 문제를 줄이기 위해, 행동 요청과 실행 진입점을 공통 흐름으로 연결했다.**

이를 통해 Player 입력과 AI 판단은 서로 다른 곳에서 시작하더라도, 행동 실행과 연속 공격 처리, 실패 시 상태 정리가 같은 흐름에서 처리되도록 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Action Request 표준화**: Player 입력과 AI 판단 결과라는 서로 다른 입력 값을 같은 행동 요청 형식으로 변환하고, 공통 실행 진입점으로 전달하도록 구성했다.

- **AI Combat Action 실행 연결**: AI가 BehaviorTree 안에서 전투 행동을 직접 실행하지 않고, Player와 같은 Action Request 흐름을 통해 전투 행동을 실행하도록 연결했다.

- **AI Combo Chain 경로 통일**: AI가 연속 공격을 직접 이어 붙이지 않고, action event 이후 같은 combat request 경로를 다시 호출해 다음 공격 단계로 이어질 수 있도록 구성했다.

### Refactoring

- **실행 의도와 실행 책임 분리**: Player 입력과 AI 판단은 실행 의도를 만들고, 실제 행동 실행과 애니메이션 재생은 공통 action 실행 흐름에서 처리하도록 책임을 나눴다.

- **AI 실행 책임 축소**: AI 판단 계층은 실행할 행동을 고르고 결과를 관찰하는 역할로 줄이고, 실제 행동 실행 판단은 공통 실행 계층으로 넘겼다.

- **상태 확정 기준 정리**: 행동 요청이 실제로 시작된 경우에만 재사용 대기 시간과 실행 상태를 확정하도록 정리했다.

### Troubleshooting

- **무기 Blueprint load 복구(B06)**: C++ class rename 이후 기존 weapon blueprint가 부모 class를 찾지 못하는 문제를 CoreRedirect 기준으로 복구했다.

- **AI 연속 공격 보정(B07)**: AI가 연속 공격을 직접 이어 붙이지 않고, 행동 이벤트를 받은 뒤 같은 전투 행동 요청 경로로 다음 공격을 요청하도록 정리했다.

- **피격 전환 안정화(B08)**: 행동 도중 피격 반응이 시작될 때 기존 행동 상태와 context가 남지 않도록 정리 기준을 보강했다.

---

## 핵심 개념

이 섹션은 KR 설명만으로 의미가 흐려지는 핵심 식별자를 제한적으로 소개한다.

이 PR의 핵심 흐름은 Player 입력과 AI 판단 결과를 Action Request로 표준화한 뒤, Action Orchestrator를 거쳐 Action Component와 Action으로 실행하는 구조다.

```text
Action Request(행동 요청)
-> Player 입력 또는 AI 판단 결과를 실제 action 실행 요청으로 바꾼 공통 요청 단위
-> 이 PR에서는 Player와 AI가 같은 실행 진입점으로 들어가기 위한 공통 요청 단위로 사용됨
```

```text
Action Orchestrator(행동 요청 조율자)
-> action request를 받아 실행 가능한 요청인지 확인하고 실행 경로로 넘기는 중간 계층
-> 공통 Request Gate, request 해석, Request Result 반환을 담당함
-> 코드에서는 `UCActionOrchestratorComponent`가 이 역할을 담당함
```

```text
Action Component(행동 실행 상태 관리자)
-> 현재 action 상태를 관리하고 실제 action 객체를 실행하는 component
-> 코드에서는 `UCActionComponent`가 이 역할을 담당함
```

```text
Action(행동 실행 객체)
-> 공격 같은 하나의 행동을 실제로 재생하고 종료시키는 실행 객체
-> montage 재생, notify 처리, combo chain, action 종료를 담당함
-> 코드에서는 `UCAction`과 파생 action class가 이 역할을 담당함
```

```text
Request Result(요청 처리 결과)
-> action request를 받은 뒤 실행이 시작됐는지, 예약됐는지, 거절됐는지 나타내는 결과값
-> 이 PR에서는 Started / Chained / Rejected / Ignored 결과에 따라 cooldown commit과 combo 후속 처리를 구분함
-> Chained는 즉시 새 montage를 시작한 결과가 아니라, chain window에서 소비할 후속 combo 요청이 accepted 된 결과를 의미함
```

```text
Combo Chain(연속 공격 연결)
-> 공격 중 특정 타이밍에 다음 공격 요청을 받아 이어지는 연속 공격 흐름
-> Player와 AI가 같은 combat request 경로를 재사용하도록 정리한 대상
```

```text
Reaction Takeover(피격 전환)
-> 공격 실행 중 피격이 우선권을 가져 기존 action을 중단하고 reaction으로 넘어가는 상황
-> 이 PR에서는 reaction 진입 전 active action cleanup이 필요한 안정성 보완 범위로 다룸
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 구조의 문제를 정리한다.

### Player / AI action 실행 비대칭 해소 필요성

기존 구조에서는 Player와 AI가 같은 combat action을 사용하더라도 실행 진입점과 실행 절차가 달랐다.

Player는 입력에서 action 실행으로 이어졌고, AI는 BehaviorTree / Blackboard 안에서 전투 행동 실행과 상태 관리를 별도로 처리하는 흐름에 가까웠다.

그 결과 같은 action이라도 Player와 AI가 서로 다른 lifecycle, montage, 상태 갱신 기준을 가질 수 있었다.

### AI와 action 실행 책임 분리 필요성

AI는 어떤 행동을 할지 판단해야 하지만, 실제 action 실행 생명주기까지 직접 소유하면 BehaviorTree / Blackboard가 action system의 실행 책임까지 떠안는 구조가 될 수 있었다.

따라서 AI는 실행 의도를 만들고 결과를 관찰하는 역할로 줄이고, 실제 action 실행은 공통 실행 경로로 넘길 필요가 있었다.

### Combo chain 안정화 필요성

Combo chain은 Player와 AI 모두 같은 연계 가능 타이밍과 action 판단 기준을 사용해야 했다.

AI가 BehaviorTree 내부에서 combo를 직접 이어 붙이면 Player와 다른 연속 공격 기준을 갖게 되므로, 같은 combat request 경로를 다시 호출하도록 정리할 필요가 있었다.

### Failure handling과 runtime cleanup 안정화 필요성

action 도중 reaction이 들어오거나 request가 실패했을 때 이전 action state와 runtime context가 남으면 이후 전투 흐름이 어긋날 수 있었다.

또한 action event를 AI 후속 판단으로 전달할 경로가 없으면 AI combo follow-up도 Player와 같은 request 경로를 재사용하기 어려웠다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. Player / AI 공통 Action Request 진입 구조 구성

- **왜**:
  Player와 AI가 같은 action을 서로 다른 절차로 실행하면 montage 실행, notify 처리, 상태 갱신 기준이 서로 달라질 수 있었다.

- **어떻게**:
  Player 입력과 AI combat intent를 Action Request(행동 요청)로 변환하고, `UCActionOrchestratorComponent`(Action Orchestrator)로 전달하도록 정리했다.

- **결과**:
  Player와 AI는 요청 source만 다르고, 실제 action 실행은 같은 공통 진입 경로를 사용한다.

### 2. Action 실행 책임 분리

- **왜**:
  AI task나 Player 입력이 실제 action lifecycle까지 직접 처리하면 입력 / 판단 / 실행 책임이 섞인다.

- **어떻게**:
  Player 입력과 BehaviorTree는 실행 의도를 만들고, `UCActionOrchestratorComponent`, `UCActionComponent`(Action Component), `UCAction`(Action)이 request 해석, 상태 소유, montage lifecycle을 나눠 처리하도록 구성했다.

- **결과**:
  action 실행은 `Action Request -> Action Orchestrator -> Action Component -> Action` 순서로 읽히며, Player와 AI가 같은 실행 생명주기 기준을 공유한다.

### 3. BehaviorTree / Blackboard 역할 축소

- **왜**:
  BehaviorTree / Blackboard가 action 실행 상태와 combo / cooldown을 직접 관리하면 AI 판단 계층과 action 실행 계층의 경계가 흐려진다.

- **어떻게**:
  BehaviorTree는 AI 의사결정과 task 흐름을 담당하고, Blackboard는 `bCanCombatAction`, `bIsCombatAction`, `NextCombatActionTime` 같은 판단 context와 실행 결과를 다음 판단에 반영할 값을 저장하도록 정리했다. 실제 action 실행은 Action Request를 통해 공통 pipeline으로 넘겼다.

- **결과**:
  AI는 action 실행을 직접 소유하지 않고, 실행 의도 생성과 결과 관찰에 집중한다.

### 4. Cooldown Commit 기준 정리

- **왜**:
  combat action request가 거절되거나 무시됐는데 cooldown이 먼저 확정되면, 실제로 공격하지 않았는데 다음 공격 가능 시간이 밀릴 수 있었다.

- **어떻게**:
  Request Result(요청 처리 결과)가 Started일 때만 `NextCombatActionTime`(다음 전투 행동 가능 시간)을 commit하도록 정리했다. Combo follow-up request는 Chained 결과로 남기고, 실제 chain 실행은 chain window consume 시점에 처리되도록 구분했다.

- **결과**:
  request 실패는 관찰 가능한 상태 변경을 남기지 않고, 실제 action 시작이 확정된 경우에만 cooldown이 반영된다.

### 5. Player / AI Combo Chain 경로 통일

- **왜**:
  AI가 BehaviorTree 안에서 combo를 직접 이어가면 Player와 다른 chain 판단 기준을 사용하게 된다.

- **어떻게**:
  `ChainWindowOpened`(연계 가능 구간 열림) event 이후 Player 입력 또는 AI follow-up request가 같은 `RequestCombatAction` 경로를 다시 호출하도록 정리했다.

- **결과**:
  Player와 AI는 같은 combo window, 같은 combat request, 같은 `UCAction_ComboAttack` chain 판단 기준을 사용한다. 다만 AI combo branch 확장은 후속 확장 지점으로 남긴다.

### 6. Action event bridge 구성

- **왜**:
  AI가 action 실행 중 발생한 event를 후속 판단에 사용하려면 action system과 enemy AI 사이의 callback 경로가 필요했다.

- **어떻게**:
  action event를 `UCAction -> UCActionComponent -> Enemy callback` 경로로 전달하도록 구성했다.

- **결과**:
  AI는 action 내부 event를 직접 소유하지 않고, 노출된 event를 받아 다음 combat request 여부를 판단할 수 있다.

### 7. Failure handling과 rollback 기준 정리

- **왜**:
  action request 실패와 실행 확정 결과가 같은 방식으로 처리되면 state와 runtime context가 실제 실행 결과와 어긋날 수 있었다.

- **어떻게**:
  Request Result가 Rejected / Ignored일 때는 관찰 가능한 state change를 남기지 않고, Started / Chained처럼 실행 시작 또는 후속 combo 요청이 accepted 된 결과에서만 후속 처리를 허용하도록 정리했다.

- **결과**:
  실패한 action request는 다음 판단에 영향을 남기지 않고, 실행된 action은 lifecycle endpoint에서 cleanup된다.

### 8. 안정성 보완 범위 반영

- **왜**:
  action orchestration 도입 과정에서 AI combo chain, reaction takeover 안정성 문제가 드러났고, 같은 PR 범위에서 기존 weapon blueprint reference 호환성 문제도 함께 복구해야 했다.

- **어떻게**:
  B06은 `ACAttachment -> ACWeaponActor` class rename 이후 끊긴 weapon blueprint parent class load를 CoreRedirect로 복구했고, B07은 AI combo를 action event 기반 follow-up request로 보정했으며, B08은 reaction 진입 전 active action cleanup 기준을 보강했다.

- **결과**:
  action 공통 실행 흐름 안에서 AI combo continuation과 reaction takeover 이후 최소 상태 복구 기준이 안정화됐고, weapon blueprint load는 호환성 복구 범위로 함께 정리됐다.

---

## 주요 처리 흐름

이 섹션은 주요 실행 순서와 분기 기준을 코드 구현 전에 흐름으로 먼저 설명한다.

### 공통 Action Request 흐름

```text
Player 입력 또는 AI 판단
-> Action Request 생성
-> Action Orchestrator 진입
-> 공통 Request Gate 확인
-> Action Component로 전달
-> Action 실행
-> montage / notify 처리
```

이 흐름은 Player와 AI가 서로 다른 source에서 출발하더라도 실제 action 실행은 같은 combat action 실행 경로로 들어가는 과정을 의미한다.

### AI Combat Action 흐름

```text
BehaviorTree 판단
-> Blackboard context 확인
-> 실행할 combat action 선택
-> Action Request 생성
-> Action Orchestrator 진입
-> Request Result 확인
   - Started -> cooldown commit
   - Rejected / Ignored -> cooldown commit 안 함
```

이 흐름은 AI가 직접 montage를 실행하지 않고, combat action 요청 결과에 따라 cooldown과 후속 판단을 처리하는 과정을 의미한다.

### Combo Chain 흐름

```text
ComboAttack 실행
-> ChainWindowOpened event
-> Player 입력 또는 AI follow-up request
-> RequestCombatAction 재호출
-> Request Result 확인
   - Rejected / Ignored -> 현재 combo 종료
   - Chained            -> 후속 combo 요청 accepted 후 chain window consume 대기
-> UCAction_ComboAttack chain 가능 여부 판단
   - No  -> 현재 combo 종료
   - Yes -> 다음 combo 단계 실행
```

이 흐름은 Player와 AI가 같은 combo timing event와 combat request 경로를 통해 다음 combo 단계로 이어지는 과정을 의미한다. AI combo branch는 후속 확장 지점으로 남는다.

### Reaction Takeover Cleanup 흐름

```text
Active Action 실행 중
-> Damage / Reaction 발생
-> Active Action abort
-> action context / state cleanup
-> Reaction 실행
-> reaction 종료 후 combat 판단 재개
```

이 흐름은 action 도중 reaction이 들어왔을 때 기존 action 상태가 남지 않도록 정리한 뒤 reaction으로 전환하고, 이후 전투 판단이 다시 이어질 수 있게 하는 최소 복구 과정을 의미한다.

---

## 구현 결과

이 섹션은 변경 이후 시스템이 어떤 동작으로 정리됐는지 요약한다.

- Player 입력과 AI BehaviorTree 판단은 모두 Action Request를 통해 공통 combat action 실행 경로로 진입한다. 다만 AI combo branch 확장은 후속 확장 지점으로 남는다.

- 실제 action 실행은 `UCActionOrchestratorComponent -> UCActionComponent -> UCAction` 순서로 처리된다.

- BehaviorTree / Blackboard는 action 실행을 직접 대체하지 않고, AI 판단 context와 `bCanCombatAction`, `bIsCombatAction`, `NextCombatActionTime` 같은 실행 결과 반영값을 관리한다.

- 일반 combat action cooldown은 실제 action 시작이 Started로 확정된 경우에만 commit된다.

- Player와 AI combo chain은 같은 chain window와 combat request 경로를 사용하되, AI combo branch 확장은 후속 확장 지점으로 남는다.

- Reaction takeover, request failure, weapon blueprint parent class reference 문제는 별도 Bug Report 범위와 연결해 안정성 보완 기준을 정리했다.

---

## 테스트 방법

### Action 실행

- Player 입력이 `UCActionOrchestratorComponent -> UCActionComponent -> UCAction` 경로로 실행되는지 확인했다.

- AI combat task가 직접 montage를 실행하지 않고 Action Request를 통해 combat action을 실행하는지 확인했다.

- action request 실패 시 관찰 가능한 state change가 남지 않는지 확인했다.

- combat action request가 Started일 때만 `NextCombatActionTime`이 갱신되는지 확인했다.

### Combo chain

- Player ComboAttack이 기존 chain window와 action index 기준으로 이어지는지 확인했다.

- AI ComboAttack이 1타 이후 같은 combat request 경로로 다음 combo 단계 예약과 실행에 진입하는지 확인했다.

- AI follow-up request가 Chained 결과로 accepted 되고, chain window consume 시점에 다음 combo 단계로 이어지는지 확인했다.

- 마지막 combo 단계에서 불필요한 chain request가 발생하지 않는지 확인했다.

### Reaction takeover

- combo action 도중 피격 시 active action이 정리된 뒤 reaction이 실행되는지 확인했다.

- reaction 종료 이후 combat availability와 blackboard state가 다음 판단에 사용할 수 있는 상태로 정리되는지 확인했다.

### Asset / build

- C++ class rename 이후 기존 weapon blueprint가 정상 load되는지 확인했다.

- `PortfolioEditor Win64 Development` 빌드가 성공하는지 확인했다.

---

## 검증 결과

- Player combat action 실행 경로가 공통 action pipeline으로 연결되는 것을 확인했다.

- AI combat action request가 BehaviorTree에서 공통 Action Request 경로로 전달되는 것을 확인했다.

- Started 결과에서만 `NextCombatActionTime`이 commit되고, Rejected / Ignored 결과에서는 관찰 가능한 state change가 남지 않는 것을 확인했다.

- Player / AI combo chain이 같은 request 경로와 chain window 기준을 사용하고, follow-up request가 Chained 결과로 accepted 된 뒤 chain 실행으로 이어지는 것을 확인했다.

- Reaction takeover 이후 active action state가 정리되고 combat 판단이 다시 이어질 수 있는 것을 확인했다.

- `ACAttachment -> ACWeaponActor` class rename 이후 기존 weapon blueprint parent class load 문제가 복구된 것을 확인했다.

---

## 관련 문서

- Issue Checklist: `D16_UE5_Portfolio_Issue_Checklist.md`

- Bug Report:
  - `B06_UE5_Portfolio_Bug_Report.md`
  - `B07_UE5_Portfolio_Bug_Report.md`
  - `B08_UE5_Portfolio_Bug_Report.md`

---

## 정리

- P15는 Player와 AI의 action 실행 진입점을 Action Request 기반 공통 pipeline으로 통일한 PR이다.

- Combo chain, cooldown commit, failure handling, reaction takeover cleanup 기준도 같은 action 실행 흐름 안에서 정리해 후속 reaction orchestration과 orchestration refactor로 확장할 기준을 남겼다.
