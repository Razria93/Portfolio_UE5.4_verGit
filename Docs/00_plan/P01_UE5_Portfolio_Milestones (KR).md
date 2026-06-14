# UE5 Portfolio - 마일스톤

본 문서는 `UE5 Action RPG Combat Portfolio`의 주요 구현 단계와 현재 진행 상태를 정리한다.

현재 프로젝트 기준에서 구현 완료 / 진행 중 / 후속 확장 범위를 확인하기 위한 마일스톤 문서로 관리한다.

---

## 1. 마일스톤 기준

```yaml
관리 기준
- 기능 단위 구현 상태
- 전투 실행 구조 정리 상태
- 문서화 / 검증 상태
- 후속 확장 후보
```

```yaml
상태 기준
완료
-> 현재 코드와 문서 기준으로 주요 목표가 구현 / 정리된 상태

진행 중
-> 주요 구조는 있으나 보완 구현, 검증, 문서화가 남은 상태

후속
-> 이후 Branch에서 다룰 상태
```

---

## 2. 전체 마일스톤 요약

```yaml
M0. 프로젝트 환경 / 문서 기반 구성
-> 완료

M1. Player 기본 조작 / 무기 / 기본 공격
-> 완료

M2. 기본 전투 루프 및 Damage Pipeline 구축
-> 완료

M3. Enemy AI 전투 행동
-> 진행 중

M4. 전투 Feedback 구축
-> 진행 중

M5. Action Pipeline 고도화
-> 진행 중

M6. Reaction Pipeline 고도화
-> 진행 중

M7. Action / Reaction 실행 간섭 처리
-> 후속

M8. Guard / Parry / Counter 전투 판정
-> 후속

M9. 제출용 기술 문서 / 포트폴리오 정리
-> 진행 중

M10. AI Workflow / Prompt Library
-> 진행 중
```

---

## 3. M0 - 프로젝트 환경 / 문서 기반 구성

### 상태

완료

### 목표

Unreal Engine 프로젝트, Git / GitHub, Markdown 문서 체계를 구성한다.

### 완료된 항목

```yaml
구현 / 환경
- Unreal Engine 5.4 프로젝트 구성
- Visual Studio 2022 개발 환경 구성
- Git / GitHub 저장소 구성
- `.gitignore` 정리

문서
- Issue Checklist
- Pull Request
- Bug Report
- System Architecture
- Portfolio Documents
- AI Workflow
```

---

## 4. M1 - Player 기본 조작 / 무기 / 기본 공격

### 상태

완료

### 목표

Player 캐릭터의 기본 이동, 카메라, 무기 장착, 기본 공격 흐름을 구성한다.

### 완료된 항목

```yaml
Player
- Character / Controller 구성
- SpringArm 기반 3인칭 카메라
- 이동 / 점프 / 회피 기반 구성

Weapon
- 무기 장착 / 해제
- WeaponActor / Attachment 기반 무기 연결

Combat
- 기본 공격
- Combo Attack
- Montage 기반 공격 실행
```

---

## 5. M2 - 기본 전투 루프 및 Damage Pipeline 구축

### 상태

완료

### 목표

공격 충돌, 데미지 적용, 피격 반응, 사망 처리까지 이어지는 기본 전투 루프를 구성한다.

### 완료된 항목

```yaml
Combat
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage 흐름
- Damage Result 처리

Reaction
- Hit Reaction
- Dead Reaction

Feedback
- Damage Feedback
- Reaction Feedback
```

---

## 6. M3 - Enemy AI 전투 행동

### 상태

진행 중

### 목표

Enemy AI가 탐색, 추적, 공격, 대기, 반응을 수행하고 Player와 공통 전투 실행 구조로 연결되도록 구성한다.

### 현재 구성된 항목

```yaml
AI
- Behavior Tree
- Blackboard
- Patrol / Chase
- Attack intent dispatch
- Combat priority / waiting behavior
- Player 바라보기
```

### 남은 항목

