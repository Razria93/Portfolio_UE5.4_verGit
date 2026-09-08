# Portfolio Review Checklist

> Historical checklist: completed checks below refer to the pre-submission 25-page draft. The current submitted source and post-submission checks are tracked from [Portfolio Production README](README.md) and [Documentation Maintenance TODO](../../01_Work_List/Documentation_Maintenance_TODO.md).

> 사용 시점: Word 초안 완료 후, PDF 변환 전·후에 반복 검수한다.

## 1. 사실성과 범위

- [ ] 모든 구현 주장은 현재 코드·문서·캡처 중 하나 이상의 근거와 연결된다.
- [x] Project Stellar 표기를 포트폴리오 표지·제작 문서에서 사용한다.
- [ ] 성능 수치의 조건, 단위, 전후 비교 기준이 자기소개서와 일치한다.
- [ ] Runtime LOD, Execution, Root Motion 등 검증 범위가 부족한 항목은 제한 사항을 함께 적는다.
- [ ] Editor Plugin과 Debug Overlay를 runtime 또는 shipping 기능으로 과장하지 않는다.
- [ ] Asset Inspector의 Unused Candidate를 삭제 가능 판정으로 표현하지 않는다.
- [ ] 서버·네트워크·멀티스레드 등 저장소 근거가 없는 경험을 넣지 않는다.

## 2. 페이지 메시지

- [ ] 각 페이지는 제목 아래 한 줄 결론을 가진다.
- [ ] 각 페이지에는 중심 시각 자료가 하나만 있다.
- [ ] 기능 나열보다 문제·판단·결과가 먼저 보인다.
- [ ] 같은 구조도, 같은 캡처, 같은 수치를 여러 페이지에서 반복하지 않는다.
- [ ] 코드 캡처는 필요한 경우에만 사용하고 15줄을 넘기지 않는다.

## 3. 시각 자료와 Word 레이아웃

- [ ] 모든 페이지가 A4 세로, 같은 여백, 같은 제목 체계를 사용한다.
- [x] p.3~p.25의 eyebrow, Index, footer, Audit Plan을 25페이지 기준으로 순연했다. PDF 페이지 객체 25개를 확인했다.
- [x] 본문 section header에는 우측 설명 텍스트가 없고, Gameplay 페이지 및 구조 페이지 p.11–p.15의 둘째 영역 제목은 `런타임 검증`으로 통일됐다. 구조 페이지 p.3은 `전투 대상 관리 흐름`을 유지한다.
- [ ] p.1의 `PORTFOLIO FOCUS`는 일반 section header·본문 형식이며, 우측 기준 문구와 대표 근거의 연파란색만 예외로 유지한다.
- [ ] 구조 페이지의 중심 시각 자료는 고정된 SVG 다이어그램이며, 작은 HTML 카드·화살표 묶음으로 대체하지 않는다.
- [ ] 본문 UI의 강조색은 검정·회색으로 통일되고, 증거 이미지 자체의 색상은 보존한다.
- [ ] 표는 배치 고정용으로만 사용하고, 외곽선·음영이 과도하지 않다.
- [ ] 이미지가 찌그러지지 않고, PDF 100% 확대에서 텍스트가 읽힌다.
- [ ] 영상은 GIF 대신 대표 썸네일과 클릭 가능한 링크로 제공한다.
- [ ] 성능 그래프의 축·단위·측정 조건이 표시된다.
- [ ] 표지에는 대표 게임 화면, 핵심 구조, 성능 수치, 디버그 또는 Tooling 화면이 균형 있게 배치된다.

## 4. 링크와 제출본

- [ ] GitHub Repository 링크가 PDF에서도 열리는지 확인한다.
- [ ] 영상 링크가 로그인 없이 재생되는지 확인한다.
- [x] Analytical A4 Wireframe을 PDF로 변환해 페이지 객체가 정확히 25개인지 확인했다.
- [ ] 파일명, 메타데이터, 연락처, GitHub 계정이 최신인지 확인한다.
- [ ] 다른 PC 또는 휴대폰에서 PDF를 열어 가독성과 링크를 재확인한다.
