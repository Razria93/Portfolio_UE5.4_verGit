# W06 Enemy Dead / Destroy Lifecycle

Enemy의 사망 연출과 Actor 생명주기 종료를 기존 Action / Reaction 구조에 결합하고, W05에서 이관한 Targeting Destroy 경계 검증을 마감하는 작업군이다.

| ID | 제목 | 파일 | 상태 |
| --- | --- | --- | --- |
| W06-01 | Enemy Dead / Destroy Lifecycle v1 | `TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md` | 계획 확정 / 구현 전 |

## 구현 순서

```text
W06-01 Goal 1: Runtime 구조 감사 + C++ 구현 + 빌드
W06-01 Goal 2: Editor Asset 연결 + PIE 통합 검증
W06-01 Goal 3: 최종 감사 + 문서 + Push + Draft PR
W06-01 Goal 4: 리뷰 대응 + Merge Ready 확정
```

실제 `main` Merge는 Goal 4 완료 후 사용자가 일반 Merge 방식으로 수행한다.

