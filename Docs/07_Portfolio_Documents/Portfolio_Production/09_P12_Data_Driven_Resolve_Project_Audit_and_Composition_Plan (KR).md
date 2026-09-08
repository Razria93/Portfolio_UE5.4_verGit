# Data-driven Resolve 경계 감사 기록 (p.11 보조 근거)

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 25페이지 전환안에서는 독립 대표 페이지를 배정하지 않고 p.11 Execution Transition의 Context 구성 보조 근거로만 사용한다.

> 상태: **코드·설계 문서 감사 완료 / 구성·A4 렌더링·독립 검토 승인 / HTML Drafted**

## 확인 근거와 주장 범위

- Action / Reaction Orchestrator는 각각 Candidate를 Context로 resolve하며, Context에는 DataKey·resolved Data·Executor가 보존된다.
- `ResolveActionData()` / `ResolveReactionData()`와 `ResolveActionExecutor()` / `ResolveReactionExecutor()`가 조회와 executor 결정의 책임을 분리한다.
- ActionDataKey는 ActionType·Index, ReactionDataKey는 DamageSpecKey·ReactionType·MatchMode를 기준으로 조회한다.
- Key는 lookup 기준이며 runtime Context는 resolved Data·Executor를 포함한 엄격한 실행 입력이다. wildcard/global match는 lookup 규칙이지, 느슨한 runtime context를 뜻하지 않는다.

## 기록 범위와 향후 사용

1. Action·Reaction의 별도 `Request → Candidate Key → Data Resolve → Execution Context → Executor` lane은 p.11 공통 전환 계약의 보조 도식으로만 사용한다.
2. Key / Data / Context 책임 비교는 S37의 Context 구성 근거로 유지한다.
3. 독립 p.14는 Combat Signal Target Outcome Resolution로 교체한다. 이 감사 기록을 p.14 결과 검증이나 범용 확장 성과로 사용하지 않는다.
