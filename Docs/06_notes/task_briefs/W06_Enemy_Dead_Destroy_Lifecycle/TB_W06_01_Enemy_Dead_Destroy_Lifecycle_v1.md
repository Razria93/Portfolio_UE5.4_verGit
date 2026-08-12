# TB W06-01 Enemy Dead / Destroy Lifecycle v1

> **2026-08-12 정책 개정:** 이 Task Brief 아래쪽에는 최초 `Alive -> Dying -> Dead`, Finalize Notify 기반 구현 기록과 당시 실행 프롬프트가 보존되어 있다. 최신 구현 기준은 `Docs/05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md`의 `Alive / Dead + DeadIn / DeadLoop + Feedback Presentation + 완료 이벤트 기반 Destroy` 계약이다. AnimBP/BT 자산 마이그레이션과 레거시 코드 제거까지 완료했으며, 상충하는 기존 문구는 구현 기준으로 사용하지 않는다.

## 최신 확정 흐름

```text
Health Alive -> Dead
-> AnimBP bIsDead = true / DeadLoop 준비
-> DeadIn Reaction
-> DeadIn Completed
-> Enemy가 Feedback Component에 Death Presentation 요청
-> Dissolve Finished
-> Presentation Watchdog 해제
-> RequestFinalizeDeath
-> 다음 Tick cleanup / Destroy
```

정상 Destroy 시점은 Dissolve 완료 이벤트가 소유한다. Timer는 `ExpectedDuration + SafetyMargin` 이후 Presentation 완료 이벤트 누락을 복구하는 Watchdog으로만 사용한다. DeadIn Reject와 formal Interrupted / Ignored는 즉시 Destroy하지 않고 동일한 Presentation 경로로 합류한다. DeadIn Started 후 종결 이벤트 누락은 Enemy Timer로 우회하지 않으며 Reaction lifecycle 계약 위반으로 취급한다.

최신 정책에서 기존 `Enter Dead State`, `Enter Alive State`, `Finalize Enemy Death` Notify는 정상 흐름에 사용하지 않는다. 아래의 기존 구현 기록은 마이그레이션 대상 파악과 이력 보존 목적으로만 유지한다.

## 2026-08-12 C++ 구현 상태

```yaml
Runtime C++: 완료
Development Build: 성공
Life-State UAsset Migration: 완료
Death Presentation Asset Integration: 대기
PIE: 대기
Commit / Push: 수행하지 않음
```

구현된 파일 축:

```text
Type/CCharacterFeedbackTypes
Component/CCharacterFeedbackComponent
Component/CHealthComponent
Component/CStateComponent
Component/CMovementComponent
Character/CAnimInstance
Character/Enemy/CEnemy
Legacy Revive / Health Notify / Finalize Notify source removed
```

첫 빌드에서는 `CStateComponent.h`의 Query 접근 지정자 이동 실수가 발견됐고 즉시 수정했다. 재빌드는 `PortfolioEditor Win64 Development`에서 성공했다.

## 작업명

```text
Enemy Dead / Destroy Lifecycle v1
```

## 브랜치

```text
feature/dead-actor-destroy-flow
```

## 기준 HEAD

```text
7a0463b9
```

## 상태

```text
Goal 1 Runtime 구현 / 생명 상태 자산 마이그레이션 완료 / Presentation 연결 및 PIE 대기
```

## 목적

Enemy의 사망 판정, Dead Reaction 연출, 최종 gameplay cleanup과 Actor `Destroy()`를 하나의 설명 가능한 생명주기로 연결한다.

확정된 Runtime 구조와 책임 계약은 `Docs/05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md`를 기준으로 한다. 본 Task Brief는 구현 절차, 에디터 연결과 검증 상태를 기록한다.

기존 Action / Reaction 파이프라인을 사망 연출과 실행 충돌 해결의 정규 경로로 유지하고, Enemy는 사망 생명주기의 시작과 최종 종료를 조정한다. W05 Player Targeting에서 실제 Actor Destroy 정책이 없어 이관했던 Target 해제 경계도 이번 작업에서 통합 검증한다.

