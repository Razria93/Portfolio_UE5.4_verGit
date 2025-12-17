#### 1-1. 태그의 정의
- **브랜치**: “이 이름이 가리키는 커밋이 계속 바뀜(움직이는 포인터)”
- **태그(tag)**: “딱 한 커밋에 대하여 ‘이름을 붙여서’ 고정시키는 포인터(안 움직임)”
    
##### 예시 :
- `v1.0.0`, `release-2025-11-01` 같은 **버전/릴리즈 시점** 표시용으로 사용
- “이 시점은 빌드 잘 됐던 버전” 같은 안전지점 마킹용
    

##### 중요한 점:
- **태그는 브랜치와 상관없이 커밋에 직접 붙음**
	- rebase로 히스토리를 바꿔도, 태그는 원래 가리키던 커밋에 그냥 그대로 있음  
	- 그래서 rebase 후에는 태그 위치가 옛날 커밋에 박혀있는 경우가 생김 → 주의 포인트

---
#### 1-2. 태그 종류

##### (1) Lightweight tag (가벼운 태그)

`git tag v1.0.0`

- 그냥 이름 → 커밋 해시 한 줄 기록
- 추가 메타데이터(작성자, 메시지 등) 거의 없음
- “빠르게 표시만 할 때” 사용
    
#### (2) Annotated tag (권장)

`git tag -a v1.0.0 -m "첫 릴리즈"`

- 내부적으로 **하나의 Git 객체**로 저장 (작성자, 날짜, 메시지 포함)
- 실무/포폴 기준: **웬만하면 이걸 쓰는 게 정석**

---
#### 1-3. 자주 쓰는 Tag 패턴

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
