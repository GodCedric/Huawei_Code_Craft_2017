//最小费用流类

#ifndef __MINCOSTFLOW__H__
#define __MINCOSTFLOW__H__

#include <stack>
#include <cstdio>
#include <cstring>
#include "graph.h"
#include "myqueue.h"

#define MAXNODE 1000
#define MAXEDGE 20000
#define MIN(a,b) ((a)<(b)?(a):(b))
#define OPPOSITE(x) (((x)&1)?((x)+1):((x)-1))
#define INFINIT ~0U>>1


using namespace std;

//颜色
enum Color{WHITE,GRAY};

//残存网络边
struct Edge_MCF
{
    int to, cap, flow, cost, rev;//终点，残存容量，单位费用，反向边编号
    Edge_MCF(int t, int c, int f, int cc, int r):to(t), cap(c), flow(f), cost(cc), rev(r){}
};

//最小费用流类
class MCF{
private:
    Graph graph;                    //关联图
    int nodeNum;                    //网络节点数目
    int consumerNum;                //消费节点数目

    vector<Edge_MCF> G[MAXN];       //残存网络邻接表
    int res;
    bool inq[MAXN];                  //节点入队标志位
    int dist[MAXN];                 //最短距离
    int prevv[MAXN];                //最短路中的父结点
    int preve[MAXN];                //最短路中的父边编号
    Queue<int,2000> myq;//自己写的队列

    vector<int> consumerNetNodes;   //消费节点所连网络节点
    int f_all;                      //消费节点容量需求和
    int superServer;//超级源
    int superConsumerNetNode;//超级汇

    Color color[MAXN];

    //基于矩阵的实现方法
    int begin[MAXNODE],eend[MAXEDGE],next[MAXEDGE],c[MAXEDGE],cost[MAXEDGE],d[MAXNODE],cur[MAXNODE];
    bool hash[MAXNODE];
    int Count = 0;
    int N,S,T;
    int wholeflow;


public:
    //构造函数
    MCF(Graph graph){

        nodeNum = graph.nodeNum;
        consumerNum = graph.consumerNum;
        superServer = nodeNum;
        superConsumerNetNode = nodeNum + 1;

        this->graph = graph;

        //按图创建残存网络
        for (int i=0;i<nodeNum;++i)
        {
            for(int j=0;j<graph.G[i].size();++j){
                addEdge(graph.G[i][j].from,graph.G[i][j].to,
                            graph.G[i][j].cap,graph.G[i][j].cost);
            }
        }

        //初始化consumerNetNodes,f
        for(int i=0;i<consumerNum;++i){
            consumerNetNodes.push_back(graph.consumers[i].netNode);
            f_all += graph.consumers[i].flowNeed;
        }

        //加入超级汇点
        for(int i=0;i<consumerNum;++i){
            //超级汇与每个消费结点所连的网络结点建立边：费用为流量需求，容量无穷
            addEdge2(consumerNetNodes[i],superConsumerNetNode,graph.consumers[i].flowNeed,0);
        }

        S = nodeNum;//超级源是倒数第二个
        T = nodeNum+1;//超级汇是最后一个
        N = T + 1;//节点个数

    }

    //给残存网络中添加边（上下行链路及其反向边）
    void addEdge(int from, int to, int cap, int cost)
    {
        int sz1 = G[from].size();
        int sz2 = G[to].size();

        //  上行路及其反向边
        G[from].push_back(Edge_MCF( to, cap, 0, cost, sz2));
        G[to].push_back(Edge_MCF( from, 0, 0, -cost, sz1));
        // 下行路及其反向边
        G[to].push_back(Edge_MCF( from, cap, 0, cost, sz1+1));
        G[from].push_back(Edge_MCF( to, 0, 0, -cost, sz2+1));
    }

    void addEdge(vector<Edge_MCF> G1[], int from,int to,int cap,int cost){
        G1[from].push_back(Edge_MCF( to, cap, 0, cost, G[to].size()));
        G1[to].push_back(Edge_MCF( from, 0, 0, -cost, G[from].size() - 1 ));
    }

    //超级源点，超级汇点添加边
    void addEdge2(int from,int to,int cap,int cost){
        G[from].push_back(Edge_MCF( to, cap, 0, cost, G[to].size()));
        G[to].push_back(Edge_MCF( from, 0, 0, -cost, G[from].size() - 1 ));
    }

