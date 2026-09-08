# Portfolio Visual Grammar Refactor Plan

> 목적: `Project Stellar` 포트폴리오의 페이지 수와 사실 주장 범위를 유지하면서, 본문을 **메인 시각화 및 흐름 → 런타임 검증 → 문제 해결·설계 판단·구현 범위**로 읽히는 A4 양식으로 재구성한다.
>
> 상태: **계획 확정 전 초안**

---

## 1. 범위와 변경 원칙

- 현행 분량은 표지 1 + Index 1 + 본문 22, 총 **24페이지**로 유지한다.
- 코드·문서·캡처·측정값으로 확인 가능한 주장만 유지한다. 이 계획은 주장 범위를 넓히지 않는다.
- Hero 게임플레이, Debug Overlay, Editor 캡처는 각각 목적이 다른 증거다. 하나의 프레임에 섞지 않는다.
- 구조 페이지의 중심 시각 자료는 스크린샷이 아니라 읽기 경로가 설계된 다이어그램일 수 있다.
- 기존 HTML의 작은 카드와 화살표 조합을 플로우차트 대체물로 사용하지 않는다.

### 1.1 우선 적용 대상

| 구분 | 페이지 | 이번 리팩터링의 역할 |
| --- | --- | --- |
| 공통 양식 | p.3~p.24 | 번호, 섹션 헤더, 색상 토큰, 고정 영역 규칙 |
| 파일럿 | p.6 Guard / Parry | 결과 이미지 3장을 검증 필드 목업으로 전환 |
| 파일럿 | p.8 Balance / Collapse | Gameplay Hero 슬롯 + 고정 `런타임 검증` + 하단 제목 축약 |
| 파일럿 | p.9 Execution Collaboration | Gameplay Hero 슬롯 + `문제 해결 경험 | HP 1 Standard Execution 사전 거절` |
| 구조 페이지 | p.11~p.18 | SVG 다이어그램 중심의 양식으로 전환 |
| 도구·기록 페이지 | p.19~p.24 | 실제 증거 캡처 또는 SVG를 중심 시각 자료로 재배치 |

---

## 2. 확인된 현행 문제

### 2.1 번호 기준이 두 개다

HTML footer는 전체 24페이지 기준을 사용하지만, p.3~p.24의 eyebrow는 본문 기준 `01~22`를 사용한다. 예를 들어 실제 p.11 `Shared Execution`은 `09`, 실제 p.24 `Traceability`는 `22`로 표기된다. Index·footer·HTML 제목·Audit Plan이 하나의 페이지 번호를 사용해야 한다.

### 2.2 우측 섹션 설명이 제목의 가로 공간을 침범한다

현행 `.section-head`는 좌측 `h2`와 우측 `span`을 flex로 배치한다. 이 구조는 `문제 해결 경험 — ...` 같은 제목을 두 줄로 밀고, 페이지별 보조 문구가 시각적 우선순위를 불필요하게 높인다.

### 2.3 런타임 검증 영역의 제목이 제각각이다

`상태 전이 런타임 검증`, `Pair Session 런타임 검증`, `결과 데이터 검증` 등으로 분산되어 있다. 검증 자료의 목적은 같으므로 둘째 영역 제목을 고정해야 한다.

### 2.4 강조색이 구조 의미를 만들지 못한다

파랑·초록의 배경·상단선·화살표가 여러 페이지에서 반복되지만, 색의 의미가 페이지마다 일관되지 않다. 시선은 분산되는데 정보 계층은 강화하지 못한다.

### 2.5 p.6 Guard / Parry의 FinalCandidate 이미지가 페이지 목적에 맞지 않는다

현행 HTML은 Block Hit·Parry·Player Hit FinalCandidate 이미지 3장을 직접 표시한다. 이 영역은 게임플레이 Hero도 아니고, 화면 자체를 읽어야 하는 검증 자료도 아니다. 결과 필드 비교가 목적이므로 이미지 대신 명시적 목업을 사용한다.

