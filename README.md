# UE5 액션 RPG 전투 시스템 포트폴리오

Unreal Engine 5.4 기반 3인칭 액션 RPG 전투 시스템 포트폴리오입니다.

본 프로젝트는 단일 전투 기능 구현보다 `Action`, `Reaction`, `Damage`, `Feedback`, `AI`가 서로 연결되는 실행 구조를 정리하는 데 초점을 둡니다. Player와 Enemy가 공통 component-driven 실행 구조를 공유하도록 구성하고, 전투 실행 흐름을 코드와 문서로 함께 관리합니다.

---

## 1. 프로젝트 정보

```yaml
프로젝트명: UE5 Action RPG Combat Portfolio
엔진: Unreal Engine 5.4
개발 환경: Visual Studio 2022
언어 / 구성: C++ / Blueprint
버전 관리: Git / GitHub
문서화: Markdown / Obsidian
목표 플랫폼: Windows PC / Unreal Editor 실행 기준
```

---

## 2. 구현 범위

현재 프로젝트는 다음 전투 시스템을 중심으로 구성되어 있습니다.

```yaml
Player
- Movement / Camera / Input
- Weapon Equip / Unequip
- Combo Attack
- Dodge
- Action 실행 흐름

Combat
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage 기반 Damage Pipeline
- Action / Reaction / Damage Feedback
- Hit / Dead Reaction

Enemy AI
- Behavior Tree / Blackboard
- AI Action Intent Dispatch
- Player / AI 공통 실행 흐름 연결

Documentation
- Issue Checklist
- Pull Request
- Bug Report
- System Architecture
- Technical Documents
- AI Workflow / Prompt Library
```

---

## 3. 핵심 설계 포인트

### Action / Reaction Execution Pipeline

Action과 Reaction을 공통 request / decision / apply / lifecycle 흐름으로 정리합니다.

### Execution Decision / Relationship / ApplyMode

실행 가능 여부, 현재 실행 중인 action / reaction과의 관계, 실제 적용 방식을 분리합니다.

### Cross-Domain Intervention

HitReaction이 Action을 interrupt하거나 DodgeAction이 Reaction을 cancel할 수 있는 구조를 다룹니다.

### Damage Pipeline

Unreal Engine 표준 `FDamageEvent`, `AActor::TakeDamage()` 흐름을 유지하면서 프로젝트 전용 hit context, damage result, reaction request, feedback request를 연결합니다.

### Data-Driven Resolve

Action, Reaction, Damage, Feedback을 key 기반으로 조회하고 실행 데이터와 executor를 resolve합니다.

### Montage Notify Timing Window

Chain, collision, feedback, intervention window를 montage timeline 위에서 제어합니다.

### Player / AI 공통 실행 구조

Player input과 AI Behavior Tree가 서로 다른 decision source를 가지더라도 실제 action / reaction 실행은 공통 component와 orchestrator 흐름을 사용합니다.

---

## 4. 문서 구조

### 제출용 기술 문서

[Docs/07_Technical_Documents](Docs/07_Technical_Documents)

포트폴리오 제출용으로 압축한 기술 설명 문서입니다.

### 시스템 구조 문서

[Docs/05_System_Architecture](Docs/05_System_Architecture)

전투 실행 구조, 책임 경계, orchestration, damage / feedback / reaction 흐름을 정리한 문서입니다.

### 작업 관리 문서

[Docs/01_Issue_CheckList](Docs/01_Issue_CheckList)

작업 단위별 목표, 범위, 체크 항목, 검증 기준을 기록합니다.

### Pull Request 문서

[Docs/04_Pull_Request](Docs/04_Pull_Request)

PR 단위 변경사항, 관련 문서, 검증 결과, 후속 작업을 정리합니다.

### Bug Report

[Docs/02_Bug_Report](Docs/02_Bug_Report)

구현 중 발생한 문제, 원인, 수정 내용, 검증 결과를 기록합니다.

### AI Workflow

[Docs/08_AI_Workflow](Docs/08_AI_Workflow)

Codex와 함께 작업하기 위한 AI 기반 작업 흐름, Prompt Library, Work Brief / Planning / Checklist 흐름을 정리합니다.

---

## 5. 대표 기술 문서

```yaml
포트폴리오 개요
-> Docs/07_Technical_Documents/T00_UE5 Portfolio Overview.md

프로젝트 기술 요약
-> Docs/07_Technical_Documents/T01_Project Technical Summary.md

전투 데이터 처리 파이프라인
-> Docs/07_Technical_Documents/T02_Combat Data Processing Pipeline.md

Action / Reaction 실행 파이프라인
-> Docs/07_Technical_Documents/T03_Action & Reaction Execution Pipeline.md

Enemy AI 전투 행동 설계
-> Docs/07_Technical_Documents/T04_Enemy AI Combat Behavior Design.md

Data-Driven 설계
-> Docs/07_Technical_Documents/T05_Data-Driven Design.md

Troubleshooting
-> Docs/07_Technical_Documents/T06_Trouble Shooting.md

AI 기반 개발 Workflow
-> Docs/07_Technical_Documents/T07_AI-Assisted Development Workflow.md
```

---

## 6. 실행 기준

1. Unreal Engine 5.4에서 `Portfolio.uproject`를 엽니다.
2. `PortfolioEditor Win64 Development` 기준으로 빌드합니다.
3. 테스트 레벨에서 Player / Enemy combat, damage, reaction, AI 흐름을 확인합니다.

---

## 7. 후속 확장 방향

```yaml
Combat
- Guard / Parry / Counter 판정
- Combat Resolution 계층 도입
- Resource / state system 고도화

Data
- DataAsset 기반 authoring 구조 정리
- Action / Reaction / Feedback data 확장

AI
- Boss pattern
- Enemy pattern data 확장
- AI decision source와 공통 execution pipeline 연결 강화

Feedback
- Damage feedback / reaction feedback 고도화
- VFX / SFX / camera feedback polish

Documentation
- System Architecture / Engine Technique 문서 체계 정리
- Technical Document 제출용 요약 보강
- AI Workflow 실사용 기반 refactor
```
