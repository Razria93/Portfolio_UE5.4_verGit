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