```yaml
후속 구현 / 검증
- Guard / Parry / Counter와 Enemy AI 반응 연결
- Boss pattern / enemy pattern data 확장
- AI decision source와 공통 execution pipeline 연결 고도화
```

---

## 7. M4 - 전투 Feedback 구축

### 상태

진행 중

### 목표

Action / Reaction / Damage 결과를 플레이어가 인지할 수 있는 전투 Feedback으로 연결한다.

### 현재 구성된 항목

```yaml
Action Feedback
- Action 실행 시작 / 종료 Feedback
- Attack timing Feedback
- Montage event 기반 Feedback 연결

Reaction Feedback
- Hit / Dead Reaction Feedback
- Reaction 실행 결과와 Feedback 연결
- Animation / VFX / SFX 연결 후보

Damage Feedback
- 피격 위치 / 방향 기반 Feedback
- Damage Impact Feedback
- Feedback request / execution 구조

Player Feedback
- Player가 인지해야 하는 화면 / 카메라 / UI Feedback 후보
- 전투 결과를 읽을 수 있는 시각 / 청각 Feedback 후보
```

### 남은 항목

```yaml
- Action / Reaction / Damage / Player Feedback 책임 경계 정리
- Feedback data authoring 구조 정리
- VFX / SFX / camera feedback polish
- System Architecture 문서 체계 재정리
```

---

## 8. M5 - Action Pipeline 고도화

### 상태

진행 중

### 목표

Player / AI의 Action 실행 흐름을 request / decision / apply / lifecycle 구조로 고도화한다.

### 현재 구성된 항목

```yaml
Action
- Action request
- Action execution decision
- Action relationship / apply mode
- Action executor lifecycle
- Action data resolve
- Montage lifecycle 기준
- Action Feedback 연결
```

### 남은 항목

```yaml
- AI action intent와 Action Pipeline 연결 고도화
- Action data authoring 구조 정리
- Action execution failure / rollback 기준 정리
- DataAsset 기반 authoring 구조 정리
```

---

## 9. M6 - Reaction Pipeline 고도화

### 상태

진행 중

### 목표

Damage 결과, 상태 변화, 피격 반응, Feedback을 Reaction 실행 흐름으로 연결하고 고도화한다.

### 현재 구성된 항목

```yaml
Reaction
- Reaction request
- Reaction execution policy
- Reaction relationship / apply mode
- Reaction executor lifecycle
- Reaction data resolve
- Hit Reaction
- Dead Reaction
- Montage lifecycle 기준
- Reaction Feedback 연결
```

### 남은 항목

```yaml
후속 구현 / 검증
- Damage Result와 Reaction Request 연결 기준 정리
- Reaction policy / execution state 기준 고도화
- Enemy AI Reaction 관찰 / 복귀 흐름 정리
- Action / Reaction 실행 관계 최종 검증
- Combat Resolution 계층 도입
- Resource / state system 연결
- Guard / Parry / Counter 판정 결과 연결
- System Architecture 문서 체계 재정리
```

---

## 10. M7 - Action / Reaction 실행 간섭 처리

### 상태

후속

### 목표

Action과 Reaction이 동시에 실행되거나 서로 개입할 때의 interrupt / cancel / block / ignore 기준을 정리한다.

### 후속 범위

```yaml
Execution Relationship Policy
- 현재 실행 중인 Action과 신규 Action의 관계 처리
- 현재 실행 중인 Action과 Reaction의 관계 처리
- 현재 실행 중인 Reaction과 신규 Reaction의 관계 처리
- Action / Reaction 우선순위 판단
- interrupt / cancel / block / ignore 기준

Execution Intervention Case
- Dodge 기반 intervention
- HitReaction 기반 Action interrupt
- Parry Reaction 기반 Action 전환 후보
- ExecutionState 전환 기준

Verification
- 기존 Combo / Dodge / HitReaction 회귀 확인
- Player / AI 공통 적용 가능성 확인
- Montage lifecycle / delegate 정리 기준 확인
```

### 남은 항목

