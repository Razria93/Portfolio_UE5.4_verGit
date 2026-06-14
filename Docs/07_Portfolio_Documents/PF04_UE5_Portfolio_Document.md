## 1. 문서 목적

본 문서는 Enemy AI가 Behavior Tree 기반 의사결정을 통해 movement / combat intent를 생성하고, 이를 Player와 동일한 component-driven execution pipeline으로 전달하는 구조를 설명한다.
핵심은 AI 전용 전투 실행 흐름을 따로 만들지 않고, **AI 판단 결과만 기존 Action / Reaction 실행 구조에 연결하는 것**이다.

---
## 2. 관련 자료 링크

##### Github

- PR: feature/ai-behaviortree-core (#30)
	https://github.com/Razria93/Portfolio_UE5.4_verGit/pull/30#issue-4173126451
  
---
## 3. 문제 정의

1차 프로젝트에서는 Enemy AI의 전투 행동이 Character 내부 API나 개별 AI task node에 직접 구현되어 사용되고 있었다.

[1차 프로젝트 AI 행동 구조]
![[07_Portfolio_Documents/img/BT_Melee.png]]


이 구조는 아래 문제를 가진다.

1. AI 행동이 montage 실행이나 state 변경에 직접 관여하면 Player와 다른 실행 흐름이 생긴다.
2. 공격, 피격, 사망 같은 전투 실행 및 흐름이 AI 전용 코드와 Player 코드로 분리될 수 있다.
3. Behavior Tree의  task node가 실행 판단과 실제 실행 책임을 함께 가지면 task가 비대해진다.
4. AI 패턴이 늘어날 때마다 behavior tree 및 task node를 새로 짜야한다.

---
## 4. 책임 경계 설정

2차 프로젝트 AI 행동 구조에서는 AI를 “실행 주체”가 아니라 “intent 생성 주체”로 본다.

- **Behavior Tree**: 현재 상황을 판단하고 다음 행동 intent를 선택한다.
- **Blackboard**: AI 판단에 필요한 target, distance, state, patrol point 등을 저장한다.
- **AI Task / Service**: Blackboard를 갱신하거나 action / movement request를 생성한다.
  
- **Orchestrator**: AI가 보낸 request를 Player 입력과 동일한 방식으로 해석한다.
- **Component / Executor**: 실제 action 실행, reaction 처리, montage lifecycle을 수행한다.

---
## 5. AI 실행 흐름

2차 프로젝트에서는 책임 경계 설정을 기반으로 AI 전투 실행은 다음 흐름으로 처리된다.

[2차 프로젝트 AI 행동 구조 (BT_Intent 레벨)]
![[07_Portfolio_Documents/img/BT_Default.png]]


[2차 프로젝트 AI 행동 구조 (BT_Action / BT_Reaction 레벨)]
<div style="display: flex; gap: 10px; align-items: center;"> 
<img src="BT_Action.png" width="250"> 
<img src="BT_Reaction.png" width="250"> 
</div>

```text
RootNode
-> Update Context
-> Execute AI Task
-> Movement or Combat Request
-> Action & Reaction Execution (PF03 문서 참고)
```

AI의 BehaviorTree는 상황을 판단한 뒤 request를 만들고, 실제 실행 가능 여부와 실행 방식은 Action / Reaction execution pipeline에서 처리한다.

---
## 6. 문제 해결

이 구조를 통해 기대한 효과는 다음과 같다.

1. Player와 Enemy가 같은 execution pipeline을 공유한다.
2. AI 전용 실행 예외 흐름을 줄인다.
3. Behavior Tree task는 request 생성에 집중한다.
4. AI 판단과 행동 실행 lifecycle을 분리한다.
5. Enemy 패턴 확장 시 request 규칙과 execution 규칙을 분리해서 관리한다.

---
## 7. 정리

Enemy AI 구조의 핵심은 AI 전용 전투 실행 시스템을 따로 만들지 않는 것이다.

AI는 Behavior Tree와 Blackboard를 통해 상황을 판단하고, 필요한 movement / combat request를 생성한다.

이후 실제 실행은 Player와 동일한 Action / Reaction execution pipeline을 통해 처리된다.

```text
AI Decision
-> Intent / Request
-> Common Execution Pipeline
-> Component Apply
-> Executor Lifecycle
```

이를 통해 AI 판단 로직과 전투 실행 로직을 분리하고, Player와 Enemy가 같은 실행 구조를 공유할 수 있는 기반을 마련하였다.

---
