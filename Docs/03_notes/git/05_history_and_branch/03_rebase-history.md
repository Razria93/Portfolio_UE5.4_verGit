## 히스토리 재배치
#### 1. 히스토리 정리
```bash
# 여러 커밋을 한 번에 손보는 도구
git rebase -i
```
- **가능한 조작 :**
	- `pick` / `p`    : 커밋 그대로 사용
	- `reword`/ `r`  : 커밋 메시지 수정
	- `edit` / `e`    : 커미 수정을 위한 정지
	- `drop` / `d`    : 커밋 제거
	- `squash` / `s` : 여러 커밋을 하나로 합치기

#### 2. Local Branch의 경우
```bash
# 1) 커밋 확인  
git log --oneline

# 2) 히스토리 정리를 시작할 기준 커밋을 정함
git rebase -i (BaseCommitHash) # BaseCommitHash : 정리의 시작점

# 3) Editor에서 편집 후
:wq # 저장 후 종료 명령어

# A) 이후 충돌이 없다면 아래 rebase작업을 거친 뒤 히스토리만 바뀌고 종료 
# --- 

# B-1) 이후 충돌이 발생한다면 수동 수정 작업 
# B-2) 파일의 수동 수정 작업이 끝났을 경우
git add <FileName> 
git rebase --continue

# B-3) 작업을 마치기 어려워 중단하고 싶을 경우
git rebase --abort
```
##### rebase -i 작업 과정
1. BaseCommitHash 이후부터 기존 HEAD까지 있는 커밋들을 
2. Edit에서 지정한 재성성 규칙에 따라 커밋을 재구성하여 
3. BaseCommitHash 뒤에 새 커밋체인을 만들고
4. 마지막 새 커밋의 해시를 해당 브랜치의 ref에 기록하여 
5. 브랜치(ref)가 새 체인의 tip을 가리키게 함

##### 3. 이미 push된 Branch의 경우 (원격의 Branch 수정)
```bash
# 0) 원격 최신 내용 받아오기
git fetch origin

# 1) 내가 정리하고 싶은 'feature 브랜치'로 이동
git switch (feature)   # HEAD가 feature 브랜치로

# 2) main 최신(origin/main)을 기준으로 내 커밋들 재정렬
git rebase -i origin/main

# 3) 로컬 feature 브랜치 히스토리 정리 끝난 뒤
git push --force-with-lease      # origin/feature/move-system 덮어쓰기
```
##### 원격의 브랜치 rebase -i 작업 과정
1. 원격 브랜치에 대응하는 로컬 브랜치가 있음
	- 우리가 rebase로 수정하는 대상은 “로컬 브랜치”뿐 원격의 브랜치를 직접 수정하지 않음
2. 이 원격과 연결된 로컬 브랜치의 히스토리를 로컬에서 고침
3. 그 수정된 히스토리로 원격 브랜치를 강제로 덮어씌우는 것
4. 이것이 `rebase` + `push --force-with-lease` 조합의 의미임
   
---