```yaml
- Action / Reaction relationship matrix 정리
- Execution intervention policy 작성
- Action / Reaction apply mode 기준 보강
- System Architecture 문서 체계 반영
```

---

## 11. M8 - Guard / Parry / Counter 전투 판정

### 상태

후속

### 목표

Stella Blade 스타일의 Guard / Parry / Counter 판정을 현재 전투 실행 구조 위에 확장한다.

### 후속 범위

```yaml
Guard
- Guard 입력 / 상태 / 판정
- Guard Break

Parry
- 선입력
- Parry Window
- Combat Resolution 기반 판정
- Parry Reaction interrupt
- Damage / Reaction Feedback 연결

Counter
- Counter 가능 조건
- Counter 실행 흐름
- Action / Reaction 관계 처리
```

### 관련 준비 문서

```yaml
D20
-> Parry Work Brief
-> Parry Feature Work Planning
-> Parry Work Checklist Draft
```

---

## 12. M9 - 제출용 기술 문서 / 포트폴리오 정리

### 상태

진행 중

### 목표

프로젝트 구조와 구현 의도를 평가자가 이해할 수 있도록 제출용 기술 문서와 README를 정리한다.

### 현재 구성된 항목

```yaml
Portfolio Documents
- PF00 Portfolio Overview
- PF01 Project Summary
- PF02 Combat Data Pipeline
- PF03 Action / Reaction Execution
- PF04 Enemy AI Combat Behavior
- PF05 Data-Driven Design
- PF06 Troubleshooting
- PF07 AI-Assisted Workflow

README
- 프로젝트 개요
- 구현 범위
- 핵심 설계 포인트
- 문서 탐색 경로
```

### 남은 항목

```yaml
후속 정리
- Project Stella 명칭 반영 여부 정리
- Documentation Index / 문서군별 Index 갱신
- System Architecture / Engine Technique 문서 역할 분리
- 제출용 기술 문서 최종 검수
```

---

## 13. M10 - AI Workflow / Prompt Library 구축

### 상태

진행 중

### 목표

Codex와 함께 작업하기 위한 AI 기반 작업 흐름과 Prompt Library를 구성한다.

### 현재 구성된 항목

```yaml
AI Workflow
- Index
- Overview
- Project Context
- Operation Guide
- Work Pipeline
- Backlog

Prompt Library
- Prompt Blueprint
- Working Rule
- Working Reference
- Work Planning
- Document Writing
- Review / Verification
- Git Operation

D20 검증
- Work Brief
- Feature Work Planning
- Work Checklist Draft
```

### 남은 항목

```yaml
- Prompt Flow / Routing 계층 재정리
- Work Brief / Planning / Checklist 필드 계약 정리
- Document Writing Prompt 정리
- Prompt 문장 품질 검수
- 실제 D20 구현 Branch에서 Workflow 재검증
```

---

## 14. 태그 후보

```yaml
v0.1-player-combat-core
-> Player / Weapon / 기본 공격

v0.2-basic-combat-damage-pipeline
-> 기본 전투 루프 및 Damage Pipeline 구축

v0.3-enemy-ai-combat
-> Enemy AI 전투 행동

v0.4-combat-feedback
-> 전투 Feedback 구축

v0.5-action-pipeline
-> Action Pipeline 고도화

v0.6-reaction-pipeline
-> Reaction Pipeline 고도화

v0.7-action-reaction-intervention
-> Action / Reaction 실행 간섭 처리

v0.8-guard-parry-counter
-> Guard / Parry / Counter 확장

v0.9-portfolio-docs
-> 제출용 기술 문서 / README 정리

v0.10-ai-workflow
-> AI Workflow / Prompt Library
```

---

## 15. 현재 우선순위

```yaml
1. 제출용 README / Portfolio Documents 정리
2. Documentation Index / 문서군별 Index 갱신
3. 기존 System Architecture 문서 체계 정리
4. Action / Reaction 실행 간섭 처리 기준 정리
5. Parry 구현
6. Guard / Counter 구현
7. 구현 결과 기반 Verification Log / PR Document 작성
```
