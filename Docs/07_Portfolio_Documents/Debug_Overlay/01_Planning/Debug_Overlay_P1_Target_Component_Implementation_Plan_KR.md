# Debug Overlay P1 Target Component Implementation Plan

## 1. 목적

이 문서는 `UCDebugOverlayTargetComponent`를 실제 구현하기 전 구현 contract를 고정한다.

P1 Target Selection 설계에서 결정한 기준은 다음과 같다.

- P1에서는 debug overlay 한정 component로 구현한다.
- component 이름은 `UCDebugOverlayTargetComponent`로 둔다.
- component 소유 위치는 `ACPlayerController`다.
- Enemy source chain은 `TargetComponent -> RecentCombatTarget -> WorldScanFallback`이다.
- `WorldScanFallback`은 최후 fallback으로만 유지한다.
- 범용 `UCTargetSelectionComponent` 승격은 브랜치 마감 후 별도 리팩터링 후보로 둔다.

이번 문서는 코드 구현이 아니라, 다음 구현 단계에서 흔들리면 안 되는 파일/API/연결/검증 기준을 정리한다.

## 2. 구현 대상 파일

P1 Target Component 구현 대상 파일은 다음으로 고정한다.

| 파일 | 목적 |
| --- | --- |
| `Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.h` | debug overlay target provider component 선언 |
| `Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.cpp` | weak target 보관, getter, stale 판단 구현 |
| `Source/Portfolio/Controller/CPlayerController.h` | component 멤버 선언 |
| `Source/Portfolio/Controller/CPlayerController.cpp` | controller-owned subobject 생성 |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp` | Enemy resolve chain 전환 |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h` | RecentCombatTarget query API 추가 후보 |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp` | recent combat pair 기록/query 구현 후보 |

`Build.cs`, config, asset, map 파일은 변경하지 않는다. 구현 중 `Build.cs` 변경이 필요해 보이면 멈추고 사용자에게 질문한다.

## 3. Component 책임

`UCDebugOverlayTargetComponent`는 debug overlay evidence용 target provider다.

필수 책임:

- 현재 overlay 대상으로 볼 Enemy를 `TWeakObjectPtr<AActor>`로 보관한다.
- HUD가 순간 조회할 수 있는 raw pointer getter를 제공한다.
- target source와 target summary 문자열을 제공한다.
- target invalid 또는 stale 상태를 판단할 수 있게 한다.
- debug overlay evidence claim을 위해 source를 명시한다.

금지 책임:

- gameplay target selection을 바꾸지 않는다.
- combat action target을 강제하지 않는다.
- action/reaction 실행 흐름에 개입하지 않는다.
- lock-on 상태를 만들지 않는다.
- target cycling UI를 구현하지 않는다.
- AI target selection을 변경하지 않는다.
- 기존 `ITargetContextProvider`를 확장하지 않는다.

## 4. Component API 후보

P1 구현 시 component API 후보는 다음으로 둔다.

```cpp
bool HasDebugOverlayTarget() const;
AActor* GetDebugOverlayTargetActor() const;
FString GetDebugOverlayTargetSummary() const;
FString GetDebugOverlayTargetSource() const;
void SetDebugOverlayTarget(AActor* InTargetActor);
void ClearDebugOverlayTarget();
```

구현 정책:

- 내부 저장은 `TWeakObjectPtr<AActor>`를 사용한다.
- raw pointer는 HUD draw 시점의 순간 조회 반환으로만 허용한다.
- actor name은 표시용 summary에 별도로 보관하거나 getter에서 `GetNameSafe`로 생성한다.
- target이 invalid면 `HasDebugOverlayTarget()`은 false를 반환한다.
- P1에서는 Blueprint 노출을 필수로 보지 않는다.

Blueprint 노출이나 console command가 필요해지면 구현 전에 사용자에게 질문한다.

## 5. `ACPlayerController` 연결 방식

`ACPlayerController`는 이미 `UCPlayerFeedbackComponent`를 controller-owned subobject로 보유한다. P1 구현은 이 패턴을 따른다.

구현 후보:

```cpp
UPROPERTY(VisibleAnywhere)
class UCDebugOverlayTargetComponent* DebugOverlayTargetComponent = nullptr;
```

생성 후보:

```cpp
DebugOverlayTargetComponent = CreateDefaultSubobject<UCDebugOverlayTargetComponent>(TEXT("DebugOverlayTarget"));
check(DebugOverlayTargetComponent);
```

정책:

- `ACPlayer`의 `FCharacterComponentReferences` 흐름은 건드리지 않는다.
- `ACPlayer`에 새 component를 붙이지 않는다.
- HUD는 `GetOwningPlayerController()`에서 component를 조회한다.
- component가 없으면 바로 `RecentCombatTarget` fallback으로 내려간다.

## 6. HUD Fallback Chain

HUD의 Enemy resolve 순서는 다음으로 전환한다.

```text
TargetComponent
RecentCombatTarget
WorldScanFallback
```

구현 순서:

1. `GetOwningPlayerController()`에서 `UCDebugOverlayTargetComponent`를 찾는다.
2. component가 valid target을 제공하면 해당 actor를 Enemy panel 대상으로 사용한다.
3. 표시 문구는 `EnemySource: TargetComponent`와 target summary를 사용한다.
4. component가 없거나 target invalid면 Store의 recent combat pair를 조회한다.
5. recent combat pair가 player 기준 상대 Enemy를 제공하면 `EnemySource: RecentCombatTarget`으로 표시한다.
6. recent combat pair가 stale/invalid이면 기존 world scan fallback으로 내려간다.
7. world scan 결과가 1개면 `EnemySource: WorldScanFallback`으로 표시한다.
8. world scan 결과가 0개면 `EnemySource: None`으로 표시한다.
9. world scan 결과가 여러 개면 `EnemySource: Ambiguous(Count=N)`으로 표시한다.

기존 `RefreshCachedEnemyIfNeeded()` / `ResolveDisplayEnemy()`는 삭제하지 않고 world scan fallback 구현부로 재배치하거나 유지한다.

## 7. HUD 표시 문구

P1 표시 문구 후보는 다음으로 둔다.

TargetComponent 성공:

```text
EnemySource: TargetComponent
EnemyTarget: Selected=BP_CEnemy_C_1
```

RecentCombatTarget 성공:

```text
EnemySource: RecentCombatTarget
EnemyRecentCombat: Source=BP_CPlayer_0 Target=BP_CEnemy_C_1 Age=0.42
```

WorldScanFallback 성공:

```text
EnemySource: WorldScanFallback
EnemyFallback: Selected=BP_CEnemy_C_1 Policy=FirstValid Count=1
```

실패/보류:

```text
EnemySource: None
EnemySource: Ambiguous(Count=2)
EnemySource: Stale
```

최종 문구가 HUD 폭을 넘거나 capture 가독성을 해치면 구현 중 멈추고 사용자에게 문구 축약 여부를 질문한다.

## 8. Store Recent Combat Pair

RecentCombatTarget fallback은 Store 기반으로 구현한다.

단일 `TargetActor`를 저장하지 않는다. combat 방향에 따라 enemy 후보가 `SourceActor`일 수도 있고 `TargetActor`일 수도 있기 때문이다.

Store 내부 후보 구조:

```cpp
struct FDebugOverlayRecentCombatPair
{
	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<AActor> TargetActor;
	FString SourceName;
	FString TargetName;
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString EventName;
};
```

기록 지점:

- `FDebugOverlaySnapshotStore::RecordCombatTargetPacket`
- `FDebugOverlaySnapshotStore::RecordCombatResult`

정책:

- `FDebugOverlaySnapshot` copy에는 UObject pointer를 넣지 않는다.
- recent combat pair는 Store 내부 별도 상태로 둔다.
- name, frame, time, event name은 pointer invalid 상황 설명을 위해 함께 보관한다.
- Shipping에서는 record/query가 no-op 또는 false/empty가 되어야 한다.

## 9. Store Query API 후보

HUD가 recent combat pair를 조회하기 위한 API 후보는 다음으로 둔다.

```cpp
static bool TryGetRecentCombatPair(
	const UObject* InWorldContextObject,
	FDebugOverlayRecentCombatPair& OutPair);
