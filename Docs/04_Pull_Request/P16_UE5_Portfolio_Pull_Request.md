# UE5 Portfolio Pull Request

## 제목

**P16: Reaction 공용 실행 Pipeline 구성 및 Feedback 책임 분리**

## 날짜

**2026.05.09**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/reaction-orchestration`

---

## 요약

### 작업 요약

본 PR은 `TakeDamage` 이후 발생하는 reaction 실행 흐름을 공통 execution pipeline으로 재구성한 작업이다.

### 이전 구조의 한계점

기존 구조에서는 pending reaction 소비, runtime state 적용, montage lifecycle, feedback 처리 책임이 Player / AI / Component / Executor에 분산되어 있었다.

### 현재 구조의 보완사항

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Reaction 실행 진입점 단일화
- TakeDamage 이후 UCReactionOrchestratorComponent를 통해 reaction request 처리

2. Runtime 책임 분리
- UCReactionOrchestratorComponent : request 해석 / candidate resolve / execution decision 구성
- UCReactionComponent    : active reaction state 소유 / execution result 적용
- UCReaction            : montage lifecycle / notify 기반 feedback 처리

3. AI 역할 축소
- BT는 reaction을 직접 실행하지 않고 active reaction state 관찰

4. Feedback 책임 분리
- DamageFeedback은 damage impact 기준
- ReactionFeedback은 reaction execution timing 기준
```

---
## 변경 범위

### Reaction Execution Pipeline

#### A. Damage Reaction Request 진입점

- `TakeDamage` 이후 damage result를 reaction orchestration으로 전달하는 진입 흐름을 추가했다.

**Flow**
```yaml
UCTakeDamageComponent
-> FDamageReactionRequest 구성
-> UCReactionOrchestratorComponent::RequestDamageReaction 호출
```

**Structure**
```yaml
FDamageReactionRequest
- IntentSource     : reaction request 발생 출처
- TakeDamagePacket : TakeDamage 처리 결과와 context
```

#### B. Reaction Orchestrator 진입 및 판단 흐름

- `UCReactionOrchestratorComponent`를 추가하여 damage reaction request 해석, candidate resolve, execution decision, component dispatch를 담당하도록 구성했다.

**Flow**
```yaml
TakeDamage 호출됨
-> FDamageReactionRequest 구성
-> UCReactionOrchestratorComponent::RequestDamageReaction 호출
-> ...
-> UCReactionComponent
-> UCReaction
```

**Structure**
```yaml
UCReactionOrchestratorComponent
- RequestDamageReaction          : damage reaction request의 orchestration 진입점
- ResolveDamageReactionCandidate : TakeDamage 결과를 Hit / Dead reaction candidate로 변환
- ExecuteReactionCandidate       : reaction candidate를 execution pipeline으로 전달
- ResolveExecutionApplyMode      : relationship 기준으로 Start / Intervene 적용 방식 결정
- DispatchReactionDecision       : 최종 execution result를 UCReactionComponent에 전달
```

#### C. Reaction Result 구조 분리

- 외부 호출자에게 반환할 request result 구조체와 component가 소비할 execution result 구조체를 분리했다.

**Structure**
```yaml
FReactionRequestResult
- 외부 반환용 request result
- ResultType   : 외부 호출자에게 반환할 request 처리 결과
- RejectReason : request 거절 사유

FReactionExecutionResult
- component 소비용 execution result
- Decision              : component가 소비할 execution decision
- Relationship          : active execution과 incoming reaction의 관계
- ApplyMode             : component가 수행할 적용 방식
- ResolvedContext       : 실행할 reaction context
- RejectReason          : execution 거절 사유
- InterventionDirective : active execution 중단 지시
```

#### D. UCReactionComponent Runtime State 정리

- pending reaction consume 흐름을 제거하고, `UCReactionComponent`가 active reaction state와 execution result 적용을 담당하도록 정리했다.

