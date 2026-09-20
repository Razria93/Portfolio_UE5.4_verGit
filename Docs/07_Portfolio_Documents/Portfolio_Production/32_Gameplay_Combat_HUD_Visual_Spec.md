# Gameplay Combat HUD Visual Spec

> 2026-09-20 최종 상태: 사용자 PIE 확인 완료. TODO 문자열은 제거했고 가드·닷지·처형과 패링 강조를 실제 상태에 연결했다. 우측 상단 작은 원은 미구현 돌진 슬롯이다. 가드브레이크는 리소스만 보관한다. 아래 시안·대기·TODO 기록은 변경 이력이며, 현재 구현은 [33 구현·검증](33_Gameplay_Combat_HUD_Implementation_and_Validation.md), 전체 변경은 [34 마감 기록](34_Gameplay_Combat_HUD_Branch_Closure.md)을 따른다.

## 목적

32: Stellar Blade 참고 HUD의 배치·리소스·폰트·미구현 표현 초안.

### 현재 적용: 축소 전 크기 비교 (2026-09-18)

사용자 PIE 피드백으로 축소 전 `4108e444`의 치수를 다시 적용함. 아래 축소안·생성 시안 기록은 비교용으로 보존함. 폰트 종류와 `BU → BE → SH → HP`, 데이터 연결은 유지함.

- 2560×1440 기준 Target 폭 660 → 880, HP 높이 10 → 28, 이름 font size 18 → 21.
- Player HP 폭 360 → 580, 높이 10 → 18, 행 간격 24 → 28, 라벨 font size 15 → 17.
- Balance 회전 후 외곽 약 8.5 → 12, 칸 중심 간격 15 → 19.
- 아이템 44 → 56, 스킬 64 → 72, 상단 보조 원 20 → 28.
- 당시 명세에 없는 Player HP 높이는 초기 프리뷰의 두꺼운 게이지를 참고한 비교값이며, 생성 이미지 픽셀의 정확한 복원은 아님. 최종 크기는 재확인 대기.

후속 정렬: 포션 박스 두 개의 상단 0 / 하단 128을 기준으로 BU 게이지 상단과 HP 게이지 하단을 맞춤. 자원 행 원점은 -7부터 105까지 균등 배치(간격 약 37.33). 각 행의 글자·게이지 크기와 가로 위치는 유지하며 포션 위 원형 표시·아래 TODO는 기준에서 제외함.

텍스트 중앙 정렬 보완: 게이지 중심 Y를 기준으로 HP 숫자·BU/BE/SH 대시·자원 라벨의 세로 위치를 올림. 현재 Oxanium 기준 시각 보정. 최초 빌드는 Editor DLL 잠금으로 중단되었으나, 이후 사용자 확인 및 공통 셀 변경의 재빌드에 포함함.

### 공통 셀 표현 (현재 구현)

마지막 배치 정리: 포션 아래와 스킬 위 TODO를 각 아이콘 중심으로 가운데 정렬. 포션 위 원형 자리표시는 제거하고, 스킬 그룹 위 원은 후속 사용자 요청으로 복원함(기능 연결 없음). 스킬 아이콘 옆 작은 원도 유지. 아래 과거 시안의 보조 원형 슬롯 기록은 현재 표시 상태와 구분함.

후속 픽셀 정렬: `CPixelAlignedGaugeImage`가 최종 Paint geometry 배율로 셀 크기·간격·묶음 간격을 한 번 정수화하고, 게이지 원점부터 동일 pitch로 반복 배치함. 개별 셀 좌표를 각각 반올림하지 않음. 기존 열 수와 폭 예산을 보존하기 위해 실제 그려지는 폭·높이는 배율에 따라 조금 작아질 수 있음. 숫자 위치와 게임 값은 유지하며 부분 충전 열의 채움만 비율을 유지함. 아래 수치는 정렬 전 논리 geometry 기준임.

