# UE5 Portfolio Pull Request

## 제목

**P64: 일반 Hit 지상 넉백·공격 시작 페이싱 구현과 전투 이동 디버그 시각화**

## 날짜

**2026.09.21**

## 상태

- [x] 공격별 넉백 설정과 Damage → Reaction 실행 정보 전달
- [x] 일반 Hit 시작에 소유되는 지상 수평 이동과 세대별 정리
- [x] 콤보 첫 타·후속타 시작 방향 보정 및 Serial 0 경로 보완
- [x] 기존 Debug Overlay의 넉백·AttackFacing 패널·이벤트·설정 연결
- [x] 발밑 월드 드로우·Foreground·고정 행 표시 보완
- [x] 자동 테스트·구현 문서·측정 가이드 작성
- [x] 최신 전체 자동 회귀 36개 및 로컬 LFS 무결성 검증
- [x] 사용자 콤보 방향 보정 및 디버그 표현 확인
- [x] 관찰용 설정에서 주요 수동 경계·환경 시나리오 사용자 확인
- [ ] PR 리뷰 및 병합

## 브랜치

- Base: `main`
- Branch: `feat/combat-knockback`
- Base / Merge Base: `d4012433`
- Implementation HEAD: `10d8af56`
- 조사 범위: `d4012433..10d8af56`, 9개 커밋.

선행 PR #123의 전투 HUD·액션 상태 조회·처형 생명주기는 기준선이다. 이번 PR에서 새로 구현한 기능으로 중복 집계하지 않는다. 본 문서는 로컬 PR 초안이며 원격 PR 생성·푸시는 별도 작업이다.

## 요약

일반 Hit 리액션이 실제 시작된 경우에만 적용되는 지상 수평 넉백을 추가했다. 공격별 속도·시간을 DamageSpec에 두고, 대상 판정에서 확정한 방향을 Reaction 실행까지 전달한다. 실제 이동은 CharacterMovement의 Root Motion Source로 처리하며 Hit 실행 세대에 소유권을 연결한다.

넉백으로 타깃 위치가 바뀐 뒤에도 다음 콤보가 이전 방향으로 진행되던 문제에는 공격별 시작 페이싱을 추가했다. 첫 공격과 실제 소비된 후속타에 동일하게 적용하며, 외부 요청 Serial과 내부 실행 세대를 분리했다. 공격 내내 타깃을 추적하거나 카메라를 강제로 돌리는 기능은 아니다.

두 기능의 적용·제외·종료를 기존 디버그 도구에서 확인할 수 있도록 관측 기록, Player/Focus Enemy 패널, 이벤트 필터와 발밑 공간 표시를 연결했다. 현재 상태와 마지막 시도·이동 기록을 구분하고, 넉백 패널은 상태 전환에도 10행을 유지한다.

## 핵심 개념

- Knockback Spec / Context: 공격의 설정값과 이번 피격에서 확정한 수평 방향을 분리한다.
- 실행 세대: 이전 실행의 늦은 시작·정리가 새 실행을 변경하지 않도록 소유권을 구분한다.
- 일회 시작 페이싱: 실제 공격 시작 후 다음 Action Tick에서 한 번 보정한다. 예약 및 지속 추적과 구분한다.
- Current / Last attempt / Motion: 현재 이동 소스, 마지막 처리 시도, 마지막 실제 이동 기록은 서로 다른 정보다.
- Nominal / Actual XY: 설정상 거리와 관측한 수평 순변위다. 이론 거리는 보장 이동 거리가 아니다.

## 변경 배경

기존 Hit 표현에 공격별 이동 효과를 연결하되 판정이나 준비 조회만으로 캐릭터가 이동해서는 안 됐다. 실행 실패·동기 완료·중단·연속 피격에서도 이동 소스가 남거나 새 실행을 잘못 정리하지 않는 수명 경계가 필요했다.