**Flow**
```yaml
Before
-> PendingReaction 저장
-> Player Tick 또는 Enemy BT에서 consume
-> reaction 실행

After
-> UCReactionOrchestratorComponent가 request 판단
-> UCReactionComponent가 execution result 적용
-> UCReaction executor 실행
```

**Structure**
```yaml
UCReactionComponent
- ApplyReactionDecision        : orchestrator가 만든 execution result 소비
- TryStartReaction             : active reaction이 없을 때 incoming reaction 시작
- TryInterruptReaction         : active reaction 중단 후 incoming reaction 시작
- TryCancelReaction            : active reaction 취소 처리
- StartActiveReactionInternal  : active context 설정 후 executor start
- StopActiveReactionInternal   : active executor stop 요청
- EndActiveReactionInternal    : active context와 runtime state 정리
```

#### E. UCReaction Executor Lifecycle 정리

- `UCReaction`을 montage 기반 reaction executor로 정리하고, component state 적용과 montage lifecycle 처리를 분리했다.

**Structure**
```yaml
UCReaction
- Start             : montage 실행 및 runtime state 설정
- Stop              : external stop request 수신
- FinishCompleted   : 정상 완료
- FinishInterrupted : 외부 요인에 의한 중단
- FinishCancelled   : 의도적 취소
- FinishAborted     : 비정상 종료 또는 fallback cleanup
- MontageEnd        : montage 완료 callback 처리
```

#### F. AI Reaction Pending Consume 제거

- AI가 pending reaction을 직접 consume하지 않고, active reaction state를 관찰하도록 역할을 축소했다.

**Flow**
```yaml
Before
-> BT가 pending reaction 확인
-> BT task가 reaction consume
-> reaction 실행

After
-> UCReactionOrchestratorComponent가 reaction 실행 판단
-> UCReactionComponent가 active reaction state 소유
-> BT는 active reaction 종료 여부만 관찰
```

**Changes**
```yaml
AI / BT
- Player Tick pending consume 제거
- BT pending consume 제거
- UCBTTask_StartReaction 역할 축소
- UCBTTask_WaitEndReaction을 active reaction 관찰 task로 정리
- Blackboard / Service를 active reaction state 기준으로 갱신
```

#### G. Feedback 책임 분리

- damage event 기반 feedback과 reaction execution timing 기반 feedback을 서로 다른 component로 분리했다.

**Structure**
```yaml
DamageFeedback
- 담당 component : UCDamageFeedbackComponent
- 기준 데이터     : TakeDamagePacket / DamageImpactInfo
- 처리 대상       : hit VFX / hit SFX / hit stop / camera shake

ReactionFeedback
- 담당 component : UCReactionFeedbackComponent
- 기준 데이터     : reaction type / damage spec key / timing / trigger key
- 처리 대상       : reaction montage timing에 맞춘 feedback
```

#### H. Damage Impact Metadata 추가

- damage feedback 위치와 방향을 안정적으로 계산하기 위해 `FDamageImpactInfo`를 damage pipeline에 전달하도록 구성했다.

**Flow**
```yaml
DamageImpactInfo
-> FHitContext
-> ApplyDamage Payload / Context
-> TakeDamage Payload / Context
-> UCDamageFeedbackComponent
```

**Structure**
```yaml
FDamageImpactInfo
- ImpactPoint              : feedback 재생 위치
- ImpactNormal / Direction : feedback 방향 계산 기준
- ImpactSource             : 위치 정보 산출 방식
```

```yaml
Impact Source
- SweepResult 우선 사용
- SweepResult가 없으면 GetClosestPointOnCollision fallback
- fallback도 실패하면 default 위치 사용
```

#### I. Notify 구조 분리

- action notify와 reaction notify의 base를 분리하고, reaction control notify와 reaction feedback notify의 책임을 분리했다.

**Flow**
```yaml
Reaction Notify
-> UCReactionComponent
-> Active UCReaction
-> ReactionFeedbackRequest
-> UCReactionFeedbackComponent
```

