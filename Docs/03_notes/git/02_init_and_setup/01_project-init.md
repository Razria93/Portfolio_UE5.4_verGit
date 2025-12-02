## 프로젝트 초기화
#### 0. Project Initialize
##### **프로젝트 폴더 생성 및 초기화**
- **UnrealEngine의 경우**
	- **프로젝트 위치(ex. C : )를 루트로 하여 **프로젝트 이름(ex. Portfolio)으로 폴더를 생성하고 내부에 프로젝트를 초기화함
#### 1. Git Initialize
##### 순서
1. Git 저장소(.git 폴더)는 **프로젝트 폴더(ex. Portfolio)**가 루트여야 함
2. **Git bash 열기 (둘 중 하나)**
```bash
1. 프로젝트 폴더에서 'Open Git Bash here' 
2. Root 폴더에서 cd C:\(프로젝트 폴더)
```


-  **Git 저장소 초기화**
```bash
git init
```

#### 2. Git Config Initialize
##### **Global**
- **개인정보 설정**
```bash
git config --global user.name "MyName"
git config --global user.email "MyEmail"
```
		  
-  **기본 브랜치명 변경**
```bash
# 기본형
git config --global init.defaultBranch <DefaultBranchName>

# 일반적으로
git config --global init.defaultBranch main
```

#### 3. Remote Initialize

###### 1. 현재 Remote 확인
```bash
git remote -v
```
##### 2. Remote 추가 / 이름 변경 / 삭제
###### 1. 새 remote 추가
```bash
git remote add <RemoateName> <New_URL>
git remote -v
```
###### 2. remote 이름 변경
```bash
git remote rename <origin> <old-origin>
git remote rename <old-origin> <origin>
```
###### 3. remote 삭제
```bash
git remote remove <origin>
# 혹은
git remote rm <origin>
```
###### 주의!
- remote를 지운다고 해서 **원격 저장소가 삭제되는 건 아님**  
- 이 로컬 저장소에서 그 원격과의 연결만 끊는 것
##### 4. remote tracking 설정 
###### 최초 push 시
```bash
# 최초로 원격에 올릴 때 트래킹 설정
# 아래와 같이 설정할 경우 로컬 main ↔ 원격 origin/main 이 서로 연결(추적) 됨
git switch main         # tracking 설정할 Local의 브랜치
git push -u origin main # tracking 설정할 Remote의 <저장소>와 <브랜치>

# 이후에는
git push                # => 자동으로 origin main 으로
git pull                # => 자동으로 origin/main 에서
```
###### 이미 존재하는 브랜치에 수동으로 설정
```bash
# 현재 브랜치를 origin/main을 추적하도록 설정
git branch --set-upstream-to=origin/main
# 또는 브랜치 이름을 명시해서
git branch --set-upstream-to=origin/main main

# 이후에는
git push                # => 자동으로 origin main 으로
git pull                # => 자동으로 origin/main 에서
```
###### 현재 상태 확인하는 방법
```bash
git branch -vv
```

##### 5. Remote URL 바꾸기 (HTTPS ↔ SSH, 또는 새 주소)
###### 1. origin의 URL 변경
```bash
git remote set-url origin <New_URL>
```
###### 2. 변경 확인
```bash
git remote -v
```

---