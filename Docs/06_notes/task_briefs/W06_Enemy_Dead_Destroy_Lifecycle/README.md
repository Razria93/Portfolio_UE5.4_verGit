# W06 Enemy Dead / Destroy Lifecycle

Enemy의 사망 판정, DeadIn / DeadLoop 표현, Character·Weapon Dissolve와 Actor 생명주기 종료를 기존 Action / Reaction 구조에 결합한 작업군이다. W05에서 이관한 Targeting Destroy 경계도 함께 검증했다.

| ID | 제목 | 파일 | 상태 |
| --- | --- | --- | --- |
| W06-01 | Enemy Dead / Destroy Lifecycle v1 | `TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md` | Runtime / Asset / PIE / 문서 완료, Merge 준비 |

## 기준 문서

- 현재 구조와 책임 계약: [S31 Enemy Dead / Presentation / Destroy 생명주기 설계](../../../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- 구현·에셋·검증 기록: [TB W06-01](TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md)
- 해결 기록과 남은 Execution cleanup: [N14](../../N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md)

## 완료 범위

```text
Goal 1: Runtime 구조 감사와 C++ 구현
Goal 2: Editor Asset 연결과 PIE 통합 검증
Goal 3: 구조·API·Debug Overlay 정리
P0 문서 동기화: 완료
```

남은 절차는 최종 diff / build 상태 확인, Push, PR, 리뷰 대응과 사용자의 일반 Merge다.
