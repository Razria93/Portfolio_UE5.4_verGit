# UE5 Portfolio Pull Request

## 제목

**P61: Enemy Balance Collapse and Execution Collaboration**

## 날짜

**2026.09.04**

## 상태

- [x] Enemy Target 측 `UCBalanceComponent`와 Balance / Collapse lifecycle 도입
- [x] Parry / Damage Result 기반 Balance 반영, CollapseIn / Loop / Out, CollapseHit 연결
- [x] Source × Target `UCExecutionCollaborationComponent` pair-session 도입
- [x] Target-owned Standard / Lethal outcome, reservation, commit, terminal 계약 연결
- [x] Standard Execution Down / Recovery와 Lethal Health / Death handoff 반영
- [x] 활성 execution pair의 participant movement collision ignore 및 종료 복구
- [x] Execution Action/Reaction data, Montage Notify, AnimBP pose, presentation asset 반영
- [x] Action / Reaction과 Execution Session Event category 분리
- [x] Actor 인스턴스 기준 Event history와 Focused Enemy Event Log 도입
- [x] Debug Overlay Settings Registry 및 Editor panel 연결
- [x] Facing / AI actor-scoped diagnostics와 stale Combat display 정리
- [x] `git diff --check`, `PortfolioEditor Win64 Development` build, 사용자 PIE 핵심 흐름 확인

## 브랜치

- Base: `main`
- Branch: `feat/balance-collapse-execution`
- Base HEAD: `b1bbee027337aa0f04815535f43df38672711057`
- Implementation HEAD: `edf729ffcf388b219e57d0c1482b930b5e7c55f2` — `docs: align execution and overlay operations`
- Merge Policy: 일반 Merge Commit

## 대표 스크린샷

이번 PR은 TestRoom PIE의 Character Details / Event Log와 Execution montage를 실행 근거로 사용한다.
별도 screenshot evidence 파일은 추가하지 않으며, Player/Enemy Blueprint, AnimBP, montage, Debug Overlay와
아래 검증 기록을 근거로 둔다.

## 요약

이번 PR은 Enemy가 받은 Parry / Damage Result를 단발성 reaction만으로 소비하지 않고, Target-owned
Balance lifecycle로 누적하도록 전환한다. Balance threshold를 처음 넘으면 Collapse In / Loop 수명에
진입하며, Collapse Loop는 단순 prone pose가 아니라 Execution opportunity를 제공하는 lifecycle 구간이 된다.

Execution은 일반 Action 하나가 아니라 Source와 Target이 reservation, outcome, commit, terminal을 공유하는
pair transaction이다. Target이 자신의 상태로 Standard 또는 Lethal outcome을 한 번 결정하고, Standard는
Balance-owned Down / Recovery로, Lethal은 기존 Health / Death presentation으로 handoff한다. 따라서
Health, Balance, Action / Reaction, Death가 동일 state를 중복 소유하지 않는다.

동시에 Debug Overlay를 gameplay 권위와 분리된 관찰 계층으로 정리했다. Event Log는 일반
Action / Reaction 판단과 Execution Session lifecycle을 다른 category로 기록하며, Actor 인스턴스 단위의
history를 통해 Focused Enemy 범위에서 정확한 원인 흐름을 조회한다.

## 핵심 개념

### Balance Lifecycle

`UCBalanceComponent`가 Enemy Target의 Balance count, threshold, Collapse / Execution 상태, Loop TTL,
reservation 및 reset을 소유하는 lifecycle이다. Combat Signal은 result packet을 검증·routing할 뿐 Count와
presentation 수명을 직접 소유하지 않는다.

### Execution Opportunity

Execution 가능 여부는 별도 bool source of truth가 아니다.

```text
BalanceLifecycle == CollapseLoopActive
AND active execution reservation 없음
```

일 때만 `IsExecutionOpportunityAvailable()`가 true가 된다. reservation은 opportunity를 잠시 보호하며,
Commit 전 취소에서는 보관한 Collapse Loop TTL을 재개할 수 있다.

### Execution Session

`UCExecutionCollaborationComponent`가 Source × Target의 pair relationship만 소유한다. session은
Target snapshot / revision, outcome, reservation, Source Action serial, Target Reaction terminal을
연결하며, 장기 Down 또는 Dead presentation을 직접 소유하지 않는다.

### Standard / Lethal Outcome

Execution outcome은 Source 입력의 옵션이 아니라 Target이 reservation 수락 시 자신의 Health와
`LethalCondition`으로 결정하는 정책이다.

