# Reaction Orchestration 구조 구축 및 Reaction Feedback / Damage Impact 흐름 정리

## 제목

`✨ feat: reaction orchestration 구조 구축 및 reaction feedback / damage impact 흐름 정리 (#46)`

## 요약

- 본 PR에서는 **TakeDamage 이후 reaction 실행 흐름을 Reaction Orchestrator 중심 구조로 재구성**하였고, 기존 pending 기반 reaction 실행 흐름을 제거하여 Player / AI가 같은 reaction execution pipeline을 공유하도록 정리하였음.

- 핵심 방향은 다음과 같음.

	- `TakeDamage -> ReactionOrchestrator -> ReactionComponent -> CReaction` 흐름을 명시적으로 구성함.

	- `ReactionComponent`는 active reaction state와 executor 제어에 집중하도록 정리함.

	- `CReaction`은 montage lifecycle, stop / finish reason, reaction feedback 요청을 담당하도록 정리함.

	- BT는 reaction을 실행하지 않고 active reaction state를 관찰하는 구조로 축소함.

	- hit feedback과 reaction execution feedback을 분리하고, damage impact 위치 정보를 damage feedback에서 사용할 수 있도록 구성함.

- 또한 reaction feedback notify, reaction control notify, damage feedback, feedback matching, action notify base 정리, 관련 asset 및 설계 문서를 함께 추가 / 보완하였음.


---

## 완료 작업

### 1. Reaction Orchestration 요청 / 결과 구조 추가

- `CReactionOrchestrationStructure`를 추가함

- reaction request / result / decision / policy / query 구조를 추가함

- `FDamageReactionRequest`를 기반으로 `TakeDamage` 이후 reaction 실행 요청을 표현하도록 구성함

- `EReactionRequestResultType`, `EReactionRequestRejectReason`, `EReactionOrchestrationDecision`, `FReactionExecutionPolicy`를 추가하여 reaction request 결과와 orchestration decision을 분리함

### 2. ReactionOrchestratorComponent 추가

- `UCReactionOrchestratorComponent`를 추가함

- `RequestReaction()`을 reaction 실행 요청의 외부 진입점으로 구성함

- `ResolveReactionContext()`에서 damage result를 reaction type / reaction data / reaction executor로 구체화함

- `ResolveReactionPolicy()`에서 request 시점의 priority / interrupt 권한 / force interrupt 정책을 해석함

- `OrchestrateQuery()`에서 active reaction과 incoming reaction의 경쟁 상태를 판단함

- `DispatchReactionDecision()`에서 최종 decision을 `ReactionComponent`에 적용하도록 구성함

### 3. ReactionComponent 역할 정리

- 기존 pending reaction consume 흐름을 제거함

- `ReactionComponent`를 active reaction state와 executor 제어 중심으로 정리함

- `ApplyReactionDecision()`을 추가하여 orchestrator decision을 적용하도록 구성함

- `TryStartReaction`, `TryInterruptReaction`, `TryCancelReaction` 흐름을 구성함

- `StartActiveReactionInternal`, `StopActiveReactionInternal`, `EndActiveReactionInternal`을 통해 active reaction lifecycle을 정리함

- stale active reaction 상태를 정리할 수 있도록 fallback cleanup을 보강함

### 4. CReaction lifecycle 정리

- `CReaction`이 실제 reaction executor로서 montage 실행, control window, feedback notify, stop / finish 처리를 담당하도록 정리함

- `Stop()`은 external stop request를 받고, stop reason에 따라 finish reason을 확정하도록 구성함

- `FinishCompleted`, `FinishInterrupted`, `FinishCancelled`, `FinishAborted`를 분리함

- `MontageEnd`는 정상 완료 감지 중심으로 사용하고, system stop은 `Stop()`에서 명시적으로 처리하도록 정리함

- `WantToInterrupt`, `WantToCancel`, `AllowInterruptionBy`, `AllowCancelBy`를 local policy hook으로 유지함

### 5. AI reaction pending consume 제거 및 관찰 구조로 축소

- Player tick의 pending reaction consume 흐름을 제거함

- BT pending reaction consume 흐름을 제거함

- `CBTTask_StartReaction`은 reaction 실행 주체가 아니도록 축소함

- `CBTTask_WaitEndReaction`은 active reaction state를 관찰하는 역할로 정리함

- BT service / blackboard는 pending request가 아니라 active reaction state를 관찰하도록 정리함

### 6. Reaction Feedback 구조 추가

- `CReactionFeedbackComponent`를 추가함

- `CReactionFeedbackStructure`를 추가하여 reaction feedback key / timing / request / execution key 구조를 분리함

- reaction type, damage spec key, timing, trigger key 기반 feedback matching을 구성함

- wildcard matching과 duplicate execution key filtering을 정리함

- `CReaction`이 자신의 active context를 기반으로 reaction feedback request를 생성하도록 구성함

### 7. Reaction Feedback Notify / Reaction Control Notify 추가

- `CAnimNotifyState_Reaction`을 `CAnimNotifyState_ReactionControl`로 rename함

- reaction control window와 reaction feedback window를 분리함

