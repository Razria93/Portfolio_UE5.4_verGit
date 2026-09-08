# UE5 Portfolio Word Layout Handoff

> Historical handoff: this file preserves the pre-submission 25-page Word transfer specification. For the current 23-page submitted source, start from [Portfolio Production README](README.md).

## 1. 목적과 공통 설정

이 문서는 문서상 25페이지 포트폴리오를 Word A4 세로 문서로 옮길 때의 표 기반 제작 명세다. 현행 `UE5_Portfolio_A4_Analytical_Wireframe.html`은 전환 전 24페이지 시안이며, 신규 p.15 Execution Pair Coordination과 이후 페이지 순연은 HTML 전환 단계에서 반영한다. HTML은 웹사이트가 아니라, 페이지 구성과 정보 밀도를 확인하기 위한 문서 시안이다.

- 용지: A4 세로
- 여백: 상하좌우 14mm
- 본문 사용 영역: 약 182mm × 269mm
- 권장 글꼴: 맑은 고딕 또는 본문 가독성이 같은 고딕 계열
- 제목: 20~24pt, 굵게 / 본문: 9.5~10.5pt / 캡션·푸터: 8~8.5pt
- 색상: 검정·진회색·연회색을 기본으로 사용한다. 증거 상태의 실제 이미지 색상만 보존하며, p.1 대표 근거 셀의 연파란색만 예외다.
- 표 테두리: 외곽선 또는 필요한 구분선만 0.5pt 회청색. 모든 셀에 테두리를 넣지 않는다.
- 셀 안쪽 여백: 기본 상하 0.18cm, 좌우 0.25cm. 제목 영역은 상하 0.28cm.
- 단락: 본문 줄간격 1.25~1.35, 문단 뒤 3~5pt. 셀 안에서는 빈 줄 대신 문단 간격을 사용한다.

## 2. Word 표 제작 공통 규칙

1. 페이지마다 바깥 표 하나를 만들고, 행 높이는 고정 대신 최소 높이로 둔다. 이미지 영역만 권장 높이를 적용한다.
2. 바깥 표의 테두리는 없음으로 두고, 헤더 하단과 푸터 상단에는 0.5pt 선을 적용한다.
3. 이미지·캡처 자리에는 도형보다 표 셀을 사용한다. 연한 회청색 채우기와 점선 외곽선을 적용하고, 셀 안에 캡처 목적·권장 비율·증거 상태를 중앙 정렬로 적는다.
4. 좌우 분할 영역은 바깥 표 안에 중첩 표를 넣는다. Word에서 이미지가 밀려도 다른 페이지로 흐르지 않도록 이미지 셀에는 문단 나누기 금지를 적용한다.
5. 헤더·푸터는 문서 기능 대신 페이지 표 안에 넣는다. PDF 변환 시 섹션별 정보가 고정되어 보이고, 개별 페이지 수정이 쉬워진다.
6. 실제 이미지 삽입 후에는 이미지 자르기 비율만 조정한다. 표 행의 높이와 중심 메시지는 유지한다.

## 3. HTML 블록 → Word 요소 대응

| HTML 블록 | Word에서의 구현 | 핵심 설정 |
| --- | --- | --- |
| page | 페이지당 바깥 표 1개 | 1열, 페이지 유형에 맞는 행 구성 |
| page-head | 바깥 표 첫 행의 2열 중첩 표 | 좌 74% / 우 26%, 우측 정보 위쪽·오른쪽 정렬 |
| content | 바깥 표 가운데 행들 | 행 사이 간격은 빈 행 대신 문단 뒤 간격으로 확보 |
| placeholder | 채우기 셀 또는 1×1 중첩 표 | 연한 회청색, 점선 0.5pt, 세로·가로 가운데 정렬 |
| grid-2 | 2열 중첩 표 | 50% / 50%, 셀 사이 0.35cm |
| grid-58-42 | 2열 중첩 표 | 58% / 42%, 셀 사이 0.35cm |
| flow | 1행 다열 중첩 표 | 단계 4~5칸, 연결 화살표는 별도 좁은 셀 또는 텍스트 |
| metric | 1×1 강조 셀 | 좌측 2.25pt 초록 테두리, 연한 초록 채우기 |
| footer | 바깥 표 마지막 행의 2열 중첩 표 | 좌 55% / 우 45%, 8pt, 우측 오른쪽 정렬 |

