#### C1. git add
##### 원본 키워드
- `git add` 호출 시
	- 파일 내용을 읽어서 해시 계산
	- `.git/objects`에 동일한 해시의 Blob이 있으면 재사용, 없으면 새 Blob 저장
	- Index의 경로 → Blob 해시 엔트리를 새 해시로 교체.

##### 확장 설명
- `git add` 호출 시
	1. 워킹 디렉토리 파일을 기준으로 해시 계산
	2. 이미 존재하는 `BlobHash` 일 경우 해당 `hash` 재사용
	3. 존재하지 않는 `BlobHash`일 경우 `Blob`을 생성하고 해당 `hash`를 사용
	4. Index에 있는 해당 경로 Entry정보를 업데이트함

---
#### C2. git commit - Tree 생성
##### 원본 키워드
- `git commit`호출 시
	1. Index 내용을 정렬하여 Tree 객체 생성
	2. 하위 디렉터리부터 Tree를 만들고 상위 Tree가 이를 참조

##### 확장 설명
- `git commit`호출 시
	1. Index의 모든 경로를 디렉터리 구조 기준으로 묶음
	2. 각 폴더마다 `해당 폴더의 Tree 객체`를 만듦
	3. `Tree 객체`안에 `구성내용(파일/폴더의 hash 목록)`을 저장함
	4. 해당 `Tree 객체 전체`에 대하여 hash를 계산한 값이 폴더의 `hash`값이 됨
	5. 해당 작업을 재귀적으로 반복하여 Root까지 진행함
	6. `Commit`은 이 중에서 `RootDirectory Tree Hash`만 잡고 있으면 역추적이 가능함

---
#### C3. git commit - Commit 생성 & 브랜치 갱신
##### 원본 키워드
- `Commit 객체 = Tree 해시 + Parent 해시 + 메타데이터`
- `HEAD가 가리키는 브랜치 ref의 커밋 해시를 새 커밋으로 교체`

##### 확장 설명
- Commit은 대략 다음 정보로 구성됨
	- `tree` <루트 Tree 해시>
	- `parent` <부모 커밋 해시들>
	- `author, committer, message` <메타데이터들>

- 새 Commit을 저장한 뒤 :
	- refs/heads/main 같은 브랜치 `ref`에 새 `Commit Hash`를 써 넣고, HEAD는 **여전히** 해당 브랜치 ref를 가리킴
	  
- **주의**해야할 부분 :
	- HEAD는 `Branch`의 `ref`를 가리키고 있음
		- 즉, Branch가 변경되지 않는 이상 HEAD가 가리키고 있는 것은 변하지 않음
		- 변하는 것은 `Branch의 ref`이며, `git commit`을 할 경우 `ref`가 갱신되는 것
		  
---
