#include <cstdio>
#include <cstring>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>
#include <queue>
using namespace std;

#define fo(i,x,y) for(int i=(x);i<=(y);++i)
#define db double 

const int n = 200,robot_num = 10,berth_num = 10,Z=15000,lostime = 1000,boat_num = 5,mxgood_num = lostime*10+10,mapsize = n*n+2;
const int berthdist = 500;
const int maxn = 200;
int d_list[mxgood_num],d0;
int fx[4][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};

struct Param{
    db alp,minicapacity,dying,beta,pred,b_broadcast;
    int tiaoshi,footprint,out_sw,rushtime,miniberth;
    Param() {
        alp = 0.6182;
        pred = 0.1352;
        beta = 3.0928;
        dying = 0.7931;
        footprint = 4;
        b_broadcast = 1.1772;
        miniberth = 13;
        minicapacity = 1.0;
        tiaoshi = 0;
        out_sw = 1;
        rushtime = 12000;
    }
}param;

int cnt_berth = 0,cnt_gd = 0,cnt_boat = 0;
int gd_num,zid,money,boat_capacity;
char ch[n][n];

struct Goods {
    int x,y,ddl,v;
    int ava;
    Goods() {}
    Goods(int x,int y,int ddl,int v):x(x),y(y),ddl(ddl),v(v){}
};
Goods onmap[mxgood_num];
int onmap0=0;

struct Berth {
    int x,y,transport_time,loading_speed;
    int gdlist[mxgood_num],g0,g1;
    int duelcap;
    int sumval,tot;
}berth[berth_num];

struct Boat {
    int pos, status,arr_time;
    int gdlist[mxgood_num],g0,g1;
}boat[boat_num];

db vf[robot_num][mxgood_num];
int di[robot_num][mxgood_num],dib[mxgood_num][berth_num+2];
int mp[n][n][4];

int rcap(int i){ 
    return (boat_capacity - (boat[i].g0 - boat[i].g1 + 1));
}

int inberth(int x,int y,int sw = 1) {
    if(sw) return(mp[x][y][1]);
    fo(i,0,berth_num-1)
        if(berth[i].x <= x && berth[i].y <= y && berth[i].x+3 >=x && berth[i].y+3 >=y) return i;
    return -1;
}

int check(int x,int y,int crush = 1,int rid = -1) {
    if(x < 0 || x >= n || y < 0 || y >= n) return 0;
    if(ch[x][y] == '#') return 0;
    if(crush && mp[x][y][2] != -1 && mp[x][y][2] != rid) return 0;
    return 1;
}

struct Robot{
    int x, y, goods,status,val,mt;
    int t;
    int d[n][n],f[n][n];
    int notin(int x,int y,int t) {
        if(t<mxgood_num) return(x != onmap[t].x || y != onmap[t].y);
        else return(inberth(x,y) != (t-mxgood_num));
    }
    int qu[mapsize][3],vis[n][n];
    void getmap(int id = -1) {
        if(id == -1) return ;
        if(t == -1) {
            f[this->x][this->y] = -1;
            return;
        }
        int hd=0,tl=0;
        fo(i,0,n-1) fo(j,0,n-1) d[i][j] = 1e9,f[i][j]=-1,vis[i][j]=0;
        if(t<mxgood_num) {
            qu[++tl][0] = onmap[t].x;qu[tl][1] = onmap[t].y;qu[tl][2] = 0;
            vis[onmap[t].x][onmap[t].y] = 1;
        } else if(t>=mxgood_num) {
            int b = t-mxgood_num;
            fo(xx,berth[b].x+1,berth[b].x+2) fo(yy,berth[b].y+1,berth[b].y+2)
            {
                berth[b].y+1,vis[xx][yy]=1;
                d[xx][yy] = 0;
                qu[++tl][0] = xx;qu[tl][1] = yy;qu[tl][2] = 0;
            }
        }
        while(hd < tl) {
            int x = qu[++hd][0], y = qu[hd][1], v = qu[hd][2];
            fo(i,0,3) {
                int xx = x + fx[i][0], yy = y + fx[i][1];
                if(check(xx,yy,1,t) && d[xx][yy] > v + 1) {
                    d[xx][yy] = v + 1;
                    qu[++tl][0] = xx;qu[tl][1] = yy;qu[tl][2] = v + 1;
                    f[xx][yy] = i;
                    vis[xx][yy]=1;
                }
            }
        }
    }
    void moveto(int id) 
    {
        if(t == -1) return ;
        int tt = t;
        if(t >= mxgood_num) {
            int b = t-mxgood_num;
            tt = berth[b].x*mxgood_num + berth[b].y + mxgood_num;
        }
        int x = tt/n, y = tt%n;
        if(x == this->x && y == this->y) return;
        int dir = f[x][y];
        if(dir == -1) return;
        this->x += fx[dir][0];
        this->y += fx[dir][1];
        return;
    }
    void check_map(int id) {
        getmap(id);
        moveto(id);
    }
}robot[robot_num];

int Time=0,Total=0;

