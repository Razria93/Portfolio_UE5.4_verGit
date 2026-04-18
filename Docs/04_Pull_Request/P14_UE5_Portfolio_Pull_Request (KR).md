# Combat Feedback 파이프라인 구축 및 Player / Enemy Feedback 흐름 통합

## 제목

`✨ feat: combat-feedback 파이프라인 구축 및 player / enemy feedback 흐름 통합 (#39)`

## 요약

- 본 PR에서는 **action-feedback / reaction-feedback**을 **Player-side와 Enemy-side 모두에서 동작하도록 구현**하였고, **Player-side local feedback**으로 **player-feedback**도 추가로 구현하였음.

- **action-feedback**은 다음과 같이 처리하였음.

  - `AnimNotifyState_ActionFeedback`로 **Trail On / Off**를 실행함.

  - `AnimNotify_ActionFeedback`로 **Sword SFX**와 **Buff VFX / Buff SFX**를 실행함.

  - `CAction`의 `BeginPlayAction`, `EndPlayAction`로 **ActionStart / ActionEnd**를 실행함.

- **reaction-feedback**은 `TakeDamage` 결과를 dispatch하는 흐름에 연결하여 **hit VFX / hit SFX / Hit Stop**을 구현하였고, **player-feedback**에서는 **CameraShake**를 구현하였음.

- 또한 **action-feedback request 생성 경로**를 `CAction` 직접 참조 구조에서 **owner-level interface 기반 공통 흐름**으로 재구성하였고, **Enemy 공격 종료 cleanup**은 `EndAttackTask`로 이관하였음.


---

## 완료 작업

### 1. Feedback 구조 계층 분리

- 전투 feedback을 **reaction-feedback**, **action-feedback**, **player-feedback**의 세 계층으로 구분하여 정리함

- **Player-side / Enemy-side 공통 feedback**과 **Player local feedback**의 역할 경계를 분리함

### 2. Shared Combat Feedback 경로 정리

- `CWorldSubsystem_CombatFeedback`를 추가함

- world-level combat feedback 처리 경로를 구성함

- **hit-stop** 및 **camera shake** 처리의 공통 feedback 경로를 정리함

- **player-facing feedback**과 **shared feedback**의 실행 대상을 분리함

### 3. Reaction Feedback 구현

- `CReactionFeedbackComponent` 구조를 추가함

- `TakeDamage` 처리 이후 결과를 dispatch하는 흐름에 **reaction-feedback**을 연결함

- **hit VFX / hit SFX / Hit Stop**이 Player-side / Enemy-side 모두에서 동작하도록 구현함

- reaction 결과를 기반으로 공통 combat-feedback 경로가 실행되도록 정리함

### 4. Player Feedback 구현

- `CPlayerFeedbackComponent`를 추가함

- Player local 단위 feedback 처리 경로를 분리함

- 전투 결과를 기반으로 해당 Player에게만 적용되는 **`CameraShake`**를 구현함

- camera shake 처리를 player-feedback 계층으로 위임함

### 5. Action Feedback 구현

- `CActionFeedbackComponent`를 추가함

- **Trail / VFX / SFX** 실행 경로를 분리함

- `AnimNotifyState_ActionFeedback`를 기반으로 **`Trail On / Off`**를 구현함

- `AnimNotify_ActionFeedback`를 기반으로 **`Sword SFX`**를 Player-side / Enemy-side 모두에서 동작하도록 구현함

- `AnimNotify_ActionFeedback`를 기반으로 **`Buff VFX / Buff SFX`**를 구현하여 단발성 feedback의 실행 타이밍을 notify로 제어할 수 있도록 정리함

- **`ActionStart` / `ActionEnd`** 시점에 실행 가능한 action-feedback API를 추가함

### 6. Action Feedback 데이터 및 실행 구조 재구성

- **action-feedback**을 **request / timing / trigger** 기반으로 매칭하는 구조로 재구성함

- `FActionFeedbackRequest`, `FActionFeedbackKey`, `EActionFeedbackTiming` 구조를 추가함

- Trail / VFX / SFX 데이터를 분리하여 실행 경로를 독립적으로 정리함

- **duplicate execution key filtering**을 추가함

- request 및 execution 로그 출력 구조를 정리함

