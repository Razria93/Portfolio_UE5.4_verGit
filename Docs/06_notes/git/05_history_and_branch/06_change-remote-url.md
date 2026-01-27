## 원격 저장소 관리
#### 1. 현재 Remote 확인
```bash
git remote -v
```

---
#### 2. Remote 추가 / 이름 변경 / 삭제
##### 1. 새 remote 추가
```bash
git remote add <RemoateName> <New_URL>
git remote -v
```
##### 2. remote 이름 변경
```bash
git remote rename origin old-origin
git remote rename old-origin origin
```
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
#### 3. Remote URL 바꾸기 (HTTPS ↔ SSH, 또는 새 주소)
##### 1. origin의 URL 변경
```bash
git remote set-url origin <New_URL>
```
##### 2. 변경 확인
```bash
git remote -v
```


---
#### 4. Remote + 브랜치 조합에서 자주 쓰는 패턴

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
