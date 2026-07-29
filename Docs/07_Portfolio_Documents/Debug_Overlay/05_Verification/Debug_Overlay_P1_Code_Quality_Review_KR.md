# Debug Overlay P1 Code Quality Review

## 1. 목적

이 문서는 `feature/debug-overlay-evidence-plan` 브랜치에서 추가/수정된 debug overlay 관련 코드를 W05 Code Quality Plan 기준으로 검토한 결과를 기록한다.

이번 단계는 코드 수정이 아니라 findings 정리다. 수정 대상은 다음 작업에서 별도 구현 단위로 처리한다.

검토 기준:
- `W05_Naming_Rules.md`
- `W05_API_Const_Consistency_Work_Plan.md`
- `W05_Tuning_Constants_Rules.md`
- `W05_Type_Header_Organization_Rules.md`
- `W05_Comment_Section_Cleanup_Work_Plan.md`

추가 검토 기준:
- debug-only 경계가 gameplay flow를 오염시키지 않는지 확인한다.
- Shipping 빌드에서 overlay 저장/표시 경로가 no-op 또는 비활성인지 확인한다.
- 실제 코드에서 읽지 못하는 값을 evidence처럼 표시하지 않는지 확인한다.

## 2. 변경 파일 목록

| 파일 | 역할 |
| --- | --- |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.h/.cpp` | Canvas 기반 P0.5 overlay 표시, Player/Enemy panel, source chain 표시 |
| `Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.h/.cpp` | debug overlay 전용 명시 target 저장 component |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotTypes.h` | overlay snapshot / summary / event entry 타입 |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h/.cpp` | World별 snapshot store, event ring, recent combat pair 저장 |
| `Source/Portfolio/Core/Debug/CDebugOverlayGameMode.h/.cpp` | TestRoom 수동 연결용 debug overlay GameMode |
| `Source/Portfolio/Core/Debug/FExecutionOrchestratorDebug.cpp` | execution decision event를 SnapshotStore에 기록 |
| `Source/Portfolio/Core/Debug/FCombatSignalDebug.cpp` | collision window / target packet / combat result event를 SnapshotStore에 기록 |
| `Source/Portfolio/Core/Debug/FCombatResultDebug.cpp` | combat result receive event를 SnapshotStore에 기록 |
| `Source/Portfolio/Core/Debug/FAICombatBTDebug.cpp` | AI combat task event를 SnapshotStore에 기록 |
| `Source/Portfolio/Controller/CPlayerController.h/.cpp` | debug target Exec command와 TargetComponent 소유 |
| `Source/Portfolio/Character/Player/CPlayer.h` | overlay용 parry stagger count getter 제공 |
| `Source/Portfolio/Character/Enemy/CEnemy.h` | overlay용 parry stagger count getter 제공 |

## 3. W05 Naming 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `CDebugOverlayTargetComponent.h:16-21` | public API가 `Has/Get/Set/Clear` 계열로 분리되어 있고 PascalCase를 따른다. |
| `FDebugOverlaySnapshotStore.h:27-58` | `Is/Record/Add/Get/Try/Reset` 계열로 gate, record, query, lifecycle 의미가 드러난다. |
| `CPlayerController.h:18-24` | Exec command 이름이 console command로 읽기 쉽고 debug overlay prefix가 명확하다. |
| `CPlayer.h:133-134`, `CEnemy.h:205-206` | Stagger getter는 read-only getter로 의미가 명확하다. |

### LowRiskFix

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:73-180` | local variable lowerCamelCase / Component suffix | `stateComp`, `actionComp`, `reactionComp`, `defenseComp`, `movementComp`, `healthComp`는 기존 로컬 `Comp` 허용 범위라 큰 문제는 아니다. 다만 같은 파일 안에서 `FindComponent` helper와 함께 쓰이므로 다음 cleanup에서 `stateComponent` 등으로 통일할지 검토 가능하다. |
| `CDebugOverlayHUD.cpp:212-237` | helper naming | `AddLine`, `AddActorStatusLines`, `AddSnapshotLines`는 간결하지만 HUD rendering helper임이 약하다. P1에서 HUD가 커질 경우 `AppendOverlayLine`, `AppendActorStatusLines`처럼 append 의미를 맞추는 것이 더 명확하다. |
| `CPlayerController.h:73-74`, `CPlayerController.cpp:212-244` | bool mutation API naming | `SelectDebugOverlayTargetFromView()`, `SelectDebugOverlayNearestEnemy()`는 target component를 변경하고 성공 여부를 반환한다. W05 기준상 bool 반환 mutation helper는 `TrySelect...` 계열이 더 명확하다. |
| `FDebugOverlaySnapshotStore.h:52`, `FDebugOverlaySnapshotStore.cpp:448` | bool + Out parameter query naming | `GetSnapshotCopy()`는 bool 성공 여부와 `OutSnapshot`을 함께 사용한다. 같은 store에 `TryGetRecentCombatPair()`가 있으므로 `TryGetSnapshotCopy()`로 맞추는 것이 더 일관적이다. |