Stellar의 Root Motion 공격은 카메라가 적을 따라가도 캐릭터가 이전 공격 방향을 유지할 수 있었다. 애니메이션의 전역 회전 설정을 바꾸는 대신, 각 공격의 실제 시작 시점에 몸의 방향만 보정하는 방식을 선택했다.

텍스트만으로는 적용 여부와 방향·이동량을 구분하기 어려웠다. 초기 드로우는 몸 중심에 가려졌고 넉백 패널은 활성·종료에 따라 행 수가 바뀌었다. 발밑 표시와 고정 행으로 보완했다.

## 주요 변경

### 1. 공격별 설정과 대상 판정

`FDamageSpec`에 `FCombatKnockbackSpec`을 추가했다. Speed / Duration의 기본값은 0이므로 기존 공격은 명시적으로 설정하기 전까지 넉백을 사용하지 않는다.

`ResolveDamageKnockback`은 승인된 일반 Hit, Normal 외부 입력 정책, 유효한 설정·Actor와 지상 조건을 확인하고 공격자에서 피격자 방향을 수평으로 확정한다. 결과는 CombatSignalTarget Context → ReactionCandidate → ReactionExecutionContext로 전달한다. 이 단계에서는 이동을 시작하지 않는다.

Guard, Parry, CollapseHit, 처형, 사망 및 공중 피격은 적용 범위에서 제외한다.

### 2. Hit 실행 수명과 이동 소유권

`UCReactionComponent`는 컨텍스트 설정·상태 진입·Executor 시작 이후에도 실행 세대를 확인한다. 시작 실패나 시작 중 즉시 완료에는 넉백을 적용하지 않는다. 일반 Hit가 활성 상태로 시작됐고 대상 Hit 몽타주에 Root Motion이 없을 때 Movement에 전달한다.

`UCMovementComponent`는 수평 ConstantForce Root Motion Source를 생성한다. 우선순위는 명명된 코드 상수, Override / IgnoreZ 등의 설정은 고정 구현 정책이며 공격별 조절값은 속도·시간이다. Player/Enemy의 CharacterMovement 참조는 캐릭터 참조 구성에서 주입한다.

새 Hit는 기존 소스를 정리하고 교체한다. 종료·중단·Reset·참조 재주입·EndPlay 및 적용 조건 상실에서 자기 소스만 정리하며 오래된 소유 세대의 Stop은 무시한다. 다른 이동 소스나 공중 속도까지 일괄 제거하지 않는다. AI 이동 중단 호출 뒤에는 재진입 여부를 확인한다.

### 3. 공격 시작 페이싱과 콤보 연결

`FActionData`에 활성화 옵션, 최대 수평 거리, 최대 각도를 추가했다. 기본 활성화 값은 false다. 최대 각도는 회전량을 잘라내는 값이 아니라 적용 허용 조건이다.

`CAction::Start`와 `CAction_ComboAttack::ConsumeChain`에서 실제 재생 성공 뒤 공통 시작 훅을 호출한다. ActionComponent는 실행 세대와 타깃을 보관하고 다음 Action Tick에서 현재 실행 → 자신의 상태 → 타깃 조건을 확인한다. CharacterMovement보다 먼저 실행되도록 Tick 의존성을 설정한다.

`TryFaceTarget`은 지상·거리·각도·동일 위치 조건을 검사한 뒤 캐릭터 Yaw만 보정한다. 타깃 변경·해제, 액션 종료·교체·초기화에서 대기를 폐기한다. ControlRotation과 SpringArm 정책, 애니메이션 Root Motion 설정은 변경하지 않는다.

### 4. 실행 식별과 코드 배치 정리

정상 플레이어 입력도 외부 ActionRequestSerial이 0일 수 있으므로 이를 무효 실행으로 판단하지 않는다. 별도 내부 `ActiveActionGeneration`과 발급 카운터를 도입하여 첫 타와 실제 후속타 소비를 식별한다. 재생 전에 세대를 보관하고 돌아온 뒤 비교해 콜백 재진입 시 새 실행을 보호한다.

