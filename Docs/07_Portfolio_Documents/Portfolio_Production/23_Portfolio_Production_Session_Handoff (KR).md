# Portfolio Production Session Handoff

> Historical handoff: this record describes the pre-submission 25-page production plan. The current submitted source and 23-page scope are defined in [Portfolio Production README](README.md).

> 갱신일: 2026-09-06  
> 목적: 다음 작업 세션이 현재 포트폴리오의 페이지 구조, 증거 상태, 표현 제한과 미완료 작업을 짧은 시간 안에 파악하고 안전하게 이어서 작업하도록 돕는다.

---

## 1. 현재 기준선

- 대상: 넥슨 넥토리얼 게임 프로그래머 지원용 `Project Stellar` 포트폴리오
- 형식: A4 세로 HTML 와이어프레임 → PDF 또는 Word 이관
- 문서 기준 분량: **표지 1 + 인덱스 1 + 본문 23 = 총 25페이지**
- HTML 현행 분량: **25페이지**. p.15 Execution Pair Coordination 삽입과 기존 p.15–p.24의 p.16–p.25 순연을 반영했다.
- 주 산출물: `UE5_Portfolio_A4_Analytical_Wireframe.html`
- 현재 단계: 페이지 구조와 Placeholder는 작성됨. 실제 Hero·런타임 캡처는 일부 미확보 상태다. p.19의 80 Enemy Before/After 원본 CSV·로그는 Evidence 경로에 고정했으며, 반복 측정은 보완 예정이다.
- 최근 변경: p.11 Before/After 전환, p.12 정책 판단, p.13 Source 전달, p.14 Target Outcome, p.15 Pair 3-lane SVG를 HTML에 적용했다. E11–E15는 여전히 제출용 캡처 대기다.

## 2. 먼저 읽을 문서 — 순서 고정

1. [Production Master Plan](../../../00_plan/P03_UE5_Portfolio_Production_Master_Plan%20(KR).md)
2. [Page Spec Index](00_Portfolio_Page_Spec_Index%20(KR).md)
3. [Case Evidence & Page Assignment Plan](21_Portfolio_Case_Evidence_and_Page_Assignment_Plan%20(KR).md)
4. [Evidence Capture Ledger](../../../98_Evidence/Portfolio_Evidence_Capture_Ledger%20(KR).md)
5. [Claim Freeze Log](03_Portfolio_Claim_Freeze_Log%20(KR).md)
6. [Capture Shot List](02_Portfolio_Capture_Shot_List%20(KR).md)
7. [Review Checklist](Portfolio_Review_Checklist%20(KR).md)
8. 작업 대상별 `xx_Pxx_*_Project_Audit_and_Composition_Plan` 문서

작업 전에는 위 문서의 현재 페이지 번호가 HTML과 맞는지 먼저 확인한다. 코드와 문서의 내용이 다르면 현재 코드를 우선하고, 차이와 문서 수정 필요성을 보고한다.

## 3. 문서 기준 25페이지 맵

| p. | 페이지 | 주 메시지 | 대표 사례 / 상태 |
| ---: | --- | --- | --- |
| 1 | Cover | 구조 설계·측정 검증·문서 기록의 3축 | 대표 게임플레이 Hero 필요 |
| 2 | Index | 포트폴리오 읽기 순서 | 구조 확정 |
| 3 | System Map | 요청→해석→결과 반영→관찰의 관계 지도 | 구조 확정 |
| 4 | Targeting | Player 선택과 Lock 적용 | 신규 Hero / runtime capture 필요 |
| 5 | Combo / Hit | HitWindow 순서 계약 | B05, 신규 runtime capture 필요 |
| 6 | Guard / Parry | 방어 판정과 피격 반응 분리 | B12, FinalCandidate evidence ready |
| 7 | Dodge Intervention | 관계·Allow 기반 개입 | 신규 capture 필요 |
| 8 | Balance / Collapse | state·serial·timer lifecycle | 일부 evidence ready, 전이 capture 필요 |
| 9 | Execution Collaboration | Pair Transaction | 구현 구조 확인, session capture 필요 |
| 10 | Death Lifecycle | Dead 이후 Presentation·Facing 억제·Finalize/Destroy 분리 | A04 대표, A05 보조, 신규 capture 필요 |
| 11 | Execution Transition | 공통 전환 계약과 terminal cleanup | E11a/b, B10, 신규 capture 필요 |
| 12 | Intervention Policy | Want·Allow·Window 정책 | B09, E12 capture 필요 |
| 13 | Combat Signal Source Boundary | Source 사실 전달과 Target 비조작 | E13 capture 필요 |
| 14 | Target Outcome Resolution | Packet·Target State 기반 Normal/Guard/Parry 판정 | E14 capture 필요 |
| 15 | Execution Pair Coordination | reservation·Active·Commit·Release 협업 거래 | E15a/b/c capture 필요 |
| 16 | AI Intent | Blackboard→Intent→Task 분리 | B03 |
| 17 | Combat Target | SoT·Revision·lifetime | dedicated capture 필요 |
| 18 | Participation | evidence 기반 role assignment | 3 Enemy capture 필요 |
| 19 | Profiling | 동일 조건의 후보 구성·유효 대상 확정 지연 분석 | 80 Enemy 원본 CSV·로그 Ready / 3회 반복 측정 보완 예정 |
| 20 | Runtime Debug | Focus된 Enemy의 상태·이력 read-only 대조 | Current AI 보조 증거 Ready / 통합 실사용 화면 필요 |
| 21 | Overlay Editor Plugin | Editor↔PIE command bridge | evidence ready, 성능 개선 주장 금지 |
| 22 | Asset Inspector | 선택 조건 기반의 안전한 참조 조사 | Panel UI Ready / Selected Asset·Tree·CSV 실사용 캡처 필요 |
| 23 | Root Motion Tool | raw track Apply/Revert safety contract | 동일 복제 Asset Before·Apply·Revert 3분할 capture 필요 |
| 24 | AI Workflow | AI 초안을 개발자 판단·검증 기록으로 통제 | 동일 Work ID Brief·Review·Verification 문서 보드 및 다이어그램 필요 |
| 25 | Change Traceability | B05 증상·D13 선행 기준·P12·과거 patch·현재 Hit Window 순서 계약 대조 | 사용자 제작 관계 다이어그램 + 현행 first-overlap capture 필요 |

