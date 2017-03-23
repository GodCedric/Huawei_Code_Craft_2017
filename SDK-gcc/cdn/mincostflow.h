//最小费用流类

#ifndef __MINCOSTFLOW__H__
#define __MINCOSTFLOW__H__

//#include <iostream>
//#include <vector>
//#include <queue>
#include <stack>
//#include <algorithm>
#include <cstdio>
#include <cstring>
#include "graph.h"

using namespace std;

//队列元素
typedef pair<int, int> P;//first保存最短距离，second保存顶点的编号

//残存网络边
struct Edge_MCF
{
    int to, cap, cost, rev;//终点，残存容量，单位费用，反向边编号
    Edge_MCF(int t, int c, int cc, int r):to(t), cap(c), cost(cc), rev(r){}
};

//最小费用流类
class MCF{
private:
    Graph graph;                    //关联图
    int nodeNum;                    //网络节点数目

    vector<Edge_MCF> G[MAXN];       //残存网络邻接表
    bool inq[MAXN];                  //节点入队标志位
    int dist[MAXN];                 //最短距离
    int prevv[MAXN];                //最短路中的父结点
    int preve[MAXN];                //最短路中的父边编号
    int addflow[MAXN];              //可增加量

    vector<int> consumerNetNodes;   //消费节点所连网络节点
    int f_all;                      //消费节点容量需求和
    priority_queue<P, vector<P>, greater<P> >q;//队列

public:
    //构造函数
    MCF(int n, Graph graph):nodeNum(n){

        this->graph = graph;

        //按图创建残存网络
        for (int i=0;i<n;i++)
        {
            for(int j=0;j<graph.G[i].size();j++){
                addEdge(graph.G[i][j].from,graph.G[i][j].to,
                            graph.G[i][j].cap,graph.G[i][j].cost);
            }
        }

        //初始化consumerNetNodes,f
        for(int i=0;i<graph.consumerNum;i++){
            consumerNetNodes.push_back(graph.consumers[i].netNode);
            f_all += graph.consumers[i].flowNeed;
        }

    }

    //给残存网络中添加边（上下行链路及其反向边）
    void addEdge(int from, int to, int cap, int cost)
    {
        //  上行路及其反向边
        G[from].push_back(Edge_MCF( to, cap, cost, G[to].size()));
        G[to].push_back(Edge_MCF( from, 0, -cost, G[from].size() - 1 ));
        // 下行路及其反向边
        G[to].push_back(Edge_MCF( from, cap, cost, G[from].size()));
        G[from].push_back(Edge_MCF( to, 0, -cost, G[to].size() - 1));
    }
    //超级源点，超级汇点添加边
    void addEdge(vector<Edge_MCF> G1[],int from,int to,int cap,int cost){
        G1[from].push_back(Edge_MCF( to, cap, cost, G[to].size()));
        G1[to].push_back(Edge_MCF( from, 0, -cost, G[from].size() - 1 ));
    }

    //多源多汇最小费用流算法
    int multiMinCostFlow(const vector<int> &servers,            //服务器位置
                          vector<int> minCostPath[],int &m){   //路径及路径数目
         //创建残量网络副本
         //vector<Edge_MCF> G1[MAXN];
         //copy(this->G,this->G+MAXN,G1);
         vector<Edge_MCF> G1[nodeNum+2];
         copy(this->G,this->G+nodeNum+2,G1);

         //加入超级源与超级汇
         int superServer = nodeNum;//超级源
         for(int i=0;i<servers.size();i++){
             //超级源与每个服务器建立边：费用0，容量无穷
             addEdge(G1,superServer,servers[i],INF,0);
         }
         int superConsumerNetNode = nodeNum+1;//超级汇
         for(int i=0;i<consumerNetNodes.size();i++){
             //超级汇与每个消费结点所连的网络结点建立边：费用为流量需求，容量无穷
             addEdge(G1,consumerNetNodes[i],superConsumerNetNode,graph.consumers[i].flowNeed,0);
         }

         //最小费用流
         //初始化参数
         int pathCnt = 0;       //路径数目
         int f = f_all;         //流量需求
         int flowNeed = f;
         int res = 0;           //返回值：存在路径解时返回最小费用，否则返回-1
         for(int i=0;i<m;i++){
             minCostPath[i].clear();
         }

         while (f>0)//未满足流量需求时继续寻找
         {
             //spfa算法求解最短路径
             //priority_queue<P, vector<P>, greater<P> >q;//队列
             //queue<int> q;
             fill(dist, dist+nodeNum+2, INF);//距离初始化为INF
             fill(inq, inq+nodeNum+2, false);//入队标志初始化
             dist[superServer] = 0;
             addflow[superServer] = INF;
             q.push(P(0, superServer));
             inq[superServer] = true;
             while (!q.empty())
             {
                 P p = q.top();
                 q.pop();
                 int v = p.second;
                 inq[v] = false;
                 for (int i = 0; i<G1[v].size(); i++)
                 {
                     Edge_MCF &e = G1[v][i];
                     if (e.cap>0 && dist[e.to]>dist[v]+e.cost)//松弛操作
                     {
                         dist[e.to] = dist[v] + e.cost;
                         prevv[e.to] = v;//更新父结点
                         preve[e.to] = i;//更新父边编号
                         addflow[e.to] = min(addflow[v], e.cap);
                         if(!inq[e.to]){
                            q.push(P(dist[e.to], e.to));
                            inq[e.to] = true;
                         }

                     }
                 }
             }

             //判断是否存在最短路径，若否，返回最大值
             if (dist[superConsumerNetNode] == INF){
                 return INF;
             }

             //求取最短路径最小流量增加量
             //int d = f;
             int d = addflow[superConsumerNetNode];
             stack<int> stk;        //存储路径编号栈
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x]){
                 stk.push(x);
                 //d = min(d, G1[prevv[x]][preve[x]].cap);//从t出发沿着最短路返回s找可改进量
             }

