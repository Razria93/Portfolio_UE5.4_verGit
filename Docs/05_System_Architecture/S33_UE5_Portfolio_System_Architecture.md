# S33 공통 Combat Target Kernel 및 의사결정 통합 설계

## 0. 문서 목적과 상태

이 문서는 Player와 Enemy가 공통으로 사용하는 **전투 대상(Combat Target)의 단일 진실 공급원**과 그 주변 의사결정 계층을 정의한다.

핵심 질문은 다음과 같다.

> 현재 이 전투 주체가 실제로 싸우는 대상은 누구인가?

이 질문의 정답은 `UCCombatTargetComponent`만 소유한다. Player 입력, AI Perception, Behavior Tree, 카메라, HUD, Action, Reaction은 이 값을 직접 대체하지 않고 각각 후보를 수집하거나, 대상을 선택하거나, 확정된 대상을 소비한다.

### 문서 상태

- 설계 계약: 확정
- Goal 1 — 조사와 설계 확정: 완료
- Goal 2 — 공통 Combat Target Kernel: 구현 완료
- Goal 3 — Player Target 마이그레이션: 구현 완료
- Enemy Target Selection 및 Blackboard 투영: 후속 작업
- Engage·Facing 연동: 후속 작업

### S32와의 관계

- `S32`: 공통 Target 구조를 결정하기까지의 조사, 대안 비교, 기존 구조 분석 기록
- `S33`: 확정된 최신 계약과 현재 구현 상태를 기준으로 한 구현·검토 기준 문서

S32는 설계 이력으로 유지한다. 이후 구현과 리뷰에서는 이 문서를 최신 기준으로 사용한다.

---

## 1. 최종 결정 요약

### 1.1 공통 상태 저장소

Player와 Enemy 모두 `UCCombatTargetComponent`를 가진다.

```text
Player / Enemy
→ UCCombatTargetComponent
   → CurrentCombatTarget
   → Revision
   → ChangeReason
   → Target lifetime 관리
```

이 컴포넌트는 다음만 책임진다.

- 확정된 Combat Target 저장
- `SetTarget()` / `ClearTarget()` 계약
- 대상 변경 Revision 관리
- 대상 `EndPlay` 및 약한 참조 만료 처리
- 대상 변경 통지

후보 탐색, 점수 계산, AI 판단, Engage 승인, 카메라 보정은 책임지지 않는다.

### 1.2 선택 정책은 별도 계층

Player와 Enemy는 입력과 판단 재료가 다르므로 선택 계층은 분리한다.

```text
Player
Input / Screen / Camera
→ UCPlayerTargetSelectionComponent
→ UCCombatTargetComponent

Enemy
Perception / BT Request / Candidate Set / Recipe
→ UCEnemyTargetSelectionComponent
→ UCCombatTargetComponent
```

두 선택기는 같은 SoT에 결과를 Commit하지만, 같은 컴포넌트로 억지 통합하지 않는다.

### 1.3 Blackboard는 SoT가 아니다

Blackboard의 Target 값은 `UCCombatTargetComponent` 상태를 AI 의사결정 계층에 보여주는 **투영값**이다.

```text
UCCombatTargetComponent
→ Target Changed Event
→ AI Adapter / Service
→ Blackboard CombatTargetActor, CombatTargetRevision
```

BT가 Blackboard 값을 직접 변경하여 Combat Target을 확정하면 안 된다. BT는 선택 요청을 만들고, Commit 결과를 다시 읽는다.

### 1.4 Perception Target과 Combat Target은 다르다

- Perception 대상: 감지되었거나 기억 중인 Actor
- Combat Target: 현재 전투 대상으로 명시적으로 확정된 Actor

인지했다는 사실만으로 전투 대상이 되지는 않는다.

### 1.5 Target과 Engage는 별개다

- Combat Target: 누구와 싸우는가
- Engage: 그 대상에게 현재 공격·점유·전투 진입이 허용되는가

Target 선택 성공이 Engage 성공을 의미하지 않으며, Engage 거절이 Target 삭제를 의미하지도 않는다.

---

## 2. 전체 계층 구조