## 4. 페이지 유형별 표 구조

### A. p.1 Cover

바깥 표: 1열 6행. 1행과 5행에는 내부 2열 표를 사용한다.

| 행 | 권장 최소 높이 | 내용 | 표 구조 |
| --- | ---: | --- | --- |
| 1 | 3.0cm | 문서 유형, 제목, 이름·저장소 / 1인 개발·기간·플랫폼 | 74% / 26% |
| 2 | 1.8cm | 프로젝트 소개 2~3문장 | 1열 |
| 3 | 7.4cm | 대표 전투 게임플레이 이미지 | 1열, 16:9 |
| 4 | 3.5cm | 실행 흐름 / 성능 카드 | 58% / 42% |
| 5 | 3.6cm | 단순화 다이어그램 / Debug Overlay 캡처 | 50% / 50% |
| 6 | 0.9cm | 기술 키워드 / GitHub·문서·AI Workflow | 55% / 45% |

표지에서 Hero 이미지는 Debug Overlay가 없는 깨끗한 게임플레이 화면만 사용한다. 성능 카드의 수치는 Profiling Evidence가 확정되기 전까지 대표 성능 수치 확정 대기로 둔다.

### B. p.2 Index

바깥 표: 1열 4행.

- 1행 2.8cm: Header 2열
- 2행 1.1cm: 한 줄 결론
- 3행 15.5cm: 좌우 2열 목차 + 하단 읽기 흐름 Placeholder
- 4행 0.9cm: Footer

목차는 2열, 각 섹션은 가는 하단선으로만 구분한다. 행마다 박스를 만들지 않는다.

### C. p.3 System Map

바깥 표: 1열 5행.

- Header 2.8cm
- 결론·설명 2.0cm
- 시스템 맵 Placeholder 10.5cm, 권장 4:3
- 설계 기준 / 읽는 방법 2열 4.2cm
- 키워드·푸터 1.2cm

### D. p.4, p.5, p.7 Gameplay Hero

대상: Targeting, Combo / Hit, Dodge Intervention.

바깥 표: 1열 5행.

- Header 2.8cm
- 결론·설명 1.8cm
- 중심 시각 자료 8.5~9.5cm. p.4는 16:9 프레임, p.5는 타임라인, p.7은 상태 전이 다이어그램.
- 문제·구현 포인트 또는 이미지·설명 2열 5.0cm
- 근거·키워드·푸터 2.4cm

세 페이지의 구조는 비슷하지만 중심 자료의 형식이 달라야 한다. 게임플레이 프레임, 타임라인, 상태 전이를 같은 모양의 박스로 반복하지 않는다.

### E. p.6, p.8, p.9 Gameplay Compare / Lifecycle

대상: Guard / Parry, Balance / Collapse, Execution Collaboration.

바깥 표: 1열 5행.

- Header 2.8cm
- 결론·설명 1.8cm
- 비교표 또는 상태 전이 5.0~6.0cm
- 중심 게임플레이·상태 캡처 7.5cm
- 설명 카드·푸터 3.0cm

p.8은 단일 Actor의 Balance / Collapse 상태 수명주기만 다룬다. p.9는 두 Actor Pair의 **게임플레이 결과와 런타임 데이터**를 보여 주며 큰 구조 다이어그램을 넣지 않는다. Pair reservation·commit·release 시퀀스는 p.15에서 다룬다.

### F. p.10 Death Lifecycle

바깥 표: 1열 5행.

- Header 2.8cm
- 결론·수명주기 기준 2.0cm
- Dead Presentation Hero + 4단계 lifecycle 8.0~9.0cm, 58% / 42%
- Dead Entry / Facing Suppressed / Finalize route 3열 4.5cm
- 문제 해결 경험·증거 제한·푸터 3.0cm

Hero는 Debug Overlay가 없는 Dead Presentation 프레임으로 두고, 런타임 값은 별도 crop에서만 보인다. Dead 상태의 Combat Target 삭제를 단정하지 않고, Target 보관과 Facing 소비의 분리를 설명한다.

