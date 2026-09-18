# 전투 HUD 구현 및 검증

기준: 2026-09-18, `feat/stellar-combat-hud`. 축소 배치·폰트 승인 및 HP/SH 교체 요청 이후 구현. 이후 사용자 PIE에서 너무 작다는 피드백으로 축소 전 명세 크기를 재적용함. 사용자 PIE 최종 확인 전이며 Push/PR/merge는 수행하지 않음.

## 구현

- `CPlayerController`의 새 `CombatHUDPresenter`가 로컬 플레이어 HUD를 생성함. 기존 Target Marker·Debug Overlay는 변경하지 않음.
- `CCombatHUDPresenterComponent`: Player HP, 현재 Combat Target의 표시명·HP·Balance를 구독하고 표시 snapshot을 만듦. 매 프레임 폴링하지 않음. 타겟 교체·해제·사망·EndPlay·플레이어 교체/해제 시 구독과 표시를 정리함.
- `CHealthComponent`/`CBalanceComponent`: 실제 값이 변경된 함수 종료 시 관측용 native delegate를 전달함. 피해 계산·사망·Action/Reaction 종료 정책은 변경하지 않음.
- `CEnemy::CombatHUDDisplayName`: Blueprint 기본값에서 설정하는 표시명. 미설정은 `TARGET`이며 Actor label을 사용하지 않음.
- `CCombatHUDWidget`: Native UMG WidgetTree. 현재 비교안은 Target 880px·Player HP 580px·스킬 슬롯 72px의 2560×1440 기준 배치. Player 행은 `BU → BE → SH → HP`.
- Balance는 `Clamp(Threshold - CurrentCount, 0, Threshold)`의 남은 칸. 작은 노란 다이아몬드로 표시하며 표시 allocation은 64개로 제한함. gameplay threshold는 변경하지 않음.
- 실제 HP 0은 빈 실선 격자, 미확보 HP는 대시, 미구현 자원은 빈 outline 격자와 대시로 구분함. 스킬·아이템·보조 원형 슬롯은 비활성 TODO 표현이며 입력·수량·쿨다운을 가장하지 않음.
- 외형 검토용 예외: Editor에서 `bShowResourceSamples` 기본 활성화. BU `1,200 / 1,600`(산호색), BE `700 / 1,400`(청색), SH `560 / 800`(민트색)을 고정 표시하고 SAMPLE 라벨로 구분함. Presenter/게임 상태에는 값을 주입하지 않으며 HP는 실제 값 유지. 옵션 비활성화 또는 비 Editor 빌드에서는 미구현 표현으로 복귀함. 보완 후 Editor 빌드와 HUD 자동검사 7개 통과, 실제 UMG 렌더에서 색·수치 배치 확인.
- 현재 값은 즉시 표시함. HP 잔상·감소 애니메이션은 아직 넣지 않았으며 첫 PIE 가독성 확인 후 필요 여부를 판단함.
- 후속 시각 보완: 공통 5×5 셀, BU/BE 2×2 묶음, SH 2×11 묶음, Player/Target HP 연속 3행. HP 경계 열은 세 행 모두 실제 폭으로 부분 채움. 상세 치수는 32 시각 명세의 현재 구현을 따름. 경계 반짝임은 미적용.

## 리소스와 편집 위치

- Runtime: `/Game/08_UI/CombatHUD/` — 폰트 2개, FontFace 3개, 아이콘 Texture 5개.
- 원본 및 정확한 생성 프롬프트: [Resources/CombatHUD](../../../Resources/CombatHUD/README.md).
- Oxanium Regular/Medium + Noto Sans CJK KR fallback. OFL 원문 동봉.
- imagegen 스킬의 built-in 생성 경로로 개별 RGBA 아이콘 제작. 원형 테두리는 UMG 도형으로 별도 구성. 원작 텍스처를 추출한 것이 아님.
- uasset은 기존 Git LFS 정책 적용. PNG/TTF/OTF 원본은 현 정책상 일반 Git. 과거 이력 변경 없음.
- `CCombatHUDAssetsCommandlet -CreateMissing`: 없는 전용 에셋만 생성. 기존 에셋은 덮어쓰지 않음. 일반 실행에 필요한 도구가 아님.
- 스타일 속성은 Widget 기본값에서 변경 가능하지만 현재 배치 트리는 C++이 생성함. UMG Designer에서 드래그 편집하는 WBP는 이번 구현에 없음.
- 21:9에서는 중앙 16:9 안전 영역을 유지함. 검은 letterbox를 그리지 않으며 외곽은 투명함. 실제 viewport DPI/플랫폼 safe zone은 PIE에서 확인 필요.

