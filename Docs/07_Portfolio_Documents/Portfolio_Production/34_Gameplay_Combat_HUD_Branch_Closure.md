# Stellar Combat HUD 브랜치 마감 기록

## 기준과 결론

- 기준일: 2026-09-20.
- 브랜치: `feat/stellar-combat-hud`.
- 분기 기준: `48eaae82` (`main`의 기존 에셋 통합 완료점).
- 검토한 구현 HEAD: `65157687`.
- 커밋된 변경 규모: 86개 파일, +4456 / -564. 바이너리 uasset의 Git 통계는 LFS 포인터 줄 수이며 실제 에셋 편집량이 아니다.
- 검토 시 작업 트리에 있던 `CPixelAlignedGaugeImage.cpp`의 선언 줄바꿈·공백 변경과 검토 중 발생한 `.h`의 설명 주석 삭제는 사용자 수정으로 보존했다. 계산 로직 변경은 없다.
- 마감 검토에서 새 기능은 추가하지 않았다. 줄 끝 공백 정리와 오래된 문서의 현행화만 수행했다.

검토한 소스 변경과 실행한 검사 범위에서 마감을 막는 기능 결함은 발견하지 않았다. Editor 빌드, HUD 12개와 Balance 2개 자동 테스트가 통과했고 사용자가 PIE를 확인했다. 이는 모든 플랫폼·모든 전투 시나리오의 무결함 보장이 아니라 아래 범위에 대한 마감 판단이다.

이 문서는 작업 결과 기록이다. 문서 작성 자체가 commit/push/PR 생성/merge 수행을 뜻하지 않는다.

## 1. 목표와 실제 완료 범위

기존 전투 시스템을 HUD에 표시하되 UI가 게임플레이 판정을 새로 소유하지 않도록 구성했다. 초기에 시각 배치·리소스만 연결한 뒤, 가드·닷지·처형의 실행 가능 상태를 실제 판정 경로로 조회하도록 확장했다.

| 영역 | 무엇을 바꿨는가 | 이유 |
| --- | --- | --- |
| HUD 구조 | Controller → Presenter → ViewData → Native Widget | 전투 관측과 화면 구성의 책임 분리 |
| 값 변경 관측 | Health/Balance 값 변경 delegate 추가 | 매 프레임 전체 자원 UI를 갱신하지 않기 위해 |
| 액션 상태 | 가드·닷지·처형 조회 및 패링 결과 강조 | 장식 아이콘과 실제 사용 가능 상태 구분 |
| 자원 시각화 | 플레이어 고정 단위, 보스 고정 폭·비율, 픽셀 정렬 | 최대치 의미를 유지하면서 셀 크기·틈의 불균일 완화 |
| 처형 시작 | HUD와 요청이 공통 시작 평가 사용 | 조회와 실제 요청의 조건 차이 및 진입부 중복 축소 |
| Balance/취소 | 단계·serial 검사, 커밋 전후 취소 분리, 복구 보완 | 지연 이벤트와 강제 취소로 상태가 남는 경로 방어 |
| 디버그 UI | 패널만 DebugCanvas 전경에 렌더 | 진단 텍스트가 전투 HUD 뒤에 가려지지 않게 함 |
| 리소스·문서 | 전용 폰트·아이콘·원본·라이선스·제작 기록 | 재사용/재생성과 작업 근거 보존 |

## 2. HUD 데이터 흐름과 수명

```text
CPlayerController: Possess/UnPossess 및 참조 동기화
  → CCombatHUDPresenterComponent
     ├ Health·Balance·CombatTarget 이벤트 → RefreshView
     ├ PostUpdateWork 액션 조회 → 변경 시 RefreshActions 결과 전달
     └ 확정된 자기 대상 패링 결과 → 0.25초 강조
  → FCombatHUDViewData / FHUDActionViewData
  → CCombatHUDWidget: 표시·배치·색상
     → CCombatHUDGaugeLayout / CPixelAlignedGaugeImage
```

- `BeginPlay`에서 로컬 Controller에만 HUD를 생성한다. 기본값은 C++ Widget이며 `WidgetClass` 설정 확장점은 유지한다.
- `SetControlledPlayer`는 이전 이벤트 해제 → 타깃 해제 → 패링 런타임 초기화 → 참조 캐시 → Tick 설정 → 이벤트 연결 → 타깃/표시 갱신 순서다.
- 타깃 전환 시 이전 Health/Balance 구독을 제거한다. 죽거나 파괴 중이거나 선택이 바뀐 타깃은 표시하지 않는다.
- `EndPlay`는 재연결을 차단하고 플레이어 구독·Widget을 정리한다.
- Widget은 재구성 시 배열·위젯 참조·HP 열 캐시를 초기화하고 저장된 ViewData를 다시 적용한다.
- `CEnemy`에는 Target Marker의 소켓 설정과 별도 섹션으로 `CombatHUDDisplayName`을 둔다.