## 확정 정책

### Enemy 사망 3단계

```text
1차: Dying 진입
-> Enemy가 신규 실행 의도를 차단
-> 기존 Health / AI / Action / Movement gate가 이미 처리하는 책임은 중복 구현하지 않음

2차: Dead Reaction
-> 기존 ReactionOrchestrator에 Dead Reaction 요청
-> 기존 Action / Reaction Intervention으로 활성 실행 정리

3차: Dead Montage 후반 Finalize Notify
-> Enemy의 단일 FinalizeDeath 경로 호출
-> 최종 gameplay cleanup
-> Actor Destroy
```

### Fallback

```text
Dead Reaction Rejected
Dead Reaction 비정상 Interrupted
Finalize Notify 누락 후 Reaction Completed
-> 동일한 FinalizeDeath 경로
```

`FinalizeDeath()`는 정상 Notify와 fallback이 같은 프레임 또는 연속해서 호출돼도 한 번만 실행되는 멱등 API로 구성한다.

### Player 정책

Player는 Respawn / Game Over 정책이 확정되지 않았으므로 이번 작업에서 실제로 Destroy하지 않는다. 공통 cleanup 계약을 재사용할 수 있는지는 검토하되 Enemy 정책을 Player에 강제로 일반화하지 않는다.

### 자산 정책

- Enemy Dead Montage 후반에 전용 Finalize Death Notify를 배치한다.
- C++는 Notify 타입과 Runtime API를 제공한다.
- 실제 Montage / Blueprint / ReactionData 연결은 사용자가 에디터에서 수행한다.
- 시체 유지, Ragdoll, Pooling은 이번 v1 범위에서 제외한다.

### Targeting 연계 정책

Targeting은 Enemy의 구체적인 사망 정책을 알지 않는다.

```text
Enemy Destroy
-> Actor OnEndPlay
-> UCTargetingComponent의 Target EndPlay callback
-> OnTargetChanged(PreviousTarget, nullptr)
-> Lock Assist / Marker / Debug Focus 정리
```

`OnEndPlay`를 정규 Actor 수명 종료 경로로 유지하고 `TWeakObjectPtr::IsStale()` 검사는 callback 누락 또는 Weak Object 선행 만료를 위한 fallback으로 유지한다.

## Goal 1 실제 구현 계약

### P0 조사 결론

```text
Health
- HP가 0 이하가 되면 Alive -> Dying
- 기존 Enter Dead State Notify가 Dying -> Dead 확정

실행 차단
- ActionOrchestrator와 CombatSignalTarget은 Alive가 아니면 신규 요청 거절
- MovementComponent는 Dead 실행 상태와 이동 불가 상태에서 입력 거절
- Enemy는 Dying 진입 즉시 AI path movement를 정지하고 deferred action을 제거

실행 정리
- Dead Reaction은 기존 Independent / Exclusive Intervention 경로 재사용
- Dead는 활성 Action / Reaction을 정리하며 자신은 추가 Intervention을 허용하지 않음

독립 Runtime
- Dying 진입에서 Weapon collision, trail, hit context를 즉시 정리
- Weapon Actor 파괴와 각 Component teardown은 기존 EndPlay 책임 유지
- 현재 main에는 BalanceComponent와 Balance Timer가 없으므로 이번 변경 대상 아님

Targeting
- Enemy Destroy -> OnEndPlay -> TargetingComponent callback 계약을 그대로 재사용
- TargetingComponent 수정 없음
```

### Reaction 생명주기 통지

`UCReactionComponent`는 활성 `FReactionExecutionContext`를 보존하고 다음 native event를 발행한다.

```text
Started
Completed
Interrupted
Ignored
```

