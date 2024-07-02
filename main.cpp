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
const int berthdist = 500;//泊位间距离
//lostime 货物消失时间
// g++ -O2  main.cpp -o main 
// ./PreliminaryJudge -s 0 -f 0 -d input.txt -m maps/map1.txt "./main.exe"
int d_list[mxgood_num],d0;
int fx[4][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};//右左上下
struct Param{
    db alp,minicapacity,dying,beta,pred,b_broadcast;
    int tiaoshi,footprint,out_sw,rushtime,miniberth;
    Param() 
    {
        alp = 0.6182;
        pred = 0.1352;
        beta = 3.0928; // b_dist影响 考虑beta>alp
        dying = 0.7931;
        footprint = 4;// 足迹 
        b_broadcast = 1.1772;
        miniberth = 13;

        minicapacity = 1.0;//最小运载量
        tiaoshi = 0;
        out_sw = 1;
        rushtime = 12000;
    }
}param;
int cnt_berth = 0,cnt_gd = 0,cnt_boat = 0;
int gd_num,zid,money,boat_capacity;
char ch[n][n];
struct Goods
{
    int x,y,ddl,v;
    int ava;
    Goods(){}
    Goods(int x,int y,int ddl,int v):x(x),y(y),ddl(ddl),v(v){}
};
Goods onmap[mxgood_num];int onmap0=0;
struct Berth// 定义泊位结构体
{
    int x,y,transport_time,loading_speed;
    int gdlist[mxgood_num],g0,g1; //从 1 开始编号，左闭右闭，g1=1
    int duelcap;
    int sumval,tot;
}berth[berth_num];
struct Boat
{
    int pos, status,arr_time;
    int gdlist[mxgood_num],g0,g1;
}boat[boat_num];
db vf[robot_num][mxgood_num];
int di[robot_num][mxgood_num],dib[mxgood_num][berth_num+2];//vf[i][j]表示货物对机器人i的价值，di[i][j]表示货物到机器人的距离
int mp[n][n][4];//[0,1,2,3] 0，港口，机器人，货物编号
int rcap(int i){ return (boat_capacity - (boat[i].g0 - boat[i].g1 + 1));}
int inberth(int x,int y,int sw = 1)
{
    if(sw) return(mp[x][y][1]);
    fo(i,0,berth_num-1)
        if(berth[i].x <= x && berth[i].y <= y && berth[i].x+3 >=x && berth[i].y+3 >=y) return i;
    return -1;
}
int check(int x,int y,int crush = 1,int rid = -1)
{
    if(x < 0 || x >= n || y < 0 || y >= n) return 0;
    if(ch[x][y] == '#') return 0;
    if(crush && mp[x][y][2] != -1 && mp[x][y][2] != rid) return 0;//-1 可通过
    return 1;
}
struct Robot
{
    int x, y, goods,status,val,mt; // match_time 
    int t;  //目标
    int d[n][n],f[n][n];
    int notin(int x,int y,int t)
    {
        if(t<mxgood_num) return(x != onmap[t].x || y != onmap[t].y);
        else 
            return(inberth(x,y) != (t-mxgood_num));
        // return (inberth(x,y) != (t-mxgood_num));
    }
    int qu[mapsize][3],vis[n][n];
    void getmap(int id = -1)
    {
        if(id == -1) return ;
        if(t == -1) 
        {
            f[this->x][this->y] = -1;
            return;
        }
        int hd=0,tl=0;
        fo(i,0,n-1) fo(j,0,n-1) d[i][j] = 1e9,f[i][j]=-1,vis[i][j]=0;
        if(t<mxgood_num)
        {
            qu[++tl][0] = onmap[t].x;qu[tl][1] = onmap[t].y;qu[tl][2] = 0;
            vis[onmap[t].x][onmap[t].y] = 1;
        }
        else
        if(t>=mxgood_num) 
        {
            int b = t-mxgood_num;
            fo(xx,berth[b].x+1,berth[b].x+2) fo(yy,berth[b].y+1,berth[b].y+2)
            {
                qu[++tl][0] = xx,qu[tl][1] = yy;
                qu[tl][2] = 0;
                vis[xx][yy] = 1;
            }
        }
        while(hd++<tl)
        {
            int x = qu[hd][0],y = qu[hd][1];
            d[x][y] = qu[hd][2];
            fo(l,0,3)
            {
                int xx = x+fx[l][0],yy = y+fx[l][1];
                if(check(xx,yy,1,id)) if(vis[xx][yy] == 0) 
                {
                    qu[++tl][0] = xx;qu[tl][1] = yy;
                    qu[tl][2] = qu[hd][2]+1;
                    vis[xx][yy] = 1;
                }
            }
        }

        //更新 mp[x][y][2] 锁定路径
        int xx = this->x,yy = this->y, flag = 0, fp = 1;
        mp[xx][yy][2] = id;
        f[xx][yy] = -1;//静止 
            flag = 0;
            fo(l,0,3)
            {
                int xp = xx + fx[l][0], yp = yy + fx[l][1];
                if(d[xp][yp] == d[xx][yy]-1 && check(xp,yp,1,id))
                {
                    f[xx][yy] = l;
                    xx = xp;yy = yp;
                    mp[xx][yy][2] = id;
                    flag = 1;
                    break;
                }
            }
        if(flag)
        {
            while(notin(xx,yy,t))
            {
                flag = 0;
                fo(l,0,3)
                {
                    int xp = xx + fx[l][0], yp = yy + fx[l][1];
                    if(d[xp][yp] == d[xx][yy]-1 && check(xp,yp,1,id))
                    {
                        f[xx][yy] = l;
                        xx = xp;yy = yp;
                        mp[xx][yy][2] = id;
                        flag = 1;
                        break;
                    }
                }
                if(++fp > param.footprint)
                    break;
            }
            
        }
        else 
        {
            //%4
            int l = rand()%4;
            if(l<=3)
            fo(lc,0,3)
            {
                int xp = xx + fx[l][0], yp = yy + fx[l][1];
                if(check(xp,yp,1,id))
                {
                    f[xx][yy] = l;
                    xx = xp;yy = yp;
                    mp[xx][yy][2] = id;
                    flag = 1;
                    break;
                }
                l=(l+1)%4;
            }
            else f[xx][yy] = -1;
        }
    }
}robot[robot_num];
void get(int i,int j)
{
    printf("get %d\n", i); //机器人i取货
    //入
    robot[i].goods = 1;
    if(dib[j][berth_num+1] < berth_num && dib[j][berth_num] < mapsize*2)
        robot[i].t = mxgood_num + dib[j][berth_num+1];//设 t 为最近港口
    else 
    {
        robot[i].t = mxgood_num + 0;
    }
    robot[i].val = onmap[j].v;
    //出
    d_list[++d0] = j;//dlist回收空间 回收编号（防止编号更改）
    onmap[j].ddl = -1;
    mp[robot[i].x][robot[i].y][3] = -1;// 删除地图货物
}
void pull(int i,int b)
{
    printf("pull %d\n", i); //机器人i卸货
    //入
    berth[b].gdlist[++berth[b].g0] = (robot[i].val);//货物价值
    berth[b].sumval += robot[i].val;
    berth[b].tot ++ ;
    cnt_berth += robot[i].val;
    //出
    robot[i].val = 0;
    robot[i].t = -1;//丢失目标，等待下次
    robot[i].f[robot[i].x][robot[i].y] = -1;// 冗余维护 
}
void move(int i,int f)
{
    if(!check(robot[i].x+fx[f][0],robot[i].y+fx[f][1],1,i)) 
        return;
    printf("move %d %d\n",i,f);// 机器人i向f方向移动
    mp[robot[i].x][robot[i].y][2] = -1; // 可通过了 【出】
    robot[i].x += fx[f][0];
    robot[i].y += fx[f][1];//其实都没用 1 帧只移动 1 次
}
int berth_dist(int x,int y)
{
    if(x==y) return 0;
    if(x>y) swap(x,y);
    if(x==-1) return berth[y].transport_time;
    return berthdist;
}
void go(int i) //驶向虚拟点-1
{
    printf("go %d\n",i);
    int p = boat[i].pos;
    if(p != -1)
        berth[p].duelcap -= rcap(i); //出
    boat[i].g0 = 0;
    boat[i].g1 = 1;
    // 入
    boat[i].pos = -1;
    boat[i].arr_time = zid+berth_dist(p,-1); 
}
void ship(int i,int j)// 船i到泊位j
{
    if(boat[i].pos == j) 
    {
        return ;//不需要动
    }
    printf("ship %d %d\n",i,j);
    int p = boat[i].pos;
    if(p != -1)
        berth[p].duelcap -= rcap(i); //出

    berth[j].duelcap += rcap(i);
    boat[i].pos = j;
    boat[i].arr_time = zid+berth_dist(p,j); // 入
}

