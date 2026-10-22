# 공격 시작 방향 보정

2026-09-21 최신화. 코드 `b0291c0e`, 테스트 `361aeeb2`, 사용자 Stellar BP `4a1487a3` 기준이다. 진입 문서는 [통합 검토 가이드](37_Gameplay_Knockback_and_Facing_Review_Guide.md)다.

## 범위

- `ComboAttack`의 첫 공격과 실제 소비된 콤보 후속타에 동일한 시작 방향 보정을 적용한다.
- 공격별 선택 기능이며 기존 데이터의 기본값은 비활성이다.
- 지상에서 캐릭터와 현재 CombatTarget 사이의 수평 방향으로 Yaw만 보정한다.
- 카메라/ControlRotation, 기존 회전 정책, 애니메이션 Root Motion 설정은 변경하지 않는다.
- 공격 중 지속 추적, 회전 잠금, 이동 거리 보정, Motion Warping은 포함하지 않는다.

## 실제 설정 조사

2026-09-21 구현 중 수행했던 읽기 전용 애셋 조사 결과다. 문서 최신화에서 모든 애셋의 속성을 재추출한 것은 아니다.

| 대상 | 공격 애니메이션 Root Motion | 애니메이션 Root Motion 중 일반 PhysicsRotation 허용 |
| --- | --- | --- |
| BP_CPlayer_Stellar 콤보 0~3 | 활성 | 비활성 |
| BP_CPlayer 기본 공격 0~2 | 비활성 | 비활성 |
| BP_CEnemy 기본 공격 0~2 | 비활성 | 비활성 |

Stellar AnimBP는 `RootMotionFromMontagesOnly`이고 플레이어 SpringArm은 Pawn ControlRotation을 사용한다. UE 5.4 CharacterMovement는 애니메이션 Root Motion 적용 중 위 회전 허용 플래그가 꺼져 있으면 일반 PhysicsRotation을 건너뛴다. 따라서 카메라가 타깃을 따라가더라도 몸이 공격 시작 방향을 유지하는 현상과 부합한다. 애니메이션 자체의 회전은 여전히 적용될 수 있다.

기본 플레이어/Enemy의 비 Root Motion 공격에서는 시작 보정 뒤 기존 회전 정책이 계속 동작한다. 이번 기능은 공격 내내 방향을 보장하는 기능이 아니다.

## 파일별 변경

| 파일 | 변경 및 이유 |
| --- | --- |
| `Source/Portfolio/Type/CActionDataTypes.h` | 공격별 활성화, 최대 거리, 최대 보정 각도 설정 추가. 기존 공격의 동작을 유지하도록 활성화 기본값은 false. |
| `Source/Portfolio/Action/CAction.cpp` | 몽타주 재생과 종료 델리게이트 연결 성공 후 공통 시작 훅 호출. 실패/즉시 완료에는 보정을 예약하지 않음. |
| `Source/Portfolio/Action/CAction_ComboAttack.cpp` | 후속타 재생 성공 후 같은 훅 호출. 예약 단계와 실제 공격 시작 단계를 구분. |
| `Source/Portfolio/Component/CActionComponent.h/.cpp` | 외부 요청 Serial과 독립적인 내부 실행 세대 및 타깃을 보관하고 다음 Action Tick에서 한 번만 소비. CharacterMovement보다 먼저 Tick하도록 의존성 지정. 완료/교체/초기화/EndPlay에서 대기 정보 정리. |
| `Source/Portfolio/Component/CMovementComponent.h/.cpp` | `TryFaceTarget` 추가. 지상·유한값·거리·각도·동일 위치 검증 후 캐릭터 Yaw만 변경. 기존 넉백 변경은 보존. |
| `Source/PortfolioEditor/Private/Tests/CActionFacingProbe.h` | 재생 실패/즉시 완료 및 수동 참조 주입을 위한 일시적 테스트 객체. 게임 애셋으로 저장하지 않음. |
| `Source/PortfolioEditor/Private/Tests/CActionFacingTests.cpp` | 시작/예약/후속타/실패/취소/오래된 실행 세대/타깃 해제/초기화/제외 조건과 실제 Root Motion 이동 검증. Serial 0인 실제 플레이어 입력 경로 및 시작 중 재진입 회귀 테스트 포함. |

