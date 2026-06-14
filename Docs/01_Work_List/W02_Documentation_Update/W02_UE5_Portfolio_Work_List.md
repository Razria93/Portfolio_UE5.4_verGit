# UE5 Portfolio - Work List

## 제목

**W02: AI 협업 기반 문서 운영 체계 정리**

## 날짜

**2026.06.14**

## 상태

- [x] **완료**

---

## 브랜치

- `docs/portfolio-documentation-update`

---

## 1. Branch 목표

이번 Branch는 구형 문서 구조를 AI 협업 기준에 맞는 문서 운영 체계로 업데이트한다.

AI와 사용자가 같은 기준으로 문서를 작성, 검토, 갱신할 수 있도록 주요 문서 유형의 작성 기준과 문서 간 연결 방식을 정리한다.

PR 문서는 프로젝트 전반의 변경 흐름을 압축해 보여주는 핵심 문서로 재구성하고, Issue Checklist / Work List / Bug Report는 PR 기준 용어 톤을 공유하면서 각 문서 목적에 맞게 양식과 연결 기준을 맞춘다.

```yaml
핵심 목표
- 주요 문서 유형의 작성 기준과 문서 간 연결 방식 정리
- PR 문서가 정식 변경 기록으로 기능하도록 보완본 기준을 반영
- PR 문서 검토 과정에서 만든 용어 / 목표 흐름 / 품질 기준을 후속 검토에 재사용할 수 있게 정리
- 문서 탐색 기준을 전체 Documentation Index와 문서 유형별 Index 중심으로 재구성
- System Architecture / Portfolio Document의 식별 기준을 후속 재분류에 맞게 정리
- AI Workflow / Prompt Library에 문서 작성과 Prompt 유지관리 기준을 반영
- System / Engine 문서 재분류를 다음 Branch에서 이어갈 수 있게 범위 분리
```

```yaml
목표 수준
- AI 협업 과정에서 문서 작성 기준이 흔들리지 않도록 공통 기준을 둔다
- PR 기준 용어 톤을 유지하되 Issue Checklist / Work List / Bug Report는 각 문서 목적에 맞게 압축한다
- 구형 EN 문서와 구형 문서명 흔적을 제거한다
- 문서 간 연결을 실제 파일명과 문서 유형별 Index에 맞춘다
- System / Engine 본문 재분류는 다음 Branch에서 처리할 수 있게 준비 문서로 남긴다
```

---

## 2. 완료 기준

이 Branch는 다음 조건을 만족하면 PR 가능한 상태로 본다.

```yaml
완료 기준
- 주요 문서 유형이 공통 용어 톤과 관련 문서 기준을 공유한다
- PR 문서가 브랜치별 변경 흐름을 정식 문서 위치에서 설명하고, P19까지 Pull Request Index에서 추적할 수 있다
- PR 문서 검토 과정에서 만든 용어 / 목표 흐름 / 품질 기준이 후속 PR 문서 검토에 재사용 가능한 장기 유지 audit 문서로 정리되어 있다
- 전체 Documentation Index와 문서 유형별 Index를 통해 문서의 위치, 역할, 연결 대상을 확인할 수 있다
- 구형 Technical Document 기준이 제거되고, System Architecture / Portfolio Document 식별 기준이 현재 문서 체계와 후속 재분류 범위에 맞게 정리되어 있다
- AI Workflow / Prompt Library에 문서 작성 기준, Index 작성 기준, Prompt 후보 관리 기준이 반영되어 있다
- System / Engine 재분류 기준이 다음 Branch에서 바로 이어갈 수 있는 notes 문서로 분리되어 있다
- 코드 변경 / UE Build / PIE / Editor / Asset 검증이 이번 Branch 범위에서 제외되어 있다
```

---

## 3. 필수 산출물

