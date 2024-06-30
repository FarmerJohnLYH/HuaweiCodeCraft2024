// #include <bits/stdc++.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
using namespace std;

#define fo(i,x,y) for(int i=(x);i<=(y);++i)
#define db double 
const int n = 200;
const int robot_num = 10;
const int berth_num = 10;
const int N = 210,Z=15000;
// ./PreliminaryJudge -m maps/map1.txt "./main"
// ./PreliminaryJudge -f 0 -m maps/map1.txt "./main"

// 定义机器人结构体
struct Robot
{
    int x, y, goods;
    int status;
    int mbx, mby;
    Robot() {}
    Robot(int startX, int startY) {
        x = startX;
        y = startY;
    }
}robot[robot_num + 10];

// 定义泊位结构体
struct Berth
{
    int x;
    int y;
    int transport_time;
    int loading_speed;
    int capacity;// 剩余运力 = \sigma boat[i].capacity
    int vob,goods;//泊位上的价值，货物数
    Berth(){}
    Berth(int x, int y, int transport_time, int loading_speed) {
        this -> x = x;
        this -> y = y;
        this -> transport_time = transport_time;
        this -> loading_speed = loading_speed;
    }
}berth[berth_num + 10];

// 定义船结构体
struct Boat
{
    int pos, status, capacity;
    //capacity 剩余运力
}boat[10];

int money, boat_capacity, id,boat_num = 5;
char ch[N][N];
// ‘.’ ： 空地
// ‘*’ ： 海洋
// ‘#’ ： 障碍
// ‘A’ ： 机器人起始位置，总共 10 个。
// ‘B’ ： 大小为 4*4，表示泊位的位置,泊位标号在后泊位处初始化。
int gds[N][N],st[N][N];//货物位置
int zhen;
int dis[berth_num+2][N][N],fdis[N][N][2];
db smel[N][N];// x,y气味浓度, 计算f = val/(dist^2)
int ff[4][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};//右左上下
int d[N*N][3];//队列 [0,1]=(x,y) [2]=dist
int bz[N][N];
// 初始化函数

