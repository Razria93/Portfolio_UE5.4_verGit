## 1. 문서 목적

본 문서는 프로젝트 구현 과정에서 발생한 주요 버그를 바탕으로, 문제를 재현하고 원인을 추적한 뒤 해결한 과정을 정리한다.

핵심은 단순히 버그를 수정했다는 것이 아니라, Unreal Engine 특성, AI 상태 관리, damage pipeline, action / reaction execution flow에서 발생한 문제를 어떤 기준으로 분석하고 구조적으로 정리했는지 보여주는 것이다.

---
## 2. 문제 분석 방식

각 버그는 다음 흐름을 기준으로 분석하여 문제를 해결하였고, 이를 기반으로 버그 리포트 문서를 작성하였다.

```text
현상
-> 재현 조건
-> 기대 결과와 실제 결과 비교
-> 원인 추적
-> 수정 방향
-> 검증 결과
```

---
## 3. Unreal Asset / Editor 처리 문제

**관련 버그 리포트**

- `M01-B01`: Mannequin animation을 Quinn skeleton에 적용했을 때 pose 변형 발생
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/7#issue-3694558595
  
- `M01-B02`: `USTRUCT` reference parameter로 인한 editor crash
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/10#issue-3737809293
  
- `M05-B01`: C++ class rename 이후 기존 Blueprint parent class load 실패
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/42#issue-4294988086

**문제 유형**

Unreal Engine의 asset reference, reflection, editor serialization 특성을 충분히 고려하지 않았을 때 발생한 문제들이다.

**핵심 원인**

1. `M01-B01`: **UE4 Mannequin skeleton** 기준으로 제작된 animation data를 올바른 retargeting 없이 **UE5 Quinn skeleton**으로 강제 교체하였다.
2. `M01-B02`: editor / reflection 경로에서 `USTRUCT`를 참조로 넘기면서, 참조 대상의 수명이 보장되지 않아 **dangling reference**가 발생했다.
3. `M05-B01`: C++ class re제

**관련 버그 리포트**

- `M03-B01`: `UpdateAIContext` early return으로 Blackboard cleanup 누락
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/28#issue-4020478273
	
- `M03-B02`: `AttackIndex` 초기화 누락으로 공격 패턴 고정
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/31#issue-4179159922

**문제 유형**

AI 판단에 사용하는 Blackboard 값이 상태 전환 과정에서 명확하게 갱신되지 않아 발생한 문제들이다.

**핵심 원인**

1. `M03-B01`: context build 실패를 error와 no data로 구분하지 않고 early return으로 일괄 처리하다가 Blackboard cleanup이 생략되었다.
2. `M03-B02`: `AttackIndex`를 `ClearValue()`만 하고 명시적으로 `INDEX_NONE`으로 초기화하지 않았다.

**해결 방향**

1. `M03-B01`: AI context 결과를 `Success / NoData / Error`로 분리하고, 각 결과에 따라 Blackboard 값을 명시적으로 set / clear하도록 수정했다.
2. `M03-B02`: attack index의 invalid state를 명확히 표현하기 위해 `AttackIndex`를 `INDEX_NONE`으로 초기화했다.

**정리**

이 사례들은 Behavior Tree 자체의 문제가 아니라, 판단에 사용되는 context 값과 Blackboard key의 lifecycle을 명확히 관리하지 못했을 때 발생한 문제다.

---
## 5. Damage Pipeline 내 Context 구성 타이밍 문제

**관련 버그 리포트**

- `M03-B03`: `CurrentHitWindowId` 증가 타이밍 문제로 첫 hit가 `InvalidRequest` 처리됨
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/35#issue-4208810915

**문제 유형**

collision enable과 hit context 구성 타이밍이 어긋나면서 damage request가 잘못된 context로 생성된 문제다.

**핵심 원인**

1. `M03-B03`: 
	- `CollisionEnabled()`에서 collision을 먼저 활성화하고, 이후 `CurrentHitWindowId`를 증가시켰다.
	- 첫 overlap이 즉시 발생하면 `HitWindowId = INDEX_NONE` 상태로 `FHitContext`가 만들어졌고, 이 `FHitContext`는 `ApplyDamageComponent`에서 `InvalidRequest`로 reject되었다.