void disp(int id) {
    printf("当前时间:%d 船只数量:%d 运输总价值:%d\n",Time,cnt_boat,Total);
    fo(i,0,cnt_boat-1) {
        printf("第%d艘船 ",i);
        printf("位置%d ",boat[i].pos);
        printf("状态%d ",boat[i].status);
        printf("到达时间%d ",boat[i].arr_time);
        printf("载货量%d ",boat[i].g0 - boat[i].g1);
        printf("货物列表");
        fo(j,boat[i].g1,boat[i].g0-1) printf("(%d,%d,%d) ",onmap[boat[i].gdlist[j]].x,onmap[boat[i].gdlist[j]].y,onmap[boat[i].gdlist[j]].v);
        puts("");
    }
    fo(i,0,cnt_berth-1) {
        printf("第%d个泊位 ",i);
        printf("位置(%d,%d) ",berth[i].x,berth[i].y);
        printf("泊位最大容量:%d ",berth[i].duelcap);
        printf("货物列表");
        fo(j,berth[i].g0,berth[i].g1-1) printf("(%d,%d,%d) ",onmap[berth[i].gdlist[j]].x,onmap[berth[i].gdlist[j]].y,onmap[berth[i].gdlist[j]].v);
        puts("");
    }
    puts("");
}

int eee;
int calcdis(int x,int y,int t) {
    int ret;
    if(t < mxgood_num) {
        int st = onmap[t].x * n + onmap[t].y, ed = x * n + y;
        ret = (d_list[st] == d0 ? dib[t][eee] : d_list[st]) + dib[t][eee] + (d_list[ed] == d0 ? dib[t][eee] : d_list[ed]);
    } else {
        int b = t-mxgood_num;
        int st = berth[b].x * n + berth[b].y + mxgood_num, ed = x * n + y;
        ret = (d_list[st] == d0 ? dib[t][eee] : d_list[st]) + dib[t][eee] + (d_list[ed] == d0 ? dib[t][eee] : d_list[ed]);
    }
    return ret;
}

int h[maxn],nex[maxn],to[maxn],g[maxn],c[maxn];
int S,T,flow,cost;
void add(int x,int y,int z,int w) {
    nex[++tot] = h[x]; h[x] = tot; to[tot] = y; g[tot] = z; c[tot] = w;
    nex[++tot] = h[y]; h[y] = tot; to[tot] = x; g[tot] = 0; c[tot] = -w;
}
bool vis[maxn];
int dis[maxn],pre[maxn],a[maxn];
bool spfa() {
    memset(dis,0x3f,sizeof(dis));
    memset(vis,0,sizeof(vis));
    memset(pre,0,sizeof(pre));
    dis[S] = 0; vis[S] = 1; a[S] = 1e9;
    queue<int> q; q.push(S);
    while(q.size()) {
        int x = q.front(); q.pop(); vis[x] = 0;
        for(int i=h[x];i;i=nex[i]) {
            int y = to[i];
            if(g[i] && dis[y] > dis[x]+c[i]) {
                dis[y] = dis[x]+c[i]; pre[y] = i;
                a[y] = min(a[x],g[i]);
                if(!vis[y]) vis[y] = 1, q.push(y);
            }
        }
    }
    return dis[T] != 0x3f3f3f3f;
}
void mcmf() {
    while(spfa()) {
        int x = T;
        flow += a[T]; cost += dis[T]*a[T];
        while(x != S) {
            g[pre[x]] -= a[T]; g[pre[x]^1] += a[T];
            x = to[pre[x]^1];
        }
    }
}

int dis[maxn][maxn];

void pre_work() {
    for(int i=1;i<=n;i++) for(int j=1;j<=n;j++) dis[i][j] = abs(i-1-n/2) + abs(j-1-n/2);
    for(int k=1;k<=n;k++) for(int i=1;i<=n;i++) for(int j=1;j<=n;j++) dis[i][j] = min(dis[i][j],dis[i][k]+dis[k][j]);
}
int main() {
    scanf("%d%d%d",&n,&m,&eee);
    pre_work();
    for(int i=1;i<=m;i++) scanf("%d%d%d",&onmap[i].x,&onmap[i].y,&onmap[i].v);
    for(int i=1;i<=n;i++) for(int j=1;j<=n;j++) scanf("%d",&d_list[i*n+j]);
    for(int i=1;i<=m;i++) for(int j=1;j<=m;j++) dib[i][j] = calcdis(onmap[i].x,onmap[i].y,j);
    S = 2*m+1; T = 2*m+2;
    for(int i=1;i<=m;i++) {
        add(S,i,1,0);
        add(i+m,T,1,0);
        for(int j=1;j<=m;j++) add(i,j+m,1,dib[j][i]);
    }
    mcmf();
    printf("%d\n",cost);
    for(int i=1;i<=m;i++) {
        add(S,i,1,0);
        add(i+m,T,1,0);
        for(int j=1;j<=m;j++) add(i,j+m,1,-dib[j][i]);
    }
    mcmf();
    printf("%d\n",-cost);
    for(int i=1;i<=m;i++) {
        add(S,i,1,0);
        add(i+m,T,1,0);
        for(int j=1;j<=m;j++) add(i,j+m,1,dib[i][j]);
    }
    mcmf();
    printf("%d\n",cost);
    for(int i=1;i<=m;i++) {
        add(S,i,1,0);
        add(i+m,T,1,0);
        for(int j=1;j<=m;j++) add(i,j+m,1,-dib[i][j]);
    }
    mcmf();
    printf("%d\n",-cost);
    return 0;
}
