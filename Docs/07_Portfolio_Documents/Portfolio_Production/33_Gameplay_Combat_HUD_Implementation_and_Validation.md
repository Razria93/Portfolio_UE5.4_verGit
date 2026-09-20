# 전투 HUD 구현 및 검증

기준일: 2026-09-20. 브랜치: `feat/stellar-combat-hud`.
구현 기준: `48eaae82..65157687`. 이후 마감 검토의 변경은 문서 최신화와 줄 끝 공백 정리다.
브랜치 전체의 변경 이유·게임플레이 보완·검증 한계는 [34 마감 기록](34_Gameplay_Combat_HUD_Branch_Closure.md)을 따른다.

## 현재 구조

- `CPlayerController`의 `CombatHUDPresenter`가 로컬 컨트롤러에서 HUD를 생성한다. 기본 클래스는 Native `UCCombatHUDWidget`, ZOrder는 5다.
- Presenter가 플레이어·현재 Combat Target을 관측해 `FCombatHUDViewData`를 전달한다. HP·Balance·타깃 변경은 이벤트 기반이며, 가드·닷지·처형 상태는 `TG_PostUpdateWork`에서 조회한다. 액션 데이터가 달라질 때만 액션 표시를 갱신한다.
- Widget은 C++에서 UMG 트리를 구성한다. 표시 계층이 입력 실행·자원 소모·처형 예약을 소유하지 않는다. `WBP_CombatHUD`를 필수로 만들거나 하드코딩된 WBP 경로를 로드하지 않는다.
- 플레이어/타깃 교체, 해제, 사망, EndPlay에 맞춰 구독과 표시를 정리한다.
- `CEnemy::CombatHUDDisplayName`이 대상 이름의 원천이다. 비어 있거나 해당 Enemy 타입이 아니면 `TARGET`을 사용한다.
- 디버그 텍스트 패널은 `DebugCanvas`를 사용해 UMG보다 앞에 그린다. 전투 HUD의 ZOrder를 무작정 올리는 방식이 아니다.

## 실제 연결과 샘플 구분

| 표시 | 현재 상태 |
| --- | --- |
| Player HP / Target HP | 실제 Health 값. 0은 숫자 0, 미확보 값은 대시 |
| Target Balance | `Clamp(Threshold - CurrentCount, 0, Threshold)`의 남은 수량 |
| 가드 | 실행 가능 여부와 실제 가드 자세에 따라 Ready/Active/Unavailable |
| 패링 | 승인된 자기 대상 패링 결과를 게임 시간 0.25초 강조; serial 중복 방지 |
| 닷지 | 실행 가능 여부와 액션 실행 중 상태. 퍼펙트 닷지 성공 표시는 아님 |
| 처형 | 락온 + 공통 시작 조건 평가로 Ready, 시전자 협업 세션으로 Active |
| BU / BE / Player SH / Target SH | 실제 자원 시스템 미연결. Editor 외형 검토 샘플 또는 빈 외곽선·대시 |
| 돌진 / 카운터 / 아이템·수량 / 보조 작은 원 | 미구현 자리표시. 쿨타임·비용·입력을 가장하지 않음 |
| 가드브레이크 | 아이콘 보관·로드만 함. 브레이크 판정·쿨타임·아이콘 교체 미구현 |

`bShowResourceSamples`는 Editor에서 기본 true다. BU 1200/1600, BE 700/1400, Player SH 560/800, Target SH 2700/4500을 SAMPLE 라벨과 함께 표시한다.
옵션을 끄거나 비 Editor 빌드에서는 샘플을 표시하지 않는다. HP와 Balance는 샘플 자원으로 대체하지 않는다.
TODO 문자열과 포션 위 작은 원은 제거했고, 우측 상단 돌진 슬롯과 액션 옆 작은 원은 남겨둔다.

## 게이지와 배치

