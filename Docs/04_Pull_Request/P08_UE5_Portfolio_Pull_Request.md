# UE5 Portfolio Pull Request

## 제목

**P08: TakeDamage Pipeline 구현**

## 날짜

**2026.01.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-take-damage`

---

## 요약

### 작업 요약

본 PR은 Unreal Engine의 `AActor::TakeDamage()` 수신 이후 damage 처리 흐름을 `ACEnemy -> UCTakeDamageComponent -> UCHealthComponent` 구조로 분리한 작업이다.

```yaml
AActor::TakeDamage
-> ACEnemy::TakeDamage
-> UCTakeDamageComponent::RequestTakeDamage
-> FDamageEvent type routing
-> Payload / Context / Result 구성
-> UCHealthComponent commit
```

### 작업 배경

Unreal Engine의 `AActor::TakeDamage()`는 actor 단위로 damage event를 수신하기 위한 표준 entry point임.

하지만 3D action combat에서는 `TakeDamage()` 이후 Damage 처리 전후로 여러 단계의 중간 과정이 필요하며 예시는 다음과 같다.

```yaml
Pre-processing
- DamageEvent type routing
- Instigator / DamageCauser resolve
- target state validation
- damage amount evaluation

Commit Damage
- health resource commit
- dead state update from committed health

Post-processing
- reaction / feedback dispatch point
```

이 처리를 actor 내부에 직접 구현하면 actor가 damage pipeline 전체를 소유하게 되어 책임이 비대해질 수 있다.

따라서 다음과 같이 책임을 분리하고자 한다.

```yaml
ACEnemy
- Unreal damage entry
- component routing

UCTakeDamageComponent
- DamageEvent type routing
- take damage pipeline
- damage result 구성

UCHealthComponent
- HP resource commit
- dead state update

Extension Point
- reaction dispatch
- feedback dispatch
```

단, 이 브랜치에서 reaction / feedback 실행은 직접 구현하지 않고, commit 이후 연결될 dispatch point만 후속 확장 지점으로 남겼다.

### 구현 방향

이를 다음 네 가지 축으로 정리했다.

```yaml
1. TakeDamage entry 분리
- ACEnemy::TakeDamage는 최소 validation과 component routing만 담당

2. DamageEvent type routing
- FDamageEvent::IsOfType 기반 DefaultDamageEvent 처리

3. Payload / Context / Result pipeline 구성
- 입력 원본, resolved context, 최종 결과를 구조체로 분리

