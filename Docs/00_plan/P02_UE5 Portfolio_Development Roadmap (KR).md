# UE5 Portfolio - 개발 로드맵

본 문서는 `UE5 Action RPG Combat Portfolio`의 구현 로드맵을 현재 프로젝트 기준으로 정리한다.

마일스톤은 큰 단계의 상태를 관리하고, 로드맵은 앞으로 어떤 순서로 구현 / 정리 / 검증을 진행할지 관리한다.

---

## 1. 로드맵 기준

```yaml
관리 기준
- 현재 구현된 기능을 기반으로 다음 작업 순서를 정리
- 전투 실행 구조와 문서화 작업을 함께 추적
- 후속 Branch에서 다룰 기능 / 문서 / 검증 범위 분리
```

```yaml
상태 기준
완료
-> 현재 코드와 문서 기준으로 닫힌 항목

진행 중
-> 구조 또는 문서는 있으나 구현 / 검증 / 정리가 남은 항목

다음 작업
-> 다음 Branch 또는 가까운 후속 작업에서 다룰 항목

후속 후보
-> 현재 우선순위에서 떨어져 있는 확장 후보
```

---

## 2. 현재 기준 로드맵 요약

```yaml
1. Player / 기본 전투 루프 및 Damage Pipeline
-> 완료

2. Enemy AI 전투 행동
-> 진행 중

3. 전투 Feedback 구축
-> 진행 중

4. Action Pipeline 고도화
-> 진행 중

5. Reaction Pipeline 고도화
-> 진행 중

6. Action / Reaction 실행 간섭 처리
-> 다음 작업

7. Guard / Parry / Counter
-> 다음 작업

8. 제출용 기술 문서 정리
-> 다음 작업

9. System Architecture / Engine Technique 문서 체계 정리
-> 다음 작업

10. AI Workflow 실사용 기반 Refactor
-> 후속 후보

11. Boss / Pattern / Advanced Combat
-> 후속 후보
```

---

## 3. 완료된 기반 작업

### Player / Weapon / 기본 공격

```yaml
상태
-> 완료

구성 항목
- Player Character / Controller
- SpringArm 기반 3인칭 카메라
- 기본 이동 / 점프 / 회피 기반
- Weapon Equip / Unequip
- Combo Attack
- Montage 기반 공격 실행
```

### 기본 전투 루프 및 Damage Pipeline 구축

```yaml
상태
-> 완료

구성 항목
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage 흐름
- Hit Reaction
- Dead Reaction
- 기본 Damage Feedback
```

---

## 4. 진행 중인 핵심 구조

### 전투 Feedback 구축

```yaml
상태
-> 진행 중

현재 방향
- Action Feedback 연결
- Reaction Feedback 연결
- Damage Impact Feedback 연결
- Player 화면 / 카메라 / UI Feedback 후보 정리
- Animation / VFX / SFX / camera feedback 연결 후보 정리

남은 항목
- Action / Reaction / Damage / Player Feedback 책임 경계 정리
- Feedback data authoring 구조 정리
- VFX / SFX / camera feedback polish
- System Architecture 문서 재정리
```

### Action Pipeline 고도화

```yaml
상태
-> 진행 중

현재 방향
- Action request / decision / apply / lifecycle 분리
- Action relationship / apply mode 정리
- Player / AI Action 실행 흐름 정리
- Action data resolve
- Action execution failure / rollback 기준 정리
- Montage lifecycle 기준 정리
- Action Feedback 연결

남은 항목
- AI action intent와 Action Pipeline 연결 고도화
- Action data authoring 구조 정리
- DataAsset 기반 authoring 구조 정리
```

### Reaction Pipeline 고도화

```yaml
상태
-> 진행 중

현재 방향
- Reaction request / policy / lifecycle 분리
- Reaction relationship / apply mode 정리
- Damage Result 기반 Reaction Request 연결
- Reaction data resolve
- Hit / Dead Reaction
- Reaction Feedback 연결
- Montage lifecycle 기준 정리

남은 항목
- Reaction policy / execution state 기준 고도화
- Enemy AI Reaction 관찰 / 복귀 흐름 정리
- Action / Reaction 실행 관계 최종 검증
- Combat Resolution 계층 도입
- Resource / state 처리 연결
- Guard / Parry / Counter 판정 결과 연결
- System Architecture 문서 재정리
```

