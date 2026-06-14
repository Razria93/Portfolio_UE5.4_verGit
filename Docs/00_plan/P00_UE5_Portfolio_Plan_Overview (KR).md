# UE5 Portfolio - 프로젝트 개요

본 문서는 `UE5 Action RPG Combat Portfolio`의 목표, 범위, 기술 방향, 문서 구조를 요약한다.

상세 구현 단계는 `P01_UE5_Portfolio_Milestones (KR).md`와 `P02_UE5 Portfolio_Development Roadmap (KR).md`에서 관리한다.

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

## 2. 프로젝트 목표

### 장르 목표

3인칭 액션 RPG 전투 시스템 포트폴리오.

### 구현 목표

```yaml
전투 실행 구조
- Player / Enemy 공통 Action 실행 흐름
- Reaction 실행 흐름
- Damage 처리 흐름
- Feedback 실행 흐름
- AI decision source와 실행 계층 연결

작업 방식
- Git / PR / Issue Checklist 기반 작업 관리
- Bug Report / System Architecture 기반 기록
- Portfolio Document 기반 제출용 설명
- AI Workflow / Prompt Library 기반 Codex 협업
```

---

## 3. 현재 구현 범위

```yaml
Player
- Movement / Camera / Input
- Weapon Equip / Unequip
- Combo Attack
- Dodge 기반 intervention 흐름

Combat
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage 기반 Damage Pipeline
- Hit / Dead Reaction
- Damage Feedback / Reaction Feedback

Enemy AI
- Behavior Tree / Blackboard
- Patrol / Chase / Attack
- Combat priority / waiting behavior
- Action intent dispatch

Documentation
- Issue Checklist
- Pull Request
- Bug Report
- System Architecture
- Portfolio Documents
- AI Workflow
```

---

## 4. 핵심 설계 방향

### Action / Reaction 실행 구조

Action과 Reaction을 공통 request / decision / apply / lifecycle 흐름으로 정리한다.

### Damage Pipeline

Unreal Engine 표준 `FDamageEvent`, `AActor::TakeDamage()` 흐름을 유지하면서 프로젝트 전용 hit context, damage result, reaction request, feedback request를 연결한다.

### Cross-Domain Intervention

Action, Reaction, Dodge, HitReaction 사이의 interrupt / cancel / block 관계를 명시적으로 다룬다.

### Data-Driven Resolve

Action, Reaction, Damage, Feedback을 key 기반으로 조회하고 실행 데이터와 executor를 resolve한다.

### Player / AI 공통 실행 구조

Player input과 AI Behavior Tree가 서로 다른 decision source를 가지더라도 실제 실행은 공통 component와 execution pipeline을 사용한다.

---

## 5. 문서 구조

```yaml
00_plan
-> 프로젝트 개요 / 마일스톤 / 개발 로드맵

01_Work_List
-> 작업 단위별 목표 / 체크 항목 / 검증 기준 / 작업 산출물

02_Bug_Report
-> 구현 중 발생한 문제 / 원인 / 수정 / 검증 기록

04_Pull_Request
-> PR 단위 변경 사항 / 검증 / 후속 작업 기록

05_System_Architecture
-> 시스템 구조 / 책임 경계 / 설계 결정 / 구조 변경 기록

07_Portfolio_Documents
-> 제출용 기술 문서

08_AI_Workflow
-> Codex 기반 작업 흐름 / Prompt Library / Work Brief / Planning

99_Legacy
-> 이전 Issue Checklist / 과거 문서 구조 보관
```

---

## 6. 제출용 기술 문서

```yaml
PF00
-> 프로젝트 개요

PF01
-> 프로젝트 기술 요약

PF02
-> 전투 데이터 처리 파이프라인

PF03
-> Action / Reaction 실행 파이프라인

PF04
-> Enemy AI 전투 행동 설계

PF05
-> Data-Driven 설계

PF06
-> Troubleshooting

PF07
-> AI 기반 개발 Workflow
```

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

Documentation
- Documentation Index / 문서군별 Index 갱신
- System Architecture / Engine Technique 문서 역할 분리
- 제출용 기술 문서 최종 검수
- AI Workflow 실사용 기반 refactor
```
