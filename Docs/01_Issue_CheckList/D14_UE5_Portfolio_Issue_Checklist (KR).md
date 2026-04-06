# UE5 Portfolio – Issue Checklist

## 제목

**M03-04: Player Combat Loop 1사이클 안정화**

### 날짜

- **Day 14**

- **Date : 2026.04.06**


---

### 목표

- 플레이어의 입력부터 공격 종료까지 한 사이클을 안정화함.

- 무기 장착 상태, 액션 상태 전이, notify 기반 충돌 타이밍, 콤보 선입력 흐름을 실제 플레이 가능한 수준으로 정리함.

- 플레이어 공격 루프가 공통 전투 코어와 자연스럽게 연결되도록 정리함.


---

### 브랜치

- `feature/player-combat-loop`


---

### TODO List

#### 1. 입력 및 진입 조건 정리

- [ ] `ComboAction` 입력 진입 조건 정리

- [ ] 장착 상태에서만 공격 가능하도록 규칙 확인

- [ ] Idle 외 상태에서 공격 제한 정책 정리

- [ ] 점프/피격/Dead 상태에서 입력 처리 정책 정리


#### 2. Action 상태 전이 정리

- [ ] `Idle -> Action -> Idle` 상태 전이 검증

- [ ] `ActionComponent`와 `StateComponent` 연동 확인

- [ ] 공격 종료 후 상태 복귀 확인

- [ ] action cancel 허용 여부 검토


#### 3. Notify 기반 공격 흐름 정리

- [ ] `AnimNotify_Action` begin/end 연결 검증

- [ ] `AnimNotify_Collision` 충돌 on/off 검증

- [ ] 공격 시작 시 attachment context 전달 확인

- [ ] 공격 종료 시 attachment context 초기화 확인


#### 4. 콤보 입력 정리

- [ ] `AnimNotify_PreInput` 선입력 타이밍 검증

- [ ] 콤보 다음 타수 진입 검증

- [ ] 마지막 타 종료 후 상태 초기화 확인

- [ ] combo miss input 처리 정책 정리


#### 5. 통합 검증

- [ ] 시나리오 1: 장착 후 기본 공격 시작

- [ ] 시나리오 2: 충돌 윈도우에서만 타격 발생

- [ ] 시나리오 3: 콤보 입력 정상 연결

- [ ] 시나리오 4: 종료 후 Idle 복귀 확인


---

### Notes

- 본 이슈는 플레이어 공격 루프 자체를 닫는 것이 목적이며, 피격/사망 처리 확장은 선행 이슈에서 정리함.

- 입력 처리와 AnimNotify 연결은 이후 전투 확장 기능의 기반이 되므로 우선 안정성을 확보함.


---
