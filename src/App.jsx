import { useState, useEffect, useRef, useCallback } from 'react'
import { io } from 'socket.io-client'
import Board from './Board'
import './App.css'

export default function App() {
  const BOARD_SIZE = 15

  const [players, setPlayers] = useState({})
  const [members, setMembers] = useState([])
  const [myId, setMyId] = useState(null)
  const [connected, setConnected] = useState(false)
  const [roomPassword, setRoomPassword] = useState('')
  const [joined, setJoined] = useState(false)
  const [isAutoAI, setIsAutoAI] = useState(false)

  const [board, setBoard] = useState([])

  const socketRef = useRef(null)
  const posRef = useRef([Math.floor(BOARD_SIZE / 2), Math.floor(BOARD_SIZE / 2)])

  useEffect(() => {
    const socket = io('http://localhost:5000')
    socketRef.current = socket

    socket.on('connect', () => setConnected(true))
    socket.on('disconnect', () => {
      setConnected(false)
      setJoined(false)
      setPlayers({})
      setMembers([])
      setBoard([])
    })

    socket.on('connection_response', (payload) => {
      if (payload?.id) setMyId(payload.id)
    })

    socket.on('joined', () => setJoined(true))
    socket.on('join_error', (p) => alert('방 참여 실패: ' + (p?.reason || 'unknown')))

    socket.on('room_state', (payload) => {
      if (!payload) return
      setPlayers(payload.data || {})
      setMembers(payload.members || [])
      setBoard(payload.board || [])

      const myData = payload.data ? payload.data[myId] : null
      if (myData && myData[0]) {
        posRef.current = myData[0]
      }
    })

    return () => socket.disconnect()
  }, [])

  // Auto AI Effect
  useEffect(() => {
    if (!joined || !isAutoAI) return

    // Check if it's "AI's turn". 
    // In this simple version, if there's only one player, AI is always the opponent.
    // If black just moved, white (AI) should move.
    const lastStone = board.length > 0 ? board[board.length - 1] : null

    // Heuristic: If last stone was black (player), and we are black, then trigger AI.
    // Or more simply: if board length is odd (black placed), or even (white placed) depending on who is who.
    // Let's assume the user who turned on Auto AI wants the AI to respond to their moves.
    const myIdx = members.indexOf(myId)
    const myColor = myIdx === 0 ? 'black' : 'white'

    if (lastStone && lastStone.color === myColor) {
      const s = socketRef.current
      if (s && s.connected) {
        // Delay slightly for better UX
        const timer = setTimeout(() => {
          s.emit('ai_move')
        }, 500)
        return () => clearTimeout(timer)
      }
    }
  }, [board, isAutoAI, joined, members, myId])

  const joinRoom = useCallback(() => {
    const s = socketRef.current
    if (!s || !s.connected) return
    s.emit('join', { password: roomPassword })
  }, [roomPassword])

  const move = useCallback((dx, dy) => {
    const s = socketRef.current
    if (s && s.connected && joined) s.emit('move', { dx, dy })
  }, [joined])

  useEffect(() => {
    function onKey(e) {
      if (!joined) return
      const k = e.key.toLowerCase()
      if (k === 'w' || k === 'arrowup') move(0, -1)
      if (k === 's' || k === 'arrowdown') move(0, 1)
      if (k === 'a' || k === 'arrowleft') move(-1, 0)
      if (k === 'd' || k === 'arrowright') move(1, 0)
      if (k === 'enter') {
        const s = socketRef.current
        if (s && s.connected) s.emit('place_stone')
      }
    }
    window.addEventListener('keydown', onKey)
    return () => window.removeEventListener('keydown', onKey)
  }, [move, joined])

  return (
    <div className="app-root">
      <h1>체스판 (새로 작성된 버전)</h1>
      <div className="status">{connected ? '✓ 서버 연결됨' : '✗ 연결 중...'}</div>

      <div style={{ margin: '0.5rem 0' }}>
        <input placeholder="방 비밀번호 입력" value={roomPassword} onChange={(e) => setRoomPassword(e.target.value)} />
        <button onClick={joinRoom} disabled={!roomPassword}>Join</button>
        <span style={{ marginLeft: 8 }}>{joined ? `Joined: ${roomPassword}` : 'Not joined'}</span>
      </div>

      <Board size={BOARD_SIZE} players={players} myId={myId} members={members} board={board} />

      <div className="controls">
        <button
          onClick={() => {
            const s = socketRef.current
            if (s && s.connected) s.emit('reset')
          }}
          disabled={!joined}
          style={{ padding: '0.5rem 1.5rem', background: '#d32f2f', color: 'white', border: 'none' }}
        >
          Reset Game
        </button>
        <button
          onClick={() => {
            const s = socketRef.current
            if (s && s.connected) s.emit('ai_move')
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