- 1440p 기준 셀 5×5, 내부 간격 2.5, 묶음 간격 5. 셀 뭉침 완화를 위해 가로·세로 간격을 1.5에서 확대함. 게이지 폭을 셀 수로 나누어 셀을 늘이지 않음.
- BU·BE: 2행×2열 = 200, 셀당 50. 왼쪽 열 위→아래, 오른쪽 열 위→아래 순서. 셀 크기는 고정하고 foreground 알파를 충전량으로 보간하며 빈 셀 배경은 유지. 샘플 최대치 BU 1600은 8유닛, BE 1400은 7유닛.
- Player SH: 2행×10열 = 400, 열당 40. 샘플 최대치 800은 2유닛. 부분 열은 두 행의 채움 너비로 표현.
- Player HP: 열당 100, 3행. 최대 HP가 바뀌면 열 수를 다시 구성하며 5000은 50열. 580 논리 폭의 현재 안전 한도는 77열(7700); 초과 시 경고하고 표시 열 수를 제한하되 숫자는 실제 값 유지. 최종 성장 한도는 아직 미정. 최대치가 100의 배수가 아니면 마지막 열은 부분 용량이며 억지로 완충하지 않음.
- Target HP: 고정 비율 표시, 3행×114열. 880 폭 예산에서 SH 묶음 제약을 먼저 계산한 결과이며 원작 88열의 복제가 아님.
- Target SH: 2행×22열×5묶음. 묶음당 최대치의 20%를 의미하도록 설계. 묶음 경계에 1 pitch씩 추가하여 HP 114열과 정수 픽셀 너비가 모든 검증 배율에서 일치함. 실제 SH 데이터는 미구현. Editor 샘플 옵션에서는 민트색 `2,700 / 4,500`(60%, 3묶음)과 `SAMPLE: SH`를 표시하고, 옵션 비활성/비 Editor 빌드는 outline/대시 유지.
- Target 숫자: 오른쪽 동일 x=888에 HP·SH·Balance의 현재/최대를 세로 배치. HP와 Balance는 실제 ViewData를 갱신하며 SH만 고정 샘플. 대상 해제 시 상단 패널 전체와 함께 숨김.
- 보스 HP·SH는 같은 중심에 배치하고 Balance도 게이지 시작선에 맞춰 이동. HP·SH 셀 크기는 동일하며 행 수로 구분.
- HP 배경 셀과 밝은 채움 셀을 분리함. Player는 `현재 HP / 100`, Target은 `비율 × 열 수`로 경계 열을 구하고 세 행의 실제 채움 폭을 동일하게 줄임. HP 계산은 양자화하지 않음.
- BU·BE·SH의 게임 시스템은 미구현. 외형 확인 요청에 따라 Editor에서는 `bShowResourceSamples`로 고정 샘플을 표시함: BU 산호색 `1,200 / 1,600`, BE 청색 `700 / 1,400`, SH 민트색 `560 / 800`. `SAMPLE: BU / BE / SH`를 함께 표시하며 HP는 실제 데이터 연결을 유지함. 옵션을 끄거나 비 Editor 빌드에서는 기존 outline/대시로 표시함. 충전·소모 시스템이나 가짜 애니메이션은 추가하지 않음.
- 경계 열 반짝임은 이번 단계에 넣지 않음. 셀 밀도·부분 채움을 PIE에서 먼저 확인한 후 별도 판단.

후속 보완: 포션 수량과 자원 라벨 사이 x=108에 2.67×128 회백색 세로 구분선 추가(사용자 피드백으로 1080p 약 2px로 조정). 수치 영역은 폭 220/Oxanium 15를 유지하되 고정 단위 게이지의 논리 끝에서 16만큼 떨어져 배치함. HP 0은 숫자, 미확보 값과 미구현 자원은 대시. 소수 HP는 표시만 올림하며 실제 계산·게이지 비율은 변경하지 않음.

## 관련 브랜치

- `feat/stellar-combat-hud`
- 상태: 폰트·v2 크기/배치 사용자 승인 (2026-09-18). HP/SH 순서 교체를 요청받아 v3 시안 반영. 아래 치수는 구현 시작값이며 원작 측정값 또는 적용 완료값이 아님.