**Structure**
```yaml
Reaction Notify
- UCAnimNotifyState_ReactionControl  : reaction control window
- UCAnimNotify_ReactionFeedback      : point reaction feedback
- UCAnimNotifyState_ReactionFeedback : window begin / end reaction feedback

Action Notify
- UCAnimNotify_ActionBase      : action 전용 point notify base
- UCAnimNotifyState_ActionBase : action 전용 state notify base
```

---
## 주요 Pipeline

### Reaction Request Pipeline

```yaml
TakeDamage
-> FDamageReactionRequest
-> UCReactionOrchestratorComponent::RequestReaction
-> ResolveReactionContext
-> ResolveReactionPolicy
-> OrchestrateQuery
-> DispatchReactionDecision
-> UCReactionComponent::ApplyReactionDecision
-> UCReaction::Start / Stop
```

### Reaction Feedback Pipeline

```yaml
Reaction Notify
-> UCReactionComponent
-> Active UCReaction
-> ReactionFeedbackRequest
-> UCReactionFeedbackComponent
-> Feedback 실행
```

### Damage Feedback Pipeline

```yaml
Hit Context
-> ApplyDamage Payload
-> TakeDamage Packet
-> DamageImpactInfo
-> UCDamageFeedbackComponent
-> Hit VFX / SFX / HitStop / CameraShake
```

---
## 테스트 방법

### Reaction 실행

- Player 피격 시 `TakeDamage -> UCReactionOrchestratorComponent -> UCReactionComponent -> UCReaction` pipeline으로 hit reaction이 실행되는지 확인
- Enemy 피격 시 BT pending consume 없이 reaction이 실행되는지 확인
- hit reaction 중 더 높은 우선순위 reaction 또는 dead reaction이 들어왔을 때 interrupt decision이 정상 동작하는지 확인
- reaction 완료 / interrupt / cancel / abort 이후 active reaction state가 정상 정리되는지 확인

### AI 관찰 구조

- BT가 reaction을 직접 실행하지 않고 active reaction state를 관찰하는지 확인
- reaction 중 movement / action state가 어긋나지 않는지 확인

### Feedback

- reaction feedback point notify와 window notify가 active reaction executor를 통해 실행되는지 확인
- damage feedback이 `DamageImpactInfo` 기반 위치에서 hit VFX / SFX를 재생하는지 확인
- `bFromSweep == false` 상황에서 closest point fallback으로 hit VFX 위치가 계산되는지 확인

### Asset / Build

- action notify와 reaction notify가 불필요한 trigger field를 공유하지 않는지 확인
- Player / Enemy blueprint와 montage asset이 정상 로드되는지 확인
- `PortfolioEditor Win64 Development` 빌드가 성공하는지 확인

---
## 검증 결과

- `git diff --check` 통과
- `PortfolioEditor Win64 Development` 빌드 통과
- Player / Enemy 피격 visual feedback 정상 동작 확인
- damage impact 기반 hit VFX 위치 정상 동작 확인

---
## 관련 문서

- Issue Checklist: `D17_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 reaction 실행을 pending consume model에서 orchestration 기반 execution pipeline으로 전환한 것이다.

변경 후에는 damage event 이후 생성된 reaction request를 `UCReactionOrchestratorComponent`가 판단하고, `UCReactionComponent`가 execution result를 적용하며, `UCReaction`이 montage lifecycle을 수행하도록 책임을 분리했다.

이를 통해 Player와 AI는 같은 reaction 실행 구조를 공유하게 되었고, runtime state 관리, montage execution, feedback timing 책임도 명확히 나뉘었다.

또한 feedback 구조를 damage impact 기반 feedback과 reaction execution timing 기반 feedback으로 분리하여, hit feedback과 reaction montage feedback을 서로 다른 기준으로 처리할 수 있게 됐다.

---
