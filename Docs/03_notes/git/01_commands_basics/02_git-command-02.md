## Git Command-02
#### 6. Merge
```Bash
git merge (BranchName)             # ★ 현재 브랜치에 (BranchName)을 병합
git merge --no-ff (BranchName)     # ★ Fast-forward 가능해도 '머지 커밋'을 강제로 만듦
git merge --squash (BranchName)    # ⚠ 여러 커밋 내용을 '하나의 커밋'으로만 합치기
git merge --no-commit (BranchName) # 머지 결과만 반영하고, 커밋은 직접 수동
git merge --abort                  # ⚠ 망했을 때, 머지 시도 이전 상태로 되돌리기
```
- **Merge Diagram**  ![[Pasted image 20251127214649.png]]
##### Merge Algorithm
1. **공통 조상 찾기**
	   `main`과 `feature`가 갈라지기 전 **공통 커밋(merge base)** 을 찾음 **(2번)**
2. **Fast-forward (빨리감기)의 경우**
	   두 버전의 차이는 `main 버전`에서 `feature 버전`이 되며 수정된 부분밖에 없음
	   따라서 두 버전을 병합한 새로운 커밋을 만들고 main의 HEAD를 옮기는 것이 아닌,  **main의 HEAD만 feature의 HEAD 위치로 옮기고** 작업을 마침
3.  **3-way merge 시도**
	   각 파일에 대해  `공통 조상 버전` vs `main 버전` vs `feature 버전`을 비교해서 **자동으로 합칠 수 있는지** 판단함
4. **자동으로 합쳐진 파일은 곧바로 인덱스에 반영**
		충돌 없는 파일들은 **자동 머지 완료** 상태가 되고, 인덱스(index)에 “merge 결과”로 등록
5. **진짜 충돌나는 파일만 Working Tree에 남기고 작업 중단**
		해당 부분은 개발자가 선택/수정 등을 거쳐 파일을 저장하고 Staging 후 Commit 진행

#### 7. Rebase
```Bash
# 1) Rebase 기본
git rebase (BaseBranch)                     # 현재 브랜치를 BaseBranch 뒤로 재배치
git rebase (BaseBranch) (WorkBranch)        # WorkBranch를 BaseBranch 위로 재배치

git rebase --abort                          # Rebase 도중 전부 취소 (시작 전 상태로)
git rebase --continue                       # 충돌 해결 후, 다음 커밋 계속 적용
git rebase --skip                           # 현재 충돌난 커밋을 건너뛰고 다음으로 진행
git rebase --onto                           # RootBranch에 있는 WorkBranch를 
(BaseBranch) (WorkBranch) (RootBranch)      # BaseBranch뒤로 재배치

# 2) Interacive Rebase (히스토리 정리용)
git rebase -i HEAD~N                        # 최근 N개의 커밋을 편집/합치기/삭제
git rebase -i (BaseCommitHash)              # BaseCommit 이후의 커밋들 재정렬/수정 

# 3) Remote와 함께 (위험 영역)
# 가능한 사용 X
git pull --rebase                 # 원격 변경을 pull 이후 내 커밋을 그 위에 Rebase
git push --force-with-lease       # Rebase로 바뀐 히스토리를 원격에 강제 반영
```
- **Rebase Diagram**  ![[Pasted image 20251127215049.png]]
##### Rebase Algorithm
1. **파일에서 충돌(Conflict) 해결**
	: 충돌 발생 후 남길 코드만 정리하고 마커 제거 후 저장
2. **충돌 해결된 파일은 Staging** 
   : `git add`
3. **현재 Commit의 Rebase 계속 진행**
   : `git rebase --continue`
4. **현재 Commit의 다른 파일에서 다시 충돌이 발생할 경우**
   : 1 ~ 3번 과정 반복

---
##### `Merge Algorithm` vs `Rebase Algorithm`
- **공통점**
	- 둘 다 **“할 수 있는 데까지 자동으로 적용 → 안 되는 지점에서 충돌 발생 후 멈춤”**  
- **다른 점**
	- **merge:**
	    - **한 번에 전체를 합치고**, 마지막에만 머지 커밋을 만듦
	    - 충돌은 대개 “머지 커밋 하나를 만들기 직전에서만 한 번” 발생
	- **rebase:**
	    - 커밋 **하나하나를 순서대로 재적용**함 (중요)
		    1. 특정 커밋(예: D') 적용 시 충돌 나면,
			2. rebase는 **그 커밋에서 멈춤**
	        3. 충돌 해결 후, `git rebase --continue` → 다음 커밋(E') 적용
	        4. 즉, **여러 커밋에서 여러 번 충돌이 날 수도 있음**
          
---