### 2.6 p.11~p.24 Audit Plan은 이전 페이지 체계를 사용한다

`06_P09...`부터 `19_P22...`까지의 문서는 “현행 23페이지” 및 한 페이지 앞선 번호를 기록한다. p.10 Death Lifecycle 독립 후 현행 24페이지와 맞지 않는다.

---

## 3. 공통 페이지 문법

### 3.1 번호 표기

- 표지와 Index를 제외한 모든 eyebrow는 **전체 페이지 번호**를 사용한다.
- p.3은 `03 · SYSTEM MAP`, p.11은 `11 · SHARED EXECUTION`, p.24는 `24 · TRACEABILITY`로 표기한다.
- footer의 자동 카운터, Index, Page Spec, Audit Plan 및 Word Handoff도 이 번호를 기준으로 동기화한다.

### 3.2 세 영역

본문은 제목·한 줄 결론 아래에서 다음 순서로 구성한다.

| 순서 | 고정 역할 | 제목 규칙 | 허용 자료 |
| ---: | --- | --- | --- |
| 1 | 메인 시각화 및 흐름 | 페이지별 핵심 질문을 짧게 표기 | SVG 다이어그램, Hero, Editor 캡처 중 하나 |
| 2 | 런타임 검증 | 항상 `런타임 검증` | Overlay·Event Log·검증 필드·측정 조건·Needs Capture 목업 |
| 3 | 판단의 기록 | 사실 성격에 맞는 접두어와 `|` | 문제 해결, 설계 판단, 구현 범위, 추적 범위 |

셋째 영역의 제목은 다음 중 하나만 사용한다.

```text
문제 해결 경험 | [실제 오류·충돌·수정 사례]
설계 판단 | [책임 또는 정책을 분리한 이유]
구현 범위 | [증거 미확보 또는 검증 제한]
추적 범위 | [문서·코드·검증 기록의 대조 대상]
```

`문제 해결 경험`은 Bxx·실제 수정 기록 또는 확인 가능한 오류가 있을 때만 사용한다. 모든 구조 설명에 강제하지 않는다.

### 3.3 섹션 헤더

- `.section-head`는 제목과 하단 구분선만 남긴다.
- HTML의 모든 `.section-head > span`은 삭제한다. CSS로 숨겨 출처 텍스트를 남기지 않는다.
- 기존 우측 설명은 필요한 경우 시각화 내부의 범례, 이미지 caption, 하단 목업의 필드명으로 이동한다.
- 제목은 A4에서 한 줄로 유지한다. 긴 제목은 보조 문구를 삭제하거나 핵심 명사구로 줄인다.

### 3.4 공간 예산

각 본문 페이지는 A4 인쇄 영역에서 다음 우선순위를 가진다.

1. 메인 시각화는 페이지 중 가장 큰 고정 캔버스를 차지한다.
2. `런타임 검증`은 2~3개의 읽을 수 있는 증거 또는 목업만 둔다.
3. 하단 판단 영역은 문제·해결·효과 또는 제한을 한 줄씩 읽을 수 있는 높이로 제한한다.
4. 카드 수를 늘려 설명량을 해결하지 않는다. 한 영역이 넘치면 텍스트를 줄이거나 중심 시각화에 통합한다.

---

## 4. 시각화 제작과 배치

### 4.1 구조 페이지의 SVG 원칙

- Mermaid 또는 별도 플로우차트 도구로 원본을 제작한다.
- 원본 파일과 내보낸 SVG를 함께 보관한다.
- HTML은 `visualization-stage` 내부에 SVG 하나를 배치하며, 페이지별 CSS 카드·화살표로 노드를 재구성하지 않는다.
- SVG는 고정 viewBox, 인쇄 가능한 글자 크기, 단방향 읽기 흐름, 한 개의 핵심 질문을 가진다.
- 노드 수는 페이지당 핵심 흐름이 한 번에 읽히는 범위로 제한한다. 보조 조건은 하단 검증 또는 판단 영역으로 보낸다.