공통 Target 흐름은 네 계층으로 나눈다.

```text
1. Candidate Collection
   후보를 발견하고 관찰 가능한 재료를 수집

2. Target Selection
   후보와 정책을 평가하여 전투 대상 후보를 결정

3. Combat Target Kernel
   확정 대상을 Commit하고 수명과 Revision을 관리

4. Target Consumers
   확정 대상을 카메라·이동·액션·UI·AI 판단에 사용
```

### 2.1 Candidate Collection

후보 수집은 사실을 제공할 뿐 최종 대상을 확정하지 않는다.

Player의 대표 입력:

- 화면 중앙 후보
- 거리와 View Cone
- 좌우 전환 방향
- 입력 장치의 선택·해제 요청

Enemy의 대표 입력:

- AI Perception 감지 Actor
- 시야·청각·기억 정보
- BT가 구성한 후보 목록
- 전투 형태에 따른 Selection Recipe

### 2.2 Target Selection

선택기는 후보를 평가하고 `UCCombatTargetComponent`에 Commit을 요청한다.

선택 결과는 최소한 다음을 구분할 수 있어야 한다.

- Accepted
- Unchanged
- Rejected
- Cleared
- 유효성 실패 이유
- Commit 후 Target Revision

### 2.3 Combat Target Kernel

Kernel은 선택 정책을 알지 않는다. 전달받은 대상에 대해 공통 수명 계약만 적용한다.

### 2.4 Target Consumers

Consumer는 Target을 직접 소유하거나 교체하지 않는다.

- Lock-on 카메라·회전 보정
- Target HUD Presenter
- Action Facing 보정
- Combat Signal 수신 대상 해석
- AI Engage 평가
- Debug Overlay

---

## 3. `UCCombatTargetComponent` 책임 계약

## 3.1 소유 상태

```text
CurrentCombatTarget : TWeakObjectPtr<AActor>
Revision            : 대상 상태가 바뀔 때 증가하는 식별자
LastChangeReason    : 마지막 변경 원인
```

약한 참조를 사용하는 이유는 Target Component가 대상 Actor의 수명을 소유하지 않기 때문이다.

## 3.2 공개 API 의미

### `SetTarget(NewTarget, Reason)`

- 유효한 새 Target을 저장한다.
- 이전 Target의 종료 이벤트 구독을 해제한다.
- 새 Target의 종료 이벤트를 구독한다.
- 실제 상태가 변경된 경우에만 Revision을 증가시킨다.
- 변경 이벤트를 발행한다.

현재 Target과 동일한 Actor를 다시 요청하면 상태 변경이 아니므로 No-op으로 처리한다.

### `ClearTarget(Reason)`

- 현재 Target 구독을 해제한다.
- Target을 비운다.
- 실제로 값이 있었을 때만 Revision을 증가시킨다.
- 변경 이벤트를 발행한다.

### `GetCombatTargetSnapshot()` / `HasTarget()` / `GetTarget()` / `GetRevision()`

`GetCombatTargetSnapshot()`은 Target, Revision, 마지막 변경 사유를 한 번에 반환하는 현재 SoT 조회 API다. Consumer는 읽은 Target과 Revision을 함께 보존해야 비동기·지연 결과를 검증할 수 있다. 나머지 Query API는 단순 조회 편의 API로 유지한다.

## 3.3 Revision 계약

Revision은 Actor 자체의 ID가 아니라 **Combat Target 상태의 세대 번호**다.

```text
None → A   : Revision 1
A → A      : 변화 없음
A → B      : Revision 2
B → None   : Revision 3
```

비동기 또는 지연된 판단은 요청 당시 Revision을 결과와 함께 반환해야 한다.

```text
CombatTarget = A
RequestRevision = 7

처리 중 Target이 B로 변경
CurrentRevision = 8

A에 대한 늦은 결과 도착
ResultRevision = 7

7 != 8
→ Stale Result
→ 적용하지 않음
```

## 3.4 Target 수명 계약

### 정상 종료

현재 Target의 `EndPlay`를 구독하여 Actor가 Destroy, Level Removal 등으로 월드에서 빠지면 Target을 해제한다.