`CanCommitChain`과 참조 검증 함수 등의 헤더·소스 섹션을 정리했다. 이동만 한 API의 판정 변경과 신규 세대·페이싱 로직을 구분한다. Reaction의 세대 조회·넉백 정리 헬퍼는 반복 경계를 명시하는 용도다.

### 5. 기존 디버그 도구 확장

`FCombatKnockbackDebug`, `FActionFacingDebug`가 실제 실행 결과를 기록하고 문자열·출력·드로우를 담당한다. 진단 때문에 게임플레이 판정을 재호출하거나 진단 결과를 실행 조건에 사용하지 않는다.

SnapshotStore는 월드별 Actor 약한 참조와 Actor 수 제한을 사용하며 제거·월드 정리에 연결한다. 마지막 시도와 실제 이동 기록을 분리하고 오래된 세대가 최신 결과를 덮어쓰지 않도록 한다. 기본 넉백 0/0, 빈 정리, 대기 없는 Tick은 반복 이벤트를 만들지 않는다.

기존 ViewDataBuilder / TextFormatter / DebugOverlayHUD / SettingsRegistry에 Player와 Focus Enemy를 같은 구조로 연결했다. `Knockback`, `AttackFacing` 이벤트 카테고리를 추가하며 기존 `Facing`은 Enemy AI Focus 정책 진단으로 유지한다. Editor 플러그인은 기존 설정 레지스트리 연동을 활용하고 별도 HUD를 만들지 않았다. Shipping에서는 진단 수집·저장·출력을 제외한다.

### 6. 공간 표시와 가독성

- 넉백: 흰 시작점, 노란 이론 이동 점선·도착 원, 청록 실제 변위·현재/종료 원을 발밑에 표시한다.
- 페이싱: 회색 이전 방향, 청록 실제 Yaw 변화 원호, 초록 적용 결과 화살표, 노란 타깃 방향 표식을 사용한다. Applied 이외 결과 방향은 주황색이다.
- 페이싱의 노란 원과 결과 화살표는 고정 110cm 표시 길이이며 실제 타깃 위치·공격 사거리가 아니다. 1도 이하 변화에서는 이전 선·원호를 생략한다.
- 과거 판정 위치·방향을 보관하며 현재 타깃을 따라 과거 도형을 바꾸지 않는다. `DrawDuration`은 표현 유지 시간이지 지속 보정 시간이 아니다.
- `DrawForeground`는 가림을 줄이는 선택 옵션이다. 지형·벽 너머에서도 보일 수 있으며 충돌·시야 판정과 무관하다.
- 넉백 패널은 10행을 고정하고 없는 값은 N/A / NotCaptured로 표시한다. 후속 제외 시도가 생겨도 이전 Motion 종료 시간은 유지한다.

### 7. 에셋과 문서

브랜치 전체 Content 차이는 `BP_CPlayer_Stellar.uasset`, `SciFiMap.umap` 두 파일이다. 사용자 저장본을 각각 별도 커밋으로 포함했다. Enemy BP와 Default Hit 몽타주는 최종 브랜치 차이에 없으므로 변경 에셋으로 기재하지 않는다.

바이너리 내부의 모든 옵션·맵 배치 차이는 이번 PR 문서 작성에서 재추출하지 않았다. 모든 공격 옵션이 일괄 활성화되었다거나 특정 충돌·조명 설정을 변경했다고 추정하지 않는다.

사용자가 콤보 0~3 중 빠져 있던 2·3타 넉백 설정을 보완했다고 보고했으며, 해당 Stellar BP 저장본을 `10d8af56`으로 추가 커밋했다. 구체적인 속도·시간과 임시 실험 설정 복구 여부를 바이너리에서 재추출한 것은 아니다. 아래 전체 자동 테스트는 이 추가 에셋 수정 전 `385738b6` 기준이며, 새 설정의 개별 타격 검증 완료로 확대하지 않는다.

넉백·페이싱 구현, 통합 코드 검토, 디버그 구현·측정 가이드를 추가하고 기존 아키텍처·문서 진입 링크를 연결했다.

## 주요 처리 흐름

