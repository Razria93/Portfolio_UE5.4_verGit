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

- [ ] CPlayerCharacter에 MoveForward / MoveRight 함수 추가

- [ ] Project Settings > Input에 MoveForward / MoveRight Axis 매핑 등록

- [ ] WASD 입력 기반 기본 이동 동작 확인

- [ ] 점프 / 착지 구현 (Jump / StopJumping 바인딩)

- [ ] MaxWalkSpeed / JumpZVelocity / AirControl 1차 값 설정


#### 2. 애니메이션 기본 세팅

- [ ] Idle / Walk / Run용 애니메이션 리소스 선정 (임시 OK)

- [ ] BP_CPlayerCharacter용 AnimBlueprint 생성

- [ ] Speed 기반 Idle/Walk/Run BlendSpace 생성

- [ ] AnimBP에서 Locomotion State(Idle/Move) 구성

- [ ] CPlayerCharacter에서 AnimBP에 전달할 Speed 변수 값 세팅


#### 3. Git / Issue

- [ ] Day3용 GitHub Issue 생성 (제목: `M1-03: CPlayerCharacter 이동/점프 & 기본 AnimBP 세팅`)

- [ ] 작업 단위별 커밋 (커밋 메시지에 이슈 번호 포함)

- [ ] D03 CheckList를 Obsidian에 복사하여 실제 진행 상황 체크


---

### Notes

- 


---