### 약한 참조 만료 fallback

종료 이벤트를 정상적으로 받지 못했더라도 Tick 또는 검증 시점에 Weak Pointer가 stale이면 명시적인 Clear 전이를 수행한다.

### Owner EndPlay

Component Owner가 종료할 때는 현재 Target의 종료 이벤트 구독만 해제하고 약한 참조와 Tick을 정리한다. Owner 자체가 사라지는 것은 살아 있는 Consumer가 처리해야 할 Combat Target 변경이 아니므로 Revision을 증가시키거나 변경 이벤트를 발행하지 않는다.

### 이전 Target의 늦은 Callback

```text
Current Target A
→ B로 교체
→ A의 늦은 EndPlay callback 도착
```

Callback Actor가 현재 Weak Target과 같은 Index·Serial인지 확인한다. A의 늦은 callback이 B를 지우면 안 된다.

## 3.5 Target 변경 이벤트

Target 변경은 `FCombatTargetChange` payload로 통지한다.

```cpp
OnCombatTargetChanged(const FCombatTargetChange& Change)
```

`Change`는 다음 정보를 하나의 원자적인 통지로 제공한다.

```text
PreviousTarget
CurrentSnapshot.TargetActor
CurrentSnapshot.Revision
CurrentSnapshot.LastChangeReason
```

Consumer는 이벤트 payload만으로 해당 변경 세대를 해석할 수 있다. 구독 전에 놓친 상태 또는 재진입 후 현재 상태 재조정에는 `GetCombatTargetSnapshot()`을 사용한다.

---

## 4. 현재 구현 상태와 최종 구조의 차이

## 4.1 구현 완료

- Player와 Enemy에 `UCCombatTargetComponent` 생성
- 공통 Component Reference 구조에 Combat Target 참조 추가
- 약한 Target 저장
- 동일 Target 요청 No-op
- Target 교체 및 Clear Revision 증가
- EndPlay 구독·해제
- stale target 정리
- 이전 Target의 늦은 종료 callback 방어
- Player·Enemy에서 Combat Target Component 접근 API 제공

## 4.2 Player 마이그레이션 완료

`UCPlayerTargetSelectionComponent`는 다음 Player 선택 정책만 소유한다.

- Player 후보 수집
- 후보 점수 계산
- Target 선택·전환
- 거리·View Cone·생존 상태 기반 정책 유효성 검사
- 선택 평가용 Debug Snapshot

확정 Target 저장, 변경 이벤트, Target EndPlay 구독, stale weak 정리는 모두 `UCCombatTargetComponent`가 소유한다. Player Runtime의 Combat Target SoT는 하나다.

## 4.3 후속 마이그레이션 대상

- Enemy Target Selection Component 추가
- Blackboard CombatTargetActor를 SoT 투영값으로 전환
- Combat Signal Source의 Blackboard 직접 Target 의존 제거
- Engage 결과에 Target Revision 검증 추가

---

## 5. Player Target Selection 계약

Player 선택 계층은 입력과 화면 공간 평가를 소유한다.

```text
Input
→ 후보 수집
→ 거리·View Cone·화면 중심 점수
→ 좌우 전환 평가
→ 최종 후보
→ CombatTargetComponent.SetTarget()
```

### 책임

- 선택·해제 입력 해석
- 후보 수집
- Target Eligibility 평가
- 중앙 우선 점수 계산
- 좌우 Target 전환
- 선택 실패 시 유지·해제 정책

### 비책임

- 확정 Target 저장
- Target EndPlay 수명 관리
- 카메라 회전
- 캐릭터 회전
- HUD 좌표 투영

Player 선택 정책 Component는 `UCPlayerTargetSelectionComponent`로 이름을 정리했다. 기존 Blueprint/UAsset class path와 Controller property path는 Core Redirect로 호환한다.

### Player Consumer 참조 수명

`ACPlayerController`가 소유한 Targeting, Lock Assist, HUD Consumer는 Possess 시 현재 `ACPlayer`의 `CombatTargetComponent`를 주입받는다. 각 Consumer는 변경 이벤트를 구독한 뒤 Snapshot을 조회해 현재 상태로 재조정한다.

