#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <map>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BOARD_SIZE 16

struct Point {
    int r, c;
};

// Global State
std::mutex state_mutex;
std::map<int, Point> players; // socket_fd -> Position
std::vector<std::pair<Point, int>> stones; // {{r,c}, color_type} (1=Black, 2=White)
// Simple color assignment based on join order (odd=Black, even=White for simplicity, or just tracking count)
int player_count = 0;

std::string serialize_state() {
    std::stringstream ss;
    ss << "STATE";
    
    // Players
    ss << " PLAYERS " << players.size();
    for (const auto& p : players) {
        // format: id:r:c
        ss << " " << p.first << ":" << p.second.r << ":" << p.second.c;
    }

    // Stones
    ss << " STONES " << stones.size();
    for (const auto& s : stones) {
        // format: r:c:color
        ss << " " << s.first.r << ":" << s.first.c << ":" << s.second;
    }

    ss << "\n";
    return ss.str();
}

void broadcast(const std::string& msg, const std::vector<SOCKET>& clients) {
    for (SOCKET client : clients) {
        send(client, msg.c_str(), msg.length(), 0);
    }
}

void handle_client(SOCKET client_socket, int id, std::vector<SOCKET>& all_clients) {
    char buffer[1024];
    int color = (player_count % 2 == 0) ? 1 : 2; // Simple assignment
    player_count++;

    {
        std::lock_guard<std::mutex> lock(state_mutex);
        players[id] = {BOARD_SIZE / 2, BOARD_SIZE / 2};
    }

    std::cout << "Client " << id << " connected. Color: " << color << std::endl;

    while (true) {
        int bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received <= 0) break;

        buffer[bytes_received] = '\0';
        std::string command(buffer);
        std::stringstream ss(command);
        std::string action;
        ss >> action;

        bool update_needed = false;

        {
            std::lock_guard<std::mutex> lock(state_mutex);
            if (action == "MOVE") {
                int dh, dw; // dy, dx
                ss >> dh >> dw; // Protocol: MOVE dy dx (h=row, w=col)
                
                Point& p = players[id];
                int nr = p.r + dh;
                int nc = p.c + dw;

                if (nr >= 0 && nr <= BOARD_SIZE && nc >= 0 && nc <= BOARD_SIZE) {
                    p.r = nr;
                    p.c = nc;
                    update_needed = true;
                }
            } else if (action == "PLACE") {
                // Determine color based on ID/Order logic re-check or stored
                // For this simple prototype, we'll assume the assigned color holds.
                // In a real app we'd store color in a map.
                
                Point p = players[id];
                // Check if empty
                bool empty = true;
                for (const auto& s : stones) {
                    if (s.first.r == p.r && s.first.c == p.c) {
                        empty = false;
                        break;
                    }
                }

                if (empty) {
                    stones.push_back({p, color});
                    update_needed = true;
                }
            }
        }

        if (update_needed) {
            std::string msg;
            std::vector<SOCKET> client_copy;
            {
                std::lock_guard<std::mutex> lock(state_mutex);
                msg = serialize_state();
                client_copy = all_clients; // Copy to avoid holding lock during I/O
            }
            broadcast(msg, client_copy);
        }
    }

    // Cleanup
    closesocket(client_socket);
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        players.erase(id);
        std::cout << "Client " << id << " disconnected." << std::endl;
        
        // Remove from client list
        for (auto it = all_clients.begin(); it != all_clients.end(); ) {
            if (*it == client_socket) {
                it = all_clients.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    std::vector<SOCKET> clients;
    int client_id_counter = 0;

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) continue;

        {
            std::lock_guard<std::mutex> lock(state_mutex);
            clients.push_back(client_socket);
        }

        std::thread(handle_client, client_socket, ++client_id_counter, std::ref(clients)).detach();
    }

    WSACleanup();
    return 0;
}
