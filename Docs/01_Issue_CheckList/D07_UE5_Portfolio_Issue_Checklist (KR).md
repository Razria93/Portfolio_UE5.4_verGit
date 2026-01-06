# UE5 Portfolio – Issue Checklist

## 제목

**M2-02: Dummy Enemy 기본 클래스 구현 (피격 테스트 타깃)**

### 날짜

- **Day 7**

- **Date : 2025.12.23**


---

### 목표

- 피격 테스트용 Enemy 캐릭터를 구현하여 테스트 레벨에서 안정적으로 배치/검증 가능하도록 구성

- Mesh / Capsule / MovementComponent 기본값 및 Collision/Overlap 동작을 테스트 가능한 수준으로 정리

- 후속 이슈(M2-03~05)에서 Hit/Damage/HP/UI를 얹을 수 있는 최소 베이스 제공


---

### 브랜치

- feature/combat-hit-collision


---

### TODO List

#### 1. Enemy 기본 구성

- [x] Enemy(C++ 또는 BP 기반) 캐릭터 생성 (`CEnemy`)

- [x] Mesh / Capsule / MovementComponent 기본값 세팅

- [x] Collision/Overlap 프로파일 기본 정책 정리(피격 테스트 기준)


#### 2. 피격 테스트 준비(가시성)

- [x] 타깃 식별을 위한 최소 디버그 출력(이름/태그/로그 중 택1)

- [x] 오버랩/피격 진입 여부 확인용 임시 로그 출력(진입 확인 수준)


---
### Notes
- 


---