- `Standard`: non-lethal damage 후 Balance `ExecutionDownActive` 및 `ExecutionRecovery`
- `Lethal`: Source Commit impact frame에서 Health를 `Dead`로 전이하고 execution 전용 death presentation 사용

### Actor-scoped Debug History

Debug Event history의 내부 key는 Actor name이 아니라 실제 Actor instance weak reference다. 따라서 동일한
`BP_CEnemy_C_*` 표시명을 가진 Actor가 destroy 후 재스폰되어도 서로 다른 history가 섞이지 않는다.

## 변경 배경

기존 파이프라인은 Hit / Parry 결과와 Collapse 표현, Execution 가능 여부, Death presentation 사이의 경계가
명확하지 않았다. 특히 Collapse 상태를 단순 reaction 또는 posture로만 다루면 다음 질문에 단일 권위가 없었다.

- Balance count와 Collapse Loop TTL은 어느 계층이 소유하는가
- Execution이 시작되면 Collapse Loop를 언제 예약·소비·복구하는가
- Standard execution의 down/recovery와 Lethal execution의 death가 같은 종료 정책을 공유해야 하는가
- Source Action과 Target Reaction이 같은 execution인지 어떤 값으로 검증하는가
- Debug Overlay에서 일반 실행 판단과 pair-session lifecycle을 왜 구분해야 하는가

또한 Debug Overlay는 전역 ring buffer만으로는 여러 Enemy가 동시에 활동할 때 선택 Enemy의 원인 이력이
사라졌고, Combat result 전달 단계가 직전 같은 Actor pair의 damage breakdown을 fallback으로 재사용해
현재 Parry 결과에 이전 Hit / BlockHit가 섞여 보일 수 있었다.

이번 변경은 Balance, Execution Session, Health / Death, Debug history의 권위를 분리하고, 각 단계가
자신이 실제로 가진 정보만 기록·표시하도록 정리한다.

## 주요 변경

### 1. Enemy Balance / Collapse lifecycle

대상:

```text
Source/Portfolio/Component/CBalanceComponent.*
Source/Portfolio/Component/CCombatSignalTargetComponent.*
Source/Portfolio/Reaction/CReaction_Collapse*.cpp/.h
Source/Portfolio/Type/CBalanceTypes.*
Content/03_Animation/Damaged/Collapse_*.uasset
Content/04_Montage/Collapse/*.uasset
```

```text
Parry / accepted damage result
-> CombatSignal Target ingress
-> Balance count advance
-> threshold 최초 통과
-> CollapseIn
-> CollapseLoopActive
-> TTL 만료면 CollapseOut 또는 Execution reservation
```

- `UCBalanceComponent`를 추가해 count, threshold, lifecycle serial, Loop TTL 및 reset을 Target 측으로 모았다.
- `CollapseInActive`, `CollapseLoopActive`, `CollapseOutPending`, `ExecutionPrimaryActive`,
  `ExecutionPrimaryCommitted`, `ExecutionDownActive`, `ExecutionRecoveryPending`,
  `ExecutionRecoveryActive`를 명시적인 lifecycle state로 둔다.
- Collapse Loop 중 피격은 기존 generic Hit가 아니라 `CollapseHit` reaction으로 처리한다.
- Balance lifecycle 시작의 권위는 request acceptance가 아니라 실제 Reaction Started event다.
- CollapseOut / ExecutionRecovery reset은 Notify와 lifecycle context를 검증해 한 번만 적용한다.

### 2. Execution Collaboration pair session

대상:

```text
Source/Portfolio/Component/CExecutionCollaborationComponent.*
Source/Portfolio/Action/CAction_Execution.*
Source/Portfolio/Reaction/CReaction_Execution.*
Source/Portfolio/Reaction/CReaction_ExecutionRecovery.*
Source/Portfolio/Type/CExecutionCollaborationTypes.h
Source/Portfolio/Type/CActionOrchestrationTypes.h
Source/Portfolio/Type/CReactionOrchestrationTypes.h
```

```text
Player execution input
-> Player CombatTarget snapshot / revision 확인
-> source distance / facing geometry 확인
-> target Collapse opportunity 확인
-> target-owned outcome 결정 및 reservation
-> Target primary Reaction + Source primary Action
-> pair Active
-> Source Commit Notify
-> Standard 또는 Lethal handoff
```

