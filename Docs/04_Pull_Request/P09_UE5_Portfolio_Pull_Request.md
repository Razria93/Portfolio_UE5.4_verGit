# UE5 Portfolio Pull Request

## 제목

**P09: Reaction Execution Pipeline 구현**

## 날짜

**2026.01.18**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-reaction`

---

## 요약

### 작업 요약

본 PR은 `TakeDamage` 이후 발생하는 hit / dead reaction을 `UCReactionComponent -> UCReaction` 구조로 실행할 수 있도록 Reaction Execution Pipeline을 구성한 작업이다.

```yaml
TakeDamage Commit
-> FTakeDamageResult
-> UCReactionComponent::RequestReaction
-> Resolve ReactionData
-> Resolve / Create UCReaction
-> Play or Replace Reaction
```

### 작업 배경

`UCTakeDamageComponent`가 damage 수신, health commit, reaction 실행까지 모두 직접 처리하면 damage 계산과 피격 연출 실행 책임이 강하게 섞일 수 있었다.

따라서 damage 결과 산출은 `UCTakeDamageComponent`가 담당하고, hit / dead reaction 선택과 실행은 `UCReactionComponent`가 담당하도록 분리할 필요가 있었다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. Reaction 요청 진입점 분리
- TakeDamage commit 이후 FTakeDamageResult 기반으로 reaction request 전달

2. Data-driven Reaction 선택
- FReactionDataKey / FReactionData 기반 reaction data resolve
- ApplyDamageSpecKey fallback lookup 구성

3. Component / UCReaction 책임 분리
- UCReactionComponent는 reaction data resolve / active reaction 관리
- UCReaction은 montage lifecycle과 local policy hook 담당

4. Hit / Dead Reaction 최소 정책 구현
- HitReaction / DeadReaction 실행
- DeadReaction 우선순위와 interrupt / cancel 정책 분리
```

---
## 변경 범위

### Reaction Execution Pipeline

#### A. TakeDamage 이후 Reaction Request 연결

- `UCTakeDamageComponent`가 damage commit 이후 `FTakeDamageResult`를 만들고, 해당 result를 `UCReactionComponent`에 전달하도록 연결했다.

**Flow**
```yaml
UCTakeDamageComponent
-> CommitTakeDamage
-> BuildResult
-> DispatchTakeDamageCommitted
-> UCReactionComponent::RequestReaction
```

**Structure**
```yaml
FTakeDamageResult
- bAccepted              : damage 처리 수락 여부
- ApplyDamageSpecKey     : damage spec 식별 key
- bKilled                : 이번 damage로 사망했는지 여부
- bTriggerHitReaction    : hit reaction 요청 여부
- bTriggerDeathReaction  : dead reaction 요청 여부
```

#### B. Reaction Type Resolve

- `FTakeDamageResult`의 dispatch flag를 기준으로 실행할 reaction type을 결정했다.

**Flow**
```yaml
FTakeDamageResult
-> ValidateRequest
-> ResolveReactionType
-> EReactionType::Hit / Dead / None
```

**Rule**
```yaml
ResolveReactionType
- bTriggerDeathReaction == true : Dead
- bTriggerHitReaction == true   : Hit
- otherwise                     : None
```

#### C. Reaction Data Resolve

- `ApplyDamageSpecKey + ReactionType` 조합으로 reaction data를 조회하고, exact key가 없을 경우 fallback key를 순서대로 탐색하도록 구성했다.

**Flow**
```yaml
ApplyDamageSpecKey
-> BuildCandidateSpecKeys
-> FReactionDataKey 구성
-> ReactionDataMap lookup
-> FReactionData resolve
```

**Structure**
```yaml
FReactionDataKey
- ApplyDamageSpecKey : damage spec 기준 key
- ReactionType       : Hit / Dead

FReactionData
- ReactionDataKey     : reaction data lookup key
- ReactionExecutorKey : 실행할 UCReaction class
- Montage             : 재생할 reaction montage
- PlayRate            : montage play rate
- bCanMove            : reaction 중 movement 허용 여부
```

