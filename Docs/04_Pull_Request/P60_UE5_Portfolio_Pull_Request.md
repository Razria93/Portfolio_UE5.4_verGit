# UE5 Portfolio Pull Request

## 제목

**P60: Shared Combat Target and Evidence-Based Combat Participation**

## 날짜

**2026.08.23**

## 상태

- [x] Player/Enemy 공통 `UCCombatTargetComponent`와 Target Revision 계약 도입
- [x] Player Target 선택 정책과 Enemy Combat Participation 정책의 Combat Target SoT 분리
- [x] Perception/HitReactive의 Source별 Active Evidence ingress 통합
- [x] GeneralBase 우선, live HitReactive Evidence 기반 Extra Engage admission
- [x] Action assignment lock, soft/hard release, ReturnHome suppression 분리
- [x] 마지막 Evidence 종료 뒤 Last Known Target Context 기반 Investigate handoff
- [x] Enemy Facing, Blackboard projection, Combat Action authority의 Target Snapshot/Revision 재검증
- [x] Combat Participation Debug Overlay, Editor panel, TestRoom 검증 환경 보강
- [x] Rotation mode 기반 directional locomotion presentation과 gait input·movement debug 정리
- [x] AIPerf Behavior Tree/Blackboard/AnimBP의 현재 Combat Participation 계약 정렬
- [x] `git diff --check`, C++/UHT build, Blueprint compile, headless map load, 수동 PIE 검증 완료

## 브랜치

- Base: `main`
- Branch: `feature/combat-target-participation`
- Base HEAD: `f3b6bace8f5bec7138af5e1051d49bfcef4ce88d` — Merge pull request #115 from `feature/dead-actor-destroy-flow`
- Implementation HEAD: `9818826f9d9b9442778786f3b2639cb8ffedf48a` — `fix(ai-perf): clear stale behavior tree state`
- Merge Policy: 일반 Merge Commit

## 대표 스크린샷

이번 PR은 TestRoom과 AIPerf Baseline PIE에서 Combat Participation role/slot, Evidence 수명, Investigate handoff, Blackboard 상태 전이를 확인했다. 별도 스크린샷 파일은 추가하지 않으며, TestRoom/AIPerf 자산과 Debug Overlay를 실행 근거로 사용한다.

## 요약

이번 PR은 Combat Target을 Character-owned Source of Truth로 고정하고, Enemy의 전투 참여 판단을 Perception/HitReactive의 단발성 요청이나 BT Blackboard write가 아닌 **Participant × Source × Target Active Evidence**와 allocator 결과로 전환한다.

Enemy의 Combat Target은 Evidence 자체가 아니라 assignment가 Character adapter에 적용한 결과다. 따라서 Perception, HitReactive, Blackboard, Facing, Action, Investigate가 서로 다른 상태를 장기 소유하지 않는다. 마지막 Evidence가 끝날 때만 passive Last Known Target Context를 Investigate로 넘기고, Investigate는 CombatTarget·slot을 소유하지 않는 비전투 행동으로 유지한다.

이 브랜치에는 위 구조 전환의 consumer 정리와 함께, 방향 locomotion presentation/Debug Overlay 보강, AIPerf asset 계약 동기화도 포함한다.

## 핵심 개념

### Combat Target SoT

`UCCombatTargetComponent`만 현재 Combat Target, Revision, Target 종료 구독을 보관한다. Player는 입력 기반 선택 정책을, Enemy는 Combat Participation assignment 결과를 이 Component에 반영한다.

### Active Evidence와 Assignment

Active Evidence는 `Participant × Source × Target` 단위의 live 근거다. allocator는 같은 `Participant × Target`의 Evidence를 합산해 Candidate를 만들고, slot cap과 priority를 적용해 `None / Observe / Alert / Engage` assignment를 결정한다. Assignment만 Enemy 전투 참여의 권위다.

### Last Known Target Context

마지막 관측 위치·속도·시각을 보관하는 passive context다. Candidate나 Extra admission에는 사용하지 않으며, 마지막 Active Evidence가 사라진 뒤 Investigate handoff에만 사용한다.

### Assignment Lock

진행 중인 Engage Action의 정확한 assignment를 Action 종료까지 보호한다. lock은 새로운 Candidate의 admission 자격이 아니며, hard release는 lock을 무시하고 즉시 정리한다.

