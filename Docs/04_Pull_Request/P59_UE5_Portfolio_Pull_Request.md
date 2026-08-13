# UE5 Portfolio Pull Request

## 제목

**P59: Enemy Death Presentation and Destroy Lifecycle**

## 날짜

**2026.08.13**

## 상태

- [x] Health 생명 상태를 `Alive / Dead`로 단순화
- [x] DeadIn Reaction과 DeadLoop Locomotion 분리
- [x] Enemy 사망 진입 시 신규 실행 의도와 독립 Runtime 정리
- [x] 명시적인 Reaction Complete Notify와 Montage fallback 대칭화
- [x] Character / SkeletalMesh Weapon Dissolve Presentation 연결
- [x] Character Niagara 자연 완료 기반 Enemy Destroy
- [x] Presentation 미구현·시작 실패 fallback 잔존 시간 보장
- [x] Destroy 직전 gameplay cleanup과 EndPlay teardown 분리
- [x] Targeting의 Actor EndPlay 해제 경계 통합 검증
- [x] Death Lifecycle Debug Overlay와 계약 위반 진단 추가
- [x] 관련 Architecture / Note / Work List 문서 동기화
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [x] 사용자 PIE 통합 검증 완료

## 브랜치

- Base: `main`
- Branch: `feature/dead-actor-destroy-flow`
- Base HEAD: `7a0463b9 Merge pull request #114 from Razria93/fix/targeting-stale-target-lifecycle`
- Implementation HEAD: `fc7d5b84 docs(dead): synchronize lifecycle review documentation`
- Merge Policy: 일반 Merge Commit

## 대표 스크린샷

이번 PR은 사용자 PIE에서 DeadIn, DeadLoop, Character / Weapon Dissolve, Niagara 완료 후 Destroy와 Debug Overlay를 확인했다.

별도 screenshot evidence 파일은 PR 문서에 추가하지 않고, 연결된 AnimBP / Montage / Material / Niagara / Blueprint 자산과 W06 검증 기록을 근거로 둔다.

## 요약

이번 PR은 Enemy의 HP가 0이 된 순간부터 DeadIn 연출, DeadLoop 자세, Dissolve 표현과 최종 Actor 제거까지를 하나의 설명 가능한 생명주기로 연결한다.

Health는 `Alive / Dead` 판정만 소유하고, Reaction은 사망 진입 동작을, AnimBP는 사망 후 기본 자세를, CharacterFeedback은 Character와 Weapon의 Dissolve를 담당한다. Enemy는 각 결과를 조정하되 표현 세부 구현을 알지 않고 최종 gameplay cleanup과 `Destroy()` 권한만 소유한다.

정상 Destroy 시점은 고정 시간으로 추측하지 않는다. 필수 Character Niagara가 자연 수명을 모두 소비한 `OnSystemFinished`를 정상 완료 신호로 사용한다. 표현이 구현되지 않았거나 생성에 실패한 경우에만 DeadLoop가 일정 시간 잔존한 뒤 fallback으로 정리한다.

## 핵심 개념

이번 PR의 핵심은 생명 상태, 실행 동작, 지속 자세, 시각 표현과 Actor 수명을 서로 다른 책임으로 분리하면서 하나의 종료 흐름으로 결합하는 것이다.

### DeadIn

Enemy가 사망 상태로 진입할 때 재생하는 FullBody Reaction Montage다. 기존 Action / Reaction Orchestrator와 Intervention을 그대로 사용한다.

### DeadLoop

Health가 이미 Dead인 동안 AnimBP가 선택하는 Locomotion base pose다. DeadIn이 종료되거나 시작하지 못해도 사망 자세를 유지한다.

### Death Presentation

CharacterFeedback이 Blueprint에 요청하는 Dissolve / Niagara 표현 구간이다. Character가 정상 완료 권한을 가지며 Weapon은 같은 Timeline을 따르는 동기 표현 참여자다.

### FinalizeDeath