UnPossess 시에는 Combat Target Component 참조와 이벤트 구독을 먼저 해제한다. Lock Assist는 그 뒤 Controlled Player의 회전 정책을 복구하고, HUD는 Target Marker를 숨긴다.

---

## 6. Enemy Target Selection 계약

Enemy의 선택기는 Perception 자체가 아니라 **인지 후보 중 전투 대상으로 확정할 대상을 선택하는 정책 계층**이다.

## 6.1 입력

- BT가 전달한 후보 목록 또는 후보 조회 Context
- 현재 Combat Target과 Revision
- 선택 Recipe
- 거리·가시성·위협도·전투 역할
- 후보의 생존·수용 가능 상태

## 6.2 Recipe 예시

```text
SingleMeleeTarget
RangedTargetFromGroup
HighestThreat
NearestVisible
PreserveCurrentIfValid
```

초기 구현은 필요한 최소 Recipe만 지원하고, 하나의 거대한 점수 함수로 모든 전투 형태를 섞지 않는다.

## 6.3 처리 흐름

```text
BT
→ Target Selection Request
→ UCEnemyTargetSelectionComponent
   → 후보·Recipe 검증
   → 후보 평가
   → UCCombatTargetComponent에 Commit
   → Result + Revision 반환
→ BT는 SoT Snapshot을 다시 조회
→ Blackboard에 투영
```

BT가 요청한 후보가 선택되지 않을 수 있으므로, 요청 인자를 그대로 Blackboard에 쓰면 안 된다.

## 6.4 요청 결과

동기 구현이라도 다음 정보는 명시하는 것이 좋다.

```text
RequestId 또는 RequestSerial
Decision
CommittedTarget
CommittedRevision
RejectReason
```

후속 비동기 확장 시 같은 계약을 유지할 수 있다.

---

## 7. AI Perception·BT·Blackboard 경계

## 7.1 Perception

Perception은 세계에서 감지한 사실을 제공한다.

```text
Actor A를 봄
Actor B의 소리를 들음
Actor C를 마지막으로 본 위치를 기억함
```

이 값은 후보 재료이지 Combat Target SoT가 아니다.

## 7.2 Behavior Tree

BT는 의도를 표현하는 얇은 의사결정 계층이다.

### Service

- CEnemy와 Component의 현재 상태를 읽음
- Perception·월드 정보를 읽음
- SoT Snapshot을 Blackboard에 투영
- 변경 이벤트를 구독한 Adapter가 있다면 재진입 시 Snapshot으로 재조정

### Task

- CEnemy 또는 전용 Component에 Request를 전달
- Action·Target Selection·Engage 등을 요청
- 결과를 확인하고 다음 의사결정을 진행

### 금지

- Blackboard 값을 바꿔 Runtime 상태를 직접 확정
- Action/Reaction Component 내부 상태 직접 수정
- Combat Target Component를 우회한 Target 저장
- Perception Target을 검증 없이 Combat Target으로 승격

## 7.3 Blackboard 권장 필드

```text
PerceivedTargetActor
CombatTargetActor
CombatTargetRevision
EngageState
EngageTargetRevision
TargetSelectionResult
```

실제 프로젝트에서는 기존 필드와의 마이그레이션 범위를 확인해 최소 필드부터 적용한다.

---

## 8. Event 구독과 Snapshot 재조정

이벤트만 구독하면 구독하지 않았던 시간의 상태 변경을 놓칠 수 있다.

따라서 Consumer나 AI Adapter의 진입 계약은 다음과 같다.

```text
1. OnTargetChanged 구독
2. CombatTargetComponent의 현재 Snapshot 조회
3. 로컬 투영값을 즉시 현재 상태로 재조정
4. 이후 이벤트로 증분 동기화
```

Snapshot 조회는 새로운 선택을 요청하는 것이 아니다. 과거에 이미 확정된 현재 상태를 다시 확인하는 과정이다.

```text
이전에 Target A가 확정됨
Consumer가 잠시 비활성화됨
그동안 Target B로 변경됨
Consumer 재활성화
→ 이벤트 재구독
→ Snapshot에서 B, Revision 8 확인
→ 이후 변경 이벤트 수신
```