```text
공격 DamageSpec
→ 대상 피해·Hit 결과 확정 → 넉백 설정·방향 해석
→ ReactionCandidate → ExecutionContext
→ Hit 실제 시작 및 실행 세대 재확인
→ 소유 Root Motion Source 생성
→ 완료·중단·교체·조건 상실 시 해당 소스 정리
```

```text
첫 타 / 실제 후속타 재생 성공
→ 내부 실행 세대·타깃으로 시작 보정 대기
→ 다음 Action Tick에서 대기 소비·조건 재확인
→ 캐릭터 Yaw 일회 보정 → CharacterMovement / Root Motion 진행
```

```text
실제 실행 경계의 관측 호출
→ 진단 헬퍼 → Actor별 최근 기록 / 선택적 이벤트 수집
→ Player·Focus Enemy 패널 및 기록된 공간 드로우
```

## 트러블슈팅과 설계 판단

- 외부 요청 Serial 0은 실패 근거가 아니었다. 실제 플레이어 입력 경로 테스트와 내부 실행 세대로 보완했다.
- 콤보 Notify 안에서 즉시 회전하면 이전 공격에서 추출한 Root Motion까지 새 방향으로 처리될 수 있어 다음 이동 계산 경계로 넘겼다.
- Actor 중심 드로우는 몸에 가려져 판독이 어려웠다. 발밑 기준으로 옮기고 겹치는 방향은 화살표·표식·원호로 구분했다.
- 활성일 때 두 줄, 종료일 때 한 줄을 추가하던 패널을 고정 행으로 바꿨다. 갱신을 지연하거나 값을 보간하여 짧은 실행을 숨기는 방법은 사용하지 않았다.
- 실제 변위가 작다고 충돌 원인을 단정하지 않는다. 소스 진행 시간과 월드 기록 나이도 별도로 해석한다.

## 변경 파일 범위

- Gameplay: Action/ComboAttack, Action·Reaction·Movement·CombatSignalTarget·ReactionOrchestrator, Player/Enemy 참조 구성.
- Types: DamageSpec, Knockback Spec/Context, ReactionCandidate/ExecutionContext, Action 설정과 캐릭터 참조.
- Debug: 전용 관측 헬퍼·기록 타입·공통 드로우, SnapshotStore, 패널 Builder/Formatter/HUD, 설정·이벤트 필터.
- PortfolioEditor: 넉백·페이싱 테스트/Probe, 진단 생명주기·표시·드로우 테스트.
- Content: Stellar Player BP, SciFiMap.
- Docs: 구현·검토·측정 문서 및 진입 인덱스.

## 테스트 방법

1. Editor를 종료한 뒤 `PortfolioEditor Win64 Development`를 빌드한다.
2. `UnrealEditor-Cmd -RenderOffscreen`에서 `Automation RunTests Portfolio.`를 실행한다. 진단 집중 검사는 `Portfolio.DebugOverlay.CombatMotion`을 사용한다.
3. Shipping 컴파일 여부와 실제 Cook·패키징·게임 실행을 구분해 기록한다.
4. PIE에서 설정된 일반 Hit의 단발·연속 피격, 첫 타·후속타 보정, 타깃 변경·제외 조건을 확인한다.
5. HUDVisible과 각 진단 Enabled/DrawWorld를 켜고 Focus Enemy를 지정한다. 패널 고정 행·발밑 도형·Foreground와 수집/출력 독립 제어를 확인한다.
6. `git diff --check main...HEAD`와 로컬 LFS 검사를 별도로 수행한다. 실행하지 않은 검사를 성공으로 표기하지 않는다.

구체적인 설정·색상·도형 및 시나리오는 아래 측정 가이드에 정리했다.

## 검증 결과