Enemy가 한 번만 수행하는 최종 종료 경로다. gameplay runtime을 정리하고 callback stack 밖의 다음 Tick에 Actor를 Destroy한다.

## 변경 배경

기존 사망 구조는 `Alive -> Dying -> Dead` 상태 전이와 Montage Notify에 생명 상태 확정 및 Destroy 시점을 함께 맡겼다. 그 결과 생명 판정과 애니메이션 구간이 중복되고, DeadIn이 Reaction 파이프라인에 들어가지 않거나 Notify가 없으면 즉시 파괴 또는 DeadLoop만 노출되는 문제가 있었다.

또한 Timeline 종료만으로 Actor를 파괴하면 Character 주변에 남아 흩어져야 하는 Niagara 파티클이 수명을 소비하지 못하고 함께 사라진다. 반대로 모든 상황을 고정 Timer로 처리하면 정상 연출을 임의로 잘라 시각적 완결성을 해친다.

이번 PR은 생명 상태를 `Alive / Dead`로 축소하고 DeadIn / DeadLoop / Dissolve / Destroy를 독립 책임으로 나눈 뒤, 실제 표현 완료 이벤트를 기준으로 Actor 수명을 닫는다.

## 주요 변경

### 1. 생명 상태와 실행 상태 분리

대상:

```text
Source/Portfolio/Type/CHealthTypes.h
Source/Portfolio/Component/CHealthComponent.*
Source/Portfolio/Component/CStateComponent.*
Source/Portfolio/Character/CAnimInstance.*
```

변경 내용:

- `EDeadState`를 `Alive / Dead`로 축소했다.
- 기존 UAsset 직렬화 호환을 위해 `Dead = 2` 값은 유지했다.
- `Dying / Reviving`과 부활 API 및 Health 상태 전환 Notify를 제거했다.
- Dead를 Action / Reaction 실행 상태로 투영하지 않는다.
- AnimInstance는 Health 이벤트를 구독하고 표현용 `bIsDead` 캐시만 유지한다.
- AnimBP는 `bIsDead`로 Alive Locomotion과 DeadLoop를 선택한다.

### 2. DeadIn Reaction과 정상 완료 계약

대상:

```text
Source/Portfolio/Reaction/CReaction_Dead.*
Source/Portfolio/Component/CReactionComponent.*
Source/Portfolio/Notify/CAnimNotify_CompleteReaction.*
Source/Portfolio/Type/CReactionOrchestrationTypes.h
Content/04_Montage/Damaged/M_Dead_In.uasset
```

변경 내용:

- DeadIn은 기존 Reaction Orchestrator와 Intervention 규칙으로 요청한다.
- FullBody Montage의 `Complete Reaction` Notify가 정상 완료를 명시한다.
- non-interrupted MontageEnded는 Complete Notify 누락 fallback이다.
- interrupted MontageEnded는 정규 Stop을 대신하지 않고 계약 위반 Audit만 남긴다.
- Montage와 Play Serial을 함께 확인해 오래된 callback을 무시한다.
- Dead Reaction의 Started / Completed / Interrupted / Ignored 결과를 Enemy 생명주기에 연결한다.

### 3. Enemy 최종 생명주기와 멱등 Destroy

대상:

```text
Source/Portfolio/Character/Enemy/CEnemy.*
Source/Portfolio/Component/CMovementComponent.*
Source/Portfolio/Component/CWeaponComponent.*
```

변경 내용:

- Health의 `Alive -> Dead` 이벤트로 Death Lifecycle을 시작한다.
- 신규 AI / Action / Movement 의도를 차단하고 deferred action, movement, weapon hit runtime을 정리한다.
- DeadIn이 완료되면 Death Presentation을 요청한다.
- DeadIn이 거절되거나 정규 Stop 경로에서 중단되면 같은 Presentation fallback 흐름으로 합류한다.
- `RequestFinalizeDeath()`와 `FinalizeDeath()`를 여러 경로에서 호출해도 한 번만 실행되도록 구성했다.
- Destroy 직전 gameplay cleanup과 Actor / Component의 EndPlay teardown을 구분했다.
- callback 실행 중 직접 Destroy하지 않고 다음 Tick에 수행해 재진입을 피한다.