- 기준 캔버스는 2560×1440, ScaleToFit. 21:9에서는 중앙 16:9 영역을 사용하고 외곽에 검은 배경을 그리지 않는다.
- Player HP는 3행, 열당 100. 최대값이 바뀌어 열 수가 달라질 때만 다시 만든다. 580 논리 폭 기준 77열/7700까지 표시하고 초과 시 숫자는 실제 값을 유지한다.
- BU·BE는 2행×2열/유닛당 200, 셀당 50. 셀 너비 대신 알파를 채운다. 현재는 샘플 구현이다.
- Player SH는 2행×10열/유닛당 400, 열당 40. 현재는 샘플 구현이다.
- Target HP는 고정 114열, Target SH는 22열×5묶음. 880 논리 폭 예산에서 양 끝을 맞춘다. 열 수를 퍼센트 숫자 100과 억지로 일치시키지 않는다.
- 실제 HP의 경계 열은 세 행의 너비를 동일 비율로 줄인다. 게임플레이 HP 자체를 정수화하지 않는다.
- 논리 셀 크기 5, 틈 2.5, 묶음 틈 5를 최종 Paint 배율에서 정수 픽셀로 변환한다. 완충 셀의 크기와 반복 간격을 통일하되 부분 충전 폭은 비율을 유지한다.
- Balance 도형은 최대 64개만 할당하고 숫자는 실제 최대값을 유지한다.
- HP 숫자는 표시만 올림해서 양수 소수 체력이 0으로 보이지 않도록 한다. 비정상 수치와 미확보 데이터는 대시/0 비율로 처리한다.

## 리소스

- 런타임: `/Game/08_UI/CombatHUD/`의 Composite Font 2개, FontFace 3개, Texture 7개.
- 일반 라벨·숫자: Oxanium Regular, 이름: Oxanium Medium, 한글 fallback: Noto Sans CJK KR. 원본 및 OFL 라이선스는 `Resources/CombatHUD/Fonts`에 보관한다.
- 아이콘은 생성한 원본을 프로젝트에 통합한 것이다. 원형 테두리는 UMG에서 구성한다.
- `CCombatHUDAssetsCommandlet -CreateMissing`은 없는 에셋 생성 도구다. 기존 에셋의 최신 내용 검증/재임포트 도구가 아니다.
- Target Marker의 위젯/텍스처 에셋 경로 정리도 브랜치에 포함한다. 추적 기능을 신규 구현한 것으로 집계하지 않는다.

## 검증 결과

2026-09-20 마감 검토:

- `PortfolioEditor Win64 Development` 빌드 성공. 첫 시도는 열린 Editor의 DLL 잠금으로 링크 실패했으며, 사용자 종료 후 재시도 성공.
- `Portfolio.UI.CombatHUD.` 12개 + `Portfolio.Balance.` 2개: 총 14개 모두 성공.
- 로그: `Saved/Logs/HUDClosureValidation.log` (로컬 산출물).
- `git lfs fsck`: 성공. 로컬 검사이며 원격 업로드 완료를 뜻하지 않는다.
- 사용자 PIE 확인 완료. 사용자 확인을 에이전트가 직접 실행한 시나리오별 검증으로 확대 해석하지 않는다.
- 기존 HUD 12개 반복 실행 성공 기록: `HUDFixtureFixFull1.log`, `HUDFixtureFixFull2.log`.
- RenderPreview는 RHI로 1920×1080, 2560×1440, 3440×1440 샘플 UMG를 렌더한다. 실제 전투 입력/애니메이션 재생 테스트가 아니다.

자동 검사는 값 관측, 비정상 값, 게이지 단위·간격, Widget 재구성·색상, Presenter 구독 수명, 준비된 액션/처형 조회, Balance 이벤트 경계·복구·타이머 정리를 다룬다.
패키징/cook, 모든 Montage 조합, 장시간 플레이, 플랫폼 전체 DPI와 배경 대비의 완전한 검증은 주장하지 않는다.

## 마감 범위

브랜치 마감 때문에 가드 게이지·브레이크·돌진·카운터·BE 스킬·새 WBP 구조를 추가하지 않는다.
현재 구현과 사용자 PIE 확인을 기준으로 HUD 작업을 마감하고, 미구현 기능은 후속 게임플레이 요구사항으로 분리한다.
