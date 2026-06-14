# Shared Reaction Execution_Pipeline 설계

## 1. 목적

본 문서는 reaction execution Pipeline을 `ReactionOrchestrator -> ReactionComponent -> CReaction` 구조로 재구성한 이유와 현재 구현 기준의 핵심 decision을 정리하기 위한 system architecture 문서임.

archive 문서 `A01`, `A03`, `A07`의 내용을 하나의 흐름으로 재구성함.

---

## 2. 배경

### 2.1 기존 흐름

기존 reaction 구조는 `PendingReaction`을 중심으로 동작했음.

당시 흐름은 다음에 가까웠음.

```text
TakeDamage
-> PendingReaction 저장함
-> Player Tick 또는 Enemy BT에서 consume함
-> TryExecuteReaction()
-> QueryReplaceReaction()
-> CReaction 실행 또는 대체함
```

### 2.2 문제점

문제는 판단과 실행 흐름이 `PendingReaction` 이후의 외부 소비자에게 분산된 점임.

Enemy는 BT task에서 pending reaction을 소비했고, Player는 Tick에서 pending reaction을 소비했음.

결과적으로 동일한 damage event에서 시작한 reaction이 Player와 Enemy에서 서로 다른 실행 경로를 가졌음.

### 2.3 원인

원인은 reaction이 AI부터 구현되면서 BT 내부에서 실행을 제어하는 구조가 먼저 만들어졌기 때문임.

Player는 BT가 없기 때문에 같은 pending 모델을 맞추기 위해 Tick consume 경로를 추가해야 했음.

Reaction은 player나 AI가 선택하는 intent action이 아니라, 외부 damage event에 의해 발생하는 body execution response에 가까움.

따라서 reaction request는 Player와 Enemy 모두 동일한 damage-driven execution pipeline에서 판단되고 실행되는 것이 더 적절함.

### 2.4 해결 방향

해결 방향은 damage event 이후 reaction 실행까지의 흐름과 책임을 정리하여 orchestration pipeline으로 재구성하는 것임.

```text
ReactionOrchestrator
-> request 해석과 경쟁 상태 판단을 담당함

ReactionComponent
-> active runtime state와 decision 적용을 담당함

CReaction
-> 실제 montage lifecycle과 local execution rule을 담당함
```

즉 reaction orchestration은 damage event 이후의 request 해석, 경쟁 상태 판단, 실행 적용, 실제 reaction lifecycle을 명확한 책임 단위로 분리하는 작업임.

그 결과 기존 pending-driven 실행 모델은 damage-driven orchestration 모델로 전환되고, Player와 Enemy의 reaction 실행 흐름이 통합됨.

---

## 3. 결정 사항

Reaction execution flow는 다음 구조로 정리함.

```text
TakeDamage
-> ReactionOrchestrator
-> ReactionComponent
-> CReaction
```

각 계층의 책임은 다음처럼 분리함.

```text
ReactionOrchestrator
-> request를 해석함
-> context와 policy를 resolve함
-> active reaction과 incoming reaction을 비교함
-> Start / Interrupt / Cancel / Ignore / Reject decision을 생성함

ReactionComponent
-> active reaction state를 소유함
-> orchestrator decision을 runtime state에 적용함
-> executor instance를 조회하고 제어함
-> action abort / movement lock / execution state 전환을 처리함

CReaction
-> 실제 montage lifecycle을 수행함
-> control window와 feedback notify를 처리함
-> local interrupt / cancel hook을 제공함
-> stop reason을 finish reason으로 확정함
```

이 구조에서는 `ReactionOrchestrator`가 판단의 중심이고, `ReactionComponent`가 실행 상태 적용의 중심이며, `CReaction`이 실제 실행의 중심임.

---

## 4. 요청 흐름

현재 reaction request flow는 다음과 같음.

```text
CTakeDamageComponent
	-> FDamageReactionRequest 구성함

-> UCReactionOrchestratorComponent::RequestReaction()
	-> CanAcceptReactionRequest()
	-> ResolveReactionContext()
	-> ResolveReactionPolicy()
	-> BuildOrchestrationQuery()
	-> OrchestrateQuery()
	-> DispatchReactionDecision()

-> UCReactionComponent::ApplyReactionDecision()
-> CReaction::Start() / Stop()
```

1. `RequestReaction()`
	- reaction request의 public entry point.

2. `ResolveReactionContext()`
	- damage result를 reaction execution context로 구체화.

3. `ResolveReactionPolicy()`
	-  현재 request가 어떤 권한으로 처리될 수 있는지 해석.

4. `OrchestrateQuery()`
	- active reaction과 incoming reaction의 충돌을 판단.