### DecisionNeeded

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayTargetComponent.h:19`, `CDebugOverlayTargetComponent.cpp:20-23` | API 책임명 | `GetDebugOverlayTargetSource()`는 항상 `"TargetComponent"`를 반환한다. HUD에서는 현재 이 API를 쓰지 않고 문자열을 직접 출력한다. 유지할지, 제거할지, HUD에서 사용하게 할지 결정이 필요하다. |

## 4. API const 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `CDebugOverlayHUD.h:18-19` | `ResolveDisplayEnemy`, `RefreshCachedEnemyIfNeeded`는 cache와 source line을 갱신하므로 non-const가 맞다. |
| `CPlayerController.h:77-78` | `FindDebugOverlayEnemyFromView`, `FindNearestDebugOverlayEnemy`는 조회 전용이므로 `const`가 적절하다. |
| `CDebugOverlayTargetComponent.h:16-19` | `HasDebugOverlayTarget()`, `GetDebugOverlayTargetActor()`, `GetDebugOverlayTargetSummary()`, `GetDebugOverlayTargetSource()`는 상태 변경이 없는 query API로 `const`가 적절하다. |
| `FDebugOverlaySnapshotStore.h:27-58` | static store API라 member const 적용 대상이 아니다. `Record/Add/Reset` 계열은 mutation API로 명확하다. |
| `CPlayer.h:133-134`, `CEnemy.h:205-206` | `GetParryResultCount()`, `GetParryStaggerThreshold()`는 read-only inline getter로 적합하다. |

### LowRiskFix

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:331` | ReadOnly / mutation 분리 | `ResolveDisplayEnemy`는 source line append와 fallback cache refresh를 함께 수행한다. 동작은 맞지만 query와 text build 책임이 섞여 있다. 다음 cleanup에서 target resolve result struct 또는 source-line builder 분리를 검토할 수 있다. |

### DecisionNeeded

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `FDebugOverlaySnapshotStore.cpp:61-71` | ReadOnly / const policy | `ResolveWorld(const UObject*)`에서 `const_cast<UObject*>`를 사용한다. 실제 mutation은 없지만 W05에서 `const_cast`는 위험 신호로 분류하므로, helper signature를 바꿀지 현 상태를 유지할지 판단이 필요하다. |
| `CDebugOverlayHUD.h:19`, `CDebugOverlayHUD.cpp:331` | API 책임명 | `ResolveDisplayEnemy()`는 이름상 read-only resolve처럼 보이지만 내부에서 cache와 source line을 갱신한다. non-const 자체는 맞지만 이름 또는 책임 분리 여부를 판단해야 한다. |

## 5. Header / include 정리 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `CDebugOverlayHUD.h:1-4` | `CoreMinimal.h`, required engine header, generated header 순서가 적절하다. |
| `CDebugOverlayTargetComponent.h:1-5` | `CoreMinimal.h`, `ActorComponent.h`, generated header 순서가 적절하다. |
| `FDebugOverlaySnapshotStore.h:1-10` | snapshot type include와 forward declaration이 구분되어 있다. |

