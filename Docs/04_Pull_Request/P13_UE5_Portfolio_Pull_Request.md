# UE5 Portfolio Pull Request

## 제목

**P13: Player Combat Loop 1사이클 안정화**

## 날짜

**2026.04.07**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/player-combat-loop`

---

## 요약

이번 PR에서는 **Player가 공격 입력을 시작한 뒤 타격 판정, damage 적용, 공격 종료, 상태 복귀까지 한 번의 전투 흐름으로 이어지도록 정리했다.**

이를 통해 공격 중 입력 타이밍, 충돌 판정, damage 전달, 콤보 연결이 서로 끊기지 않고 한 사이클 안에서 동작하도록 안정화했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Player 공격 1사이클 연결**: 무기 장착 상태에서 공격을 시작하고, 공격 종료 후 다시 대기 상태로 돌아오는 흐름을 구성했다.

- **콤보 선입력 연결**: 콤보 입력 가능 구간 안에서만 다음 공격을 예약하고, 마지막 공격 이후에는 다시 첫 공격부터 시작하도록 연결했다.

- **타격 유효 구간 damage 연결**: 충돌이 열려 있는 구간에서만 타격을 처리하고, 해당 타격이 damage 적용 흐름으로 이어지도록 구성했다.

### Refactoring

- **Action 상태 정리 기준 보강**: 공격 시작과 종료 시 action 상태, action index, attachment context가 함께 정리되도록 흐름을 닫았다.

- **중복 타격 방지**: 같은 충돌 구간에서 같은 대상에게 damage가 반복 적용되지 않도록 타격 기록을 정리했다.

---

## 핵심 개념

이 섹션은 이후 설명에서 반복되는 최소 용어를 먼저 정리한다.

이 PR의 핵심 흐름은 Player 공격 입력을 ComboAttack으로 실행하고, animation notify가 열어준 hit window 안에서 발생한 overlap을 damage 처리로 연결하는 구조다.

```text
ComboAttack(콤보 공격)
-> Player의 기본 연속 공격 action
-> 이 PR에서는 1타, 2타, 3타가 선입력 구간을 기준으로 이어짐
```

```text
PreInput(선입력)
-> 현재 공격이 끝나기 전에 다음 공격 입력을 미리 받아 두는 구간
-> 이 구간 안에서 들어온 입력만 다음 combo 단계 예약으로 처리함
```

```text
Hit Window(타격 유효 구간)
-> weapon collision이 켜져 실제 hit를 처리할 수 있는 구간
-> 이 구간이 열려 있을 때만 overlap을 damage 적용 후보로 사용함
```

```text
ApplyDamage / TakeDamage(damage 전달 / 수신)
-> 공격자 쪽에서 damage 요청을 만들고, 피격자 쪽에서 실제 HP 감소를 처리하는 흐름
-> 이 PR에서는 hit window에서 발생한 타격이 이 흐름으로 이어지는지 확인함
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 기존 전투 루프에서 안정화가 필요했던 지점을 정리한다.

### Player 공격 1사이클 안정화 필요성

Player 공격은 입력, action 상태 전이, montage notify, collision, damage 적용, 공격 종료가 서로 이어져야 한다.

이 중 하나라도 닫히지 않으면 공격 후 상태가 남거나, 다음 공격 입력이 의도와 다르게 처리되거나, damage 적용 타이밍을 확인하기 어려워질 수 있었다.

### ComboAttack 선입력 기준 정리 필요성

콤보 공격은 아무 때나 다음 타수로 넘어가면 안 되고, montage 안에서 허용한 입력 구간에서만 다음 공격을 예약해야 했다.

따라서 선입력 구간의 시작과 종료를 명확히 하고, 구간 밖 입력은 다음 combo 단계로 이어지지 않도록 정리할 필요가 있었다.

### Hit Window와 Damage Pipeline 연결 필요성

공격 collision은 항상 damage를 발생시키면 안 되고, animation timing으로 열린 hit window 안에서만 damage로 이어져야 했다.

또한 같은 hit window 안에서 같은 대상에게 damage가 반복 적용되지 않도록, 충돌 구간과 타격 기록을 함께 관리할 필요가 있었다.

---

## 변경 범위

이 섹션은 Player 공격 루프가 어떻게 닫혔고, 각 단계의 결과가 어떻게 달라졌는지 정리한다.

