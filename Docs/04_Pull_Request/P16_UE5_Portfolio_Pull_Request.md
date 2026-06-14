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

이번 PR에서는 **Player와 AI가 피격 리액션을 서로 다른 방식으로 처리하던 흐름을 하나의 공통 절차로 모았다.**

이를 통해 피격 이후 어떤 리액션을 실행할지 판단하고, 실행 상태를 바꾸고, 애니메이션과 피드백을 처리하는 책임이 서로 섞이지 않도록 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Reaction Request 진입점 단일화**: 피격 이후 발생한 리액션 요청이 Player Tick이나 AI BehaviorTree를 거치지 않고, 공통 요청 진입점으로 들어가도록 구성했다.

- **공통 Reaction Execution 경로 구성**: Player와 Enemy가 같은 리액션 실행 경로를 사용하도록 요청 판단, 상태 적용, 실제 실행 단계를 연결했다.

- **DamageFeedback / ReactionFeedback 분리**: 피격 순간의 hit feedback과 리액션 애니메이션 타이밍에 맞춘 feedback을 서로 다른 기준으로 처리하도록 나눴다.

- **Hit 위치 feedback 보정**: overlap 기반 타격에서도 hit VFX / SFX 위치를 더 안정적으로 정할 수 있도록 damage impact 정보를 전달했다.

### Refactoring

- **Pending Reaction Consume 제거**: Player Tick과 AI BehaviorTree가 pending reaction을 직접 소비하던 흐름을 제거하고, 리액션 요청 시점에 실행 여부를 판단하도록 정리했다.

- **Reaction Runtime 책임 분리**: 리액션 요청 판단, 실행 상태 적용, montage lifecycle 처리를 각각 다른 책임으로 나눴다.

- **AI Reaction 역할 축소**: AI는 리액션을 직접 실행하지 않고, 현재 실행 중인 리액션 상태를 관찰하는 역할로 줄였다.

- **Reaction Notify 책임 분리**: action notify와 reaction notify를 분리하고, 리액션 제어용 notify와 feedback 실행용 notify의 역할을 나눴다.

---

## 핵심 개념

아래 용어는 이후 설명을 읽기 위한 최소 용어다.

이 PR의 핵심 흐름은 피격 이후 만들어진 요청을 공통 조율 계층이 판단하고, 실행 상태 관리 계층이 그 결과를 적용한 뒤, 실제 리액션 실행 객체가 애니메이션과 notify 기반 처리를 담당하는 구조다.

```text
Reaction Request(리액션 요청)
-> 피격 이후 실행할 리액션을 요청하는 공통 요청 단위
-> 이 PR에서는 Player와 Enemy가 같은 리액션 실행 진입점으로 들어가기 위한 요청 단위로 사용됨
```

```text
Reaction Orchestrator(리액션 요청 조율자)
-> 리액션 요청을 받아 어떤 리액션을 실행할지 판단하고 실행 결과를 만드는 중간 계층
-> 피격 결과를 Hit / Dead 리액션 후보로 해석하고, 현재 실행 상태와의 관계를 기준으로 실행 방식을 결정함
-> 코드에서는 `UCReactionOrchestratorComponent`가 이 역할을 담당함
```

```text
Reaction Component(리액션 실행 상태 관리자)
-> 현재 실행 중인 리액션 상태를 소유하고, 조율 계층이 만든 실행 결과를 적용하는 component
-> 코드에서는 `UCReactionComponent`가 이 역할을 담당함
```

```text
Reaction(리액션 실행 객체)
-> hit reaction이나 dead reaction 같은 실제 리액션을 montage 기반으로 실행하는 객체
-> montage lifecycle, control window, notify 기반 feedback 실행을 담당함
-> 코드에서는 `UCReaction`과 reaction executor가 이 역할을 담당함
```

```text
Reaction Request Result(리액션 요청 처리 결과)
-> Reaction Request를 보낸 외부 호출자에게 반환되는 요청 처리 결과
-> 코드에서는 `FReactionRequestResult`가 이 역할을 담당함
```

```text
Reaction Execution Result(리액션 실행 적용 결과)
-> Reaction Component가 active reaction state에 적용할 실행 결과
-> 코드에서는 `FReactionExecutionResult`가 이 역할을 담당함
```

```text
DamageFeedback(피격 순간 feedback)
-> damage event와 hit 위치 정보를 기준으로 구성되는 feedback
-> hit VFX, hit SFX, hit stop, camera shake request 같은 피격 순간 표현을 담당함
```

