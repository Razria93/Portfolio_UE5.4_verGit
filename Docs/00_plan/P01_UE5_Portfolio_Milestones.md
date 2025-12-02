# UE5 Portfolio – Milestones & Timeline

## Keywords
- 마일스톤 정의 및 버전 태그 계획
- 1달(2025.12.01 ~ 12.31) 기준 주차별 목표
- 필수(MVP) / 확장(Extended) 구분
- Git 태그 / Issue / PR 연계 기준

---
## Details

### 1. 버전 태그 / 마일스톤 개요

#### M0 – Initial Environment & Workflow Setup

- **기간**: 2025.12.01 (Day 1)
  
- **내용**
	- [x] UE5.4 설치 확인, 새 프로젝트 생성
	- [x] Git / GitHub 리포지토리 생성 및 `.gitignore` 정리
	- [x] Obsidian `docs/` 보관함 구성 (`00_plan`, `01_daily`, `02_design`, `03_notes`)
	- [x] GitHub Issues & Projects(칸반 보드) 기본 세팅
	- [x] Git & Github 관련 문서 정리 및 추가
	- [x] UE5 Portfolio 관련 Plan-Overview/Milestones/Daily-Checklist 정리 및 추가
	  
- **Git 태그**: 없음 (준비 단계)

---

#### M1 – Character & Combat Core (v0.1)

- **기간(목표)**: W1 (2025.12.01 ~ 12.07)
  
- **내용**
	- [ ] PlayerCharacter / PlayerController C++ 클래스
	- [ ] 3인칭 카메라(SpringArm)
	- [ ] 기본 조작 구현 (이동 / 점프)
	- [ ] 단일 무기 장착/해제
	- [ ] 기본 공격 / 콤보 공격
	- [ ] Test Room(기본 전투 테스트용 레벨) 구성

- **완료 기준**
	- [ ] 플레이어가 Test Room에서 움직이고, 회피하고, 기본 콤보를 연결할 수 있음
	- [ ] 무기 장착/해제 상태에 따라 공격 가능 여부가 달라짐

- **Git 태그**
  - `v0.1-character-combat-core`

---

#### M2 – Hit, Damage, Dummy Enemy, Targeting (v0.2)

- **기간(목표)**: W2 (2025.12.08 ~ 12.14)

- **내용**
	- [ ] 근접 히트 판정(피격 판정) + 디버그 드로잉
	- [ ] 데미지 시스템(HP 감소 + 사망 + 파괴 가능한 오브젝트 최소 1종)
	- [ ] Dummy Enemy(더미 적) – 피격/사망 애니, HP 연동
	- [ ] Lock-on(타게팅 시스템) 기본 구현

- **완료 기준**
	- [ ] 플레이어 vs 더미 적 1:1 전투 루프 가능
	- [ ] Lock-on 상태에서 카메라/캐릭터가 적 방향을 추적

- **Git 태그**
  - `v0.2-hit-damage-targeting`

---

#### M3 – Enemy AI & Advanced Combat (v0.3)

- **기간(목표)**: W3 (2025.12.15 ~ 12.21)
  
- **내용**
	- [ ] 적 AI 기본 FSM
		- [ ] 대기
		- [ ] 이동
		- [ ] 추적
		- [ ] 공격
		- [ ] 피격
		- [ ] 사망
	
	- [ ] 플레이어 고급 전투 시스템
		- [ ] 가드 / 가드브레이크
		- [ ] 패링
		- [ ] 회피
	
	- [ ] 적 AI와 플레이어 고급 전투 시스템이 상호작용하도록 통합

- **완료 기준**
	- [ ] 플레이어와 적이 서로 공격/방어/회피/패링하는 전투 루프가 구성됨
	- [ ] 간단한 1:다수(복수의 적) 상황에서도 큰 문제 없이 동작

- **Git 태그**
  - `v0.3-enemy-ai-and-advanced-combat`

---

#### M4 – VFX & UI (v0.4)

- **기간(목표)**: W4 (2025.12.22 ~ 12.28)

- **내용**
	- [ ] VFX
		- [ ] 공격 파티클
		- [ ] 피격 파티클
		- [ ] 간단 파괴 이펙트
	
	- [ ] UI
		- [ ] 체력바
		- [ ] 리소스 UI
		- [ ] 데미지 UI
	
	- [ ] 데모 게임 구성 및 실행
	
- **완료 기준**
	- [ ] 짧은 데모게임 실행 및 동작
	
- **Git 태그**
  - `v0.4-vfx-and-ui`

---

#### M5 – Polish & Documentation (v0.5)

- **기간(목표)**: W5 (2025.12.29 ~ 12.31)

- **내용**
	- [ ] 포트폴리오 문서화
		- [ ] 시연 영상 캡쳐 및 업로드
		- [ ] 기술 문서 / 설계 문서 정리 (`docs/` 하위)
		- [ ] 확장 요소 및 추가 작업 계획 문서 작성
	
- **완료 기준**
	- [ ] GitHub 레포 + 영상 + 문서만으로도 포트폴리오 설명 가능
	
- **Git 태그**
  - `v0.5-polish-and-docs`

---
## Tag/Milestone Check Table

| Milestone | Tag Name                    | 내용 요약                      | 목표 주차 | 완료일시 |
| --------- | --------------------------- | -------------------------- | ----- | :--: |
| M0        | -                           | 환경 세팅 & 워크플로우              | W0    |      |
| M1        | v0.1-character-combat-core  | 캐릭터/카메라/이동/회피/<br>기본공격/콤보  | W1    |      |
| M2        | v0.2-hit-damage-targeting   | 히트/데미지/적/타게팅               | W2    |      |
| M3        | v0.3-ai-and-advanced-combat | 적 AI + 고급 전투<br>(가드/패링/회피) | W3    |      |
| M4        | v0.4-polish-and-docs        | VFX/UI/메뉴/문서화              | W4~W5 |      |

---
