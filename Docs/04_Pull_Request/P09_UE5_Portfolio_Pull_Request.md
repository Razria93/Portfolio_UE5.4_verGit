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

이번 PR에서는 **공격을 받은 캐릭터가 damage 결과에 따라 피격 또는 사망 반응을 선택하고 실행할 수 있도록 reaction 실행 흐름을 구성했다.**

이를 통해 damage 계산과 피격 반응 실행을 분리하고, 이후 Player / Enemy가 같은 reaction 실행 구조를 사용할 수 있게 했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Damage 결과 기반 reaction 연결**: damage가 실제로 적용된 뒤 결과값을 기준으로 hit reaction 또는 dead reaction을 요청하도록 연결했다.

- **Reaction data 기반 실행 선택**: 어떤 공격에서 발생한 damage인지에 따라 실행할 reaction data와 executor를 찾도록 구성했다.

- **Reaction montage 실행 흐름 구성**: 선택된 reaction이 montage를 재생하고 종료 후 movement / state를 복구하도록 실행 흐름을 연결했다.

### Refactoring

- **Damage 처리와 reaction 실행 책임 분리**: damage 결과 산출과 reaction 선택 / 실행을 서로 다른 component 책임으로 나눴다.

- **Active reaction 교체 기준 정리**: 이미 실행 중인 reaction이 있을 때 새 reaction이 들어오면 active reaction과 new reaction의 정책을 비교해 교체 여부를 판단하도록 정리했다.

- **Reaction window 제어 분리**: montage notify는 reaction 제어 구간의 flag만 열고 닫으며, 실제 교체 판단은 reaction 실행 정책이 담당하도록 구성했다.

---

## 핵심 개념

이 섹션은 이후 설명에서 반복되는 최소 용어를 먼저 정리한다.

이 PR의 핵심 흐름은 damage 처리 결과가 reaction 요청으로 넘어가고, `UCReactionComponent`가 data / executor / active state를 기준으로 실제 reaction 실행을 결정하는 구조다.

```text
Reaction Execution Pipeline(피격 반응 실행 흐름)
-> damage 결과를 기준으로 hit / dead reaction을 선택하고 montage 실행까지 이어주는 흐름
-> 이 PR에서는 TakeDamage 이후 UCReactionComponent와 UCReaction으로 이어지는 구조를 구성함
```

```text
UCReactionComponent(reaction 선택과 실행 관리 component)
-> damage 결과를 받아 reaction type, reaction data, reaction executor를 결정함
-> 현재 실행 중인 reaction과 새 reaction의 관계를 판단하고 active reaction 상태를 관리함
```

```text
UCReaction(reaction executor)
-> 실제 reaction montage lifecycle을 수행하는 실행 객체
-> validate, initialize, begin, stop, end 흐름을 담당함
```

```text
Active Reaction(현재 실행 중인 reaction)
-> 지금 캐릭터에게 적용 중인 reaction
-> 새 reaction이 들어올 때 교체 가능 여부를 판단하는 대상
```