Dead Reaction만 Enemy 사망 생명주기에서 해석한다. 명시적 Intervention은 기존 종료 경로가 담당하며, 해당 정규 경로에서 발행된 `Interrupted` / `Ignored` 생명주기 이벤트만 Finalize fallback으로 해석한다. 엔진 측 `OnMontageEnd(bInterrupted)`는 명시적 Stop 경로 밖에서 발생한 계약 위반으로 Audit만 기록하고 종료 상태를 대신 정리하지 않는다.

### Enemy 사망 생명주기

```text
OnDeadStateChanged(Dying)
-> BeginDeathLifecycle
-> StopMovement
-> ClearAllDeferredActions
-> ClearWeaponRuntimeState
-> 다음 틱 Dead Reaction 시작 watchdog 예약

Dead Reaction Started
-> 시작 watchdog 해제

Finalize Enemy Death Notify
또는 Dead Reaction Completed / Interrupted / Ignored
또는 다음 틱까지 Dead Reaction 미시작(Rejected / 요청 누락)
-> RequestFinalizeDeath
-> 다음 틱 FinalizeDeath 예약
```

`RequestFinalizeDeath()`는 공개된 Notify 진입점이다. 실제 `Destroy()`는 Reaction 및 Anim Notify callback stack이 반환된 다음 틱에 수행해 재진입을 피한다.

```text
FinalizeDeath
-> 요청/실행 flag로 중복 방지
-> 아직 Dying이면 Dead 상태 확정
-> 최종 gameplay runtime cleanup
-> ACEnemy::Destroy
-> 기존 Actor / Component EndPlay teardown
```

Reviving 또는 Alive 전이가 Finalize 예약보다 먼저 발생하면 watchdog과 Finalize timer를 취소한다. Player에는 이 생명주기를 연결하지 않는다.

### gameplay cleanup과 EndPlay teardown

```text
Destroy 직전 gameplay cleanup
- AI path movement 정지
- deferred action 제거
- Weapon collision / trail / hit context 정리

EndPlay teardown
- Action / Health / Reaction delegate 구독 해제
- 사망 watchdog / finalize timer 해제
- 각 Component runtime map과 delegate 정리
- WeaponComponent가 Weapon Actor 파괴
- Pawn 종료에 따른 AIController UnPossess / runtime teardown
```

두 단계에서 안전한 API를 일부 반복 호출할 수 있지만, Weapon Actor 파괴나 Controller teardown 책임을 Enemy가 중복 소유하지 않는다.

### 사용자 에디터 작업

Enemy Dead Montage에는 기존 `Enter Dead State` Notify 뒤, 몽타주 후반의 시각 효과가 보존되는 마지막 안전 프레임에 `Finalize Enemy Death` Notify를 배치한다.

```text
Dead Montage 시작
-> Enter Dead State
-> 사망 후반 연출 / feedback
-> Finalize Enemy Death
-> 다음 틱 Enemy Destroy
```

Finalize Notify를 누락해도 Montage Completed fallback으로 Destroy되지만, 정상 경로의 의도된 제거 시점은 전용 Notify가 결정한다.

## 책임 경계

```text
UCHealthComponent
- HP와 Alive / Dying / Dead 상태 전이
- Dead 상태 변경 event 발행

ACEnemy
- 사망 생명주기 조정
- 신규 의도 차단 연결
- FinalizeDeath 단일 진입점
- 최종 Destroy 요청

UCReactionOrchestratorComponent
- 확정된 Dead Reaction 실행 가능 여부와 충돌 해결

UCReactionComponent / UCReaction_Dead
- Dead Reaction 실행
- Started / Completed / Interrupted 생명주기 통지

Finalize Death Anim Notify
- 연출상 확정된 시점에 Enemy의 FinalizeDeath 요청
- 세부 컴포넌트를 직접 정리하지 않음

각 Runtime Component
- 자신의 timer / delegate / collision / spawned runtime state 정리

EndPlay
- Actor와 Component teardown
- gameplay finalization과 중복 호출돼도 안전해야 함
```

## 제외 범위