- `FExecutionSessionId`와 `FCombatTargetSnapshot`으로 stale Target, stale Notify, stale terminal을 거절한다.
- session은 Reserve / Active / Committed 상태와 Source / Target 역할을 보관한다.
- Target은 `LethalCondition`과 Health ratio를 기준으로 Standard/Lethal을 결정한다. Source는 outcome을
  요청하거나 중간에 바꾸지 않는다.
- Commit 이전 interruption, target 변경, external death, EndPlay는 pair cancel과 reservation release로
  수렴한다.
- Commit 이후에는 Standard와 Lethal이 서로 다른 lifecycle owner로 handoff되므로 pair session이 long-lived
  presentation을 계속 보관하지 않는다.

### 3. Standard Recovery와 Lethal Death handoff

대상:

```text
Source/Portfolio/Character/Enemy/CEnemy.*
Source/Portfolio/Character/CAnimInstance.*
Source/Portfolio/Component/CActionComponent.*
Source/Portfolio/Component/CReactionOrchestratorComponent.*
Source/Portfolio/Notify/CAnimNotify_CommitExecution.*
Source/Portfolio/Notify/CAnimNotify_ResetBalanceLifecycle.*
Content/01_Character/01_Player/BP_CPlayer.uasset
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/03_Animation/ABP_Character.uasset
Content/03_Animation/Execution/
Content/04_Montage/Execution/
```

Standard 흐름:

```text
Source Commit
-> Balance ExecutionPrimaryCommitted
-> Source / Target primary terminal
-> ExecutionDownActive
-> ExecutionRecovery
-> Reset Balance Lifecycle Notify
-> Accumulating / normal locomotion
```

Lethal 흐름:

```text
Source Commit
-> target ExecutionLethal death entry expectation
-> Health Alive -> Dead
-> Target Lethal In terminal
-> ExecutionLethal Dead presentation
-> 기존 dissolve / destroy lifecycle
```

- Standard Commit은 `StandardExecutionDamage`를 non-lethal로 적용하고 CollapseOut을 즉시 요청하지 않는다.
- Standard primary pair가 끝난 뒤의 Down/Recovery 수명은 Balance가 소유한다.
- Lethal Commit은 Target Health에 현재 HP만큼의 damage를 한 번 적용해 즉시 Dead로 전이한다.
- `ExecutionLethal` death presentation mode에서는 기존 generic Dead In을 중복 요청하지 않는다.
- AnimBP는 `bIsExecutionDownPose`와 death presentation mode를 읽어 Standard Down pose 또는
  Execution Lethal Dead Loop를 선택한다. Dead branch가 Down pose보다 우선한다.

### 4. Participant movement collision policy

대상:

```text
Source/Portfolio/Component/CExecutionCollaborationComponent.*
```

```text
pair Active
-> Source.MoveIgnoreActorAdd(Target)
-> Target.MoveIgnoreActorAdd(Source)
-> authored root motion 재생
-> complete / cancel / external death / EndPlay
-> 각 Actor가 MoveIgnoreActorRemove(Partner)
```

- active pair의 두 Character만 서로의 CharacterMovement sweep collision을 무시한다.
- 이는 teleport, 거리 보정, world collision 무시 정책이 아니라 montage root motion이 partner capsule에
  막히지 않게 하는 수명 한정 예외다.
- `ResetActiveExecutionSession()`과 cancellation / EndPlay 경로가 모두 ignore 등록을 복구한다.

### 5. Debug Overlay observability와 Actor history

대상:

```text
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.*
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreRingAccess.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStoreFilterPolicy.cpp
Source/Portfolio/Core/Debug/FDebugOverlayViewDataBuilder.*
Source/Portfolio/Core/Debug/FDebugOverlayTextFormatter.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySettingsRegistry.*
Source/Portfolio/Core/Debug/FEnemyCombatTargetFacingDebug.*
Source/Portfolio/Core/Debug/FExecutionCollaborationDebug.*
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
```

```text
runtime event
-> World Event ring buffer
-> Owner / Source / Target Actor instance history
-> Category + Scope filter
-> Character Details / Event Log / World Summary view data
-> Canvas rendering 또는 Editor panel control
```

- Event category를 `ActionReaction`과 `ExecutionSession`으로 분리했다.
  - `ActionReaction`: Action / Reaction의 accept, reserve, start, intervene, reject 판단
  - `ExecutionSession`: reservation, pair activation, commit, damage, terminal, cancel, completion lifecycle
- Event Log scope는 `World`와 `FocusedEnemy`를 지원한다. Focused Enemy는 선택 Actor가 Owner, Source,
  Target 중 하나인 event만 표시하며 동일 Actor가 여러 역할이어도 한 번만 기록한다.