---

## 1. 관찰 근거

원본: `C:/Users/starb/Videos/Bandicam/Stellablade/Cut`의 PNG 30개. 파일명 마지막 번호 기준으로 전체 contact sheet 비교 및 1번 확대 관찰함. 같은 Raven 보스전이며 일반 적·메뉴·비전투 동작은 보장하지 않음.

| 번호 | 관찰 | 한계 |
| --- | --- | --- |
| 1–2 | 상단 이름·격자 2계열·다이아몬드, BU/BE/HP/SH, 아이템, 원형 슬롯 | 보조 기호 기능 미확정 |
| 3–6 | 청색 적 VFX와 HUD | 월드 효과를 HUD 요소로 오인하지 않음 |
| 7–10 | 이동·카메라 변화에도 패널 유지 | screen-space와 월드 표시 분리 |
| 11–14 | 대형 근접 효과와 하단 UI 공존 | 명도 대비 필요 |
| 15–20 | 이동·공방에도 같은 구획 | 보간 시간은 정지 이미지로 확인 불가 |
| 21–28 | 클로즈업 중 HUD 존재 | 임의 cinematic 숨김 추가 안 함 |
| 29–30 | 종료 부근 동일 정보 구조 | 전체 사망/종료 hide 정책 미확정 |

원작 폰트명·정확한 색·시간·격자 수는 확정하지 않음. 캡처를 잘라 실행용 아이콘으로 사용하지 않음.

---

## 2. 배치·색상 제안

1440p 시각 좌표 기준이며 실제 UMG DPI 곡선에 맞춰 변환함. 21:9에서도 패널 폭을 제한함.

| 요소 | anchor / 위치 | 크기 초안 |
| --- | --- | --- |
| Target | 상단 중앙, 위 48 | 폭 640–690, 이름 24, HP 한 줄 높이 10–12 |
| 보조 자원 | HP 아래, 간격 6 | 높이 8, 미구현 outline |
| Balance | 그 아래 왼쪽, 간격 6 | 회전 후 bounding box 8–10, 간격 5, 실제 threshold 개수 |
| Player | 좌하단 x=190, 아래 72 | HP 폭 360–400, 행 간격 24, HP 한 줄 높이 10 |
| 아이템 | Player 왼쪽 x=64 | 48–56, 세로 배치 |
| 스킬 | 우하단 오른쪽 64, 아래 72 | 직경 64–72, 4개 마름모 배치 |
| 보조 스킬 | 그룹 상단/옆 | 24–32, 미구현 |

Player 행 순서는 BU → BE → SH → HP. HP/글자 #F1F2EE, 보조 자원 #9ACBCB, Balance #E9D864, 미구현 outline #69767D. 얇은 어두운 외곽선·그림자를 사용하고 큰 불투명 패널을 피함.

TODO는 빈 outline·대시·미구현 기호로 실제 0과 구분함. 잠금만으로 해금 가능한 기능처럼 보이지 않도록 디자인 검토 화면에는 설명을 함께 둠.

---

## 3. 승인된 폰트 조합

사용자는 폰트 조합을 승인함. 아래 크기는 축소 피드백에 따른 조정안이며 UMG에서의 최종 가독성 확인은 남아 있음.

저장소 검색에서 독립 TTF/OTF 원본을 발견하지 못함. 로컬 UE5.4 Slate/Fonts에는 Roboto와 DroidSansFallback 등이 있음. Font uasset 내부 전수 감사 결과는 아님.

| 용도 | 후보 | 1440p 시각 크기 |
| --- | --- | --- |
| 이름 | Oxanium Medium | 24px, 자간 약 0.18em |
| 라벨 | Oxanium Regular | 20px, 자간 약 0.06em |
| 숫자 | Oxanium Medium | 20px, 고정 폭 영역 |
| 한글/안내 | Noto Sans CJK KR Regular | 20–22px |
| 외부 미도입 대안 | 엔진 Roboto + fallback | 같은 크기에서 비교 |

