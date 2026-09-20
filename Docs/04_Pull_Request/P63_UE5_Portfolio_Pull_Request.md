# UE5 Portfolio Pull Request

## 제목

**P63: Stellar 전투 HUD 구현과 액션 상태 조회·처형 생명주기 보완**

## 날짜

**2026.09.20**

## 상태

- [x] Native UMG 전투 HUD와 플레이어·타깃 데이터 연결
- [x] 고정 자원 단위·보스 비율 게이지 및 픽셀 정렬
- [x] 가드·닷지·처형 상태와 패링 성공 표시 연결
- [x] Action/Reaction 조회·실행 평가 공통화
- [x] 처형 시작 평가·취소와 Balance 복구 경계 보완
- [x] 아이콘·폰트·라이선스 및 디버그 패널 우선순위 정리
- [x] Editor 빌드·자동 테스트 14개·LFS 검사 및 사용자 PIE 확인
- [x] 구현·검증·브랜치 마감 문서 정리
- [ ] PR 리뷰 및 병합

## 브랜치

- Base: `main`
- Branch: `feat/stellar-combat-hud`
- Base HEAD: `48eaae82`
- Implementation HEAD: `65157687`
- 마감 문서: `81510b1d`
- 최종 소스 서식 정리: `314a70c5`

선행 PR #121·#122의 에셋 통합 및 Montage 종료 계약은 기준선이다. 이번 PR의 신규 구현으로 중복 집계하지 않는다.

## 요약

플레이어의 체력과 현재 적의 이름·체력·Balance를 표시하는 전투 HUD를 추가했다. 가드·닷지·처형 아이콘은 실제 실행 가능 상태와 진행 상태를 보여주며, 패링 성공은 확정된 전투 결과를 받아 잠깐 강조한다.

화면 구성을 추가하면서 판정 책임은 기존 전투 시스템에 남겼다. HUD가 실행 가능 조건을 따로 구현하지 않도록 Action/Reaction의 평가 경로를 공유하고, 처형 조회와 실제 요청의 시작 조건도 공통화했다. 관련 검토에서 확인한 처형 취소·Balance 복구 경계와 테스트 초기화 문제를 함께 보완했다.

BU·BE·SH는 실제 자원 시스템이 아니라 Editor 외형 검토 샘플이다. 돌진·카운터·가드브레이크와 새 스킬을 구현한 PR은 아니다.

## 핵심 개념

- Presenter(게임 상태 관측·표시 데이터 전달): `UCCombatHUDPresenterComponent`가 이벤트와 액션 조회로 화면에 필요한 상태를 구성한다.
- ViewData(표시용 상태): `FCombatHUDViewData`와 `FHUDActionViewData`로 전투 객체와 Widget의 직접 의존을 줄인다.
- 준비된 실행 조회: 데이터와 executor가 이미 존재하는 경우에만 평가한다. HUD 조회 때문에 실행 객체 생성·예약·몽타주 재생을 하지 않는다.
- 픽셀 정렬: 논리 배치와 실제 Paint 배율을 구분하고, 완충 셀 크기와 반복 간격을 최종 픽셀 단위로 통일한다.

## 변경 배경

실제 전투 정보를 확인할 HUD가 필요했지만, 아직 없는 자원·스킬까지 작동하는 것처럼 표시해서는 안 됐다. 실제 데이터와 샘플을 구분하고, 값 관측과 액션 실행 가능 여부를 각각 적합한 경로로 전달해야 했다.

게이지는 화면 배율에 따라 셀 크기와 틈이 불균일하게 보일 수 있었다. 개별 좌표 반올림 대신 공통 픽셀 간격을 사용하고, 플레이어의 고정 단위와 보스의 고정 폭 비율 표현을 분리했다.

## 주요 변경

### 1. Controller–Presenter–Widget 구성

`CPlayerController`에 Presenter를 연결하고 로컬 Controller에서 Native `UCCombatHUDWidget`을 생성한다. HP·Balance·타깃 변경은 이벤트로 관측하고, 액션 상태는 `PostUpdateWork`에서 조회해 변경된 상태만 전달한다.

플레이어·타깃 교체와 해제, 사망, EndPlay에서 이전 구독과 표시를 정리한다. Widget 재구성 시 참조·셀 캐시를 초기화하고 ViewData를 다시 적용한다. Enemy 이름은 `CombatHUDDisplayName`에서 가져오며 미설정 시 `TARGET`을 사용한다.

### 2. 자원 단위와 픽셀 정렬

Player HP는 100/열, BU·BE는 200/4셀 유닛과 알파 채움, Player SH는 400/10열 유닛으로 구성했다. BU·BE·SH는 현재 샘플 표현이다.

Target HP는 114열 고정 비율, Target SH는 22열×5묶음으로 양 끝을 맞춘다. HP의 부분 열은 실제 비율을 유지하며 게임플레이 값을 정수화하지 않는다. Player HP는 77열/7700, Balance 도형은 64개로 시각 할당을 제한하고 숫자는 실제 값을 유지한다.