### 7. Player / Enemy 공통 Action Feedback 요청 흐름 정리

- 기존 **`notify -> ActionComponent -> CAction`** 직접 참조 기반 request 생성 경로를 제거함

- `ActionFeedbackRequestProvider` 인터페이스를 추가함

- Player와 Enemy가 각자의 런타임 문맥을 통해 `FActionFeedbackRequest`를 생성하도록 변경함

- **action feedback notify**가 Player / Enemy 공통 실행 경로를 공유하도록 정리함

- base `CAction`에 **`ActionStart` / `ActionEnd`** feedback dispatch를 연결함

- `GetCurAction()` 반환형을 `UCAction*`로 정리함

### 8. Enemy 공격 종료 흐름 정리

- `CBTTask_EndAttack`를 추가함

- `CAnimNotify_EndEnemyAttack`를 **종료 signal 역할**로 축소함

- 정상적인 **enemy attack-end cleanup**을 **BT end-attack 흐름**으로 이관함

- attack start 시 Enemy에 **active action feedback key**를 캐시하도록 변경함

- `AttackIndex`, `AttackActionType`, `LastAttackIndex`의 역할을 정리함

- **AI state transition cleanup**을 예상하지 못한 state 이탈에 대한 **safety-net**으로 유지함

### 9. Asset 및 관련 데이터 반영

- **reaction-feedback** 관련 VFX / SFX asset을 추가 및 연결함

- **action-feedback** 관련 Sword / Buff feedback asset을 추가 및 연결함

- montage / behavior-tree / blackboard / camera shake 관련 asset을 갱신함

- 관련 issue / PR / branch 문서를 갱신함


---

## 테스트 방법

1. Player-side / Enemy-side에서 reaction-feedback이 hit VFX / hit SFX / hit-stop까지 정상 연결되는지 확인

2. Player-side에서 camera shake가 local feedback으로 정상 실행되는지 확인

3. action-feedback이 `ActionStart`, `ActionEnd`, `TriggerOnce`, `TriggerWindowBegin`, `TriggerWindowEnd` 시점에 맞게 실행되는지 확인

4. `AnimNotifyState_ActionFeedback` 기반으로 Trail On / Off가 정상 동작하는지 확인

5. `AnimNotify_ActionFeedback` 기반으로 Sword SFX, Buff VFX, Buff SFX가 정상 실행되는지 확인

6. Player-side / Enemy-side 모두 동일한 action-feedback notify 경로를 사용할 수 있는지 확인

7. Enemy BT attack flow가 아래 순서로 정상 동작하는지 확인

   - `SelectAttackIndex`

   - `StartAttack`

   - `CommitAttackCooldown`

   - `WaitAttackEnd`

   - `EndAttack`

8. 정상적인 enemy attack-end cleanup이 `EndAttackTask`에서 처리되는지 확인

9. 예상한 attack-end 흐름을 벗어나는 경우에도 state transition cleanup이 safety-net으로 동작하는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-feedback`

- 관련 작업:

  - `M03-05: Combat Feedback 파이프라인 구축 및 Player / Enemy Feedback 흐름 통합 (#39)`


---

## 노트

- 본 PR의 핵심은 단일 feedback 기능 추가가 아니라, **combat result 생성부터 실제 플레이어 체감 피드백 출력까지 이어지는 전체 combat-feedback 경로를 정리**하는 데 있음

- feedback은 현재 **reaction-feedback**, **action-feedback**, **player-feedback**의 세 층으로 구분됨

- Enemy에 **action-feedback**을 연결하는 과정에서 기존 request 생성 경로가 `CAction`에 강하게 결합되어 있음을 확인하였고, 이를 **interface 기반 request-provider 흐름**으로 재구성하여 Player / Enemy 공통 notify 경로를 공유할 수 있도록 정리함

- **Enemy 공격 종료 cleanup**은 정상적인 공격 종료 cleanup과 예상하지 못한 state 이탈 cleanup의 두 경로로 구분하여 정리함

- 본 PR은 전체 액션 구조 개편을 완료하는 작업은 아니며, 이후 브랜치에서 action orchestration, 상태 변경과 행동 실행 순서 정리, `CAction`의 Player 의존도 축소 작업을 계속 진행할 예정임


---
