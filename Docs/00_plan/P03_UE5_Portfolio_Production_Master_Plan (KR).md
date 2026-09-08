# UE5 Portfolio - Production Master Plan

> Historical planning record: this document preserves the pre-submission 25-page plan. For the submitted 23-page source and current publishing path, see [Portfolio Production README](../07_Portfolio_Documents/Portfolio_Production/README.md).

> 목적: 넥슨 넥토리얼 게임 프로그래머 지원용 포트폴리오를 제작할 때, 페이지 범위·근거·캡처·검수 기준을 한 방향으로 관리한다.

---

## 1. 문서 역할과 운영 원칙

이 문서는 게임 기능 개발 로드맵이 아니라 **포트폴리오 제작의 상위 기준**이다.

| 문서 | 역할 |
| --- | --- |
| `P03 Production Master Plan` | 페이지 예산, 우선순위, 제작 단계, 변경 통제 |
| `Portfolio_Production/00_Page_Spec_Index` | 25페이지의 핵심 메시지·근거·시각 자료·완료 조건 |
| `98_Evidence/Portfolio_Evidence_Capture_Ledger` | 기존 증거와 신규 캡처 필요 항목의 상태 관리 |
| `Portfolio_Production/Portfolio_Review_Checklist` | PDF 제출 전 사실성·가독성·링크·개행 검수 |

상세 시스템 설명은 기존 `Sxx System Architecture`, 구현 결과는 `Pxx Pull Request`, 실제 검증 결과는 Profiling·Evidence 문서를 참조한다. 이 문서에 코드 상세나 시스템 설계 전문을 중복 기록하지 않는다.

---

## 2. 고정 범위

```text
총 분량: 25페이지
  - 표지: 1페이지
  - 인덱스: 1페이지
  - 본문: 23페이지

형식: A4 세로 Word 문서 → PDF 제출
독자: 게임 프로그래머 채용 담당자 및 실무 면접관
핵심 주장:
  복잡한 전투 상태를 C++ 책임 경계로 설계하고,
  AI·성능·디버그·Editor Tooling·개발 기록까지 검증 가능한 형태로 관리했다.
```

### 본문 우선순위

1. C++ 기반 전투 시스템 구현과 공통 실행 구조
2. AI 타깃·전투 참여의 상태 권위 분리
3. CSV Profiler 기반 원인 분석과 정량 개선
4. Debug Overlay·Editor Tooling을 통한 관찰 가능성 및 생산성 향상
5. AI 활용, GitHub, 문서화를 통한 변경 추적성과 검증 책임

---

## 3. 사전 결정 항목

아래 항목은 제작 전에 확정하거나, 확정 전에는 임시 표기로 유지한다.

| 항목 | 현재 기준 | 상태 | 결정 기준 |
| --- | --- | --- | --- |
| 프로젝트명 | `Project Stellar`를 작업 기준으로 사용 | 확정 | 표지·저장소·문서의 표기를 하나로 통일 |
| 성능 대표 수치 | 80 Enemy 1회 Before/After의 First Valid Target p95 `10.064s → 1.070s` | 범위 제한 | 자기소개서와 포트폴리오는 조건·지표를 일치시키고, 전체 FrameTime 개선·반복 재현성으로 일반화하지 않음 |
| 대표 전투 영상 | 20~30초 하이라이트 | 캡처 필요 | 콤보, 방어, 회피, 피격 또는 처형 중 3개 이상이 보이는 장면 |
| Execution 주장 범위 | 구조 구현 및 기본 흐름 | 범위 제한 | outcome별 제출용 증거가 확보되기 전에는 완전 회귀 검증 완료로 표현하지 않음 |
| Runtime LOD 주장 범위 | 측정·정책 검토 | 범위 제한 | 구현 완료·성능 개선 실적으로 표현하지 않음 |

---

## 4. 페이지 구조

| 구간 | 페이지 | 목적 |
| --- | --- | --- |
| 표지 | 1 | 프로젝트 결과·대표 성과·기술 축을 10초 안에 전달 |
| 인덱스 | 2 | Combat Core / AI & Performance / Tooling & Workflow로 탐색 제공 |
| 시스템 및 Gameplay | 3-10 | 플레이 결과와 전투 규칙, Collapse·Execution·Death의 분리된 수명주기를 제시 |
| Combat Interaction Architecture | 11-15 | Execution 전환·Intervention, 단방향 Combat Signal, Target Outcome, Execution Pair 협업을 설명 |
| Enemy AI | 16-18 | 의도 생성, 타깃 권위, 다수 참여 관리를 설명 |
| Profiling / Debug | 19-20 | 측정 기반 개선과 런타임 관찰 가능성을 증명 |
| Editor Tooling | 21-23 | 개발 생산성 도구 구현을 제시 |
| Workflow / Traceability | 24-25 | AI 활용 검증과 Git 기반 변경 추적성을 제시 |

세부 페이지 명세와 완료 조건은 [Page Spec Index](../07_Portfolio_Documents/Portfolio_Production/00_Portfolio_Page_Spec_Index (KR).md)에서 관리한다.

