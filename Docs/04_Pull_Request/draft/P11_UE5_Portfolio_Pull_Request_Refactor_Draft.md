# UE5 Portfolio Pull Request

## 제목

**P11: Player Combat Receiver 구축 및 전투 수신 루프 연결**

## 날짜

**2026.04.01**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/player-combat-receiver`

---

## 요약

이번 PR에서는 **Player가 Enemy 공격을 받았을 때 HP 감소, 피격 반응, 사망 상태까지 이어지는 전투 수신 흐름을 연결했다.**

이를 통해 Player도 공격자 역할뿐 아니라 damage를 받고 상태가 바뀌는 전투 대상 역할을 수행할 수 있게 정리했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Player damage 수신 흐름 연결**: Player에 damage 수신, HP 관리, 피격 반응 실행 컴포넌트를 연결하고 damage 수신 진입점을 구성했다.

- **Player 피격 반응 연결**: Enemy 공격으로 damage가 적용되면 Player가 pending reaction을 소비하고 hit reaction을 실행하도록 연결했다.

- **Player 사망 상태 연결**: HP가 0에 도달하면 dead state가 변경되고 gameplay state도 dead 상태로 동기화되도록 구성했다.

### Refactoring

- **Player 입력 차단 기준 정리**: Player가 피격 반응 중이거나 사망 상태일 때 공격, 장비 전환, 이동 계열 입력이 잘못 들어가지 않도록 최소 차단 기준을 정리했다.

- **Enemy 공격 context 전달 보강**: Enemy 공격의 action type과 action index가 Player의 damage 수신 흐름까지 전달되도록 공격 context 전달을 보완했다.

---

## 핵심 개념

이 섹션은 이후 설명에서 반복되는 최소 용어를 먼저 정리한다.

이 PR의 핵심 흐름은 Player가 Enemy 공격을 damage event로 수신한 뒤, HP 변화와 pending reaction을 통해 피격 / 사망 상태로 이어지는 구조다.

```text
Combat Receiver(전투 수신자)
-> 공격을 받아 damage, reaction, dead state를 처리할 수 있는 전투 대상
-> 이 PR에서는 Player를 combat receiver로 편입함
```

```text
TakeDamage(damage 수신 진입점)
-> Unreal damage event를 Player가 받는 진입 함수
-> 이 PR에서는 ACPlayer::TakeDamage가 TakeDamageComponent로 damage 처리를 위임함
```

```text
Pending Reaction(대기 중인 피격 반응)
-> damage 처리 이후 reaction component에 보관되는 실행 대기 reaction
-> Player tick에서 소비되어 실제 hit reaction 실행으로 이어짐
```

```text
DeadState(생존 / 사망 상태)
-> Health 기준으로 Player가 alive, dying, dead 중 어디에 있는지 나타내는 상태
-> StateComponent의 gameplay state와 동기화됨
```

```text
FHitContext(타격 정보)
-> 어떤 장비, 어떤 action, 몇 번째 attack index에서 발생한 타격인지 담는 정보
-> Enemy 공격 정보가 Player damage 수신 흐름까지 전달되는 기준으로 사용됨
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 Player 전투 수신 흐름에서 닫혀야 했던 지점을 정리한다.

### Player Combat Receiver 편입 필요성

이전 단계에서는 Player가 공격을 실행하는 쪽의 흐름이 중심이었다.

Player가 Enemy 공격을 받았을 때 damage를 수신하고 HP, reaction, dead state까지 처리하는 수신 흐름은 별도로 닫혀 있지 않았다.

### Player Reaction / Dead 흐름 연결 필요성

Player가 damage를 받으면 HP만 줄어드는 데서 끝나지 않고, 살아 있으면 hit reaction으로 이어지고 HP가 0이 되면 dead state로 전이되어야 했다.

이를 위해 damage 처리 결과와 reaction 실행, dead state 변경, gameplay state 동기화를 연결할 필요가 있었다.

### Enemy Attack Context 전달 필요성

Player가 어떤 공격에 맞았는지 판단하려면 Enemy 공격의 action type과 action index가 Player 수신 흐름까지 전달되어야 했다.

이 context가 없으면 Player damage 로그와 reaction 판단이 어떤 공격에서 발생했는지 추적하기 어렵다.

---

## 변경 범위

이 섹션은 Player가 전투 수신자로 동작하기 위해 어떤 책임을 연결했는지 정리한다.