### 1. Player 공격 진입 조건 정리

- **왜**:
  공격 입력은 Player가 전투 가능한 상태일 때만 실제 action으로 이어져야 했다.
  무기 미장착, 피격, 사망 같은 상태에서 공격이 시작되면 전투 상태가 어긋날 수 있었다.

- **어떻게**:
  `ComboAttack` 실행 전에 weapon equipped 상태와 Idle 상태를 확인하고, 유효하지 않은 상태에서는 공격을 시작하지 않도록 했다.

- **결과**:
  Player는 무기를 장착하고 대기 중일 때만 기본 공격을 시작하며, 피격 / 사망 상태에서는 공격 입력이 action으로 이어지지 않는다.

### 2. ComboAttack lifecycle 정리

- **왜**:
  공격 시작 이후 action 상태와 montage, attachment context가 함께 열리고 닫혀야 공격 1사이클이 안정적으로 끝난다.

- **어떻게**:
  `ComboAttack` 시작 시 action 상태를 열고 현재 action type / action index를 attachment context로 전달했다.
  공격 종료 시 montage를 종료하고 action index, 선입력 상태, attachment context를 초기화했다.

- **결과**:
  공격은 `Idle -> Action -> Idle` 흐름으로 닫히며, 다음 공격은 남아 있는 action context 없이 다시 시작된다.

### 3. PreInput 기반 Combo Chain 연결

- **왜**:
  콤보는 입력이 빠르게 들어와도 정해진 timing 안에서만 다음 공격으로 이어져야 했다.

- **어떻게**:
  `PreInput` notify begin / end로 선입력 가능 상태를 열고 닫았다.
  해당 구간 안에서 다시 들어온 공격 입력은 즉시 실행하지 않고 다음 combo 단계 예약으로 저장했다.

- **결과**:
  Player는 선입력 구간 안에서만 `1 -> 2 -> 3` combo를 이어갈 수 있고, 마지막 공격 이후에는 다시 1타부터 시작한다.

### 4. Hit Window 기반 Collision / Damage 연결

- **왜**:
  공격 collision은 animation timing으로 열린 구간에서만 damage를 발생시켜야 했다.
  같은 구간에서 같은 대상에게 damage가 반복 적용되는 것도 막아야 했다.

- **어떻게**:
  collision begin 시 hit window id를 열고 `ApplyDamageComponent`에 hit window 시작을 알렸다.
  collision overlap이 발생하면 attachment / equipment / action context를 포함한 hit context를 만들어 damage 적용을 요청했다.
  collision end 시 hit window를 닫고 해당 window의 타격 기록을 정리했다.

- **결과**:
  damage는 열린 hit window 안의 overlap에서만 발생하며, 같은 hit window 안에서 같은 대상에게 중복 적용되지 않는다.

### 5. ApplyDamage / TakeDamage 연결 확인

- **왜**:
  Player 공격 1사이클은 hit 판정에서 끝나는 것이 아니라, 실제 대상의 HP 변화까지 이어져야 한다.

- **어떻게**:
  hit context에서 damage spec key를 만들고, 공격자 쪽 `ApplyDamage` 흐름을 거쳐 target actor의 `TakeDamage`로 damage를 전달했다.

- **결과**:
  collision hit은 `ApplyDamage -> TakeDamage` 흐름으로 연결되며, 실제 적용된 damage는 `CommittedDamage` 기준으로 확인된다.

---

## 주요 처리 흐름

이 섹션은 Player 공격 입력부터 damage 적용과 상태 복귀까지 이어지는 대표 흐름을 정리한다.

### Player Combat Loop 흐름

```text
공격 입력
-> ComboAttack 진입 조건 확인
-> action 상태 시작
-> montage 재생
-> action context를 attachment에 전달
-> hit window 열림
-> weapon overlap 발생
-> ApplyDamage 요청
-> target TakeDamage 호출
-> montage 종료
-> action 상태와 context 정리
-> Idle 복귀
```

이 흐름은 Player 공격 입력이 실제 타격과 damage 적용을 거쳐 다시 대기 상태로 돌아오는 한 번의 combat loop를 의미한다.

### Combo Chain 흐름