```

P1 최소 구현에서는 Store가 Enemy를 직접 resolve하지 않는다. Store는 recent combat pair만 반환하고, HUD가 owning pawn 기준으로 상대편 Enemy를 고른다.

후속 확장이 필요하면 result struct를 둘 수 있다.

```cpp
struct FDebugOverlayTargetResolveResult
{
	TWeakObjectPtr<AActor> TargetActor;
	FString Source;
	FString Summary;
	bool bResolved = false;
	bool bStale = false;
};
```

P1 최소 구현에서는 header 확장 범위를 줄이기 위해 pair query API를 우선 검토한다. result struct가 필요해지면 `FDebugOverlaySnapshotTypes.h` 또는 component header가 아니라 Store header 쪽에 둘지 검토한다.

## 10. Player 기준 상대 Enemy 선택

RecentCombatTarget query는 owning player actor 기준으로 상대편 Enemy를 고른다.

정책:

| 조건 | 결과 |
| --- | --- |
| `SourceActor == Player`, `TargetActor`가 Enemy | `TargetActor` 선택 |
| `TargetActor == Player`, `SourceActor`가 Enemy | `SourceActor` 선택 |
| Source/Target 둘 다 Player 아님 | unresolved |
| Source/Target 둘 다 Enemy 또는 둘 다 invalid | ambiguous 또는 stale |
| weak pointer invalid | stale |

`ACEnemy` cast가 실패하면 Enemy panel 대상으로 사용하지 않는다.

## 11. Stale 정책

P1 구현 전 기본 후보는 시간 기준 stale이다.

권장 후보:

```text
RecentCombatTarget 유효 시간: 3.0초
```

판단:

- 2초는 빠른 전투 캡처 중 fallback이 자주 사라질 수 있다.
- 3초는 최근 combat 대상이라는 의미를 유지하면서 evidence 오해를 줄이는 타협점이다.
- 5초 이상은 오래된 target을 현재 target처럼 보이게 할 위험이 있다.

이 값은 구현 전 사용자 결정이 필요하면 질문한다. 별도 결정이 없으면 문서상 후보로만 유지하고 구현 프롬프트에서 다시 확정한다.

## 12. Shipping / Build 정책

P1 구현은 debug overlay evidence용이므로 shipping 노출을 피한다.

정책:

- HUD 표시 로직은 기존처럼 `#if !UE_BUILD_SHIPPING` 내부에서 동작한다.
- Store recent combat pair 기록/query도 `#if !UE_BUILD_SHIPPING` 보호를 따른다.
- component class 자체는 shipping build에 포함될 수 있으나, debug overlay 사용부는 shipping에서 no-op이어야 한다.
- component가 shipping에서 존재하더라도 gameplay flow와 연결하지 않는다.
- UMG/Slate dependency를 추가하지 않는다.
- `Build.cs` 변경은 하지 않는 방향을 우선한다.

