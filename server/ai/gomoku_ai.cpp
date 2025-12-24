// 컴파일 : g++ -O2 gomoku_ai.cpp -std=c++17 -o gomoku
#include <bits/stdc++.h>
using namespace std;

constexpr int N = 15;
constexpr int INF = 1e9;

int dx[4] = {1,0,1,1};
int dy[4] = {0,1,1,-1};

struct Board {
    int g[N][N]{};
    int turn = 1;

    bool inside(int x,int y) const {
        return x>=0 && x<N && y>=0 && y<N;
    }

    void play(int x,int y){
        g[x][y] = turn;
        turn = -turn;
    }
    void undo(int x,int y){
        turn = -turn;
        g[x][y] = 0;
    }

    bool win_after(int x,int y) const {
        int p = g[x][y];
        for(int d=0;d<4;d++){
            int cnt = 1;
            for(int s=1;s<5;s++){
                int nx=x+dx[d]*s, ny=y+dy[d]*s;
                if(!inside(nx,ny)||g[nx][ny]!=p) break;
                cnt++;
            }
            for(int s=1;s<5;s++){
                int nx=x-dx[d]*s, ny=y-dy[d]*s;
                if(!inside(nx,ny)||g[nx][ny]!=p) break;
                cnt++;
            }
            if(cnt>=5) return true;
        }
        return false;
    }

    vector<int> candidates() const {
        bool near[N][N]{};
        bool hasStone=false;
        for(int i=0;i<N;i++)
            for(int j=0;j<N;j++)
                if(g[i][j]!=0){
                    hasStone=true;
                    for(int dx=-2;dx<=2;dx++)
                        for(int dy=-2;dy<=2;dy++){
                            int ni=i+dx, nj=j+dy;
                            if(inside(ni,nj))
                                near[ni][nj]=true;
                        }
                }

        vector<int> res;
        if(!hasStone){
            res.push_back(7*15+7);
            return res;
        }

        for(int i=0;i<N;i++)
            for(int j=0;j<N;j++)
                if(near[i][j] && g[i][j]==0)
                    res.push_back(i*15+j);
        return res;
    }
};

/* ---------- 패턴 판별 ---------- */

int count_dir(const Board& b,int x,int y,int d,int p){
    int cnt=0;
    for(int s=1;s<5;s++){
        int nx=x+dx[d]*s, ny=y+dy[d]*s;
        if(!b.inside(nx,ny)||b.g[nx][ny]!=p) break;
        cnt++;
    }
    return cnt;
}

bool open_end(const Board& b,int x,int y,int d,int len){
    int nx=x+dx[d]*(len+1), ny=y+dy[d]*(len+1);
    return b.inside(nx,ny) && b.g[nx][ny]==0;
}

bool makes_open_four(Board& b,int x,int y){
    int p=b.turn;
    b.g[x][y]=p;
    for(int d=0;d<4;d++){
        int l=count_dir(b,x,y,d,p);
        int r=count_dir(b,x,y,(d+2)%4,p);
        int cnt=l+r+1;
        if(cnt==4 && open_end(b,x,y,d,l) && open_end(b,x,y,(d+2)%4,r)){
            b.g[x][y]=0;
            return true;
        }
    }
    b.g[x][y]=0;
    return false;
}

bool makes_open_three(Board& b,int x,int y){
    int p=b.turn;
    b.g[x][y]=p;
    for(int d=0;d<4;d++){
        int l=count_dir(b,x,y,d,p);
        int r=count_dir(b,x,y,(d+2)%4,p);
        int cnt=l+r+1;
        if(cnt==3 && open_end(b,x,y,d,l) && open_end(b,x,y,(d+2)%4,r)){
            b.g[x][y]=0;
            return true;
        }
    }
    b.g[x][y]=0;
    return false;
}

/* ---------- 즉시 승 / 차단 ---------- */

bool immediate_win(Board& b,int& best){
    for(int m:b.candidates()){
        int x=m/15,y=m%15;
        b.play(x,y);
        if(b.win_after(x,y)){
            b.undo(x,y);
            best=m;
            return true;
        }
        b.undo(x,y);
    }
    return false;
}

/* ---------- 위협 공간 탐색 ---------- */

bool threat_search(Board& b,int depth){
    if(depth==0) return false;

    for(int m:b.candidates()){
        int x=m/15,y=m%15;
        if(!makes_open_four(b,x,y) && !makes_open_three(b,x,y)) continue;

        b.play(x,y);
        if(b.win_after(x,y)){
            b.undo(x,y);
            return true;
        }

        bool blocked=false;
        for(int r:b.candidates()){
            int rx=r/15, ry=r%15;
            b.play(rx,ry);
            if(!threat_search(b,depth-1))
                blocked=true;
            b.undo(rx,ry);
            if(blocked) break;
        }

        b.undo(x,y);
        if(!blocked) return true;
    }
    return false;
}

/* ---------- 평가 ---------- */

int evaluate(const Board& b){
    int score=0;
    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++)
            if(b.g[i][j]==0){
                Board t=b;
                t.play(i,j);
                if(makes_open_four(t,i,j)) score+=100000;
                if(makes_open_three(t,i,j)) score+=10000;
            }
    return score;
}

/* ---------- 네가맥스 ---------- */

int negamax(Board& b,int depth,int alpha,int beta){
    if(depth==0) return evaluate(b);

    for(int m:b.candidates()){
        int x=m/15,y=m%15;
        b.play(x,y);
        int v=-negamax(b,depth-1,-beta,-alpha);
        b.undo(x,y);
        alpha=max(alpha,v);
        if(alpha>=beta) break;
    }
    return alpha;
}

/* ---------- 최종 AI ---------- */

int choose_move(Board& b){
    int m;

    if(immediate_win(b,m)) return m;

    Board tmp=b;
    tmp.turn=-tmp.turn;
    if(immediate_win(tmp,m)) return m;

    for(int c:b.candidates()){
        int x=c/15,y=c%15;
        b.play(x,y);
        if(threat_search(b,2)){
            b.undo(x,y);
            return c;
        }
        b.undo(x,y);
    }

    int best=-INF, bestMove=-1;
    for(int c:b.candidates()){
        int x=c/15,y=c%15;
        b.play(x,y);
        int v=-negamax(b,2,-INF,INF);
        b.undo(x,y);
        if(v>best){
            best=v;
            bestMove=c;
        }
    }
    return bestMove;
}

/* ---------- main ---------- */

int main(){
    Board b;
    while(true){
        int x,y;
        cin>>x>>y;
        if(x==-1) break;
        b.play(x,y);

        int ai=choose_move(b);
        cout<<ai/15<<" "<<ai%15<<endl;
        b.play(ai/15,ai%15);
    }
}
