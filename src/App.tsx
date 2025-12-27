import { useState, useEffect, useRef, useCallback } from 'react';
import { io, Socket } from 'socket.io-client';
import Board from './Board.tsx';
import './App.css';

// Define types for our state
interface PlayerData {
  [id: string]: [[number, number]];
}

interface Stone {
  r: number;
  c: number;
  color: 'black' | 'white';
}

export default function App() {
  const BOARD_SIZE = 15;

  const [players, setPlayers] = useState<PlayerData>({});
  const [members, setMembers] = useState<string[]>([]);
  const [myId, setMyId] = useState<string | null>(null);
  const [connected, setConnected] = useState(false);
  const [roomPassword, setRoomPassword] = useState('');
  const [joined, setJoined] = useState(false);
  const [isAutoAI, setIsAutoAI] = useState(false);

  const [board, setBoard] = useState<Stone[]>([]);
  const [canPlaceColor, setCanPlaceColor] = useState<number | null>(null); // 1=black,2=white

  const socketRef = useRef<Socket | null>(null);
  const posRef = useRef<[number, number]>([Math.floor(BOARD_SIZE / 2), Math.floor(BOARD_SIZE / 2)]);

  useEffect(() => {
    const socket = io('http://localhost:5000');
    socketRef.current = socket;

    socket.on('connect', () => setConnected(true));
    socket.on('disconnect', () => {
      setConnected(false);
      setJoined(false);
      setPlayers({});
      setMembers([]);
      setBoard([]);
    });

    socket.on('connection_response', (payload: { id?: string }) => {
      if (payload?.id) setMyId(payload.id);
    });

    socket.on('joined', () => setJoined(true));
    socket.on('join_error', (p: { reason?: string }) => alert('방 참여 실패: ' + (p?.reason || 'unknown')));

    socket.on('room_state', (payload: { data?: PlayerData; members?: string[], board?: Stone[], can_place_color?: number }) => {
      if (!payload) return;
      setPlayers(payload.data || {});
      setMembers(payload.members || []);
      setBoard(payload.board || []);
      setCanPlaceColor((payload as any).can_place_color ?? null);

      // We need to use the state from the argument because of the closure
      setMyId(myId => {
        const myData = payload.data ? payload.data[myId!] : null;
        if (myData && myData[0]) {
          posRef.current = myData[0];
        }
        return myId;
      });
    });

    return () => {
      socket.disconnect();
    };
  }, []);

  // Auto AI Effect
  useEffect(() => {
    if (!joined || !isAutoAI || !myId) return;

    const lastStone = board.length > 0 ? board[board.length - 1] : null;

    const myIdx = members.indexOf(myId);
    if (myIdx === -1) return;
    const myColor = myIdx === 0 ? 'black' : 'white';

    if (lastStone && lastStone.color === myColor) {
      const s = socketRef.current;
      if (s && s.connected) {
        const timer = setTimeout(() => {
          s.emit('ai_move');
        }, 500);
        return () => clearTimeout(timer);
      }
    }
  }, [board, isAutoAI, joined, members, myId]);

  const joinRoom = useCallback(() => {
    const s = socketRef.current;
    if (!s || !s.connected) return;
    s.emit('join', { password: roomPassword });
  }, [roomPassword]);

  const placeAt = useCallback((r: number, c: number) => {
    const s = socketRef.current;
    if (s && s.connected && joined) s.emit('place_stone', { r, c });
  }, [joined]);

  return (
    <div className="app-root">
      <h1>오목</h1>
      <div className={`status ${connected ? 'connected' : 'disconnected'}`}>
        {connected ? '✓ 서버 연결됨' : '✗ 연결 중...'}
        {canPlaceColor && (
          <span style={{ display: 'inline-flex', alignItems: 'center', marginLeft: 12 }}>
            <span style={{ width: 14, height: 14, borderRadius: 7, background: canPlaceColor === 1 ? '#000' : '#fff', border: '1px solid #777', marginRight: 8 }} />
            <span style={{ fontWeight: 600 }}>{canPlaceColor === 1 ? 'Black' : 'White'} to place</span>
          </span>
        )}
      </div>

      <div style={{ margin: '0.5rem 0' }}>
        <input placeholder="방 비밀번호 입력" value={roomPassword} onChange={(e: React.ChangeEvent<HTMLInputElement>) => setRoomPassword(e.target.value)} />
        <button onClick={joinRoom} disabled={!roomPassword}>Join</button>
        <span style={{ marginLeft: 8 }}>{joined ? `Joined: ${roomPassword}` : 'Not joined'}</span>
      </div>

      <Board size={BOARD_SIZE} players={players} myId={myId} members={members} board={board} onPlace={placeAt} />

      <div className="controls">
        <button
          onClick={() => {
            const s = socketRef.current;
            if (s && s.connected) s.emit('reset');
          }}
          disabled={!joined}
          style={{ padding: '0.5rem 1.5rem', background: '#d32f2f', color: 'white', border: 'none' }}
        >
          Reset Game
        </button>
        <button
          onClick={() => {
            const s = socketRef.current;
            if (s && s.connected) s.emit('ai_move');
          }}
          disabled={!joined}
          style={{ padding: '0.5rem 1.5rem', background: '#2e7d32', color: 'white', border: 'none', marginLeft: '0.5rem' }}
        >
          AI Move
        </button>
        <button
          onClick={() => setIsAutoAI(!isAutoAI)}
          disabled={!joined}
          style={{
            padding: '0.5rem 1.5rem',
            background: isAutoAI ? '#ff9800' : '#757575',
            color: 'white',
            border: 'none',
            marginLeft: '0.5rem'
          }}
        >
          {isAutoAI ? 'Auto AI: ON' : 'Auto AI: OFF'}
        </button>
      </div>
    </div>
  )
}