## 처리 순서

1. 첫 공격 또는 후속타의 실제 재생 시작 성공.
2. 재생 전에 보관한 내부 실행 세대가 현재 실행과 일치하는지 확인하고 타깃을 약한 참조로 기록.
3. 다음 Action Tick에서 대기 정보를 먼저 제거하고 현재 실행·Executor → 자신의 생존·Action 상태·Movement 참조 → 타깃 유효성·선택 일치·생존 순으로 재검증.
4. Movement에서 현재 위치 기준 수평 거리와 각도를 검사하고 한 번 회전.
5. 기존 CharacterMovement 및 Root Motion 처리 진행.

콤보 Notify 호출 도중 즉시 회전하면 현재 프레임에 추출된 이전 공격의 Root Motion까지 새 방향으로 변환될 수 있어, 회전을 다음 이동 계산 경계로 넘긴다. 따라서 호출 즉시 보정하는 구조가 아니며 최대 한 Action Tick의 지연이 있을 수 있다. 시작 시점과 같은 프레임에 발생하는 충돌 Notify까지 보정된 방향을 보장하는 기능도 아니다.

이전 실행의 시작 훅은 새 대기 정보를 지우지 않는다. 종료/교체 및 소비된 후속타가 실패한 경우에는 남아 있던 보정을 폐기한다. 타깃이 해제되거나 다른 대상으로 바뀌면 대기 중 보정은 적용하지 않는다.

## 설정 방법

캐릭터 BP의 ActionComponent → Action Datas에서 적용할 `ComboAttack` 항목마다 다음 값을 지정한다.

- `Face Combat Target On Start`: true
- `Start Facing Max Distance`: 기본 450cm, 무기/공격의 유효 거리 기준으로 조절
- `Start Facing Max Angle`: 기본 90도, 현재 몸 방향과 타깃 방향 차이의 허용 상한

최대 각도는 회전량을 잘라내는 값이 아니다. 상한을 넘으면 보정 전체를 생략한다. 거리는 수평 거리이며 높이·시야·장애물 검사나 이동 보정은 하지 않는다.

사용자가 저장한 `BP_CPlayer_Stellar.uasset`만 `4a1487a3`에 커밋했다. 에이전트가 BP를 재저장하거나 옵션을 일괄 활성화한 것은 아니다. 사용자가 제시한 1000cm / 180도 설정은 대화에서 확인한 설정이며, 모든 저장 항목에 동일하게 적용됐다고 추정하지 않는다. SciFiMap은 커밋 대상에서 제외했고, 최종 커밋 준비 시 Git 수정 목록에도 없었다.

## 코드 검토 후 정리

- `CanCommitChain`은 헤더/소스의 `Query`로, `ValidateRequiredComponentReferences`는 private 검증 섹션으로 이동했다. 이 두 함수의 판정 내용은 바꾸지 않았다.
- `HandleApplyActionStarted`는 시작 통지의 세대·Executor를 먼저 확인한 뒤 이전 예약을 비운다. 오래된 통지가 새 예약을 제거하지 않도록 순서를 유지한다.
- `ApplyPendingStartFacing`은 예약 값 복사 → 예약 초기화 → 실행/자신/타깃 검사 → `TryFaceTarget` 순서다. 별도 `IsActiveActionGeneration` API는 최종 코드에 추가하지 않았다. Reaction의 세대 조회 API와 혼동하지 않는다.
- Movement의 CharacterMovement 참조는 `FCharacterComponentReferences`에서 직접 주입한다. Action의 Tick 의존성 등록/해제는 여전히 Owner의 `GetCharacterMovement()`를 사용한다. 전체 프로젝트를 직접 주입으로 일괄 전환한 것은 아니다.
- 세대는 첫 컨텍스트 설정과 실제 후속타 소비 시 갱신된다. 시작 실패 정리는 시작 당시 세대와 현재 세대가 일치할 때만 진행한다.

