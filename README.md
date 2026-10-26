# Project Stellar — UE5.4 근접 액션 프로젝트

가드·패링·회피와 다수 적 전투를 구현한 UE5.4 기반 근접 액션 프로젝트입니다. 전투 실행·판정·반응을 연결하고, 적의 인식과 전투 참여를 다룹니다.

[프로젝트 안내](https://razria93.github.io/site/stellar.html) · [프로젝트 PDF · 23쪽](https://razria93.github.io/files/pdf/stellar-review.pdf) · [플레이·구현·검증 영상](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/) · [통합 포트폴리오](https://razria93.github.io/)

![실제 플레이 중 Player가 Enemy의 공격을 패링하는 장면](https://razria93.github.io/site/assets/stellar-cover-parry.png)

*실제 플레이 · 패링 장면. 전투·AI 시연과 검증 조건은 프로젝트 안내에서 확인할 수 있습니다.*

## 대표 영상

전투 동작과 실행 처리, 다수 적의 역할을 살펴볼 수 있는 세 영상입니다.

| 영상 | 길이 | 확인할 내용 |
| --- | --- | --- |
| [가드·패링](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/guard-parry/) | 약 1분 14초 | 방어 판정과 양측의 후속 반응 |
| [실행 결정·중단·전환](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/execution-transition/) | 약 1분 9초 | 기존 실행의 중단·정리와 다음 반응 시작 |
| [다수 적 전투 참여](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/combat-participation/) | 약 1분 53초 | 같은 대상을 둔 적들의 교전·경계·관찰 역할 |

문제 해결 영상은 아래 해당 사례에서 바로 열 수 있습니다. [전체 영상 목록](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/)

## 프로젝트 개요

| 항목 | 내용 |
| --- | --- |
| 프로젝트 | UE5.4 기반 3인칭 Action RPG, Project Stellar |
| 기간 / 규모 | 2025.12 시작 · 개발 진행 중 / 1인 개발 · AI 협업 |
| 분야 | C++ Game Programmer · 게임플레이·전투·AI |
| 핵심 기능 | 콤보·가드·패링·회피·반격·처형, 공격·피격에 따른 실행·반응 전환, 다수 적 전투 역할 배정 |
| 구현 범위 | 전투·AI·피드백·HUD·디버그 도구, 외부 에셋 가공·설정과 게임플레이 통합 |
| Engine / Language | Unreal Engine 5.4 / C++ · Blueprint |
| 개발 환경 | Windows PC / Visual Studio 2022 |

## 작업 범위·기여

- **본인 작업:** 원하는 전투·AI 동작을 요구사항으로 구체화하고, 플레이 결과를 검토하며 보완 방향과 추가 확인 범위를 판단했습니다.
- **Codex 작업:** 코드 분석·구현과 자동 검증, 영상·문서 구성을 수행했습니다. 세부 구현의 최초 작성 범위는 확인된 기록을 기준으로 구분합니다.

외부 캐릭터·모션·무기·환경·VFX·효과음 등을 활용하고 게임플레이에 통합했습니다. 외부 에셋 자체의 제작과 프로젝트에서의 가공·설정·구현은 구분합니다. [프로젝트 안내의 자료 목록](https://razria93.github.io/site/stellar.html)에서 외부 에셋 출처와 구현 구조를 확인할 수 있습니다.

## 대표 구현

### 전투 실행·반응 전환

공격 중 피격됐을 때 반응의 중단 요구와 현재 공격의 중단 허용을 함께 확인합니다. 중단 조건을 충족하면 기존 공격의 충돌·트레일·실행 정보를 정리한 뒤 피격 반응을 시작합니다. 개별 피드백은 각자의 종료 기준을 따릅니다.

[플레이 사례·처리 구조 · PDF 16–17쪽](https://razria93.github.io/files/pdf/stellar-review.pdf#page=16) · [실행 전환 구조 설명](Docs/05_System_Architecture/S37_UE5_Portfolio_Execution_Transition_and_Intervention_Architecture.md) · [공격 실행 판단 코드](Source/Portfolio/Component/CActionOrchestratorComponent.cpp)

### 다수 적 전투 역할 배정

대상별 인식·피격 근거와 참여 상한으로 교전·경계·관찰 역할을 배정합니다. 참여 근거 수집 → 역할 배정 → 개별 AI 행동 결정으로 나뉘며, 역할을 배정받았다는 사실이 즉시 공격 실행을 뜻하지는 않습니다. 각 AI는 현재 실행 상태와 거리를 확인해 행동을 결정합니다.

[플레이 사례·설계 이유 · PDF 20–21쪽](https://razria93.github.io/files/pdf/stellar-review.pdf#page=20) · [전투 참여·역할 배정 정책](Docs/05_System_Architecture/S34_UE5_Portfolio_Combat_Participation_Policy.md)

### 런타임 관찰과 에디터 도구

Focus 대상의 전투·이동·실행 상태를 Debug Overlay와 Event Log로 관찰합니다. Editor Panel은 session-only CVar 설정과 PIE Focus 요청을 연결하며 게임플레이 상태를 변경하지 않습니다. Asset Reference Inspector와 Root Motion Tool의 세부 구조는 하단 기술 문서에 보존합니다.

[디버그 도구 · PDF 22–23쪽](https://razria93.github.io/files/pdf/stellar-review.pdf#page=22) · [디버그 HUD 구현 코드](Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp)

## 문제 해결

| 사례 | 핵심 결과 | 자료 |
| --- | --- | --- |
| 인식 지연 개선 | 같은 편 Enemy를 Sight 감지 단계에서 제외. 120마리 최초 유효 대상 인식 지연 p95: 20.816초 → 1.053초 | [PDF 5–7쪽](https://razria93.github.io/files/pdf/stellar-review.pdf#page=5) · [인식 지연 비교 영상](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/ai-perception/) |
| 피격 후 타깃 유지 | 마지막 피격 반응 종료 후 기록 유지 시간을 계산. 늦거나 중복된 종료 알림과 공격자별 만료 확인 | [PDF 8–10쪽](https://razria93.github.io/files/pdf/stellar-review.pdf#page=8) · [타깃 유지 영상](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/hit-target-ttl/) |
| 가드 종료 중 피격 전환 | 이미 비워진 가드 상태의 정리 요청도 수락. 피격 반응·기본 이동 속도 설정 복원·다음 가드 실행 확인 | [PDF 11–13쪽](https://razria93.github.io/files/pdf/stellar-review.pdf#page=11) · [가드 피격 전환 영상](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/guard-hit-transition/) |

각 결과는 아래 사례별 조건과 한계 안에서 해석합니다.

- **인식 지연:** 실행별 p95의 3회 중앙값이며 전체 프레임 시간 개선은 미확인입니다.
- **타깃 유지:** 공격자 소멸 후 대기 기록 정리 보완안은 원본 미반영입니다.
- **가드 검증:** 이동 복원은 기본 이동 속도 설정의 복원이며 달리기 동작 재개를 확인한 검사는 아닙니다.

## 검증 조건·한계

### 인식 지연 개선

- 2026.10.02 측정.
- 40·80·120마리, 전후 각 3회·회당 40초·동일 배치와 카메라·녹화 없이 측정.
- 수치는 실행별 p95의 3회 중앙값.
- 전체 프레임 시간 개선은 미확인.

**인식 지연 · 측정 환경**

- 엔진·실행 환경: UE 5.4.4 · Windows 11 · D3D12 · Development
- 하드웨어: Ryzen 9 7950X3D · RTX 4070 Ti

### 피격 후 타깃 유지

- 자동 검사 NullRHI 4그룹·19시나리오와 실제 Tracking 전환 시연을 구분.
- 수정 전은 정책 복원 시연이며 당시 바이너리가 아님.
- 기록 유지와 현재 타깃 선택은 별개.
- 공격자 소멸 후 대기 기록 정리 보완안은 원본 미반영.

### 가드 종료 중 피격 전환

- 격리 PIE와 자동 검사 24조건 × 25회, 정리 호출 총 600회를 구분.
- 600회는 실제 전투 횟수가 아님.
- Sprint 유지·800 복원과 수동 PIE 이펙트 동등성은 미확인.

**가드 검증 · 설정과 확인 범위**

- 이동 속도 설정: 실제 PIE는 **Run 600**, 자동 검사는 **Run 420·Sprint 620**
- 피해량: 기존 시연 **20**, 해당 검증 **300**
- 요청 실행: 재현 코드에서 실행
- 확인 범위: **기본 이동 속도 설정의 복원**을 확인했으며, 달리기 동작 재개를 확인한 검사는 아님

[별도 PIE 기록 · 가드 종료·피격·재진입](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/guard-pie/): 위 자동 반복 검사와 구분되는 실제 PIE 기록입니다.

> 각 결과는 기록 당시의 조건에 해당하며, 현재 main 전체를 재검증한 결과는 아닙니다.

## 관련 자료

- [프로젝트 안내](https://razria93.github.io/site/stellar.html): 대표 구현·문제 해결·출처·상세 검증 문서
- [통합 PDF · 62쪽, 스텔라 3–24쪽](https://razria93.github.io/files/pdf/combined-review.pdf#page=3): 프로젝트별 개별 표지를 제외해 묶은 통합본
- [영상 목록](https://razria93.github.io/Portfolio_UE5.4_verGit/videos/): 본편 14개와 별도 가드 PIE 1개
- [System Architecture](Docs/05_System_Architecture/00_System_Architecture_Index.md): 구조와 기술 판단의 문서 색인
- [실행 타입 정의](Source/Portfolio/Type/CExecutionTypes.h): 실행 판단·결과에 사용하는 타입

## 로컬 실행

### 저장소·에셋 준비

Git과 Git LFS를 설치한 뒤 아래 명령으로 저장소를 받습니다. 이미 받은 저장소가 있다면 해당 폴더에서 `git lfs pull`을 실행합니다. `.uasset`·`.umap`은 Git LFS 추적 대상이므로 프로젝트를 열기 전에 실제 에셋 파일을 내려받아야 합니다.

```powershell
git lfs install
git clone https://github.com/Razria93/Portfolio_UE5.4_verGit.git
cd Portfolio_UE5.4_verGit
git lfs pull
```

외부 에셋을 포함한 새 환경의 전체 실행은 이번 문서 정리에서 재검증하지 않았습니다. 누락 에셋이 있다면 [외부 에셋 출처·자료 안내](https://razria93.github.io/site/stellar.html)와 해당 라이선스·준비 조건을 확인합니다.

### 에디터에서 실행

1. Unreal Engine **5.4**와 Visual Studio 2022 C++ 개발 환경을 준비합니다.
2. `Portfolio.uproject`를 열고 필요 시 Visual Studio project files를 생성합니다.
3. `PortfolioEditor | Win64 | Development`로 빌드합니다.
4. Editor에서 `TestRoom`을 열어 Player / Enemy 전투와 Debug Overlay 흐름을 확인합니다.

포함된 Editor 플러그인은 `PortfolioDebugOverlayEditor`, `AssetReferenceInspector`입니다. 결과 확인은 [Web Portfolio](https://razria93.github.io/site/stellar.html)와 영상 안내를 우선으로 하며, 로컬 실행은 코드·도구 구조를 더 깊게 검토하기 위한 경로입니다.

## 저장소 공개 범위

- **공개 범위**: `Source/`, `Config/`, `Plugins/`, `Docs/` 및 프로젝트 구성을 검토하는 데 필요한 파일
- **Unreal binary assets**: 새 `.uasset`, `.umap`은 forward-only Git LFS 정책으로 추적하며, 기존 이력은 변환하지 않습니다.
- **생성·개인 환경 파일**: `Binaries/`, `Build/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.vs/`, `output/` 및 로컬 프로파일링 캡처는 저장소에서 제외합니다.
- **외부 에셋**: 외부 에셋의 재배포 권한은 개별 라이선스로 확인해야 하며, 이 README는 해당 에셋에 별도 라이선스를 부여하지 않습니다.

<details>
<summary>추가 기술 설명 · 실행 정책·디버그·Editor Tooling</summary>

### 1. Action / Reaction 실행 정책

**문제**: 새 요청이 현재 실행과 공존·연속·개입 중 어떤 관계인지 정하지 않으면, 다음 실행 시점과 기존 실행의 정리 책임이 모호해집니다.

**판단과 구현**: `Decision → Relationship → Apply`를 분리하고, `Independent / Sequential / Exclusive` 관계를 각각 `Start / Reserve / Intervene`으로 적용합니다. 거부·무시 경로도 별도로 유지합니다.

**근거**: [Intervention Policy 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#intervention-policy) · [Action Orchestrator 코드](Source/Portfolio/Component/CActionOrchestratorComponent.cpp) · [실행 타입 정의](Source/Portfolio/Type/CExecutionTypes.h)

### 2. Focus 기반 Runtime Debug Overlay

**문제**: 전투 중 분산된 Actor 상태와 실행 문맥을 같은 기준으로 대조하지 못하면, 결과 화면만으로는 원인을 확인하기 어렵습니다.

**판단과 구현**: Focus된 Actor의 전투·이동·실행 세션 상태와 Event Log를 읽기 전용 ViewData로 조립해 HUD에 표시합니다. Editor Panel은 session-only CVar 설정과 PIE Focus 요청만 연결하며 게임플레이 상태를 변경하지 않습니다.

**근거**: [Runtime Debug 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#runtime-debug) · [Overlay Editor 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#overlay-editor-plugin) · [HUD 코드](Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp)

### 3. 조사·복구 경로를 갖는 Editor Tooling

**문제**: Asset 관계 조사와 Root Motion 변환은 결과만 남기거나 원본을 바로 변경하면 후속 검토와 복구가 어려워집니다.

**판단과 구현**: Asset Reference Inspector는 Dependencies / Referencers Tree와 unused **검토 후보**를 CSV로 기록합니다. Root Motion Tool은 Apply 전 Track을 검증·백업하고 Revert에서 복구합니다.

**근거**: [Asset Inspector 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#asset-inspector) · [Root Motion Tool 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#root-motion-tool) · [Inspector 코드](Plugins/AssetReferenceInspector/Source/AssetReferenceInspector/Private/UI/SAssetReferenceInspectorWidget.cpp) · [Root Motion 코드](Source/PortfolioEditor/Private/Animation/CAnimModifier_TransferBodyMotionToRoot.cpp)

</details>

<details>
<summary>이전 제출본·개발 문서와 자료 관리</summary>

### 2026-10-23 제출 자료 보존

2026-10-23 제출 자료는 [23쪽 PDF](https://razria93.github.io/Portfolio_UE5.4_verGit/portfolio/files/project-stellar-20261023-v41.pdf)와 [PowerPoint](https://razria93.github.io/Portfolio_UE5.4_verGit/portfolio/files/project-stellar-20261023-v41.pptx), 영상 15개 및 공개 문서 9개로 구성됩니다. [자료 대응표](https://razria93.github.io/Portfolio_UE5.4_verGit/catalog.html)에서 관련 페이지와 근거를 찾을 수 있습니다. 검증 결과는 각 자료의 기록 시점과 조건을 따르며, 현재 main 전체의 재검증 결과를 뜻하지 않습니다. 기존 HTML 자료는 [이전 웹 포트폴리오](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html)에 보존합니다.

- [남은 구현과 제출 준비](Docs/07_Portfolio_Documents/Portfolio_Production/stellar_portfolio_completion_checklist.md)
- [영상 근거 제작 가이드](Docs/07_Portfolio_Documents/Portfolio_Production/stellar_video_evidence_guide.md)
- [디버그 촬영 설정](Docs/07_Portfolio_Documents/Portfolio_Production/stellar_capture_presets.md)
- [순찰 전용 재현 장면](Docs/06_notes/N_Patrol_Replay_Scene.md)

- [이전 통합 자료 안내](https://razria93.github.io/Portfolio_UE5.4_verGit/portfolio/) · [이전 통합 PDF · 62쪽](https://razria93.github.io/Portfolio_UE5.4_verGit/portfolio/files/yeom-dongseop-game-client-portfolio-20261023.pdf)

### 기존 개별 사이트 관리

제출 자료의 HTML·CSS·대응표·배포 규칙은 [portfolio-web](portfolio-web/README.md)에서 관리합니다. 영상과 문서 바이너리는 버전별 Release에 보존하고 Pages로 제공합니다. `Portfolio_Production` 및 PF00–PF07은 기존 제작·설계 기록으로 보존합니다.

- [Current submitted portfolio source and publication guide](Docs/07_Portfolio_Documents/Portfolio_Production/README.md)
- [Evidence ledger and documentation maintenance TODO](Docs/01_Work_List/Documentation_Maintenance_TODO.md)
- [Legacy technical notes (PF00–PF07)](Docs/07_Portfolio_Documents/00_Portfolio_Document_Index.md)

### 후속 근거 관리

- 영상 교체 시 버전별 Release와 `portfolio-web/release-manifest.json`, 공개 대응표를 함께 갱신합니다.
- HUD 샘플 자원 연동과 보스전은 별도 신규 기능 범위로 구성합니다. 기존 회피·처형 HUD·순찰 및 녹화 도구를 신규 구현으로 다시 집계하지 않습니다.
- 과거 40·80·120명 18회 측정 집계는 당시 조건의 자료로 선별하며 현재 main 재측정 결과로 표시하지 않습니다.

</details>