## 3. 게이지·숫자·아이콘

| 대상 | 단위/구조 | 현재 데이터 상태 |
| --- | --- | --- |
| Player HP | 3행, 100/열, 최대값에 따라 열 수 구성 | 실제 Health |
| BU·BE | 2×2 셀/유닛당 200, 셀당 50, 알파 채움 | Editor 샘플 |
| Player SH | 2×10 셀/유닛당 400, 열당 40 | Editor 샘플 |
| Target HP | 114열, 고정 폭 안에서 비율 채움 | 실제 Health |
| Target SH | 22열×5묶음, 묶음당 20% | Editor 샘플 |
| Target Balance | Threshold - CurrentCount, 다이아몬드와 숫자 | 실제 Balance |

- 논리 좌표와 최종 픽셀 좌표를 구분한다. 논리 틈 2.5를 무조건 2 또는 3으로 바꾸는 대신 최종 배율에서 공통 셀 크기·간격을 정수화해 반복한다.
- 완충 셀을 같은 격자에 놓되 부분 충전 경계는 실제 비율을 유지한다. 스크린샷을 사후 리사이즈한 결과까지 동일 픽셀을 보장하지 않는다.
- HP 0, 값 미확보, 미구현 자원을 구분한다. 소수 HP는 숫자 표시만 올림하고 게임 값은 변경하지 않는다.
- Player HP 표시 한도는 77열/7700이다. 초과 시 경고·열 제한을 적용하고 수치는 그대로 보여준다. Balance 도형은 최대 64개로 제한한다. 이 정책은 모든 성장 수치를 동일 비율로 압축하는 기능이 아니다.
- 아이콘은 Guard/Dodge/Counter/Execution/Rush와 GuardBreak 상태용, Vial 총 7개 Texture다. GuardBreak는 사용 중인 상태 아이콘이 아니라 준비된 리소스다.
- 일반 텍스트/숫자는 `F_HUDLabel`, 대상 이름은 `F_HUDName`이다. 이름에만 자간을 적용한다. Oxanium Regular/Medium과 Noto 한글 fallback을 보관한다.
- 샘플 자원은 SAMPLE 라벨로 구분하고 비 Editor 빌드에는 표시하지 않는다. TODO 문구는 제거했다.

## 4. 가용성 조회와 실행 평가

### 준비된 데이터 조회

`CActionComponent::FindPreparedActionContext`와 `CReactionComponent::FindPreparedGlobalReactionContext`는 이미 준비된 데이터/실행 객체 맵에서 context를 조립한다. HUD 조회 때문에 executor를 생성하거나 몽타주를 재생하지 않는다.

### Action / Reaction Orchestrator

- Action: `QueryCombatActionAvailability`가 Intent를 Key로 해석하고 `QueryPreparedActionAvailability`에 위임한다.
- Reaction: `QueryPreparedExecutionReactionAvailability`가 처형 타입을 검증·Key 구성하고 `QueryPreparedReactionAvailability`에 위임한다.
- 공통 평가 부분을 `EvaluateActionContext` / `EvaluateReactionContext`로 분리해 조회와 기존 실제 요청이 재사용한다.
- Action은 Start/Intervene를 허용한다. 처형 Reaction 조회는 `bRequireStart=true`로 Start만 허용한다. 예약·지연을 즉시 사용 가능으로 표시하지 않는다.
- 실행 요청에서 이미 구성한 Query를 평가 함수에 전달한다. 같은 요청에서 Snapshot을 다시 만드는 중복을 피한다.
- Overlay API 최종 이름은 `BuildOverlaySnapshot`이다. 정책 레지스트리를 갱신하고 현재 상태를 수집하는 기존 책임을 유지한다. 중간에 논의했던 갱신 없는 Read 경로는 최종 구조가 아니다.
- 이 조회는 게임플레이 실행 부작용을 만들지 않는다는 뜻이지 내부 캐시 갱신까지 전혀 없다는 뜻은 아니다.

### 처형 공통 시작 평가

`EvaluateExecutionStart`는 시전자 상태, 타깃 Snapshot, 거리·방향, 대상 세션/처형 기회, 준비된 데이터, 양측 실행 평가, 피해 적용 가능 여부를 확인한다.

- HUD 조회는 가능 여부와 Block을 반환한다.
- 실제 요청은 요청 시점에 새로 평가하고, 그 호출의 Snapshot·대상·일반 처형 피해량으로 예약을 진행한다.
- 실패 상세는 평가 결과에 담아 실제 요청에서 로그로 기록한다. 매 프레임 HUD 조회가 거절 로그를 반복하지 않는다.
- 대상의 예약 수락, 예약 후 시작, 활성화, 커밋 검증은 변경 경계의 책임이므로 유지한다. 모든 검사가 단 한 번으로 통합된 것은 아니다.
- HUD의 Ready는 이후 몽타주 재생 성공까지 보장하는 약속이 아니다.