4. Health commit 분리
- UCHealthComponent가 HP clamp / damage / heal / dead 판정 담당
```

---
## 변경 범위

### TakeDamage Pipeline

#### A. ACEnemy TakeDamage Entry 구성

- `ACEnemy::TakeDamage()`를 override하여 Unreal damage 수신 진입점을 확보하고, 실제 처리는 `UCTakeDamageComponent`에 위임하도록 구성했다.

**Flow**
```yaml
AActor::TakeDamage
-> ACEnemy::TakeDamage
-> minimal validation 수행
-> UCTakeDamageComponent::RequestTakeDamage 호출
```

**Structure**
```yaml
ACEnemy
- UCTakeDamageComponent : damage event 해석과 처리 pipeline 담당
- UCHealthComponent     : HP resource와 dead flag 관리
- UCStateComponent      : 기존 state component 유지
```

#### B. UCTakeDamageComponent Entry / DamageEvent Routing 구성

- `RequestTakeDamage()`를 외부 entry point로 두고, `FDamageEvent` type에 따라 처리 함수를 분기하도록 구성했다.

**Flow**
```yaml
RequestTakeDamage
-> ProcessTakeDamage
-> DamageEvent type 확인
-> DefaultDamageEvent이면 HandleDefaultDamageEvent로 routing
```

**Structure**
```yaml
FDefaultDamageEvent
- ApplyDamageSpecKey : apply damage 단계에서 전달된 spec key
- ApplyDamageSpec    : damage 계산에 사용된 spec
- ApplyDamageResult  : apply damage 단계의 결과
- ClassID            : EDamageEventTypeId::DefaultDamage
```

#### C. TakeDamage Payload 구성

- engine damage entry로 들어온 원본 입력과 apply damage metadata를 `FTakeDamagePayload`에 모아 보관하도록 구성했다.

**Flow**
```yaml
DamageAmount / FDefaultDamageEvent
-> BuildPayload 호출
-> FTakeDamagePayload 생성
```

**Structure**
```yaml
FTakeDamagePayload
- DamagedActor       : damage를 수신한 actor
- EventInstigator    : engine entry로 들어온 instigator
- DamageCauser       : damage causer
- ApplyDamageSpecKey : apply damage spec 식별 key
- ApplyDamageSpec    : apply damage spec
- ApplyDamageResult  : apply damage result
- RequestedDamage    : TakeDamage entry로 요청된 damage amount
```

#### D. TakeDamage Context 구성

- payload를 기반으로 실제 damage 계산에 사용할 resolved object와 pre-state snapshot을 `FTakeDamageContext`로 구성했다.

**Flow**
```yaml
FTakeDamagePayload
-> ResolveInstigatorController
-> BuildContext 호출
-> FTakeDamageContext 생성
```

**Structure**
```yaml
FTakeDamageContext
- DamagedActor       : resolved damaged actor
- Instigator         : resolved controller
- DamageCauser       : resolved damage causer
- bWasDead           : commit damage 이전 dead 여부
- HealthPoint_Before : commit damage 이전 HP
- HealthPoint_After  : commit damage 이후 HP
- RequestedDamage    : 요청 damage
- MitigatedDamage    : 방어 / 감쇠 이후 damage
- FinalTakenDamage   : 최종 수신 damage
- FinalAppliedDamage : health에 실제 commit된 damage
```

#### E. Instigator Resolve Fallback 구성

- `EventInstigator`가 없을 때도 damage source를 추적할 수 있도록 `DamageCauser` 기반 fallback을 구성했다.

**Flow**
```yaml
ResolveInstigatorController
-> EventInstigator가 유효하면 사용
-> DamageCauser::GetInstigatorController 확인 후 유효하면 사용
-> DamageCauser가 Pawn이면 Pawn Controller 확인 후 유효하면 사용
-> DamageCauser Owner의 InstigatorController 확인 후 유효하면 사용
-> DamageCauser Owner가 Pawn이면 Owner Pawn Controller 확인 후 유효하면 사용
-> 모두 실패하면 nullptr 반환
```

#### F. TakeDamage Evaluate / Commit 구성

- `HandleDefaultDamageEvent()` 내부에서 damage 처리 절차를 `Validate -> Payload -> Context -> Evaluate -> Commit` 단계로 분리했다.

- `EvaluateTakeDamage()`는 damage를 적용해도 되는지 판단하고, reject reason 또는 accepted 상태를 결정했다.

- `CommitTakeDamage()`는 accepted damage를 `UCHealthComponent`에 반영하고, 실제 적용량과 commit 이후 상태를 result에 기록했다.

- Reaction dispatch와 feedback dispatch는 commit 이후 연결될 extension point로 남겼다.

**Processing Flow**
```yaml
HandleDefaultDamageEvent
-> ValidateRequest
-> BuildPayload
-> BuildContext
-> EvaluateTakeDamage
-> CommitTakeDamage
```

**Evaluate TakeDamage**
```yaml
- RejectReason
	- InvalidTarget
	- InvalidCauser
	- InvalidInstigator
	- AlreadyDead

- Accepted
	- 위 reject reason에 해당하지 않을 때
```

**Commit TakeDamage**
```yaml
UCHealthComponent::TakeDamage
-> FinalAppliedDamage 기록
-> HealthPoint_After 기록
-> committed health 기준 dead state 갱신
-> bKilled 계산
```

#### G. TakeDamage Result 구성

- damage 처리 결과를 `FTakeDamageResult`로 정리하여 accepted / rejected 여부와 최종 damage amount를 추적할 수 있도록 구성했다.

**Structure**
```yaml
FTakeDamageResult
- bAccepted          : damage 처리 수락 여부
- RejectReason       : reject reason
- bKilled            : health commit 이후 dead 여부
- RequestDamage      : 요청 damage
- MitigatedDamage    : 방어 / 감쇠 이후 damage
- FinalTakenDamage   : 최종 수신 damage
- FinalAppliedDamage : health에 실제 반영된 damage
```

#### H. UCHealthComponent 구성

- HP resource 관리와 dead 판정을 `UCHealthComponent`로 분리했다.

**Flow**
```yaml
InitializeHealth
-> SetMaxHP
-> SetCurrentHP
-> UpdateDeadState

