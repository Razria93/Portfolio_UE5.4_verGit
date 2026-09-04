# Debug Overlay / Execution Collaboration 브랜치 검증 기록

## 목적

이 문서는 Balance / Collapse, Execution Collaboration, Debug Overlay 정합화 브랜치의 실제 수동
확인 범위와 아직 수행하지 않은 회귀 범위를 분리해 기록한다. 설계 문서의 완료 표시는 아래의 확인 범위를
넘어서는 전체 기능 매트릭스 통과를 뜻하지 않는다.

## 확인 환경

- Map: `/Game/00_UnitTest/TestRoom`
- Runtime: non-shipping PIE
- Debug Overlay: `HUDVisible=1`, `CaptureEnabled=1`

## 확인 완료

| 항목 | 결과 | 확인 의미 |
| --- | --- | --- |
| Editor build | 통과 | 이번 브랜치의 C++/Editor 모듈 변경이 build 가능한 상태임을 확인 |
| Overlay empty state | 통과 | event 또는 focus가 없을 때 Character Details/Event Log/World Summary가 안전하게 빈 상태를 표시 |
| Execution participant movement collision | 통과 | 활성 pair에서 participant capsule이 root motion을 막지 않음 |
| Execution 종료 뒤 collision 복구 | 통과 | session 종료 뒤 `MoveIgnoreActorAdd` 등록이 남지 않고 정상 충돌로 복구 |

## 아직 별도 회귀가 필요한 항목

| 항목 | 이유 | 마감 조건 여부 |
| --- | --- | --- |
| Standard / Lethal outcome 전체 매트릭스 | threshold, cancel, terminal, recovery를 조합한 회귀 확인 필요 | 후속 검증 |
| 동일 이름 Enemy destroy 후 재스폰의 Recent AI 이력 분리 | 현재 TestRoom에 재스폰을 재현할 test spawner가 없음 | 이번 브랜치 마감 조건에서 제외 |
| Editor Debug Overlay의 CVar lookup 빈도 | 기능 정합성과 별개인 Editor tooling 성능 개선 항목 | 별도 작업 |

## 관련 현재 계약

- Execution Collaboration: `S36_UE5_Portfolio_Execution_Collaboration_Architecture.md`
- Balance / Collapse: `S35_UE5_Portfolio_Enemy_Balance_Collapse_Architecture.md`
- Overlay 운영: `../02_Operation/Debug_Overlay_Operation_Guide_KR.md`

## 기록 원칙

- `Completed`는 해당 표의 실제 확인 범위만 의미한다.
- 아직 실행하지 않은 Standard/Lethal 조합은 성공으로 추정하지 않는다.
- 동일 이름 Actor 재스폰 이력은 구현상 Actor 인스턴스 key로 분리되지만, PIE 재현 검증은 별도 test fixture가 생긴 뒤 수행한다.