- Actor history는 weak Actor instance key로 보관하고 Enemy EndPlay에서 해당 Actor 이력을 제거한다.
- Character Details의 `[Recent Action / Reaction]`은 actor별 최신 일반 실행 판단이고,
  `[Execution Session]`은 활성 pair-session의 실시간 상태다.
- Combat result 전달 단계가 직전 pair snapshot을 fallback으로 조립하지 않게 했다. 현재 packet에 없는
  request/final/reaction breakdown은 표시하지 않으므로 Parry 결과에 이전 Hit / BlockHit가 섞이지 않는다.
- Facing transition, Recent AI summary도 Actor instance 중심으로 정리해 같은 이름 Actor의 stale 표시를 줄였다.

### 6. Debug Overlay 표시·설정·문서 정합화

대상:

```text
Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp
Source/Portfolio/Core/Debug/FDebugOverlayCanvasRenderer.cpp
Source/Portfolio/Core/Debug/FDebugOverlayDisplayConfig.*
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
Docs/05_System_Architecture/S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md
Docs/05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md
Docs/07_Portfolio_Documents/Debug_Overlay/
```

- HUD Visible, Capture Enabled, domain diagnostics, Character Details block visibility의 역할을 분리했다.
- Settings Registry가 CVar name, display name, help, type, parent gate, enum option을 제공하고 Editor plugin이
  이를 순회해 Bool/Int/Float/Enum control을 생성한다.
- Event Log line은 panel 폭을 넘으면 축약하며, Combat은 `Source` / `Target` 역할을 명시해 Defense 결과의
  관점을 혼동하지 않게 했다.
- 현재 runtime 문서에서는 `[Player]`, `[Enemy]`, `[Event Log: Category | Scope: ...]`, `[World Summary]`를
  화면 계약으로 사용한다. 과거 P0/P1 panel 명칭과 CVar는 historical record로 보존한다.
- 삭제된 legacy execution presentation notify의 Config/Source 잔존 참조를 정리했다.

## 주요 처리 흐름

### Balance / Collapse

```text
Parry 또는 accepted damage result
-> UCCombatSignalTargetComponent 검증
-> UCBalanceComponent count advance
-> threshold 최초 통과
-> CollapseIn Reaction Started
-> CollapseLoopActive
-> TTL 만료: CollapseOut
   또는 execution reservation: Loop TTL pause
```

### Execution 시작과 취소

```text
Player Target Lock + source geometry valid
-> target IsExecutionOpportunityAvailable
-> target outcome 결정
-> reservation
-> target primary Reaction / source primary Action
-> pair Active + participant movement ignore

Commit 이전 취소 / external death / EndPlay
-> pair cancel
-> movement ignore restore
-> reservation release
-> 보관한 Collapse Loop TTL resume (가능한 경우)
```

### Standard Execution

```text
Source Commit Notify
-> Standard damage non-lethal apply
-> Balance ExecutionPrimaryCommitted
-> pair primary terminal
-> ExecutionDownActive
-> ExecutionRecovery
-> Reset Balance Lifecycle Notify
-> normal Balance lifecycle
```

### Lethal Execution

```text
Source Commit Notify
-> ExecutionLethal death entry expectation
-> Health Alive -> Dead
-> hard gameplay cleanup
-> active Target Lethal In terminal
-> Execution Lethal presentation / Dead Loop
-> dissolve / destroy
```

### Debug Event 관찰

```text
Action / Reaction decision
-> ActionReaction event
-> Actor history + World history

Execution lifecycle
-> ExecutionSession event
-> Actor history + World history

Event Log query
-> Category filter
-> Scope (World / Focused Enemy)
-> noise / collision filter
-> display limit
```

## 트러블슈팅과 설계 판단

### Balance를 단순 Action / Reaction state로 두지 않은 이유

Balance count, Collapse Loop TTL, reservation, Standard Recovery reset은 하나의 montage보다 오래 유지되고 여러
Reaction terminal/Notify를 가로지른다. 이를 Action 또는 Reaction runtime에 넣으면 Count와 presentation의
권위가 섞이므로 Target-owned lifecycle로 분리했다.

### Execution을 일반 실행 판단과 같은 Event category로 두지 않은 이유