**Fallback Lookup**
```yaml
1. Exact Key
2. Index Any
3. Action + Index Any
4. Equipment + Action + Index Any
```

#### D. UCReaction Executor Resolve / Cache

- `FReactionData`가 지정한 `ReactionExecutorKey`를 기준으로 UCReaction executor를 찾거나 생성하고, 이후 재사용할 수 있도록 cache함.

**Flow**
```yaml
FReactionData
-> ResolveReaction
-> FindReaction
-> 없으면 CreateReaction
-> ReactionExecutorMap cache
```

**Structure**
```yaml
UCReactionComponent
- ReactionDatas        : editor에서 설정하는 reaction data list
- ReactionClasses      : editor에서 설정하는 reaction executor class list
- ReactionDataMap      : FReactionDataKey -> FReactionData
- ReactionExecutorMap  : UClass -> UCReaction instance
```

#### E. Active Reaction 교체 판단

- active reaction이 없으면 바로 재생하고, active reaction이 있으면 active / incoming reaction의 정책 hook을 통해 교체 가능 여부를 판단했다.

**Flow**
```yaml
Play Case
- active reaction 없음
- PlayReaction

Replace Case
- active reaction 있음
- QueryAcceptNewReaction
- Stop active reaction
- Play incoming reaction
```

**Rule**
```yaml
QueryAcceptNewReaction
- ActiveReaction::AllowInterruptionBy(query)
- NewReaction::WantToInterrupt(query)
```

#### F. UCReaction Executor Lifecycle

- `UCReaction`은 montage 기반 reaction executor로 동작하고, validate / initialize / begin / stop / end 흐름을 담당한다.

**Flow**
```yaml
PlayReaction
-> UCReaction::Validate
-> UCReaction::Initialize
-> UCReaction::Begin
	-> reaction montage 재생
	-> montage end delegate binding
-> OnMontageEnd
-> UCReaction::End
```

**Structure**
```yaml
UCReaction
- Validate   : reaction data와 owner 유효성 확인
- Initialize : runtime flag / active montage cache 초기화
- Begin      : reaction montage 재생 및 montage end delegate binding
- Stop       : interrupt / cancel stop reason을 받아 active montage 중단 요청
- End        : montage 종료 이후 runtime cleanup
```

#### G. Reaction State / Movement 적용

- reaction 실행 중 movement와 character state를 조정하고, reaction 종료 후 복구하도록 구성했다.

**Flow**
```yaml
PlayReaction
-> UpdateMovementToImmovable
-> UpdateStateToReaction
-> UCReaction::Begin

EndReaction
-> RestoreMovementToMovable
-> RestoreStateToIdle
-> ClearActiveReaction
```

#### H. Reaction Window Notify 연결

- montage notify state를 통해 active reaction의 interrupt / cancel 가능 window를 열고 닫도록 구성했다.

**Flow**
```yaml
UCAnimNotifyState_Reaction::NotifyBegin
-> UCReactionComponent::OnReactionWindowBegin
-> ActiveReaction SetInterruptible / SetCancelable

UCAnimNotifyState_Reaction::NotifyEnd
-> UCReactionComponent::OnReactionWindowEnd
-> ActiveReaction window flag reset
```

**Structure**
```yaml
EReactionWindowType
- Interruptible : incoming reaction에 의해 중단될 수 있는 window
- Cancelable    : cancel 요청을 받을 수 있는 window
```

#### I. Hit / Dead Reaction 정책

- 최소 구현 범위로 `HitReaction`과 `DeadReaction`을 구현하고, dead reaction이 최상위 우선순위를 갖도록 구성했다.