```text
Reaction Window(reaction 제어 구간)
-> montage notify state로 열고 닫는 reaction 제어 구간
-> Interruptible / Cancelable / ImmuneToReaction 같은 runtime flag를 active reaction에 전달함
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 damage 처리 이후 reaction 실행 흐름에서 분리해야 했던 책임을 정리한다.

### Damage 처리와 reaction 실행 책임 분리 필요성

기존 구조에서 damage 수신, HP 반영, 피격 반응 실행이 한 흐름 안에 강하게 붙으면 damage 계산과 연출 실행 책임이 섞일 수 있었다.

Damage 결과 산출은 `UCTakeDamageComponent`가 담당하고, 그 결과를 보고 어떤 reaction을 실행할지는 별도 component가 담당하도록 분리할 필요가 있었다.

### Hit / Dead reaction 선택 기준 필요성

공격을 받은 뒤 항상 같은 피격 반응을 실행하면 안 되고, damage 결과가 사망으로 이어졌는지에 따라 hit reaction과 dead reaction을 구분해야 했다.

이를 위해 `FTakeDamageResult`의 accepted 여부와 reaction trigger flag를 기준으로 reaction type을 결정할 필요가 있었다.

### Reaction 실행 상태 관리 필요성

피격 반응은 montage 기반 실행이므로 시작, 중단, 종료, state 복구, movement 복구를 한 lifecycle 안에서 관리해야 했다.

또한 이미 reaction이 실행 중일 때 새 reaction이 들어오면 현재 reaction을 유지할지, 중단하고 새 reaction으로 바꿀지 판단할 기준이 필요했다.

---

## 변경 범위

이 섹션은 damage 결과가 reaction 실행으로 이어지기 위해 어떤 책임과 흐름을 구성했는지 정리한다.

### 1. TakeDamage 이후 Reaction Request 연결

- **왜**:
  Damage commit 이후 hit / dead reaction을 실행하려면 damage 처리 결과가 reaction 실행 component로 전달되어야 했다.

- **어떻게**:
  `UCTakeDamageComponent`가 accepted damage 이후 `FTakeDamageResult`를 만들고, 해당 result를 `UCReactionComponent`의 request 진입점으로 전달하도록 연결했다.

- **결과**:
  Damage 계산과 HP 반영이 끝난 뒤, 같은 결과값을 기준으로 reaction 실행 여부를 판단할 수 있게 됐다.

### 2. Reaction Type 결정

- **왜**:
  Damage 결과가 일반 피격인지 사망 피격인지에 따라 실행할 reaction이 달라져야 했다.

- **어떻게**:
  `FTakeDamageResult`의 `bAccepted`, `bTriggerHitReaction`, `bTriggerDeathReaction` 값을 기준으로 `Hit / Dead / None` reaction type을 결정했다.

- **결과**:
  일반 hit는 hit reaction으로, 사망 hit는 dead reaction으로 분리되어 실행된다.

### 3. Reaction Data 조회와 Executor 재사용

- **왜**:
  공격 종류와 reaction type에 따라 다른 montage와 실행 객체를 선택해야 했다.

- **어떻게**:
  `ApplyDamageSpecKey`와 reaction type을 조합해 reaction data를 조회하고, 정확한 key가 없으면 fallback key를 순서대로 탐색했다.
  Reaction executor는 class 기준으로 찾거나 생성한 뒤 cache에 보관해 재사용하도록 구성했다.

- **결과**:
  공격 data에 맞는 reaction montage와 executor를 선택할 수 있고, 같은 executor class는 반복 생성하지 않고 재사용된다.

### 4. Active Reaction 교체 판단

- **왜**:
  이미 reaction이 실행 중일 때 새 reaction이 들어오면, 기존 reaction을 유지할지 새 reaction으로 교체할지 결정해야 했다.

- **어떻게**:
  Active reaction의 `AllowInterruptionBy()`와 new reaction의 `WantToInterrupt()`를 함께 확인해 교체 가능 여부를 판단했다.
  교체가 허용되면 active reaction을 interrupted로 중단하고 새 reaction을 시작했다.

- **결과**:
  Reaction 교체는 단순 덮어쓰기가 아니라 active / new reaction 양쪽의 정책을 기준으로 처리된다.

### 5. UCReaction lifecycle 구성

- **왜**:
  Reaction montage 실행은 유효성 확인, runtime 초기화, montage 재생, 종료 callback, cleanup 순서가 필요했다.

- **어떻게**:
  `UCReaction`이 `Validate -> Initialize -> Begin -> Stop / MontageEnd -> End` 흐름을 담당하도록 구성했다.
  Montage 종료 callback에는 serial guard를 두어 이전 montage callback이 active reaction을 잘못 종료하지 않도록 했다.

- **결과**:
  Reaction executor가 montage 기반 실행과 종료 정리를 하나의 lifecycle로 관리한다.

### 6. Reaction 실행 중 movement / state 적용

- **왜**:
  Reaction 실행 중에는 캐릭터가 피격 상태로 전환되고, reaction data에 따라 이동 가능 여부도 달라져야 했다.

- **어떻게**:
  Reaction 시작 시 movement를 immovable 상태로 조정하고 state를 reaction으로 전환했다.
  Reaction 종료 시 movement와 state를 복구하고 active reaction 정보를 비웠다.

- **결과**:
  Reaction 실행 중 상태와 이동 제어가 reaction lifecycle에 맞춰 적용되고 복구된다.

### 7. Reaction Window Notify 연결

- **왜**:
  Reaction 교체 가능 여부와 cancelable 상태는 montage의 특정 구간에 따라 달라져야 했다.

- **어떻게**:
  `CAnimNotifyState_Reaction`이 montage 구간 시작 / 종료 시 `UCReactionComponent`에 window begin / end를 전달하도록 연결했다.
  `UCReactionComponent`는 active reaction의 interruptible / cancelable flag를 갱신했다.

- **결과**:
  Montage notify는 reaction 제어 구간을 전달하고, interrupt 판단과 cancel hook에서 사용할 runtime flag를 active reaction에 남긴다.

### 8. Hit / Dead Reaction 최소 정책 구성

- **왜**:
  1차 구현에서는 최소 범위로 hit reaction과 dead reaction의 상위 처리 기준만 확정하면 됐다.

- **어떻게**:
  Hit reaction은 dead reaction에 의해 interrupt될 수 있고, 그 외 interruption은 interruptible window를 따르도록 했다.
  Cancel 관련 hook은 cancelable flag를 기준으로 읽을 수 있게 남겼다.
  Dead reaction은 다른 reaction에 의해 interrupt / cancel되지 않도록 구성했다.

- **결과**:
  Dead reaction이 hit reaction을 중단할 수 있고, 사망 반응은 실행 중 다른 reaction으로 덮이지 않는다.

---

## 주요 처리 흐름

이 섹션은 damage 결과가 reaction data 선택, active reaction 존재 여부 확인, reaction 교체 판단, montage 실행으로 이어지는 대표 흐름을 정리한다.

### Reaction Request 흐름

```text
Damage commit
-> FTakeDamageResult 생성
-> UCReactionComponent request 진입
-> damage 결과 유효성 확인
-> reaction type 결정
-> reaction data 조회
-> reaction executor 조회 또는 생성
```

이 흐름은 damage 처리가 끝난 뒤, 실행할 reaction 후보를 찾는 과정을 의미한다.

### Reaction 실행 / 교체 흐름

```text
new reaction 준비
-> active reaction 존재 여부 확인
-> active reaction 없음?
   - Yes -> new reaction 시작
   - No  -> active reaction이 중단을 허용하는지 확인
         -> new reaction이 중단을 원하는지 확인
         -> 교체 가능?
            - No  -> 현재 reaction 유지
            - Yes -> active reaction 중단
                  -> new reaction 시작
