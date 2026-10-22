# Project Stellar 문서 안내

[프로젝트 소개로 돌아가기](../README.md) · [웹 포트폴리오](https://razria93.github.io/Portfolio_UE5.4_verGit/)

처음 방문했다면 대표 README의 화면과 기여 범위를 먼저 확인한 뒤, 아래에서 관심 주제를 선택하세요. 현재 구현 설명과 개발 당시의 계획·분석 기록을 구분해서 안내합니다.

## 목적에 따라 읽기

| 알고 싶은 내용 | 먼저 읽기 | 다음 단계 |
| --- | --- | --- |
| 가드 종료 중 피격 누락 | [문제·해결·코드](03_Issue_Analysis_Report/I09_Guard_Cleanup_Case_Study.md) | [검증 결과와 적용 범위](98_Evidence/Combat_Case_Studies_20261021/README.md) |
| 피격 근거 TTL 시작 시점 | [문제·해결·코드](03_Issue_Analysis_Report/I10_HitReactive_TTL_Case_Study.md) | [검증 결과와 적용 범위](98_Evidence/Combat_Case_Studies_20261021/README.md) |
| AI Perception 대상 필터링 | [문제·해결·코드](03_Issue_Analysis_Report/I11_AI_Perception_Case_Study.md) | [검증 결과와 적용 범위](98_Evidence/Combat_Case_Studies_20261021/README.md) |
| 최신 영상과 외부 공개 주소 | [14개 영상 목록](07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md) | 파일 버전·번호 변경·공개 URL 상태 |
| 공격과 피격의 실행 전환 | [공통 실행 정책 — S37](05_System_Architecture/S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md) | 문서의 코드 링크 → 증거 상태와 표현 제한 |
| 공통 타깃과 Enemy 참여 | [타깃 구조 — S33](05_System_Architecture/S33_UE5_Portfolio_System_Architecture.md) | [참여 정책 — S34](05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md) → 구현·수동 검증 항목 |
| 두 Actor의 처형 협업 | [처형 세션 — S36](05_System_Architecture/S36_UE5_Portfolio_Execution_Collaboration_Architecture.md) | 시작·취소·결과 확정 → 관련 코드와 회귀 항목 |
| 무기 장착·회전 문제 해결 | [Socket·Pivot 사례 — I04](03_Issue_Analysis_Report/I04_UE5_Portfolio_Issue_Analysis_Report.md) | [좌표계·표현 구조 — S39](05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md) → 구현·근거 목록 |
| Montage 종료 문제 해결 | [종료 설정 사례 — I06](03_Issue_Analysis_Report/I06_UE5_Portfolio_Issue_Analysis_Report.md) | [수정·감사·검증 — F08](04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md) |
| 넉백·공격 방향 보정 | [통합 검토 가이드](07_Portfolio_Documents/Portfolio_Production/37_Gameplay_Knockback_and_Facing_Review_Guide.md) | 코드 열람 지도 → 자동 검사와 사용자 PIE 범위 |
| 코드부터 살펴보기 | [README 코드 읽기 안내](../README.md#code-guide) | 관심 시스템의 구현 파일 → 해당 설계 문서 |
| 실행 환경과 확인 범위 | [로컬 실행 안내](../README.md#로컬-실행과-확인-범위) | [검증 근거 목록](98_Evidence/Stellar_Asset_Integration/README.md) |

## 주제별 전체 자료

- [현재 시스템과 설계 기록](05_System_Architecture/00_System_Architecture_Index.md): 현행 구조를 먼저 안내하고 과거 설계는 별도 목록으로 보존합니다.
- [문제 해결 사례](03_Issue_Analysis_Report/00_Issue_Analysis_Report_Index.md): 증상·원인·판단·수정·검증을 따라 읽습니다.
- [캐릭터·무기·환경 통합](07_Portfolio_Documents/Stellar_Asset_Integration/README.md): 외부 에셋과 직접 통합한 범위, 사례별 근거를 확인합니다.
- [수정과 검증 기록](04-02_Fix_Pull_Request/00_Fix_Pull_Request_Index.md): 후속 변경의 범위와 확인 결과를 확인합니다.

## 자료를 읽을 때의 기준

- 구현 존재, 자동 검사, 사용자 PIE 확인, 제출용 화면·영상 확보는 서로 다른 상태입니다. 각 문서의 기준일·커밋·한계를 함께 확인합니다.
- 웹 포트폴리오는 제출본이고, 후속 구현은 시스템 문서에 추가될 수 있습니다. 현재 구조는 시스템 인덱스의 현행 문서에서 확인합니다.
- S37의 작성 당시 전환 캡처 대기 기록과 후속 가드 통합 근거를 구분합니다. S36은 결과별 회귀·캡처 보완 상태를 명시합니다. 문서 링크가 있다는 이유로 모든 동작이 검증 완료된 것은 아닙니다.
- 영상은 [최신 목록](07_Portfolio_Documents/Portfolio_Production/Video_Catalog.md)을 기준으로 읽습니다. 기존 13개 URL은 접근 가능하지만 최신 제작본과의 내용 일치는 별도 확인 대상입니다. TTL 공개본 37초와 최신 재촬영본 38.8초를 구분합니다.
- PPT 제작 과정의 22페이지 로컬 최신본과 웹의 23페이지 HTML은 다른 산출물입니다. PPT를 갱신했다고 웹·YouTube도 갱신된 것은 아닙니다.

<details>
<summary>개발 기록·문서 유지 관리</summary>

개별 기록의 날짜와 상태를 유지합니다. 과거 계획을 현재 구현 설명으로 대신 사용하지 않습니다.

| 문서 유형 | 역할 | 목록 |
| --- | --- | --- |
| Project Plan | 프로젝트 목표·개발 순서 | [개요](00_plan/P00_UE5_Portfolio_Plan_Overview%20%28KR%29.md), [로드맵](00_plan/P02_UE5%20Portfolio_Development%20Roadmap%20%28KR%29.md) |
| Issue Checklist | 과거 작업 체크리스트 | [이전 체크리스트](99_Legacy/Issue_CheckList/00_Issue_Checklist_Index.md) |
| Work List | 브랜치별 범위와 검증 기준 | [작업 목록](01_Work_List/00_Work_List_Index.md) |
| Bug Report | 재현 조건과 원인 분석 | [버그 기록](02_Bug_Report/00_Bug_Report_Index.md) |
| Pull Request | 변경 요약과 검증 | [PR 기록](04_Pull_Request/00_Pull_Request_Index.md) |
| Portfolio Production | 제출 HTML·이미지·배포 규칙 | [제작·배포 안내](07_Portfolio_Documents/Portfolio_Production/README.md) |
| Legacy Portfolio | 이전 PF00–PF07 설계 | [이전 포트폴리오 문서](07_Portfolio_Documents/00_Portfolio_Document_Index.md) |
| Maintenance | 후속 자료 보강 | [유지 관리 목록](01_Work_List/Documentation_Maintenance_TODO.md), [캡처 상태](98_Evidence/Portfolio_Evidence_Capture_Ledger%20%28KR%29.md) |

문서 ID는 D(체크리스트), W(작업), B(버그), P(PR), S(구조), PF(포트폴리오)를 사용합니다. 방문자에게 보여 주는 링크에는 ID와 함께 내용을 알 수 있는 제목을 씁니다. GitHub Issue/PR 번호는 확인된 경우에만 연결하며 파일명이 바뀌면 관련 인덱스도 갱신합니다.

`Docs/ignore/`는 현재 문서 진입점이 아닌 과거 보관 영역입니다.

</details>