이 규칙은 BT 재진입, UI 재생성, Debug Focus 재구독에도 동일하게 적용한다.

---

## 9. Engage 계약

## 9.1 Target과 분리하는 이유

Enemy가 Player를 Combat Target으로 가지고 있어도 다음 이유로 공격 권한이 없을 수 있다.

- 근접 Engage Slot이 없음
- 공격 가능 거리 밖
- 다른 협업 Actor가 우선권을 가짐
- 현재 Action/Reaction 상태가 공격을 허용하지 않음

따라서 Engage는 Target의 부가 bool이 아니라 별도의 승인 결과다.

## 9.2 Event + Snapshot

Engage 평가 결과도 변경 이벤트로 Blackboard에 투영할 수 있다. 다만 구독 재진입 시 현재 상태를 놓치지 않도록 Snapshot 조회가 함께 필요하다.

## 9.3 Revision 검증

Engage 결과는 어떤 Target 상태를 기준으로 계산했는지 기록한다.

```text
Engage Request
- Target A
- TargetRevision 7

Engage Result
- Accepted
- Target A
- ResultRevision 7

현재 CombatTarget Revision 8
→ 결과 폐기
```

## 9.4 Request Result와 Current Snapshot

두 값은 목적이 다르다.

- Request Result: 방금 보낸 요청이 어떻게 처리되었는가
- Current Snapshot: 현재 시스템의 권위 있는 상태가 무엇인가

BT는 Task 결과로 요청의 성공 여부를 확인하고, Blackboard 투영은 현재 Snapshot을 기준으로 유지한다.

---

## 10. 피격 후 Engage 흐름

피격당했다는 이유만으로 Blackboard를 직접 Engage로 바꾸지 않는다.

권장 흐름:

```text
Damage / Combat Result 수신
→ 공격자를 Perception 또는 강제 후보 Context에 반영
→ Enemy Target Selection Request
→ Combat Target Commit
→ 해당 Target + Revision으로 Engage Request
→ Engage 결과 검증
→ AI Intent를 Engage로 투영
```

이미 더 높은 우선순위의 Combat Target이 있거나 공격자가 유효하지 않다면 기존 Target을 유지할 수 있다.

---

## 11. Target Consumer 책임

## 11.1 Target Lock Assist

Lock Assist는 확정 Target을 소비하여 표현과 조작을 보정한다.

- 카메라 Yaw/Pitch 추적
- Lock-on 이동 회전 정책
- Target Focus Offset
- 근거리 Pitch 완화

후보를 선택하거나 Target을 저장하지 않는다.

## 11.2 Target HUD Presenter

- Combat Target 조회
- Target Socket/World Location 조회
- 화면 좌표 투영
- 화면 밖 Visibility 처리
- Target 변경 시 동일 Widget 갱신

## 11.3 Combat Signal Source

공격·신호의 수신 대상을 정할 때 Blackboard 임시값보다 Combat Target SoT를 우선 사용한다. 특정 Action이 명시적인 개별 Target을 캡처했다면 Action Context의 Target과 Revision 계약을 따른다.

## 11.4 Debug Overlay

최소한 다음을 구분하여 표시한다.

- Perceived Target
- Combat Target
- Combat Target Revision
- Selection Result / Reject Reason
- Engage Target Revision
- Consumer가 사용 중인 Target Snapshot

---

## 12. Action Facing 계약

Target 저장과 공격 중 회전 보정은 분리한다.

Action은 시작 시 Target과 Revision을 캡처하고, 데이터가 허용한 구간에서만 Target 방향을 추적한다.

```text
Action Start
→ Combat Target Snapshot 캡처
→ Tracking Window
   → 제한된 속도로 Target 방향 보간
→ Commit Point
   → 공격 방향 고정
→ Hit Window / Follow Through
```

콤보의 다음 공격은 새 Action 또는 다음 SubAction 시작 시 최신 Target Snapshot을 다시 평가할 수 있다.

### 권장 데이터

```text
bUseTargetFacing
TrackingStart
TrackingEnd 또는 FacingCommitNotify
MaxYawSpeed
MaxTrackAngle
TargetRevisionPolicy
```