**해결 방향**

1. `M03-B03`: 
	- collision을 활성화하기 전에 `CurrentHitWindowId`를 먼저 갱신하여, 첫 overlap callback에서 생성되는 `FHitContext`에도 유효한 `HitWindowId`가 들어가도록 수정했다.

**정리**

이 사례는 damage 계산 문제가 아니라, montage notify timing과 collision callback timing 사이의 순서 문제였다.

---
## 6. Player와 AI의 Combo Chain 흐름 불일치

**관련 버그 리포트**

- `M05-B02`: AI ComboAttack이 다음 combo step으로 이어지지 않고 첫 타만 반복됨
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/43#issue-4349689185

**문제 유형**

Player는 chain 입력 흐름을 사용하고 있었지만, AI는 같은 chain request 흐름을 재사용하지 않아 combo가 이어지지 않은 문제다.

**핵심 원인**

1. `M05-B02`: 
	- AI가 montage notify timing에서 Player와 동일한 chain request를 다시 발행하지 않았다.
	- Player와 AI의 combo chain 진입 경로가 달라 같은 `ComboAttack` chain 판단 경로를 공유하지 못했다.

**해결 방향**

1. `M05-B02`: 
	- `ChainWindowOpened` notify를 event로 변환하고, `ActionComponent`를 통해 Enemy가 event를 수신하도록 callback 경로를 구성했다.
	- Enemy는 event 수신 시 기존 combat action request를 다시 발행하여 해당 request가 `ActionOrchestrator -> ComboAttack executor`흐름에서 chain으로 판정되게 정리했다.

**정리**

이 사례는 AI 전용 combo 로직이 부족한 문제가 아니라, Player와 AI가 같은 execution pipeline을 공유하지 못한 문제였다.

---
## 7. Reaction Takeover 이후 Action State 불일치

**관련 버그 리포트**

- `M05-B03`: AI가 ComboAttack 중 피격되면 Reaction 이후 action / state / blackboard 상태가 꼬이는 문제
	링크: https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/44#issue-4349693918

**문제 유형**

Reaction이 시작될 때 기존 active context을 명확히 정리하지 않아 runtime state가 서로 어긋난 문제다.

**핵심 원인**

1. `M05-B03`: 
	- HitReaction 진입 시 `ExecutionState`만 Reaction으로 전환되고, `ActionComponent`의 active context는 정리되지 않았다.
	- 그 결과 Reaction 종료 후 `ExecutionState`는 Idle이 되었지만 `CurrentActionType`은 `ComboAttack`으로 남아, 이후 action request가 `NoExecutableAction`으로 reject되었다.

**해결 방향**

1. `M05-B03`: 
	- HitReaction 시작 전에 `ActionComponent`의 active action을 abort하여, 이전 `ComboAttack`의 active context와 `CurrentActionType`을 먼저 초기화했다.
	- HitReaction 중에는 AI가 새 공격을 요청하지 않도록 Reaction 상태를 combat 가능 여부 판단에 포함했다. 
	- Reaction 종료 후에는 `ExecutionState`와 `CurrentActionType`이 모두 Idle 기준으로 정리되도록 처리 순서를 맞췄다.

**정리**

이 사례는 AI Blackboard만의 문제가 아니라, HitReaction 진입 시 기존 active context를 함께 정리하지 않아 `StateComponent`, `ActionComponent`, Blackboard가 서로 다른 실행 상태를 보게 된 문제였다.

---
## 8. 정리

이번 Troubleshooting 과정에서 확인한 핵심은 대부분의 문제가 단일 함수의 실수가 아니라, 실행 흐름의 책임 경계가 불명확할 때 발생했다는 점이다.

Asset / Editor 문제는 Unreal Engine의 asset reference, reflection, serialization 규칙을 기준으로 정리했다.

AI 문제는 Blackboard context와 combat state lifecycle을 기준으로 추적했고, 전투 실행 문제는 damage context 생성 시점, combo chain request 흐름, reaction 진입 시 active action 정리 여부를 중심으로 분석했다.

이를 통해 버그 수정 과정이 단순 패치가 아니라, Action / Reaction / Damage / AI 구조를 공통 execution pipeline으로 정리하는 근거가 되었다.

---