## 변경 배경

기존 Enemy 경로에는 Perception working memory, HitReactive 처리, BT Blackboard, Engage slot, Combat Target이 서로 가까운 의미를 가진 채 흩어져 있었다. 그 결과 다음 경계가 불명확했다.

- Perception 상실과 Combat Target 해제가 같은 의미인지
- HitReactive가 언제까지 Extra Engage 자격을 갖는지
- Action 중 assignment가 바뀌어도 어떤 Target/Revision을 Action이 신뢰하는지
- 마지막 Target 정보로 Investigate할 때 이전 Evidence가 Candidate로 되살아나는지

이번 전환은 Target SoT, Evidence, assignment, action snapshot, investigate memory를 별도 책임으로 분리해 이 문제를 해소한다.

## 주요 변경

### 1. Shared Combat Target Kernel과 consumer Revision 계약

대상:

```text
Source/Portfolio/Component/CCombatTargetComponent.*
Source/Portfolio/Component/CPlayerTargetSelectionComponent.*
Source/Portfolio/Component/CTargetLockAssistComponent.*
Source/Portfolio/Component/CTargetHUDPresenterComponent.*
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Type/CCombatTargetTypes.h
```

- Player/Enemy가 `UCCombatTargetComponent`의 Target Snapshot과 Revision을 공통으로 소비하게 했다.
- Player Target selection은 후보 평가·요청만 담당하고, Enemy adapter는 assignment projection만 담당한다.
- Blackboard는 SoT가 아닌 projection이며, BT는 값을 직접 써서 Target을 확정하지 않는다.
- Facing과 Combat Action은 현재 Target Snapshot/Revision을 다시 확인해 늦은 callback이나 이전 Target의 Action 진행을 막는다.

### 2. Evidence 중심 Combat Participation allocator

대상:

```text
Source/Portfolio/System/Combat/CWorldSubsystem_CombatParticipation.*
Source/Portfolio/Component/CEnemyCombatParticipationComponent.*
Source/Portfolio/Type/CCombatParticipationTypes.h
Source/Portfolio/Controller/CAIController.*
```

```text
Perception / HitReactive ingress
-> Active Evidence Registry (Participant × Source × Target)
-> Participation Pair Candidate (Participant × Target)
-> GeneralBase / HitReactiveExtra allocator
-> Assignment
-> Combat Target / Facing / Blackboard / Intent consumer
```

- Perception과 HitReactive는 등록 경로만 다르고 동일한 allocator를 사용한다.
- GeneralBase Engage slot을 우선 사용한다.
- Base가 찼을 때만 live HitReactive Evidence가 있는 Candidate가 HitReactiveExtra admission을 받을 수 있다.
- Session, session phase, minimum commitment를 제거했다. Evidence가 없는 Candidate는 존재하지 않는다.

### 3. Evidence lifetime, release, Investigate handoff

대상:

```text
Source/Portfolio/Component/CEnemyHitReactiveComponent.*
Source/Portfolio/System/Combat/CWorldSubsystem_CombatParticipation.*
Source/Portfolio/Controller/CAIController.*
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.*
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.*
```

- Perception Evidence는 LOS와 target memory timeout을 기준으로 유지한다.
- HitReactive Evidence는 accepted hit 뒤 reaction terminal에서 `HitReactivePostReactionTTL`을 시작한다. 최신 ResultSerial만 terminal callback을 적용한다.
- HitReactive anchor 위치·반경을 두어 긴 TTL이 무제한 추격으로 바뀌지 않게 했다.
- soft release는 Evidence/context만 제거하고 matching Action lock이 있다면 Action 종료를 기다린다.
- hard release는 죽음, EndPlay, UnPossess/unregister, hostile/identity 무효에서 Evidence, assignment, lock을 즉시 정리한다.

### 4. Investigate를 전투 참여에서 분리

대상:

```text
Source/Portfolio/System/Combat/CWorldSubsystem_CombatParticipation.*
Source/Portfolio/Component/CEnemyCombatParticipationComponent.*
Source/Portfolio/Controller/CAIController.*
Source/Portfolio/AI/BehaviorTree/Task/CBTTask_SelectAlertPoint.*
```

```text
마지막 Active Evidence 제거
-> assignment 재계산 및 기존 Target 해제
-> EvidenceExhausted event dispatch 재검증
-> Last Known Target Context 전달
-> Blackboard Investigate 요청
```