### 금지

- 공격 전체 구간에서 무제한 Target 추적
- 카메라 회전을 공격 궤적 보정의 대체물로 사용
- Target이 변경됐는데 이전 요청 결과로 방향을 갱신

---

## 13. Player·Enemy 대칭성과 비대칭성

| 영역 | Player | Enemy | 공통 여부 |
|---|---|---|---|
| 확정 Target 저장 | CombatTargetComponent | CombatTargetComponent | 공통 |
| Target 수명 | Weak / EndPlay / Revision | Weak / EndPlay / Revision | 공통 |
| 후보 입력 | Camera / Screen / Input | Perception / BT / World | 다름 |
| 선택 정책 | 중앙 우선 / 좌우 전환 | Recipe / Threat / Role | 다름 |
| 의사결정 주체 | 사용자 | BT / AI Controller | 다름 |
| 카메라 보정 | 사용 | 일반적으로 미사용 | 다름 |
| Facing 소비 | Player Action | Enemy Action | 계약 공통화 가능 |
| Blackboard 투영 | 불필요 | 필요 | Enemy 전용 Adapter |

대칭화 대상은 **상태와 수명 계약**이다. 입력·선택·표현 방식까지 동일하게 만들 필요는 없다.

---

## 14. 구현 단계와 완료 기준

## Goal 1 — 조사와 설계 확정

상태: 완료

- Player Targeting, Enemy Perception, BT, Blackboard, Consumer 조사
- 공통 SoT와 분리 Selection 구조 확정
- Target과 Engage 분리 확정

## Goal 2 — 공통 Combat Target Kernel

상태: 구현 완료, Consumer 연결 전

- `UCCombatTargetComponent`
- `FCombatTargetSnapshot` / `FCombatTargetChange` 타입
- Player·Enemy 소유 및 Component Reference 연결
- Revision, 원자적 변경 통지, Target/Owner 수명 계약

## Goal 3 — Player Target 마이그레이션

상태: 구현 완료

```text
Player Target Selection Component
→ Selection 책임만 유지
→ 결과를 CombatTargetComponent에 Commit
→ LockAssist / HUD / Debug Consumer 전환
→ 중복 CurrentTarget 제거
```

완료 기준:

- Player Combat Target SoT가 하나뿐임
- 기존 선택·전환·카메라·마커 동작 회귀 없음
- Target Destroy/EndPlay 시 이벤트가 한 번만 발생
- Possess/UnPossess 뒤 Consumer가 Snapshot으로 현재 Player 상태를 재조정하고 이전 Player 구독을 남기지 않음

## Goal 4 — Enemy Target Selection과 BB 투영

- `UCEnemyTargetSelectionComponent` 추가
- 최소 Selection Recipe 구현
- Perception 후보와 Combat Target 분리
- TargetChanged Event + Snapshot Adapter
- Blackboard CombatTargetActor/Revision 투영

완료 기준:

- Perception 감지만으로 Combat Target이 자동 확정되지 않음
- BT는 Request를 통해서만 Target 변경
- Blackboard가 SoT를 역으로 덮어쓰지 않음

## Goal 5 — Engage와 피격 정책

- Target Revision을 포함한 Engage Request/Result
- Engage Event + Snapshot 투영
- 피격 공격자를 후보로 반영
- Selection과 Engage 승인 후 Intent 갱신

## Goal 6 — Action Facing 및 Consumer 정리

- Player/Enemy Action Facing Window
- Combat Signal Source SoT 전환
- Debug Overlay의 Perception/Target/Engage 구분
- 잔여 Blackboard 직접 Target 의존 제거

---

## 15. 수동 검증 시나리오

## 15.1 Kernel 수명

```text
None → A
```

- Target A 저장
- Revision 1 증가
- Changed Event 1회

```text
A → A
```

- Revision 변화 없음
- 중복 Changed Event 없음

```text
A → B
```

- A 구독 해제
- B 구독 시작
- Revision 증가
- Previous=A, Current=B

```text
A의 늦은 Destroy / EndPlay
```

