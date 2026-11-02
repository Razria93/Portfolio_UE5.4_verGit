# 실행 정책·디버그·에디터 도구

[프로젝트 README](../../README.md)

<a id="execution-policy"></a>

### 1. Action / Reaction 실행 정책

**문제**: 새 요청이 현재 실행과 공존·연속·개입 중 어떤 관계인지 정하지 않으면, 다음 실행 시점과 기존 실행의 정리 책임이 모호해집니다.

**판단과 구현**: `Decision → Relationship → Apply`를 분리하고, `Independent / Sequential / Exclusive` 관계를 각각 `Start / Reserve / Intervene`으로 적용합니다. 거부·무시 경로도 별도로 유지합니다.

**근거**: [Intervention Policy 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#intervention-policy) · [Action Orchestrator 코드](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/main/Source/Portfolio/Component/CActionOrchestratorComponent.cpp) · [실행 타입 정의](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/main/Source/Portfolio/Type/CExecutionTypes.h)

<a id="runtime-observation"></a>

### 2. Focus 기반 Runtime Debug Overlay

**문제**: 전투 중 분산된 Actor 상태와 실행 문맥을 같은 기준으로 대조하지 못하면, 결과 화면만으로는 원인을 확인하기 어렵습니다.

**판단과 구현**: Focus된 Actor의 전투·이동·실행 세션 상태와 Event Log를 읽기 전용 ViewData로 조립해 HUD에 표시합니다. Editor Panel은 session-only CVar 설정과 PIE Focus 요청만 연결하며 게임플레이 상태를 변경하지 않습니다.

**근거**: [Runtime Debug 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#runtime-debug) · [Overlay Editor 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#overlay-editor-plugin) · [HUD 코드](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/main/Source/Portfolio/Core/Debug/CDebugOverlayHUD.cpp)

<a id="editor-tooling"></a>

### 3. 조사·복구 경로를 갖는 Editor Tooling

**문제**: Asset 관계 조사와 Root Motion 변환은 결과만 남기거나 원본을 바로 변경하면 후속 검토와 복구가 어려워집니다.

**판단과 구현**: Asset Reference Inspector는 Dependencies / Referencers Tree와 unused **검토 후보**를 CSV로 기록합니다. Root Motion Tool은 Apply 전 Track을 검증·백업하고 Revert에서 복구합니다.

**근거**: [Asset Inspector 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#asset-inspector) · [Root Motion Tool 상세](https://razria93.github.io/Portfolio_UE5.4_verGit/legacy-portfolio.html#root-motion-tool) · [Inspector 코드](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/main/Plugins/AssetReferenceInspector/Source/AssetReferenceInspector/Private/UI/SAssetReferenceInspectorWidget.cpp) · [Root Motion 코드](https://github.com/Razria93/Portfolio_UE5.4_verGit/blob/main/Source/PortfolioEditor/Private/Animation/CAnimModifier_TransferBodyMotionToRoot.cpp)