- 다른 Active Evidence가 남아 있으면 만료한 Evidence만 제거하고 Investigate하지 않는다.
- 새 Evidence, ReturnHome suppress, 다른 Target assignment가 확인되면 pending handoff를 폐기한다.
- Investigate는 assignment, CombatTarget, Facing, Engage slot을 유지하지 않는다.

### 5. AI consumer와 AIPerf asset 정렬

대상:

```text
Source/Portfolio/AI/Blackboard/CAIKey.*
Source/Portfolio/AI/Blackboard/CAIKeyRegistry.h
Content/00_Profiling/00_AI_Performance/02_Controller/02_Enemy/AI/
Content/00_Profiling/00_AI_Performance/03_Animation/ABP_AIPerf_Character.uasset
```

- Behavior Tree는 Combat Target/Participation Blackboard projection을 읽어 분기한다.
- AIPerf `BB_AIPerf_Default`에 필수 Combat Target/Participation key를 동기화했다.
- AIPerf 상태 BT의 Focus 의존성을 제거하고, Chase/Observe/Engage Positioning에 남은 stale runtime 값을 정리했다.
- `ABP_AIPerf_Character`를 현재 dead-state contract에 맞춰 정리했다.

### 6. 관찰성과 검증 환경

대상:

```text
Source/Portfolio/Core/Debug/FCombatParticipationDebug.*
Source/Portfolio/Core/Debug/FDebugOverlayDisplayConfig.*
Plugins/PortfolioDebugOverlayEditor/Source/PortfolioDebugOverlayEditor/
Content/00_UnitTest/TestRoom.umap
```

- Combat Participation Debug Overlay에 source, Evidence lifetime, HitReactive anchor, Base/Extra admission, assignment/lock 상태를 표시한다.
- Editor panel에서 Player/Enemy section과 detail block, Combat Participation anchor 표시를 독립적으로 제어한다.
- TestRoom과 AIPerf Baseline을 current Blackboard/BT 계약에 맞췄다.

### 7. Directional locomotion presentation

대상:

```text
Source/Portfolio/Type/CMovementTypes.h
Source/Portfolio/Character/CAnimInstance.*
Source/Portfolio/Character/Player/CPlayer.*
Source/Portfolio/Controller/CPlayerController.*
Source/Portfolio/Core/Debug/FMovementDebug.*
Content/03_Animation/
Content/05_BlendSpace/
```

- `ControllerDesired`와 `FixedFacing` rotation mode에서는 `Directional`, `OrientToMovement`에서는 `Forward` locomotion presentation을 선택한다.
- Walk/Sprint hold 입력을 Controller가 일관되게 dispatch하고, Player가 gait 적용을 담당하게 했다.
- 1D/2D BlendSpace와 방향별 locomotion animation asset을 연결하고, movement debug에서 gait·방향·입력 결과를 관찰할 수 있게 했다.
- 이 변경은 Combat Participation의 권위를 바꾸지 않는다. Combat Target/Facing 결과가 rotation mode를 바꿀 때 presentation이 올바르게 따라가도록 하는 consumer 정렬이다.

## 주요 처리 흐름

### Enemy 전투 참여

```text
Perception 또는 Accepted Hit
-> Evidence 등록·갱신
-> Candidate / allocator rebuild
-> Assignment 변경
-> Enemy Combat Target adapter 적용
-> Facing / Blackboard projection / Intent / Action authority 검증
```

### 마지막 Evidence 종료

```text
Evidence 만료·withdraw
-> Active Registry에서 즉시 제거
-> 다른 Evidence 존재: 종료
-> 마지막 Evidence: Last Known Target Context 기반 event 예약
-> assignment 제거 뒤 dispatch 조건 재검증
-> Investigate 요청 또는 event 폐기
```

### ReturnHome과 hard release

```text
ReturnHome edge
-> participation suppress
-> ingress 차단 + soft release
-> ReturnHome 해제 뒤 현재 LOS만 다시 ingress

Death / EndPlay / Unregister / hostile·identity invalid
-> hard release
-> Evidence + context + assignment + lock 즉시 제거
```

## 트러블슈팅과 설계 판단

### HitReactive TTL이 Reaction보다 먼저 끝나는 문제

