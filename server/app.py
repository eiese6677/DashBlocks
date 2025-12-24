import ctypes
import os
from flask import Flask, request
from flask_socketio import SocketIO, emit, join_room, leave_room
from flask_cors import CORS
import eventlet

# We must monkey patch early, but after some imports it might be safer to do it here
eventlet.monkey_patch()

app = Flask(__name__)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='eventlet')

# --- C++ DLL Interop -----------------------------------------------------
dll_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "game_logic.dll"))
print(f"DEBUG: Attempting to load DLL at {dll_path}")

try:
    game_lib = ctypes.CDLL(dll_path)
    print("DEBUG: DLL Loaded Successfully")
except Exception as e:
    print(f"DEBUG: DLL Load Failed: {e}")
    raise e

# void init_game(int room_id)
game_lib.init_game.argtypes = [ctypes.c_int]

# void move_player(int room_id, int player_id, int dx, int dy, int* out_r, int* out_c)
game_lib.move_player.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, 
                                 ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]

# bool place_stone(int room_id, int r, int c, int color)
game_lib.place_stone.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
game_lib.place_stone.restype = ctypes.c_bool

# void reset_game(int room_id)
game_lib.reset_game.argtypes = [ctypes.c_int]

# void get_state(int room_id, int* p_buf, int* p_count, int* s_buf, int* s_count)
game_lib.get_state.argtypes = [ctypes.c_int, 
                               ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int),
                               ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]

# void get_ai_move(int room_id, int color, int* out_r, int* out_c)
game_lib.get_ai_move.argtypes = [ctypes.c_int, ctypes.c_int, 
                                 ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]

BOARD_SIZE = 15
rooms = {} # pw -> list of sids

def get_room_id(pw):
    return abs(hash(pw)) % 10000

def get_player_id(sid):
    return abs(hash(sid)) % 10000

def broadcast_room(pw):
    room_id = get_room_id(pw)
    p_buf = (ctypes.c_int * 300)() 
    s_buf = (ctypes.c_int * 768)()
    p_count = ctypes.c_int(0)
    s_count = ctypes.c_int(0)

    game_lib.get_state(room_id, p_buf, ctypes.byref(p_count), s_buf, ctypes.byref(s_count))

    count = p_count.value
    room_data = {}
    members = rooms.get(pw, [])
    
    c_players = {}
    for i in range(count):
        pid = p_buf[i*3]
        r = p_buf[i*3+1]
        c = p_buf[i*3+2]
        c_players[pid] = [r, c]
        
    for m in members:
        pid = get_player_id(m)
        if pid in c_players:
            room_data[m] = [c_players[pid]]

    board_state = []
    sc = s_count.value
    for i in range(sc):
        r = s_buf[i*3]
        c = s_buf[i*3+1]
        color_val = s_buf[i*3+2]
        color = 'black' if color_val == 1 else 'white'
        board_state.append({'r': r, 'c': c, 'color': color})

    room_name = f"room:{pw}"
    socketio.emit('room_state', {
        'members': members,
        'data': room_data,
        'board': board_state
    }, room=room_name)

@socketio.on('connect')
def handle_connect():
    print("Client connected:", request.sid)
    emit('connection_response', {'status': 'connected', 'id': request.sid})

@socketio.on('disconnect')
def handle_disconnect():
    sid = request.sid
    for pw, members in rooms.items():
        if sid in members:
            members.remove(sid)
            broadcast_room(pw)
            if not members:
                del rooms[pw]
            break

@socketio.on('join')
def handle_join(evt_data):
    pw = str(evt_data.get('password', '')).strip()
    sid = request.sid
    room_id = get_room_id(pw)
    pid = get_player_id(sid)

    game_lib.init_game(room_id)
    
    members = rooms.setdefault(pw, [])
    if sid not in members:
        members.append(sid)
        join_room(f"room:{pw}")

    r, c = ctypes.c_int(0), ctypes.c_int(0)
    game_lib.move_player(room_id, pid, 0, 0, ctypes.byref(r), ctypes.byref(c))

    emit('joined', {'room': pw, 'id': sid})
    broadcast_room(pw)

@socketio.on('move')
def handle_move(evt_data):
    sid = request.sid
    pw = None
    for p, m in rooms.items():
        if sid in m:
            pw = p
            break
    if not pw: return

    room_id = get_room_id(pw)
    pid = get_player_id(sid)
    dx = int(evt_data.get('dx', 0))
    dy = int(evt_data.get('dy', 0))

    out_r = ctypes.c_int()
    out_c = ctypes.c_int()
    game_lib.move_player(room_id, pid, dx, dy, ctypes.byref(out_r), ctypes.byref(out_c))
    broadcast_room(pw)

@socketio.on('place_stone')
def handle_place_stone():
    sid = request.sid
    pw = None
    members = []
    for p, m in rooms.items():
        if sid in m:
            pw = p
            members = m
            break
    if not pw: return

    color = 0
    if members.index(sid) == 0: color = 1 # Black
    elif members.index(sid) == 1: color = 2 # White
    else: return

    room_id = get_room_id(pw)
    pid = get_player_id(sid)

    r, c = ctypes.c_int(0), ctypes.c_int(0)
    game_lib.move_player(room_id, pid, 0, 0, ctypes.byref(r), ctypes.byref(c))
    
    success = game_lib.place_stone(room_id, r.value, c.value, color)
    if success:
        broadcast_room(pw)

@socketio.on('reset')
def handle_reset():
    sid = request.sid
    pw = None
    for p, m in rooms.items():
        if sid in m:
            pw = p
            break
    if not pw: return

    room_id = get_room_id(pw)
    game_lib.reset_game(room_id)
    broadcast_room(pw)

@socketio.on('ai_move')
def handle_ai_move():
    sid = request.sid
    pw = None
    members = []
    for p, m in rooms.items():
        if sid in m:
            pw = p
            members = m
            break
    if not pw: return

    room_id = get_room_id(pw)
    
    # AI color logic: if 1 member, AI is opposite of player 1. 
    # If 2 members, AI is... whichever? Let's say AI plays for the turn-holder if possible, 
    # but for now, let's just make AI play as 'white' (2) by default or opposite of asker.
    my_color = 1 if members.index(sid) == 0 else 2
    ai_color = 1 if my_color == 2 else 2 # AI is opposite of the one who requested it

    ar, ac = ctypes.c_int(0), ctypes.c_int(0)
    game_lib.get_ai_move(room_id, ai_color, ctypes.byref(ar), ctypes.byref(ac))
    
    success = game_lib.place_stone(room_id, ar.value, ac.value, ai_color)
    if success:
        broadcast_room(pw)

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