| 항목 | 결과 | 근거·범위 |
| --- | --- | --- |
| Editor Development 빌드 | 성공 | 2026-09-21 최종 통합 검증에서 최신 코드 재빌드 |
| 최신 CombatMotion 자동 테스트 | 8/8 성공 | 아래 전체 36개에 포함. DrawGeometry·행 안정성 포함 |
| 전체 Portfolio 자동 테스트 | 36/36 성공, 실패 0, 종료 코드 0 | `-RenderOffscreen`, `Automation RunTests Portfolio.`. 넉백 6·페이싱 4·진단 8·HUD 12·Balance 2·Animation 2·Weapon 2 |
| Shipping 빌드 | 성공 | 2026-09-21 최신 `Portfolio Win64 Shipping` 재빌드. Cook·패키징·게임 실행은 미수행 |
| 실제 CharacterMovement / Root Motion | 기존 자동 테스트 기록 | 충돌 월드·프레임 간격·실제 몽타주 기반 격리 검증. 실제 맵 전체 검증과 구분 |
| 사용자 PIE | 콤보 보정·디버그 표현 및 주요 경계·환경 확인 | 넉백 2.0초, Hit 몽타주 실행 배율 0.1의 관찰용 설정. 아래 사용자 보고 범위에서 문제 미발견 |
| 브랜치 공백 검사 | 경고 5곳 | Movement 3곳, Reaction 2곳. 문서 작성에서 소스를 임의 수정하지 않음 |
| 로컬 Git LFS 무결성 | 성공 | `git lfs fsck`: OK. UE 전체 에셋 참조 감사나 원격 업로드 검증과는 다름 |
| 원격 PR | 미생성 | 푸시·원격 PR 생성은 별도 요청 대상 |

최종 통합 검증 기준은 `385738b6`, 검사일은 2026-09-21이다. 로그는 `Saved/Logs/CombatKnockbackPRFinalValidation.log`이며 Git 추적 제외인 로컬 산출물이다. HUD RenderPreview도 성공했으나 화면 없는 자동 검사는 실제 플레이 화면 시각 검수와 같지 않다. 기존 Regacy 메쉬의 M_Man_Body / M_Man_ChestLogo 참조 누락과 반복 CVar 조회 경고는 남아 있다. 성공 결과를 전체 에셋 무경고·무결점으로 해석하지 않는다.

### 사용자 수동 검증 기록

2026-09-21 사용자 보고: 실험 편의를 위해 넉백 지속 시간을 2.0초, Hit 몽타주 실행 배율을 0.1로 설정했다. 안내한 필수 경계 시나리오와 환경 시나리오(가이드 응답의 2·3번)를 모두 확인했고 문제가 발견되지 않았다. 언덕·벽, 두 캐릭터를 겹친 상태 및 번갈아 타격하는 시험을 명시적으로 수행했다.

보고된 범위는 단발 종료·연속 교체·사망·처형 전환·페이싱 취소/타깃 변경·거리/각도 경계와 벽·경사/계단·캐릭터 충돌·AI 추적·HitStop·공격 Root Motion에서 피격으로의 전환이다. 에이전트가 개별 로그·영상을 독립 분석한 결과는 아니며, 짧은 페이싱 대기 취소나 AI 콜백 재진입이 내부적으로 발생했는지까지 계측 확인한 것으로 확대하지 않는다.

이 결과는 관찰용 설정의 동작 안정성 확인이다. 기본 지속 시간·재생 배율에서의 최종 타격감, 양쪽 피격 및 모든 제외 조건의 추가 전수 검증을 의미하지 않는다. 임시 설정의 복구 여부와 최종 유지값은 별도로 확인한다.

## Scope Guard

이번 PR에서 하지 않은 것:

- 공중 발사, 벽꿍, 넘어짐, 넉백 저항·슈퍼아머, 별도 DataAsset 전환.
- 공격 중 연속 타깃 추적, 회전 잠금, 강제 카메라 정렬, Motion Warping·거리 보정.
- Guard·Parry·CollapseHit·처형·사망·공중 피격으로 넉백 범위 확대.
- 별도 디버그 HUD·범용 프레임워크, 전체 이동 궤적 저장·재생.
- 모든 공격 데이터·애니메이션 Root Motion 옵션의 전역 변경.
- 멀티플레이 예측·복제, 전체 플랫폼·패키징 검증, 측정 없는 성능 개선 주장.

