
#### 1. 전제
```bash
main:        A ── B ── C
	          \
feature:       D ── E ── [F] ← HEAD
```
- `main` : `rebase`의 `target`이 될 `branch`
- `feature` : 현재 `HEAD`가 위치한 `branch`
- `HEAD` : 현재 `head-branch`의 맨 끝(F)을 가리킴

#### 02. git rebase (CommitHash)
```bash
git switch feature      # HEAD는 feature의 최신 Commit에 위치함 (F)
git rebase main         # rebase 대상으로 main을 지정함
```
1. 현재 브랜치(`feature`)와 대상 브랜치(`main`) 사이의 공통 조상을 기준으로
2. 현재 브랜치에 있는 커밋들을 골라서
3. 대상 브랜치 `tip` 뒤에 새 커밋으로 재생성해서 붙이고
	   - 이 때 부모 커밋이 달라져서 `hash`가 달라지기 때문에 다른 커밋임
4. 브랜치 `ref`를 새 체인의 끝으로 옮기는
5. 히스토리 재작성(커밋 다시쓰기) 작업을 `rebase`라고 함

#### 03.git rebase -i (CommitHash)
##### 알고리즘
###### 1. 현재 브랜치와 대상 브랜치가 같음
- `git rebase -i (CommitHash)` 에서 `(CommitHash)`는 **지금 HEAD가 있는 브랜치의 과거 커밋**
- 새 베이스도 그 브랜치 위의 커밋이라서 “브랜치를 다른 브랜치 위로 옮기는” 게 아니라  
- “**자기 자신 브랜치를 자기 과거 위로 재정렬하는 형태**”가 됨

###### 2. 공통 조상은 (CommitHash)에 들어가는 대상 커밋이 공통 조상이 됨
- “`(CommitHash)`를 기준(base) 커밋으로 삼고,  그 **이후 커밋들만 재생성함**”

###### 3. (CommitHash) 이후부터 HEAD까지의 커밋은 재생성 해야하는 커밋이 됨
- 범위에 있는 커밋들을 새 베이스 위애서 다시 만듦

###### 4. 범위에 있던 커밋들을 `Edit`하여 재생성하고 (`CommitHash`)를 `base`로 하여 그 뒤에 붙인 뒤 브랜치의 ref를 새로 구성한 체인의 마지맛 커밋(`tip`)으로 옮김

---
