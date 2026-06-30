# N14. Dead Destroy and Execution Cleanup Follow-up Note

## 목적

이 문서는 P31 작업 중 논의된 후속 작업 후보를 정리한다.

P31은 actor / component lifecycle teardown 기준을 정리하는 작업이다. 이 과정에서 별도 작업으로 분리해야 할 두 가지 필요성이 확인됐다.

```text
1. Dead 이후 Actor Destroy 프로세스 정리
2. Action / Reaction 상태 전환 중 gameplay runtime cleanup 순서 정책 정리
```

두 작업은 모두 cleanup과 관련되지만 성격이 다르다.

```text
Dead Destroy Flow
-> 객체 생명주기 종료와 Destroy 호출 전후의 teardown 흐름

Execution Runtime Cleanup Boundary
-> 전투 실행 중 action / reaction / overlay / feedback / weapon state가 전환될 때의 cleanup 순서
```

---

## 1. Dead Destroy Flow

### 문제 / 필요성

현재 Dead는 전투 상태와 reaction 흐름 안에서 처리된다. Dead 이후 actor가 언제, 어떤 절차로 Destroy되는지는 후속 구현 범위에서 고정한다.

Dead 이후 Destroy를 구현하려면 다음 흐름을 정책으로 정해야 한다.

```text
Dead state 진입
-> death reaction / feedback 처리
-> AI / behavior tree / perception 정지
-> combat collision / weapon actor / hit window 정리
-> character collision / mesh 상태 처리
-> delay / corpse 유지 / ragdoll / pooling 여부 결정
-> Destroy 호출
-> EndPlay teardown 검증
```

### 현재 상태와 예상되는 문제

현재 프로젝트에는 다음 기반이 있다.

```text
HealthComponent
-> Dead state event 발생

StateComponent
-> Dead execution state 반영

ReactionComponent / Reaction
-> Dead reaction 실행 가능

WeaponComponent / WeaponActor
-> spawned weapon actor cleanup 경로 보유

AIController
-> OnPossess / OnUnPossess lifecycle 보유
```

Dead 이후 Destroy flow가 추가되면 다음 문제를 검토해야 한다.

```text
- death feedback이 끝나기 전에 actor가 Destroy될 수 있음
- AI perception delegate / blackboard / behavior tree가 죽은 pawn을 계속 참조할 수 있음
- weapon collision 또는 hit window가 열린 상태로 남을 수 있음
- EndPlay cleanup과 gameplay cleanup이 중복 호출될 수 있음
- corpse 유지, delay destroy, ragdoll, pooling 중 어떤 정책을 쓸지 불명확할 수 있음
```

### 해결 방향

Dead 이후 Destroy는 별도 feature 또는 refactor branch에서 다룬다.

P31에서는 Destroy가 발생했을 때 호출될 lifecycle cleanup hook을 점검한다.

후속 Destroy 작업에서는 다음 정책을 먼저 결정한다.

```text
Destroy trigger
-> Dead state 진입 즉시
-> death reaction 종료 후
-> feedback 완료 후
-> 일정 delay 후

Destroy target
-> character actor 전체 Destroy
-> corpse 유지 후 component 비활성화
-> pooling 후보 유지

Cleanup ordering
-> gameplay 종료 cleanup
-> AI / perception 중지
-> collision 비활성화
-> weapon actor 정리
-> feedback / VFX 종료 또는 분리
-> actor Destroy
```

### 작업 프로세스 제안

```text
1. Dead state 진입 경로 전수 확인
2. death reaction / feedback 완료 시점 확인
3. AIController / BehaviorTree / Blackboard 종료 기준 확인
4. collision / weapon / combat signal cleanup 순서 설계
5. Destroy 시점 정책 결정
6. EndPlay teardown과 중복 cleanup 여부 검증
7. PIE에서 Dead -> cleanup -> Destroy smoke test
```

### 목표

```text
- Dead 이후 actor 종료 흐름을 명확히 한다.
- Destroy 호출 전 필요한 gameplay cleanup을 정책화한다.
- Destroy 이후 EndPlay teardown이 중복 호출되어도 안전한지 확인한다.
- AI / weapon / collision / feedback이 죽은 actor를 계속 참조하지 않게 한다.
```

### 종료 조건

```text
- Dead state 진입 후 Destroy 시점이 문서와 코드에서 설명 가능하다.
- death feedback / reaction이 의도한 시점까지 보존된다.
- weapon collision / hit window / combat signal 경로가 닫힌다.
- AIController / BT / perception 쪽 stale reference가 남지 않는다.
- PIE에서 Dead 이후 crash 또는 stale collision이 재현되지 않는다.
```

### 구현 및 예상 수정 대상

```text
Source/Portfolio/Component/CHealthComponent.*
Source/Portfolio/Component/CStateComponent.*
Source/Portfolio/Component/CReactionComponent.*
Source/Portfolio/Reaction/CReaction_Dead.*
Source/Portfolio/Character/Enemy/CEnemy.*
Source/Portfolio/Controller/CAIController.*
Source/Portfolio/Component/CWeaponComponent.*
Source/Portfolio/Weapon/CWeaponActor.*
Source/Portfolio/System/Combat/*
```

후속 브랜치 후보:

```text
feature/dead-actor-destroy-flow
refactor/dead-destroy-lifecycle-flow
```

---

## 2. Execution Runtime Cleanup Boundary

### 문제 / 필요성

Action / Reaction interrupt 흐름은 이미 구현되어 있다.

overlay, feedback window, weapon collision, hit context, ResultOut, Repulse 같은 상태와 결과 전달이 늘어나면 interrupt 과정의 cleanup 순서가 더 중요해진다.