### 4. Character / Weapon Dissolve Presentation

대상:

```text
Source/Portfolio/Type/CCharacterFeedbackTypes.h
Source/Portfolio/Component/CCharacterFeedbackComponent.*
Source/Portfolio/Component/CWeaponComponent.*
Source/Portfolio/Weapon/CWeaponActor.h
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/06_Weapon/BP_CWeaponActor_Sword.uasset
Content/07_FX/Niagara/P_DissolveEdge_SK_1.uasset
Character / Weapon Dissolve Material assets
```

변경 내용:

- CharacterFeedback은 Blueprint에 Presentation 시작을 요청한다.
- Blueprint는 실제 표현 자원 생성 결과를 `Started / Unavailable / Finished`로 반환한다.
- Character와 현재 SkeletalMesh Weapon은 같은 Timeline 값을 받아 함께 Dissolve된다.
- Weapon은 독립 완료 barrier나 Enemy Destroy 권한을 갖지 않는다.
- 필수 Character Niagara가 생성에 실패하면 `Unavailable`을 통지해 fallback delay를 유지한다.
- 정상 시작 후에는 Timer를 해제하고 Character Niagara의 `OnSystemFinished`가 최종 완료를 통지한다.

### 5. AI·Targeting·Debug 종료 경계

대상:

```text
Content/01_Character/02_Enemy/AI/BehaviorTree/State/BT_Dead.uasset
Content/01_Character/02_Enemy/AI/BehaviorTree/State/BT_HitReact.uasset
Source/Portfolio/Core/Debug/FDeathLifecycleDebug.*
Source/Portfolio/Core/Debug/FDebugOverlay*.*
Plugins/PortfolioDebugOverlayEditor/Source/*
Docs/06_notes/task_briefs/W05_Player_Targeting/*
```

변경 내용:

- Dead BT는 Focus를 비우고 사망 상태에서 대기한다.
- HitReact BT는 Reaction 종료를 관찰하며 Reaction 자체를 다시 시작하지 않는다.
- Enemy Destroy는 Actor `OnEndPlay`로 Targeting에 전달된다.
- Targeting은 weak index / serial identity를 확인해 이전 Target의 늦은 callback이 새 Target을 해제하지 않게 한다.
- Death Lifecycle의 Health, DeadIn, Presentation, fallback과 finalization 상태를 Debug Overlay에 표시한다.
- 계약 위반은 Overlay에 항상 남기고 `Portfolio.Debug.DeathLifecycleAudit`이 활성화되면 Output Log에도 복제한다.

## 주요 처리 흐름

### 정상 흐름

```text
Damage Commit
-> Health Alive -> Dead
-> Enemy Death Lifecycle 시작
-> 신규 실행 의도와 독립 Runtime 정리
-> AnimBP DeadLoop base pose 준비
-> DeadIn Reaction Started
-> Complete Reaction Notify
-> DeadIn Completed
-> Death Presentation 요청
-> Character Niagara 생성 성공
-> Character / Weapon Dissolve
-> Character Niagara OnSystemFinished
-> Presentation Finished
-> 다음 Tick gameplay cleanup / Enemy Destroy
-> EndPlay teardown
```

### Presentation fallback

```text
Presentation listener 없음
또는 필수 Character Niagara 생성 실패
-> Presentation Unavailable
-> DeadLoop fallback 잔존 시간 유지
-> Finalize 요청
-> 다음 Tick Enemy Destroy
```

### DeadIn 비정상 경로

```text
DeadIn Rejected / 미시작
또는 정규 Stop 경로의 Interrupted / Ignored
-> Death Presentation 진입 시도
-> 정상 Started 또는 Unavailable 결과에 따라 동일한 종료 흐름 사용
```

## 변경 파일 범위

