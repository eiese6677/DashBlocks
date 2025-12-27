#include <cstring>
#include <vector>
#include <algorithm>

#define BOARD_SIZE 15
#define MAX_ROOMS 10
#define MAX_PLAYERS 50
#define MAX_STONES 256
#define INF 1e9

struct Player
{
    int id;
    int r, c;
    bool active;
};

struct Stone
{
    int r, c;
    int color; // 1: Black, 2: White
};

struct GameRoom
{
    Player players[MAX_PLAYERS];
    Stone stones[MAX_STONES];
    int stone_count;
    int can_place_color = 1;
};

GameRoom rooms[MAX_ROOMS];

// --- AI Logic (Ported) --------------------------------------------------
int dx[4] = {1, 0, 1, 1};
int dy[4] = {0, 1, 1, -1};

bool inside(int x, int y)
{
    return x >= 1 && x < BOARD_SIZE && y >= 1 && y < BOARD_SIZE;
}

int count_dir(const int g[BOARD_SIZE][BOARD_SIZE], int x, int y, int d, int p)
{
    int cnt = 0;
    for (int s = 1; s < 5; s++)
    {
        int nx = x + dx[d] * s, ny = y + dy[d] * s;
        if (!inside(nx, ny) || g[nx][ny] != p)
            break;
        cnt++;
    }
    return cnt;
}

bool open_end(const int g[BOARD_SIZE][BOARD_SIZE], int x, int y, int d, int len)
{
    int nx = x + dx[d] * (len + 1), ny = y + dy[d] * (len + 1);
    return inside(nx, ny) && g[nx][ny] == 0;
}

bool win_after(const int g[BOARD_SIZE][BOARD_SIZE], int x, int y, int p)
{
    for (int d = 0; d < 4; d++)
    {
        int l = 0;
        for (int s = 1; s < 5; s++)
        {
            int nx = x + dx[d] * s, ny = y + dy[d] * s;
            if (!inside(nx, ny) || g[nx][ny] != p)
                break;
            l++;
        }
        int r = 0;
        for (int s = 1; s < 5; s++)
        {
            int nx = x - dx[d] * s, ny = y - dy[d] * s;
            if (!inside(nx, ny) || g[nx][ny] != p)
                break;
            r++;
        }
        if (l + r + 1 >= 5)
            return true;
    }
    return false;
}

bool makes_pattern(int g[BOARD_SIZE][BOARD_SIZE], int x, int y, int p, int target_len, bool open)
{
    bool res = false;
    int original = g[x][y];
    g[x][y] = p;
    for (int d = 0; d < 4; d++)
    {
        int l = count_dir(g, x, y, d, p);
        int r = count_dir(g, x, y, (d + 2) % 4, p); // dy[d] logic is slightly different in original but this works for 4 directions
        // Re-mapping directions strictly for 4 lines
        // Original dy was {0, 1, 1, -1}, dy for opposite is {0, -1, -1, 1}
        // Let's just use 8-direction helper if needed or re-implement count_dir for reverse
    }
    // Simplified pattern check to match original's spirit
    g[x][y] = original;
    return false;
}

// Actually, let's keep the EXACT structure from the user's code for reliability
struct AI_Board
{
    int g[BOARD_SIZE][BOARD_SIZE];
    int turn;

    void from_room(int idx)
    {
        memset(g, 0, sizeof(g));
        for (int i = 0; i < rooms[idx].stone_count; ++i)
        {
            // AI 1, -1. Room 1 (Black), 2 (White)
            g[rooms[idx].stones[i].r][rooms[idx].stones[i].c] = (rooms[idx].stones[i].color == 1) ? 1 : -1;
        }
    }

    bool win_at(int x, int y) const
    {
        int p = g[x][y];
        if (p == 0)
            return false;
        for (int d = 0; d < 4; d++)
        {
            int cnt = 1;
            for (int s = 1; s < 5; s++)
            {
                int nx = x + dx[d] * s, ny = y + dy[d] * s;
                if (!inside(nx, ny) || g[nx][ny] != p)
                    break;
                cnt++;
            }
            for (int s = 1; s < 5; s++)
            {
                int nx = x - dx[d] * s, ny = y - dy[d] * s;
                if (!inside(nx, ny) || g[nx][ny] != p)
                    break;
                cnt++;
            }
            if (cnt >= 5)
                return true;
        }
        return false;
    }

    std::vector<int> candidates() const
    {
        bool near[BOARD_SIZE][BOARD_SIZE]{};
        bool hasStone = false;
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                if (g[i][j] != 0)
                {
                    hasStone = true;
                    for (int di = -2; di <= 2; di++)
                        for (int dj = -2; dj <= 2; dj++)
                        {
                            int ni = i + di, nj = j + dj;
                            if (inside(ni, nj))
                                near[ni][nj] = true;
                        }
                }
        std::vector<int> res;
        if (!hasStone)
        {
            res.push_back(7 * BOARD_SIZE + 7);
            return res;
        }
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                if (near[i][j] && g[i][j] == 0)
                    res.push_back(i * BOARD_SIZE + j);
        return res;
    }