- Player Respawn / Game Over / 실제 Player Destroy
- Corpse 유지 시간 정책
- Ragdoll
- Actor Pooling
- Dead Reaction 시스템 교체
- TargetingComponent가 Enemy 사망 정책을 직접 조회하는 결합
- 단일 사망 기능만을 위한 범용 생명주기 프레임워크 도입
- Focus Locomotion 정책과 백업 자산 복구

## Merge 전 목표모드 계획

```text
Goal 1
Runtime 구조 감사 + C++ 구현 + Development Build + Local Commit

Goal 2
사용자 Editor Asset 연결 + PIE 통합 검증 + 필요한 최소 보완

Goal 3
최종 구조 감사 + 문서 동기화 + Build + Commit + Push + Draft PR

Goal 4
외부 리뷰 대응 + 재검증 + Thread 정리 + Merge Ready 확정

사용자
일반 Merge 방식으로 main 병합
```

---

## Goal 1. Runtime 구현과 Editor Handoff

### 작업 범위

- Dead 처리 경로 전수조사
- 기존 시스템이 이미 차단하거나 정리하는 범위 식별
- Enemy 사망 3단계 계약과 실제 코드의 정합성 확인
- 멱등적인 `FinalizeDeath()` 경로 구현
- Finalize Death Anim Notify 구현
- Dead Reaction Reject / Interrupt / Notify 누락 fallback 구현
- Targeting `OnEndPlay` 연계 검토
- `git diff --check`
- `PortfolioEditor Win64 Development` 빌드
- 의미 단위 Local Commit
- 사용자 Editor 작업 목록 작성

### 중단 조건

다음 중 하나가 발견될 때만 큰 구조 변경을 멈추고 먼저 보고한다.

- Reaction Started / Completed / Interrupted를 Dead 실행과 식별할 수 없음
- Reaction callback 내부 Destroy가 현재 실행 상태를 안전하게 종료할 수 없음
- 기존 EndPlay cleanup이 멱등하지 않아 단순 보완으로 해결할 수 없음
- Enemy의 사망 주도권을 어느 객체가 갖는지 현재 구조와 충돌함

### 실행 프롬프트

````text
현재 `feature/dead-actor-destroy-flow` 브랜치에서 Enemy Dead / Destroy Runtime 구현을 목표모드로 시작해줘.

기준:
- 현재 HEAD는 `main`의 `7a0463b9`에서 분기했다.
- 기존 워크트리 변경은 보존한다.
- 이번 목표에서는 UAsset이나 에디터 데이터를 수정하지 않는다.
- 기존 Action / Reaction / Health / AI / Movement / Weapon / EndPlay 구조를 우선 재사용한다.
- 새로운 컴포넌트는 기존 책임으로 해결할 수 없는 근거가 확인될 때만 제안한다.
- 기준 문서는 `Docs/06_notes/task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md`다.

구현 전에 다음 P0를 전수조사해줘.

1. Health의 Alive → Dying → Dead 전이와 OnDeadStateChanged 구독 구조
2. Dead Reaction 요청, Intervention, Started / Completed / Interrupted 경로
3. 현재 Dead 상태가 AI Intent, Action 요청, Movement 입력을 어디까지 차단하는지
4. Weapon Collision, Hit Window, Combat Signal, Balance Timer 등 독립 Runtime 상태의 cleanup 경로
5. Enemy Destroy와 EndPlay 중복 정리가 안전한지
6. Dead Reaction callback 내부에서 즉시 Destroy할 때 재진입 문제가 있는지
7. TargetingComponent의 OnEndPlay 해제가 Enemy Destroy와 정상 결합되는지

P0에서 큰 구조 충돌이 발견되면 임의로 재설계하지 말고 근거와 최소 권장안을 먼저 보고해줘. 충돌이 없다면 문서의 확정 정책으로 자율 구현해줘.

정책:

```text
1차
Dying 진입
→ Enemy가 신규 실행 의도를 차단
→ 기존에 이미 존재하는 차단은 중복 구현하지 않음

2차
Dead Reaction
→ 기존 Action / Reaction Intervention으로 활성 실행 정리

3차
Dead Montage 후반 Finalize Notify
→ 최종 gameplay cleanup
→ Enemy Destroy
```

Fallback:

```text
Dead Reaction Rejected
Dead Reaction 비정상 Interrupted
Finalize Notify 누락 후 Reaction Completed
→ 동일한 FinalizeDeath 경로
```

요구사항:

- FinalizeDeath는 여러 경로에서 호출돼도 한 번만 실행되는 멱등 API로 만든다.
- Notify는 Enemy의 공개된 사망 마감 요청 API만 호출하고 세부 컴포넌트를 직접 정리하지 않는다.
- Reaction Completed는 정상 Notify 누락 fallback으로 사용한다.
- 정상적인 Dead Reaction 흐름과 비정상 Interrupt를 구분한다.
- Destroy 직전 필요한 gameplay cleanup과 EndPlay teardown의 책임을 분리한다.
- Player는 실제 Destroy하지 않는다.
- 기존 Targeting OnEndPlay 계약을 변경하지 않고 통합 가능한지 확인한다.
- 설계와 구현이 달라진 부분은 기준 문서에 반영한다.
- `git diff --check`와 `PortfolioEditor Win64 Development` 빌드를 수행한다.
- 검증 성공 후 의미 단위로 커밋하되 Push하지 않는다.

완료 시 다음을 보고해줘.

1. 기존에 이미 처리되고 있던 사망 정리
2. 새로 추가한 책임과 API
3. 변경 파일과 커밋
4. 빌드 결과
5. 사용자가 에디터에서 배치해야 할 Notify와 위치
6. PIE 검증 시나리오
7. 남은 위험이나 결정 사항
````

---

## Goal 2. Asset 연결과 PIE 통합 검증

### 작업 범위

사용자가 다음 에디터 작업을 담당한다.

- Enemy Dead Montage 후반에 Finalize Death Notify 배치
- 필요한 Blueprint / ReactionData 연결 확인
- PIE 실행과 결과 전달

에이전트는 정확한 배치 위치와 검증 순서를 안내하고, 결과에 따라 필요한 최소 코드 보완을 수행한다.

### 검증 항목

```text
정상 사망
-> Dying
-> Dead Reaction
-> Finalize Notify
-> Destroy

Fallback
-> Finalize Notify 누락
-> Dead Reaction Completed
-> Destroy

Targeting
-> 현재 Target Enemy 사망 / Destroy
-> Target 해제와 Marker / Lock Assist 정리
-> event 중복 없음

Switching
-> A에서 B로 Target 변경
-> A Destroy
-> B 유지
```

### 실행 프롬프트

````text
Enemy Dead / Destroy Runtime 구현의 에디터 자산 연결과 PIE 통합 검증을 목표모드로 이어서 진행해줘.

기준 문서:
- `Docs/06_notes/task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md`

너는 먼저 현재 코드와 직전 구현 보고를 확인하고, 내가 에디터에서 수행해야 할 작업을 정확한 순서로 안내해줘.

내가 담당할 작업:

- Enemy Dead Montage에 Finalize Death Notify 배치
- 필요한 Blueprint 또는 ReactionData 연결
- PIE 실행과 관찰 결과 전달

검증 대상:

1. 정상 Dead Reaction 후 Notify 시점에 Enemy가 Destroy되는지
2. 사망 연출이 Destroy 때문에 너무 일찍 잘리지 않는지
3. 현재 Target Enemy가 Destroy되면 Target이 정확히 한 번 해제되는지
4. 사망으로 이미 Target이 해제된 뒤 Destroy되어도 중복 OnTargetChanged가 없는지
5. A → B 전환 후 A를 Destroy해도 B가 유지되는지
6. Target Marker와 Lock Assist가 Target 해제와 함께 정리되는지
7. AI Focus, 이동, 공격, Weapon Collision이 사망 후 남지 않는지
8. 반복 호출 또는 fallback에서 중복 Destroy나 크래시가 없는지

