# Debug Overlay P1 Target Selection Design

## 1. 목적

이 문서는 P1 debug overlay에서 Enemy panel에 표시할 대상을 어떻게 결정할지 고정한다.

P0.5에서는 `WorldScanFallback`으로 월드에 존재하는 단일 `ACEnemy`를 찾아 표시했다. 이 방식은 TestRoom의 단일 enemy 확인에는 충분하지만, 최종 evidence에서 "이 Enemy panel이 실제 현재 대상 Enemy를 표시한다"는 claim으로 사용하기에는 약하다.

P1의 목표는 Enemy panel source chain을 명확히 만들고, `WorldScanFallback`을 최후 fallback으로 낮추는 것이다.

> Update: P1 Target Selection 최종 정책은 `Debug_Overlay_P1_Target_Selection_Decision_KR.md`를 우선한다. 이후 구현은 자동 fallback chain이 아니라 `TargetComponent.Nearest`, `None` 기반 명시 target 정책을 따른다. 이 문서의 기존 fallback chain과 trace 설명은 과거 설계 맥락으로만 본다.

## 2. 최종 결정

P1에서는 debug overlay 한정 component를 먼저 구현한다.

| 항목 | 결정 |
| --- | --- |
| P1 component 이름 | `UCDebugOverlayTargetComponent` |
| 소유 위치 | `ACPlayerController` |
| 목적 | debug overlay가 읽을 selected enemy 제공 |
| 범위 | overlay evidence용 target provider |
| 비범위 | 범용 combat targeting, lock-on, target cycling UI |
| 후속 리팩터링 후보 | 브랜치 마감 후 `UCTargetSelectionComponent` 또는 `UCTargetProviderComponent`로 승격 검토 |

이 결정은 "현재 브랜치에서는 debug overlay evidence를 닫고, 범용 target system은 별도 리팩터링으로 분리한다"는 정책을 따른다.

## 3. 왜 Debug 전용으로 먼저 닫는가

범용 Target Component로 바로 설계하면 다음 범위가 함께 열린다.

- lock-on 상태
- target cycling
- camera 기준 target 후보 정렬
- 공격 방향 보정
- combat action target 강제
- UI/UX 표시
- AI target selection과의 관계

현재 브랜치의 목표는 debug overlay evidence다. 따라서 P1에서는 gameplay flow를 바꾸지 않고, overlay가 "어떤 Enemy를 보고 있는지"만 신뢰도 있게 표시하는 데 집중한다.

## 4. 기존 코드 근거

| 코드 위치 | 근거 | 판단 |
| --- | --- | --- |
| `Source/Portfolio/Controller/CPlayerController.h` | `UCPlayerFeedbackComponent`를 controller-owned component로 보유 | target provider component를 controller에 붙이는 패턴이 가능 |
| `Source/Portfolio/Controller/CPlayerController.cpp` | 입력 처리와 player intent dispatch 담당 | target selection 의도와 가까운 위치 |
| `Source/Portfolio/Character/Player/CPlayer.h` | 전투/상태 component와 `FCharacterComponentReferences` 흐름이 많음 | P1에서 더 건드리면 범위가 커질 수 있음 |
| `Source/Portfolio/Interface/TargetContextProvider.h` | AI perception용 `GetTargetPriority()` interface | overlay target provider와 의미가 다르므로 재사용하지 않음 |
| `Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp` | 현재 `TActorIterator<ACEnemy>` 기반 fallback 사용 | P1에서 최후 fallback으로 낮춤 |
| `Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp` | combat target/result hook에서 source/target name 저장 | RecentCombatTarget diagnostic의 근거 |
| `Source/Portfolio/Type/CCombatSignalTargetTypes.h` | `FCombatSignalTargetPacket`에 Source/Target actor 존재 | Store recent combat pair 기록 가능 |
| `Source/Portfolio/Type/CCombatResultTypes.h` | `FCombatResultPacket`에 Source/Target actor 존재 | Store recent combat pair 기록 가능 |

## 5. Source Policy

P1 Enemy Selection source policy는 다음으로 고정한다.

```text
TargetComponent.Nearest
None
```

표시 문구는 다음 기준을 사용한다.

| Source | 의미 | Evidence claim |
| --- | --- | --- |
| `EnemySource: TargetComponent.Nearest` | 사용자 명령으로 nearest enemy를 명시 선택한 enemy | 명시 command 기반 target selection evidence |
| `EnemySource: None` | 명시 target 없음 | 성공 evidence로 사용하지 않음 |
| `EnemySource: RecentCombatTarget` | 최근 combat pair에서 player 기준 상대 Enemy를 선택 | P1 기본 source chain에서 제외. diagnostic 후보 |
| `EnemySource: WorldScanFallback` | 월드 scan 결과 enemy가 1개라서 선택 | P1 기본 source chain에서 제외. diagnostic/debug fallback 후보 |
| `EnemySource: Ambiguous` | 다중 후보로 대상 확정 불가 | 특정 Enemy evidence로 사용하지 않음 |
| `EnemySource: Stale` | 이전 source가 invalid 또는 timeout됨 | fallback 또는 재확인 필요 |

