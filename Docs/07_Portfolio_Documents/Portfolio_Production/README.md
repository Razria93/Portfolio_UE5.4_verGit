# Portfolio Production

이 폴더는 `Project Stellar` 제출 포트폴리오의 원본, 제작 기록, 캡처·다이어그램을 관리한다.

## Current canonical artifacts

| Role | Canonical item | Policy |
| --- | --- | --- |
| Live web portfolio | [GitHub Pages](https://razria93.github.io/Portfolio_UE5.4_verGit/) | 현재 제출 HTML과 Assets를 공개하는 배포본입니다. |
| Gameplay video | [Issue #119](https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/119) | 영상 업로드 전 안내 허브이며, 업로드 후 같은 링크에서 재생·타임스탬프를 관리합니다. |
| Submitted document source | [염동섭_UE5_Portfolio_Project_Stellar.html](염동섭_UE5_Portfolio_Project_Stellar.html) | 현재 제출 기준 HTML. 23개의 보이는 페이지를 가진다. |
| Visual assets | [Assets/](Assets/) | HTML의 상대 경로로 참조되는 캡처·다이어그램 원본이다. |
| Generated PDF | `output/pdf/UE5_Portfolio_Project_Stellar.pdf` | HTML에서 생성하는 산출물이며 Git 원본으로 추적하지 않는다. |
| Static publication staging | `output/portfolio-pages/` | `Scripts/Publish/Publish-PortfolioDocument.ps1`로 생성한다. |

## Submission scope

- 제출본은 표지·인덱스·본문을 포함한 23페이지다.
- p.24 `AI Workflow`와 p.25 `Traceability`는 향후 보완을 위한 숨김 초안으로 HTML 소스에만 보존하며, 현재 제출본·정적 배포본에는 포함하지 않는다.
- 예전 `UE5_Portfolio_A4_Wireframe.html`, 25페이지 전환 계획, Word Handoff는 역사적 제작 기록이다. 현재 제출 원본으로 사용하지 않는다.

## Navigation

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

온라인 문서판은 제출 HTML을 직접 복사한 `index.html`과 필요한 `Assets/`로만 구성한다. 배포본을 직접 수정하지 않으며, 문서 원본을 수정한 뒤 publish script를 다시 실행한다. `main`에 원본·Assets·publish script 변경을 push하면 GitHub Pages workflow가 배포본을 갱신한다.
