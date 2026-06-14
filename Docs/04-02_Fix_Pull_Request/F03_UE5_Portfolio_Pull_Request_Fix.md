# UE5 Portfolio Pull Request Fix

## 제목

**F03: README 문서 링크 갱신**

## 날짜

**2026.06.14**

## 상태

- [x] **완료**

---

## 브랜치

- `fix/readme-documentation-links`

---

## 요약

이번 Fix PR에서는 **README에 남아 있던 구형 문서 경로를 현재 문서 구조에 맞게 갱신했다.**

문서 구조 재정리 이후 `Docs/07_Technical_Documents`, `Docs/01_Issue_CheckList`, `Txx` 기반 대표 문서 경로가 README에 남아 있어, repository 진입점에서 존재하지 않는 문서로 연결될 수 있었다.

---

## 변경 사항

- README의 포트폴리오 문서 링크를 `Docs/07_Portfolio_Documents` 기준으로 수정했다.

- README의 Work List 문서 링크를 `Docs/01_Work_List` 기준으로 수정했다.

- 대표 문서 목록의 구형 `Txx` 경로를 현재 `PFxx_UE5_Portfolio_Document.md` 경로로 갱신했다.

- README 안의 구형 `Technical Documents` 표현을 현재 문서 체계의 `Portfolio Documents` 기준으로 정리했다.

---

## 검증 결과

- README에서 `07_Technical_Documents`, `01_Issue_CheckList`, `T0x` 기반 구형 문서 경로가 남지 않았는지 확인했다.

- `git diff --check -- README.md` 기준 공백 오류가 없음을 확인했다.

- 이번 Fix PR은 문서 링크 보정만 포함하므로 UE C++ 빌드, PIE, Editor / Asset 검증은 수행하지 않았다.

---

## 관련 문서

- Pull Request: `P19_UE5_Portfolio_Pull_Request.md`

- Documentation Index: `00_Documentation_Index.md`

- Portfolio Document Index: `00_Portfolio_Document_Index.md`

- Work List Index: `00_Work_List_Index.md`

---