- 현재 B 유지
- Revision 변화 없음

```text
B Destroy / EndPlay
```

- Target Clear
- Revision 증가
- Changed Event 1회

```text
Component Owner EndPlay
```

- Target delegate 구독 해제
- Weak Target 및 Tick 정리
- Revision 변화 없음
- Changed Event 없음

## 15.2 Player 회귀

- 선택·해제
- 중앙 우선 Target
- 좌우 전환
- 방향 후보 부재 시 유지
- 거리 초과·사망·EndPlay 해제
- Lock Assist와 Marker가 같은 Target 사용

## 15.3 Enemy 흐름

- Perception 후보가 있어도 선택 요청 전에는 Combat Target이 없음
- Selection Request 성공 후 SoT와 BB 투영 일치
- Target 변경 후 이전 Revision의 Engage 결과 무시
- 피격 공격자가 유효 후보로 들어감
- 기존 Target 우선순위가 높으면 피격 공격자로 불필요하게 교체하지 않음

## 15.4 구독 재진입

- Consumer 비활성 중 Target 변경
- 재활성 시 이벤트 구독
- Snapshot 조회로 즉시 최신 Target/Revision 복구
- 이후 변경 이벤트 정상 수신

---

## 16. 불변 규칙

1. 확정 Combat Target은 `UCCombatTargetComponent`만 저장한다.
2. Selection Component는 선택 결과를 Commit하지만 Target 수명을 소유하지 않는다.
3. Consumer는 Target을 직접 교체하지 않는다.
4. Perception과 Blackboard는 Combat Target SoT가 아니다.
5. Target 변경 결과와 지연된 정책 결과는 Revision으로 검증한다.
6. Engage 승인과 Target 선택은 별도 계약이다.
7. Event 구독자는 재진입 시 Snapshot으로 현재 상태를 재조정한다.
8. Target의 늦은 종료 Callback이 새 Target을 지우면 안 된다.
9. Action Facing은 데이터가 허용한 Tracking Window에서만 작동한다.
10. Player와 Enemy의 공통화 범위는 저장·수명·Revision 계약까지다.

---

## 17. 금지 패턴

- BT Task가 Blackboard만 수정하여 Target을 확정
- Perception의 마지막 감지 Actor를 곧바로 Combat Target으로 간주
- Lock Assist 또는 HUD Presenter가 자체 Target을 저장
- Player Selection Component와 Combat Target Component가 각각 CurrentTarget을 장기 보유
- Engage 결과에 Target Revision이 없음
- 이벤트만 구독하고 초기 Snapshot을 읽지 않음
- 요청한 후보를 Commit 결과 확인 없이 Blackboard에 기록
- Target 변경 뒤 이전 Target의 callback을 현재 Target 종료로 처리

---

## 18. 주요 코드 접점

현재 Goal 2 기준 주요 파일:

```text
Source/Portfolio/Component/CCombatTargetComponent.h
Source/Portfolio/Component/CCombatTargetComponent.cpp
Source/Portfolio/Type/CCombatTargetTypes.h
Source/Portfolio/Type/CCharacterComponentReferenceTypes.h
Source/Portfolio/Character/Player/CPlayer.h
Source/Portfolio/Character/Player/CPlayer.cpp
Source/Portfolio/Character/Enemy/CEnemy.h
Source/Portfolio/Character/Enemy/CEnemy.cpp
```

후속 Player 마이그레이션 조사 대상:

```text
Source/Portfolio/Component/CPlayerTargetSelectionComponent.*
Source/Portfolio/Component/CTargetLockAssistComponent.*
Source/Portfolio/Component/CTargetHUDPresenterComponent.*
Player Controller 입력 및 Debug Overlay Target Focus 경로
```

후속 Enemy 마이그레이션 조사 대상:

```text
AI Perception 갱신 경로
CBTService_UpdateAIContext
CBTService_UpdateAIIntentState
TargetActor Blackboard 소비 Task/Service
Engage Assignment Context
Combat Signal Source Target 해석
```

---

## 19. 리뷰 체크리스트

구현 리뷰에서는 다음 순서로 확인한다.

