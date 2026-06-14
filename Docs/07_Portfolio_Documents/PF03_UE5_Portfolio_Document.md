## 1. 문서 목적

본 문서는 본 프로젝트의 Action & Reaction Execution Pipeline에 대해서 설명한다.
핵심은 공격 객체와 피격 객체 사이에서 **Damage Event에 담길 데이터를 어떻게 생성하고, 송신하고, 수신하고, 해석하고, 적용하는가**를 책임 구조 기준으로 정리하는 데 있다.

---
## 2. 관련 자료 링크

##### Github

- PR: feature/action-orchestration (#45)
	https://github.com/Razria93/Portfolio_UE5.4_verGit/pull/45#issue-4350868136
	
- PR: feature/reaction-orchestration (#47)
	https://github.com/Razria93/Portfolio_UE5.4_verGit/pull/47#issue-4408323958
  
---
## 3. 문제 정의

[1차 프로젝트 액션 & 리액션 실행 파이프라인]
![[Action Reaction Execution Pipeline.png]]
이 구조는 아래 문제를 가진다.

1. Action과 Reaction이 모두 **montage 기반 실행**임에도, Action은 PlayerInput / Weapon Component 중심, Reaction은 TakeDamage / Character 중심으로 **서로 다른 실행 흐름**으로 구성되어 있다.
2. Action 실행은 DoAction API에 강하게 결합되어 있어, 새로운 Action을 추가하거나 Player 외의 주체가 Action을 실행하려면 **유사한 호출 경로를 별도로 구성**해야 한다.
3. Reaction 처리는 Character 내부에서 HP / Damage Feedback / montage 실행 / dead 판정 / state 전환까지 함께 담당하고 있어, **피격 처리 책임이 과도하게 집중**되어 있었다.
4. state 변경 / montage 실행 / montage 종료 / action & reaction 판단 책임이 분산 되어 있어, **기존 실행을 중단하고 다른 실행으로 전환하는 흐름을 처리하기 어려웠다.**

---
## 4. 책임 경계 설정

1차 구조의 문제를 해결하기 위해 2차 구조에서는 Action과 Reaction을 모두 **execution unit**으로 보고, 실행 흐름을 다음 책임으로 분리하였다.

- **Orchestrator**: 실행 요청을 해석하고 실행 방식을 결정한다.
- **Component**: 현재 실행 상태를 관리하고 위에서 결정된 실행 방식을 적용한다.
- **Executor**: 실제 montage 기반 실행, 중단 및 종료 관리와 notify 처리를 담당한다.
- **Data**: 실행에 필요한 데이터값을 제공한다.

이를 통해 Action과 Reaction이 서로 다른 경로로 실행되더라도, 내부적으로는 같은 execution pipeline을 따라 처리되도록 정리하였다.

---
## 5. Action / Reaction 실행 구조 재구성

2차 프로젝트에서는 책임 경계 설정을 기반으로 Action / Reaction 실행 구조 재구성을 진행한다.

Action은 player input 또는 AI intent에서 시작하고, Reaction은 damage result에서 시작하지만, Rquest 부터는 동일한 Execution Pipeline을 따른다.

특히 실행 판단을 단순히 “실행 가능 여부”로만 보지 않고, 다음 기준으로 나누었다.

**CAction / CReaction에서 Query를 보고 판단**
- **ExecutionDecision**: 실행을 수락할지, 거절할지, 무시할지 
- **ExecutionRelationship**: active중인 실행과 독립적인지, 연속적인지, 충돌하는지
  
**Orchestrator에서 Query / Decision / Relationship을 보고 판단**
- **ExecutionAcceptMode**: 즉시 시작할지, 예약할지, 기존 실행에 개입할지
- **ExecutionInterventionDirective**: 기존 실행에 개입 할 경우 정지가 가능한지 및 무엇을 정지해야할지
  
결과적으로 2차 구조에서는 Action과 Reaction을 아래 다이어그램과 같은 흐름으로 재구성하였다.

[2차 프로젝트 액션 & 리액션 실행 파이프라인]
![[Unified Execution Pipeline.png]]

---
## 6. 문제 해결

이 재구성을 통해 기대한 효과는 다음과 같다.

1. Action과 Reaction을 같은 execution 단위로 비교할 수 있게 되었다.
2. state 변경, montage 실행, 종료 처리를 Component / Executor 책임으로 분리할 수 있게 되었다.
3. HitReaction이 AttackAction을 interrupt하거나, DodgeAction이 HitReaction을 cancel하는 cross-domain intervention을 처리할 수 있게 되었다.
4. ComboAttack처럼 기존 실행과 이어지는 reserve execution을 별도 흐름으로 다룰 수 있게 되었다.
5. 새로운 Action / Reaction을 추가할 때 입력 경로가 아니라 candidate resolve와 executor 구현 중심으로 확장할 수 있게 되었다.

---
## 7. 정리

1차 프로젝트에서는 Action과 Reaction이 모두 montage 기반 실행임에도 서로 다른 구조로 처리되고 있었다.

```text
Action 
-> PlayerInput / WeaponComponent / DoAction 중심 실행 

Reaction 
-> TakeDamage / Character 내부 처리 중심 실행
```

state 변경 / montage 실행 / montage 종료 / action & reaction 판단 책임이 여러 객체에 흩어져 있었고, 기존 실행을 중단하거나 다른 실행으로 전환하는 흐름을 공통 규칙으로 처리하기 어려웠다.

따라서 2차 프로젝트에서는 이를 다음 흐름으로 재구성하였다.

``` text
Request 
-> Candidate
-> ExecutionContext
-> Decision
-> Relationship
-> AcceptMode
-> Dispatch
-> Component Apply
-> Executor Lifecycle
```

핵심은 Action과 Reaction을 별개의 기능 흐름이 아니라 **공통 execution pipeline 위에서 처리되는 실행 단위**로 정리한 것이다.

이를 통해 실행 판단, 상태 적용, montage lifecycle, intervention, reserve execution을 같은 구조 안에서 확장할 수 있는 기반을 마련하였다.

---