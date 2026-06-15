# UE5 Portfolio Bug Report

## 제목

**B11: Guard In 중 Release 입력 시 Guard Hold 상태에 고정될 수 있는 문제**

## 날짜

**2026.06.15**

## 상태

- [ ] **진행중**

---

## 브랜치

- `feature/parry-action`

---

## 요약

- Guard 입력으로 `Block_In`이 실행되는 도중 key release가 들어오면 `Block_Out`이 즉시 실행되지 못하고 Guard Hold 상태가 남을 수 있다.

- 원인은 release 입력이 `Block_In` 실행 중에 들어왔을 때, 해당 종료 의도를 보관했다가 안전한 시점에 다시 처리하는 v1 경로가 없기 때문이다.

- v1에서는 Guard 전용 pending release를 두고, `Block_In` 완료 또는 exit 시점에 기존 action request 판단 경로로 다시 제출하는 방식으로 해결한다.

- deferred request를 Orchestrator 수준으로 일반화하는 구조 논의는 `N02_Guard_Release_Deferred_Request_Note.md`에서 별도로 다룬다.

---

## 영향 범위

- Guard In / Hold / Out lifecycle

- Guard release 입력 처리

- Guard pose / guard 판정 / parry 판정 runtime 상태 정리

---

## 환경

- 엔진: Unreal Engine 5.4

- 관련 브랜치:
  - `feature/parry-action`

- 관련 코드:
  - `Source/Portfolio/Action/CAction_Guard.cpp`
  - `Source/Portfolio/Action/CAction_Guard.h`
  - `Source/Portfolio/Component/CActionOrchestratorComponent.cpp`
  - `Source/Portfolio/Component/CActionComponent.cpp`
  - `Source/Portfolio/Component/CDefenseComponent.cpp`
  - `Source/Portfolio/Component/CDefenseComponent.h`

- 관련 에셋:
  - `Content/04_Montage/GuardAndParry/M_Block_In_Montage.uasset`
  - `Content/04_Montage/GuardAndParry/M_Block_Out_Montage.uasset`
  - `Content/03_Animation/ABP_Character.uasset`

---

## 발생 조건

- Guard key pressed로 `Block_In`이 실행된다.

- `Block_In`이 끝나기 전에 Guard key release가 들어온다.

- release 요청이 현재 실행 중인 `Block_In`과 충돌해 `Block_Out`으로 이어지지 못한다.

---

## 재현 방법

1. Player가 Guard key를 눌러 `Block_In`을 실행한다.

2. `Block_In` montage가 끝나기 전에 Guard key를 release한다.

3. `Block_Out` montage가 실행되는지 확인한다.

4. release 이후 Guard Hold pose가 남는지 확인한다.

5. `CanGuard`, `CanParry`, `IsGuardingPose` 상태가 release 이후 기대대로 정리되는지 확인한다.

---

## 기대 결과 vs 실제 결과

**기대 결과**

- `Block_In` 중 release가 들어와도 release 의도는 버려지지 않아야 한다.

- `Block_In`이 안전하게 빠져나갈 수 있는 시점에 `Block_Out`이 실행되어야 한다.

- release 이후 `CanGuard`, `CanParry`, `IsGuardingPose`가 남지 않아야 한다.

**실제 결과**

- `Block_In` 실행 중 release 입력이 들어오면 `Block_Out` 요청이 즉시 처리되지 못할 수 있다.

- release 의도를 보관하는 경로가 없으면 `Block_In` 종료 후 Guard Hold pose가 남을 수 있다.

---

## 원인

- Guard release는 “새 공격 입력”이 아니라 현재 Guard 유지 의도를 종료하는 입력이다.

- 현재 실행 중인 `Block_In` 때문에 release 요청을 즉시 실행할 수 없을 때, 해당 요청을 pending 상태로 보관하는 경로가 없다.

- 보관된 요청을 완료 시점에 바로 실행하면 reaction takeover, dead state, cinematic lock, action relationship 정책을 우회할 수 있다.

- 따라서 pending release는 소비 시점에도 기존 action request 판단 경로를 다시 통과해야 한다.

---

## 수정 방향

v1에서는 Guard 전용 pending release로 범위를 좁힌다.

```text
Guard Released
-> 현재 Guard In 실행 중인가?
   - Yes -> Guard Out pending 저장
   - No  -> 기존 Guard Out request 실행

Guard In Complete 또는 exit notify
-> pending Guard Out이 있는가?
   - Yes -> pending 소비
        -> 기존 request pipeline으로 Guard Completed request 재평가
   - No  -> Guard Hold 유지
```

장기적으로는 Orchestrator의 deferred request pipeline으로 일반화할 수 있다.

---

## 수정 기준

- release 입력은 `Block_In` 실행 중에 들어와도 버려지지 않는다.

- pending release 소비는 기존 action request 판단 경로를 우회하지 않는다.

- `Block_Out` 실행이 불가능한 상태라면 pending request는 reject / ignore / expire 처리될 수 있어야 한다.

- Guard 전용 pending 구현은 이후 Orchestrator deferred request 구조로 승격 가능한 형태로 둔다.

---

## 검증 계획

- [ ] `Block_In` 초반 release 시 `Block_Out`으로 이어지는지 확인한다.

- [ ] `Block_In` 후반 release 시 `Block_Out`으로 이어지는지 확인한다.

- [ ] 정상 Hold 이후 release 시 기존 `Block_Out` 흐름이 유지되는지 확인한다.

- [ ] release 이후 `CanGuard=false`, `CanParry=false`, `IsGuardingPose=false`로 정리되는지 확인한다.

- [ ] reaction takeover 또는 action stop 상황에서 pending release가 잘못 실행되지 않는지 확인한다.

---

## 회귀 방지 기준

- Guard release 입력은 `Block_In` 실행 타이밍에 따라 유실되지 않아야 한다.

- Guard pose / guard 판정 / parry 판정 상태는 release 또는 interrupt 이후 stale state로 남지 않아야 한다.

- pending release는 직접 실행이 아니라 기존 request 판단 경로로 재평가되어야 한다.

---

## 관련 PR / 문서

- Work List: `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`

- Note: `Docs/06_notes/N02_Guard_Release_Deferred_Request_Note.md`

---

## 비고

- 본 Bug Report는 v1에서 해결할 Guard release 유실 문제를 추적한다.

- deferred request 구조의 장기 설계 근거는 N02 Note에서 관리한다.

---