    void AddEdge3(int a,int b,int flow, int v){
        Count++; next[Count] = begin[a]; begin[a] = Count; eend[Count] = b; c[Count] = flow; cost[Count] = v;
        Count++; next[Count] = begin[b]; begin[b] = Count; eend[Count] = a; c[Count] = 0; cost[Count] = -v;
    }


    bool spfa(){
        //spfa算法求解最短路径
        for(int i=0;i<nodeNum+2;++i){
           dist[i] = INF;
        }
        dist[superServer] = 0;
        myq.enqueue(superServer);
        inq[superServer] = true;
        while (!myq.is_empty())
        {
        	int v = myq.dequeue();
            inq[v] = false;
            int sz = G[v].size();
            for (int i = 0; i<sz; ++i)
            {
                Edge_MCF &e = G[v][i];
                if (e.cap>e.flow && dist[e.to]>dist[v]+e.cost)//松弛操作
                {
                    dist[e.to] = dist[v] + e.cost;
                    prevv[e.to] = v;//更新父结点
                    preve[e.to] = i;//更新父边编号
                    if(!inq[e.to]){
                       myq.enqueue(e.to);
                       inq[e.to] = true;
                    }
                }
            }
        }

        //判断是否存在最短路径，若否，返回最大值
        if (dist[superConsumerNetNode] == INF){
        	return false;
        }else{
        	return true;
        }
    }