```text
ReactionFeedback(리액션 실행 feedback)
-> reaction executor의 montage timing과 lifecycle event를 기준으로 실행되는 feedback
-> 리액션 애니메이션 중 특정 타이밍에 맞춘 VFX / SFX 실행을 담당함
```

```text
Damage Impact Info(피격 위치 정보)
-> hit feedback 위치와 방향을 정하기 위해 damage pipeline으로 전달하는 impact metadata
-> 코드에서는 `FDamageImpactInfo`가 이 역할을 담당함
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 구조의 문제를 정리한다.

### Player / AI reaction 실행 비대칭 해소 필요성

기존 구조에서는 피격 이후 리액션을 실행하는 경로가 Player와 Enemy에서 다르게 나뉘어 있었다.

Player는 Tick 기반 pending reaction 소비 흐름에 의존했고, Enemy는 BehaviorTree에서 pending reaction을 확인하고 실행하는 구조에 가까웠다.

그 결과 같은 reaction이라도 실행 진입점, 상태 적용 시점, 종료 관찰 방식이 서로 달라질 수 있었다.

### Reaction 실행 책임 분리 필요성

기존 흐름에서는 대기 중인 리액션을 저장하는 일, 실행 가능 여부를 판단하는 일, 현재 실행 상태를 바꾸는 일, 실제 애니메이션을 재생하고 끝내는 일이 한 흐름에 섞여 있었다.

리액션 요청을 해석하는 책임과 현재 실행 상태를 바꾸는 책임, 실제 montage를 재생하고 종료시키는 책임을 분리할 필요가 있었다.

### AI Reaction 역할 축소 필요성

AI는 리액션을 직접 실행하기보다, 현재 리액션이 실행 중인지와 종료됐는지를 관찰하는 편이 적합했다.

BehaviorTree가 pending reaction consume까지 담당하면 AI 판단 계층이 reaction system의 실행 책임까지 떠안게 된다.

따라서 AI는 active reaction state를 관찰하고, 실제 reaction 실행은 공통 reaction pipeline에 맡기도록 정리할 필요가 있었다.

### Feedback 책임 분리 필요성

피격 상황에서는 hit VFX / SFX, hit stop, camera shake, reaction montage timing feedback이 함께 발생할 수 있다.

하지만 이 표현들은 실행 기준이 서로 다르다.

피격 순간 feedback은 damage event와 hit 위치 정보를 기준으로 실행되어야 하고, reaction feedback은 reaction montage timing을 기준으로 실행되어야 했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. 리액션 요청 진입점 단일화

- **왜**:
  Player Tick과 Enemy BehaviorTree가 pending reaction을 직접 소비하면, 같은 피격 상황에서도 Player와 Enemy의 리액션 실행 절차가 달라질 수 있었다.

- **어떻게**:
  `TakeDamage` 이후 피격 결과를 Reaction Request(리액션 요청)로 만들고, `UCReactionOrchestratorComponent`(Reaction Orchestrator)로 전달하도록 정리했다.

- **결과**:
  Player와 Enemy는 같은 reaction request 진입점을 사용하고, 리액션 실행 여부는 공통 orchestration 흐름에서 판단된다.

### 2. 리액션 판단과 결과 구조 구성

- **왜**:
  피격 결과를 어떤 리액션으로 해석할지, 기존 리액션과 새 리액션을 어떻게 조율할지 판단하는 위치가 명확해야 했다.

- **어떻게**:
  Reaction Orchestrator가 피격 결과를 Hit / Dead reaction 후보로 해석하고, active reaction과 incoming reaction의 관계를 기준으로 실행 결정을 만들도록 구성했다. 외부 반환용 Reaction Request Result와 component 소비용 Reaction Execution Result도 분리했다.

- **결과**:
  reaction request는 `요청 해석 -> 후보 결정 -> 실행 결정 -> component 전달 -> 외부 결과 반환` 순서로 읽히며, 판단 책임과 결과 적용 책임이 분리된다.

### 3. 리액션 실행 상태와 실행 객체 책임 분리

- **왜**:
  pending reaction context와 active reaction state가 섞이면, 리액션 실행 중단이나 종료 이후 상태 정리 기준이 흐려질 수 있었다.

- **어떻게**:
  pending consume 흐름을 제거하고, Reaction Component가 active reaction state를 소유하며 Reaction Execution Result를 적용하도록 정리했다. 실제 리액션 재생은 `UCReaction`(Reaction)이 montage lifecycle 기준으로 처리하도록 분리했다.

- **결과**:
  Reaction Component는 active reaction 시작, 중단, 종료, runtime cleanup을 담당하고, Reaction은 실제 montage 재생과 notify 기반 feedback 실행을 담당한다.

### 4. AI Reaction 관찰 구조 전환

- **왜**:
  BehaviorTree가 pending reaction을 직접 consume하면 AI 판단 계층과 reaction 실행 계층의 경계가 흐려졌다.

- **어떻게**:
  AI pending reaction consume 흐름을 제거하고, BehaviorTree는 active reaction state와 종료 여부를 관찰하도록 역할을 줄였다.

- **결과**:
  AI는 reaction 실행을 직접 소유하지 않고, 현재 reaction 상태를 기준으로 다음 판단을 이어갈 수 있다.

### 5. DamageFeedback / ReactionFeedback 책임 분리

- **왜**:
  hit VFX / SFX와 reaction montage timing feedback은 같은 피격 상황에서 발생하지만, 실행 기준이 서로 달랐다.

- **어떻게**:
  DamageFeedback(피격 순간 feedback)은 damage event와 impact metadata를 기준으로 hit feedback과 local feedback request를 구성한다.
  ReactionFeedback(리액션 실행 feedback)은 reaction executor의 timing과 trigger key를 기준으로 실행하도록 분리했다.
  hit detection 계층에서 `FDamageImpactInfo`(Damage Impact Info)를 만들고, ApplyDamage / TakeDamage pipeline을 통해 DamageFeedback으로 전달하도록 구성했다.

- **결과**:
  피격 순간 표현과 리액션 애니메이션 타이밍 표현이 서로 다른 context로 관리된다. DamageFeedback은 Damage Impact Info를 기준으로 hit VFX / SFX 위치와 방향을 결정할 수 있다.

### 6. Reaction Notify 책임 분리

- **왜**:
  action notify와 reaction notify가 같은 base와 trigger field를 공유하면, action 전용 이벤트와 reaction 전용 이벤트의 의미가 섞일 수 있었다.

- **어떻게**:
  action notify base와 reaction notify base를 분리하고, reaction control notify와 reaction feedback notify를 별도 역할로 나눴다.

- **결과**:
  reaction notify는 active Reaction을 통해 control window와 feedback request를 처리하고, action notify와 불필요한 trigger field를 공유하지 않는다.

---

## 주요 처리 흐름

이 섹션은 주요 실행 순서와 분기 기준을 코드 구현 전에 흐름으로 먼저 설명한다.

### Reaction Request 흐름

```text
TakeDamage 발생
-> Reaction Request 생성
-> Reaction Orchestrator 진입
-> Hit / Dead reaction 후보 결정
-> 현재 reaction과 새 reaction의 관계 확인
-> Reaction Execution Result 생성
-> Reaction Component에 실행 결과 전달
-> Reaction Request Result 반환
```

이 흐름은 피격 이후 만들어진 리액션 요청이 공통 진입점에서 해석되고, 현재 리액션 상태와의 관계를 기준으로 실행 여부가 결정되는 과정을 의미한다.

### Reaction Execution 흐름

```text
Reaction Component가 Reaction Execution Result 수신
-> 현재 reaction 상태 확인
-> 적용 가능?
   - No  -> 실행 적용 실패
   - Yes -> active reaction context 설정
         -> Reaction executor start
         -> montage / notify 처리
         -> completed / interrupted / ignored 처리
         -> active reaction state cleanup
