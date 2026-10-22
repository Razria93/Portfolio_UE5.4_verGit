# Portfolio Production

이 폴더는 `Project Stellar` 제출 포트폴리오의 원본, 제작 기록, 캡처·다이어그램을 관리한다.

## 자료 상태 기준

2026-10-23, PR #133 병합 main `aeb9e7ed`를 기준으로 구현·문서 상태를 대조했다. 현재 저장소가 추적하는 HTML과 과거 초안의 19페이지 PPT 편집 계획, 로컬 PDF, 공개 영상 상태를 구분한다. PPT 원본의 현재 버전이나 새 PDF 게시 완료를 확인하지 않은 채 최신 제출본으로 지정하지 않는다.

## 추적 원본과 배포 경로

| Role | Canonical item | Policy |
| --- | --- | --- |
| Live web portfolio | [GitHub Pages](https://razria93.github.io/Portfolio_UE5.4_verGit/) | 추적 HTML·Assets의 배포 경로. 이번 접근 도구로 현재 공개 페이지 내용을 확인하지 못했으며 최신 배포 완료 판정은 보류한다. |
| Gameplay video | [Issue #119](https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/119) | 2026-10-23 API 조회에서 업로드 예정 본문·댓글 0. 아직 영상 공개 완료로 표시하지 않는다. |
| Submitted document source | [염동섭_UE5_Portfolio_Project_Stellar.html](염동섭_UE5_Portfolio_Project_Stellar.html) | 추적 HTML 원본. 표시 23페이지와 숨김 초안 2개를 구분한다. 새 PPT 원본과 동일 문서로 취급하지 않는다. |
| Visual assets | [Assets/](Assets/) | HTML의 상대 경로로 참조되는 캡처·다이어그램 원본이다. |
| Generated PDF | `output/pdf/UE5_Portfolio_Project_Stellar.pdf` | 기존 HTML용 로컬 파일 존재 확인. 내용·페이지 렌더링 및 최신 PPT와의 일치는 미검증이고 Git 원본으로 추적하지 않는다. |
| Static publication staging | `output/portfolio-pages/` | `Scripts/Publish/Publish-PortfolioDocument.ps1`로 생성한다. |

과거 보존 초안의 `portfolio_integrated_details_refined.pptx` 19페이지 구성은 편집 계획 참고다. 이번에 검색한 Codex 작업 영역 및 원본 output 영역에서는 해당 파일을 찾지 못했다. 다른 위치의 존재·최신 버전은 미확인이며, 파일 부재를 전체 컴퓨터에 대한 결론으로 확대하지 않는다.

## Submission scope

- 추적 HTML은 표지·인덱스·본문을 포함한 표시 23페이지다. 최종 제출물 선택과 공개 상태는 별도로 확인한다.
- p.24 `AI Workflow`와 p.25 `Traceability`는 향후 보완을 위한 숨김 초안이다. 표시·인쇄 페이지에서는 제외하지만 publish script가 HTML을 그대로 복사하므로 **정적 배포 HTML 소스에도 포함된다**. CSS 숨김은 비공개 처리가 아니며 공개에서 제외하려면 별도의 소스 제거·배포 변경이 필요하다. 이번 문서 PR은 배포 내용을 변경하지 않는다.
- 예전 `UE5_Portfolio_A4_Wireframe.html`, 25페이지 전환 계획, Word Handoff는 역사적 제작 기록이다. 현재 제출 원본으로 사용하지 않는다.

## Navigation

- [남은 구현·촬영·제출 작업](stellar_portfolio_completion_checklist.md)
- [영상 근거 제작 가이드](stellar_video_evidence_guide.md)
- [디버그 촬영 설정](stellar_capture_presets.md)
- [PIE 재현 시나리오와 완료 계약](../../06_notes/N_Portfolio_Replay_Scenarios.md)
- [순찰 전용 맵과 실제 BT 이동 근거](../../06_notes/N_Patrol_Replay_Scene.md)

- [Combat Motion Diagnostics](38_Gameplay_Combat_Motion_Diagnostics.md): 넉백·공격 시작 페이싱 관측 구조, 사용법, 변경 파일 및 PIE 확인 절차
- [Knockback and Attack Facing Review Guide](37_Gameplay_Knockback_and_Facing_Review_Guide.md): 통합 열람 순서·코드 지도·최신 검증·미확인 경계
- [Grounded Hit Knockback](35_Gameplay_Combat_Knockback_Implementation.md): 공격별 설정·데이터 전달·실행 소유권·이동 정리
- [Attack Start Facing](36_Gameplay_Attack_Start_Facing.md): 콤보별 일회성 방향 보정·내부 실행 세대·Root Motion 경계
- [Gameplay Combat HUD Plan](31_Gameplay_Combat_HUD_Design_and_Implementation_Plan%20(KR).md): 전체 HUD 및 현재 데이터 연결 범위
- [Gameplay Combat HUD Visual Spec](32_Gameplay_Combat_HUD_Visual_Spec.md): 배치·폰트·아이콘·TODO 초안
- [Gameplay Combat HUD Implementation](33_Gameplay_Combat_HUD_Implementation_and_Validation.md): 구현·자동검사·사용자 PIE 확인 범위
- [Gameplay Combat HUD Branch Closure](34_Gameplay_Combat_HUD_Branch_Closure.md): 브랜치 전체 변경·설계 판단·마감 검토·범위 밖 항목
- [Page Spec Index](00_Portfolio_Page_Spec_Index%20(KR).md): 페이지별 메시지·근거 계획의 역사적 기준
- [Portfolio Review Checklist](Portfolio_Review_Checklist%20(KR).md): 제출 전 점검 기록
- [Runtime Capture and Video Runbook](29_Portfolio_Runtime_Capture_and_Video_Runbook%20(KR).md): 캡처·영상 확보 절차
- [Documentation Maintenance TODO](../../01_Work_List/Documentation_Maintenance_TODO.md): 제출 이후 보완·동기화 작업의 단일 목록

## Publication rule

온라인 문서판은 제출 HTML을 직접 복사한 `index.html`과 필요한 `Assets/`로만 구성한다. 배포본을 직접 수정하지 않으며, 문서 원본을 수정한 뒤 publish script를 다시 실행한다. `main`에 Portfolio_Production 폴더 내 문서·원본·Assets 또는 publish script 변경을 push하면 GitHub Pages workflow가 배포본을 갱신한다.

이번 문서도 workflow의 경로 조건에 해당한다. 병합 전 staging 산출물이 기존 HTML·Assets와 같은지 확인한다. 문서 정리가 새 PPT/PDF·영상 게시를 수행하는 것은 아니다.
