import pygame
import socket
import threading
import sys

# Configuration
HOST = '127.0.0.1'
PORT = 5000
BOARD_SIZE = 16
CELL_SIZE = 40
MARGIN = 20
WINDOW_SIZE = MARGIN * 2 + CELL_SIZE * BOARD_SIZE

# Colors
BACKGROUND = (220, 179, 92) # Wood-like
LINE_COLOR = (0, 0, 0)
BLACK_STONE = (0, 0, 0)
WHITE_STONE = (255, 255, 255)
CURSOR_COLOR = (255, 0, 0)
OTHER_CURSOR_COLOR = (0, 0, 255)

# Game State
players = {} # id -> (r, c)
stones = []  # (r, c, color_code)
my_pos = [BOARD_SIZE // 2, BOARD_SIZE // 2]
sock = None

def parse_state(data):
    global players, stones
    try:
        parts = data.strip().split(' ')
        if parts[0] != 'STATE': return
        
        idx = 1
        new_players = {}
        new_stones = []

        if parts[idx] == 'PLAYERS':
            count = int(parts[idx+1])
            idx += 2
            for _ in range(count):
                p_data = parts[idx].split(':')
                pid = int(p_data[0])
                r = int(p_data[1])
                c = int(p_data[2])
                new_players[pid] = (r, c)
                idx += 1
        
        if idx < len(parts) and parts[idx] == 'STONES':
            count = int(parts[idx+1])
            idx += 2
            for _ in range(count):
                s_data = parts[idx].split(':')
                r = int(s_data[0])
                c = int(s_data[1])
                color = int(s_data[2])
                new_stones.append((r, c, color))
        
        players = new_players
        stones = new_stones
        
    except Exception as e:
        print(f"Error parsing state: {e}")

def receive_data():
    global sock
    while True:
        try:
            data = sock.recv(4096)
            if not data: break
            msg = data.decode()
            # Handle potentially multiple messages stuck together? 
            # For this simple prototype, we assume one packet usually contains the state update.
            # Ideally we'd buffer by newline.
            for line in msg.split('\n'):
                if line:
                    parse_state(line)
        except:
            break

def main():
    global sock, my_pos
    
    # Init Pygame
    pygame.init()
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("DashBlocks (C++ & Pygame)")
    clock = pygame.time.Clock()

    # Network Connect
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        threading.Thread(target=receive_data, daemon=True).start()
        
        # Initial Move to register
        sock.send(f"MOVE 0 0".encode()) 
    except Exception as e:
        print(f"Connection failed: {e}")
        return

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                dx, dy = 0, 0
                if event.key == pygame.K_UP or event.key == pygame.K_w: dy = -1
                elif event.key == pygame.K_DOWN or event.key == pygame.K_s: dy = 1
                elif event.key == pygame.K_LEFT or event.key == pygame.K_a: dx = -1
                elif event.key == pygame.K_RIGHT or event.key == pygame.K_d: dx = 1
                elif event.key == pygame.K_RETURN:
                    sock.send("PLACE".encode())
                    continue
                
                if dx != 0 or dy != 0:
                    sock.send(f"MOVE {dy} {dx}".encode())

        # Rendering
        screen.fill(BACKGROUND)

        # Draw Grid
        for i in range(BOARD_SIZE + 1):
            pos = MARGIN + i * CELL_SIZE
            # Vertical
            pygame.draw.line(screen, LINE_COLOR, (pos, MARGIN), (pos, WINDOW_SIZE - MARGIN))
            # Horizontal
            pygame.draw.line(screen, LINE_COLOR, (MARGIN, pos), (WINDOW_SIZE - MARGIN, pos))

        # Draw Stones
        for r, c, color in stones:
            cx = MARGIN + c * CELL_SIZE
            cy = MARGIN + r * CELL_SIZE
            c_color = BLACK_STONE if color == 1 else WHITE_STONE
            pygame.draw.circle(screen, c_color, (cx, cy), CELL_SIZE // 2 - 2)
            # Outline for contrast
            pygame.draw.circle(screen, (100,100,100), (cx, cy), CELL_SIZE // 2 - 2, 1)

        # Draw Players
        for pid, (r, c) in players.items():
            cx = MARGIN + c * CELL_SIZE
            cy = MARGIN + r * CELL_SIZE
            # Just a simple square or ring for player cursor
            pygame.draw.rect(screen, CURSOR_COLOR, (cx - 10, cy - 10, 20, 20), 2)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