검증 중 코드 문제가 확인되면 원인을 분석하고 필요한 최소 수정까지 진행해줘. 에디터 자산을 직접 임의 수정하지 말고, 필요한 자산 변경은 나에게 안내해줘.

모든 항목이 확인되기 전에는 목표를 완료 처리하지 말고, 확인된 항목과 남은 항목을 구분해서 관리해줘.
````

---

## Goal 3. 최종 감사, 문서, Push와 Draft PR

### 작업 범위

- Runtime / UAsset 변경 범위 감사
- 프로젝트 API 배치, 섹션, 책임, 가시성 규칙 검토
- N14와 W05 현재 Task Brief 동기화
- Feature PR 문서 작성
- `git diff --check`
- Development Build
- 책임 단위 Commit
- 원격 `main` 동기화 확인
- 일반 Merge 전제의 Branch Push
- 한국어 Draft PR 생성

P58은 당시 Feature PR의 역사적 기록이므로 수정하지 않는다. W05의 현재 상태를 설명하는 README와 Task Brief는 `OnEndPlay` 기준으로 갱신한다.

### 실행 프롬프트

````text
`feature/dead-actor-destroy-flow` 작업을 머지 준비 상태로 마감하고 Draft PR까지 생성하는 작업을 목표모드로 진행해줘.

전제:
- Runtime 구현과 사용자 에디터 자산 연결이 완료됐다.
- PIE 검증 결과를 현재 대화에서 확인한다.
- 머지 방식은 일반 Merge다.
- Push와 Draft PR 생성을 승인한다.
- 실제 main Merge는 수행하지 않는다.
- 기준 문서는 `Docs/06_notes/task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md`다.

진행 순서:

1. 워크트리 전체 변경 파일과 diff를 감사한다.
2. 기존 프로젝트의 API 배치, 섹션 규칙, 책임 경계, 가시성 규칙과 비교한다.
3. Dead / Destroy 정상 경로와 fallback이 중복 실행에 안전한지 검토한다.
4. Targeting OnEndPlay 통합 계약을 다시 검토한다.
5. 관련 문서를 현재 구현 기준으로 갱신한다.
   - N14 Dead Destroy 후속 문서
   - W05 현재 Task Brief와 README
   - W05의 OnDestroyed 표현을 OnEndPlay 계약으로 갱신
   - P58은 당시 PR 기록이므로 수정하지 않음
6. 기존 PR 문서 양식과 최근 Feature PR을 조사한다.
7. 다음 유효한 P 번호로 한국어 Feature PR 문서를 작성한다.
8. `git diff --check`를 수행한다.
9. `PortfolioEditor Win64 Development` 빌드를 수행한다.
10. 변경을 책임 단위로 커밋한다.
11. `origin/main`을 fetch하고 branch divergence를 확인한다.
12. 원격 main 변경이 있으면 일반 Merge 방식으로 반영하고 필요한 재검증을 수행한다.
13. Branch를 Push한다.
14. 로컬 PR 문서를 본문 기준으로 사용해 한국어 Draft PR을 생성한다.

PR에는 다음을 명시한다.

- Enemy 사망 3단계 구조
- 기존 Action / Reaction 재사용 범위
- Finalize Notify와 fallback
- 멱등적인 최종 정리
- EndPlay와 gameplay cleanup 책임 구분
- Targeting Destroy 통합 검증
- Player Destroy 제외 범위
- 빌드 및 PIE 검증 결과
- 수동 자산 변경 내용

완료 시 다음을 보고해줘.

- 최종 커밋 목록
- Push 결과
- Draft PR 번호와 URL
- 빌드 및 PIE 검증
- 알려진 제한사항
- 리뷰에서 중점적으로 볼 부분
````

---

## Goal 4. 리뷰 대응과 Merge Ready 확정