```yaml
PR 문서
- Docs/04_Pull_Request/P01_UE5_Portfolio_Pull_Request.md ~ P19_UE5_Portfolio_Pull_Request.md
- Docs/04_Pull_Request/00_Pull_Request_Index.md
```

```yaml
문서 유형별 Index
- Docs/00_Documentation_Index.md
- Docs/99_Legacy/Issue_CheckList/00_Issue_Checklist_Index.md
- Docs/01_Work_List/00_Work_List_Index.md
- Docs/02_Bug_Report/00_Bug_Report_Index.md
- Docs/04_Pull_Request/00_Pull_Request_Index.md
- Docs/05_System_Architecture/00_System_Architecture_Index.md
- Docs/07_Portfolio_Documents/00_Portfolio_Document_Index.md
```

```yaml
AI Workflow / Prompt Library
- Docs/08_AI_Workflow/00_Index/AI_Workflow_Index (KR).md
- Docs/08_AI_Workflow/03_Operation/AI_Workflow_Operation_Guide (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library 하위 Prompt Blueprint / Prompt Files / Prompt Management 문서
```

```yaml
후속 분류 Note
- Docs/06_notes/N01_System_Engine_Document_Reclassification_Note.md
```

---

## 4. 완료된 작업 범위

### 4.1 문서 작성 기준 통일

- [x] Issue Checklist / Work List / Bug Report / PR 문서가 같은 용어 톤과 관련 문서 기준으로 연결되도록 정리
- [x] 각 문서 유형이 공통 문서 구조를 공유하되, 목적에 맞는 작성 기준을 덧씌울 수 있게 Prompt Library 기준 보강

### 4.2 PR 문서 운영 기준 정식화

- [x] P01~P18 PR 문서가 브랜치별 목표, 변경 흐름, 검증 결과, 관련 문서를 정식 변경 기록으로 설명하도록 정리
- [x] PR 문서가 프로젝트 변경 이력을 압축해 보여주는 핵심 문서로 기능하도록 보완본 기준 반영

### 4.3 PR 문서 검토 기준 유지관리

- [x] PR 문서 검토 과정에서 만든 용어, 목표 흐름, 문서 품질 기준을 후속 검토에 재사용할 수 있는 장기 유지 기준으로 축약
- [x] 중간 audit 산출물은 정리하고, 후속 PR 문서 보완에 필요한 기준만 관리 대상으로 남김

### 4.4 문서 식별 / 탐색 체계 정리

- [x] 전체 Documentation Index는 상위 라우터로 두고, 상세 목록은 문서 유형별 Index에서 찾도록 분리
- [x] 구형 `Technical Document` / `Txx` 기준을 제거하고, System Architecture / Portfolio Document / archive 문서가 현재 문서 체계 안에서 추적되도록 식별 기준 정리
- [x] Portfolio Document와 Index 작성 기준을 Prompt Library에 반영해 이후 문서 추가 시 같은 탐색 / 연결 기준을 재사용할 수 있게 정리

### 4.5 AI 협업 운영 기준 보강

- [x] 작업 종료 시 Prompt 반영 후보를 점검하고, 후보 기록 / 즉시 반영 / 보류를 구분하는 운영 기준 추가
- [x] Commit 전 staged format-only 변경과 내용 변경을 분리해 검토하는 Git preflight 기준 추가

### 4.6 후속 System / Engine 재분류 범위 분리

- [x] System Architecture 본문 재작성, Engine Technique 문서 유형 신설, Portfolio Document 본문 재구성은 이번 Branch의 완료 범위와 분리
- [x] 다음 Branch에서 System / Engine 문서 재분류를 이어갈 수 있도록 판단 기준을 notes 문서로 남김

---

## 5. 비범위

```yaml
비범위
- System Architecture 본문 재작성
- Engine Technique 문서 유형 신설
- Engine Implementation Records 최종 분리
- Portfolio Document 본문 재구성
- History / Records 체계 최종 확정
- UE C++ 코드 변경
- UE Build / PIE / Editor / Asset 검증
```

