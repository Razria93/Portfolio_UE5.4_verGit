# Asset Blueprint Validation Prompt

## 1. 목적

C++ Build만으로 확인되지 않는 Editor / Asset / Blueprint 영향을 점검한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Blueprint parent, exposed property, Asset reference 영향이 있을 때
-> Montage / AnimNotify / DataAsset / DataTable 변경 가능성이 있을 때
-> C++ class / enum / struct / property rename 영향이 있을 때
```

---

## 3. 사용 방법

대상 변경과 관련 Asset / Blueprint 후보를 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 변경 내용이 Unreal Editor / Asset / Blueprint에 미치는 영향을 점검해줘.

대상 변경:
- [Branch / PR / commit range / changed files / 작업 요약]

관련 문서:
- Work List:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:
- Code Review:

검증 목표:
- C++ Build 성공과 Editor / Asset 검증을 분리해줘.
- Blueprint parent, compile status, exposed property, serialized reference 영향을 확인해줘.
- Montage notify, notify state, notify window timing, montage section 영향을 확인해줘.
- DataAsset / DataTable entry, key mismatch, missing / duplicate / fallback 위험을 확인해줘.
- rename이 Blueprint, Asset reference, redirector, serialized enum value에 영향을 주는지 확인해줘.
- 확인하지 못한 항목은 미검증으로 남겨줘.

출력 형식:

Asset / Blueprint 검증

Blueprint
- 상태:
- 확인 대상:
- 결과:
- 미검증:

Montage / AnimNotify
- 상태:
- 확인 대상:
- 결과:
- 미검증:

DataAsset / DataTable
- 상태:
- 확인 대상:
- 결과:
- 미검증:

Reference / Redirector
- 상태:
- 확인 대상:
- 결과:
- 미검증:

Serialized Value
- 상태:
- 확인 대상:
- 결과:
- 미검증:
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 변경 파일
-> 관련 Blueprint / Asset 후보
-> 관련 enum / struct / property 변경
-> 검증 가능한 Editor 환경 여부
```

---

## 6. 출력 기준

```yaml
출력 기준
-> Blueprint 영향
-> Montage / AnimNotify 영향
-> DataAsset / DataTable 영향
-> Reference / Redirector 영향
-> Serialized Value 영향
-> 미검증 항목
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Asset / Blueprint 검증 항목 점검

비범위
-> C++ 코드 리뷰
-> 실제 Editor 조작
```

---

## 8. 제약 조건

```yaml
제약 조건
-> C++ Build 성공을 Asset 검증 완료로 해석하지 않음
-> 확인하지 못한 Asset 상태를 완료로 표현하지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 영향 있음 / 영향 없음 / 미확인을 분리
-> 확인해야 할 Asset 후보를 구체적으로 남김
```

---

## 10. 검증 기준

```yaml
검증 기준
-> 확인 대상과 결과가 분리되는가
-> 미검증 항목이 Verification Log로 옮길 수 있게 작성되는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> Asset / Blueprint 영향 항목이 분리됨

실패
-> 대상 변경 또는 Asset 후보 확인 불가

미검증
-> Editor에서 직접 확인해야 하는 항목
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Asset Blueprint Validation Prompt
-> Editor / Asset / Blueprint 영향 점검

Verification Log Prompt
-> 수행한 검증 결과 기록
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> Asset 유형별 체크리스트
```