## Serial 0 보완

초기 구현은 외부 `ActionRequestSerial == 0`을 무효 실행으로 판단했다. 하지만 `ACPlayer::HandleCombatAction()`은 Serial을 지정하지 않아 정상 플레이어 공격도 0이다. 따라서 최초 테스트의 양수 Serial에서는 통과했지만 실제 플레이어 입력에서는 보정이 모두 차단됐다.

- `ActiveActionGeneration` / `NextActionGeneration`: 외부 요청과 독립적인 내부 실행 식별자. 첫 공격 컨텍스트 설정과 실제 후속타 소비 때 갱신하며 예약만으로는 바뀌지 않는다.
- `PendingStartFacingGeneration`: 보정이 속한 실행 단계. 외부 Serial 0도 정상 처리한다.
- 완료/취소/초기화 시 현재 세대는 무효화하되 발급 카운터는 유지한다.
- 재생 호출 전에 세대를 보관하고 돌아온 뒤 일치 여부를 확인한다. 이전 실행의 실패 처리나 늦은 시작 훅이 같은 Executor·Serial의 새 실행을 지우지 않는다.
- 실제 플레이어 입력 API로 콤보 0~3을 요청하는 테스트를 추가했다. 런타임 데이터/Executor는 테스트용으로 준비하고 실제 입력·오케스트레이터·첫 타/콤보 소비·방향 보정 경로를 통과시킨다. 사용자 PIE 자체를 자동화한 것은 아니다.

## 검증 결과

- PortfolioEditor Win64 Development 빌드 성공.
- 최신 `Automation RunTests Portfolio.`: 총 28개 Success, 종료 코드 0. HUD RenderPreview는 NullRHI 때문에 본 검사를 생략하며 렌더 재검증으로 간주하지 않는다.
- `Portfolio.Combat.ActionFacing`: Gates, Lifecycle, RootMotion, PlayerInputZeroSerial 모두 성공.
- 실제 Stellar 첫 공격 몽타주와 AnimBP를 사용한 일시적 테스트 월드에서 CharacterMovement를 진행: 수평 이동 약 66.19cm, 몸 Yaw 45도, ControlRotation -30도 유지.
- 실제 몽타주는 일시적으로 복제하고 Notify를 제거해 이동과 회전만 격리했다. 원본 애셋은 수정하지 않았다.
- 최신 재검증 로그: `Saved/Logs/KnockbackCommitValidation.log`. 기존 `ActionFacingGenerationValidation.log`, `ActionFacingValidation.log`, `AttackFacingAudit.log`는 과거 로컬 기록이며 Git에 포함되지 않는다.
- Regacy 메쉬의 머티리얼 참조 누락 및 콘솔 변수 반복 조회 경고가 존재한다. 테스트 성공과 경고 없는 실행을 구분한다.
- 이전 실패는 미초기화 테스트 월드의 TickPose 조건 및 테스트 참조와 캐릭터 초기화 충돌을 수정한 뒤 재검증했다. 성공 결과를 얻기 위해 기대 이동/방향 검증을 제거하지 않았다.

## 사용자 PIE 확인 항목

사용자는 보완 후 콤보 시작부터 끝까지 적 방향을 보고 공격해 원하는 결과를 얻었다고 확인했다. 이는 해당 플레이 상황의 방향 보정 확인이며 아래 모든 경계 조건을 개별 수행했다는 뜻은 아니다.

1. Stellar의 콤보 0~3에 보정을 켜고 지상에서 가까운 적을 타기팅한다. 첫 공격 시 몸만 적을 향하고 카메라가 강제로 점프하지 않아야 한다.
2. 공격 사이 적이 약 45도 옆으로 이동하도록 배치한 뒤 콤보를 이어간다. 다음 타격 시작 때 새 방향을 향하고, 진행 중에는 지속 추적하지 않아야 한다.
3. 콤보를 예약만 했을 때는 회전하지 않아야 한다. 후속타가 실제 시작될 때 보정돼야 한다.
4. 타깃 해제/교체, 공격 취소·피격 전환, 사망 상황에서 종료된 공격의 보정이 뒤늦게 나오지 않아야 한다.
5. 최대 거리·각도 밖, 공중 상태, 옵션 비활성 공격은 보정되지 않아야 한다.
6. 실제 충돌 Notify/HitStop/블렌딩을 포함한 콤보 연결의 타격감은 PIE에서 확인한다. 해당 조합과 Enemy AI Focus의 시각적 체감은 자동 테스트로 완료했다고 간주하지 않는다.