    //多源多汇最小费用流算法
    int multiMinCostFlow(vector<int> &servers,            //服务器位置
                          vector<int> minCostPath[],int& m){   //路径及路径数目
         int serversNum = servers.size();

         //创建残量网络副本
         //vector<Edge_MCF> G1[nodeNum+2];
         //copy(this->G,this->G+nodeNum+2,G1);

         //加入超级源与超级汇
         for(int i=0;i<serversNum;++i){
             //超级源与每个服务器建立边：费用0，容量无穷
             addEdge2(superServer,servers[i],INF,0);
         }


         //最小费用流
         //初始化参数
         int pathCnt = 0;       //路径数目
         int f = f_all;         //流量需求
         //int flowNeed = f;
         res = 0;           //返回值：存在路径解时返回最小费用，否则返回-1
         for(int i=0;i<m;++i){
             minCostPath[i].clear();
         }

         while (f>0)//未满足流量需求时继续寻找
         {

             //spfa算法求解最短路径
             for(int i=0;i<nodeNum+2;++i){
                dist[i] = INF;
             }
             dist[superServer] = 0;
             myq.enqueue(superServer);
             inq[superServer] = true;
             while (!myq.is_empty())
             {
                 int v = myq.dequeue();
                 inq[v] = false;
                 int sz = G[v].size();
                 for (int i = 0; i<sz; ++i)
                 {
                     Edge_MCF &e = G[v][i];
                     if (e.cap>e.flow && dist[e.to]>dist[v]+e.cost)//松弛操作
                     {
                         dist[e.to] = dist[v] + e.cost;
                         prevv[e.to] = v;//更新父结点
                         preve[e.to] = i;//更新父边编号
                         if(!inq[e.to]){
                            myq.enqueue(e.to);
                            inq[e.to] = true;
                         }

                     }
                 }
             }

             //判断是否存在最短路径，若否，返回最大值
             if (dist[superConsumerNetNode] == INF){

                 //直接在原图上操作
                 G[superServer].clear();//先把超级源清空
                 //把新添加的servers的那些边删掉
                 for(int i=0;i<serversNum;++i){
                     G[servers[i]].pop_back();
                 }
                 for(int i=0;i<nodeNum+2;++i){
                     int nnn = G[i].size();
                     for(int j=0;j<nnn;++j){
                         G[i][j].flow = 0;  //把流量复原
                     }
                 }

                 return INFMAX;
             }

             //求取最短路径最小流量增加量
             int d = INF;
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G[prevv[x]][preve[x]];
                 d = min(d,e.cap-e.flow);

             }

             //更新路径及费用
             f -= d;
             res += d*dist[superConsumerNetNode];//h[t]表示最短距离的同时，也代表了这条最短路上的费用之和，乘以流量d即可得到本次增广所需的费用

             //修改网络残量值
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G[prevv[x]][preve[x]];
                 e.flow += d;
                 G[x][e.rev].flow -= d;
             }
         }


         //获取路径：深度优先搜索
         //从超级源出发
         fill(color, color+nodeNum+2, WHITE);
         vector<int> path;//记录路径

         int recordFlow = INF;//记录流量

         for(int i=0;i<G[superServer].size();++i){
            Edge_MCF& e = G[superServer][i];
        	while(e.cap>0 && e.flow>0){
        		int to = e.to;
        		int cap = e.cap;
        		int flow = e.flow;
        		recordFlow = INF;
        		path.clear();
        		DFSvisit(e.to,recordFlow,path,minCostPath,pathCnt);
        		e.flow -= recordFlow;
        	}
         }

         m=pathCnt;

         //直接在原图上操作
         G[superServer].clear();//先把超级源清空
         //把新添加的servers的那些边删掉
         for(int i=0;i<serversNum;++i){
            G[servers[i]].erase(G[servers[i]].end()-1);
         }
         for(int i=0;i<nodeNum+2;++i){
            int nnn = G[i].size();
            for(int j=0;j<nnn;++j){
                G[i][j].flow = 0;  //把流量复原
            }
         }
         return res;
    }

    void DFSvisit(int u, int& recordFlow, vector<int>& path, vector<int> minCostPath[],int &pathCnt){
            //遇到超级汇点停止，输出一条路径
            if(u == superConsumerNetNode){
                path.push_back(superConsumerNetNode);
                minCostPath[pathCnt] = path;//添加路径
                minCostPath[pathCnt].pop_back();
                vector<int>::iterator itr = minCostPath[pathCnt].end();
                int temp = *(itr-1);
                int cons = graph.netToConsumer[temp];   //对应的消费节点
                minCostPath[pathCnt].push_back(cons);
                minCostPath[pathCnt].push_back(recordFlow);

                pathCnt++;
                return;
            }
            path.push_back(u);//路径添加节点
            color[u] = GRAY;

            for(int i=0;i<G[u].size();++i){
                Edge_MCF& e = G[u][i];
                if(e.cap>0 && e.flow>0 && color[e.to]==WHITE){//往流量为正的，正向边，且未被探索过的边搜索
                	recordFlow = min(recordFlow, e.flow);//时刻保存路径上的最小流量
                    DFSvisit(e.to,recordFlow,path,minCostPath,pathCnt);
                    e.flow -= recordFlow;//回溯减去流量
                    color[u] = WHITE;//回溯重置节点颜色
                    return;
                }
            }
        }



    //多源多汇最小费用流算法，用于求fit，即不需要求path
    int multiMinCostFlow2(vector<int> &servers){
        int serversNum = servers.size();

         //创建残量网络副本
         //vector<Edge_MCF> G1[nodeNum+2];
         //copy(this->G,this->G+nodeNum+2,G1);


         //加入超级源与超级汇
         for(int i=0;i<serversNum;++i){
             //超级源与每个服务器建立边：费用0，容量无穷
             addEdge2(superServer,servers[i],INF,0);
         }

         //最小费用流
         //初始化参数
         int f = f_all;         //流量需求
         res = 0;           //返回值：存在路径解时返回最小费用，否则返回-1

         while (f>0)//未满足流量需求时继续寻找
         {
             //spfa算法求解最短路径
             for(int i=0;i<nodeNum+2;++i){
                dist[i] = INF;
             }
             dist[superServer] = 0;
             myq.enqueue(superServer);
             inq[superServer] = true;
             while (!myq.is_empty())
             {
            	 int v = myq.dequeue();

                 inq[v] = false;
                 int sz = G[v].size();
                 for (int i = 0; i<sz; ++i)
                 {
                     Edge_MCF &e = G[v][i];
                     if (e.cap>e.flow && dist[e.to]>dist[v]+e.cost)//松弛操作
                     {
                         dist[e.to] = dist[v] + e.cost;
                         prevv[e.to] = v;//更新父结点
                         preve[e.to] = i;//更新父边编号
                         if(!inq[e.to]){
                            myq.enqueue(e.to);
                            inq[e.to] = true;
                         }
                     }
                 }
             }

             //判断是否存在最短路径，若否，返回最大值
             if (dist[superConsumerNetNode] == INF){

                 //直接在原图上操作
                 G[superServer].clear();//先把超级源清空
                 //把新添加的servers的那些边删掉
                 for(int i=0;i<serversNum;++i){
                     G[servers[i]].pop_back();
                 }
                 for(int i=0;i<nodeNum+2;++i){
                     int nnn = G[i].size();
                     for(int j=0;j<nnn;++j){
                         G[i][j].flow = 0;  //把流量复原
                     }
                 }

                 return INFMAX;
             }

             //求取最短路径最小流量增加量
             int d = INF;
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G[prevv[x]][preve[x]];
                 d = min(d,e.cap-e.flow);

             }

             //更新路径及费用
             f -= d;
             res += d*dist[superConsumerNetNode];//h[t]表示最短距离的同时，也代表了这条最短路上的费用之和，乘以流量d即可得到本次增广所需的费用
             //修改网络残量值
             for (int x = superConsumerNetNode; x != superServer; x = prevv[x])
             {
                 Edge_MCF &e = G[prevv[x]][preve[x]];
                 e.flow += d;
                 G[x][e.rev].flow -= d;

             }
         }

         //直接在原图上操作
         G[superServer].clear();//先把超级源清空
         //把新添加的servers的那些边删掉
         for(int i=0;i<serversNum;++i){
            G[servers[i]].pop_back();
         }
         for(int i=0;i<nodeNum+2;++i){
            int nnn = G[i].size();
            for(int j=0;j<nnn;++j){
                G[i][j].flow = 0;  //把流量复原
            }
         }

         return res;
    }



    int aug(int u, int f){
    	if(u == T){//遇到了汇点，计算代价，返回流量
            return f;
    	}
    	hash[u] = true;
    	for(int now=cur[u];now;now=next[now]){
    		if(c[now] && (!hash[eend[now]]) && (d[u] == d[eend[now]]+cost[now])){
    			if(int tmp = aug(eend[now],MIN(f,c[now]))){
    				c[now] -= tmp;
    				c[OPPOSITE(now)] += tmp;
    				cur[u] = now;
    				return tmp;
    			}
    		}
    	}
        return 0;
    }

    bool modlabel(){

    	int tmp = INFINIT;
        for (int i = 0; i<N; i++){
        	if (hash[i]){
        		for (int now = begin[i]; now; now = next[now]){
        			if (c[now]&&(!hash[eend[now]])){
        				tmp = MIN(tmp,d[eend[now]]+cost[now]-d[i]);
        			}
        		}
        	}
        }

        if (tmp == INFINIT)
            return true;
        for (int i = 0; i<N; i++){
        	if (hash[i]){
        		hash[i] = false;
        	    d[i] += tmp;
            }
        }

        return false;
    }

    //多源多汇最小费用流算法，用于求fit，即不需要求path
    int multiMinCostFlow3(vector<int> &servers){
        //初始化
        Count = 0;
        for(int i=0;i<nodeNum+2;++i){
        	begin[i] = 0;
        	d[i] = 0;
        	cur[i] = 0;
        	//hash[i] = 0;
        }
        for(int i=0;i<nodeNum+2;++i){
        	eend[i] = 0;
        	next[i] = 0;
        	c[i] = 0;
        	cost[i] = 0;
        }

        wholeflow = 0;

        //超级源加到1号位置
        int serversNum = servers.size();
        for(int i=0;i<serversNum;++i){
            //超级源与每个服务器建立边：费用0，容量无穷
        	AddEdge3(S,servers[i],INF,0);
        }

    	//加上网络节点
        for (int i=0;i<nodeNum;++i)
        {
            for(int j=0;j<graph.G[i].size();++j){
            	//上行
            	AddEdge3(graph.G[i][j].from,graph.G[i][j].to,
                            graph.G[i][j].cap,graph.G[i][j].cost);
            	//下行
            	AddEdge3(graph.G[i][j].to,graph.G[i][j].from,
                            graph.G[i][j].cap,graph.G[i][j].cost);
            }
        }

        //加上超级汇
        for(int i=0;i<consumerNum;++i){
            //超级汇与每个消费结点所连的网络结点建立边：容量为流量需求，费用为0
        	AddEdge3(consumerNetNodes[i],T,graph.consumers[i].flowNeed,0);
        }

        res = 0;
        int tmp;
        while(true){
        	for(int i=0;i<N;++i){
        		cur[i] = begin[i];
        	}
        	while(tmp = aug(S,INFINIT)){
        		res += tmp*d[S];
        		wholeflow += tmp;
        		memset(hash,0,sizeof(hash));
        	}
        	if(modlabel())
        		break;
        }

        if(wholeflow != f_all)
            return INFMAX;
        else
            return res;
    }

};

#endif
