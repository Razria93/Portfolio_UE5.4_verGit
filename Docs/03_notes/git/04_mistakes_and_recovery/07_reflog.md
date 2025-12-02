## 파일 복구하기

#### 1. reflog 복구 패턴

##### 패턴 A. 구조용 브랜치를 만들어서 커밋의 생존성 보장

```bash
# 1) 특정 커밋을 구조용 브랜치로 붙잡기
git branch rescue <TargetHash>

# 2) N단계 전 HEAD 상태를 구조용 브랜치로 붙잡기
git branch rescue HEAD@{N}
```

##### 패턴 B. 브랜치를 **그 시점으로 되돌리기** (reset)

```bash
git reset --hard <TargetHash>
# 또는
git reset --hard HEAD@{N}
```

---
#### 2. reflog 복구 시나리오

##### 1. `reset --hard` 잘못 해서 커밋이 사라진 것처럼 보일 때

###### 복구 시나리오
###### 1. reflog 보기

```bash
git reflog
```
- 여기서 reset 하기 **직전 상태**(보통 `HEAD@{1}` 혹은 `HEAD@{2}`)를 찾음

예를 들어:
```bash
<ReflogHash> HEAD@{n}: commit: feat: 기본 이동 시스템 구현
```
###### 2. 그 시점으로 되돌리기
```bash
git reset --hard <ReflogHash>
# 또는
git reset --hard HEAD@{n}
```
- `reset --hard`로 지운 것처럼 보였던 커밋이 다시 돌아옴

---
##### 2. 브랜치를 삭제했는데, 그 브랜치의 커밋을 살리고 싶은 경우

###### 복구 시나리오
###### 1. 해당 브랜치의 마지막 CommitHash 찾기
```bash
git reflog           
git reflog show HEAD     # HEAD라는 ref의 log만 보여줌 (HEAD 포인터 이력)
git reflog --date=iso    # 날자 출력 포멧 옵션을 추가
git reflog --all         # 모든 ref의 log를 보여줌
```
###### 2. 새 브랜치로 붙잡기
```bash
git branch <NewBranchName_Restore> <ReflogHash>
```
- 삭제된 브랜치 내용을 다시 브랜치로 복구
- 브랜치가 CommitHash를 가리키는 포인터이기 때문에 
  위와 같이 명시해두면 브랜치를 삭제하지 않는 이상 Commit이 보존됨

---
##### 3. rebase 꼬여서 커밋이 이상해졌을 때
###### 복구 시나리오
###### 1. rebase 실행 전 커밋을 reflog로 검색
```bash
   git reflog
   
   # 조회 내역
   12ab34c HEAD@{2}: rebase: starting
   98ff321 HEAD@{3}: commit: 작업 전 마지막 커밋
   ```
   * `rebase: starting` 바로 **이전**이, 리베이스 시작 직전 상태
###### 2. 리베이스 이전 상태로 되돌리기
   ```bash
   git reset --hard HEAD@{3}
   # 또는 그 해시(98ff321)
   ```
- 리베이스하기 전 상태로 완전히 복구

---
##### 4. Detached HEAD 상태에서 커밋했다가 잃어버린 것 같을 때

###### 복구 시나리오
###### 1. --detach 상태에서 생성한 커밋을 reflog로 검색
```bash
git reflog

# 조회 내역
aa11bb2 HEAD@{1}: commit: 실험용 수정
cc33dd4 HEAD@{2}: checkout: moving to <해시>
```
- 이 `aa11bb2`가 해당 커밋

###### 2. 복구 :
```bash
git branch <NewBranchName_Restore> <ReflogHash>
```
- `NewBranchName_Restore` 브랜치가 (ReflogHash) 커밋을 가리키게 되어, 다시 정상 브랜치 위 커밋으로 사용 가능

---
