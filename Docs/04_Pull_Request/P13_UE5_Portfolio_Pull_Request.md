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

### 작업 요약

본 PR은 Player `ComboAttack`이 **Input - montage begin - notify - collision - apply / take damage - montage end - state clear** 까지 하나의 combat loop로 닫히는지 정리하고 검증한 작업이다.

### 구현 목표

Player 공격 흐름이 다음 순서로 안정적으로 이어지는 것을 목표로 한다.

```yaml
Input
-> Action
-> Notify
-> Collision
-> ApplyDamage
-> TakeDamage
-> Action End
-> Idle
```

### 확인한 기준

```yaml
1. 입력 조건
- weapon equipped 상태에서만 공격 가능
- Reaction / Dead 상태에서는 공격 입력 차단

2. Action lifecycle
- Idle -> Action -> Idle 상태 전이
- 공격 종료 후 action state 정리

3. Collision timing
- notify window에서만 hit 처리
- attachment context 전달 / 초기화

4. Combo input
- pre-input window 안에서만 다음 combo 예약
- 1 -> 2 -> 3 이후 다시 1타부터 시작

5. Damage pipeline
- ApplyDamage -> TakeDamage 연결
- dead / dying 이후 추가 damage 차단
```

---
## 변경 범위

### Player Combat Loop

#### A. 입력 및 진입 조건 정리

- `ComboAttack` 입력 진입 조건을 정리했다.

**Rule**
```yaml
Allow
- weapon equipped
- Idle state

Reject
- unequipped
- Reaction state
- Dead state
- non-combat available state
```

#### B. Action 상태 전이 정리

- 공격 시작과 종료 시 상태 전이가 정상적으로 닫히도록 확인했다.

**Flow**
```yaml
Idle
-> ComboAttack input
-> Action
-> Montage / Notify
-> Action End
-> Idle
```

#### C. Notify 기반 Collision 연결

- montage notify timing을 기준으로 collision window가 열리고 닫히도록 검증했다.

**Flow**
```yaml
AnimNotify_Action Begin
-> Action context push
-> AnimNotify_Collision Begin
-> hit detection
-> AnimNotify_Collision End
-> Action context clear
```

#### D. Combo Pre-input 정리

- pre-input window 내부 입력만 다음 combo 단계 예약으로 처리했다.

**Rule**
```yaml
Pre-input window open
- next combo input accepted
- next action index reserved

Pre-input window closed
- next combo input ignored
```

#### E. ApplyDamage / TakeDamage 연결 검증

- hit window에서 발생한 공격이 damage pipeline으로 연결되는지 확인했다.

**Flow**
```yaml
Collision Hit
-> ApplyDamage
-> FDamageEvent
-> TakeDamage
-> Health Commit
```

---
## 주요 Pipeline

### Player ComboAttack Pipeline

```yaml
Input
-> ComboAttack
-> Montage
-> Notify
-> Collision
-> ApplyDamage
-> TakeDamage
-> Action End
-> Idle
```

### Combo Chain Pipeline

```yaml
Combo 1
-> PreInput
-> Combo 2
-> PreInput
-> Combo 3
-> End
-> Reset to Combo 1
```

### Damage Pipeline

```yaml
Hit Window
-> Attachment Context
-> ApplyDamage
-> TakeDamage
-> CommittedDamage
```

---
## 테스트 방법

### Combat Loop

- 무기 장착 후 기본 공격 입력 시 `Idle -> Action -> Idle` 흐름이 정상적으로 동작하는지 확인
- 공격 종료 후 action state와 movement state가 정상 복귀하는지 확인

### Combo Chain

- 공격 중 pre-input window 안에서 입력하면 `1 -> 2 -> 3` combo가 정상적으로 연결되는지 확인
- pre-input window 밖에서 입력하면 다음 combo 예약 없이 무시되는지 확인
- 마지막 combo 종료 후 다음 입력이 다시 1타부터 시작되는지 확인

### Collision / Damage

- collision window에서만 실제 hit와 `ApplyDamage / TakeDamage` 로그가 발생하는지 확인
- 각 타수에서 hit window id와 action context index가 정상적으로 전달되는지 확인
- HP가 0에 도달한 이후 추가 hit은 `CommittedDamage = 0`으로 처리되고 추가 HP 감소가 발생하지 않는지 확인

---
## 검증 결과

- Player `ComboAttack` 1사이클 정상 동작 확인
- `Idle -> Action -> Idle` 상태 전이 정상 동작 확인
- `1 -> 2 -> 3` combo chain 정상 동작 확인
- notify 기반 collision timing 정상 동작 확인
- `ApplyDamage -> TakeDamage` pipeline 연결 정상 동작 확인
- dead / dying 이후 추가 damage 차단 동작 확인

---
## 비범위

- 공중 combo, cancel, guard, dodge 같은 확장 전투 정책은 이번 범위에 포함하지 않는다.

---
## 관련 문서

- Issue Checklist: `D14_UE5_Portfolio_Issue_Checklist.md`

---
## 정리

이 PR의 핵심은 Player `ComboAttack`이 입력부터 damage 적용과 상태 복귀까지 하나의 combat loop로 닫히는지 검증한 것이다.

변경 후 Player 공격은 input, action lifecycle, notify timing, collision window, damage pipeline이 하나의 흐름으로 연결되며, combo input도 pre-input window 기준으로 처리된다.

---