```

이 흐름은 새 reaction이 들어왔을 때 현재 reaction을 유지할지, 중단하고 새 reaction으로 바꿀지 판단하는 과정을 의미한다.

### Reaction Lifecycle 흐름

```text
UCReaction Validate
-> runtime initialize
-> movement / state 적용
-> montage 재생
-> montage end delegate binding
-> Stop 또는 MontageEnd
-> runtime cleanup
-> movement / state 복구
-> active reaction clear
```

이 흐름은 reaction executor가 montage 실행부터 종료 정리까지 담당하는 순서를 의미한다.

### Reaction Window 흐름

```text
Reaction montage notify begin
-> active reaction window flag open
-> interrupt 판단 또는 cancel hook에서 flag 사용
-> Reaction montage notify end
-> active reaction window flag close
```

이 흐름은 montage notify가 직접 reaction을 교체하지 않고, 교체 판단에 필요한 window 상태만 전달하는 과정을 의미한다.

---

## 구현 결과

- Damage commit 이후 `FTakeDamageResult`를 기준으로 hit / dead reaction 실행 여부를 결정할 수 있다.

- Reaction data lookup과 executor cache를 통해 공격 종류별 reaction montage와 executor를 선택할 수 있다.

- Active reaction이 있는 상태에서도 active / new reaction 정책을 비교해 유지 또는 교체를 결정할 수 있다.

- Reaction montage lifecycle 안에서 movement / state 적용과 복구가 함께 처리된다.

- Hit reaction과 dead reaction의 최소 상위 처리 기준이 구성되어 dead reaction이 hit reaction을 중단할 수 있다.

---

## 테스트 방법

### Reaction Request

- Accepted damage 이후 `UCReactionComponent` request 진입점이 호출되는지 확인한다.

- `FTakeDamageResult`의 hit / dead reaction flag에 따라 `Hit / Dead / None` reaction type이 결정되는지 확인한다.

### Reaction Data / Executor

- `ReactionDatas`와 `ReactionClasses`가 `UCReactionComponent`에 설정되어 있는지 확인한다.

- `ApplyDamageSpecKey + ReactionType` 기준으로 reaction data가 조회되는지 확인한다.

- Exact key가 없을 때 fallback lookup이 순서대로 동작하는지 확인한다.

- Reaction executor가 생성되고 cache에서 재사용되는지 확인한다.

### Reaction Lifecycle

- Hit reaction montage가 재생되고 종료 후 movement / state가 복구되는지 확인한다.

- Montage end callback이 현재 active reaction 기준으로만 처리되는지 확인한다.

- Begin 실패 또는 Stop 이후 active reaction 상태가 정리되는지 확인한다.

### Active Reaction 교체

- Active reaction이 없을 때 new reaction이 바로 실행되는지 확인한다.

- Active reaction이 있을 때 interrupt 허용 여부에 따라 새 reaction이 거절되거나 교체되는지 확인한다.

- Dead reaction이 active hit reaction을 interrupt할 수 있는지 확인한다.

### Reaction Window

- `Interruptible / Cancelable / ImmuneToReaction` notify state 구간에서 active reaction flag가 변경되는지 확인한다.

- Window flag가 active reaction의 interruptible / cancelable runtime state에 반영되는지 확인한다.

---

## 검증 결과

- `UCTakeDamageComponent -> UCReactionComponent` request 연결을 확인했다.

- Hit reaction / dead reaction type resolve를 확인했다.

- `FReactionDataKey` 기반 reaction data lookup과 fallback lookup을 확인했다.

- Reaction executor 생성과 cache 재사용을 확인했다.

- Reaction montage 재생, 종료 callback, movement / state 복구를 확인했다.

- Reaction window 기반 interruptible / cancelable flag 반영을 확인했다.

- Dead reaction이 hit reaction을 중단할 수 있고, 다른 reaction에 의해 중단되지 않는 최소 정책을 확인했다.

---

## 비범위

- Shield / Absorb / Knockback / HitStop / GuardBreak는 후속 확장 범위로 남긴다.

- Reaction queue, enqueue, delayed replacement 같은 대기열 기반 처리는 이번 PR 범위에 포함하지 않는다.

- Player / AI별 통합 reaction orchestration 구조는 이후 PR에서 확장한다.

---

## 관련 문서

- Issue Checklist: `D10_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

P09는 damage 결과가 피격 / 사망 reaction 실행으로 이어지는 첫 reaction execution pipeline을 구성한 PR이다.

Damage 계산은 `UCTakeDamageComponent`가 담당하고, reaction 선택 / 교체 / montage lifecycle은 `UCReactionComponent`와 `UCReaction`이 담당하도록 책임을 분리했다.

이 구조를 통해 hit / dead reaction의 최소 실행 정책을 확정하고, 이후 Player / Enemy 공통 reaction 실행 구조와 orchestration 확장으로 이어질 수 있게 했다.