---

## 6. 검증 기준

### 문서 존재 / 번호 확인

- [x] Issue Checklist / Bug Report / Work List 관련 목록 확인
- [x] Work List Index에서 W01~W03 추적 가능 여부 확인
- [x] W01~W03 Work List 문서 확인
- [x] P01~P19 PR 문서 확인
- [x] 문서 유형별 Index 존재 확인

### 구형 명칭 검색

- [x] `Portfolio Technical Document` 잔존 여부 확인
- [x] `Technical_Documents` 잔존 여부 확인
- [x] `Txx` 잔존 여부 확인
- [x] `Refactor_Draft` 잔존 여부 확인
- [x] `관련 PR / 문서` 잔존 여부 확인

### Index 검증

- [x] `W02`, `W03`, `P19`, `PFxx`, `S29`, `Portfolio Document` 검색 확인
- [x] Work List Index에서 W01~W03 항목 확인
- [x] Pull Request Index에서 P19 항목 확인
- [x] Documentation Index가 상위 라우터 역할을 유지하는지 확인

### 공백 검증

- [x] `git diff --check` 실행
- [x] LF / CRLF warning은 공백 오류와 분리해 보고

### Git preflight

- [x] staged / unstaged / untracked 변경 분리
- [x] 에셋 변경과 unrelated 변경을 커밋 후보에서 분리
- [x] format-only 변경은 내용 변경과 분리

---

## 7. 미검증 / 확인 필요 항목

```yaml
미검증
- System Architecture 본문 최신성
- Engine Technique 문서 유형 최종 폴더 구조
- Portfolio Document 본문 재구성 범위
- History / Records 체계 최종 구성
- UE Build / PIE / Editor / Asset 검증
```

```yaml
확인 필요
- 다음 Branch에서 System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records 경계 확정
- Portfolio Document가 내부 원천 문서를 얼마나 압축해 보여줄지 결정
- History 문서 체계를 별도 문서 유형으로 둘지, Records / Archive 조합으로 대체할지 결정
```

---

## 8. 후속 작업

- 다음 Branch에서 `N01_System_Engine_Document_Reclassification_Note.md` 기준으로 System / Engine 문서를 재분류한다.
- System Architecture는 현재 시스템 구조 설명 중심으로 재작성한다.
- Engine Technique Document 문서 유형 신설 여부와 폴더 번호를 결정한다.
- Portfolio Document는 내부 System / Engine / AI Workflow 문서를 기반으로 제출용 설명 문서로 재구성한다.
- History / Records / Archive 체계를 별도 문서 유형으로 둘지 재검토한다.

---

## 9. 관련 문서

- PR 문서: `P19_UE5_Portfolio_Pull_Request.md`
- Work List Index: `00_Work_List_Index.md`
- Pull Request Index: `00_Pull_Request_Index.md`
- Documentation Index: `00_Documentation_Index.md`
- System / Engine Note: `N01_System_Engine_Document_Reclassification_Note.md`
- AI Workflow:
  - `AI_Workflow_Index (KR).md`
  - `AI_Workflow_Operation_Guide (KR).md`
  - `AI_Work_Pipeline (KR).md`
- Prompt Management:
  - `01_Prompt_Change_Management_Rule (KR).md`
  - `02_Prompt_Pattern_Candidates (KR).md`

---

## 10. 정리

W02는 구형 문서 구조를 AI 협업 기준의 문서 운영 체계로 업데이트한 작업 목록이다.

이번 Branch에서는 문서 작성 기준, 문서 탐색 기준, Prompt 유지관리 기준을 같은 운영 흐름 안에서 정리했다.

그중 PR 문서는 브랜치별 변경 흐름을 압축해 보여주는 핵심 변경 기록으로 재구성했고, System / Engine 본문 재분류는 다음 Branch에서 이어갈 수 있도록 범위를 분리했다.