초기 TTL은 accepted hit 직후부터 짧게 흘러 Reaction이 아직 실행 중인데 Extra Engage 자격이 사라질 수 있었다. TTL을 `HitReactivePostReactionTTL`로 재정의하고 reaction terminal과 ResultSerial을 연결했다. reaction이 시작되지 않거나 reject/ignore된 경우만 accepted hit 시점부터 TTL을 시작한다.

### Session을 유지한 채 Evidence만 지우는 문제

Session이 Candidate의 근거로 남으면 Evidence가 만료되어도 Alert/Observe 또는 Base Engage 후보가 유지된다. Session을 권위 구조에서 완전히 제거하고, Active Evidence만 Candidate/assignment의 근거로 삼았다. 오래된 위치 정보는 별도의 passive context로만 보존한다.

### 마지막 위치와 현재 Target 위치를 혼동하는 문제

HitReactive TTL 종료 시점의 Target 현재 위치를 Investigate 위치로 읽으면, AI가 이미 알 수 없는 정확한 현재 위치를 추적하는 문제가 생긴다. 따라서 Evidence가 관측된 시점의 위치·시각만 기록하고, 마지막 Evidence 종료 시 가장 최신 observation을 Investigate에 사용한다.

### Action lock과 Candidate 자격을 혼동하는 문제

lock이 있다고 새 Extra admission 자격까지 유지하면 만료한 HitReactive Evidence가 Extra 후보를 부활시킨다. lock은 이미 확정된 정확한 Engage assignment를 Action 종료까지 보호할 뿐, 새 allocator admission의 근거가 아니라는 규칙으로 분리했다.

### AIPerf Blackboard/BT stale 상태

AIPerf Baseline을 headless로 로드했을 때 `BB_AIPerf_Default`가 현재 필수 Combat Target/Participation key를 갖지 않아 ensure가 발생했다. Blackboard key를 동기화한 뒤 Focus 의존성과 상태 BT의 stale 값을 정리했다. 이후 Blueprint compile, headless load, PIE에서 오류·ensure·stale 상태가 재현되지 않았다.

## 변경 파일 범위

```text
Source/Portfolio
- CombatTargetComponent / EnemyCombatParticipationComponent
- WorldSubsystem_CombatParticipation
- AI Controller, BT Service/Task/Decorator, Blackboard key registry
- Enemy Facing / HitReactive / Action / Combat Signal consumer
- Player TargetSelection / HUD / LockAssist consumer
- Debug Overlay runtime

Content
- Player/Enemy Blueprint, Blackboard, Behavior Tree, AnimBP
- TestRoom, AIPerf Baseline asset
- Directional locomotion animation / BlendSpace assets

Plugins/PortfolioDebugOverlayEditor
- CVar access, section/detail toggle panel

Docs
- S32 historical baseline 안내
- S33 current Combat Target architecture
- S34 current Combat Participation policy
- Debug Overlay operation/README, AIPerf runtime LOD README, P60 PR record
```

## 테스트 방법

### Static check

- `git diff --check`
- 기준 커밋 `f3b6bac` 대비 구조·책임·문서 감사
- Header/CPP API 및 UHT signature 확인

### Build와 asset load

- `PortfolioEditor Win64 Development` build
- `CompileAllBlueprints -ProjectOnly -NoSave`
- TestRoom headless load: `/Game/00_UnitTest/TestRoom -game -NullRHI`
- AIPerf Baseline headless load: `/Game/00_Profiling/00_AI_Performance/00_Map/00_Baseline/MAP_AIPerf_Baseline_40Enemy -game -NullRHI`

### PIE

- Base/Extra cap과 HitReactive Extra 유지
- LOS 상실, Perception memory timeout, HitReactive post-reaction TTL, anchor 반경 이탈
- 마지막 Evidence 종료, Investigate handoff, 재인지, stale handoff 차단
- ReturnHome suppress와 hard release
- A/B Target 경쟁에서 Participant당 하나의 assignment 유지
- AIPerf Baseline의 Focus 제거 후 BT 상태 전이와 stale Blackboard 값 미잔류

## 검증 결과