`ComboAttack`, `Hit`, `Guard` 같은 일반 Action / Reaction 판단은 “왜 accept / reserve / reject됐는가”를
보여준다. 반면 Execution Session은 “두 Actor pair가 어느 reservation / commit / terminal 단계인가”를 보여준다.
둘을 같은 `Execution` category로 저장하면 Character Details의 최신 실행 판단이 pair lifecycle event에 덮인다.

### 이전 Combat breakdown fallback을 제거한 이유

같은 Source / Target pair라는 사실은 같은 combat transaction이라는 뜻이 아니다. 직전 Hit나 BlockHit의
damage/reaction 값을 다음 Parry result에 붙이면 디버그가 실제 runtime보다 더 그럴듯하지만 틀린 정보를
표시한다. 현재 Result packet에 없는 상세값은 생략하는 것이 맞다.

### Actor name이 아닌 Actor instance로 Debug history를 보관한 이유

`BP_CEnemy_C_6` 같은 name은 재스폰 뒤 재사용될 수 있다. name key history는 죽은 Enemy의 AI/Facing/Combat
이력이 새 Enemy에 남는 문제를 만들므로 weak Actor instance key와 EndPlay cleanup으로 교체했다.

### Execution pair에만 movement ignore를 적용한 이유

Player가 execution montage의 authored root motion을 재생할 때 Target capsule에 막혀 연출이 바깥에서
시작될 수 있었다. pair 상호 ignore는 그 문제만 해결하며, world 또는 다른 Character 전체를 무시하지 않는다.

## 변경 파일 범위

```text
Source/Portfolio
- Balance / CombatSignalTarget / Action / Reaction / Execution Collaboration
- Enemy Death coordinator / AnimInstance / Movement / Weapon
- Enemy Facing / AI intent / Runtime LOD consumer
- Debug Overlay runtime, Snapshot Store, formatter, renderer, settings registry
- Execution / Collapse Notify, Type, editor animation modifier

Content
- Player / Enemy Blueprint, ABP_Character, TestRoom
- Collapse / Execution animation과 montage
- Execution SFX와 presentation asset
- Combat VFX / SFX asset maintenance

Plugins/PortfolioDebugOverlayEditor
- Registry-driven CVar access와 settings widget

Docs
- Roadmap, S35 Balance / Collapse, S36 Execution Collaboration
- Debug Overlay operation / capture / verification / current contract
```

## 테스트 방법

### Static check

```text
git diff --check origin/main...HEAD
base b1bbee0 대비 Balance / Execution / Debug Overlay 책임 및 문서 정합성 감사
legacy execution presentation notify Config / Source 잔존 참조 검색
```

### Build

```text
PortfolioEditor Win64 Development
```

### PIE

```text
1. TestRoom에서 Debug Overlay HUD와 Capture를 활성화한다.
2. Player/Enemy Character Details, Event Log, World Summary empty state를 확인한다.
3. Collapse opportunity 이후 execution pair를 시작한다.
4. active pair 중 player root motion이 Target capsule에 막히지 않는지 확인한다.
5. execution 종료 뒤 player와 target의 일반 collision이 복구되는지 확인한다.
6. Action / Reaction과 Execution Session Event Log category가 서로 다른 event를 보이는지 확인한다.
7. Focused Enemy scope에서 선택 Enemy 관련 event만 보이는지 확인한다.
```

## 검증 결과

| 항목 | 결과 | 근거 |
| --- | --- | --- |
| `git diff --check` | 통과 | whitespace error 없음 |
| `PortfolioEditor Win64 Development` build | 통과 | 사용자 build 확인 |
| Overlay empty state | 통과 | 사용자 PIE에서 Character Details / Event Log / World Summary empty state 확인 |
| Execution participant collision | 통과 | active pair 중 collision으로 montage 위치가 밀리지 않음 |
| Execution 종료 뒤 collision 복구 | 통과 | session 종료 뒤 정상 충돌 복구 확인 |
| Standard / Lethal 전체 outcome matrix | 후속 검증 | threshold, cancel, terminal, recovery 조합의 회귀 matrix 미완료 |
| 동일 이름 Enemy 재스폰 Recent AI history | 이번 마감 제외 | TestRoom에 재스폰 test spawner가 없음 |

## 설계 판단 기준

- Balance count / lifecycle, Execution Session, Health / Death, Debug history는 서로 다른 owner가 소유한다.
- Execution outcome은 Source가 아닌 Target이 한 번 결정한다.
- Standard는 Balance recovery로, Lethal은 Death lifecycle로 handoff한다.
- session은 synchronized primary pair까지만 소유하며 Down/Dead의 장기 presentation을 소유하지 않는다.
- Debug Overlay는 gameplay 상태를 변경하지 않는 read-only observer다.
- Event Log는 현재 event가 실제로 가진 값만 기록·표시한다.
- Actor history는 표시명이 아닌 Actor instance identity로 구분한다.

