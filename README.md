# DashBlocks
## 프로젝트 구조

```
eslint.config.js
index.html
package.json
README.md
vite.config.js
client/
	main.py
public/
server/
	app_minimal.py
	app.py
	game_logic.cpp
	requirements.txt
	server.cpp
	test.cpp
	test.py
	__pycache__/
	ai/
		gomoku_ai.cpp
src/
	App.css
	App.jsx
	Board.jsx
	index.css
	main.jsx
	Player.jsx
```

## 실행 방법

1) Python 백엔드 실행 (Windows PowerShell 예시)

```powershell
# 가상환경 생성(선택)
python -m venv .venv
.\.venv\Scripts\Activate.ps1

# 의존성 설치
pip install -r server/requirements.txt

# 서버 실행(예: app.py를 메인으로 사용하는 경우)
python server/app.py
```

2) 프론트엔드 개발 서버 실행 (Vite)

```powershell
# 프론트엔드 의존성 설치
npm install

# 개발 서버 실행
npm run dev

# 또는 Vite 직접 실행
npx vite
```

3) 빌드 (배포용)

```powershell
# 프론트엔드 빌드
npm run build

# 빌드 결과는 일반적으로 `dist/` 폴더에 생성됩니다.
```

참고 및 팁:
- `server/requirements.txt`에 필요한 Python 패키지가 명시되어 있는지 확인하세요.
- `package.json`의 `scripts` 섹션에 `dev`, `build`, `start` 등이 정의되어 있으면 해당 스크립트를 사용하세요.
- 네이티브 C++ 코드(`server/*.cpp`, `server/game_logic.cpp`, `server/ai/gomoku_ai.cpp`)가 있다면 추가 빌드(예: `cmake` 또는 `g++`) 단계가 필요할 수 있습니다.
- 포트 충돌이 발생하면 `app.py` 또는 Vite 설정에서 포트를 변경하세요.

## 각 파일 설명

- `eslint.config.js`: ESLint 설정 파일 (코드 스타일/검사 설정).
- `index.html`: 웹앱의 루트 HTML 파일.
- `package.json`: 프로젝트 메타데이터, 스크립트, 프론트엔드 의존성 목록.
- `README.md`: 프로젝트 설명서(이 파일).
- `vite.config.js`: Vite 개발/번들 설정.

- `client/`: 클라이언트 관련 코드/스크립트 폴더.
  - `main.py`: 클라이언트 관련 Python 스크립트(프로젝트에 따라 역할이 다를 수 있음).

- `public/`: 정적 자원(이미지, 폰트 등)을 두는 폴더.

- `server/`: 서버 쪽 소스와 테스트, 빌드 관련 파일들.
  - `app_minimal.py`: 간단하거나 예제용 서버 애플리케이션.
  - `app.py`: 메인 서버 애플리케이션(주요 엔드포인트 포함).
  - `game_logic.cpp`: 게임 로직을 구현한 C++ 소스파일.
  - `requirements.txt`: Python 의존성 목록(서버 실행에 필요한 패키지).
  - `server.cpp`: C++로 작성된 서버 또는 관련 네이티브 코드.
  - `test.cpp`: C++ 테스트/샘플 코드.
  - `test.py`: Python 단위/통합 테스트 스크립트.
  - `__pycache__/`: Python 바이트코드 캐시 디렉터리(자동 생성).
  - `ai/gomoku_ai.cpp`: 오목(Gomoku) 인공지능 로직(C++).

- `src/`: 프론트엔드 React 소스 코드.
  - `App.css`: 애플리케이션 전역 스타일.
  - `App.jsx`: 메인 React 컴포넌트.
  - `Board.jsx`: 보드 표시용 컴포넌트.
  - `index.css`: 기본 스타일 시트.
  - `main.jsx`: 프론트엔드 진입점(React 렌더링 시작 파일).
  - `Player.jsx`: 플레이어 관련 컴포넌트/로직.
