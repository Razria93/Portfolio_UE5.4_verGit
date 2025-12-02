#### 1. reflog의 개념
##### 1) 일반 `git log` vs `git reflog`

###### `git log`
  * **현재 브랜치가 가리키는 커밋 그래프**를 보여줌
  * 브랜치에서 `연결이 끊긴 커밋 (더 이상 어떤 브랜치에서도 가리키지 않는 commit)`은 안 나옴
###### `git reflog`
  * HEAD(또는 특정 브랜치)가 **과거에 가리켰던 모든 커밋 이동 기록**을 보여줌
  * `commit`, `reset`, `rebase`, `checkout`, `merge`, `cherry-pick` 등으로 HEAD/브랜치가 어디를 가리키는지 바뀔 때마다 **한 줄씩 기록**됨
  * 브랜치에서 끊겨서 안 보이는 커밋도 reflog 안에는 기록이 남아 있음
###### 중요한 포인트:
* reflog는 **로컬에만 있음** (원격과 공유 안 됨)
* 나중에 `git gc`(garbage collection)가 오래된 것들을 실제로 지우기 전까지는 reflog를 통해 웬만한 실수는 복구가 가능함

---
#### 2. 기본 사용법: `git reflog` 보는 법

```bash
git reflog
```

예시 출력(형식만 보시면 됨):

```bash
ab12cd3 (HEAD -> main) HEAD@{0}: commit: fix: 이동 시스템 버그 수정
9f8e7d6 HEAD@{1}: commit: feat: 기본 이동 시스템 구현
1234abc HEAD@{2}: reset: moving to HEAD~1
56789ff HEAD@{3}: checkout: moving from main to feature/move-system
...
```

각 줄을 해석하면:

* `HEAD@{0}` : **현재 상태**
* `HEAD@{1}` : 바로 직전 상태
* `HEAD@{2}` : 그 전 상태 … 이런 식

###### 중요한포인트 :
- **reset/checkout/rebase 하기 전에 HEAD가 가리키던 커밋을 
- `HEAD@{n}` 형태로 다시 가리킬 수 있음

---
#### 3. 복구의 핵심 패턴

##### 패턴 A. 구조용 브랜치를 만들어서 커밋의 생존성 보장

```bash
# 1) 특정 커밋을 구조용 브랜치로 붙잡기
git branch rescue <TargetHash>

# 2) N단계 전 HEAD 상태를 구조용 브랜치로 붙잡기
git branch rescue HEAD@{N}
```

- 현재 switch된 브랜치나 워킹 디렉토리는 건드리지 않고, 
  (`TargetHash`) 또는 `HEAD@{N}`이 가리키던 커밋을 **rescue라는 새 브랜치로 참조(ref)를 하나 더 만들어 둠
- 브랜치가 커밋을 가리키는 동안, 그 커밋은 절대로 Dangling(어디서도 참조되지 않는 상태) 이 되지 않으므로 git gc에 의해 삭제되지 않음
- 즉, rescue 브랜치는 `이 커밋을 절대 잃어버리지 않겠다`는 안전용(구조용) 손잡이 역할

---
##### 패턴 B. 브랜치를 **그 시점으로 되돌리기** (reset)

```bash
git reset --hard <TargetHash>
# 또는
git reset --hard HEAD@{N}
```

* HEAD, Index, WorkingDirectory 모두 (TargetHash)에 맞게 변경함
* **현재 작업 내용 날려도 상관없을 때** 사용하는 방식

---
#### 4. 상황별 reflog 복구 시나리오
##### 1. `reset --hard` 잘못 해서 커밋이 사라진 것처럼 보일 때

###### 상황 예시 :
`git reset --hard HEAD~2` 했더니
최근 2개 커밋이 날아간 것처럼 보임.

###### 복구 시나리오
###### 1. reflog 보기

```bash
git reflog
```
- 여기서 reset 하기 **직전 상태**(보통 `HEAD@{1}` 혹은 `HEAD@{2}`)를 찾습니다.

