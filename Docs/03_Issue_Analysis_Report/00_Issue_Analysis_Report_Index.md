# Project Stellar 문제 해결 사례

[대표 README](../../README.md#problem-solving) · [목적별 문서 안내](../00_Documentation_Index.md)

## 먼저 읽을 사례

| 사례 | 확인할 판단 | 다음 자료 |
| --- | --- | --- |
| [가드 종료 중 피격 누락 — I09](I09_Guard_Cleanup_Case_Study.md) | 반복 정리 수락, 판단과 적용 사이 상태 변화 | [검증 결과·출처](../98_Evidence/Combat_Case_Studies_20261021/README.md) |
| [피격 근거 TTL 시작 시점 — I10](I10_HitReactive_TTL_Case_Study.md) | Reaction 수명과 근거 수명 분리, 파괴 경로 한계 | [검증 결과·출처](../98_Evidence/Combat_Case_Studies_20261021/README.md) |
| [AI Perception 대상 필터링 — I11](I11_AI_Perception_Case_Study.md) | 인지 필터와 반복 측정, 프레임 시간 구분 | [검증 결과·출처](../98_Evidence/Combat_Case_Studies_20261021/README.md) |
| [무기 규격 차이를 흡수하는 Socket·Pivot — I04](I04_UE5_Portfolio_Issue_Analysis_Report.md) | 장착 좌표와 무기 내부 회전 기준 분리, 표현 계층과 복구 책임 | [S39 구조](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md), 본문 코드·근거 링크 |
| [자동 완료에 가려진 Montage 종료 설정 — I06](I06_UE5_Portfolio_Issue_Analysis_Report.md) | 자연 재생 종료와 명시적 실행 완료 분리, 감사 범위 설정 | [F08 수정·검증](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md) |
| [넉백·공격 시작 방향 보정](../07_Portfolio_Documents/Portfolio_Production/37_Gameplay_Knockback_and_Facing_Review_Guide.md) | 실행 성공 뒤 이동 적용, 이전 실행과 현재 실행의 정리 구분 | 통합 가이드의 코드 지도·자동 검사·PIE 범위 |

각 사례는 **증상 → 원인과 제약 → 선택한 해결책 → 구현 → 검증과 한계** 순서로 읽습니다. 기존 결과는 당시 기록이며 이번 문서 정리에서 새 런타임 검증을 수행한 것으로 해석하지 않습니다.

## 전체 분석 기록

| ID | 주제 | 문서 | 근거 상태 |
| --- | --- | --- | --- |
| I01 | 기존 분석 기록 | [I01 분석 문서](I01_UE5_Portfolio_Issue_Analysis_Report.md) | 당시 기록 유지 |
| I02 | MoveTo 도착 판정 | [I02 분석 문서](I02_UE5_Portfolio_Issue_Analysis_Report.md) | 당시 기록 유지 |
| I03 | 캐릭터·리타게팅 | [I03 분석 문서](I03_UE5_Portfolio_Issue_Analysis_Report.md) | 설정 화면·Action/Movement 분리·Additive 미사용 확인 |
| I04 | 무기 좌표계·Socket·Pivot | [I04 분석 문서](I04_UE5_Portfolio_Issue_Analysis_Report.md) | 구현·사용자 PIE 확인 |
| I05 | Trail 데이터·수명 | [I05 분석 문서](I05_UE5_Portfolio_Issue_Analysis_Report.md) | 구현·사용자 PIE 확인 |
| I06 | Montage 종료 계약 | [I06 분석 문서](I06_UE5_Portfolio_Issue_Analysis_Report.md) | 감사 로그·사용자 PIE 확인 |
| I07 | 환경 통합 | [I07 분석 문서](I07_UE5_Portfolio_Issue_Analysis_Report.md) | 간단한 작업 내역 / 상세 증거 수집 종료 |
| I08 | Content 참조·호환성·LFS | [I08 분석 문서](I08_UE5_Portfolio_Issue_Analysis_Report.md) | 보존·이주 및 자체 Inspector 사용 화면 확인 |
| I09 | 가드 종료 중 피격 누락 | [사례 문서](I09_Guard_Cleanup_Case_Study.md) | 경계 자동화·실제 충돌 통합, 스냅샷 범위 명시 |
| I10 | 피격 근거 TTL 시작 시점 | [사례 문서](I10_HitReactive_TTL_Case_Study.md) | 19시나리오·역사적 재촬영, Pending 잔여 미해결 |
| I11 | AI Perception 대상 필터링 | [사례 문서](I11_AI_Perception_Case_Study.md) | 18회 원시 재집계, 전체 FPS 개선 주장 제외 |

---