현재 cleanup은 다음 지점에 나뉘어 있다.

```text
Action / Reaction ClearRuntime
Action / Reaction CleanupRuntimeEffects
WeaponComponent ClearRuntimeWeaponState
FeedbackComponent ClearRuntimeFeedback
DefenseComponent ClearGuardState
```

이 흐름은 지금 구조에서 동작한다. 상태 간 영향이 커지면 다음 질문에 답해야 한다.

```text
cleanup 전에 어떤 데이터를 snapshot해야 하는가?
weapon collision / hit context는 언제 닫아야 하는가?
feedback은 cleanup 전 데이터로 실행하는가, cleanup 후 안정 상태에서 실행하는가?
result-out dispatch는 active context clear 전인가 후인가?
overlay는 reaction / dead / repulse 진입 시 유지하는가 지우는가?
```

### 현재 상태와 예상되는 문제

현재는 action / reaction lifecycle이 안정적으로 종료되는 것이 핵심이었다.

초기 구조에서는 overlay, external result dispatch, complex feedback dependency가 상대적으로 적었기 때문에 action / reaction이 자신의 runtime state를 정리하면 충분했다.

다음 기능이 확장되면 cleanup 순서가 결과 품질에 직접 영향을 줄 수 있다.

```text
Guard overlay
Parry / Repulse
TimingCue
CombatSignal outcome
ResultOut
Execution / Balance
Dead / Stagger priority
```

예상 문제:

```text
- ClearRuntime 이후 feedback에 필요한 active action data가 사라질 수 있음
- WeaponContext를 지운 뒤 result-out packet을 만들 수 있음
- collision close가 늦어져 이미 중단된 공격이 추가 hit를 만들 수 있음
- overlay cleanup이 reaction / dead / repulse 우선순위와 충돌할 수 있음
- finish event listener가 cleanup 전후 상태를 다르게 해석할 수 있음
```

### 해결 방향

gameplay runtime cleanup을 다음 단계로 나눠 정책화한다.

```text
Before
-> active action / reaction / overlay / weapon / feedback context snapshot

Transition
-> 종료 또는 중단 원인 확정
-> collision / hit window / feedback window 정리
-> movement / state / overlay 적용 또는 복구
-> result-out / consequence dispatch

After
-> active context clear
-> runtime cache clear
-> terminal event broadcast 또는 후처리
```

정확한 순서는 기능 사례가 생긴 뒤 확정한다.

우선 적용 시점은 Repulse v1 또는 minimal ResultOut 구현 이후가 적절하다. 이때 attacker result, target outcome, reaction / feedback / weapon cleanup 순서가 실제 요구사항으로 드러난다.

### 작업 프로세스 제안

```text
1. Action / Reaction 종료 및 interrupt 경로 전수 확인
2. ClearRuntime / CleanupRuntimeEffects 호출 순서 정리
3. Weapon / Feedback / Defense cleanup 호출 지점 목록화
4. ResultOut / Repulse 사례에서 필요한 snapshot 데이터 정의
5. cleanup 전후 상태표 작성
6. result dispatch / feedback dispatch / active context clear 순서 결정
7. 필요한 API 분리 또는 이름 보정
8. PIE에서 interrupt / parry / dead / stagger smoke test
```

### 목표

```text
- action / reaction 상태 전환 전후의 데이터 보존 기준을 명확히 한다.
- cleanup 순서를 정책으로 고정한다.
- result dispatch와 feedback dispatch가 필요한 context를 잃지 않게 한다.
- weapon collision / hit context / overlay / feedback runtime state가 의도한 시점에 정리되게 한다.
```

### 종료 조건

```text
- interrupt 전후 snapshot 기준이 문서화되어 있다.
- ClearRuntime / CleanupRuntimeEffects / ClearRuntimeWeaponState / ClearRuntimeFeedback / ClearGuardState 호출 순서가 설명 가능하다.
- ResultOut / Repulse 사례에서 필요한 데이터가 cleanup 전에 확보된다.
- cleanup 이후 stale hit window, stale overlay, stale feedback state가 남지 않는다.
- 기존 Action / Reaction interrupt smoke test가 통과한다.
```

### 구현 및 예상 수정 대상

```text
Source/Portfolio/Action/CAction.*
Source/Portfolio/Action/CAction_Guard.*
Source/Portfolio/Reaction/CReaction.*
Source/Portfolio/Component/CActionComponent.*
Source/Portfolio/Component/CReactionComponent.*
Source/Portfolio/Component/CWeaponComponent.*
Source/Portfolio/Component/CActionFeedbackComponent.*
Source/Portfolio/Component/CReactionFeedbackComponent.*
Source/Portfolio/Component/CDefenseComponent.*
Source/Portfolio/Component/CCombatSignalTargetComponent.*
Source/Portfolio/Component/CCombatSignalSourceComponent.*
Source/Portfolio/Type/*
```

후속 브랜치 후보:

```text
refactor/execution-runtime-cleanup-boundary
```

---

## P31과의 관계

P31은 다음 후속 작업에서 사용할 cleanup hook을 신뢰할 수 있게 만드는 것을 목표로 한다.

```text
P31
-> object lifecycle teardown cleanup 기준
-> BeginPlay / EndPlay / OnPossess / OnUnPossess / timer / delegate / spawned actor cleanup

Dead Destroy Flow
-> Dead 이후 actor 종료 프로세스와 Destroy 전후 cleanup 순서

Execution Runtime Cleanup Boundary
-> 전투 상태 전환 중 snapshot / cleanup / result dispatch / active context clear 순서
```

---