- `CAnimNotify_ReactionFeedback`을 추가하여 point reaction feedback을 실행할 수 있도록 구성함

- `CAnimNotifyState_ReactionFeedback`을 추가하여 window begin / end reaction feedback을 실행할 수 있도록 구성함

- reaction notify는 `ReactionComponent`를 통해 active reaction executor로 전달되도록 구성함

### 8. DamageFeedbackComponent 분리

- 기존 hit feedback 성격의 component를 `CDamageFeedbackComponent`로 분리 / 명확화함

- `TakeDamagePacket` 기반으로 hit VFX / hit SFX / hit stop / camera shake request를 처리하도록 구성함

- reaction feedback과 damage feedback의 책임을 분리함

- `DamageFeedback`은 damage event와 impact metadata를 기준으로 하고, `ReactionFeedback`은 reaction execution timing을 기준으로 하도록 정리함

### 9. DamageImpactInfo 추가

- `FDamageImpactInfo`와 `EDamageImpactInfoSource`를 추가함

- `FHitContext`, apply damage payload / context, default damage event, take damage payload / context에 damage impact info를 전달하도록 구성함

- `ACWeaponActor`에서 `SweepResult` 우선, `GetClosestPointOnCollision()` fallback 기준으로 impact point를 구성함

- `CDamageFeedbackComponent`가 `TakeDamagePacket.Context.DamageImpactInfo`를 기반으로 hit feedback 위치 / 방향을 계산할 수 있도록 구성함

### 10. Action Notify base 정리

- action 전용 notify trigger filter를 `CAnimNotify_ActionBase`, `CAnimNotifyState_ActionBase`로 분리함

- 공통 notify base인 `CAnimNotify`, `CAnimNotifyState`는 action trigger field를 직접 갖지 않도록 정리함

- reaction notify가 불필요한 action trigger field를 갖지 않도록 구조를 정리함

### 11. Asset 및 Blueprint 반영

- Player / Enemy blueprint에 reaction orchestrator, reaction feedback, damage feedback 관련 구성을 반영함

- hit reaction montage와 관련 skeleton / blackboard asset을 갱신함

- Slash hit VFX 및 HealPositive 계열 feedback asset을 추가 / 갱신함

- TestRoom 및 관련 unit test asset을 갱신함

### 12. 설계 문서 및 이슈 문서 추가

- `D17`: Reaction Orchestration issue checklist 추가함

- `S06`: Action / Reaction Orchestration 비교 문서 추가함

- `S07`: Orchestrator / Data / Component 책임 분리 문서 추가함

- `S08`: Execution Orchestration API 모델 문서 추가함

- `S09`: Reaction Pending 모델 문서 추가함

- `S10`: Reaction Execution Policy 모델 문서 추가함

- `S11`: Weapon Trail Trace 모델 문서 추가함

- `S12`: Reaction Lifecycle 모델 문서 추가함

- `S13`: Combat Feedback 모델 문서 추가함

- `S14`: AI Reaction Observation 모델 문서 추가함

- `S15`: Action Orchestration Refactor 모델 문서 추가함


---

## 테스트 방법

1. Player가 피격될 때 `TakeDamage -> ReactionOrchestrator -> ReactionComponent -> CReaction` 흐름으로 hit reaction이 실행되는지 확인

2. Enemy가 피격될 때 BT pending consume 없이 reaction이 실행되는지 확인

3. hit reaction 중 더 높은 우선순위 reaction 또는 dead reaction이 들어왔을 때 interrupt decision이 정상 동작하는지 확인

4. reaction 완료 / interrupt / cancel / abort 흐름에서 active reaction state가 정상 정리되는지 확인

5. reaction 중 movement / action state가 어긋나지 않는지 확인

6. BT가 reaction을 직접 실행하지 않고 active reaction state를 관찰하는지 확인

7. reaction feedback point notify와 window notify가 active reaction executor를 통해 정상 실행되는지 확인

8. damage feedback이 `DamageImpactInfo` 기반 위치에서 hit VFX / hit SFX를 재생하는지 확인

9. `bFromSweep == false` 상황에서도 closest point fallback으로 hit VFX 위치가 자연스럽게 계산되는지 확인

10. action notify와 reaction notify가 서로 불필요한 trigger field를 공유하지 않는지 확인

11. Player / Enemy blueprint와 montage asset이 정상 로드되고 visual feedback이 정상 재생되는지 확인

12. `PortfolioEditor Win64 Development` 빌드가 성공하는지 확인


---

## 검증

- `git diff --check` 통과함

- `PortfolioEditor Win64 Development` 빌드 통과함

- Player / Enemy 피격 visual feedback 정상 동작 확인함

- damage impact 기반 hit VFX 위치 정상 동작 확인함


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/reaction-orchestration`

- 관련 작업:

  - `D17: Reaction Orchestration issue checklist`

  - `S06 ~ S15: Reaction Orchestration 및 후속 구조 설계 문서`

- 후속 작업 후보:

  - `feature/action-orchestration-refactor`

  - action orchestration을 reaction orchestration에서 정리한 책임 분리 원칙에 맞춰 리팩터링


---