5. `DispatchReactionDecision()`
	- 판단 결과를 reaction component에 적용.

---

## 5. 결정 타입

현재 reaction orchestration decision은 다음 범위를 사용함.

```text
Start
Interrupt
Cancel
Ignore
Reject
```

1. `Start` 
	- active reaction이 없을 때 incoming reaction을 시작하는 decision.

2. `Interrupt`
	- 객체 외부 요인에 의해 active reaction을 중단하고 incoming reaction으로 교체하는 decision.

3. `Cancel`
	- 객체 내부 요청에 의해 active reaction을 중단하는 decision.
	- 이후 `dodge action` 혹은 `execution reaction` 같은 후속 정책을 실행할 수도 있음.

4. `Ignore`
	- 요청이 유효하지만 현재 상태에서 처리하지 않는 decision.

5. `Reject`
	- 요청 자체 또는 실행 조건이 유효하지 않은 decision.

기존 설계 단계에서 검토했던 `ReplacePending`, `Enqueue`, queue 기반 처리 흐름은 1차 구현 범위에서 제외함.

---

## 6. 생명주기 의미

Reaction lifecycle에서 용어는 다음 의미로 고정함.

```text
Start
-> active reaction이 없을 때 executor를 시작함

Interrupt
-> active reaction을 외부 요인으로 중단하고 incoming reaction을 시작함

Cancel
-> active reaction을 의도적으로 중단함

Stop
-> component가 executor에게 중단을 요청함

Finish
-> executor가 종료 사유를 확정하고 component에 종료를 알림

MontageEnd
-> montage의 정상 완료를 감지하는 callback임
```

중요한 점은 `Stop`과 `Finish`가 같은 개념이 아니라는 점임.

`Stop`은 외부 제어 요청이고, `Finish`는 executor가 runtime state를 정리하고 종료 사유를 확정하는 단계임.

따라서 `ReactionComponent`는 `Stop`을 호출하고, `CReaction`은 `FinishCompleted`, `FinishInterrupted`, `FinishCancelled`, `FinishAborted` 중 하나로 종료를 확정함.

---

## 7. 현재 구현

현재 구현 기준의 핵심 클래스는 다음과 같음.

```text
UCReactionOrchestratorComponent
	-> RequestReaction()
	-> ResolveReactionContext()
	-> ResolveReactionPolicy()
	-> OrchestrateQuery()
	-> DispatchReactionDecision()

UCReactionComponent
	-> ApplyReactionDecision()
		- TryStartReaction()
		- TryInterruptReaction()
		- TryCancelReaction()
	-> StopActiveReactionInternal() (Interrupt / Cancel의 경우)
	-> StartActiveReactionInternal()
	-> EndActiveReactionInternal()

UCReaction
	-> Start()
	-> Stop() (Interrupt / Cancel의 경우)
	-> FinishCompleted()
	-> FinishInterrupted()
	-> FinishCancelled()
	-> FinishAborted()
	-> OnReactionControlWindowBegin()
	-> OnReactionFeedback()
```

Guard / parry / counter / launch / knockdown / queue는 후속 확장 범위로 분리함.

---

## 8. 결과

이 결정의 장점은 다음과 같음.

```text
TakeDamage 이후 reaction 실행 진입점이 단일화됨
Player와 AI가 같은 reaction execution pipeline을 공유함
ReactionComponent가 pending 저장소가 아니라 active runtime state manager로 정리됨
CReaction이 montage / notify / feedback lifecycle에 집중할 수 있음
Reaction 간 경쟁 상태 판단을 orchestrator에서 일관되게 처리할 수 있음
```

주의할 점은 다음과 같음.

```text
ReactionOrchestrator가 ReactionComponent의 active state와 executor 정보를 조회해야 함
장기적으로 component에 있는 DataAsset / DataProvider 가 외부로 분리될 필요가 있음
Cancel decision을 결정하는 구체적인 cancel policy는 후속 작업에서 확정해야 함
```

---

## 9. 후속 작업

후속 작업 후보는 다음과 같음.

```text
Reaction definition data를 DataAsset 또는 DataProvider 계층으로 분리함
guard / parry / poise / super armor 기반 policy를 ResolveReactionPolicy에 반영함
cancel decision의 실제 request source와 policy를 확정함
action orchestration을 reaction 구조에서 정리한 책임 분리 원칙에 맞춰 리팩터링함
```

---

## 10. 관련 문서

관련 상세 문서는 다음과 같음.

```text
A01_UE5_Portfolio_Action_Reaction_Orchestration_Comparison
A03_UE5_Portfolio_Execution_Orchestration_API_Model
A07_UE5_Portfolio_Reaction_Lifecycle_Model
```

---