`CCombatHUDGaugeLayout`은 단위·논리/픽셀 치수를, `CPixelAlignedGaugeImage`는 최종 Paint geometry 보정을 담당한다. 2560×1440 기준 ScaleToFit 배치이며 21:9에서는 중앙 16:9 영역을 사용한다.

### 3. 액션 상태와 공통 평가

가드는 실행 가능 여부와 방어 자세, 닷지는 실행 가능 여부와 활성 액션, 처형은 락온·공통 시작 평가와 시전자 협업 세션을 기준으로 표시한다. 패링은 승인된 자기 대상 결과를 serial로 중복 방지하고 게임 시간 0.25초 강조한다.

`FindPreparedActionContext` / `FindPreparedGlobalReactionContext`로 준비된 실행 정보를 조회하고, `EvaluateActionContext` / `EvaluateReactionContext`를 실제 요청과 공유한다. 이미 만든 Query를 재사용해 요청 내부의 중복 Snapshot 구성을 줄였다. 처형 Reaction은 Start만 허용하고 예약·지연을 즉시 사용 가능으로 취급하지 않는다.

Overlay의 최종 API는 `BuildOverlaySnapshot`이다. 정책 레지스트리 갱신과 현재 상태 수집을 유지한다. 내부 캐시 갱신까지 없는 순수 함수로 바꾼 것은 아니다.

### 4. 처형·Balance 생명주기 보완

`EvaluateExecutionStart`가 HUD 조회와 실제 요청의 시작 조건을 평가한다. 요청은 HUD 결과를 재사용하지 않고 그 시점에 다시 검사한다. 실패 원인은 상세 로그로 남기고, 대상의 예약 수락과 활성화·커밋 경계 검증은 유지한다.

커밋 전 취소는 예약 해제와 Collapse 복구, 커밋 후 취소는 동일 serial의 남은 `ExecutionPrimaryCommitted` 정리를 수행한다. 이미 적용한 피해는 되돌리지 않는다. 이전 참가자 이벤트는 참조 교체 전에 해제한다.

Balance는 늦게 도착한 이전 단계 종료 이벤트를 현재 단계와 대조한다. Reset은 최종 상태를 먼저 정리한 뒤 알리고, Shutdown은 게임플레이 상태 전이 알림을 생략한다. 복구 재시도 지연이 0이면 다음 Tick으로 예약하며 횟수 제한·취소를 유지한다.

### 5. 리소스·디버그·문서 정리

Composite Font 2개, FontFace 3개, Texture 7개와 원본·OFL 라이선스를 보관했다. Target Marker 관련 위젯·텍스처 경로도 정리했다. `CCombatHUDAssetsCommandlet -CreateMissing`은 없는 에셋 생성용이며 기존 에셋 감사·재임포트 도구가 아니다.

디버그 패널만 `DebugCanvas`로 렌더해 UMG보다 앞에 표시한다. 새 WBP Designer 구조는 도입하지 않았다. 오래된 설계 기록에는 현재 구현 문서 링크를 추가하고, 브랜치 전체 변경과 검증 한계를 별도 마감 기록으로 정리했다.

## 주요 처리 흐름

```text
HP / Balance / Target 이벤트 → Presenter → ViewData → Widget 표시
PostUpdateWork → 액션 가용성 조회 → 변경된 액션 상태만 Widget에 전달
승인된 패링 결과 → 중복 serial 검사 → 0.25초 강조 → 만료

HUD 처형 조회 → 공통 시작 평가 → 가능 여부 표시
실제 처형 요청 → 새 공통 시작 평가 → 예약 → 양측 실행 → 활성화 → 커밋
강제 취소 → 세션 초기화 → 로컬 실행 취소 → 커밋 전/후 Balance 정리
```

## 트러블슈팅과 설계 판단

- 셀을 전체 폭에 맞춰 늘이는 대신 공통 픽셀 크기·간격을 사용했다. 부분 채움은 유지하며 HP 자체를 양자화하지 않는다.
- HUD 조회와 실제 요청은 조건을 공유하지만 호출 시점은 다르다. 조회 후 상태 변경과 예약 경계 검증을 제거하지 않는다.
- 자동 테스트의 참조 초기화·컴포넌트 등록·WorldContext 누락을 수정했다. 런타임 판정을 완화해 테스트를 통과시킨 것이 아니다.
- 가드 재시작 제한을 가드브레이크로 해석하지 않는다. 브레이크 아이콘은 준비된 리소스로만 남겼다.

## 변경 파일 범위

- Runtime: HUD Presenter/Widget/Types, 게이지 배치·Paint 보정, Controller·Enemy·Debug Overlay 통합.
- Gameplay: Health/Balance 관측, Action/Reaction 준비 데이터 조회·평가, ExecutionCollaboration 시작·취소·이벤트 연결.
- PortfolioEditor: HUD/Balance 자동 테스트와 HUD 리소스 생성 Commandlet.
- Content/Resources: 폰트·아이콘·Blueprint 연결·Target Marker 경로·원본·라이선스.
- Docs: 설계 기록, 현재 구현·검증, 액션 연동, 브랜치 마감 및 본 PR 문서.

