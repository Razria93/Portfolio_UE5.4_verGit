# Git Commit PR Preflight Prompt

## 1. 목적

Commit 또는 PR 직전에 changed files, staged / unstaged / untracked, 관련 문서, 검증 상태, 미검증 항목을 점검한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Commit 전 staging 대상을 정리할 때
-> PR 전 변경 범위와 검증 상태를 확인할 때
-> 관련 없는 변경이 섞였는지 확인할 때
```

---

## 3. 사용 방법

대상 작업과 현재 Git 상태를 확인하도록 요청하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
현재 작업을 Commit / PR 전에 점검해줘.

대상 작업:
- [Branch / Work Checklist / 작업 요약]

관련 문서:
- Work Checklist:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:
- Code Review:
- Asset Blueprint Validation:

점검 목표:
- git status와 changed files를 먼저 확인해줘.
- staged / unstaged / untracked 상태를 분리해서 정리해줘.
- 사용자 변경과 AI 작업 변경을 섞어 되돌리지 말아줘.
- 변경 파일을 Code / Docs / Prompt / Asset / Config / Generated 기준으로 그룹화해줘.
- 관련 없는 변경이 섞였으면 별도 그룹으로 분리해줘.
- Commit 전에는 포함 추천과 제외 추천을 명확히 제안해줘.
- PR 전에는 변경 요약, 검증 상태, 미검증 항목, 관련 문서 누락을 점검해줘.
- destructive git 명령은 제안하지 말아줘.

출력 형식:

Preflight Summary
- Branch:
- Changed files:
- Staged:
- Unstaged:
- Untracked:

File Groups
- Code:
- Docs:
- Prompt:
- Asset:
- Config:
- Generated:
- Unrelated / 확인 필요:

Document Links
- Work Checklist:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:
- Code Review:
- Asset Blueprint Validation:

Verification Status
- Build:
- Code Flow:
- PIE:
- Editor:
- Asset:
- 미검증:

Commit Recommendation
- 포함 추천:
- 제외 추천:
- 추가 확인:

PR Readiness
- 준비됨 / 보류 권장:
- 이유:
- PR 작성 시 포함할 핵심 문장:
````

---

## 5. 입력 기준

```yaml
입력 기준
-> Branch명
-> 작업 요약
-> 관련 문서
-> 현재 Git 상태
-> 검증 결과
```

---

## 6. 출력 기준

```yaml
출력 기준
-> Git 상태
-> 파일 그룹
-> 문서 연결
-> 검증 상태
-> Commit 추천
-> PR 준비도
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Commit / PR 전 점검

비범위
-> 실제 staging / commit / PR 실행
-> destructive git 명령
```

---

## 8. 제약 조건

```yaml
제약 조건
-> git reset / checkout -- / 강제 삭제 제안 금지
-> staged 상태가 불명확하면 commit 권장하지 않음
-> 수행하지 않은 검증을 완료로 표현하지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 관련 변경 / unrelated 변경을 분리
-> staged 대상이 불명확하면 확인 필요로 표시
```

---

## 10. 검증 기준

```yaml
검증 기준
-> changed files가 그룹화되었는가
-> commit 포함 / 제외 추천이 분리되었는가
-> PR readiness가 검증 상태와 연결되는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> Commit / PR 전 점검 항목이 정리됨

실패
-> Git 상태 확인 불가

미검증
-> Build / PIE / Editor / Asset 검증 상태 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Git Commit PR Preflight Prompt
-> Commit / PR 전 운영 점검

PR Document Writing Prompt
-> PR 문서 작성
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> Commit message 추천 기준
-> staged split 기준
```