db f_pow[mapsize * 2+10],d_pow[lostime * 2],db_pow[mapsize * 2+10];
db power(int x,db y,int sw = 1)
{
    if(sw == 1)
    {
        // if(f_pow[0] == -1) fo(i,0,mapsize*2) f_pow[i] = pow(i,y);
        return f_pow[x];   
    }
    else if(sw == 2)
    {
        // if(d_pow[0] == -1) fo(i,0,lostime) d_pow[i] = pow(i,y);
        return d_pow[x];
    }
    else if(sw == 3)
    {
        // if(db_pow[0] == -1) fo(i,0,mapsize*2) db_pow[i] = pow(i,y);
        return db_pow[x];
    }
    return(pow(x,y));//cmath
}

void Init()
{
    srand(42);//设置随机种子 
    param = Param();
    memset(mp,255,sizeof mp);
    //初始化Onmap
    d0 = 0;
    // d_pow[0] = f_pow[0] = db_pow[0] = -1;
    
    fo(i,0,mapsize*2) f_pow[i] = pow(i,param.alp);
    fo(i,0,lostime) d_pow[i] = pow(i,param.dying);
    fo(i,0,mapsize*2) db_pow[i] = pow(i,param.beta);

    fo(i,0,n-1) scanf("%s", ch[i]); //只保存#和. 其他信息已知了 
    fo(i,0,n-1) fo(j,0,n-1) 
    {
        if(ch[i][j] == 'B') ch[i][j] = '.';//泊位不是障碍
        if(ch[i][j] == 'A') ch[i][j] = '.';//机器人起始位置不是障碍
        if(ch[i][j] == '*') ch[i][j] = '#';//海洋是障碍
    }
    fo(i,0,berth_num-1)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
        berth[id].duelcap = berth[id].g0 = 0;
        berth[id].g1 = 1;
        berth[id].sumval = berth[id].tot = 0;
    }
    
    fo(i,0,berth_num-1)
    {
        fo(xx,berth[i].x,berth[i].x+3) fo(yy,berth[i].y,berth[i].y+3)
            mp[xx][yy][1] = i;
    }

    scanf("%d", &boat_capacity);
    fo(i,0,boat_num-1)
    {
        boat[i].g0 = 0; boat[i].g1 = 1; 
        boat[i].arr_time = 0;
    }
    fo(i,0,robot_num-1)
    {
        memset(robot[i].f,255,sizeof robot[i].f);//=-1
        memset(robot[i].d,127,sizeof robot[i].d);
        robot[i].val = 0;robot[i].t = -1;
    }
    char okk[5];scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}
