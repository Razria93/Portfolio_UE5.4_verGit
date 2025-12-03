# UE5 Portfolio – Issue Checklist

### Date
- Day 2
- Date : 2025.12.02

### Objective
- PlayerCharacter / PlayerController C++ 클래스 생성
- 3인칭 카메라(SpringArm) 기본 세팅
- Test Room 기본 레벨 생성

### Branch
- feature/character-camera-core

---

### TODO List

#### 1. Test Room
- [x] `TestRoom` 레벨 생성
- [x] 바닥, 벽, 간단 장애물(박스/계단) 배치

#### 2. 캐릭터 & 컨트롤러
- [x] `APortfolioPlayerController` C++ 클래스 생성 및 프로젝트에 등록
- [x] `APortfolioCharacter` C++ 클래스 생성 및 DefaultPawnClass로 설정
- [x] 캡슐 / Mesh / MovementComponent 기본값 설정

#### 3. 카메라
- [x] SpringArm + CameraComponent 추가
- [x] 카메라 위치/각도(높이, 뒤쪽 거리) 기본값 세팅
- [x] 마우스 이동으로 카메라 회전

---

### Notes
- 프로젝트를 구성하는 것 보다 처음쓰는 commit 과 git을 관리하는게 훨씬 오래걸렸음
- 하지만 사용하고보니 강력하고 개발자에게는 반드시 필요한 기술이였다고 생각함
- 또한 Github에서 제공하는 issue/project 또한 프로젝트를 기획/관리하기 상당히 용이함

---
