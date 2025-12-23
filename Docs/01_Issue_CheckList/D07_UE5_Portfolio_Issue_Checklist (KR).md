# UE5 Portfolio – Issue Checklist

## 제목

**M2-02: Dummy Enemy 기본 클래스 & HP 구조 구현**

### 날짜

- **Day 7**

- **Date : 2025.12.23**


---

### 목표

- 피격 테스트용 Enemy 캐릭터 구현

- 플레이어와 공유 가능한 HP/Stat 구조 기초 설계

- Hit/Damage 시스템 연동을 위한 “피격 대상 컨테이너” 준비


### 브랜치

- feature/combat-hit-damage


---

### TODO List

#### 1. Enemy 기본 구성

- [ ] Enemy(C++ 또는 BP 기반) 캐릭터 생성 (`CEnemy`)

- [ ] Mesh / Capsule / MovementComponent 기본값 세팅

- [ ] 레벨에 Dummy Enemy 여러 개 배치


#### 2. HP / Stat 구조

- [ ] Enemy용 HP/Stat 구조(또는 컴포넌트) 정의

- [ ] Player와 공유 가능하거나 호환 가능한 형태로 설계

- [ ] HP 감소 / 0 이하 상태 플래그(Dead 등)만 우선 구현


#### 3. Hit 수신 준비

- [ ] “Damage/Hit를 받을 수 있는 인터페이스 또는 컴포넌트” 연결만 먼저 구현 (실제 계산 로직은 M2-03에서)

- [ ] 피격 시 간단한 반응(로그, 색 변경 등)만 임시로 붙여서 작동 여부 확인


---

### Notes
- 


---