## 로컬 검증

- `PortfolioEditor Win64 Development`: UHT/C++ 빌드 통과.
- 신규 리소스 import: 0 errors / 0 warnings.
- `Portfolio.UI.CombatHUD`: 공통 셀 보완 후 7개 통과.
- Enemy HP·Balance 실수치, SH Editor 샘플 2700/4500 추가 후 빌드·7개 자동검사 및 실제 UMG 렌더 확인. 그 이후 TODO 두 곳 가운데 정렬과 기능 미확정 원 두 개 제거는 소스 반영 및 diff 검사만 완료; 열린 에디터를 유지하기 위해 재빌드/PIE는 수행하지 않음. 사용자가 게이지 표현을 정상 확인했으며, 디버그 오버레이 표시 우선순위는 논의만 했고 변경하지 않음.
- 고정 자원 단위 / 보스 5묶음 보완: BU·BE 200/유닛(50/셀, 알파), Player HP 100/열, SH 400/10열 유닛(40/열). 최대 HP 증감 시 위젯 열 재구성, 피해만 받을 때 열 수 유지, 부분 단위 및 비정상적으로 큰 최대치의 할당 상한을 검사함. 보스 HP 114열과 SH 22열×5묶음의 정수 픽셀 폭 일치를 6개 배율에서 검사. 보스 SH 실제 데이터 연결은 범위 밖.
- 픽셀 정렬 보완 후 빌드 및 동일 7개 재통과. 6개 배율에서 고정 pitch·폭 예산을 검사함. 실제 UMG 렌더 PNG의 Player HP 완전 충전 셀을 측정: 1920×1080은 3×3px / 가로·세로 틈 2px, 2560×1440 및 3440×1440은 5×5px / 틈 2px로 균일. 부분 충전 경계는 측정에서 구분함. PIE 및 창 크기 변경의 사용자 검수는 별도이며, 캡처 후 이미지 리사이즈까지 보장하지 않음.
  - HealthObservation: 초기화·피해·회복·최대값 변경·사망·무효 입력·0 최대값.
  - BalanceObservation: 중간 패리·중복 serial·threshold·shutdown/reset.
  - ResourceView: 비율 및 0/미구현 구분.
  - WidgetConstruction: 실제 UMG 트리 생성·snapshot 적용/초기화.
  - GaugeLayout: 공통 셀 높이·묶음 간격·폭 제한·부분 열 계산. WidgetConstruction에서도 HP 경계 열의 세 행이 실제 절반 폭으로 표시되는지 검사함.
  - PresenterLifecycle: 타겟 획득·교체·이전 구독 해제·사망·clear·EndPlay·플레이어 해제.
  - RenderPreview: RHI를 사용한 1920×1080, 2560×1440, 3440×1440 UMG 렌더 및 PNG export.
- PresenterLifecycle의 EndPlay는 테스트 world에서 delegate를 직접 전달한 검사임. 실제 레벨 전환/Destroy의 모든 순서를 검증했다는 의미가 아님.
- 결과: `Saved/Automation/CombatHUD/index.json`, `hud-*.png` (로컬 생성물).
- 자동 렌더는 sample snapshot이며 실제 플레이 결과가 아님. 한글·영문·격자·아이콘 출력 및 화면 경계 안 배치를 확인함.
- Editor 시작 로그의 기존 DebugOverlay Console object 조회 성능 경고 등은 별개이며, 전체 프로젝트 로그 무경고를 주장하지 않음.
- 패키징/cook, 실제 밝고 어두운 게임 배경 대비, Marker/Debug Overlay 중첩, 사용자 PIE는 미검증.

## 사용자 확인 — 한 번에 확인할 최소 범위

1. 에디터를 새 빌드로 열고 기존 게임 맵에서 PIE. 기존 `CPlayerController` 계열을 쓰면 별도 위젯 배치 없이 표시되어야 함.
2. Player 피해·회복, 적 선택·교체·해제·처치, 패리 후 Balance 감소/복구를 확인. 이전 타겟 값이 남지 않아야 함.
3. `BU → BE → SH → HP` 순서, 작은 Balance, 아이콘·한글 표시명과 밝은/어두운 배경 가독성 확인. 미구현 칸은 작동하지 않는 자리를 의미함.
4. 창 크기 변경 및 PIE 재시작. 잘림·HUD 중복·기존 Target Marker/Debug Overlay 가림 여부 확인.

문제가 있으면 해상도와 캡처 한 장, 수행한 동작만 전달하면 됨. 기존 전투 전체를 다시 검수할 필요는 없음. 확인 후 필요한 표현 조정과 최종 마감 커밋을 진행함.