             //将该路径存入minCostPath
             int node;
             while(!stk.empty()){
                 node = stk.top();
                 stk.pop();
                 minCostPath[pathCnt].push_back(node);
                 //cout<<node<<' ';
             }

             //去除超级汇点并加入消费节点，路径流量
             vector<int>::iterator endx = minCostPath[pathCnt].end();
             minCostPath[pathCnt].erase(endx-1);//去掉超级汇点
             node = minCostPath[pathCnt][minCostPath[pathCnt].size()-1];
             minCostPath[pathCnt].push_back(this->graph.netToConsumer[node]);//加入消费结点
             if(d <= flowNeed){
                 minCostPath[pathCnt].push_back(d);//实际流量
                 //cout<<d<<endl;
             }
             else{
                 minCostPath[pathCnt].push_back(flowNeed);
                 //cout<<d<<endl;
             }

             //更新路径及费用
             pathCnt++;
             f -= d;
             res += d*dist[superConsumerNetNode];//h[t]表示最短距离的同时，也代表了这条最短路上的费用之和，乘以流量d即可得到本次增广所需的费用
             //cout<<f<<" "<<res<<endl;
             //修改网络残量值
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G1[prevv[x]][preve[x]];
                 e.cap -= d;
                 G1[x][e.rev].cap += d;

             }
         }
         m=pathCnt;
         return res;
    }


    //多源多汇最小费用流算法，用于求fit，即不需要求path
    int multiMinCostFlow2(const vector<int> &servers){
         //创建残量网络副本
         //vector<Edge_MCF> G1[MAXN];
         //copy(this->G,this->G+MAXN,G1);
         vector<Edge_MCF> G1[nodeNum+2];
         copy(this->G,this->G+nodeNum+2,G1);

         //加入超级源与超级汇
         int superServer = nodeNum;//超级源
         for(int i=0;i<servers.size();i++){
             //超级源与每个服务器建立边：费用0，容量无穷
             addEdge(G1,superServer,servers[i],INF,0);
         }
         int superConsumerNetNode = nodeNum+1;//超级汇
         for(int i=0;i<consumerNetNodes.size();i++){
             //超级汇与每个消费结点所连的网络结点建立边：费用为流量需求，容量无穷
             addEdge(G1,consumerNetNodes[i],superConsumerNetNode,graph.consumers[i].flowNeed,0);
         }

         //最小费用流
         //初始化参数
         int f = f_all;         //流量需求
         int flowNeed = f;
         int res = 0;           //返回值：存在路径解时返回最小费用，否则返回-1

         while (f>0)//未满足流量需求时继续寻找
         {
             //spfa算法求解最短路径
             //priority_queue<P, vector<P>, greater<P> >q;//队列
             fill(dist, dist + nodeNum+2, INF);//距离初始化为INF
             fill(inq, inq+nodeNum+2, false);//入队标志初始化
             dist[superServer] = 0;
             addflow[superServer] = INF;
             q.push(P(0, superServer));
             inq[superServer] = true;
             while (!q.empty())
             {
                 P p = q.top();
                 q.pop();
                 int v = p.second;
                 inq[v] = false;
                 for (int i = 0; i<G1[v].size(); i++)
                 {
                     Edge_MCF &e = G1[v][i];
                     if (e.cap>0 && dist[e.to]>dist[v]+e.cost)//松弛操作
                     {
                         dist[e.to] = dist[v] + e.cost;
                         prevv[e.to] = v;//更新父结点
                         preve[e.to] = i;//更新父边编号
                         addflow[e.to] = min(addflow[v], e.cap);
                         if(!inq[e.to]){
                            q.push(P(dist[e.to], e.to));
                            inq[e.to] = true;
                         }

                     }
                 }
             }

             //判断是否存在最短路径，若否，返回最大值
             if (dist[superConsumerNetNode] == INF){
                 return INF;
             }

             //求取最短路径最小流量增加量
             //int d = f;
             int d = addflow[superConsumerNetNode];
             /*stack<int> stk;        //存储路径编号栈
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x]){
                 stk.push(x);
                 d = min(d, G1[prevv[x]][preve[x]].cap);//从t出发沿着最短路返回s找可改进量
             }*/


             //更新路径及费用
             f -= d;
             res += d*dist[superConsumerNetNode];//h[t]表示最短距离的同时，也代表了这条最短路上的费用之和，乘以流量d即可得到本次增广所需的费用
             //cout<<f<<" "<<res<<endl;
             //修改网络残量值
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G1[prevv[x]][preve[x]];
                 e.cap -= d;
                 G1[x][e.rev].cap += d;

             }
         }
         return res;
    }
};

#endif