void Input()
{
    scanf("%d%d", &zid, &money);//帧序号，当前金钱
    scanf("%d", &gd_num);//表示场上新增货物的数量 K<=10
    fo(i,0,gd_num-1)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);//(x,y) 货物val<=200
        cnt_gd += val;
        if(d0 == 0) 
        {
            onmap[onmap0++] = Goods(x,y,lostime+zid,val) ;
        }
        else onmap[d_list[d0]] = Goods(x,y,lostime+zid,val),--d0;
    }
    //遍历onmap，检查lostime>zid的货物并删除 
    fo(it,0,onmap0-1)
    {
        if ((onmap[it].ddl) <= zid && onmap[it].ddl!=-1) 
        {
            int x = onmap[it].x,y = onmap[it].y;
            mp[x][y][3] = -1;
            d_list[++d0] = it;//dlist回收空间 回收编号（防止编号更改）
            onmap[it].ddl = -1;
        }
    }
    
    fo(i,0,robot_num-1)
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &robot[i].status);
    fo(i,0,4)
        scanf("%d%d\n", &boat[i].status, &boat[i].pos);
    char okk[100];
    scanf("%s", okk);
}
int inrobot(int x,int y,int sw = 1)
{
    if(sw) return(mp[x][y][2]);
    return -1;
}
void getmap()
{
    //现在mp[2]保存了已有的路径
    fo(i,0,onmap0-1)
    if(onmap[i].ddl > zid)
        mp[onmap[i].x][onmap[i].y][3] = i;//用于计算 di dib 
}
void di_calc()
{
    // bfs计算货物j到机器人i的距离，保存为di[i][j]; 计算货物j到港口k的距离，保存为dib[j][k];dib[j][berth_num] = min(dib[j][k])
    // 从机器人和港口出发即可 
    int hd = 0,tl = 0;
    int qu[mapsize][3],vis[n][n];
    fo(j,0,onmap0-1) 
    {
        fo(k,0,berth_num+1) dib[j][k] = 1e9;
        fo(i,0,robot_num-1) di[i][j] = 1e9;
    }
    fo(ii,0,robot_num-1 + berth_num) 
    {
        int i = (ii>=robot_num)?(ii-robot_num):ii;
        hd=tl=0;
        if(ii < robot_num)
        {
            qu[++tl][0] = robot[i].x;qu[tl][1] = robot[i].y;qu[tl][2] = 0;
            vis[robot[i].x][robot[i].y] = (ii+1);   
        }
        else 
        {
            fo(xx,berth[i].x+1,berth[i].x+2) fo(yy,berth[i].y+1,berth[i].y+2)
            {
                qu[++tl][0] = xx;qu[tl][1] = yy;qu[tl][2] = 0;
                vis[xx][yy] = (ii+1);   
            }
        }
        while(hd++<tl)
        {
            int x = qu[hd][0],y = qu[hd][1];
            if(mp[x][y][3]!=-1 && mp[x][y][3] >= 0 && mp[x][y][3] <= onmap0-1) 
            {
                if(ii<robot_num) di[i][mp[x][y][3]] = qu[hd][2];
                else 
                    dib[mp[x][y][3]][i] = qu[hd][2];
            }
            fo(l,0,3)
            {
                int xx = x+fx[l][0],yy = y+fx[l][1];
                if(check(xx,yy,0) && (vis[xx][yy] != (ii+1)))
                {
                    qu[++tl][0] = xx;qu[tl][1] = yy;qu[tl][2] = qu[hd][2]+1;
                    vis[xx][yy] = (ii+1);
                }
            }
        }
    }
    fo(j,0,onmap0-1)
    if(onmap[j].ddl > zid)
    {
        dib[j][berth_num] = 1e9;
        fo(k,0,berth_num-1)
            if(dib[j][k] < dib[j][berth_num]) //最近的港口
            {
                dib[j][berth_num] = dib[j][k];
                dib[j][berth_num+1] = k;//货物 j 的最佳港口 k
            }
    }
}
void vf_calc()
{
    fo(i,0,robot_num-1) fo(j,0,onmap0-1)
        vf[i][j] = -1;
    fo(i,0,robot_num-1) fo(j,0,onmap0-1)
    if(onmap[j].ddl>zid && di[i][j]+dib[j][berth_num] < mapsize*2)
    {
        // 货物j 机器人i
        if(di[i][j]+zid > (onmap[j].ddl) ) vf[i][j] = -1; //拿不到 
        else 
        {
            if(zid > param.rushtime) vf[i][j] = 1e6*(db)onmap[j].v / power(di[i][j],param.alp,1)  / power(dib[j][berth_num],3);
            else vf[i][j] = 1e6*(db)onmap[j].v / power(di[i][j],param.alp,1) / power(onmap[j].ddl-zid,param.dying,2) / power(dib[j][berth_num],3);
            int b = dib[j][berth_num + 1];
            if(b >= 0 && b < berth_num && berth[b].duelcap>0)
                vf[i][j] *= param.b_broadcast;        
        }
        
    }
    //计算货物j对机器人i的价值，保存为vf[i][j]
}
#define fd(i,x,y) for(int i=(x);i>=(y);--i)
int f[mxgood_num][(1<<10)+10][4];
db g[mxgood_num][(1<<10)+10];
int needmatch(int i)
{
    // return(robot[i].goods == 0); //忽略物品价值，可以改善“动来动去”的问题 
    return(robot[i].goods == 0 && robot[i].t == -1);
}
void matchup()
{
    int S = (1<<10)-1;
    g[0][0]=f[0][0][1]=f[0][0][2]=0;
    f[0][0][3]=zid;//[3]有效性=zid 代替清零，每帧只有一次matchup
    fo(j,0,onmap0-1) onmap[j].ava = 0;
    fo(i,0,robot_num-1)
    if(robot[i].t>=0 && robot[i].t < onmap0)
        onmap[robot[i].t].ava = 1;

    if(onmap[0].ava == 0 && onmap[0].ddl>zid)
        fo(i,0,robot_num-1)// 机器人i ，货物0
        if(needmatch(i)) //手里没货 return(robot[i].goods == 0 && robot[i].t == -1);
        {
            g[0][(1<<i)] = vf[i][0];
            f[0][(1<<i)][2] = i;
            f[0][(1<<i)][1] = 0;
            f[0][(1<<i)][3] = zid;
        }//因为j=0没有 j-1层

    fo(j,1,onmap0-1)
    {
        g[j-1][0]=0;
        f[j-1][0][1]=f[j-1][0][2]=0;
        f[j-1][0][3]=zid;//[3]有效性=zid
        fo(s,0,S)
            if(f[j-1][s][3]==zid)
            {
                g[j][s] = g[j-1][s];
                fo(l,1,3)
                    f[j][s][l] = f[j-1][s][l];
            }
        fd(s,S,0)
        {

            if(onmap[j].ava == 0 && onmap[j].ddl>zid) //0是可用
            fo(i,0,robot_num-1)// 机器人i ，货物j
            if(needmatch(i)) //手里没货
                if((s&(1<<i)) == 0 && f[j-1][s][3]==zid)
                    if(f[j-1][s][3]==zid && g[j-1][s]+vf[i][j] > g[j][s|(1<<i)] && di[i][j] < mapsize)//i->j
                    {
                        g[j][s|(1<<i)] = g[j-1][s]+vf[i][j];
                        f[j][s|(1<<i)][2] = i;
                        f[j][s|(1<<i)][1] = j;
                        f[j][s|(1<<i)][3] = zid;
                    }
        }
    }
    int st = 0,k=onmap0-1;
    db mx=-1;
    fo(s,0,S) if(g[k][s] > mx && f[k][s][3] == zid)
        st = s ,mx = g[k][s];//“不拿”可能优于拿，包括各种拿不到的问题
    int cnt = 0;
    fo(i,0,robot_num-1)
        if(needmatch(i))
            robot[i].t = -1;//其实没用 needmatch => robot[i]没货+没目标 ;相当于不改变已选的目标 
    
    while(st)
    {
        int i = f[k][st][2],j = f[k][st][1];// 机器人i 货物j
        robot[i].t = j;
        robot[i].mt = zid;
        st ^= (1<<i);
        --k;
        if(++cnt > 10 && param.tiaoshi) 
            break;
    }
}
int onway(int i)
{ 
    return( robot[i].f[robot[i].x][robot[i].y] >=0 && robot[i].f[robot[i].x][robot[i].y] <= 3 );
}
void getroute()
{
    //每次move过后，mp路径标记删除，所以抵达终点时，只留下1个标记 
    fo(i,0,robot_num-1)
        if((!onway(i)) && robot[i].t!=-1 && robot[i].notin(robot[i].x,robot[i].y,robot[i].t))
            robot[i].getmap(i);//锁定路径
    
    fo(i,0,robot_num-1)
    if(robot[i].t!=-1)
    { //运行
        int t = robot[i].t,flag = 0;
        if(!robot[i].notin(robot[i].x,robot[i].y,t))
        {
            flag=1;
            if(t<mxgood_num ) get(i,t);
            else if(t>=mxgood_num) pull(i,t-mxgood_num);// 放货
        }
        if(!flag)//取、放货后丢失目标
        {
            if(onway(i))
                move(i,robot[i].f[robot[i].x][robot[i].y]);
            if(!robot[i].notin(robot[i].x,robot[i].y,t))
            {
                flag=1;
                if(t<mxgood_num ) get(i,t);
                else if(t>=mxgood_num) pull(i,t-mxgood_num);// 放货
            }
        }
    }
}
db getval(int i,int j)// 船 i 港口 j
{
    if(j == 6 && berth[j].duelcap > 0)
        printf("");
    int vsum=0;
    int s=berth[j].g1 + berth[j].duelcap,t = s + min(berth[j].g0 - s + 1,rcap(i)) - 1;
    if(t<s) 
        return -1;
    // s=货物编号开始，t=s+min(港口总货物，船剩余空间)-1
    db tsum=((t-s+1)/berth[j].loading_speed) + berth_dist(boat[i].pos,j);// 装载时间+距离
    db avg = (db)berth[j].sumval / (db)zid, avt = (db)berth[j].tot / (db)zid;
    fo(l,s,t) 
        vsum+=berth[j].gdlist[l];
    //如果船还没装满，计算“预计新到货物的影响”
    if(t - s + 1 < rcap(i))
    {
        db newgood = min((db)(rcap(i)-(t-s+1)),avt * tsum); // 新货物个数（能装走的新货物才算）
        vsum += newgood * avg * param.pred;
    }
    tsum += berth[j].transport_time;  // 加上送走时间 
    return (db)vsum/tsum;
}
void boat_route()
{
    //sts=0 移动中,1 正常停泊装货,2 泊位外等待
    fo(i,0,4)
        if(boat[i].status == 1) //重用 dp
        {
            int p = boat[i].pos,flag=0;
            db mx = 0.1,tmp = 0;int amx = -1;
            //todo：修改策略。 原策略：泊位空才换船 
            if(p==-1 || berth[p].g0 - berth[p].g1 + 1 <= 0 || ( (berth[p].g0 - berth[p].g1 + 1 < berth[p].loading_speed && rcap(i) > 2*(berth[p].g0 - berth[p].g1 + 1)))) //泊位空
            //考虑一次ship会亏1帧的装载时间 
            {
                fo(j,0,berth_num-1)
                if((tmp = getval(i,j))>mx)
                {
                    mx = tmp;
                    amx = j;
                }
                
                if(zid > param.rushtime || berth[amx].g0 - berth[amx].g1 + 1 > param.miniberth)
                if(amx!=-1 && amx != p && berth[amx].transport_time + zid + berth_dist(p,amx) < Z) //剩余时间够送走的 
                    ship(i,amx),flag=1;// 考虑 送走
            }
            p = boat[i].pos;//更新旧的位置 冗余维护 
            if(!flag)
            {
                if(p!=-1 && (boat[i].g0 - boat[i].g1 + 1 >= param.minicapacity * boat_capacity || berth[p].transport_time + zid >= Z)) //冗余时间
                {
                    go(i);
                    flag = 1;
                }
                //模拟 load
                if(!flag)
                {
                    int cnt = min(min(berth[p].loading_speed,rcap(i)), berth[p].g0 - berth[p].g1 + 1);
                    //min 装载速度，剩余容量，泊位剩余货物
                    int s = berth[p].g1,t = berth[p].g1+cnt-1;
                    fo(j,s,t)//
                    {
                        cnt_boat += berth[p].gdlist[j];
                        boat[i].gdlist[++boat[i].g0] = berth[p].gdlist[j];//入
                        berth[p].duelcap--; // 船容量-- => 港口duelcap--

                        berth[p].gdlist[j] = 0;//出
                        berth[p].g1++;
                    }
                }
            }
        }
}
void work()
{
    if(param.tiaoshi && zid%1000 == 0)
        printf("");
    getmap();
    di_calc();
    vf_calc();//计算货物j对机器人i的价值，保存为vf[i][j]
    matchup();
    getroute();//get move pull
    boat_route();// 运行船舶 
}
int main()
{
    if(param.tiaoshi)
        freopen("input.txt","r",stdin);
    Init();
    for(int zhen = 1; zhen <= Z; zhen ++)
    {
        Input();
        work();
        puts("OK");
        fflush(stdout);
    }
    if(param.out_sw)
    {
        freopen("output.txt","a",stdout);
        printf("总掉落%d\n港口：%d\n上船：%d\n",cnt_gd,cnt_berth,money);
        // printf("港口信息：\n");
        // fo(i,0,berth_num-1)
        //     printf("%d %d\n",berth[i].sumval,berth[i].tot);
        fclose(stdout);
    }
    return 0;
}
//todo: 新的船舶策略 +30000p 检查港口货物 
//todo: 每1000帧检查状态 