소스·테스트·플레이어 BP는 위 커밋으로 분리했다. 푸시·PR 생성·머지는 수행하지 않았다.

## 2026-10-08 Enemy 공격 회전 정책 보완

위 최초 구현 기록과 달리, Enemy는 ComboAttack 실행 중 Gameplay Focus를 해제하고 FixedFacing을 사용한다. 공통 시작 보정은 각 타 시작 시 한 번만 적용하고, 이후 플레이어 이동을 지속 추적하지 않는다. 타깃/시야 상실도 현재 타의 고정 방향을 해제하지 않는다. 정식 Action 종료·중단 시 감각 상태에 따라 추적 또는 이동 방향 회전으로 복귀한다. 사망·Balance 억제와 Reaction 회전 소유권은 기존 우선순위를 유지한다.

ACAIController가 소유한 캐릭터의 시작 보정은 실행 직전 현재 시야도 확인한다. 감각 기억에 타깃 객체가 남아 있어도 숨은 대상의 실제 위치로 보정하지 않는다. 플레이어 시작 보정 규칙은 그대로 유지한다.

BT_Default의 State_Router 마지막 자식에 무조건 실행 가능한 Fallback_ReevaluateIntent(Wait 0.2초, 편차 0)를 둔다. 공격 중 시야 상실로 Engage 진입 조건이 무효화되어도 상위 서비스가 다음 상태를 평가할 실행 시간을 확보한다. 다음 상태 선택은 기존 UpdateAIIntentState가 담당한다.

공격 정상 종료는 Complete Notify가 담당한다. CAction::OnMontageEnd의 자연 종료/예상하지 못한 중단 처리는 관찰 로그이며 자동 Complete 폴백으로 바꾸지 않는다. 재현된 Engage 정지는 Complete 누락이 아니라 정상 종료 후 상태 분기 공백이었다.


## 2026-10-22 G3 AI 핵심 분리 검증

- 기준은 D1과 P2-A가 머지된 main `c413528c`이다. 참여 인원, 시야 거리, BT_Engage 공격 빈도, 순찰 도착 반경, EQS 범위/선택 방식은 main 설정을 유지한다.
- BP_CEnemy는 ComboAttack 0/1/2의 `bFaceCombatTargetOnStart`만 활성화한다. 세 공격의 기존 몽타주 참조와 CharacterMovement의 MaxStepHeight는 유지한다.
- 실제 PIE에서 감지 갱신 사이에 플레이어를 시야 밖으로 이동시키자 마지막 관측 위치가 숨은 액터 좌표로 바뀌는 문제가 재현되었다. 감지 콜백은 `FAIStimulus::StimulusLocation`, 주기적 갱신은 Perception에 저장된 현재 Sight 샘플을 사용하도록 보완했다. 같은 조건의 PIE와 지연된 감지 샘플 회귀 테스트로 확인한다.
- G3 자동화 검증에는 관측 위치 유지·재발견·동일 기억 재수색 방지와 AI 공격 방향 정책·시야 게이트가 포함된다. 에디터 테스트가 네이티브 AIController를 생성하므로 PortfolioEditor에 AIModule의 명시적 private 의존성을 추가한다.
- PIE 검증은 실제 SciFiMap과 BP/AnimBP/BT를 실행하고 임시 액터 배치와 시점별 행동 요청으로 조건을 재현한다. 수동 보스전 전체 플레이를 검증했다는 의미는 아니다. 임시 검증 코드는 최종 변경에 포함하지 않는다.
- 에셋 회귀 검증은 BT의 Dead/HitReact 직렬화 값과 native enum의 일치, 무조건 fallback 연결, ComboAttack 0/1/2의 방향 옵션·유효한 몽타주 유지를 확인한다.