### 4.2 페이지별 중심 다이어그램

| 페이지 | 메인 시각화의 질문 | 권장 SVG 유형 |
| ---: | --- | --- |
| 8 | Parry 누적과 Collapse 상태 변화가 실제 플레이에서 어떻게 보이는가 | Debug Overlay 없는 Gameplay Hero |
| 9 | 두 Actor의 Execution Pair가 실제 플레이에서 어떻게 시작·진행되는가 | Debug Overlay 없는 Gameplay Hero |
| 11 | Action과 Reaction이 무엇을 공유하고 어디서 분리되는가 | two-lane convergence / split |
| 12 | 요청은 어떤 순서로 Accept·Relationship·ApplyMode를 결정하는가 | decision tree |
| 13 | Hit 정보는 어떻게 결과 packet과 Consumer까지 보존되는가 | pipeline |
| 14 | Key·Data·Context는 어디서 해석되고 실행으로 전달되는가 | resolve flow |
| 15 | Blackboard 문맥은 어떻게 Intent와 Task 요청으로 바뀌는가 | intent flow |
| 16 | 누가 Target 상태를 요청하고 누가 같은 값을 소비하는가 | authority map |
| 17 | live Evidence가 어떻게 Participation Role이 되는가 | aggregation flow |
| 18 | 성능 수치가 언제 제출 가능한 대표값이 되는가 | evidence gate |
| 22 | Apply와 Revert가 어떤 안전 계약으로 분리되는가 | safety contract flow |
| 23 | AI 초안은 어떤 검토·검증 뒤에 반영되는가 | workflow flow |
| 24 | B05 한 변경을 어떤 기록으로 다시 대조하는가 | traceability timeline |

p.19~p.21은 Evidence Ledger에서 Ready인 실제 캡처를 메인 시각화로 우선 사용한다. 흐름도는 그 캡처를 해석하는 작은 보조 자료로 제한한다.

### 4.3 단색 스타일

- 본문 UI는 검정·짙은 회색·옅은 회색·흰색만 사용한다.
- 파랑·초록의 상단선, 배경, 화살표, 강조 텍스트는 제거한다.
- 강조는 검정 테두리 두께, 명도 차이, 굵기, 여백으로 표현한다.
- 실제 게임·Editor 캡처 안의 색은 증거 이미지의 일부이므로 변경하지 않는다.

---

## 5. p.6 Guard / Parry 전환

### 5.1 변경

- `debug_overlay_p1_final_block_hit.png`, `debug_overlay_p1_final_parry.png`, `debug_overlay_p1_final_player_hit.png`의 직접 `<img>` 배치를 제거한다.
- `런타임 검증`에는 세 개의 단색 검증 목업을 배치한다.
- 각 목업은 실제 C04a 증거 또는 코드·문서에서 확인된 필드만 텍스트로 사용한다.

```text
Block Hit
Defense Outcome | Guard
Reaction Outcome | BlockHit
Final Damage | [확인값]
Commit Damage | [확인값]

Parry
Defense Outcome | Parry
Reaction Outcome | Parry
Final Damage | [확인값]
Commit Damage | [확인값]

Player Hit
Defense / Reaction Outcome | [확인값]
Final / Commit Damage | [확인값]
```

### 5.2 증거 처리

- C04a의 기존 상태는 “결과 필드의 출처”로만 유지한다.
- p.6의 화면은 결과 검증 **목업**이며, 게임플레이 Hero나 제출용 Overlay 캡처처럼 보이게 만들지 않는다.
- Evidence Ledger, Capture Shot List, Page Spec에는 FinalCandidate 이미지가 p.6의 직접 시각 자료에서 제외됐음을 기록한다.

---

## 6. 문서 번호 동기화