### LowRiskFix

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CPlayerController.cpp:8-12` | include group 순서 | `EngineUtils.h`가 engine header인데 `Core/Debug`와 `Type` project headers 사이에 위치한다. W05 기준상 project internal header를 먼저 모으고 engine header를 뒤로 보내는 정리가 가능하다. |
| `CDebugOverlayHUD.cpp:13-17` | include group 순서 | `Engine/Canvas.h`, `EngineUtils.h`, `GameFramework/*`가 `Type/CActionKeyTypes.h`보다 앞에 있다. W05 기준으로는 project Type header를 engine header보다 먼저 두는 정리가 가능하다. |
| `FDebugOverlaySnapshotStore.cpp:3-10` | include group 순서 | `Type/CCombatResultTypes.h`, `Type/CCombatSignalTargetTypes.h`가 Unreal/Engine include 뒤에 있다. W05 기준으로 project Type include를 engine header보다 먼저 배치하는 정리가 가능하다. |

### Later

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `FDebugOverlaySnapshotTypes.h` | Type header organization | 현재는 debug snapshot 전용 타입이 작고 단일 책임이므로 유지 가능하다. P1에서 subject 분리, EventLog filter, Player/Enemy event 분리가 들어오면 `EventEntry`, `RecentCombatPair`, `Filter` 타입 분리를 재검토한다. |

## 6. Tuning constants 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `FDebugOverlaySnapshotStore.cpp:14-16` | event ring capacity와 display limit은 internal policy 값이며 `static constexpr`로 분리되어 있다. |
| `CPlayerController.cpp:17-18` | trace distance `5000.f`, nearest radius `1500.f`는 debug target selection의 내부 default policy 값이며 `static constexpr`로 분리되어 있다. |
| `CDebugOverlayHUD.cpp:21-31` | HUD 위치, 크기, 색상, fallback cooldown, stale timeout이 anonymous namespace 상수로 분리되어 있다. |

### LowRiskFix

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:29-31` | internal policy constant | `FLinearColor` 값은 `static const`로 되어 있다. 프로젝트 컴파일 정책상 문제는 없지만, 다음 cleanup에서 `const FLinearColor` 또는 UE 스타일에 맞춘 명명만 확인하면 충분하다. |

### Later

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:21-31` | design data vs internal policy | overlay 위치/크기/색상은 P0.5에서는 internal policy로 충분하다. P1 이후 사용자 조정이 필요해지면 CVar 또는 preset config 후보로 분리한다. |

## 7. Comment / section 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `FDebugOverlaySnapshotStore.h:26-57`, `FDebugOverlaySnapshotStore.cpp:207-419` | `Gate`, `Execution Record`, `Combat Record`, `AI Record`, `Event Log`, `Snapshot Query`, `Lifecycle` 섹션은 책임 단위가 분명하다. |
| `CPlayerController.h:66-78`, `CPlayerController.cpp:77-98`, `212-305` | `Debug Overlay Exec`, `Debug Overlay Target` 섹션은 debug-only 확장 영역을 명확히 구분한다. |

### LowRiskFix

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:19-302` | section comment | anonymous namespace helper가 길어졌지만 내부 섹션이 없다. P1에서 HUD helper가 더 커지기 전에 `Text Formatting`, `Actor Status Formatting`, `Snapshot Lines`, `Panel Styling` 정도의 최소 섹션으로 나누는 cleanup이 가능하다. |

## 8. Shipping guard / debug-only 경계 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `CDebugOverlayGameMode.cpp:7-9` | Shipping에서는 HUDClass를 debug overlay HUD로 지정하지 않는다. |
| `CDebugOverlayHUD.cpp:431-485` | `DrawHUD` 내부가 `#if !UE_BUILD_SHIPPING`으로 보호되고 `IsEnabled()` gate를 통과해야 그린다. |
| `FDebugOverlaySnapshotStore.cpp:18-42`, `209-481` | CVar 선언과 store mutation이 non-shipping guard 안에 있다. Shipping query는 false/empty/no-op로 정리되어 있다. |
| `CPlayerController.cpp:29-32`, `79-98`, `210-305` | TargetComponent 생성과 Exec 구현이 non-shipping guard 안에 있다. input asset/config 변경 없이 console command로만 접근한다. |
| `FExecutionOrchestratorDebug.cpp:209-222`, `272-285` | overlay record 호출은 store의 `IsCollecting()` / record API를 통해 shipping no-op 경계에 걸려 있고, 기존 audit CVar return과 분리되어 있다. |
| `FCombatSignalDebug.cpp:88-101`, `302-310`, `332-340`, `410-419` | overlay collect 경로가 기존 audit log gate와 분리되어 있고, shipping에서는 store API가 no-op으로 동작한다. |
| `FCombatResultDebug.cpp:73-84` | combat result overlay 기록은 audit 출력보다 먼저 분리되어 있으며 shipping에서는 store API no-op 경계를 따른다. |
| `FAICombatBTDebug.cpp:157-170`, `185-198` | AI combat task overlay 기록은 기존 audit CVar와 분리되어 있고, shipping에서는 store API no-op 경계를 따른다. |

### LowRiskFix

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:19-302` | Shipping guard | HUD 표시/포맷팅 helper가 shipping에서도 컴파일된다. 실행 경로는 guard되어 있으나 debug overlay display 전용 helper이므로 `#if !UE_BUILD_SHIPPING` 범위 안으로 축소할 수 있다. |
| `FDebugOverlaySnapshotStore.cpp:61-129` | Shipping guard | store 내부 world resolve / event formatting helper 일부가 shipping에서도 컴파일된다. 실행 경로는 no-op이지만 debug overlay store 전용 helper라면 non-shipping guard 안으로 범위를 좁힐 수 있다. |

### DecisionNeeded

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CDebugOverlayHUD.cpp:429-485` | Shipping guard | shipping에서는 `DrawHUD()`가 `Super::DrawHUD()`도 호출하지 않고 완전 no-op이 된다. debug 전용 HUD라면 허용 가능하지만, shipping에서 이 HUD class가 지정될 가능성을 방어하려면 `Super::DrawHUD()` 위치 조정 여부를 결정해야 한다. |
| `CDebugOverlayTargetComponent.h:7-24`, `CDebugOverlayTargetComponent.cpp:5-40` | Shipping guard | component class/API 자체는 shipping에서도 컴파일된다. 현재 생성 경로는 `ACPlayerController`에서 non-shipping guard로 차단되어 있으나, debug-only component 정책을 class/API까지 엄격히 적용할지 결정이 필요하다. |
| `CPlayerController.h:17-24` | Shipping guard / UHT surface | `UFUNCTION(Exec)` 3개가 shipping class interface에도 남고 cpp body만 no-op 처리된다. runtime 동작은 차단되지만 command surface 자체를 shipping에서 제거할지 여부는 UHT/빌드 정책까지 포함해 결정해야 한다. |
| `CPlayerController.h:30-31` | Shipping guard / reflected member | `DebugOverlayTargetComponent` UPROPERTY가 shipping class layout/reflection에도 남는다. constructor 생성은 non-shipping guard되어 있으나 debug-only reflected member를 shipping에 남길지 판단이 필요하다. |

## 9. TargetComponent source chain / gameplay flow 검토

### NoIssue

| 위치 | 판단 |
| --- | --- |
| `CDebugOverlayHUD.cpp:331-425` | fallback chain은 `TargetComponent -> RecentCombatTarget -> WorldScanFallback` 순서로 구현되어 있고, 각 source 문구도 실제 선택 경로와 일치한다. |
| `CDebugOverlayHUD.cpp:359-381` | recent combat pair는 viewer pawn이 source 또는 target인 경우에만 Enemy로 resolve된다. 무관한 combat event를 Enemy panel source로 승격하지 않는다. |
| `CDebugOverlayHUD.cpp:403-425` | world scan fallback은 TargetComponent와 RecentCombatTarget 실패 이후에만 실행된다. 다중 enemy는 `Ambiguous(Count=N)`으로 차단되어 임의 선택 evidence가 되지 않는다. |
| `FDebugOverlaySnapshotStore.cpp:149-159` | recent combat pair는 weak actor pair와 name/time/frame/event만 저장하며 snapshot copy에 raw pointer를 넣지 않는다. |
| `FDebugOverlaySnapshotStore.cpp:338`, `377` | `RecordCombatTargetPacket`, `RecordCombatResult`에서 recent combat pair를 기록하며 기존 audit log format이나 gameplay result 흐름을 변경하지 않는다. |

### DecisionNeeded

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CPlayerController.cpp:216-220` | source claim 정확도 | `DebugOverlaySelectTarget`은 view trace 실패 시 nearest enemy fallback까지 수행한다. 사용자가 command 이름만 보면 view trace source로 이해할 수 있다. overlay에는 여전히 `EnemySource: TargetComponent`로 표시되므로, target component 내부에 selection method를 기록할지 결정이 필요하다. |
| `CPlayerController.cpp:212-229` | target set UX | `DebugOverlaySelectTarget` 실패 시 기존 explicit target을 clear한다. fallback chain으로 내려가는 설계에는 맞지만, 실패한 재선택이 기존 TargetComponent 고정을 해제해도 되는지는 운영 정책 판단이 필요하다. |
| `CDebugOverlayHUD.cpp:385-399` | fallback state 표시 | Recent combat pair가 stale/not matched인 경우 `EnemyRecentCombat: Stale/NotMatched`를 출력한 뒤 WorldScanFallback 또는 None/Ambiguous로 내려간다. 이 중첩 표시는 evidence 설명에는 도움이 되지만 화면이 길어질 수 있다. 유지할지 축약할지 결정이 필요하다. |

### Later

| 위치 | 기준 | 내용 |
| --- | --- | --- |
| `CPlayerController.cpp:272-275` | trace target resolve | view trace는 `HitResult.GetActor()`가 직접 `ACEnemy`일 때만 성공한다. Enemy 부착 무기/컴포넌트/프록시를 맞는 경우 nearest fallback으로 갈 수 있으므로 P1 안정화 이후 owner/attached actor resolve 보강 후보로 둔다. |
| `CDebugOverlayHUD.cpp:147-155` | evidence claim 범위 | Runtime LOD와 AI current value는 아직 `N/A` / `NotCaptured` placeholder다. P1 Runtime LOD / AI 보강 전까지 성공 evidence로 주장하지 않는다. |
| `FDebugOverlaySnapshotStore.cpp:149-159` | subject ownership | recent combat pair는 P1 TargetComponent 기준은 충족한다. 다만 Player/Enemy EventLog 분리 시 subject ownership 모델을 별도 설계해야 한다. |

## 10. Findings 요약

### LowRiskFix

| 항목 | 파일 | 제안 |
| --- | --- | --- |
| include group 정리 | `CDebugOverlayHUD.cpp`, `FDebugOverlaySnapshotStore.cpp`, `CPlayerController.cpp` | W05 include 순서에 맞춰 project Type/Core/Debug와 engine header 순서를 정리한다. |
| HUD helper 섹션 정리 | `CDebugOverlayHUD.cpp` | anonymous namespace 내부 helper를 formatting / status / snapshot / style 그룹으로 나눈다. |
| HUD append helper naming 검토 | `CDebugOverlayHUD.cpp` | `AddLine` 계열을 `Append...` 계열로 바꿀지 검토한다. |
| bool mutation helper naming | `CPlayerController.*` | `SelectDebugOverlayTargetFromView`, `SelectDebugOverlayNearestEnemy`를 `TrySelect...` 계열로 바꾸는 low-risk rename을 검토한다. |
| snapshot query naming | `FDebugOverlaySnapshotStore.*`, `CDebugOverlayHUD.cpp` | `GetSnapshotCopy`를 `TryGetSnapshotCopy`로 맞추는 low-risk rename을 검토한다. |
| debug-only helper guard 축소 | `CDebugOverlayHUD.cpp`, `FDebugOverlaySnapshotStore.cpp` | shipping에서도 컴파일되는 overlay 전용 anonymous namespace helper를 non-shipping guard 안으로 좁힐지 검토한다. |

### DecisionNeeded

| 항목 | 파일 | 결정 필요 내용 |
| --- | --- | --- |
| `GetDebugOverlayTargetSource()` 유지 여부 | `CDebugOverlayTargetComponent.*`, `CDebugOverlayHUD.cpp` | 현재 API는 존재하지만 HUD는 직접 `"TargetComponent"` 문자열을 출력한다. 제거, 사용, 또는 selection method 확장 중 결정이 필요하다. |
| view trace 실패 시 nearest fallback source 표시 | `CPlayerController.cpp`, `CDebugOverlayTargetComponent.*`, `CDebugOverlayHUD.cpp` | `DebugOverlaySelectTarget`이 nearest fallback으로 선택한 경우에도 `EnemySource: TargetComponent`만 표시된다. source claim을 더 세분화할지 결정이 필요하다. |
| 실패한 `DebugOverlaySelectTarget`이 기존 target을 clear할지 | `CPlayerController.cpp` | 현재는 선택 실패 시 explicit target을 해제한다. 운영상 이전 target 유지가 더 나은지 결정이 필요하다. |
| Exec command / reflected member shipping 노출 정책 | `CPlayerController.h`, `CDebugOverlayTargetComponent.*` | cpp body와 생성 경로는 no-op/guard되어 있으나 UFUNCTION/UPROPERTY/class surface는 shipping에도 남는다. UHT 안전성과 debug-only 엄격성 사이에서 결정이 필요하다. |
| `ResolveWorld`의 `const_cast` 유지 여부 | `FDebugOverlaySnapshotStore.cpp` | 실제 mutation은 없지만 W05 위험 신호다. helper signature 변경 또는 현 상태 유지 중 결정한다. |
| stale/not matched recent combat line 유지 여부 | `CDebugOverlayHUD.cpp` | fallback chain 설명력과 화면 길이 사이에서 유지/축약 여부를 결정한다. |

### Later

| 항목 | 파일 | 이유 |
| --- | --- | --- |
| Runtime LOD actual value | `CDebugOverlayHUD.cpp` | P1 Runtime LOD 보강 단계에서 처리한다. 현재는 성공 evidence로 쓰지 않는다. |
| AI current value 보강 | `CDebugOverlayHUD.cpp` | P1 AI 표시 보강 단계에서 처리한다. |
| Player/Enemy EventLog 분리 | `FDebugOverlaySnapshotStore.*`, `CDebugOverlayHUD.cpp` | subject ownership 설계 이후 처리한다. |
| EventLog category filter | `FDebugOverlaySnapshotStore.*`, `CDebugOverlayHUD.cpp` | P1 별도 구현 단위로 처리한다. |
| view trace owner/attached actor resolve | `CPlayerController.cpp` | 직접 hit actor만 `ACEnemy`로 cast하는 현재 구현을 P1 안정화 이후 보강한다. |

### NoIssue

| 항목 | 판단 |
| --- | --- |
| SnapshotStore API section | W05 section comment 기준에 맞게 gate / record / query / lifecycle이 분리되어 있다. |
| Store lifetime policy | World별 store, weak actor pair, name/time/frame 저장 방식은 장기 raw pointer 보관 정책을 위반하지 않는다. |
| gameplay flow 오염 | TargetComponent는 debug-only controller-owned component이며 기존 combat/action target flow를 변경하지 않는다. |
| Stagger getter | read-only getter로 panel 표시만 지원하며 store/event 확장을 하지 않는다. |

## 11. 다음 작업 제안

1. `LowRiskFix` cleanup 구현
   - include group 정리
   - HUD helper 섹션 정리
   - `Try...` naming 정리
   - 필요 시 `GetSnapshotCopy` naming 정리

2. `DecisionNeeded` 항목 사용자 결정
   - `GetDebugOverlayTargetSource()`를 제거할지 HUD에서 사용할지 결정
   - `TargetComponent` source를 `TargetComponent(ViewTrace)` / `TargetComponent(NearestFallback)`처럼 세분화할지 결정
   - 실패한 `DebugOverlaySelectTarget`이 기존 target을 clear할지 유지할지 결정
   - Exec command / UPROPERTY / component class shipping 노출 정책 결정
   - `ResolveWorld`의 `const_cast` 유지 여부 결정

3. P1 다음 기능 작업
   - EventLog category filter 설계/구현
   - Player/Enemy Recent/EventLog 분리 설계
   - Runtime LOD actual 표시 설계