## Scope Guard

이번 PR에서 하지 않은 것:

- Player Balance / resource 정책
- Player Death, Game Over, Respawn, Revive
- 다중 대상 execution, network replication / prediction
- Motion Warping 또는 raw teleport 기반 execution alignment
- Ragdoll, pooling, 영구 corpse 정책
- Debug Overlay preset 기능
- Editor Debug Overlay의 CVar lookup 빈도 최적화
- Standard / Lethal 전체 gameplay tuning 및 자동화된 regression harness

## 리스크 / 리뷰 포인트

- Standard/Lethal montage의 Commit / Complete / Reset Notify 위치가 lifecycle correlation과 일치해야 한다.
- committed primary pair와 ExecutionRecovery를 일반 incoming Action/Reaction이 중단하지 않아야 한다.
- cancel, external death, EndPlay의 모든 경로에서 reservation과 mutual movement ignore가 남지 않아야 한다.
- `ExecutionLethal` presentation mode가 generic Dead In을 중복 요청하지 않아야 한다.
- 새 Event category / CVar를 추가할 때 Settings Registry, runtime CVar, Editor widget의 계약이 함께 유지되어야 한다.
- Editor widget은 Slate attribute 평가 때 CVar lookup을 반복하므로, ConsoleManager warning은 별도 성능 작업으로 추적한다.
- Actor instance history의 destroy / respawn 실증은 별도 test fixture가 생긴 뒤 수행한다.

## 후속 작업

1. Standard / Lethal outcome, pre-commit cancel, terminal, recovery의 PIE regression matrix 작성·수행
2. TestRoom 또는 test fixture에 Enemy destroy / respawn scenario를 추가해 Actor-scoped Recent AI history 검증
3. Editor Debug Overlay panel lifetime 한정 CVar resolve/cache 정책 검토
4. 실제 전투 거리와 authored montage에 맞춰 `MaxStartDistance`, facing angle, lethal threshold를 tuning
5. 필요 시 motion warping 기반 execution alignment를 별도 lifecycle 단계로 설계

## 관련 문서

- [Roadmap](../00_plan/P02_UE5%20Portfolio_Development%20Roadmap%20(KR).md)
- [S35 Enemy Balance / Collapse Lifecycle](../05_System_Architecture/S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md)
- [S36 Execution Collaboration Architecture](../05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md)
- [Debug Overlay Operation Guide](../07_Portfolio_Documents/Debug_Overlay/02_Operation/Debug_Overlay_Operation_Guide_KR.md)
- [Debug Overlay / Execution Branch Verification](../07_Portfolio_Documents/Debug_Overlay/05_Verification/Debug_Overlay_Execution_Collaboration_Branch_Verification_KR.md)

## 대표 커밋

```text
f29f91f9 feat(balance): add collapse lifecycle domain
46a24934 feat(enemy): integrate balance collapse lifecycle
74415372 feat(execution): implement collaboration lifecycle
c7580883 feat(execution): author collaboration pairs and debug visibility
a29b20f7 feat(balance): unify incapacitated presentation flow
4757c07e feat(execution): refine presentation and combat flow
1def1018 feat(execution): gate external combat input
02376084 refactor(debug-overlay): unify capture, settings, and event categories
f21b89a3 refactor(debug-overlay): structure combat event details
c8882040 refactor(debug): finalize actor scoped diagnostics
edf729ff docs: align execution and overlay operations
```

## 정리

이번 PR은 Enemy의 Balance, Collapse, Execution, Death presentation을 하나의 거대한 state로 합치지 않았다.
Balance는 opportunity와 recovery 수명을, Execution Collaboration은 synchronized pair transaction을,
Health / Death는 lethal finalization을, Debug Overlay는 Actor-scoped 관찰 이력을 각각 소유한다.

이로써 Collapse를 단순 knockdown 연출로 소비하지 않고 Standard/Lethal execution으로 확장할 수 있으며,
commit 이후의 결과와 취소 가능한 pre-commit 구간을 명확히 구분한다. 또한 Debug Overlay가 실제 runtime
계약과 다른 stale 값을 조립하지 않고, 선택한 Actor의 lifecycle 원인을 범위별로 추적할 수 있게 했다.