```text
Combo 1 실행
-> PreInput window 열림
-> 다음 공격 입력
-> 다음 combo 단계 예약
-> 현재 montage 종료
-> 예약된 Combo 2 실행
-> PreInput window 열림
-> 다음 공격 입력
-> 예약된 Combo 3 실행
-> 마지막 combo 종료
-> 다음 입력은 Combo 1부터 시작
```

이 흐름은 공격 입력을 즉시 다음 공격으로 실행하지 않고, montage가 허용한 선입력 구간에서 다음 combo 단계로 예약하는 과정을 의미한다.

### Hit Window Damage 흐름

```text
collision begin notify
-> hit window open
-> damaged target 기록 초기화
-> weapon overlap
-> hit context 생성
-> ApplyDamage 요청
-> 중복 hit 여부 확인
-> target TakeDamage 호출
-> damaged target 기록
-> collision end notify
-> hit window close
-> damaged target 기록 제거
```

이 흐름은 weapon collision이 켜진 동안 발생한 overlap만 damage 후보로 사용하고, 같은 hit window 안의 중복 타격을 막는 과정을 의미한다.

---

## 구현 결과

- Player 기본 공격은 입력, action 상태, montage notify, collision, damage 적용, 상태 복귀가 하나의 combat loop로 연결된다.

- ComboAttack은 선입력 구간 안에서만 다음 타수를 예약하고, 마지막 타수 이후에는 다시 첫 타수부터 시작한다.

- Weapon collision은 hit window 기준으로 열리고 닫히며, 열린 구간에서 발생한 overlap만 damage 적용으로 이어진다.

- 같은 hit window 안에서 같은 대상에게 damage가 중복 적용되지 않도록 타격 기록이 관리된다.

- 공격 종료 후 action index, 선입력 상태, attachment context가 정리되어 다음 공격 사이클에 이전 상태가 남지 않는다.

---

## 테스트 방법

### 공격 진입 / 종료

- 무기 장착 상태에서 기본 공격 입력 시 `Idle -> Action -> Idle` 흐름이 정상적으로 닫히는지 확인한다.

- 무기 미장착, Reaction, Dead 상태에서는 공격 입력이 action으로 이어지지 않는지 확인한다.

- 공격 종료 후 action state와 movement state가 정상 복귀하는지 확인한다.

### Combo Chain

- 선입력 구간 안에서 입력하면 `1 -> 2 -> 3` combo가 순서대로 연결되는지 확인한다.

- 선입력 구간 밖에서 입력하면 다음 combo 단계가 예약되지 않는지 확인한다.

- 마지막 combo 종료 후 다음 입력이 다시 1타부터 시작되는지 확인한다.

### Collision / Damage

- collision window 안에서만 hit와 damage 로그가 발생하는지 확인한다.

- 각 타수에서 hit window id와 action context index가 정상적으로 전달되는지 확인한다.

- 같은 hit window 안에서 같은 대상에게 damage가 중복 적용되지 않는지 확인한다.

- HP가 0에 도달한 이후 추가 hit이 추가 HP 감소로 이어지지 않는지 확인한다.

---

## 검증 결과

- Player `ComboAttack` 1사이클이 정상 동작하는 것을 확인했다.

- `Idle -> Action -> Idle` 상태 전이가 정상 동작하는 것을 확인했다.

- `1 -> 2 -> 3` combo chain이 선입력 구간 기준으로 정상 연결되는 것을 확인했다.

- 선입력 구간 밖 입력은 다음 combo 예약으로 처리되지 않는 것을 확인했다.

- notify 기반 collision timing이 정상 동작하는 것을 확인했다.

- `ApplyDamage -> TakeDamage` damage pipeline이 연결되는 것을 확인했다.

- dead / dying 이후 추가 damage 차단 동작을 확인했다.

---

## 비범위

- 공중 combo, action cancel, guard, dodge 같은 확장 전투 정책은 이번 범위에 포함하지 않는다.

- Player / AI 공통 action request 구조와 action orchestration 고도화는 후속 PR 범위로 둔다.

---

## 관련 문서

- Issue Checklist: `D14_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

P13은 Player 기본 공격이 입력부터 damage 적용과 상태 복귀까지 한 사이클로 닫히는지 안정화한 PR이다.

이후 전투 확장 작업은 이 PR에서 정리한 ComboAttack, PreInput, Hit Window, ApplyDamage / TakeDamage 연결을 기반으로 이어진다.