## 리스크 / 리뷰 포인트

- 페이싱은 최대 한 Action Tick 뒤 적용된다. 같은 프레임의 모든 충돌 Notify가 보정된 방향을 사용한다고 보장하지 않는다.
- 넉백은 애니메이션 Root Motion과 중첩 적용하지 않는다. 실제 블렌딩·HitStop·AI 이동 재진입·경사면·계단·캐릭터 충돌은 추가 수동 확인 대상이다.
- Nominal과 Actual XY의 차이, 소스 시간의 설정 시간 초과는 원자료를 숨기지 않는다. 특정 원인으로 확정하려면 별도 측정이 필요하다.
- 발밑 드로우는 캡슐 기준 오프셋이지 지형 투영이 아니다. Foreground는 벽 너머까지 표시할 수 있다.
- Current / Last attempt / Motion은 서로 다른 시점·세대일 수 있다. 종료된 값을 현재 활성 값으로 표시하지 않는다.
- 기존 Regacy 머티리얼 참조·반복 CVar 조회 경고와 이번 기능 성공 여부를 구분한다.

## 후속 작업

1. 최종 통합 검증은 완료했다. 이후 코드·에셋 변경이 생기면 영향 범위의 검증을 다시 수행한다.
2. 주요 수동 경계·환경 시험은 사용자 확인을 받았다. 임시 관찰용 설정의 복구 또는 최종 유지값을 확인하고, 최종 속도에서의 체감 확인과 미계측 경계는 구분해 관리한다.
3. PR 문서를 확정하고 게시·리뷰·병합으로 현재 구현 범위를 마감한다. 새로운 이동·카메라 정책은 별도 범위로 결정한다.

## 관련 문서

- [일반 Hit 지상 넉백 구현](../07_Portfolio_Documents/Portfolio_Production/35_Gameplay_Combat_Knockback_Implementation.md)
- [공격 시작 방향 보정](../07_Portfolio_Documents/Portfolio_Production/36_Gameplay_Attack_Start_Facing.md)
- [통합 코드 검토 가이드](../07_Portfolio_Documents/Portfolio_Production/37_Gameplay_Knockback_and_Facing_Review_Guide.md)
- [전투 이동 진단 구현](../07_Portfolio_Documents/Portfolio_Production/38_Gameplay_Combat_Motion_Diagnostics.md)
- [넉백·페이싱 디버그 측정 가이드](../07_Portfolio_Documents/Debug_Overlay/02_Operation/Combat_Motion_Debug_Measurement_Guide_KR.md)

35~37 문서는 초기 구현 커밋 기준 기록을 포함한다. 최신 디버그 변경과 맵 포함 여부는 본 PR의 HEAD 및 38·측정 가이드와 구분해서 읽는다. GitHub PR 본문으로 게시할 때는 문서 상대 링크를 게시 커밋의 저장소 링크로 변환한다.

## 대표 커밋

```text
b0291c0e feat(combat): add grounded hit knockback and attack start facing
361aeeb2 test(combat): cover knockback and attack start facing
4a1487a3 feat(combat): update Stellar player blueprint settings
a7bb5336 docs(combat): document knockback and attack facing implementation
85885d79 feat(debug): add knockback and attack facing diagnostics
1feade20 test(debug): cover combat motion diagnostics and presentation
8ac3d2d0 docs(debug): document combat motion diagnostics and measurement
385738b6 chore(map): update SciFi map
10d8af56 fix(combat): configure knockback for Stellar combo hits 2 and 3
```

## 정리

이번 PR은 피격 이동과 공격 시작 방향 보정을 실제 실행 수명에 연결하고, 동일한 실행 경계의 결과를 기존 디버그 도구에서 확인할 수 있게 했다. 이동·판정·표시의 책임과 실행 세대를 구분하며, 측정값과 검증 범위를 과장하지 않는 상태로 리뷰한다.
