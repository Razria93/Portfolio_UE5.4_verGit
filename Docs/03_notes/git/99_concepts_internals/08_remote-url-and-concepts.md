#### 1. Remote 개념
- 이 Local 저장소가 연결되어 있는 **원격(Remote) 저장소의 별칭 + 주소**

보통:
- 이름: `origin` (관례상 기본 이름)
- URL: HTTPS or SSH

---
#### 2-1. 현재 Remote 확인
```bash
git remote -v
```
##### HTTPS 출력 예:
```bash
origin  https://github.com/Razry/UE5_Portfolio.git (fetch)
origin  https://github.com/Razry/UE5_Portfolio.git (push)
```
##### SSH 출력 예 :
```bash
origin  git@github.com:Razry/UE5_Portfolio.git (fetch)
origin  git@github.com:Razry/UE5_Portfolio.git (push)
```

---
#### 2-2. Remote URL 바꾸기 (HTTPS ↔ SSH, 또는 새 주소)
##### 1. origin의 URL 변경
```bash
git remote set-url origin <새_URL>
```
###### 예시:
- HTTPS → SSH로 변경:
```bash
git remote set-url origin git@github.com:Razry/UE5_Portfolio.git
```    
- GitHub 계정/레포 이름을 바꾼 뒤:
```bash
git remote set-url origin https://github.com/Razry/NewRepoName.git
```    

##### 2. 변경 확인
```bash
git remote -v
```

---
#### 2-3. Remote 추가 / 이름 변경 / 삭제
##### 1. 새 remote 추가 (예: fork or 백업용)
```bash
git remote add upstream https://github.com/OriginalOwner/Repo.git
git remote -v
```
###### 이렇게 한다면
- `origin` → 내 GitHub
- `upstream` → 원본 저장소 (위에서 설정)

##### 2. remote 이름 변경
```bash
git remote rename origin old-origin
git remote rename old-origin origin
```
- 이건 잘 안 쓰이긴 하는데, 기존 이름 헷갈릴 때 사용.

##### 3. remote 삭제

```bash
git remote remove origin
# 혹은
git remote rm origin
```
###### 주의!
- remote를 지운다고 해서 **원격 저장소가 삭제되는 건 아님**  
- 이 로컬 저장소에서 그 원격과의 연결만 끊는 것

---
#### 2-4. Remote + 브랜치 조합에서 자주 쓰는 패턴

##### 1. 원격 주소 바꾸고 계속 작업 이어가기
```bash
git remote -v                               # 현재 확인
git remote set-url origin git@github.com:Razry/UE5_Portfolio.git # 주소 변경
git remote -v                               # 변경 확인
git fetch origin                            # 새 주소에서 잘 받아와지는지 확인
git push origin main                        # 정상 푸시 되는지 확인
```

##### 2.  fork 구조에서 upstream 추가
```bash
# 내 repo는 origin으로 연결된 상태
git remote add upstream https://github.com/Original/UE5_Project.git

git fetch upstream
git checkout main
git merge upstream/main        # 또는 rebase upstream/main
```

---
