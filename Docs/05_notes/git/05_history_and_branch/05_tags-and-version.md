## 태그 및 버전 관리

- Tag는 일반적으로 Release지점 마킹용
##### 1. 현재 HEAD에 태그 달기
```bash
# 가벼운 태그 
git tag v1.0.0  

# Annotated 태그 (추천) 
git tag -a <TagName> -m "TagMessage"
```

#### 2. 특정 커밋에 태그 달기
```bash
git log --oneline          # 커밋 해시 확인 
git tag -a <TagName> <CommitHash> -m "TagMessage"
```

#### 3. 태그 목록 보기 / 상세 보기
```bash
git tag                     # 태그 이름만 전체 목록 
git show <TagName>          # 태그가 가리키는 커밋 + 태그 메시지까지 확인
```

#### 4. 태그를 원격으로 올리기
- 기본적으로 `git push`만 하면 태그는 **자동으로 안 올라감**.
```bash
# 특정 태그만 보내기 
git push origin <TagName>  

# 모든 로컬 태그 전부 보내기 
git push origin --tags
```

#### 5. 태그 삭제
```bash
# 로컬 태그 먼저 삭제
git tag -d <TagName>

# 원격 태그 삭제 (둘 중 하나 사용)
git push origin :refs/tags/<TagName>
git push origin --delete <TagName>
```

---
#### 6. 잘못올린 태그 수정 시나리오
```bash
# 이미 태그가 잘못 올라간 상태
git tag -d <TagName>                                      # 1. 로컬 태그 삭제
git push origin --delete <TagName>                        # 2. 원격 태그 삭제
git tag -a <TagName> <EditCommitHash> -m "EditTagMassage" # 3. 재태그
git push origin <TagName>                                 # 4. 원격에 push  
```
##### 권장 순서
1. **내 로컬에서 태그 상태를 먼저 정리**
    - `git tag -d v1.0.0` → 내 작업 환경에서 “이 이름은 더 이상 쓰지 않는다”를 명확히 함.
2. 그 다음 **원격에서 같은 이름의 태그 삭제**
    - `git push origin --delete v1.0.0`
3. 그리고 **로컬에서 올바른 커밋에 태그 다시 생성 → 원격에 푸시**
##### 해당 순서를 권장하는 이유
- 로컬과 원격이 서로 다른 v1.0.0을 잠깐이라도 동시에 가지고 있는 애매한 상태를 피할 수 있음
- 내가 보는 `git tag`, `git show v1.0.0` 결과와 원격이 가지고 있는 태그의 의미가 계속 일치하도록 유지하기 쉬움

---