### Enemy AI 전투 행동

```yaml
상태
-> 진행 중

현재 방향
- Behavior Tree / Blackboard 기반 AI 행동
- Patrol / Chase / Attack
- Combat priority / waiting behavior
- AI action intent dispatch

남은 항목
- Guard / Parry / Counter에 대한 AI 반응 연결
- Boss pattern / enemy pattern data 확장
- AI decision source와 execution pipeline 연결 고도화
```

---

## 5. 다음 구현 로드맵

### 5.1. Action / Reaction 실행 간섭 처리

```yaml
상태
-> 다음 작업

목표
- Action과 Reaction의 실행 관계 정리
- Execution Relationship Policy 정리
- Execution Intervention Case 정리
- interrupt / cancel / block / ignore 기준 정리
- Action / Reaction relationship matrix 작성
- Execution intervention policy 작성

검증 기준
- 기존 Combo / Dodge / HitReaction 회귀 확인
- Player / AI 공통 적용 가능성 확인
- Montage lifecycle / delegate 정리 기준 확인
```

### 5.2. Parry 구현

```yaml
상태
-> 다음 작업

목표
- 선입력 기반 Parry Action
- Parry Window
- Combat Resolution 기반 판정
- Parry 성공 시 Damage 무효화
- Parry Reaction interrupt
- Damage Feedback / Reaction Feedback 연결

준비 문서
- D20 Work Brief
- D20 Feature Work Planning
- D20 Work Checklist Draft

검증 기준
- Build
- Code Flow
- PIE
- Editor / Asset
```

### 5.3. Guard 구현

```yaml
상태
-> 다음 작업

목표
- Guard 입력 / 상태
- Guard 가능 조건
- Guard 성공 / 실패 처리
- Guard Break 후보 구조
- Damage / Resource / Feedback 연결
```

### 5.4. Counter 구현

```yaml
상태
-> 다음 작업

목표
- Counter 가능 조건
- Counter Action 실행
- Action / Reaction 관계 처리
- Feedback 연결
```

---

## 6. 문서 정리 로드맵

### README / 제출용 기술 문서

```yaml
상태
-> 다음 작업

대상
- README
- PF00 ~ PF07 Portfolio Documents
- Documentation Index / 문서군별 Index

목표
- 포트폴리오 첫 진입 문서 정리
- 제출용 기술 설명 압축
- 대표 문서 탐색 경로 정리
```

### System Architecture / Engine Technique

```yaml
상태
-> 다음 작업

목표
- 순수 System Architecture 설명 문서와 결정 / 이슈 기록 분리
- Engine Technique 설명 문서와 Engine Decision / Issue 기록 분리
- 기존 System Architecture 문서 재분류
- Architecture Decision Record / Architecture Issue Report 기준 정리
```

### AI Workflow

```yaml
상태
-> 후속 후보

목표
- D20 실제 구현 Branch에서 Work Brief / Planning / Checklist 흐름 재검증
- Prompt Flow / Routing 계층 보완
- Work Checklist 갱신 규칙 정리
- Document Writing Prompt 정리
```

---

## 7. 후속 확장 후보

```yaml
Advanced Combat
- Perfect Parry / Normal Parry
- Perfect Dodge
- Execution
- Aerial Attack
- Down Attack
- Skill System

Enemy / Boss
- Boss pattern
- Enemy pattern data
- Wave system
- Group combat

Animation / Movement
- Foot IK
- ALS-style locomotion
- Parkour
- Camera direction animation

VFX / UI
- Final hit VFX / SFX polish
- Damage UI
- Resource UI
- Camera shake / hit stop
```

---

## 8. 현재 우선순위

```yaml
1. README / P00 / P01 / P02 최신화
2. Documentation Index / 문서군별 Index 갱신
3. 제출용 Portfolio Documents PF00 ~ PF07 검수
4. System Architecture / Engine Technique 문서 체계 정리
5. Action / Reaction 실행 간섭 처리 기준 정리
6. Parry 구현
7. Guard / Counter 구현
8. AI Workflow 실사용 기반 refactor
```