| 항목 | 결과 | 근거 |
| --- | --- | --- |
| C++/UHT build | 통과 | `PortfolioEditor Win64 Development` build 성공 |
| Blueprint compile commandlet | 통과 | 0 errors, 0 warnings, failed-load 0 |
| TestRoom headless load | 통과 | LoadMap, world bring-up, 정상 teardown 확인 |
| AIPerf Baseline headless load | 통과 | Blackboard required-key ensure 없이 LoadMap 완료 |
| Combat Participation PIE matrix | 통과 | Evidence/Extra/Investigate/ReturnHome/hard release/A-B Target 시나리오 확인 |
| AIPerf PIE | 통과 | Focus 정리 뒤 상태 전이와 stale 값 해소 확인 |
| `git diff --check` | 통과 | whitespace error 없음 |

## 설계 판단 기준

- Combat Target은 Character-owned SoT 하나만 사용한다.
- Evidence는 live eligibility, Last Known Target Context는 Investigate 기억으로 역할을 분리한다.
- Assignment는 Enemy 전투 참여의 유일한 권위이며, Blackboard는 projection이다.
- Action lock은 lifecycle hold가 아니라 이미 확정된 assignment 보호다.
- HitReactive Extra는 live Evidence가 있을 때만 새 admission을 허용한다.
- Investigate는 전투 참여 상태가 아니라 마지막 observation을 소비하는 비전투 행동이다.

## Scope Guard

- 새로운 시간 기반 Session timeout, Session phase, grace/cooldown 정책은 추가하지 않았다.
- Hit acceptance/serial dedupe 규칙과 Base/Extra cap 수치는 변경하지 않았다.
- Facing/HUD/Action consumer의 공개 구조를 새로 만들지 않고 Combat Target Snapshot 계약에 정렬했다.
- AIPerf 변경은 current Blackboard/BT/AnimBP 계약 호환과 stale state 제거 범위로 제한했다.

## 리스크 / 리뷰 포인트

- `HitReactivePostReactionTTL`과 anchor radius는 gameplay tuning 값이므로 장시간 플레이 결과에 맞춰 별도 조정한다.
- Action lock은 exact existing assignment만 보호해야 하며 새 Candidate admission 근거로 사용하면 안 된다.
- 새 Evidence source를 추가할 때는 Active Evidence와 Last Known Target Context를 동시에 갱신하고, final Evidence exhaustion 규칙을 통과해야 한다.
- Blackboard key 또는 BT asset을 새 profiling 세트에 복제할 때는 `CAIKeyRegistry` required-key 계약을 함께 적용해야 한다.

## 후속 작업

- 실제 전투 거리·이동 속도에 맞춘 HitReactive TTL/anchor tuning 측정
- AIPerf 외 profiling map에도 Blackboard/BT asset 계약을 주기적으로 audit
- Evidence source별 automated runtime coverage 확대

## 관련 문서

- [S32 공통 Combat Target 상태 및 의사결정 경계 설계](../05_System_Architecture/S32_UE5_Portfolio_System_Architecture.md)
- [S33 공통 Combat Target Kernel 및 의사결정 통합 설계](../05_System_Architecture/S33_UE5_Portfolio_System_Architecture.md)
- [S34 Combat Participation 정책](../05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md)
- [Debug Overlay Operation Guide](../07_Portfolio_Documents/Debug_Overlay/02_Operation/Debug_Overlay_Operation_Guide_KR.md)
- [AI Runtime LOD README](../07_Profiling/AI_Performance/Runtime_LOD/README.md)

## 대표 커밋

```text
b799d395 refactor(targeting): add shared combat target kernel
d6bce179 refactor(targeting): migrate player consumers to combat target
54eab239 feat(locomotion): add directional gait presentation and debug overlay
03c40f7b feat(combat): centralize enemy participation lifecycle
447c0bc4 refactor(combat): make participation evidence-driven
5146c685 fix(combat): defer investigate handoff until release
062dd191 feat(debug): expand combat participation evidence diagnostics
5378d4b7 chore(ai-perf): align focus and combat blackboard assets
9818826f fix(ai-perf): clear stale behavior tree state
```

## 정리

Combat Target의 저장 책임, Enemy participation의 자격 판단, Action 중 assignment 보호, Investigate용 마지막 위치 기억을 분리했다. 이로써 Perception/HitReactive/BT/Blackboard가 서로의 상태를 직접 확정하지 않고도, 동일한 allocator와 Snapshot 계약을 통해 일관된 전투 참여 흐름을 유지한다.