1. Target 상태를 새로 저장하는 코드가 생겼는가?
   - 생겼다면 Combat Target SoT의 Projection 또는 Action Snapshot인지 확인한다.
2. Target을 바꾸는 코드가 `SetTarget` / `ClearTarget`을 우회하는가?
3. 지연 결과가 Target Revision을 검증하는가?
4. Event 구독자가 초기 Snapshot을 읽는가?
5. Perception·Blackboard·Engage·Combat Target의 의미가 섞였는가?

### Goal 4 구현 메모 — Enemy Target Selection과 Blackboard Projection

- `UCEnemyTargetSelectionComponent`는 BT가 전달한 명시 후보를 검증해 `UCCombatTargetComponent`에만 commit 요청한다. Component 자체는 Current Target을 저장하지 않는다.
- `PerceivedTargetActor`는 Perception 후보이며, `CombatTargetActor`는 확정 Combat Target의 projection이다. `CombatTargetRevision`은 동일 Snapshot 세대의 Revision이며, Clear된 상태도 `CombatTargetActor = None`과 증가한 현재 Revision으로 투영한다.
- `UCBTTask_RequestCombatTargetSelection`만 후보를 Target 선택 요청으로 승격한다. Blackboard write는 Combat Target을 변경하지 않는다.
- Projection은 `UCBTService_UpdateAIContext`가 Tick 중 Snapshot을 읽어 수행한다. 후보 상실은 Perception 값만 clear하며 현재 Combat Target을 자동 clear하지 않는다.

### Goal 4 UAsset 수동 연결

- Enemy Blackboard에 `PerceivedTargetActor` Object/Actor 키와 `CombatTargetRevision` Int 키를 추가한다.
- Blackboard의 기존 `TargetActor` Object/Actor 키는 `CombatTargetActor`로 이름을 변경한다. BT의 해당 key selector 참조도 함께 갱신한다.
- Blackboard의 기존 `TargetPriority` Int 키는 `PerceivedTargetPriority`로 이름을 변경한다. 인지 후보를 참조하는 BT key selector도 함께 갱신한다.
- 기존 `Set Focus` BT Task는 `Set Combat Target Focus`로 교체한다. `Clear Focus`는 Gameplay Focus 공용 해제 Task로 유지한다.
- Perception 후보가 준비된 BT 경로에 `Request Combat Target Selection` Task를 연결한다. Task 연결 전에는 Perception만 갱신되고 Combat Target은 확정되지 않는다.

### Goal 5-A 구현 메모 — Engage Revision과 Combat Signal Target

- `FEngageRequestContext`와 `FEngageAssignmentContext`는 Target Actor와 함께 `TargetRevision`을 보존한다.
- Engage Subsystem은 request bucket 구성과 기존 assignment lease 보존 전에 Enemy Combat Target Snapshot의 Actor·Revision을 함께 검증한다. 일치하지 않는 지연 요청·할당은 다음 결과에 반영하지 않는다.
- Combat Signal timing cue의 기본 Target 해석은 AI Blackboard가 아니라 Owner Character의 `UCCombatTargetComponent` Snapshot을 사용한다.

### Goal 6 구현 메모 — Enemy Facing과 Action Consumer Revision 검증

- Focus Task는 Blackboard projection의 Target Actor·Revision이 현재 Combat Target Snapshot과 일치할 때만 Gameplay Focus를 설정한다.
- Combat Action Task는 같은 projection 검증 뒤 Snapshot을 Enemy에 전달하며, Enemy는 Action 요청 직전에 Snapshot을 다시 검증한다.
- Combo chain 예약은 최초 수락된 Combat Target Snapshot을 보존하고, chain window에서 같은 Actor·Revision인지 재검증한다. Target 세대가 바뀌면 예약을 진행하지 않는다.
6. Player와 Enemy의 선택 정책을 억지로 공통화했는가?
7. Target Consumer가 Selection 또는 수명 책임을 침범하는가?
8. Destroy·EndPlay·stale·이전 callback 경계가 안전한가?

이 체크리스트를 만족하는 한, 후보 수집과 전투 정책은 Player와 Enemy의 필요에 맞게 독립적으로 확장할 수 있다.
