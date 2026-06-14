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

AI와 사용자가 같은 기준으로 문서를 작성, 검토, 갱신할 수 있도록 PR 문서, 문서군 Index, Prompt Library, audit 산출물, notes를 정리한다.

```yaml
핵심 목표
- PR 문서 P01~P18 정식 문서 승격
- PR draft audit 산출물 정리
- 문서군별 Index 작성
- System Architecture / Portfolio Document 파일명 체계 정리
- AI Workflow / Prompt Library 기준 갱신
- System / Engine 문서 재분류 후속 범위 분리
```

```yaml
목표 수준
- AI 협업 과정에서 문서 기준이 흔들리지 않도록 공통 운영 기준을 둔다
- 공통 문서 구조 위에 문서군별 override를 적용할 수 있게 한다
- 불필요한 EN 문서와 구형 문서명 흔적을 제거한다
- 문서 간 연결을 실제 파일명과 Index 기준으로 맞춘다
- System / Engine 본문 재분류는 다음 Branch에서 처리할 수 있게 준비 문서로 남긴다
```

---

## 2. 완료 기준

이 Branch는 다음 조건을 만족하면 PR 가능한 상태로 본다.

```yaml
완료 기준
- P01~P18 PR draft가 정식 PR 문서로 승격되어 있다
- PR draft audit 중간 산출물이 정리되고 장기 유지 audit 기준이 남아 있다
- 전체 Documentation Index와 문서군별 Index가 작성되어 있다
- Pull Request Index에 P01~P18과 P19 연결 기준이 반영되어 있다
- System Architecture 문서 파일명이 Sxx 체계로 정리되어 있다
- Portfolio Document 문서군이 PFxx 체계로 정리되어 있다
- AI Workflow / Prompt Library가 Portfolio Document / PFxx 기준으로 갱신되어 있다
- System / Engine 재분류 기준이 notes 문서로 분리되어 있다
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
문서군 Index
- Docs/00_Documentation_Index.md
- Docs/99_Legacy/Issue_CheckList/00_Issue_Checklist_Index.md
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

### 4.1 PR 문서 체계 정리

- [x] P01~P18 draft 문서를 정식 PR 문서로 승격
- [x] PR 문서 내 구형 draft 표현 제거
- [x] `## 관련 문서` 섹션명 기준 정리
- [x] PR 문서 용어 / 목표 흐름 / audit 결과 반영
- [x] `Docs/04_Pull_Request/00_Pull_Request_Index.md` 작성

### 4.2 audit 산출물 정리

- [x] PR draft audit 중간 산출물 정리
- [x] 장기 유지 audit 문서 4종으로 축약
- [x] Term Usage 기준 / 실제 용어 맵 / PR Goal Flow / Draft Finding 역할 분리
- [x] 중간 산출물 정리 상태 반영

### 4.3 문서군 Index 작성

- [x] `Docs/00_Documentation_Index.md`를 상위 라우터로 재작성
- [x] Issue Checklist / Bug Report / Pull Request / System Architecture / Portfolio Document 문서군별 Index 작성
- [x] `Dxx / Bxx / Pxx / Sxx / PFxx` ID 체계 반영
- [x] 상세 목록은 각 문서군별 Index에서 관리하도록 분리

### 4.4 System / Portfolio 문서 파일명 정리

- [x] System Architecture 문서를 `Sxx_UE5_Portfolio_System_Architecture.md` 형식으로 정리
- [x] System Architecture archive 문서를 `Axx_UE5_Portfolio_System_Architecture_Archive.md` 형식으로 정리
- [x] Portfolio Document 문서를 `PFxx_UE5_Portfolio_Document.md` 형식으로 정리
- [x] `Technical Document` / `Txx` 기준을 `Portfolio Document` / `PFxx` 기준으로 갱신

### 4.5 AI Workflow / Prompt Library 갱신

- [x] AI Workflow 문서군에서 `Portfolio Technical Document` 구형 명칭 제거
- [x] Prompt Library에 `Portfolio_Document_Writing_Prompt`와 `Documentation_Index_Writing_Prompt` 기준 반영
- [x] Prompt 변경 후보 자동 감지 / 후보 기록 / 승인 반영 흐름 보강
- [x] Git preflight에서 staged format-only 변경을 내용 변경과 분리하는 기준 추가

### 4.6 후속 System / Engine 재분류 범위 분리

- [x] System / Engine 문서 재분류 기준을 `N01_System_Engine_Document_Reclassification_Note.md`로 분리
- [x] System Architecture 본문 재작성과 Engine Technique 문서군 신설은 후속 Branch 범위로 분리
- [x] Portfolio Document 본문 재구성은 후속 범위로 분리

---

## 5. 비범위

```yaml
비범위
- System Architecture 본문 재작성
- Engine Technique 문서군 신설
- Engine Implementation Records 최종 분리
- Portfolio Document 본문 재구성
- History / Records 체계 최종 확정
- UE C++ 코드 변경
- UE Build / PIE / Editor / Asset 검증
```

---

## 6. 검증 기준

### 문서 존재 / 번호 확인

- [x] W01~W03 Work List 문서 확인
- [x] P01~P19 PR 문서 확인
- [x] 문서군별 Index 존재 확인

### 구형 명칭 검색

- [x] `Portfolio Technical Document` 잔존 여부 확인
- [x] `Technical_Documents` 잔존 여부 확인
- [x] `Txx` 잔존 여부 확인
- [x] `Refactor_Draft` 잔존 여부 확인
- [x] `관련 PR / 문서` 잔존 여부 확인

### Index 검증

- [x] `W02`, `P19`, `PFxx`, `S29`, `Portfolio Document` 검색 확인
- [x] Pull Request Index에 P19 행 추가
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
- Engine Technique 문서군 최종 폴더 구조
- Portfolio Document 본문 재구성 범위
- History / Records 체계 최종 구성
- UE Build / PIE / Editor / Asset 검증
```

```yaml
확인 필요
- 다음 Branch에서 System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records 경계 확정
- Portfolio Document가 내부 원천 문서를 얼마나 압축해 보여줄지 결정
- History 문서 체계를 별도 문서군으로 둘지, Records / Archive 조합으로 대체할지 결정
```

---

## 8. 후속 작업

- 다음 Branch에서 `N01_System_Engine_Document_Reclassification_Note.md` 기준으로 System / Engine 문서를 재분류한다.
- System Architecture는 현재 시스템 구조 설명 중심으로 재작성한다.
- Engine Technique Document 문서군 신설 여부와 폴더 번호를 결정한다.
- Portfolio Document는 내부 System / Engine / AI Workflow 문서를 기반으로 제출용 설명 문서로 재구성한다.
- History / Records / Archive 체계를 별도 문서군으로 둘지 재검토한다.

---

## 9. 관련 문서

- PR Document: `P19_UE5_Portfolio_Pull_Request.md`
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

이번 Branch에서는 문서 작성 / 검토 / 갱신 기준을 안정화하고, System / Engine 본문 재분류는 다음 Branch에서 이어갈 수 있도록 범위를 분리했다.