int check(int x,int y,int crush=1)//考虑机器人碰撞
{
    if(crush==0)// 不考虑碰撞
        return (x>=1&&x<=n&&y>=1&&y<=n&&(ch[x][y]!='#' && ch[x][y]!='*'));
    return (x>=1&&x<=n&&y>=1&&y<=n&&(ch[x][y]!='#' && ch[x][y]!='*'&& ch[x][y]!='A'));
}
int value(int x,int y)
{
    return (x*n+y+1);
}
void flood_fill()
{
    memset(dis,127,sizeof(dis));
    // 以 10 个泊位为中心，向外扩散，直到遇到障碍或者边界。
    // 共用队列 d
    int hd=0,tl=0,xx,yy;
    // 10个泊位分别跑，代码更简洁 
    fo(i,0,berth_num-1)
    {
        hd = tl = 0;
        fo(x,berth[i].x,berth[i].x+3) fo(y,berth[i].y,berth[i].y+3)
        {
            ch[x][y] = '0'+i;
            d[++tl][0]=x;d[tl][1]=y;
            d[tl][2]=berth[i].transport_time;
            dis[i][x][y]=berth[i].transport_time;
        }
        int x,y;
        int bzv = value(berth[i].x,berth[i].y);
        while(hd++<tl)
        {
            x=d[hd][0];y=d[hd][1];
            fo(l,0,3)
            {
                xx=x+ff[l][0];yy=y+ff[l][1];
                if(check(xx,yy,0)) // 不考虑碰撞的 check
                {
                    if(bz[xx][yy]==bzv) continue;
                    bz[xx][yy]=bzv;
                    d[++tl][0]=xx;d[tl][1]=yy;
                    dis[i][xx][yy]=d[tl][2]=d[hd][2]+1;
                }
            }
        }

    }
    fo(x,0,n-1) fo(y,0,n-1)
    {
        fdis[x][y][0]=1e9;
        // minimize (transport_time + dist), 设 dis[l][x][y] 表示从(x,y)到泊位l的最短距离+transport_time
        // 设fdis[x][y] 表示 ，为抵达“代价最小”泊位，应该选择的方向; 若有两个，则随机选择 
        // fdis[x][y][1] = argmin(dis[l][x][y]), l=0~9, 
        // fdis[x][y][0] = min(dis[l][x][y])
        fo(l,0,berth_num-1)
            fdis[x][y][0] = min(fdis[x][y][0],dis[l][x][y]);
    }
    fo(x,0,n-1) fo(y,0,n-1)
    {
        fdis[x][y][1]=4;
        fo(l,0,3)
        {
            if(check(x+ff[l][0],y+ff[l][1],0))
                if(fdis[x+ff[l][0]][y+ff[l][1]][0]+1==fdis[x][y][0])
                {
                    if(fdis[x][y][1]==4)
                        fdis[x][y][1]=l;
                    else if(rand()&1) fdis[x][y][1]=l;
                }
        }
    }
}
void Init()
{
    // freopen("input.txt", "r", stdin);
    for(int i = 1; i <= n; i ++)
        scanf("%s", ch[i] + 1);
    for(int i = 0; i < berth_num; i ++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
        //id(0 <= id < 10)为该泊位的唯一标号，(x,y)表示该泊位的左上角坐标
        //time(1 <= time <= 1000)表示该泊位轮船运输到虚拟点的时间(虚拟点移动到泊位的时间同)，即产生价值的时间，时间用帧数表示。
        //Velocity(1 <= Velocity <= 5)表示该泊位的装载速度，即每帧可以装载的物品数，单位是：个。（装载是很快的）
        //保证对于每次提交全部泊位到虚拟点时间和相同。
        //lyh 这里怎么对装卸做最优化？ 
        // minimize (transport_time + dist), 设 dis[l][x][y] 表示从(x,y)到泊位l的最短距离+transport_time
        // 设fdis[x][y] 表示 ，为抵达“代价最小”泊位，应该选择的方向; 若有两个，则随机选择 
        // fdis[x][y][1] = argmin(dis[l][x][y]), l=0~9, 
        // fdis[x][y][0] = min(dis[l][x][y])
    }
    flood_fill();
    scanf("%d", &boat_capacity);
    //  capacity(1 <= capacity <= 1000)，表示船的容积，即最多能装的物品数。
    char okk[100];
    scanf("%s", okk);
    // 初始化工作
    printf("OK\n");
    fflush(stdout);
}
void run(int x,int y,int val)
{
    if(val==0) return;
    val = val*160000;
    int hd=0,tl=1,xx,yy;
    int bzv = value(x,y);// 根据起始点设置标记值，代替清零
    bz[x][y] = bzv;
    d[1][0]=x;d[1][1]=y;d[1][2]=0;
    smel[xx][yy]+=(db)val;
    while(hd++<tl)
    {
        x=d[hd][0];y=d[hd][1];
        fo(i,0,3)
        {
            xx=x+ff[i][0];yy=y+ff[i][1];
            if(check(xx,yy)) // 考虑机器人位置的 check
            {
                if(bz[xx][yy]==bzv) continue;
                bz[xx][yy]=bzv;
                d[++tl][0]=xx;d[tl][1]=yy;d[tl][2]=d[hd][2]+1;
                smel[xx][yy]+=(db)val/(db)d[tl][2]/(db)d[tl][2];
            }
        }
    }
}
// 输入函数
const int lostime = 1000;//货物消失时间
int vallist[Z*10][3],v0,v1=1;//(x,y),time
void get(int i)
{
    printf("get %d\n", i);
    robot[i].goods=1;
    int x=robot[i].x,y=robot[i].y;
    run(x,y,-gds[x][y]);// 去除气味
}
void pull(int i)
{
    printf("pull %d\n", i);
    int b = ch[robot[i].x][robot[i].y]-'0';
    berth[b].vob+=robot[i].goods;
    berth[b].goods++;
    robot[i].goods=0;
}
void move(int i,int f)
{
    printf("move %d %d\n",i,f);
    ch[robot[i].x][robot[i].y] = '.';
    robot[i].x+=ff[f][0];
    robot[i].y+=ff[f][1];
    ch[robot[i].x][robot[i].y] = 'A';
}
void go(int i)
{
    printf("go %d\n",i);//驶向虚拟点-1
    // boat[i].status=2;
    boat[i].pos=-1;
    boat[i].capacity=boat_capacity;
}
void ship(int i,int j)
{
    printf("ship %d %d\n",i,j);
    if(boat[i].pos!=-1)
    {
        int k=boat[i].pos;
        berth[k].capacity-=boat[i].capacity;
        berth[i].capacity+=boat[i].capacity;
        // boat[i].capacity=0;
    }
    boat[i].pos=j;
    boat[i].status=0;
}
int maxvob()
{
    int mx=0,argf=rand()%berth_num;
    fo(i,0,berth_num-1)
    {
        if(berth[i].vob>mx && berth[i].goods - berth[i].capacity>0)
        //剩余货物>预定运力
        {
            mx=berth[i].vob;
            argf=i;
        }
    }
    return argf;
}
int Input()
{
    scanf("%d%d", &id, &money);//帧序号，当前金钱
    int num;
    scanf("%d", &num);//表示场上新增货物的数量 K<=10
    for(int i = 1; i <= num; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        if(gds[x][y]>0)
            // 货物位置重复? Admin：不存在。物品只能生成在空地 
            gds[x][y] = max(gds[x][y], val);
            // gds[x][y]+=val;
        else
            gds[x][y]=val; 
        st[x][y] = zhen;
        run(x,y,val);//扩散气味
        //坐标x,y ,金额val<=200
        vallist[++v0][0]=x;vallist[v0][1]=y;vallist[v0][2]=zhen+lostime;
    }
    //检查货物消失  
    for(;v1<=v0;++v1)
    {
        if(vallist[v1][2]<=zhen)
            run(vallist[v1][0],vallist[v1][1],-gds[vallist[v1][0]][vallist[v1][1]]);// 不考虑货物重复
        else break;
    }
    //机器人状态
    for(int i = 0; i < robot_num; i ++)
    {
        int sts;
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &sts);
        if(sts==1)// 正常运行
        {
            //lyh 
            if(robot[i].goods==0)// 可以取货
            {
                if(gds[robot[i].x][robot[i].y]>0) get(i);//id=i
                else
                {
                    //往货物方向移动
                    int x=robot[i].x,y=robot[i].y;
                    int xx,yy,argf=4;
                    db mx=-1e9;
                    fo(i,0,3)
                    {
                        xx=x+ff[i][0];yy=y+ff[i][1];
                        if(check(xx,yy)&&smel[xx][yy]>mx)
                        {
                            mx=smel[xx][yy];
                            argf = i;
                        }
                    }
                    if(argf<4)
                        move(i,argf);
                }
            }// 取货 or 不取货、向货物移动
            if(robot[i].goods==1)// 有货物，向泊位移动（必然先前没移动）
            {
                //往泊位方向移动
                if(fdis[robot[i].x][robot[i].y][1] < 4) 
                    move(i,fdis[robot[i].x][robot[i].y][1]);
                if(ch[robot[i].x][robot[i].y]=='B')//到达泊位
                    pull(i); // 卸货
            }
        }
        //不考虑恢复的问题先，假设自动恢复
        //goods 是否携带1/0 ,坐标(x,y) ,  0 表示恢复状态 1 表示正常运行状态
    }
    for(int i = 0; i < 5; i ++)
        scanf("%d%d\n", &boat[i].status, &boat[i].pos);
    fo(i,0,4)
    {
        //towrite
        //船舶到达泊位 -> 生成场面信息 -> 执行船舶指令 ->泊位装卸货物。
        if(boat[i].status==1)
        {
            //boat.capacity 剩余运力
            if(boat[i].capacity==0 || (boat[i].pos!=-1 && zhen+berth[boat[i].pos].transport_time >= Z-2)) go(i);
            else 
            if(boat[i].pos!=-1)
            {
                if(berth[boat[i].pos].goods==0) ship(i,maxvob());
            }
            else ship(i,maxvob()); //boat[i].pos==-1
        }
        if(boat[i].status == 2) i=i;//继续等待
        if(boat[i].status == 0) i=i;//考虑所有决策理性，不破坏“移动中的船”
    }
    //计算装载对各状态的影响 
    fo(i,0,4)
    if(boat[i].status == 1 && boat[i].pos!=-1)
    {
        int k=boat[i].pos;
        if(berth[k].goods>0 && boat[i].capacity>0)
        {
            int t=min(boat[i].capacity,berth[k].loading_speed);
            boat[i].capacity-=t;
            berth[k].goods-=t;
            // berth[k].vob-=t;
        }
    }

    //船的状态 0 表示移动(运输)中 1 表示正常运行状态(即装货状态或运输完成状态)
    //2 表示泊位外等待状态
    //表示目标泊位，如果目标泊位是虚拟点，则pos=-1
    char okk[100];
    scanf("%s", okk);
    return id;
}

int main()
{
    Init();
    for(zhen = 1; zhen <= 15000; zhen ++)
    {
        int id = Input();
        // for(int i = 0; i < robot_num; i ++)
        //     printf("move %d %d\n", i, rand() % 4);
        puts("OK");
        fflush(stdout);
    }

    return 0;
}
