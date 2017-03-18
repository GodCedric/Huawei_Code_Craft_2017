#ifndef MINCOSTFLOW_H
#define MINCOSTFLOW_H

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include "graph.h"

using namespace std;

typedef pair<int, int>P;//first保存最短距离，second保存顶点的编号

struct Edge_MCF
{
    int to, cap, cost, rev;//终点，容量（指残量网络中的），费用 , 反向边编号
    Edge_MCF(int t, int c, int cc, int rev) :to(t), cap(c), cost(cc), rev(rev){}
};
class MCF{//最小费用流（指定需求流量）
public:
    int nodeNum;//顶点数
    int edgeNum;//边数
    int K_nodeNum;//扩展顶点数
    int tempnode;//中间点


    vector<Edge_MCF> G[maxn*2];//图的邻接表
    int h[maxn];//顶点的势
    int inq[maxn*2];//是否在队列
    int dist[maxn*2];//最短距离
    int prevv[maxn*2];//最短路中的父结点
    int preve[maxn*2];//最短路中的父边
    Graph graph;


    void init(int n,int m){
        K_nodeNum = n + m;
        for(int i=0;i<K_nodeNum;i++){
            G[i].clear();
        }
    }

    void addEdge(int from, int to, int cap, int cost)
    {
        //加入上行链路及其反向边
        G[from].push_back(Edge_MCF( to, cap, cost, G[to].size()));
        G[to].push_back(Edge_MCF( from, 0, -cost, G[from].size()-1));
        //下行链路增加一中间点，分裂成两条边
        tempnode++;
        G[to].push_back(Edge_MCF( tempnode, cap, cost, G[tempnode].size()));
        G[tempnode].push_back(Edge_MCF( to, 0, -cost, G[to].size()-1));
        G[tempnode].push_back(Edge_MCF( from, cap, cost, G[from].size()));
        G[from].push_back(Edge_MCF( tempnode, 0, -cost, G[tempnode].size()-1));
    }

    void addEdge(vector<Edge_MCF> G1[],int from,int to,int cap,int cost){

        //给超级源超级汇加边是单向边，不存在反向平行边
        G1[from].push_back(Edge_MCF( to, cap, cost, G1[to].size()));
        G1[to].push_back(Edge_MCF( from, 0, -cost, G1[from].size()-1));
    }


    //获取MCF(转存Graph到MCF的G)
    void createMCF(const Graph &graph){
        this->graph = graph;
        nodeNum = graph.nodeNum;
        edgeNum = graph.edgeNum;
        init(nodeNum,edgeNum);
        tempnode = graph.nodeNum - 1;
        for (int i = 0; i<graph.nodeNum; i++)
        {
            for(int j=0;j<graph.G[i].size();j++){
                addEdge(graph.G[i][j].from,graph.G[i][j].to,
                            graph.G[i][j].cap,graph.G[i][j].cost);

            }
        }
    }



    //多源多汇最小费用流算法
    int multiMinCostFlow(const vector<int> &servers,const vector<int> &consumerNetNodes,
                          int f,
                          //string& result;
                          vector<int> minCostPath[],int &m,
                          int &flowAll,int &minCost){
         vector<Edge_MCF> G1[maxn*2];
         //vector<int> minCostPath[maxn];
         copy(this->G,this->G+maxn*2,G1);

         ////建立超级源超级汇，并添加相应的边
         int superServer = nodeNum + edgeNum;//超级源
         for(int i=0;i<servers.size();i++){
             //超级源与每个服务器建立边：费用0，容量无穷
             addEdge(G1,superServer,servers[i],INF,0);
         }
         int superConsumerNetNode = superServer + 1;//超级汇
         for(int i=0;i<consumerNetNodes.size();i++){
             //超级汇与每个消费结点所连的网络结点建立边：费用0，容量为消费节点要求
             addEdge(G1,consumerNetNodes[i],superConsumerNetNode,graph.consumers[i].flowNeed,0);
         }

         //单次调用超级源到超级汇的最小费用流
         stack<int> stk;
         int pathCnt=0;
         int flowNeed=f;
         int res = 0;
         flowAll=0;
         minCost=0;
         for(int i=0;i<m;i++){
             minCostPath[i].clear();
         }
         //fill(h, h + nodeNum+2, 0);
         while (f>0)//f>0时还需要继续增广
         {
             priority_queue<P, vector<P>, greater<P> >q;
             fill(dist, dist+K_nodeNum+2, INF);//距离初始化为INF
             fill(inq, inq+K_nodeNum+2, 0);//入队标志置0
             dist[superServer] = 0;
             inq[superServer] = 1;
             q.push(P(0, superServer));
             while (!q.empty())
             {
                 P p = q.top();
                 q.pop();
                 int v = p.second;
                 inq[v]--;
                 //if (dist[v]<p.first)continue;//p.first是v入队列时候的值，dist[v]是目前的值，如果目前的更优，扔掉旧值
                 for (int i = 0; i<G1[v].size(); i++)
                 {
                     Edge_MCF &e = G1[v][i];
                     if (e.cap>0 && dist[e.to] > dist[v]+e.cost)//松弛操作
                     {
                         dist[e.to] = dist[v] + e.cost;
                         prevv[e.to] = v;//更新父结点
                         preve[e.to] = i;//更新父边编号
                         if(!inq[e.to]){
                            inq[e.to]++;
                            q.push(P(dist[e.to], e.to));
                         }
                     }
                 }
             }

             //cout<<f<<"  "<<dist[superConsumerNetNode];
             if (dist[superConsumerNetNode] == INF){//如果dist[t]还是初始时候的INF，那么说明s-t不连通，不能再增广了
                 minCost=res;

                 return -1;
             }
             //for (int j = 0; j<nodeNum+2; j++)//更新h
                 //h[j] += dist[j];
             int d = f;
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x]){
                 stk.push(x);
                 d = min(d, G1[prevv[x]][preve[x]].cap);//从t出发沿着最短路返回s找可改进量
             }
             int node;
             while(!stk.empty()){
                 node=stk.top();
                 stk.pop();
                 minCostPath[pathCnt].push_back(node);
                 cout<<node<<' ';
             }

             //路径处理
             vector<int>::iterator endx = minCostPath[pathCnt].end();
             minCostPath[pathCnt].erase(endx-1);//去掉超级汇点
             node = minCostPath[pathCnt][minCostPath[pathCnt].size()-1];
             minCostPath[pathCnt].push_back(this->graph.netToConsumer[node]);//加入消费结点


             if(d<=flowNeed){
                 minCostPath[pathCnt].push_back(d);//实际流量
                 flowAll+=d;
                 cout<<d<<endl;
             }
             else{
                 minCostPath[pathCnt].push_back(flowNeed);
                 flowAll+=d;
                 cout<<d<<endl;
             }
             pathCnt++;
             f -= d;
             //res += d*h[superConsumerNetNode];//h[t]表示最短距离的同时，也代表了这条最短路上的费用之和，乘以流量d即可得到本次增广所需的费用
             cout<<f<<" "<<res<<endl;
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G1[prevv[x]][preve[x]];
                 e.cap -= d;//修改残量值
                 //反向边增加
                 e = G1[x][e.rev];
                 e.cap += d;
             }

         }
         m=pathCnt;
         return res;
    }
};

#endif // MINCOSTFLOW_H
