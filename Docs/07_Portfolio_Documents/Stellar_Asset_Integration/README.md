# Stellar 캐릭터·무기·환경 통합 사례

## 1. 작업의 중심

새 캐릭터·애니메이션·무기·환경 세트를 기존 전투 프로젝트에 통합했다. 단순 에셋 교체가 아니라 체형·무기 중심·부착 소켓·표현 수명·참조 구조의 차이를 해결하는 작업이었다.

원본 캐릭터·애니메이션·무기·환경 콘텐츠의 제작은 이 프로젝트의 기여로 주장하지 않는다. 본인의 작업은 리타게팅과 연결·설정, 충돌·조명 환경 구성, 통합에 필요한 C++ 구조와 검증·참조 관리다. 이 문서는 기존 제출 HTML을 대체하지 않는 후속 사례 기록이다.

## 2. 문제와 결과

| 통합 과정의 문제 | 해결 방향 | 상세 |
| --- | --- | --- |
| 체구 차이로 상체가 눌리는 리타게팅 | Action/Movement 분리, Spine 회전 전달 조정; Additive 미사용 | [I03](../../03_Issue_Analysis_Report/I03_UE5_Portfolio_Issue_Analysis_Report.md) |
| 장탈착 이격·양손 전환·손잡이 중심 회전 | Socket Transition, HandGrip Pivot, 무기 자식 계층, Action Scope | [I04](../../03_Issue_Analysis_Report/I04_UE5_Portfolio_Issue_Analysis_Report.md) |
| Trail 설정이 여러 곳에 분산 | 데이터 정의 집중화, 무기 Actor의 인스턴스 관리 | [I05](../../03_Issue_Analysis_Report/I05_UE5_Portfolio_Issue_Analysis_Report.md) |
| 자동 완료에 가려진 잘못된 Notify 설정 | 명시적 종료, Trigger 감사, 사용자 PIE 검증 | [I06](../../03_Issue_Analysis_Report/I06_UE5_Portfolio_Issue_Analysis_Report.md) |
| 새 맵의 플레이 환경 구성 | 맵 도입·충돌 해결·조명 구성의 간단한 작업 내역 | [I07](../../03_Issue_Analysis_Report/I07_UE5_Portfolio_Issue_Analysis_Report.md) |
| 대량 경로 정리에서 참조와 호환성 보존 | 실제 사용 확인 후 보존/이주, redirects, forward-only LFS | [I08](../../03_Issue_Analysis_Report/I08_UE5_Portfolio_Issue_Analysis_Report.md) |

## 3. 검증의 범위

코드·커밋과 당시 세션을 대조했고, 기존 감사 로그 및 사용자 PIE 정상 확인을 연결했다. 바이너리 파일의 존재만으로 내부 설정이 맞다고 판단하지 않았다. 리타게팅과 참조 조회의 근거 상태는 개별 보고서에서 구분한다. 환경의 충돌·조명은 사용자 설명에 근거한 간단한 작업 내역으로 한정하며 추가 증거 제출을 요구하지 않는다.

기술적으로는 새 에셋 규격에 맞춰 변환 책임과 종료 책임을 나눈 것이 핵심이다. 운영 측면에서는 Placeholder 리소스라도 실제 Material 사용을 확인한 뒤 보존한 판단이 대표 사례다. 성능 개선이나 모든 참조의 완전 검증은 주장하지 않는다.

후속 사용자 화면으로 Retargeter의 Spine 0.7 등 설정과 자체 Asset Reference Inspector의 Placeholder Referencers 조회를 확인했다. [화면 기록](../../98_Evidence/Stellar_Asset_Integration/Capture_Record.md)에 관찰 내용을 정리했으며, 이미지 원본 파일은 저장소에 포함하지 않았다.

## 4. 근거로 이동

문서별 역할은 구분한다. 작업 내역은 추가·이동·변경·제거를, 문제 해결 보고서는 증상과 선택 이유·결과를, 근거 목록은 출처와 확인 한계를, 설계 문서는 현재 책임과 실행 계약을 설명한다. 과거 PR의 당시 결과를 최신 계약으로 덮어쓰지 않는다.

- [통합 작업 내역 W07](../../01_Work_List/W07_Stellar_Asset_Integration/W07_UE5_Portfolio_Work_List.md)
- [주장별 근거 목록](../../98_Evidence/Stellar_Asset_Integration/README.md)
- [최종 Fix 기록 F08](../../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)
- [현재 Weapon 설계 S39](../../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)

---