```text
Content
- Enemy / Player / AI performance Blueprint
- AnimBP / Dead animation / DeadIn Montage
- Character / Weapon SkeletalMesh와 Dissolve Material
- Weapon Actor Blueprint
- Niagara Dissolve effect
- TestRoom

Source/Portfolio
- Enemy / AnimInstance
- Health / State / Movement / Weapon / CharacterFeedback
- Action / Reaction / Orchestrator
- Reaction Complete Notify
- Health / State / Feedback / Orchestration types
- Death Lifecycle Debug Overlay
- 제거된 Revive / WaitDeadState / Health Notify 코드

Plugins/PortfolioDebugOverlayEditor
- Death audit CVar와 Editor panel

Docs
- S31 current contract
- 관련 Architecture 경계 문서
- N14 follow-up index
- W05 Targeting 통합 검증
- W06 구현·자산·PIE 기록
```

## 테스트 방법

### Static check

```powershell
git diff --check main...HEAD
```

### Build

```text
PortfolioEditor Win64 Development
```

### PIE

```text
1. Enemy HP를 0으로 만들어 Alive -> Dead 단일 전이를 확인한다.
2. DeadIn FullBody Montage와 DeadLoop 자세가 자연스럽게 연결되는지 확인한다.
3. Character와 장착 Weapon이 같은 Timeline으로 Dissolve되는지 확인한다.
4. Character Niagara의 잔여 파티클이 자연 종료된 뒤 Enemy가 파괴되는지 확인한다.
5. Presentation 미구현 또는 생성 실패에서 fallback 시간 후 파괴되는지 확인한다.
6. 사망 후 AI, Action, Movement와 Weapon hit runtime이 재개되지 않는지 확인한다.
7. 현재 Target Enemy Destroy 시 Target / Marker / Lock Assist가 한 번만 해제되는지 확인한다.
8. A에서 B로 Target을 전환한 뒤 A가 Destroy되어도 B가 유지되는지 확인한다.
9. Debug Overlay Death EventLog와 계약 위반 진단 CVar를 확인한다.
```

## 검증 결과

사용자 PIE 확인 완료:

```text
PASS - Health Alive -> Dead 단일 전이
PASS - DeadIn FullBody Reaction과 DeadLoop 자연 연결
PASS - Character / Weapon Dissolve
PASS - Character Niagara 자연 완료 후 다음 Tick Destroy
PASS - Presentation 미구현 / 생성 실패 fallback
PASS - 신규 AI / Action / Movement 의도 차단
PASS - Weapon collision / hit runtime cleanup
PASS - 현재 Target Destroy 단일 해제
PASS - 사망 선행 해제 후 Destroy 중복 event 없음
PASS - 이전 Target Destroy가 새 Target을 해제하지 않음
PASS - Marker / Lock Assist / Debug Focus 정리
PASS - Death Overlay 상태와 EventLog Filter
```

정적 검증:

```text
PASS - PortfolioEditor Win64 Development build
PASS - git diff --check
PASS - 문서 Markdown fence / 상대 링크 검사
```

## 설계 판단 기준

- Health의 생명 상태와 Action / Reaction 실행 상태를 합치지 않는다.
- DeadIn Montage는 Dead 판정의 원본이 아니라 이미 확정된 사망을 표현하는 Reaction이다.
- DeadLoop는 DeadIn 성공 여부와 무관하게 사망 자세를 보장한다.
- 정상 Presentation은 고정 Timer로 자르지 않고 실제 표현 자원의 완료를 기다린다.
- fallback delay는 정상 Presentation watchdog이 아니라 미구현·시작 실패 시 최소 잔존 시간을 보장한다.
- Character가 Presentation 완료 권한을 가지며 Weapon은 동기 표현 참여자로 둔다.
- Feedback Component는 표현 결과를 전달하지만 Actor Destroy 권한은 갖지 않는다.
- Enemy의 finalization은 멱등이며 callback 재진입을 피한다.
- EndPlay는 gameplay 연출을 시작하지 않고 teardown만 수행한다.

## Scope Guard

이번 PR에서 하지 않은 것:

- Player 사망 후 Destroy / Game Over
- Respawn / Revive
- Ragdoll
- Actor Pooling과 영구 시체 정책
- Execution 협업 사망
- Active Niagara를 고정 Timer로 강제 종료하는 정책
- Weapon별 독립 Presentation 완료 barrier
- 모든 Actor에 적용하는 범용 Death Lifecycle Framework
- 범용 Action / Reaction Execution cleanup 재설계
- Focus Locomotion 1D / 8Way 정책
- Enemy Focus / 공격 Target / 피격 후 Engage 정책
- Enemy Status HUD와 Player Resource HUD

## 리스크 / 리뷰 포인트

- `EDeadState::Dead`의 직렬화 값 `2`가 기존 UAsset 호환을 위해 유지되는지 확인한다.
- DeadIn의 정상 종료와 interruption이 기존 S26 Complete / Stop 계약을 우회하지 않는지 확인한다.
- Presentation Started 이후 fallback Timer가 해제되고 정상 Niagara 완료 권한이 보존되는지 확인한다.
- Presentation Unavailable에서만 fallback delay가 최종 종료를 요청하는지 확인한다.
- Weapon Dissolve 실패가 Character의 정상 완료를 막는 독립 barrier로 변하지 않았는지 확인한다.
- `RequestFinalizeDeath()`의 중복 호출이 한 번의 cleanup / Destroy로 수렴하는지 확인한다.
- Destroy 직전 gameplay cleanup과 EndPlay teardown이 같은 연출을 중복 실행하지 않는지 확인한다.
- Targeting의 EndPlay callback이 Actor instance identity를 검증하는지 확인한다.
- Binary asset 변경이 DeadIn / DeadLoop / Dissolve / AI 대기 / TestRoom 검증 범위에 한정되는지 확인한다.

## 후속 작업

다음 branch에서 우선 검토:

1. `backup/focus-locomotion-policy` 기준 비포커스 1D / 포커스 8Way Locomotion 정책
2. Enemy Focus, 공격 Target과 피격 후 Engage 정책
3. Enemy Status HUD의 Name / HP / Balance 표현
4. N14의 범용 Action / Reaction Execution cleanup 경계

다음 기능이 실제 요구될 때 별도 설계:

```text
Player Game Over / Respawn
Enemy Revive
Ragdoll / Pooling / 영구 시체
Execution 협업 사망
```

## 관련 문서

- `Docs/05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md`
- `Docs/05_System_Architecture/S26_UE5_Portfolio_System_Architecture.md`
- `Docs/06_notes/N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md`
- `Docs/06_notes/task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/README.md`
- `Docs/06_notes/task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md`
- `Docs/06_notes/task_briefs/W05_Player_Targeting/README.md`

## 대표 커밋

```text
c74b7c33 docs(dead): add enemy destroy lifecycle plan
4582dbe4 feat(dead): add enemy death finalization lifecycle
0bd640e2 feat(reaction): add explicit reaction completion notify
72e45573 refactor(dead): finalize enemy death lifecycle
7e707f9d assets(dead): configure dead-in reaction and locomotion
63951d08 feat(dead): coordinate dissolve presentation finalization
de6b043d assets(dead): configure character and weapon dissolve presentation
814b3cac feat(debug): expose death lifecycle diagnostics
6a05a7c5 docs(dead): update presentation and diagnostics contract
fc7d5b84 docs(dead): synchronize lifecycle review documentation
```

## 정리

이번 PR은 Enemy 사망을 단순한 Montage 재생이나 지연 Destroy가 아니라, Health 판정에서 DeadIn / DeadLoop / Dissolve와 최종 Actor 종료까지 이어지는 명시적 생명주기로 정리한다.

각 계층은 자기 책임만 수행하고, 정상 표현은 Character Niagara의 자연 완료를 존중하며, 표현을 시작할 수 없는 경우만 fallback 잔존 시간으로 안전하게 종료한다. 이로써 사망 연출의 시각적 완결성과 Runtime cleanup, Targeting 종료 경계를 함께 보장한다.