## 5. 게임플레이 생명주기 보완

### Health / Balance 관측

- Health 초기화·피해·회복·최대값 변경·사망에서 최종 값이 달라졌을 때 `OnHealthValuesChanged`를 보낸다.
- Balance 값 알림을 `NotifyBalanceValuesChanged`에 모았다. 상태/표현 알림과 Reaction 실행 요청은 역할별로 구분한다.
- Reset은 타이머·예약·복구 카운터 및 값을 정리한 뒤 표현 → 생명주기 → 값 순으로 알린다. 관측자가 중간 초기화 상태를 보지 않도록 한다.
- Shutdown은 게임플레이 상태 전이 알림을 생략하고 표현/값 관측을 정리하며 패킷 중복 방지 이력을 비운다. Reset과 완전히 같은 동작으로 합치지 않는다.

### 경계·복구·취소

- 같은 lifecycle serial이라도 이미 끝난 CollapseIn/CollapseOut/ExecutionRecovery 단계의 늦은 종료 이벤트는 현재 단계와 대조해 무시한다.
- 예약만 취소하면 기존 표현을 유지한다. 활성화된 처형을 커밋 전에 취소하면 Collapse 자세와 남은 Loop 시간을 복구한다.
- 커밋 전 대상 취소는 예약 해제, 커밋 후 취소는 동일 serial의 `ExecutionPrimaryCommitted`가 남아 있을 때 Balance를 중단한다. 이미 적용한 HP 피해는 되돌리지 않는다.
- 세션 초기화 → 로컬 실행 취소 → Balance 정리 순서로 처리한다. 정상 완료는 양측 terminal 확인 뒤 기존 Down 전환 경로를 유지한다.
- 복구 재시도 지연이 0이면 `SetTimerForNextTick`을 사용한다. 동기 재귀나 0초 Timer의 미실행에 의존하지 않는다. 재시도 횟수 제한과 취소를 유지한다.
- Collaboration 참조를 교체하기 전에 이전 참가자 이벤트를 해제한다. 새 객체에만 Remove를 하는 방식으로 이전 구독이 남지 않게 한다.

## 6. 소스·에셋 검토 범위

| 파일군 | 검토 내용 |
| --- | --- |
| `CCombatHUDPresenterComponent.*` | 소유권, 이벤트 연결/해제, 상태 조회, 사망·타깃 전환 |
| `CCombatHUDWidget.*`, `CCombatHUDTypes.h` | 트리 재구성, 값/액션 분리, 샘플, 표시 상한, 수치 유효성 |
| `CCombatHUDGaugeLayout.h`, `CPixelAlignedGaugeImage.*` | 논리/픽셀 좌표, 부분 채움, 보스 정렬 |
| `CActionComponent.*`, `CReactionComponent.*` | 생성 없는 준비 데이터 조회 |
| `CActionOrchestratorComponent.*`, `CReactionOrchestratorComponent.*` | 조회/실행 공통 평가, 결과·사유 대칭성 |
| `CExecutionCollaborationComponent.*`, 관련 Types | 공통 시작 평가, 예약 경계, 이벤트 재연결, 커밋 전후 취소 |
| `CBalanceComponent.*`, `CBalanceTypes.h`, `CHealthComponent.*` | 관측 순서, Reset/Shutdown 차이, 단계 경계·타이머 |
| `CObservableOverlayComponent.*` | Build 이름과 레지스트리 갱신 유지 |
| `CPlayerController.*`, `CEnemy.h`, `CDebugOverlayHUD.cpp` | 통합 지점, 이름 설정, 디버그 표시 순서 |
| Runtime/Editor Build.cs | Slate/UMG/렌더·에셋 도구 의존성 |
| HUD/Balance 테스트 4개 파일 | fixture, 가용성·수명·렌더·경계 검증 |
| `CCombatHUDAssetsCommandlet.*` | 없는 리소스만 생성하는 범위와 제한 |
| Content 변경 19개 | Player/Enemy/Controller BP, 폰트·아이콘, Target Marker 교체/경로 이동 |
| Resources / Docs | 원본·라이선스·선택 기록, 과거 설계와 최종 상태의 일치 |

Content의 상세 Blueprint 그래프를 바이너리 diff만으로 완전히 검증했다고 주장하지 않는다. 코드 경로·로컬 LFS 무결성·에디터 자동 렌더/테스트·사용자 PIE 확인을 구분해 근거로 사용했다.

## 7. 검증 결과와 재현