예를 들어:
```git
9f8e7d6 HEAD@{1}: commit: feat: 기본 이동 시스템 구현
```
###### 2. 그 시점으로 되돌리기
```bash
git reset --hard 9f8e7d6
# 또는
git reset --hard HEAD@{1}
```
- `reset --hard`로 지운 것처럼 보였던 커밋이 다시 돌아옴

---
##### 2. 브랜치를 삭제했는데, 그 브랜치의 커밋을 살리고 싶은 경우

###### 상황 예시 :
```bash
git branch -D feature/move-system
```
- 했는데, 나중에 보니 그 브랜치에 있던 작업이 필요함.

###### 복구 시나리오
###### 1. 해당 브랜치의 마지막 CommitHash 찾기
```bash
git reflog           
git reflog show HEAD     # HEAD라는 ref의 log만 보여줌 (HEAD 포인터 이력)
git reflog --date=iso    # 날자 출력 포멧 옵션을 추가
git reflog --all         # 모든 ref의 log를 보여줌
```
- 위 명령어들을 기반으로 그 브랜치가 마지막으로 가리키던 커밋 해시를 찾음
###### ref에 대한 개념정리
- `ref` : 
	- 이름 있는 포인터(HEAD, 브랜치, 원격 브랜치 등) → 커밋 해시 한 개를 가리킴
- `branch` : 
	- `refs/heads/*`에 있는 ref. 
	- 특정 `Commit`에 대해서 임의로 만들수 있는 `ref`이며 `Labeling`을 할 수 있음
- `HEAD :`
	- `내가 지금 작업 중인 ref`를 가리키는 `특별 ref`
- `reflog` : 
	- 각 `ref`가 과거에 어떤 해시를 가리켰는지에 대한 로컬 히스토리
   
###### 2. 새 브랜치로 붙잡기
```bash
git branch <NewBranchName_Restore> <ReflogHash>
```
- 삭제된 브랜치 내용을 다시 브랜치로 복구
- 브랜치가 CommitHash를 가리키는 포인터이기 때문에 
  위와 같이 명시해두면 브랜치를 삭제하지 않는 이상 Commit이 보존됨

---
#### 5. rebase 꼬여서 커밋이 이상해졌을 때

###### 상황 예시 :
```bash
git rebase main
```
- 하다가 충돌 정리 이상하게 해서 히스토리가 망가진 느낌일 때
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
#### 6. Detached HEAD 상태에서 커밋했다가 잃어버린 것 같을 때

###### 상황 예시 :
```bash
git switch --detach <CommitHash>
# 작업 후
git commit -m "CommitMessage"
git switch main
```
- `--detach` 상태로 commit할 경우 해당 commit은 어떤 브랜치에도 안붙어있음
	- `--detach` 에서 임시로 생성된 branch는 해당 commit만 가리키고 있기 때문
	- 하지만 reflog에는 남음

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
#### 5. 주의할 점
##### 1. **reflog는 로컬에만 있음**
* 다른 사람이랑 공유 안 됨.
* 다른 PC에서는 네 reflog 기록을 볼 수 없음.

##### 2. **영원히 남지 않는다**
* Git은 일정 시간이 지나면, 브랜치·태그에서 **더 이상 참조하지 않는 커밋들을** `git gc`를 통해 실제로 삭제함
* 보통 기본 설정에선 **수십일~수개월 정도는 유지**되지만, “언젠가 사라진다”는 건 확실함
* `git gc --prune=now` 
    - 같은 aggressive GC를 직접 돌리면 진짜로 되돌릴 수 없게 될 수 있음. 
    - 웬만하면 손대지 않는 게 안전

##### 3. reflog로 복구할 때 또 `reset --hard`를 쓰면
   * 현재 워킹 디렉토리 수정사항은 날아감
   * 복구 시나리오가 헷갈리면 **항상 먼저 “구조용 브랜치”를 하나 파고 시작하는 게 가장 안전**

---

