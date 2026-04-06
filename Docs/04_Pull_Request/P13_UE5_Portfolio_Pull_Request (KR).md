# Player Combat Loop 1사이클 안정화

## 제목

`✨ feat: player-combat-loop 1사이클 안정화`

## 요약

- 본 PR은 플레이어 공격 루프를 `입력 -> Action -> Notify -> Collision -> ApplyDamage -> TakeDamage -> 종료` 흐름으로 1사이클이 닫히는지 정리 및 검증한 작업을 포함함.

- `ComboAction` 입력 조건, `Idle -> Action -> Idle` 상태 전이, notify 기반 충돌 타이밍, 콤보 선입력 흐름이 실제 플레이 가능한 수준으로 동작하는지 정리 및 확인함.

- 콤보는 `1 -> 2 -> 3` 흐름으로 연결되고, 다음 입력에서 다시 `1`타부터 시작되는 현재 동작 기준을 확인함.

- 또한 플레이어 공격 루프가 공통 전투 코어와 자연스럽게 연결되는지 `ApplyDamage -> TakeDamage` 흐름과 attachment context 전달 기준으로 함께 검증함.


---

## 완료된 항목

### 1. 입력 및 진입 조건 정리

- `ComboAction` 입력 진입 조건 정리 및 확인

- 장착 상태에서만 공격 가능하도록 구성된 규칙 확인

- Idle 외 상태에서 공격 제한 정책 정리 및 확인

- `Reaction / Dead` 상태에서 입력 차단 정책 확인

### 2. Action 상태 전이 정리

- `Idle -> Action -> Idle` 상태 전이 검증

- `ActionComponent`와 `StateComponent` 연동 확인

- 공격 종료 후 상태 복귀 확인

### 3. Notify 기반 공격 흐름 정리

- `AnimNotify_Action` begin/end 연결 검증

- `AnimNotify_Collision` 충돌 on/off 검증

- 공격 시작 시 attachment context 전달 확인

- 공격 종료 시 attachment context 초기화 확인

### 4. 콤보 입력 정리

- `AnimNotify_PreInput` 선입력 타이밍 검증

- 콤보 다음 타수 진입 검증

- 마지막 타 종료 후 상태 초기화 확인

- 선입력 창 내부 입력은 다음 타수 예약, 창 외 입력은 무시되는 현재 정책 정리 및 확인

### 5. 공통 전투 코어 연결 검증

- 각 타수별 hit window / action context 전달 확인

- `ApplyDamage -> TakeDamage` 흐름 정상 연결 확인

- dead / dying 이후 추가 타격이 `CommittedDamage = 0`으로 처리되는 현재 동작 확인


---

## 테스트 방법

1. 무기 장착 후 기본 공격 입력 시 `Idle -> Action -> Idle` 흐름이 정상적으로 동작하는지 확인

2. 공격 중 선입력 창 안에서 입력하면 `1 -> 2 -> 3` 콤보가 정상적으로 연결되는지 확인

3. 선입력 창 밖에서 입력하면 다음 타수 예약 없이 무시되는지 확인

4. collision window에서만 실제 타격과 `ApplyDamage / TakeDamage` 로그가 발생하는지 확인

5. 각 타수에서 hit window id와 action context index가 정상적으로 전달되는지 확인

6. HP가 0에 도달한 이후 추가 타격은 `CommittedDamage = 0`으로 처리되고 추가 HP 감소가 발생하지 않는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/player-combat-loop`

- 관련 작업:

  - `M03-04: Player Combat Loop 1사이클 안정화 (#37)`


---

## 노트

- 본 PR의 초점은 플레이어 공격 루프 자체를 닫는 것이며, 공중 콤보 / 액션 캔슬 / 가드 / 회피 연계 같은 확장 정책은 이번 범위에 포함하지 않음.

- 현재 기준 정책은 `선입력 창 내부 입력만 다음 타수 예약으로 처리`하는 것이며, 창 외 입력은 무시되도록 정리 및 확인함.

- 전투 수치 확장이나 추가 전투 정책은 이후 브랜치에서 이어서 확장 예정임.


---