## G3 리뷰 후 경계 조건 보완

리액션 실행 중 현재 시야를 잃었을 때는 숨은 타깃의 Gameplay Focus만 즉시 해제한다. 회전 모드는 리액션 동안 유지하고 종료·중단 시 현재 감각 상태로 다시 동기화한다. 종료 전에 재발견해도 리액션이 끝날 때까지 추적 회전을 복구하지 않는다. 사망·Balance 억제·타깃 제거의 기존 안전 정리는 이 유예보다 우선한다. 관찰 스냅샷은 이 상태를 `HoldForReaction`으로, Focus 지시는 해제, 회전 지시는 유지로 표시한다.

기존 G3 PIE에서 사용한 `bShouldInvestigate || bIsInvestigating` 판정은 수색 요청만으로 통과할 수 있었다. 강화한 실제 BT 검증에서는 Equip 중 이동이 잠긴 상태로 수색에 진입하면 `BT_Investigate` fallback이 요청을 지워 버리는 조건을 재현했다. `UpdateAIIntentState`는 아직 시작하지 않은 수색을 이동 가능 시점까지 Idle에서 기다리게 한다. 대기 중 요청과 관측 앵커를 유지하며, 이동이 풀리면 기존 수색 트리로 진입한다. 이미 실행 중인 수색과 사망·리액션·Balance 우선순위는 유지한다. BT 에셋과 이동·감각·EQS 튜닝 수치는 바꾸지 않는다.

수색 검증은 요청 여부와 실제 실행을 구분한다. 실제 `bIsInvestigating` 전환, 활성 `BT_Investigate`/MoveTo 노드, 관측 앵커를 향한 경로 목적지, 캐릭터 변위 및 재발견 후 중단을 함께 확인한다. 리액션 PIE는 실제 피해 경로와 기존 피격 몽타주를 사용하되 감각 이벤트를 관찰할 시간을 확보하기 위해 해당 런타임 몽타주의 재생 속도만 일시 조정한다. 검증용 배치·계측 코드는 에셋으로 저장하거나 최종 소스에 포함하지 않는다.

## G3 리뷰 보완 최종 검증 결과

- C1~C4 누적 경계에서 기본 Unity 활성·Adaptive Unity 비활성의 PortfolioEditor Win64 Development 빌드, 전체 Portfolio 자동화, CDodgeAssets Verify, CExecutionMontageAudit를 각각 통과했다.
- 자동화 통과 수: C1 85개, C2 88개, C3 88개, C4 89개. 최종 89개 중 성공 87개·경고 동반 성공 2개, 실패·미실행 0개다. 기존 머티리얼 참조 누락과 cleanup 관련 경고를 성공과 구분한다.
- 실제 PIE: G3 4조건 31개, 리액션 경계 3조건 38개, D1 17조건 518개, P2-A 4조건 60개 판정 통과.
- native 수색은 StartInvestigate 상태 전환, BT_Investigate의 MoveTo, 관측 앵커와 수평 오차 0cm인 경로 목적지, 100cm 초과 변위 및 재발견 후 하위 트리 이탈까지 확인했다. EQS 전체 경유점 완주나 수색 시간 소진 전체를 검증했다는 뜻은 아니다.
- 공격 중 타깃 이동에 따른 최대 Yaw 변화 0도, 계측한 D1 사망 후 추가 이동·타격 0, P2-A 앵커 표식 Z+10cm를 확인했다.
- 임시 PIE 하네스와 추가 NavigationSystem 의존성은 제거했다. 결과 수치 기록 후 코드·에셋은 C4 검증본과 동일하며, G3 에셋은 리뷰 전 해시를 유지한다. 원본 104개 변경과 두 보존본은 그대로 유지한다.
- 최종 미커밋 범위는 C1 12파일, C2 8파일, C3 1파일, C4 2파일이다. 커밋·PR 생성은 수행하지 않았다. Shipping/Cook, 네트워크, 성능 및 사용자 수동 보스전 완주는 별도 검증 범위다.