### G. p.11~p.18 Structure / AI

대상: Execution Transition, Intervention Policy, Combat Signal Source Boundary, Target Outcome Resolution, Execution Pair Coordination, AI Intent, Combat Target, Participation.

바깥 표: 1열 5행.

- Header 2.8cm
- 결론·문제 기준 2.0cm
- 단순 흐름도 또는 계층도 8.5~10.0cm
- 문제 / 책임 분리 / 효과 중 2개 카드 4.5cm
- 관련 근거·푸터 1.4cm

중심 도식은 클래스 다이어그램을 그대로 옮기지 않는다. 문제 → 책임 분리 → 단순 흐름 → 효과를 한 페이지 안에서 읽을 수 있어야 한다. 도식의 상자 수는 7~9개를 넘기지 않는다.

### H. p.19 Profiling

바깥 표: 1열 5행.

- Header 2.8cm
- 결론·측정 원칙 2.0cm
- CSV 전후 비교 그래프 8.5cm, 권장 16:9
- 원인 / 검증 조건 2열 4.5cm
- 제한 사항·푸터 2.0cm

수치 카드, 그래프, 설명은 같은 측정 조건을 가리켜야 한다. 확정 전에는 수치 대신 확정 대기를 표기한다.

### I. p.20 Runtime Debug 및 p.21~p.23 Tooling

바깥 표: 1열 5행.

- Header 2.8cm
- 한 줄 결론 1.7cm
- 주요 캡처 8.0~9.0cm. Runtime Debug는 4:3, Editor 도구는 UI가 읽히는 4:3을 권장.
- 입력/출력 또는 기존 불편/만든 도구 2열 4.5cm
- 범위 제한·푸터 1.8cm

p.20은 Runtime 디버그가 shipping UI가 아닌 read-only 개발 도구임을 푸터와 본문에서 모두 표시한다. p.21~23은 Editor-only라는 범위를 명시한다.

### J. p.24 AI Workflow 및 p.25 Traceability

바깥 표: 1열 5행.

- Header 2.8cm
- 한 줄 결론 1.8cm
- 단계 흐름 3.0cm
- 증거 보드 또는 추적 흐름 8.0cm
- 사람의 판단 책임·링크·푸터 3.0cm

p.24는 AI의 자동 생성 결과가 아니라 Work Brief → 계획 → 구현 → 검증 → 기록의 사람 중심 절차를 보여준다. p.25는 문서 수 나열이 아니라 Issue → 설계 → 구현 → 검증 → PR의 한 변경 흐름을 보여준다.

## 5. Placeholder 작성 규칙

Placeholder 문구는 다음 순서로 작성한다.

1. 필요한 증거 이름
2. 권장 비율
3. 상태 또는 촬영·재작성 방식
4. 캡처에서 반드시 보여야 할 판단 포인트

예시:

- 대표 전투 게임플레이 영상 프레임 · 16:9 · 신규 캡처
- CSV 전후 비교 그래프 · 수치 확정 대기
- Action·Reaction 단순화 다이어그램 · 포트폴리오용 재작성
- Debug Overlay FinalCandidate 캡처 · 4:3 · 개발용 read-only 상태 표시

## 6. Word 이관 순서와 최종 점검

1. 25페이지 전환안의 25개 페이지를 Word에서 동일한 순서로 빈 바깥 표로 먼저 만든다. HTML을 기준으로 이관한다면 p.15 삽입과 이후 번호 순연을 먼저 반영한다.
2. 제목·결론·본문을 먼저 이관하고, Placeholder는 실제 증거가 준비될 때까지 유지한다.
3. 각 페이지의 중심 시각 자료를 교체한 뒤, 표 행 높이가 변하지 않는지 확인한다.
4. PDF로 저장해 페이지 수가 25페이지인지, 제목·푸터가 밀리지 않는지 확인한다.
5. Evidence Ledger와 대조해 수치, 구현 범위, 검증 상태를 마지막으로 검수한다.

이관 시 판단 기준은 Portfolio Review Checklist와 Portfolio Evidence Capture Ledger를 우선한다.