## 4. 작업 원칙과 표현 제한

1. 사용자가 승인한 25페이지와 본문 23페이지 예산을 임의로 늘리거나 줄이지 않는다.
2. 한 페이지는 하나의 핵심 메시지와 하나의 중심 시각 자료만 갖는다. 보조 영역은 중심 주장을 반복하지 않고 증거·사례·제한을 보강한다.
3. 실제 증거가 없는 결과는 `Needs Capture`, Placeholder 또는 `구현 범위`로 표시한다. 완료한 검증처럼 쓰지 않는다.
4. 게임플레이 Hero와 Debug Overlay 검증 화면을 한 프레임에 섞지 않는다.
5. `Project Stellar` 표기를 사용한다. `Project Stella`는 사용하지 않는다.
6. p.10 Death는 Combat Target 삭제 자체가 아니라 **Target 보관과 Facing 입력 소비의 분리**를 다룬다.
7. p.19 Profiling은 80 Enemy 1회 Before/After의 후보 수·유효 대상 확정 지연만 표현한다. 전체 FrameTime 개선 또는 반복 재현성으로 일반화하지 않는다.
8. p.21 Overlay Editor Plugin은 Editor-only CVar/PIE bridge다. 현재 코드에 없는 CVar cache 성능 개선이나 runtime 소유권을 주장하지 않는다.
9. p.22 Asset Inspector의 Unused Candidate는 삭제 판정이 아니다.
10. p.23 Root Motion은 capture 전 visual preservation 결과가 아니라 Apply/Revert 안전 계약으로만 표현한다.

## 5. 다음 작업의 우선순위

| 우선순위 | 작업 | 완료 조건 |
| ---: | --- | --- |
| P1 | p.19 Profiling 3회 반복 측정 | 80 Enemy 동일 조건의 평균·분산을 확보해 재현성 주장 여부를 판단 |
| P0 | p.1, p.4~p.10 Hero 캡처 확보 | HUD·Debug Overlay 없는 게임플레이 프레임 확보 |
| P1 | p.10 Death lifecycle crop 확보 | Dead Entry / Facing Suppressed / Finalize route 3개 확보 |
| P1 | p.11~p.15 runtime crop 확보 | E11~E15의 상태·이벤트·결과가 읽히는 캡처 확보 |
| P1 | p.5, p.7, p.17~p.18 runtime crop 확보 | 대표 사례별 상태·이벤트·결과가 읽히는 캡처 확보 |
| P2 | p.23~p.25 artifact와 runtime evidence 연결 | Root Motion / Workflow / Traceability의 주장 범위를 완성 |

## 6. 문서 동기화 규칙

아래 영향이 있으면 반드시 함께 갱신한다.

| 변경 | 함께 갱신할 문서 |
| --- | --- |
| 페이지 추가·삭제·순서 변경 | Master Plan, Page Spec Index, HTML Index/footer, Word Handoff, Review Checklist |
| Placeholder를 실제 캡처로 교체 | Evidence Ledger, Capture Shot List, Claim Freeze Log, 해당 페이지의 Audit Plan |
| 성능 수치 또는 조건 변경 | Profiling 문서, Evidence Ledger, Claim Freeze Log, 자기소개서 문구 |
| 대표 사례 변경 | Case Evidence & Page Assignment Plan, Page Spec Index, 해당 페이지 HTML |
| 주장 또는 용어 변경 | HTML, Page Spec Index, Claim Freeze Log, 관련 Architecture / Audit Plan |

## 7. 다음 세션의 권장 작업 순서