```

이 흐름은 Reaction Component가 실행 상태를 적용하고, Reaction이 실제 montage lifecycle을 수행한 뒤 runtime state를 정리하는 과정을 의미한다.

### AI Reaction 상태 관찰 흐름

```text
Enemy 피격
-> Reaction Orchestrator가 reaction 실행 판단
-> Reaction Component가 active reaction state 소유
-> BehaviorTree는 active reaction state 관찰
-> reaction 종료 확인
-> 다음 AI 판단으로 복귀
```

이 흐름은 AI가 리액션을 직접 실행하지 않고, 공통 reaction pipeline에서 관리되는 active reaction 상태를 관찰하는 과정을 의미한다.

### Feedback 분리 흐름

```text
DamageFeedback
-> damage event 수신
-> Damage Impact Info 확인
-> hit VFX / SFX / hit stop 실행
-> camera shake request 구성 / 전달

ReactionFeedback
-> reaction notify 수신
-> active Reaction context 확인
-> montage timing 기준 feedback 실행
```

이 흐름은 피격 순간 표현과 리액션 애니메이션 타이밍 표현을 서로 다른 기준으로 처리하는 과정을 의미한다.

---

## 구현 결과

이 섹션은 변경 이후 시스템이 어떤 동작으로 정리됐는지 요약한다.

- Player와 Enemy는 `TakeDamage` 이후 같은 Reaction Request 진입점과 Reaction Orchestrator 판단 흐름을 사용한다.

- Reaction Orchestrator는 피격 결과를 기반으로 Hit / Dead reaction 후보를 만들고, active reaction과 incoming reaction의 관계를 기준으로 실행 결정을 만든다.

- Reaction Component는 active reaction state를 소유하고, Reaction Execution Result를 적용하며, reaction 시작 / 중단 / 종료 이후 runtime state를 정리한다.

- Reaction은 montage lifecycle, control window, notify 기반 feedback 실행을 담당하는 실행 객체로 정리됐다.

- BehaviorTree는 pending reaction을 직접 consume하지 않고, active reaction state를 관찰하는 역할로 축소됐다.

- DamageFeedback은 damage event와 impact metadata를 기준으로 hit feedback과 local feedback request를 구성하고, ReactionFeedback은 reaction montage timing을 기준으로 feedback을 실행한다.

- Damage Impact Info는 hit detection 계층에서 생성되어 damage pipeline을 통해 전달되며, ApplyDamage / TakeDamage 계층이 collision 위치를 다시 계산하지 않도록 유지한다.

---

## 테스트 방법

### Reaction 실행

- Player 피격 시 `TakeDamage -> UCReactionOrchestratorComponent -> UCReactionComponent -> UCReaction` 경로로 hit reaction이 실행되는지 확인했다.

- Enemy 피격 시 BehaviorTree pending consume 없이 reaction이 실행되는지 확인했다.

- hit reaction 중 더 높은 우선순위 reaction 또는 dead reaction이 들어왔을 때 interrupt decision이 정상 동작하는지 확인했다.

- reaction 완료 / interrupt / ignore 이후 active reaction state가 정상 정리되는지 확인했다.

### AI 관찰 구조

- BehaviorTree가 reaction을 직접 실행하지 않고 active reaction state를 관찰하는지 확인했다.

- reaction 중 movement / action state가 어긋나지 않는지 확인했다.

### Feedback

- reaction feedback point notify와 window notify가 active Reaction을 통해 실행되는지 확인했다.

- DamageFeedback이 Damage Impact Info 기반 위치에서 hit VFX / SFX를 재생하는지 확인했다.

- `bFromSweep == false` 상황에서 closest point fallback으로 hit VFX 위치가 계산되는지 확인했다.

### Asset / build

- action notify와 reaction notify가 불필요한 trigger field를 공유하지 않는지 확인했다.

- Player / Enemy blueprint와 montage asset이 정상 load되는지 확인했다.

- `PortfolioEditor Win64 Development` 빌드가 성공하는지 확인했다.

---

## 검증 결과

- `git diff --check`가 통과했다.

- `PortfolioEditor Win64 Development` 빌드가 통과했다.

- Player / Enemy 피격 visual feedback이 정상 동작하는 것을 확인했다.

- damage impact 기반 hit VFX 위치가 정상 동작하는 것을 확인했다.

- Player와 Enemy가 같은 reaction request 진입점과 active reaction state 기준을 사용하는 것을 확인했다.

- DamageFeedback과 ReactionFeedback이 서로 다른 기준으로 실행되는 것을 확인했다.

---

## 관련 문서

- Issue Checklist: `D17_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

P16은 피격 이후 reaction 실행을 pending consume model에서 Reaction Orchestrator 기반 공통 execution pipeline으로 전환하고, reaction request 판단 / active reaction state 적용 / montage lifecycle 실행 / feedback timing 처리를 분리한 PR이다.

DamageFeedback과 ReactionFeedback을 분리하고 Damage Impact Info를 전달해, 피격 순간 feedback과 reaction execution feedback을 서로 다른 기준으로 처리할 수 있게 했다.
