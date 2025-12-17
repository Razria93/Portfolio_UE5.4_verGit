## 브랜치 간 병합 혹은 재배치
#### 1. Remote의 Commit이 필요한 경우
```bash
# --- merge를 하고 싶을 경우 ---
# [의미] 현재 branch(main)에다가 차이가 있는 대상 branch(origin/main)의 커밋들을 합쳐서 적용해라
# 원격 저장소의 변경사항을 가져옴 (fetch)
# 1. Remote에 있는 origin의 main 브랜치에서 최신 커밋들을 Local로 가져옴
# 2. Local에 origin/main 이라는 remote-tracking 브랜치로 저장됨
git fetch origin main
git switch main
git merge origin/main

# 또는 한 줄로 (fetch + merge)
git pull --no-rebase origin main

# --- rebase를 하고 싶을 경우 ---
# [의미] 현재 branch(main)의 커밋들을 대상 branch(origin/main)의 HEAD 뒤로 재배치해라

git fetch origin main
git switch main
git rebase origin/main

# 또는 한 줄로 (fetch + rebase)
git pull --rebase origin main
```


---
#### 2. 차이점
- merge는 “분기+합류가 보이는 **병렬** 이력”
- rebase는 “다시 재생된 **직렬** 이력”

최종 코드 상태는 같을 수 있음 

따라서 **히스토리의 모양/이야기를 어떻게 가져갈지에 따라 merge vs rebase를 선택**

---
#### 3. 추가설명
##### origin :
- 원격 저장소(URL)에 붙인 로컬 별명(라벨)
  - `git remote add <RemoteName> <URL>` 으로 설정함

##### rebase / merge :
- 두 명령어 모두 **“현재 체크아웃된 브랜치 기준으로”** 작업이 이루어짐

- **merge :**
  - 현재 브랜치에 **다른 브랜치의 변경 사항을 통합**함
  - 일반적인 경우, 현재 브랜치의 tip 다음에 새로운 merge Commit이 생성됨
  - 단, fast-forward가 가능한 경우에는 새로운 merge 커밋 없이
    현재 브랜치의 HEAD가 대상 브랜치 HEAD 위치로 그대로 이동(FF merge)

- **rebase :**
  - 현재 브랜치에서 **대상 브랜치에 없는 커밋들만 골라서**,
    대상 브랜치의 HEAD 뒤로 순서대로 “다시 적용”하여 **새 커밋**을 만듦
  - 재생성이 완료되면, 현재 브랜치의 HEAD를 새로 만들어진 마지막 커밋으로 옮김
  - 이 작업은 **현재 브랜치의 히스토리를 재작성**하는 것이고,
    대상 브랜치의 포인터는 그대로임
    - 다른 로컬 브랜치가 이 변화를 따라잡게 하려면,
      그 브랜치를 체크아웃한 뒤 rebase된 브랜치를 대상으로
      fast-forward merge를 해줄 수 있음
    - 원격 저장소(origin)의 브랜치를 갱신하려면 `git push`(필요시 `--force-with-lease`)를 사용해야 함
      
---
