# Code Review Prompt

## 1. 목적

구현 결과에서 버그, 회귀, 책임 경계 붕괴, Unreal C++ 관례 위반, 누락된 검증을 우선 찾는다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 코드 변경 후 리뷰가 필요할 때
-> PR 전 회귀 위험을 확인할 때
-> 책임 경계와 Unreal C++ 관례 위반 가능성을 점검할 때
```

---

## 3. 사용 방법

대상 변경 범위와 관련 문서를 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 변경 내용을 Project Stella 기준으로 코드 리뷰해줘.

대상 변경:
- [PR 번호 / Branch / commit range / changed files]

관련 문서:
- Work List:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Document:

리뷰 목표:
- 변경 요약보다 버그, 회귀, 책임 경계 위반, Unreal C++ 관례 위반, 누락 검증을 먼저 찾아줘.
- 코드의 실제 타입명 / API / enum / struct / component / executor / subsystem / asset reference를 기준으로 판단해줘.
- 확인하지 못한 Asset / Blueprint / PIE 항목은 추정으로 단정하지 말고 미검증 항목으로 남겨줘.

출력 형식:
1. Findings
2. Open Questions / Assumptions
3. Test Gaps
4. Summary

Findings 작성 기준:
- 심각도 높은 문제부터 작성
- 파일 / 함수 / 라인 또는 구체 코드 위치 포함
- 문제 현상, 발생 조건, 영향, 수정 방향 연결
- 문제가 없으면 주요 코드 리뷰 이슈는 찾지 못했다고 명시하고 Test Gaps만 정리

중점 리뷰 기준:
- Orchestrator / Component / Executor / AnimNotify 책임 경계
- Action / Reaction / Damage / Feedback / CombatResolution 책임 분리
- UObject lifetime / UPROPERTY / IsValid / Delegate / Montage lifecycle
- DataAsset / DataTable / Blueprint / Asset reference 영향
- Build / Code Flow / PIE / Editor / Asset 검증 분리
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 변경 범위
-> 관련 코드 / 문서
-> 검증 결과
-> 사용자가 우려하는 위험
```

---

## 6. 출력 기준

```yaml
출력 기준
-> Findings
-> Open Questions / Assumptions
-> Test Gaps
-> Summary
```

---

## 7. 범위 / 비범위

```yaml
범위
-> 코드 리뷰
-> 회귀 / 책임 경계 / 검증 누락 확인

비범위
-> 코드 수정
-> PR Document 작성
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 칭찬보다 Findings 우선
-> 변경과 무관한 오래된 문제는 핵심 Finding으로 다루지 않음
-> 미확인 Asset / Blueprint 항목을 단정하지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 현재 변경과 직접 연결되는지 먼저 판단
-> 문서 차이는 코드 Finding과 분리
```

---

## 10. 검증 기준

```yaml
검증 기준
-> Finding에 위치 / 영향 / 수정 방향이 있는가
-> Test Gap이 Build / Code Flow / PIE / Editor / Asset으로 분리되는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 주요 Findings 또는 이슈 없음 판단과 Test Gap이 정리됨

실패
-> 변경 범위 확인 불가

미검증
-> Runtime / Editor / Asset 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Code Review Prompt
-> 구현 결과의 문제를 Finding 중심으로 검토

Verification Log Prompt
-> 수행한 검증 결과를 로그로 정리
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> 심각도 기준
-> 코드 리뷰 출력 템플릿
```