## 테스트 방법

1. Editor 종료 후 `PortfolioEditor Win64 Development`를 빌드한다.
2. RHI를 사용해 `Automation RunTests Portfolio.UI.CombatHUD.+Portfolio.Balance.`를 실행한다.
3. `git diff --check main...HEAD`와 `git lfs fsck`로 형식·로컬 LFS 무결성을 확인한다.
4. PIE에서 HP·타깃 변경 및 가드·패링·닷지·처형 표시, 디버그 패널 중첩을 확인한다.

위 항목은 재현 절차다. 실제 수행 결과와 검증 범위는 아래와 구분한다.

## 검증 결과

| 항목 | 결과 | 근거·범위 |
| --- | --- | --- |
| Editor Win64 Development | 성공 | 열린 Editor DLL 잠금으로 첫 링크 실패, 사용자 종료 후 재빌드 성공 |
| HUD 자동 테스트 | 12/12 성공 | 값 관측·가용성·Widget/Presenter 수명·게이지·렌더 |
| Balance 자동 테스트 | 2/2 성공 | 단계 경계·커밋 후 취소·타이머·Reset/Shutdown 관측 |
| 자동 RenderPreview | 성공 | 1920×1080, 2560×1440, 3440×1440 샘플 UMG |
| Git LFS fsck | 성공 | 로컬 객체 검사; 전체 과거 이력이나 원격 상태 보장 아님 |
| 공백 검사 | 통과 | 마감 정리 후 브랜치 차이 검사 |
| PIE | 사용자 확인 완료 | 에이전트의 개별 시나리오 전수 실행으로 확대하지 않음 |

검사일은 2026-09-20이며 자동 검사 로그는 `Saved/Logs/HUDClosureValidation.log`다. 로그·렌더 PNG는 로컬 산출물이다. PR 문서 작성 단계에서 동일 검사를 새로 실행한 것으로 기록하지 않는다.

## Scope Guard

이번 PR에서 하지 않은 것:

- 실제 BU·BE·SH 자원 충전/소모, BE 스킬 구현.
- 돌진·카운터·가드브레이크 판정/쿨타임, 퍼펙트 닷지 성공 표시.
- 아이템 수량·사용, 보조 작은 원의 기능.
- HP 잔상 애니메이션, WBP Designer 전환, 신규 가드 게이지.
- 선행 브랜치의 캐릭터·무기 통합 및 Montage 종료 계약 재구현.

## 리스크 / 리뷰 포인트

- Editor 자원 샘플은 SAMPLE 라벨로 구분하며 비 Editor 빌드에서는 표시하지 않는다.
- 표시 한도 초과 시 숫자가 기준이다. 최대 성장 수치에 대한 새 표현 정책은 후속 범위다.
- Ready는 요청 시점의 조건 통과를 뜻하며 이후 모든 Montage 재생 성공을 보장하지 않는다.
- 패키징/cook, 네트워크·다중 로컬 플레이어, 전 플랫폼 DPI, 모든 Montage 조합·장시간 플레이는 검증 범위 밖이다.
- 기존 Debug Overlay 콘솔 조회 성능 경고 등과 별개로, 전체 엔진 로그 무경고나 측정 없는 성능 개선은 주장하지 않는다.

## 후속 작업

PR 리뷰·병합으로 현재 HUD 범위를 마감한다. 미구현 자원·액션은 별도 게임플레이 작업에서 규칙을 정한 뒤 표시를 연결하며, 이번 마감을 위해 기능 범위를 확대하지 않는다.

## 관련 문서

- [브랜치 마감 기록](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/314a70c5/Docs/07_Portfolio_Documents/Portfolio_Production/34_Gameplay_Combat_HUD_Branch_Closure.md)
- [현재 구현 및 검증](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/314a70c5/Docs/07_Portfolio_Documents/Portfolio_Production/33_Gameplay_Combat_HUD_Implementation_and_Validation.md)
- [액션 상태 연동](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/314a70c5/Resources/CombatHUD/ActionStateIntegration.md)
- [리소스 원본·라이선스](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/314a70c5/Resources/CombatHUD/README.md)

## 대표 커밋

```text
757e5e21 feat(ui): implement compact Stellar combat HUD and validation
12aa0d24 feat(ui): refine combat HUD resource units and pixel alignment
d9a8fdb8 fix(debug): render overlay panels above combat HUD
36757b4b refactor(combat): unify availability queries and harden execution lifecycle
6db350a1 refactor(ui): organize combat HUD presentation and pixel-aligned rendering
65157687 test(ui): fix combat HUD action test initialization
81510b1d docs(ui): finalize combat HUD branch closure record
314a70c5 style(ui): finalize combat HUD source formatting
```

## 정리

이번 PR은 전투 정보를 화면에 연결하면서 판정과 표시의 책임을 분리하고, 조회에 필요한 공통 평가와 생명주기 경계를 보완했다. 실제 기능·샘플·미구현 자리를 구분한 상태로 현재 HUD 범위를 마감한다.