px는 목표 시각 크기이며 UMG Font Size 확정값이 아님. 숫자 tabular 지원·한글 fallback은 실제 파일 검증이 필요함. 얇은 글자의 1080p 가독성을 확인함.

Oxanium [upstream OFL](https://github.com/sevmeyer/oxanium/blob/master/OFL.txt), Noto [공식 사용 안내](https://github.com/notofonts/noto-docs/blob/main/docs/website/use.md)에서 OFL 배포를 확인함. Runtime 반입 시 실제 파일의 버전·출처·저작권·라이선스를 보존함. 공식 원본을 임시 폴더에 받아 비교했으며 시스템 설치·프로젝트 font import는 하지 않음. 원작 폰트라고 주장하지 않음.

[실제 폰트 비교](Assets/HUD/font-study.png): 28/22/17px, 밝고 어두운 배경. Pillow 렌더이며 UMG 결과가 아님. 샘플 한글의 표시를 확인했으나 모든 glyph와 UE composite fallback 검증을 의미하지 않음. 숫자는 별도 고정 폭 영역을 유지함.

| 파일 | 공식 다운로드 경로 | SHA256 |
| --- | --- | --- |
| Oxanium-Regular.ttf | https://raw.githubusercontent.com/sevmeyer/oxanium/master/fonts/ttf/Oxanium-Regular.ttf | 2B9C2A53EE0A8248AB901463838CC81075B410724046A09683B3CC91870FE52A |
| Oxanium-Medium.ttf | https://raw.githubusercontent.com/sevmeyer/oxanium/master/fonts/ttf/Oxanium-Medium.ttf | D0676DE4894CD22591B4BB538DAE5B8E06C44E0FB943300A7CFF3945FE643689 |
| NotoSansCJKkr-Regular.otf | https://raw.githubusercontent.com/notofonts/noto-cjk/main/Sans/OTF/Korean/NotoSansCJKkr-Regular.otf | 6BCB2A0703AA137E874FC2DFFA85F6C21BA9A67FA329E81B8C801663AF7E992A |

재현: Pillow가 있는 Python으로 `Assets/HUD/render_font_preview.py <폰트 폴더>` 실행. Roboto는 스크립트의 engine-font-directory 인자로 조정 가능함. 최초 PowerShell 실행 정책 제한과 PATH Python 부재를 확인했고 정책 변경 없이 UE 내장 Python + 임시 폴더 Pillow로 렌더함. 엔진 설치 파일은 수정하지 않음.

---

## 4. 생성 시안

### 현재 기준: HP/SH 교체 v3

[v3 시안](Assets/HUD/hud-layout-project-preview-v3.png)은 사용자가 승인한 v2 크기·배치를 바탕으로 좌하단 HP와 SH의 라벨·게이지를 함께 맞바꾼 이미지임. BU/BE/SH는 미구현 빈 격자, 맨 아래 HP는 흰색 채움 게이지임. 다른 요소의 크기·배치를 변경하는 요청은 하지 않았음.

built-in imagegen 편집 결과이며 정확한 pixel 보존 또는 실제 UMG 실행 결과를 의미하지 않음. [v3 프롬프트](Assets/HUD/layout-v3-prompt.txt)를 보존함.

### 현재 검토본: 축소 v2

[축소 배치 v2](Assets/HUD/hud-layout-project-preview-v2.png). 2026-09-18 사용자 피드백에 따라 기존 프리뷰보다 Concept A의 작은 화면 점유율을 우선함. built-in imagegen으로 편집했으며 실제 UMG가 아님.

- 적·Player HP를 얇은 한 줄 격자로 변경하고 패널 폭을 축소함.
- Balance 다이아몬드를 눈에 띄게 축소하고 왼쪽에 밀집 배치함.
- 큰 TODO 제목·퍼센트 숫자·상단 장식선을 제거함. 미구현 칸은 빈 격자와 대시로 구분함.
- 폰트 조합은 승인 상태로 유지함. 이미지의 생성 글자는 실제 Oxanium 렌더가 아님.
- 생성 시안은 2절 치수의 정밀 도면이 아님. 특히 이번 출력의 상단 폭은 목표보다 좁아 보이며, 실제 UMG에서는 명세 치수를 시작점으로 조정함.
- 기존 프리뷰의 보조 원형 표시 일부가 이번 생성본에서 누락됨. 구현 범위에서 삭제한 것은 아니며 작은 TODO 슬롯으로 유지함.
- 밝은 배경에서 미구현 outline과 작은 글자의 대비는 UMG 단계에서 확인함.

[v2 편집 프롬프트](Assets/HUD/layout-v2-prompt.txt)

### 이전 검토본

[Concept A](Assets/HUD/hud-concept-a.png)는 built-in imagegen 생성 디자인 보드임. 실제 프로젝트 화면/UMG가 아니며 글자는 특정 폰트의 specimen이 아님.

- 검토 가능: 흰색 아이콘·청회색 비활성 슬롯·노란 Balance.
- 차이: 오른쪽 확대 패널은 설명용. 아이템 가로 배치는 제안의 세로 배치와 다름. 격자가 원작보다 성김. 수치는 예시임.
- 아이콘은 보드 내부 샘플이며 개별 투명 Texture가 아님. 승인 후 개별 생성·알파·여백·작은 크기를 검수함.
- [프로젝트 캡처 기반 배치 시안](Assets/HUD/hud-layout-project-preview.png)을 추가함. 기존 p05_combo_hit.jpg에 imagegen으로 HUD를 합성한 디자인 자료이며 최신 Stellar 캐릭터의 PIE나 실제 구현 결과가 아님. 생성 편집이므로 원본 pixel 보존을 보장하지 않음. 글자도 정확한 폰트 specimen이 아님.
- 배치 시안의 60%/72% 숫자는 설명용 예시이며 최종 필수 표시가 아님. 실제안에서는 작은 격자 밀도·아이콘 직경을 2절 기준으로 다시 맞춤.

생성일: 2026-09-17. built-in 사용, CLI 미사용. 아래는 생성 요청의 요약이며 정확한 전체 프롬프트는 함께 저장한 파일을 참조함.

> 16:9 Project Stellar HUD concept; top TARGET/grid HP/auxiliary unavailable/three balance diamonds; bottom-left BU BE HP SH with only HP filled; unavailable item/skill slots; original white slash/sweep/downstrike/shield icons; dark sci-fi arena; explicit design-preview labeling.

[생성 프롬프트](Assets/HUD/concept-prompt.txt)

[프로젝트 배치 생성 프롬프트](Assets/HUD/layout-prompt.txt)

---

## 5. 상태와 다음 확인

- [x] 참고 30개 비교·대표 확대 관찰.
- [x] HP·Balance·Target 경로 조사.
- [x] 생성 시안과 아이콘 스타일 샘플.
- [x] 폰트 후보·라이선스 출처 조사.
- [x] 실제 폰트 크기별 비교 및 샘플 한글 육안 확인 (UMG fallback 미검증).
- [x] 기존 프로젝트 캡처 기반 배치 시안 (최신 PIE 아님).
- [x] 사용자 폰트 조합 승인.
- [x] 사용자 축소 배치 v2 확인, HP/SH 행 교체 요청 반영.
- [x] Native UMG·데이터 연결·Editor 빌드·HUD 자동검사 7개 (공통 셀 보완 포함).
- [x] 1080p·1440p·3440×1440 실제 UMG 오프스크린 렌더와 한글 fallback 확인.
- [ ] 실제 PIE의 밝고 어두운 배경·Marker/Overlay 겹침·시각 최종 승인.

구현·검증 범위와 최소 PIE 체크는 [33 구현 기록](33_Gameplay_Combat_HUD_Implementation_and_Validation.md)을 참조함. 생성 시안은 계속 디자인 자료로만 취급함.

전체 구현은 [31 계획](31_Gameplay_Combat_HUD_Design_and_Implementation_Plan%20%28KR%29.md)을 따름.