아래 Audit Plan의 현재 페이지 표기를 현행 24페이지 기준으로 변경한다. 파일명은 기존 감사 이력을 보존하기 위해 유지하되, 제목·상태·목적에 적힌 현행 페이지 번호는 수정한다.

| Audit Plan 파일 | 현행 페이지 |
| --- | ---: |
| `06_P09_Shared_Execution_...` | 11 |
| `07_P10_Intervention_Policy_...` | 12 |
| `08_P11_Combat_Signal_...` | 13 |
| `09_P12_Data_Driven_Resolve_...` | 14 |
| `10_P13_AI_Intent_...` | 15 |
| `11_P14_Combat_Target_...` | 16 |
| `12_P15_Participation_...` | 17 |
| `13_P16_Profiling_...` | 18 |
| `14_P17_Runtime_Debug_...` | 19 |
| `15_P18_Overlay_Editor_Plugin_...` | 20 |
| `16_P19_Asset_Inspector_...` | 21 |
| `17_P20_Root_Motion_Tool_...` | 22 |
| `18_P21_AI_Workflow_...` | 23 |
| `19_P22_Traceability_...` | 24 |

`22_P10_Death_Lifecycle_...`은 현행 p.10을 유지한다.

---

## 7. 구현 순서

1. 이 문서의 공통 규칙을 `04_Portfolio_Page_Composition_Application_Plan`에 반영하고, 기존의 모순된 적용 기록을 정리한다.
2. HTML의 전체 페이지 번호, section-head 우측 span, 단색 토큰을 먼저 정리한다.
3. p.6·p.8·p.9를 파일럿으로 구현하고 A4 PDF 렌더링으로 제목 줄바꿈·영역 높이를 검수한다.
4. p.11~p.18 SVG를 제작·배치한다. p.18은 raw CSV 재첨부 전 성능 결과를 전면 성과로 쓰지 않는다.
5. p.19~p.24를 실제 증거 캡처와 SVG 유형에 맞게 재배치한다.
6. Page Spec, Audit Plan, Evidence Ledger, Capture Shot List, Enrichment Backlog, Claim Freeze Log, Review Checklist, Word Handoff를 실제 변경 내용에 맞게 동기화한다.
7. 24페이지 section 수, Index·footer·eyebrow 번호, SVG/이미지 경로, PDF 인쇄 가독성을 검수한다.

---

## 8. 완료 검수 기준

- [ ] Analytical Wireframe의 `.page` section 수가 24개다.
- [ ] p.3~p.24 eyebrow가 실제 전체 페이지 번호와 일치한다.
- [ ] footer·Index·Page Spec·Audit Plan의 번호가 일치한다.
- [ ] `.section-head > span`이 HTML에 남아 있지 않다.
- [ ] 둘째 영역 제목이 적용 대상에서 모두 `런타임 검증`이다.
- [ ] 문제 해결 제목은 `문제 해결 경험 | [사례]` 형식을 사용한다.
- [ ] 본문 UI의 파랑·초록 강조색이 제거됐다.
- [ ] p.6에 FinalCandidate 결과 이미지 `<img>`가 남아 있지 않다.
- [ ] 구조 페이지의 메인 시각화가 독립 SVG 산출물로 배치됐다.
- [ ] Evidence Ledger 상태보다 강한 결과 문구가 없다.
- [ ] PDF 100% 확대에서 섹션 제목과 다이어그램 라벨을 읽을 수 있다.

---

## 9. 관련 기준

- `P03_UE5_Portfolio_Production_Master_Plan (KR).md`
- `00_Portfolio_Page_Spec_Index (KR).md`
- `21_Portfolio_Case_Evidence_and_Page_Assignment_Plan (KR).md`
- `Portfolio_Evidence_Capture_Ledger (KR).md`
- `03_Portfolio_Claim_Freeze_Log (KR).md`
- `04_Portfolio_Page_Composition_Application_Plan (KR).md`
- `02_Portfolio_Capture_Shot_List (KR).md`
- `Portfolio_Review_Checklist (KR).md`
