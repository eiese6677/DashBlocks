# DashBlocks

C++ 게임 로직과 Python(Flask + Socket.IO) 서버,
React(Vite) 프런트엔드로 구성된 **실시간 오목(Gomoku) 게임 프로젝트**입니다.
Docker 기반 실행을 기본으로 하며, 로컬 개발도 지원합니다.

---

## 1. 프로젝트 구조

```
[Client / Browser]
└─ React (Vite)
   ├─ src/main.tsx
   │  └─ React 앱 시작
   │
   ├─ src/App.tsx
   │  ├─ Socket.IO 서버 연결
   │  ├─ 서버 이벤트 수신
   │  │  ├─ joined
   │  │  ├─ room_state
   │  │  └─ connection_response
   │  └─ 입력 이벤트 전송
   │     ├─ move
   │     ├─ place_stone
   │     └─ ai_move
   │
   ├─ src/Board.tsx
   │  └─ 서버에서 받은 board 상태 렌더링
   │
   └─ src/Player.jsx
      └─ 플레이어 커서 위치 표시

        │
        │ Socket.IO (WebSocket)
        ▼

[Server / Python]
└─ Flask + Flask-SocketIO
   ├─ server/app.py
   │  ├─ 서버 초기화
   │  │  ├─ eventlet monkey patch
   │  │  └─ Socket.IO 설정
   │  │
   │  ├─ C++ DLL 로드
   │  │  └─ game_logic.dll (ctypes)
   │  │
   │  ├─ 방 / 플레이어 관리
   │  │  ├─ rooms { password -> [sid] }
   │  │  └─ room_id / player_id 생성
   │  │
   │  ├─ Socket.IO 이벤트 처리
   │  │  ├─ connect
   │  │  ├─ join
   │  │  ├─ move
   │  │  ├─ place_stone
   │  │  └─ ai_move
   │  │
   │  ├─ broadcast_room()
   │  │  ├─ C++ 상태 조회
   │  │  ├─ JSON 변환
   │  │  └─ 방 전체 브로드캐스트
   │  │
   │  └─ 서버 실행 진입점
   │
   └─ server/requirements.txt
      └─ Python 의존성

        │
        │ ctypes (DLL 호출)
        ▼

[Core Logic / C++]
└─ Game Logic (DLL)
   ├─ server/game_logic.cpp
   │  ├─ 방별 게임 상태 관리
   │  ├─ 바둑판 상태 저장
   │  ├─ 플레이어 위치 관리
   │  ├─ 돌 배치 검증
   │  └─ 상태 조회 함수 제공
   │
   ├─ server/ai/gomoku_ai.cpp
   │  ├─ 네가맥스 기반 AI
   │  ├─ 후보 수 탐색
   │  └─ 최적 수 반환
   │
   └─ game_logic.dll
      └─ Python에서 로드되는 핵심 로직
```

---

## 2. 기술 스택

* **Frontend**

  * React + TypeScript
  * Vite
* **Backend**

  * Python (Flask, Flask-SocketIO, eventlet)
  * ctypes 기반 C++ 연동
* **Game Logic / AI**

  * C++ (Shared Library: `.dll` / `.so`)
* **Infra**

  * Docker (권장 실행 방식)

---

## 3. 실행 방법 (Docker 권장)

### 3.1 Docker 이미지 빌드

프로젝트 루트에서 실행:

```bash
docker build -t dashblocks .
```

> ⚠️ `game_logic.cpp` 또는 `gomoku_ai.cpp`를 수정했다면
> **반드시 다시 `docker build` 해야 변경 사항이 반영됩니다.**

---

### 3.2 Docker 컨테이너 실행

```bash
docker run -d -p 5000:5000 --name dashblocks-game dashblocks
```

* `-d` : 백그라운드 실행
* `5000:5000` : 서버 포트 노출

---

### 3.3 접속

브라우저에서 접속:

```
http://localhost:5000
```

---

### 3.4 로그 확인 (선택)

```bash
docker logs dashblocks-game
```

---

### 3.5 컨테이너 중지 / 삭제

```bash
docker stop dashblocks-game
docker rm dashblocks-game
```

---

## 4. ⚠️ C++ 코드 수정 시 반드시 알아야 할 점

이 프로젝트는 **Python에서 C++ DLL(SO)을 `ctypes`로 로드**합니다.

### 중요한 사실

* C++ 코드를 수정해도 **자동 반영되지 않습니다**
* Docker 이미지에 DLL이 포함되므로,
  **이미지를 다시 빌드해야 합니다**

### 수정 반영 절차 (필수)

```bash
docker stop dashblocks-game
docker rm dashblocks-game
docker build -t dashblocks .
docker run -d -p 5000:5000 --name dashblocks-game dashblocks
```

✔️ 이 과정을 거쳐야 C++ 로직 / AI 변경이 반영됩니다.

---

## 5. 로컬 개발 실행 (대안)

Docker 없이 로컬에서 프런트엔드 / 백엔드를 개별 실행할 수 있습니다.

---

### 5.1 백엔드 실행 (Python)

```bash
cd server

python -m venv .venv
# Windows
.venv\Scripts\activate
# macOS / Linux
source .venv/bin/activate

pip install -r requirements.txt
python app.py
```

서버 기본 주소:

```
http://localhost:5000
```

---

### 5.2 프런트엔드 실행 (Vite)

```bash
npm install
npm run dev
```

기본 주소:

```
http://localhost:5173
```

---

## 6. 개발 참고 사항

* Python 서버는 **상태 관리 / 네트워크 / 브로드캐스트 담당**
* C++은 **게임 규칙 / AI / 성능 민감 로직 담당**
* Socket.IO 기반 실시간 통신
* AI는 추후 **네가맥스 / 알파베타 / Iterative Deepening** 확장 가능
