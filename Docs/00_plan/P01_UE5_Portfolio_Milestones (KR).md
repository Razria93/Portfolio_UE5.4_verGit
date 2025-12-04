# UE5 Portfolio – 마일스톤 & 타임라인

## **Keywords**

- 마일스톤 정의 및 버전 태그 계획
    
- 1개월(2025.12.01 ~ 12.31) 기준 주차별 목표
    
- 필수(MVP) 및 확장(Extended) 구분
    
- Git 태그 / Issue / PR 운영 기준
    

---
---
# **1. 버전 태그 / 마일스톤 개요**

## **M0 – 초기 환경 & 워크플로우 세팅**

**기간:** 2025.12.01 (Day 1)

### **내용**

- [ ] UE5.4 설치 및 신규 프로젝트 생성
    
- [ ] Git/GitHub 리포지토리 생성 및 `.gitignore` 정리
    
- [ ] Obsidian `docs/` 폴더 구조 구성
    - `00_plan`, `01_daily`, `02_design`, `03_notes`
        
- [ ] GitHub Issues / Projects(칸반) 기본 구성
    
- [ ] Git & GitHub 워크플로우 문서 추가
    
- [ ] Plan-Overview, Milestones, Issue-Checklist 문서 추가
    

**Git 태그:** `v0.0-setup`

---

## **M1 – 캐릭터 & 전투 코어 (v0.1)**

**기간:** Week 1 (2025.12.01 ~ 12.07)

### **내용**

- [ ] `PlayerCharacter` / `PlayerController` C++ 클래스 구현
    
- [ ] 3인칭 카메라(SpringArm) 구성
    
- [ ] 기본 이동(move/jump) 구현
    
- [ ] 무기 장착/해제 로직
    
- [ ] 기본 콤보 공격 시스템
    
- [ ] 전투 테스트용 “Test Room” 레벨 구성
    

### **완료 기준**

- [ ] 플레이어가 이동, 점프, 회피, 기본 콤보를 수행할 수 있음
    
- [ ] 무기 장착 상태에 따라 공격 가능 여부가 달라짐
    

**Git 태그:** `v0.1-character-combat-core`

---

## **M2 – 히트/데미지/더미 적/타게팅 (v0.2)**

**기간:** Week 2 (2025.12.08 ~ 12.14)

### **내용**

#### 근접 전투 판정

- [ ] Dummy Enemy 추가
    
- [ ] 근접 Hit 판정 구현
    
- [ ] 데미지 시스템 (HP 감소, 사망, 파괴 가능한 오브젝트 1종)
    
- [ ] 플레이어/적의 피격 및 사망 애니메이션 연동
    

#### 타게팅 시스템

- [ ] 기본 Lock-on 타게팅 시스템 구현
    

### **완료 기준**

- [ ] 플레이어 vs 더미 적 1:1 전투 루프 가능
    
- [ ] Lock-on 상태에서 카메라 및 캐릭터가 적을 안정적으로 추적
    

**Git 태그:** `v0.2-hit-damage-targeting`

---

## **M3 – Enemy AI & 고급 전투 (v0.3)**

**기간:** Week 3 (2025.12.15 ~ 12.21)

### **내용**

#### 적 AI (FSM)

- [ ] Idle
    
- [ ] Move
    
- [ ] Chase
    
- [ ] Attack
    
- [ ] Hit Reaction
    
- [ ] Death
    

#### 플레이어 고급 전투 시스템

- [ ] Guard / Guard Break
    
- [ ] Parry
    
- [ ] Dodge
    

#### 통합

- [ ] 적 AI와 플레이어 고급 전투 시스템을 상호작용 가능하도록 통합
    

### **완료 기준**

- [ ] 가드/패링/회피/공격이 모두 포함된 전투 루프 구성
    
- [ ] 1대 다수 상황에서도 큰 문제 없이 작동
    

**Git 태그:** `v0.3-enemy-ai-and-advanced-combat`

---

## **M4 – VFX & UI (v0.4)**

**기간:** Week 4 (2025.12.22 ~ 12.28)

### **내용**

#### VFX

- [ ] 공격 이펙트
    
- [ ] 피격 이펙트
    
- [ ] 간단한 파괴 이펙트
    

#### UI

- [ ] HP Bar
    
- [ ] Resource UI
    
- [ ] Damage UI
    

#### Demo

- [ ] 짧은 플레이어블 데모 구성
    

### **완료 기준**

- [ ] 전투, UI, VFX 피드백이 포함된 데모 플레이 가능
    

**Git 태그:** `v0.4-vfx-and-ui`

---

## **M5 – Polish & 문서화 (v0.5)**

**기간:** Week 5 (2025.12.29 ~ 12.31)

### **내용**

- [ ] 포트폴리오 시연 영상 촬영/업로드
    
- [ ] 기술/설계 문서 정리 (`docs/` 구조 기반)
    
- [ ] 확장 로드맵 작성
    

### **완료 기준**

- [ ] GitHub Repo + 영상 + 문서만으로 포트폴리오 설명 가능
    

**Git 태그:** `v0.5-polish-and-docs`

---

# **2. 태그 / 마일스톤 체크 테이블**

|Milestone|Tag Name|요약|목표 주차|완료|
|---|---|---|---|:-:|
|**M0**|`v0.0-setup`|초기 환경 설정 & 워크플로우 구성|W0||
|**M1**|`v0.1-character-combat-core`|캐릭터/카메라/이동/기본 콤보|W1||
|**M2**|`v0.2-hit-damage-targeting`|히트/데미지/더미 적/타게팅|W2||
|**M3**|`v0.3-enemy-ai-and-advanced-combat`|적 AI & 고급 전투 (가드/패링/회피)|W3||
|**M4**|`v0.4-vfx-and-ui`|VFX/UI/데모|W4~W5||
|**M5**|`v0.5-polish-and-docs`|문서화 & 마무리 작업|W5||

---

### 필요하면 아래도 추가 제작 가능

- 한국어 ↔ 영어 병기 버전
    
- Obsidian 전용 Backlink-friendly 버전
    
- “Extended Scope” (2026 이후 확장 계획)
    
- GitHub Wiki 최적화 버전
    

원하면 말해줘!