구현 중 `Build.cs` 변경 없이는 컴파일이 어렵다고 판단되면 사용자에게 질문한다.

## 13. 구현 순서

P1 Target Component 구현 순서는 다음으로 고정한다.

1. `UCDebugOverlayTargetComponent` type/API 추가
2. `ACPlayerController`에 controller-owned subobject 연결
3. Store 내부 recent combat pair 구조 추가
4. `RecordCombatTargetPacket` / `RecordCombatResult`에서 pair 기록
5. Store recent combat pair query API 추가
6. HUD에서 player 기준 recent combat 상대 Enemy resolve 구현
7. HUD enemy resolve chain을 `TargetComponent -> RecentCombatTarget -> WorldScanFallback`으로 전환
8. build 검증
9. PIE 수동 확인

PIE 캡처/패키징은 P1 검증 이후로 미룬다.

## 14. 구현 중 질문해야 할 결정 요소

다음 상황이 생기면 임의 결정하지 않고 사용자에게 질문한다.

- stale timeout을 3.0초로 확정할지 여부
- `UCDebugOverlayTargetComponent` Blueprint 노출 필요 여부
- target set/clear를 console command, debug input, 자동 recent combat 중 무엇으로 시작할지 여부
- Store query API를 pair output param으로 둘지 result struct로 확장할지 여부
- HUD 표시 문구가 길어서 layout 조정이 필요한 경우
- `Build.cs` 변경 필요성이 생기는 경우
- 범용 Target Component로 확장해야 할 요구가 다시 생기는 경우

## 15. 제외 범위

이번 구현 계획의 제외 범위는 다음과 같다.

- 코드 구현
- 범용 `UCTargetSelectionComponent`
- lock-on system
- target cycling UI
- combat action target 강제
- camera/aim assist
- AI target selection 변경
- 기존 `ITargetContextProvider` 확장
- 기존 audit log format 변경
- `.umap`, `.uasset`, config, `Build.cs` 변경
- 최종 촬영/패키징

## 16. 검증 계획

구현 계획 문서 단계의 검증:

- 문서가 P1 Target Selection 설계와 충돌하지 않는지 확인한다.
- 구현 대상 파일이 P1 scope 밖으로 늘어나지 않았는지 확인한다.
- code/asset/config/Build.cs 변경이 없는지 확인한다.
- `git diff --check`를 수행한다.

실제 구현 단계의 검증 후보:

- `PortfolioEditor Win64 Development` 빌드
- Shipping guard 확인
- `EnemySource: TargetComponent` 표시 확인
- TargetComponent target invalid 시 `RecentCombatTarget` fallback 확인
- RecentCombatTarget stale 시 `WorldScanFallback` fallback 확인
- 다중 enemy에서 `Ambiguous(Count=N)` 확인

## 17. 완료 기준

이 문서가 완료되면 다음이 확정된 것으로 본다.

- `UCDebugOverlayTargetComponent` 구현 파일/API 후보
- `ACPlayerController` 연결 방식
- Store recent combat pair 기록/query 방향
- HUD fallback chain 전환 기준
- Shipping/build 정책
- 구현 중 질문해야 할 결정 요소

## 18. 다음 작업

다음 작업은 `P1 Target Component 실제 구현`이다.

구현 단계에서는 이 문서 범위를 기준으로 component, controller 연결, Store recent combat pair, HUD fallback chain만 최소 변경한다.