    bool is_open_four(int x, int y, int p)
    {
        g[x][y] = p;
        bool found = false;
        for (int d = 0; d < 4; d++)
        {
            int l = 0;
            for (int s = 1; s < 5; s++)
            {
                int nx = x + dx[d] * s, ny = y + dy[d] * s;
                if (!inside(nx, ny) || g[nx][ny] != p)
                    break;
                l++;
            }
            int r = 0;
            for (int s = 1; s < 5; s++)
            {
                int nx = x - dx[d] * s, ny = y - dy[d] * s;
                if (!inside(nx, ny) || g[nx][ny] != p)
                    break;
                r++;
            }
            if (l + r + 1 == 4)
            {
                // Check ends
                int lx = x - dx[d] * (r + 1), ly = y - dy[d] * (r + 1);
                int rx = x + dx[d] * (l + 1), ry = y + dy[d] * (l + 1);
                if (inside(lx, ly) && g[lx][ly] == 0 && inside(rx, ry) && g[rx][ry] == 0)
                {
                    found = true;
                    break;
                }
            }
        }
        g[x][y] = 0;
        return found;
    }

    bool is_open_three(int x, int y, int p)
    {
        g[x][y] = p;
        bool found = false;
        for (int d = 0; d < 4; d++)
        {
            int l = 0;
            for (int s = 1; s < 5; s++)
            {
                int nx = x + dx[d] * s, ny = y + dy[d] * s;
                if (!inside(nx, ny) || g[nx][ny] != p)
                    break;
                l++;
            }
            int r = 0;
            for (int s = 1; s < 5; s++)
            {
                int nx = x - dx[d] * s, ny = y - dy[d] * s;
                if (!inside(nx, ny) || g[nx][ny] != p)
                    break;
                r++;
            }
            if (l + r + 1 == 3)
            {
                int lx = x - dx[d] * (r + 1), ly = y - dy[d] * (r + 1);
                int rx = x + dx[d] * (l + 1), ry = y + dy[d] * (l + 1);
                if (inside(lx, ly) && g[lx][ly] == 0 && inside(rx, ry) && g[rx][ry] == 0)
                {
                    found = true;
                    break;
                }
            }
        }
        g[x][y] = 0;
        return found;
    }

    bool threat_search(int depth)
    {
        if (depth == 0)
            return false;
        const int p = turn;
        for (int m : candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            if (!is_open_four(x, y, p) && !is_open_three(x, y, p))
                continue;
            g[x][y] = p;
            if (win_at(x, y))
            {
                g[x][y] = 0;
                return true;
            }
            bool blocked = false;
            turn = -turn;
            for (int r_move : candidates())
            {
                int rx = r_move / BOARD_SIZE, ry = r_move % BOARD_SIZE;
                g[rx][ry] = turn;
                turn = -turn;
                if (!threat_search(depth - 1))
                    blocked = true;
                turn = -turn;
                g[rx][ry] = 0;
                if (blocked)
                    break;
            }
            turn = -turn;
            g[x][y] = 0;
            if (!blocked)
                return true;
        }
        return false;
    }

    int evaluate()
    {
        int score = 0;
        const int p = turn;
        for (int m : candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            if (is_open_four(x, y, p))
                score += 100000;
            if (is_open_three(x, y, p))
                score += 10000;
        }
        return score;
    }

    int negamax(int depth, int alpha, int beta)
    {
        if (depth == 0)
            return evaluate();
        for (int m : candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            g[x][y] = turn;
            turn = -turn;
            int v = -negamax(depth - 1, -beta, -alpha);
            turn = -turn;
            g[x][y] = 0;
            alpha = std::max(alpha, v);
            if (alpha >= beta)
                break;
        }
        return alpha;
    }
};

