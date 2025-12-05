# UE5 Portfolio – Issue Checklist

### Date

- **Day 3**

- **Date : 2025.12.03**


---

### Objective

- CPlayerCharacter 이동(Idle/Walk/Run) 기본 로직 구현

- 점프 기능 및 이동 관련 파라미터 1차 튜닝

- AnimBP 기반 기본 Locomotion(Idle/Move) 뼈대 구성


### Branch

- feature/character-move-core


---

### TODO List

#### 1. 이동 / 점프 로직

- [x] CPlayerCharacter에 MoveForward / MoveRight 함수 추가

- [x] Project Settings > Input에 MoveForward / MoveRight Axis 매핑 등록

- [x] WASD 입력 기반 기본 이동 동작 확인

- [x] 점프 / 착지 구현 (Jump / StopJumping 바인딩)

- [x] MaxWalkSpeed 1차 값 설정


#### 2. 애니메이션 기본 세팅

- [x] Idle / Walk / Run용 애니메이션 리소스 선정 (임시 OK)

- [x] BP_CPlayerCharacter용 AnimBlueprint 생성

- [x] Speed 기반 Idle/Walk/Run BlendSpace 생성

- [x] AnimBP에서 Locomotion State(Idle/Move) 구성

- [x] CPlayerCharacter에서 AnimBP에 전달할 Speed 변수 값 세팅


#### 3. Git / Issue

- [x] Day3용 GitHub Issue 생성 (제목: `M1-03: CPlayerCharacter 이동/점프 & 기본 AnimBP 세팅`)

- [x] 작업 단위별 커밋 (커밋 메시지에 이슈 번호 포함)

- [x] D03 CheckList를 Obsidian에 복사하여 실제 진행 상황 체크


---

### Notes

- **컴포넌트 분리 설계**  
    이동 관련 책임을 명확히 하기 위해 `UCMovementComponent`를 별도로 구성하고, 캐릭터 이동에 필요한 모든 연산을 해당 컴포넌트에서 처리하도록 구조를 재정립함
    
- **애니메이션 자산 마이그레이션 이슈 및 해결 (M1-B01: #7)**  
	- 엔진 버전 차이로 인해 기존 프로젝트의 애니메이션만 그대로 가져올 경우 **스켈레톤 호환 오류 및 리타게팅 실패 문제가 발생함**
	- 이를 해결하기 위해 이전 프로젝트에서 **Mesh와 Skeleton만 최소 단위로 가져온 뒤**, 기존 애니메이션을 **현재 Mesh/Skeleton 구조에 맞게 리타게팅하는 방식으로 문제를 해결함** 
    
- **향후 로코모션 고도화 계획**  
    기본 로코모션 구현 이후, 8-Way 기반 방향 전환, 애니메이션 이동 거리 기반의 모션 보정, 발 동기화(Foot Sync) 등을 포함한 정교한 로코모션 시스템으로 확장할 예정임
    
- **ALS(Advanced Locomotion System) 도입 검토**  
    고급 이동 시스템 참조 및 기능 통합을 위해 ALS 기반 설계 일부를 도입하는 방향을 검토 중임


---