### 1. Player 수신 component 구성

- **왜**:
  Player가 Enemy 공격을 받으려면 damage 수신, HP 관리, reaction 실행을 담당할 component가 필요했다.

- **어떻게**:
  `ACPlayer`에 `UCTakeDamageComponent`, `UCHealthComponent`, `UCReactionComponent`를 추가하고 생성자에서 초기화했다.

- **결과**:
  Player는 damage를 수신하고 HP 변화와 reaction 실행을 처리할 수 있는 combat receiver가 됐다.

### 2. Player TakeDamage 진입점 연결

- **왜**:
  Enemy 공격이 Player에게 전달될 때 Unreal damage event가 Player 내부 수신 흐름으로 들어와야 했다.

- **어떻게**:
  `ACPlayer::TakeDamage()`를 override하고, 유효한 damage 요청은 `UCTakeDamageComponent::RequestTakeDamage()`로 위임했다.
  `TakeDamageComponent`가 없을 때는 engine 기본 damage 값이 유지되는 fallback 흐름도 남겼다.

- **결과**:
  Enemy 공격은 Player의 `TakeDamage` 진입점을 거쳐 damage 수신 component로 전달된다.

### 3. Player damage / reaction loop 연결

- **왜**:
  Player가 damage를 받은 뒤 살아 있다면 hit reaction을 실행해야 했다.
  damage 처리와 reaction 실행이 분리되어 있으면 피격 후 상태 전환이 끊길 수 있었다.

- **어떻게**:
  `TakeDamageComponent`는 damage commit 이후 `ReactionComponent`에 pending reaction을 요청했다.
  Player tick에서는 pending reaction을 소비하고 실행 가능한 경우 hit reaction을 실행하도록 연결했다.

- **결과**:
  Player는 Enemy 공격을 받은 뒤 damage 결과를 기준으로 hit reaction에 진입한다.

### 4. Health / DeadState 동기화

- **왜**:
  HP가 0에 도달했을 때 health state와 gameplay state가 따로 움직이면 사망 상태 입력 차단과 animation 상태가 어긋날 수 있었다.

- **어떻게**:
  `UCHealthComponent`가 HP 변화 이후 `DeadState`를 갱신하고 `OnDeadStateChanged`를 broadcast하도록 정리했다.
  `UCStateComponent`는 해당 event를 받아 DeadState 변화에 맞춰 gameplay state를 동기화했다.

- **결과**:
  Player는 HP 기준 dead state와 gameplay state가 함께 전환되며, 사망 상태에서 추가 입력과 상태 흐름이 일관되게 처리된다.

### 5. Player 입력 차단 기준 정리

- **왜**:
  Player가 reaction 중이거나 dead 상태일 때 공격, 장비 전환, 이동 입력이 들어가면 전투 상태가 어긋날 수 있었다.

- **어떻게**:
  action 계열 입력은 `CanActionInput()`을 거쳐 alive 상태와 현재 gameplay state를 확인하도록 했다.
  movement 입력은 alive 상태를 확인하고, `StopJump`는 release 입력이므로 별도 차단 없이 유지했다.

- **결과**:
  Player는 reaction / dead 상태에서 주요 action 입력이 차단되고, 사망 상태에서는 이동 입력도 처리되지 않는다.

### 6. Enemy Attack Context 전달 보강

- **왜**:
  Player가 Enemy 공격을 수신할 때 어떤 attack type / index에서 발생한 hit인지 전달되어야 damage와 reaction 로그를 추적할 수 있었다.

- **어떻게**:
  Enemy attack task에서 attack action type과 attack index를 구성하고, weapon component를 통해 attachment hit context로 전달했다.

- **결과**:
  Enemy 공격 정보가 Player damage 수신 흐름까지 전달되어, Player가 받은 hit의 action context를 확인할 수 있게 됐다.

---

## 주요 처리 흐름

이 섹션은 Enemy 공격이 Player damage 수신과 reaction / dead 상태로 이어지는 대표 흐름을 정리한다.

### Player Damage Receive 흐름

```text
Enemy attack 실행
-> weapon hit 발생
-> Player TakeDamage 진입
-> TakeDamageComponent로 처리 위임
-> damage 요청 검증
-> HP 반영
-> damage result 생성
```

이 흐름은 Enemy 공격이 Player에게 전달된 뒤 Player 내부 damage 수신 component를 통해 HP 변화로 이어지는 과정을 의미한다.