TakeDamage
-> HP clamp
-> CurrentHP 갱신
-> UpdateDeadState
-> PrintTakeDamageContextInfo
```

**Structure**
```yaml
UCHealthComponent
- MaxHP
- PreviousHP
- CurrentHP
- bIsDead
- TakeDamage
- TakeHeal
- UpdateDeadState
```

---
## 주요 Pipeline

### TakeDamage Entry Pipeline

```yaml
Target->TakeDamage
-> ACEnemy::TakeDamage
-> UCTakeDamageComponent::RequestTakeDamage
-> ProcessTakeDamage
-> HandleDefaultDamageEvent
```

### TakeDamage Processing Pipeline

```yaml
HandleDefaultDamageEvent
-> ValidateRequest
-> BuildPayload
-> BuildContext
-> EvaluateTakeDamage
-> CommitTakeDamage
-> Update Result
```

### Health Commit Pipeline

```yaml
FinalTakenDamage
-> UCHealthComponent::TakeDamage
-> CurrentHP clamp
-> FinalAppliedDamage
-> UpdateDeadState
-> bKilled 계산
```

---
## 테스트 방법

### TakeDamage Entry

- Enemy에 `UCTakeDamageComponent`, `UCHealthComponent`가 부착되어 있는지 확인
- 공격자의 ApplyDamage 흐름에서 target의 `TakeDamage()`가 호출되는지 확인
- `ACEnemy::TakeDamage()`가 직접 damage를 처리하지 않고 `RequestTakeDamage()`로 위임하는지 확인

### DamageEvent Routing

- `FDefaultDamageEvent`가 `FDamageEvent`를 통해 전달되는지 확인
- `DamageEvent.IsOfType(FDefaultDamageEvent::ClassID)` 분기가 정상 동작하는지 확인
- 지원하지 않는 damage event type이 들어왔을 때 DefaultDamage 처리와 구분되는지 확인

### Payload / Context / Result

- `FTakeDamagePayload`에 damaged actor, instigator, damage causer, apply damage metadata가 기록되는지 확인
- `FTakeDamageContext`에 resolved instigator와 HP snapshot이 기록되는지 확인
- invalid target / causer / instigator / already dead 상황에서 reject reason이 설정되는지 확인

### Health Commit

- `FinalTakenDamage`가 `UCHealthComponent::TakeDamage()`로 전달되는지 확인
- HP가 0 아래로 내려가지 않고 clamp되는지 확인
- HP가 0이 되었을 때 `bIsDead`가 true로 전환되는지 확인

### Debug Output

- `PrintTakeDamageSummaryInfo()`에서 object / damage amount 정보가 출력되는지 확인
- `UCHealthComponent` 로그에서 HP 변화량과 dead 상태가 확인되는지 확인

---
## 검증 결과

- `ACEnemy::TakeDamage -> UCTakeDamageComponent::RequestTakeDamage` routing 확인
- `FDefaultDamageEvent` type routing 확인
- Payload / Context / Result 기반 TakeDamage pipeline 확인
- `ResolveInstigatorController()` fallback 흐름 확인
- `UCHealthComponent` HP clamp / damage / dead flag update 확인
- TakeDamage / Health debug output 확인

---
## 관련 문서

- Issue Checklist: `D09_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 Unreal `TakeDamage()` 수신 이후의 damage 처리 책임을 actor 내부 함수에서 component pipeline으로 분리한 것이다.

변경 후 책임은 다음과 같이 정리됐다.

```yaml
ACEnemy
- Unreal TakeDamage entry
- minimal validation
- UCTakeDamageComponent routing

UCTakeDamageComponent
- FDamageEvent type routing
- Payload / Context / Result 구성
- damage validity 평가
- Health commit 요청

UCHealthComponent
- HP resource 관리
- damage / heal clamp
- dead flag update
```

이를 통해 damage event 해석, damage 결과 계산, HP resource commit을 단계별로 추적할 수 있게 되었고, 이후 Reaction Pipeline과 Combat Core Shared 구조로 확장할 기반을 마련했다.

---
