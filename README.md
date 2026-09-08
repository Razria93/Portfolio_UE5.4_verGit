# Project Stellar — UE5.4 C++ Action RPG Combat Portfolio

`Project Stellar`는 Player와 Enemy가 공유하는 Action / Reaction 실행 구조를 설계하고, 런타임 관찰·검증 도구와 Editor 도구까지 구현한 Unreal Engine 5.4 C++ 액션 RPG 포트폴리오입니다.

[Web Portfolio](https://razria93.github.io/Portfolio_UE5.4_verGit/) · [Gameplay Video — 업로드 예정](https://github.com/Razria93/Portfolio_UE5.4_verGit/issues/119) · [Portfolio Production Docs](Docs/07_Portfolio_Documents/Portfolio_Production/README.md) · [System Architecture](Docs/05_System_Architecture/00_System_Architecture_Index.md)

## At a glance

| Area | What this repository demonstrates |
| --- | --- |
| Combat execution | Action / Reaction 요청을 Decision → Relationship → Apply로 분리하고, Independent / Sequential / Exclusive 관계에 따라 Start / Reserve / Intervene을 결정합니다. |
| Measurement & validation | Runtime Debug Overlay, Focus 기반 상태 관찰, Event Log, CSV profiling과 증거 문서로 구현 결과를 재현·검증합니다. |
| Editor tooling | session-only Overlay 제어 패널, Asset Reference Inspector, Root Motion Transfer 도구로 authoring·조사·검증 경로를 제공합니다. |

## Key implementation

### Shared combat execution

- Player 입력과 AI Behavior Tree가 공통 Action / Reaction Orchestrator로 요청을 전달합니다.
- 현재 실행과의 관계를 판정해 Independent는 즉시 시작하고, Sequential은 다음 실행을 예약하며, Exclusive는 기존 실행을 정리한 뒤 개입합니다.
- Combo, Guard, Parry, Dodge, Execution은 같은 요청·수락·적용 계약 위에서 동작합니다.

### Combat signal and damage pipeline

- Hit Context와 Source Payload를 구성해 Unreal Engine의 `FDamageEvent` / `AActor::TakeDamage()` 경로 위에 Target 처리 계층을 연결합니다.
- Target은 상태와 방어 규칙을 해석해 Normal / Guard / Parry 등의 Outcome을 확정하고, Reaction·Feedback 경로로 이어집니다.
- Montage Notify와 Notify State가 Combo chain, collision window, execution commit 등 시간 창을 제어합니다.

### AI and runtime observation

- Behavior Tree / Blackboard 기반 AI가 공통 실행 경로를 사용하며, Combat Participation과 Runtime LOD 정책으로 전투 참여와 갱신 비용을 관리합니다.
- Runtime Debug Overlay는 Focus 대상의 상태·실행 문맥·최근 Event를 읽기 전용 ViewData로 표시합니다.
- 캡처·CSV·문서 근거를 함께 남겨 구현 결과와 검증 조건을 추적합니다.

### Editor tools

- **Portfolio Debug Overlay Editor**: Nomad Panel에서 session-only 표시 옵션과 PIE Focus 요청을 준비합니다.
- **Asset Reference Inspector**: Dependencies / Referencers 관계 Tree, unused candidate scan, timestamped CSV export를 제공합니다.
- **Root Motion Tool**: Animation Modifier 기반으로 Apply 전 검증, Track backup, Revert 계약을 제공합니다.

## Run locally

1. Unreal Engine **5.4**와 Visual Studio 2022 C++ 개발 환경을 준비합니다.
2. `Portfolio.uproject`를 열고 필요 시 Visual Studio project files를 생성합니다.
3. `PortfolioEditor | Win64 | Development`로 빌드합니다.
4. Editor에서 `TestRoom`을 열어 Player / Enemy 전투와 Debug Overlay 흐름을 확인합니다.

프로젝트에는 `PortfolioDebugOverlayEditor`와 `AssetReferenceInspector` Editor 플러그인이 포함됩니다.

## Documentation

- [Current submitted portfolio source and publication guide](Docs/07_Portfolio_Documents/Portfolio_Production/README.md)
- [System Architecture Index](Docs/05_System_Architecture/00_System_Architecture_Index.md)
- [Evidence ledger and documentation maintenance TODO](Docs/01_Work_List/Documentation_Maintenance_TODO.md)
- [Legacy technical notes (PF00–PF07)](Docs/07_Portfolio_Documents/00_Portfolio_Document_Index.md)

현재 제출 기준은 `Portfolio_Production`의 23페이지 HTML과 GitHub Pages입니다. PF00–PF07은 이전 포트폴리오 설계 기록으로 보존합니다.

## Repository notes

- 새 Unreal binary asset(`.uasset`, `.umap`)은 forward-only Git LFS 정책으로 추적합니다. 기존 이력은 변환하지 않습니다.
- `output/`은 HTML 원본에서 생성하는 PDF·정적 배포 staging 경로이며 Git에서 추적하지 않습니다.

## Remaining roadmap

- DataAsset 기반 authoring 확장과 Counter / Boss pattern 설계
- AI pattern·Runtime LOD·profiling 범위 확장
- VFX / SFX / camera feedback polish와 추가 런타임 증거 확보