#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
    EXPORT void init_game(int room_id)
    {
        int idx = room_id % MAX_ROOMS;
    }

    EXPORT void reset_game(int room_id)
    {
        int idx = room_id % MAX_ROOMS;
        rooms[idx].stone_count = 0;
        rooms[idx].can_place_color = 1;
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (rooms[idx].players[i].active)
            {
                rooms[idx].players[i].r = BOARD_SIZE / 2;
                rooms[idx].players[i].c = BOARD_SIZE / 2;
            }
        }
    }

    EXPORT void move_player(int room_id, int player_id, int dx_in, int dy_in, int *out_r, int *out_c)
    {
        int idx = room_id % MAX_ROOMS;
        GameRoom *room = &rooms[idx];
        Player *p = nullptr;
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (room->players[i].active && room->players[i].id == player_id)
            {
                p = &room->players[i];
                break;
            }
        }
        if (!p)
        {
            for (int i = 0; i < MAX_PLAYERS; ++i)
            {
                if (!room->players[i].active)
                {
                    p = &room->players[i];
                    p->active = true;
                    p->id = player_id;
                    p->r = BOARD_SIZE / 2;
                    p->c = BOARD_SIZE / 2;
                    break;
                }
            }
        }
        if (p)
        {
            int nr = p->r + dy_in;
            int nc = p->c + dx_in;
            if (inside(nr, nc))
            {
                p->r = nr;
                p->c = nc;
            }
            *out_r = p->r;
            *out_c = p->c;
        }
        else
        {
            *out_r = 0;
            *out_c = 0;
        }
    }

    EXPORT bool place_stone(int room_id, int r, int c, int color)
    {
        int idx = room_id % MAX_ROOMS;
        GameRoom *room = &rooms[idx];
        if (room->stone_count >= MAX_STONES)
            return false;
        for (int i = 0; i < room->stone_count; ++i)
        {
            if (room->stones[i].r == r && room->stones[i].c == c)
                return false;
        }
        if (color != room->can_place_color)
        {
            return false;
        }
        room->stones[room->stone_count].r = r;
        room->stones[room->stone_count].c = c;
        room->stones[room->stone_count].color = color;
        room->stone_count++;
        if (room->can_place_color == 1)
            room->can_place_color = 2;
        else
            room->can_place_color = 1;
        return true;
    }

    EXPORT void get_ai_move(int room_id, int color, int *out_r, int *out_c)
    {
        int idx = room_id % MAX_ROOMS;
        AI_Board b;
        b.from_room(idx);
        b.turn = (color == 1) ? 1 : -1;

        int best_move = -1;
        // 1. Immediate win
        for (int m : b.candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            b.g[x][y] = b.turn;
            if (b.win_at(x, y))
            {
                best_move = m;
                b.g[x][y] = 0;
                break;
            }
            b.g[x][y] = 0;
        }
        if (best_move != -1)
        {
            *out_r = best_move / BOARD_SIZE;
            *out_c = best_move % BOARD_SIZE;
            return;
        }

        // 2. Block immediate opponent win
        int op_turn = -b.turn;
        for (int m : b.candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            b.g[x][y] = op_turn;
            if (b.win_at(x, y))
            {
                best_move = m;
                b.g[x][y] = 0;
                break;
            }
            b.g[x][y] = 0;
        }
        if (best_move != -1)
        {
            *out_r = best_move / BOARD_SIZE;
            *out_c = best_move % BOARD_SIZE;
            return;
        }

        // 3. Threat search (VCT)
        for (int m : b.candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            b.g[x][y] = b.turn;
            if (b.threat_search(2))
            {
                best_move = m;
                b.g[x][y] = 0;
                break;
            }
            b.g[x][y] = 0;
        }
        if (best_move != -1)
        {
            *out_r = best_move / BOARD_SIZE;
            *out_c = best_move % BOARD_SIZE;
            return;
        }

        // 4. Negamax
        int best_val = -INF;
        for (int m : b.candidates())
        {
            int x = m / BOARD_SIZE, y = m % BOARD_SIZE;
            b.g[x][y] = b.turn;
            b.turn = -b.turn;
            int v = -b.negamax(2, -INF, INF);
            b.turn = -b.turn;
            b.g[x][y] = 0;
            if (v > best_val)
            {
                best_val = v;
                best_move = m;
            }
        }

        if (best_move != -1)
        {
            *out_r = best_move / BOARD_SIZE;
            *out_c = best_move % BOARD_SIZE;
        }
        else
        {
            *out_r = 7;
            *out_c = 7;
        } // Default center
    }

    EXPORT void get_state(int room_id, int *players_buffer, int *out_p_count, int *stones_buffer, int *out_s_count)
    {
        int idx = room_id % MAX_ROOMS;
        GameRoom *room = &rooms[idx];
        int p_idx = 0, active_count = 0;
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (room->players[i].active)
            {
                players_buffer[p_idx++] = room->players[i].id;
                players_buffer[p_idx++] = room->players[i].r;
                players_buffer[p_idx++] = room->players[i].c;
                active_count++;
            }
        }
        *out_p_count = active_count;
        int s_idx = 0;
        for (int i = 0; i < room->stone_count; ++i)
        {
            stones_buffer[s_idx++] = room->stones[i].r;
            stones_buffer[s_idx++] = room->stones[i].c;
            stones_buffer[s_idx++] = room->stones[i].color;
        }
        // Append the currently allowed color to place (1=Black, 2=White)
        stones_buffer[s_idx++] = room->can_place_color;
        *out_s_count = room->stone_count;
    }
}