| 검증 | 결과/범위 |
| --- | --- |
| Editor Win64 Development | 성공. 최초 DLL 잠금 실패 후 사용자 Editor 종료 및 재빌드 성공 |
| HUD 테스트 | 12/12 성공 |
| Balance 테스트 | 2/2 성공 |
| RenderPreview | RHI 사용, 1080p / 1440p / 3440×1440 샘플 렌더 |
| 사용자 PIE | 확인 완료 보고. 개별 시나리오 수행 내역을 추가로 추정하지 않음 |
| Git LFS fsck | 로컬 검사 성공 |
| 브랜치 차이 공백 검사 | 마감 정리 후 `git diff --check main` 통과 |

마감 자동 검사 로그는 `Saved/Logs/HUDClosureValidation.log`다. 로컬 생성물이므로 저장소 배포 증거로 자동 포함되지 않는다.

```powershell
& 'C:/Program Files/Epic Games/UE_5.4/Engine/Build/BatchFiles/Build.bat' PortfolioEditor Win64 Development '-Project=C:/UE5_Portfolio/Portfolio_UE5.4_verGit/Portfolio/Portfolio.uproject' -WaitMutex -NoHotReloadFromIDE

& 'C:/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'C:/UE5_Portfolio/Portfolio_UE5.4_verGit/Portfolio/Portfolio.uproject' -unattended -nop4 -nosplash -RenderOffscreen '-ExecCmds=Automation RunTests Portfolio.UI.CombatHUD.+Portfolio.Balance.' '-TestExit=Automation Test Queue Empty' '-log=HUDClosureValidation.log'
```

테스트 목록:

- HUD: ActionAvailability, ActionPresentationLifecycle, BalanceObservation, ExecutionAvailability, ExecutionReferenceBinding, GaugeLayout, HealthObservation, PresenterLifecycle, RenderPreview, ResourceView, WidgetConstruction, WidgetLifecycle.
- Balance: LifecycleBoundaries, RuntimeResetObservation.

중간 테스트 문제도 수정했다. ActionAvailability의 참조 초기화 누락으로 `InvalidComponent`가 발생했고, Presentation 테스트는 미등록 컴포넌트 Tick 및 WorldContext 누락으로 중단됐다. fixture를 보완했으며 런타임 판정을 완화해 통과시킨 것이 아니다. 패링 승인/거절, 시간 만료, 중복/새 serial 표시도 명시적으로 검사한다.

검증하지 않은 범위: 패키징/cook, 네트워크·다중 로컬 플레이어, 전 플랫폼 DPI, 모든 Montage 조합·레벨 전환·장시간 플레이. 기존 로그 경고가 전혀 없다는 주장이나 측정 없는 성능 개선 주장도 하지 않는다.

## 8. 브랜치 밖으로 남기는 사항

- 가드 게이지, 가드브레이크 판정·쿨타임, 퍼펙트 닷지 성공 신호.
- 돌진·카운터·BE 스킬 및 자원 충전/소모 시스템.
- 아이템 수량·사용과 보조 작은 원의 기능.
- HP 잔상/감소 애니메이션, WBP Designer 기반 재작성, 전용 DataAsset 재설계.
- Player HP 7700 초과 성장 정책 및 Balance 64 초과 시 더 풍부한 표현.

위 사항은 숨겨진 완료 기능이 아니라 의도적으로 남긴 범위다. 이번 브랜치의 마감 전제에 새 기능 구현을 추가하지 않는다.

## 9. 주요 커밋

| 커밋 | 작업 |
| --- | --- |
| `4108e444`, `83bbb5e8` | HUD 시각 설계·승인 자료 |
| `46c9367c` | Health/Balance 값 관측 |
| `757e5e21` | HUD 기본 구현·검증 도구 |
| `12aa0d24` | 고정 자원 단위·픽셀 정렬 |
| `d9a8fdb8` | 디버그 패널 전경 렌더 |
| `4f396580` | 액션 아이콘 통합·리소스 정리 |
| `36757b4b` | 가용성 조회 공통화·처형/Balance 생명주기 보완 |
| `5c679894` | Balance 경계·Reset 테스트 |
| `6db350a1` | HUD 상태 연동·구조·픽셀 렌더 정리 |
| `6d03c4da` | HUD 액션·Widget 수명 테스트 |
| `89a11069` | Widget 설정·배치 순서 가독성 정리 |
| `65157687` | HUD 테스트 fixture 보완 |

## 관련 문서

- [현재 구현 및 검증](33_Gameplay_Combat_HUD_Implementation_and_Validation.md)
- [시각 명세와 과거 비교안](32_Gameplay_Combat_HUD_Visual_Spec.md)
- [최초 설계 계획](31_Gameplay_Combat_HUD_Design_and_Implementation_Plan%20%28KR%29.md)
- [액션 상태 연결](../../../Resources/CombatHUD/ActionStateIntegration.md)
- [리소스 원본·라이선스](../../../Resources/CombatHUD/README.md)