## 6. `UCDebugOverlayTargetComponent` 최소 책임

P1 component는 debug overlay 전용 read provider다.

필수 책임:

- 현재 overlay 대상 Enemy를 약참조로 보관한다.
- HUD가 현재 selected enemy를 조회할 수 있게 한다.
- target source와 summary 문자열을 제공한다.
- invalid/stale 상태를 구분할 수 있게 한다.
- raw pointer를 장기 보관하지 않는다.

비책임:

- combat target을 강제하지 않는다.
- action/reaction 실행 대상을 바꾸지 않는다.
- lock-on 상태를 만들지 않는다.
- target cycling UI를 제공하지 않는다.
- camera assist나 aim correction을 하지 않는다.
- AI target selection에 관여하지 않는다.

## 7. Component 위치 결정

`UCDebugOverlayTargetComponent`는 `ACPlayerController`가 소유한다.

이유:

- target selection은 입력, 카메라, player intent와 가깝다.
- `ACPlayerController`에는 이미 controller-owned component 패턴이 있다.
- `ACPlayer`의 character component reference injection 흐름을 건드리지 않아도 된다.
- debug overlay가 `GetOwningPlayerController()`에서 provider를 찾기 쉽다.

대안으로 `ACPlayer` 소유도 가능하지만 P1에서는 선택하지 않는다. `ACPlayer`는 이미 전투/상태 component가 많고, debug target provider를 붙이면 character setup 흐름까지 건드릴 가능성이 있다.

## 8. 기존 `ITargetContextProvider` 재사용 금지

`ITargetContextProvider`는 AI perception에서 "이 actor가 target 후보로 유효한가"와 priority를 제공하는 interface다.

P1의 target provider는 "player/controller가 현재 overlay 대상으로 선택한 Enemy가 무엇인가"를 제공한다. 의미가 다르므로 기존 interface를 확장하거나 재사용하지 않는다.

필요하면 P1 구현에서 별도 debug 전용 query API를 사용한다.

## 9. Target Provider API 후보

구체 API 이름은 구현 단계에서 코드 스타일에 맞춰 확정한다. 설계 기준은 다음과 같다.

```cpp
bool HasDebugOverlayTarget() const;
AActor* GetDebugOverlayTargetActor() const;
FString GetDebugOverlayTargetSummary() const;
FString GetDebugOverlayTargetSource() const;
void SetDebugOverlayTarget(AActor* InTargetActor);
void ClearDebugOverlayTarget();
```

권장 타입:

- 내부 보관: `TWeakObjectPtr<AActor>`
- HUD 반환: 순간 조회용 raw pointer 허용
- 표시용: actor name / source / state 문자열 별도 제공

P1에서는 Blueprint 노출을 필수로 보지 않는다. 에디터 수동 지정이나 테스트 입력이 필요하다고 판단되면 별도 결정 후 추가한다.

## 10. HUD 연결 방향

HUD는 target 선택의 주체가 아니라 consumer다.

P1 HUD enemy resolve 순서:

1. `GetOwningPlayerController()`에서 `UCDebugOverlayTargetComponent` 조회
2. component가 valid target을 제공하면 `EnemySource: TargetComponent.Nearest`
3. component가 없거나 target invalid면 `EnemySource: None`
4. `RecentCombatTarget`과 `WorldScanFallback`은 기본 HUD path에서 자동 표시하지 않고 diagnostic 후보로만 둔다.

HUD는 이 source를 화면에 명시한다. 최종 evidence에서는 `TargetComponent.Nearest` source 캡처만 target selection claim으로 사용한다.

## 11. RecentCombatTarget Diagnostic 설계

RecentCombatTarget은 단일 `TargetActor`를 저장하는 방식으로 설계하지 않는다.

combat 흐름에서는 방향이 바뀐다.

| 상황 | SourceActor | TargetActor | Enemy panel 후보 |
| --- | --- | --- | --- |
| Player가 Enemy를 공격 | Player | Enemy | TargetActor |
| Enemy가 Player를 공격 | Enemy | Player | SourceActor |

따라서 Store에는 최근 combat pair를 기록하고, HUD가 owning pawn 기준으로 상대편 Enemy를 선택한다.

권장 Store 내부 구조:

```cpp
TWeakObjectPtr<AActor> SourceActor;
TWeakObjectPtr<AActor> TargetActor;
FString SourceName;
FString TargetName;
uint64 FrameNumber;
float WorldTimeSeconds;
FString EventName;
```

정책:

- Store snapshot copy에 UObject pointer를 직접 넣지 않는다.
- 최근 combat candidate는 Store 내부 별도 상태로 보관한다.
- HUD는 전용 query API로 weak pair를 조회한다.
- weak pointer invalid 시 stale로 처리한다.
- stale 상태는 diagnostic 후보로만 기록하며, P1 기본 HUD path에서는 `WorldScanFallback`으로 자동 하강하지 않는다.

## 12. Stale 정책

P1 구현 전 stale 기준은 다음 후보 중 하나로 결정한다.

| 후보 | 설명 | 권장 |
| --- | --- | --- |
| 시간 기준 | 최근 combat 후 2~3초 이내만 유효 | 권장 |
| frame 기준 | 기록 frame과 현재 frame 차이 기준 | 보조 |
| explicit clear | combat 종료 또는 target clear 시 제거 | 후속 |

P1 최소 구현에서는 시간 기준을 우선 검토한다. stale timeout 기본값은 구현 단계에서 사용자 결정이 필요하면 질문한다.

## 13. WorldScanFallback 유지 정책

`WorldScanFallback`은 삭제하지 않는다.

역할:

- diagnostic/debug fallback 검증
- P1 이전 WorldScanFallback 기반 동작 비교
- TestRoom 단일 enemy 상황에서 별도 진단이 필요할 때 보조 확인

제한:

- 다중 enemy면 `Ambiguous(Count=N)`로 표시한다.
- enemy가 없으면 `None` 또는 `NotCaptured(NoEnemy)`로 표시한다.
- `WorldScanFallback`을 `TargetComponent` 기반 evidence처럼 설명하지 않는다.
- 최종 제출용 enemy claim은 가능한 한 `TargetComponent` source 기준으로만 사용한다.

## 14. 다중 Enemy 정책

P1 최소 정책:

| 상황 | 표시 |
| --- | --- |
| TargetComponent valid target 있음 | `TargetComponent.Nearest` |
| TargetComponent 없음 | `None` |
| TargetComponent invalid | `None` |
| RecentCombatTarget valid | P1 기본 Enemy panel에는 자동 표시하지 않음 |
| world enemy 1개 | P1 기본 Enemy panel에는 자동 표시하지 않음 |

임의 첫 번째 Enemy를 성공 evidence처럼 표시하지 않는다.

## 15. 구현 대상 후보

다음 구현 단계의 후보 파일은 문서상 다음으로 둔다.

```text
Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.h
Source/Portfolio/Core/Debug/CDebugOverlayTargetComponent.cpp
Source/Portfolio/Controller/CPlayerController.h
Source/Portfolio/Controller/CPlayerController.cpp
Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.h
Source/Portfolio/Core/Debug/FDebugOverlaySnapshotStore.cpp
```

`Build.cs` 변경은 하지 않는 방향을 우선한다. 기존 module dependency 안에서 컴파일이 되지 않거나 include 정책상 문제가 생기면 구현 중 멈추고 사용자에게 질문한다.

## 16. 구현 전 결정 요소

구현 중 아래 항목이 필요해지면 임의 결정하지 않고 사용자에게 질문한다.

- stale timeout 기본값
- `UCDebugOverlayTargetComponent`를 Blueprint 노출할지 여부
- target set/clear 입력 또는 console command가 필요한지 여부
- Store에 weak recent combat pair를 추가하는 API 이름
- HUD 표시 문구가 길어져 layout 조정이 필요한 경우
- `Build.cs` 변경이 필요해 보이는 경우
- P1 범위를 넘어 combat gameplay target으로 연결해야 할 것처럼 보이는 경우

## 17. P1 비목표

P1 Target Selection에서는 다음을 하지 않는다.

- 범용 `UCTargetSelectionComponent` 구현
- lock-on system 구현
- target cycling UI 구현
- combat action target 강제
- camera/aim assist 구현
- AI target selection 변경
- 기존 `ITargetContextProvider` 확장
- 기존 audit log format 변경
- `.umap`, `.uasset`, config, `Build.cs` 변경
- 최종 촬영/패키징

## 18. 완료 기준

이 설계 기준으로 다음 단계에 들어갈 수 있다.

- `UCDebugOverlayTargetComponent`를 P1 debug-only provider로 고정
- component 소유 위치를 `ACPlayerController`로 고정
- source 표시를 `TargetComponent.Nearest`, `None`으로 고정
- `WorldScanFallback`을 diagnostic/debug fallback 후보로 제한
- RecentCombatTarget은 Store weak source/target pair 기반 diagnostic 후보로 둠
- 기존 `ITargetContextProvider`와 의미를 분리
- 범용 Target Component 승격은 브랜치 마감 후 리팩터링 후보로 분리

## 19. 다음 작업

다음 작업은 `P1 Target Component 구현 계획 문서 작성`이다.

구현 계획 문서에서는 `UCDebugOverlayTargetComponent`의 파일/API, `ACPlayerController` 연결 방식, source type 저장, HUD의 명시 target 표시 정책 변경 범위를 구현 단위로 확정한다.