### Player Reaction 흐름

```text
damage commit
-> pending reaction 요청
-> Player tick
-> pending reaction 존재 확인
-> pending reaction 소비
-> reaction 실행 가능 여부 확인
-> hit reaction 실행
```

이 흐름은 damage 결과로 만들어진 pending reaction이 Player tick에서 소비되어 실제 hit reaction으로 이어지는 과정을 의미한다.

### Player DeadState 흐름

```text
HP 감소
-> HP 0 도달
-> DeadState 변경
-> OnDeadStateChanged broadcast
-> StateComponent가 event 수신
-> gameplay state를 Dead로 동기화
```

이 흐름은 Health 기준 사망 상태 변화가 Player의 gameplay state로 반영되는 과정을 의미한다.

### Enemy Attack Context 흐름

```text
Enemy attack task
-> attack action type / index 결정
-> weapon component에 action context 전달
-> attachment hit context 구성
-> Player TakeDamage 흐름으로 전달
```

이 흐름은 Enemy가 어떤 공격으로 Player를 맞췄는지 추적할 수 있도록 action context를 hit context에 포함하는 과정을 의미한다.

---

## 구현 결과

- Player는 damage 수신, HP 관리, 피격 반응 실행 component를 통해 damage 수신과 hit reaction을 처리할 수 있다.

- Enemy 공격이 Player에게 전달되면 Player HP가 감소하고, 살아 있는 경우 pending reaction을 거쳐 hit reaction으로 이어진다.

- HP가 0에 도달하면 `DeadState`가 변경되고 Player gameplay state도 Dead 상태로 동기화된다.

- Reaction / Dead 상태에서는 주요 action 입력이 차단되어 전투 상태가 어긋나지 않는다.

- Enemy attack action type / index가 Player hit context까지 전달되어 damage 수신 로그와 reaction 판단을 추적할 수 있다.

---

## 테스트 방법

### Damage Receive

- Enemy가 Player를 인지하고 공격하도록 유도한다.

- 첫 타격 시 Player HP가 감소하는지 확인한다.

- `ACPlayer::TakeDamage -> UCTakeDamageComponent` 흐름으로 damage가 처리되는지 확인한다.

### Reaction

- Player 피격 시 Reaction 상태에 진입하는지 확인한다.

- Player `HitReact`가 정상 재생되는지 확인한다.

- 연속 피격 시 reaction replace / interrupt 흐름이 동작하는지 확인한다.

### Dead Lifecycle

- 누적 피격으로 HP가 0이 되면 `DeadState`가 `Alive -> Dying -> Dead` 흐름으로 전이되는지 확인한다.

- Dead 상태 진입 이후 추가 피격이 무효 처리되는지 확인한다.

### Input Blocking

- Reaction 중 공격 / 장비 전환 입력이 차단되는지 확인한다.

- Dead 상태에서 이동 / action 입력이 차단되는지 확인한다.

- `StopJump`는 release 입력으로 정상 처리되는지 확인한다.

### FHitContext

- Enemy 공격 로그에서 `FHitContext`가 Player 수신 흐름까지 전달되는지 확인한다.

- `AttachmentType`, `EquipmentType`, `ActionType`, `ActionIndex`가 유효하게 기록되는지 확인한다.

---

## 검증 결과

- Player damage receive flow가 정상 동작하는 것을 확인했다.

- Player hit reaction 진입을 확인했다.

- 연속 피격 시 reaction replace / interrupt 흐름을 확인했다.

- DeadState lifecycle과 `UCStateComponent` sync를 확인했다.

- Reaction / Dead 상태 입력 차단을 확인했다.

- Enemy attack context 전달을 확인했다.

---

## 비범위

- Player 공격 루프와 combo 입력 안정화는 이번 범위에 포함하지 않는다.

- ApplyDamage / TakeDamage 공통 damage 규칙 정리는 후속 범위로 둔다.

- 후속 PR에서 도입된 공통 action 실행 조율 구조는 이 PR 설명에 소급하지 않는다.

---

## 관련 문서

- Issue Checklist: `D12_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

P11은 Player를 전투 수신자로 편입해 Enemy 공격을 받았을 때 damage, health, reaction, dead state가 하나의 수신 루프로 이어지도록 정리한 PR이다.

이후 Player 공격 루프와 공통 damage 규칙은 이 PR에서 연결한 Player receiver 기반 위에서 확장된다.