---

## 5. 제작 단계와 완료 게이트

| 단계 | 산출물 | 완료 게이트 |
| --- | --- | --- |
| A. 기준 확정 | 프로젝트명, 성능 대표 수치, 25페이지 제목 | 표지·본문·자기소개서 간 용어와 수치 충돌이 없음 |
| B. 증거 확보 | 게임 영상·스크린샷·프로파일링 그래프·Tooling 캡처 | Evidence Ledger의 필수 항목이 모두 `Ready` 또는 대체 근거가 있음 |
| C. 시각 자료 제작 | 단순화된 구조도·그래프·타임라인 | 한 페이지에서 1개의 중심 시각 자료만 사용, 글자 크기가 PDF에서 읽힘 |
| D. 본문 작성 | 페이지별 결론·근거·제한 사항 | 모든 주장에 코드·문서·캡처·측정 중 하나 이상의 근거가 연결됨 |
| E. Word 편집 | A4 세로 레이아웃·링크·이미지 배치 | 표·이미지·텍스트가 페이지 밖으로 넘치지 않음 |
| F. PDF 검수 | 최종 PDF, 링크 및 인쇄 확인 | Review Checklist 전체 통과 |

---

## 6. 페이지 작성 공통 규칙

모든 본문 페이지는 아래 순서를 사용한다.

```text
[섹션 번호] 제목
한 줄 결론

문제 또는 설계 기준 2~3문장

중심 시각 자료 1개
  - 게임 화면 / 구조도 / 성능 그래프 / Editor 도구 화면 중 하나

근거 2~3개
  - 책임 또는 판단 기준
  - 구현 데이터 또는 시퀀스
  - 검증 결과 또는 제한 사항

하단: 기술 키워드 · 관련 문서 또는 GitHub 링크 · 검증 범위
```

- Gameplay 페이지: 영상 프레임 + 짧은 타임라인
- 구조 페이지: 문제 → 책임 분리 → 흐름 → 효과
- 성능 페이지: 측정 조건 → 원인 → 변경 → 전후 수치
- Tooling 페이지: 기존 불편 → 도구 흐름 → 화면 → 범위 제한
- Workflow 페이지: 입력 → 판단 → 검증 → 기록

---

## 7. 변경 통제

다음 변경은 이 문서와 Page Spec Index를 함께 갱신한다.

| 변경 | 필요한 조치 |
| --- | --- |
| 페이지 추가·삭제·순서 변경 | 총 본문 22페이지 유지, 대체·축소 페이지와 이유 기록 |
| 성능 수치 변경 | 측정 조건, CSV 또는 로그, 자기소개서 표기를 함께 확인 |
| 기능 구현 상태 변경 | Evidence Ledger의 상태와 claim 범위 갱신 |
| 신규 캡처 교체 | 파일 경로, 캡처 목적, 사용 페이지 갱신 |
| 시스템 용어 변경 | 표지·인덱스·본문·문서 링크의 표기를 함께 변경 |

---

## 8. 관련 문서

- [Page Spec Index](../07_Portfolio_Documents/Portfolio_Production/00_Portfolio_Page_Spec_Index (KR).md)
- [Evidence Capture Ledger](../98_Evidence/Portfolio_Evidence_Capture_Ledger (KR).md)
- [Portfolio Review Checklist](../07_Portfolio_Documents/Portfolio_Production/Portfolio_Review_Checklist (KR).md)
- [Portfolio Document Index](../07_Portfolio_Documents/00_Portfolio_Document_Index.md)
- [Development Roadmap](P02_UE5 Portfolio_Development Roadmap (KR).md)
- Legacy A4 HTML Wireframe (archived before the submitted 23-page source; not published)
- [Submitted 23-page Portfolio HTML](../07_Portfolio_Documents/Portfolio_Production/염동섭_UE5_Portfolio_Project_Stellar.html)
- [Word Layout Handoff](../07_Portfolio_Documents/Portfolio_Production/UE5_Portfolio_Word_Layout_Handoff (KR).md)
- [Page Enrichment Backlog](../07_Portfolio_Documents/Portfolio_Production/01_Page_Enrichment_Backlog (KR).md)
- [Portfolio Capture Shot List](../07_Portfolio_Documents/Portfolio_Production/02_Portfolio_Capture_Shot_List (KR).md)
- [Portfolio Production Session Handoff](../07_Portfolio_Documents/Portfolio_Production/23_Portfolio_Production_Session_Handoff (KR).md)
- [Portfolio Claim Freeze Log](../07_Portfolio_Documents/Portfolio_Production/03_Portfolio_Claim_Freeze_Log (KR).md)
- [Page Composition Application Plan](../07_Portfolio_Documents/Portfolio_Production/04_Portfolio_Page_Composition_Application_Plan (KR).md)
- [p.7·p.8 Project Audit and Composition Plan](../07_Portfolio_Documents/Portfolio_Production/05_P07_P08_Project_Audit_and_Composition_Plan (KR).md)
