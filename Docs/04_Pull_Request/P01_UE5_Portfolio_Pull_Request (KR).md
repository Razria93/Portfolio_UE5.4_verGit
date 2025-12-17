# 캐릭터 / 컨트롤러 / 카메라 코어 및 TestRoom 레벨 세팅 구현

## 제목

✨ feat: 캐릭터 / 컨트롤러 / 카메라 코어 및 TestRoom 레벨 세팅 구현 (#4)

## 요약

- 이 PR은 초기 플레이 테스트를 위한 **PlayerCharacter / PlayerController 기본 구성**, **카메라 세팅**, 그리고 **TestRoom 테스트 레벨**을 구현함
    

---

## 완료 항목

### 1. TestRoom 레벨

-  TestRoom 레벨 생성
    
-  바닥/벽 및 간단한 블록아웃 프롭 추가 (박스, 계단 등)
    

### 2. 캐릭터 & 컨트롤러

-  `APortfolioPlayerController` C++ 클래스 추가
    
-  `APortfolioCharacter` C++ 클래스 추가
    
-  캡슐/스켈레탈 메시/무브먼트 컴포넌트 기본값 세팅
    

### 3. 카메라 시스템

-  `SpringArm` + `CameraComponent` 기반 카메라 구성
    
-  기본 카메라 높이, 숄더 오프셋, 거리 세팅
    
-  마우스 기반 카메라 회전 구현 (LookUp / LookRight 바인딩)
    

---

## 테스트 방법

1. 프로젝트 실행
    
2. 시작 맵으로 TestRoom이 로드되는지 확인
    
3. **WASD**로 이동
    
4. 마우스 이동으로 카메라 회전
    
5. 캐릭터 피벗 및 메시 방향(orientation) 확인
    
6. 충돌 및 캡슐 높이/감각이 적절한지 확인
    

---

## 관련 이슈 / 브랜치

- 브랜치: `feature/character-camera-core`
    
- 이슈: #4
    

---

## 노트

- 기본 캐릭터 및 카메라 세팅이 완료됨
    
- 현재 입력 시스템은 임시 구현이며, 코어 마일스톤 이후 **Enhanced Input**으로 전환 예정
    

---

## 다음 PR에서 진행할 항목

-  기본 이동(WASD) 및 점프 기능 구현
    
-  초기 AnimBP 구성(Idle/Walk/Run) 적용