### 작업 범위

- 모든 Review와 미해결 Thread 조회
- 지적의 타당성, 재현 가능성, 위험도 분석
- 필요한 코드 / 문서 보완
- Build와 회귀 검증
- Commit / Push
- 기존 한국어 대응 양식에 맞춘 Review 댓글
- 해결된 Thread Resolve
- PR 본문과 로컬 문서 동기화
- Merge 가능 상태 확인

### 실행 프롬프트

````text
Enemy Dead / Destroy Feature PR의 리뷰 대응과 Merge Ready 검증을 목표모드로 진행해줘.

권한:
- GitHub 리뷰 스레드 조회를 승인한다.
- 타당한 리뷰 보완을 위한 코드와 문서 수정을 승인한다.
- 검증된 수정의 Commit과 Push를 승인한다.
- 각 리뷰 스레드에 한국어 대응 댓글을 작성하는 것을 승인한다.
- 대응이 완료된 리뷰 스레드의 Resolve를 승인한다.
- 실제 main Merge는 수행하지 않는다.

진행 순서:

1. PR의 모든 리뷰와 미해결 Thread를 조회한다.
2. 각 지적을 코드와 문서에 대조해 타당성, 위험도, 수정 필요성을 분류한다.
3. 타당한 지적은 기존 구조의 책임 경계를 유지하는 최소 변경으로 수정한다.
4. `git diff --check`와 필요한 Development Build를 수행한다.
5. 수정 내용을 책임 단위로 Commit하고 Push한다.
6. 기존 프로젝트의 한국어 리뷰 대응 양식을 조사한다.
7. 각 Thread에 다음 내용을 포함해 대응한다.
   - 어떤 문제였는지
   - 어떻게 수정했는지
   - 왜 해당 방식인지
   - 어떤 검증을 통과했는지
   - 반영 커밋
8. 해결된 Thread를 Resolve한다.
9. PR 본문과 로컬 PR 문서가 최종 구현과 일치하는지 확인한다.
10. CI, 미해결 Thread, Merge conflict, Worktree 상태를 확인한다.

완료 기준:

- 타당한 리뷰 지적이 모두 반영됨
- 미해결 Review Thread 없음
- Build 성공
- PR 본문과 문서가 최신
- Branch가 origin과 동기화됨
- Merge conflict 없음
- Worktree Clean
- 실제 Merge만 사용자에게 남은 상태

완료 후 일반 Merge 가능 여부와 남은 사용자 작업을 보고해줘.
````

## 최종 완료 조건

```text
- Enemy Dying에서 신규 전투 의도가 차단된다.
- Dead Reaction이 기존 Intervention 경로로 실행된다.
- Finalize Notify에서 최종 cleanup과 Destroy가 실행된다.
- Reject / 비정상 Interrupt / Notify 누락에서 fallback이 실행된다.
- FinalizeDeath는 중복 호출에 안전하다.
- Destroy와 EndPlay cleanup이 충돌하지 않는다.
- 현재 Target Destroy가 정확히 한 번 Target을 해제한다.
- 사망 해제 후 Destroy에서 Target event가 중복되지 않는다.
- 이전 Target Destroy가 새 Target을 해제하지 않는다.
- Marker / Lock Assist / Debug Focus가 Target과 함께 정리된다.
- Player 실제 Destroy는 포함하지 않는다.
- Development Build와 지정 PIE 검증을 통과한다.
- 문서와 PR 본문이 구현과 일치한다.
- 미해결 Review Thread와 Merge conflict가 없다.
- 실제 일반 Merge만 사용자에게 남는다.
```

## 후속 작업

W06-01 Merge 이후 다음 작업을 진행한다.

```text
backup/focus-locomotion-policy
-> 최신 main에서 feat/focus-locomotion-policy 생성
-> 백업 커밋 0daf7bf7 검토 후 cherry-pick
-> 비포커스 1D + Sprint / 포커스 8Way 정책 구현
```