**Rule**
```yaml
HitReaction
- DeadReaction에 의한 interrupt 허용
- 그 외 interruption은 Interruptible window 기준
- cancel은 Cancelable window 기준

DeadReaction
- active HitReaction을 interrupt할 수 있음
- 다른 reaction에 의해 interrupted / canceled 되지 않음
```

---
## 주요 Pipeline

### Reaction Execution Request Pipeline

```yaml
TakeDamage Commit
-> FTakeDamageResult
-> UCReactionComponent::RequestReaction
-> ProcessReaction
-> ResolveReactionType
-> ResolveReactionData
-> ResolveReaction
```

### Reaction Execution Play Pipeline

```yaml
Incoming Reaction
-> active reaction 없음
-> PlayReaction
-> Validate / Initialize / Begin
-> ChangeActiveReaction
```

### Reaction Execution Replace Pipeline

```yaml
Incoming Reaction
-> active reaction 있음
-> QueryAcceptNewReaction
-> ActiveReaction Stop
-> IncomingReaction PlayReaction
-> ChangeActiveReaction
```

### Reaction Execution Window Pipeline

```yaml
Reaction Montage NotifyState
-> OnReactionWindowBegin / End
-> ActiveReaction window flag update
-> AllowInterruptionBy / AllowCancelBy 판단에 사용
```

---
## 테스트 방법

### Reaction Request

- `UCTakeDamageComponent`가 accepted damage 이후 `UCReactionComponent::RequestReaction()`을 호출하는지 확인
- `FTakeDamageResult`의 `bTriggerHitReaction / bTriggerDeathReaction`에 따라 Hit / Dead reaction type이 resolve되는지 확인

### Reaction Data / UCReaction Executor

- `ReactionDatas`, `ReactionClasses`가 `UCReactionComponent`에 설정되어 있는지 확인
- `ApplyDamageSpecKey + ReactionType` 기준으로 reaction data가 resolve되는지 확인
- exact key가 없을 때 fallback lookup이 정상 동작하는지 확인
- UCReaction executor가 생성되고 cache 재사용되는지 확인

### Reaction Lifecycle

- HitReaction montage가 재생되고 종료 후 movement / state가 복구되는지 확인
- active reaction 중 새 reaction이 들어왔을 때 interrupt 가능 여부에 따라 교체되거나 reject되는지 확인
- montage end callback이 active reaction 기준으로 처리되는지 확인

### Hit / Dead Policy

- 일반 hit에서 HitReaction이 실행되는지 확인
- 사망 hit에서 DeadReaction이 실행되고 HitReaction보다 우선되는지 확인
- DeadReaction이 interrupt / cancel되지 않는지 확인

### Reaction Window

- `Interruptible / Cancelable` notify state 구간에서 active reaction flag가 정상 변경되는지 확인
- window flag가 `AllowInterruptionBy / AllowCancelBy` 판단에 반영되는지 확인

---
## 검증 결과

- `TakeDamage -> UCReactionComponent` request 연결 확인
- HitReaction / DeadReaction type resolve 확인
- `FReactionDataKey` 기반 reaction data lookup 확인
- UCReaction executor 생성 및 cache 재사용 확인
- reaction montage 재생 / 종료 / state 복구 확인
- reaction window 기반 interrupt / cancel 정책 반영 확인

---
## 관련 문서

- Issue Checklist: `D10_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 damage 처리 결과를 직접 montage 실행으로 연결하지 않고, `UCReactionComponent`가 reaction 선택과 실행을 담당하는 별도 reaction execution pipeline을 구성한 것이다.

변경 후 책임은 다음과 같이 정리됐다.

```yaml
UCTakeDamageComponent
- damage commit
- FTakeDamageResult 생성

UCReactionComponent
- Hit / Dead reaction 선택
- active reaction play / replace 관리

UCReaction
- montage lifecycle
- interrupt / cancel window policy
```

이를 통해 damage 계산과 reaction 실행 책임을 분리하고, 이후 Player combat receiver / AI HitReact / Reaction orchestration으로 확장할 기반을 마련했다.

---