1. 이 문서와 2절의 필수 문서를 읽는다.
2. 요청받은 페이지의 HTML, Page Spec, Audit Plan, Evidence Ledger 행을 대조한다.
3. 코드·문서·Git 이력으로 실제 구현 범위와 대표 사례를 확인한다.
4. 페이지가 다루어야 할 중심 메시지·중심 시각 자료·보조 증거·표현 제한을 먼저 제안한다.
5. 승인 후 HTML과 연결 문서를 수정한다.
6. HTML 섹션 수, footer, index, page number, PDF 렌더링 가능 여부를 점검한다.
7. 완료 시 작업 결과, 변경 문서, 증거 상태, 남은 보완 항목을 짧게 보고한다.

## 8. 직전 작업 기록

- p.11–p.15에 Before/After, 정책 판단, Source 전달, Target Outcome, Pair 3-lane SVG를 반영했다.
- HTML의 Index, footer denominator, 이후 eyebrow 및 페이지 번호는 25페이지 기준이다.
- Master Plan, Page Spec, Evidence Ledger, Capture Shot List, Claim Freeze Log, Word Handoff, Review Checklist를 25페이지 기준으로 동기화했다.
- Chrome headless PDF 렌더링에서 페이지 객체 25개를 확인했다. HTML page section과 `h1`도 각각 25개다.
- p.11–p.15의 두 번째 섹션 제목은 모두 `런타임 검증`이며 E11–E15 슬롯은 `Needs Capture`로 표시했다.

## 9. 작업 결과 보고 형식

```text
작업 결과
- [무엇을 변경했는지]

변경 문서 / 파일
- [파일과 핵심 변경]

증거 상태
- Ready / Needs Capture / Blocked와 그 이유

남은 보완
- [다음 작업에서 필요한 선택 또는 캡처]
```

## 10. 다음 세션 시작 프롬프트

```text
[PORTFOLIO PRODUCTION CONTEXT]

현재 작업은 넥슨 넥토리얼 게임 프로그래머 지원용 `Project Stellar` 포트폴리오 제작의 일부다.
문서 기준 산출물은 A4 세로 25페이지 전환안이며, 표지 1페이지·인덱스 1페이지·본문 23페이지다. HTML은 신규 p.15 삽입 전까지 24페이지를 유지한다.

작업 전 반드시 아래 문서를 순서대로 읽어라.

1. Docs/07_Portfolio_Documents/Portfolio_Production/23_Portfolio_Production_Session_Handoff (KR).md
2. Docs/00_plan/P03_UE5_Portfolio_Production_Master_Plan (KR).md
3. Docs/07_Portfolio_Documents/Portfolio_Production/00_Portfolio_Page_Spec_Index (KR).md
4. Docs/07_Portfolio_Documents/Portfolio_Production/21_Portfolio_Case_Evidence_and_Page_Assignment_Plan (KR).md
5. Docs/98_Evidence/Portfolio_Evidence_Capture_Ledger (KR).md
6. Docs/07_Portfolio_Documents/Portfolio_Production/03_Portfolio_Claim_Freeze_Log (KR).md
7. Docs/07_Portfolio_Documents/Portfolio_Production/Portfolio_Review_Checklist (KR).md
8. 현재 작업 대상에 해당하는 Project Audit and Composition Plan 문서

필수 원칙:

1. 승인된 25페이지 예산을 임의로 바꾸지 않는다.
2. 각 페이지는 핵심 메시지 하나와 중심 시각 자료 하나를 유지한다.
3. 코드·문서·캡처·측정값으로 확인 가능한 주장만 쓴다. 미확보 증거는 Placeholder 또는 구현 범위·제한으로 명시한다.
4. 현재 코드와 기존 문서가 다르면 코드를 우선하고, 차이와 문서 수정 필요성을 보고한다.
5. Hero 게임플레이 화면과 Debug Overlay 검증 화면을 섞지 않는다.
6. `Project Stellar` 표기를 사용한다.
7. 페이지 구성·주장 범위·성능 수치·캡처 파일·검수 기준에 영향이 있으면 관련 기준 문서를 함께 갱신한다.
8. 기존 게임 개발 로드맵과 포트폴리오 제작 계획을 혼동하지 않는다.

작업 순서:

1. 요청한 페이지 또는 증거의 현재 HTML·Page Spec·Audit Plan·Evidence Ledger 상태를 요약한다.
2. 필요 시 현재 코드·문서·Git 이력으로 구현과 대표 사례를 검증한다.
3. 중심 메시지, 중심 시각 자료, 보조 증거, 표현 제한, 문서 갱신 범위를 먼저 제안한다.
4. 승인되었거나 사용자가 바로 구현을 요청했으면 HTML과 관련 문서를 수정한다.
5. HTML page section 수, index·footer·page number, PDF 렌더링 가능 여부를 확인한다.
6. 완료 후 작업 결과, 변경 문서, 증거 상태, 남은 보완 항목을 짧게 보고한다.

[작업 요청]

여기에 다음 작업을 구체적으로 작성한다.
```
