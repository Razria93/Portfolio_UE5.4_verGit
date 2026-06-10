# UE5 Portfolio – Issue Checklist

## 제목

**M03-04: Player Combat Loop 1사이클 안정화**

### 날짜

- **Day 14**

- **Date : 2026.04.06**

---
### 브랜치

- feature/player-combat-loop

---
### 목표

- 플레이어의 입력부터 공격 종료까지 한 사이클을 안정화함.

- 무기 장착 상태, 액션 상태 전이, notify 기반 충돌 타이밍, 콤보 선입력 흐름을 실제 플레이 가능한 수준으로 정리함.

- 플레이어 공격 루프가 공통 전투 코어와 자연스럽게 연결되도록 정리함.

---
### TODO 리스트

#### 1. 입력 및 진입 조건 정리

- [x] `ComboAction` 입력 진입 조건 정리

- [x] 장착 상태에서만 공격 가능하도록 규칙 확인

- [x] Idle 외 상태에서 공격 제한 정책 정리

- [x] Reaction / Dead 상태에서 입력 처리 정책 정리

#### 2. Action 상태 전이 정리

- [x] `Idle -> Action -> Idle` 상태 전이 검증

- [x] `ActionComponent`와 `StateComponent` 연동 확인

- [x] 공격 종료 후 상태 복귀 확인

#### 3. Notify 기반 공격 흐름 정리

- [x] `AnimNotify_Action` begin/end 연결 검증

- [x] `AnimNotify_Collision` 충돌 on/off 검증

- [x] 공격 시작 시 attachment context 전달 확인

- [x] 공격 종료 시 attachment context 초기화 확인

#### 4. 콤보 입력 정리

- [x] `AnimNotify_PreInput` 선입력 타이밍 검증

- [x] 콤보 다음 타수 진입 검증

- [x] 마지막 타 종료 후 상태 초기화 확인

- [x] combo miss input 처리 정책 정리

#### 5. 통합 검증

- [x] 시나리오 1: 장착 후 기본 공격 시작

- [x] 시나리오 2: 충돌 윈도우에서만 타격 발생

- [x] 시나리오 3: 콤보 입력 정상 연결

- [x] 시나리오 4: 종료 후 Idle 복귀 확인

#### 6. 후속 검토 범위

- [ ] Jump 상태 공격 입력 허용 여부 검토

- [ ] action cancel 허용 여부 및 허용 시점 정책 검토

- [ ] 필요 시 콤보 디버그 로그 최소 유지 범위 정리

---
### 비고

- Jump 공격 허용 / action cancel / 디버그 로그 유지 범위는 후속 정책 검토로 분리한다.

- 본 이슈는 플레이어 공격 루프 자체를 닫는 것이 목적이며, 피격 / 사망 처리 확장은 선행 이슈에서 정리함.

- 입력 처리와 AnimNotify 연결은 이후 전투 확장 기능의 기반이 되므로 우선 안정성을 확보함.

- 플레이어 공격 루프는 `입력 -> Action -> Notify -> Collision -> ApplyDamage -> TakeDamage -> 종료` 흐름으로 1사이클이 닫히도록 정리되었음.

- 콤보는 `1 -> 2 -> 3` 연결 후 다음 입력에서 다시 `1`로 시작되도록 동작함.

- 선입력 창 내부 입력은 다음 타수 예약으로 처리되고, 선입력 창 외 입력은 무시되도록 정리되었음.

- hit window와 action context가 각 타수에 맞게 전달되며, 공통 전투 코어와 연결되도록 정리되었음.

---
