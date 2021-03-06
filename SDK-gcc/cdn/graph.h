//图类

#ifndef __GRAPH__H__
#define __GRAPH__H__

#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <map>

using namespace  std;

//全局变量
#define MAXN 1000			//最大网络节点数
#define INF 100000	    	//数值上限
#define INFMAX 1000000      //代价上限
#define MAXPATH 50000		//网络路径上限
#define MAXPATHNODE 1000	//单条路径节点数上限
#define ENDTIME 88.5

//边
struct Edge{
	int from, to, cap, cost;   //起始，终止，流量，单价
	Edge(){}
	Edge(int u, int v, int c, int w):from(u), to(v), cap(c), cost(w){}
};

//消费节点
struct Consumer{
	int no, netNode, flowNeed;      //编号，所连网络节点编号，流量需求
};

//图
class Graph{
public:
	//图参数
	int nodeNum;		//节点数
	int edgeNum;		//边数
	int consumerNum;	//消费节点数
	int serverCost;		//服务器单价

	//spfa成员
	bool inq[MAXN];                  //节点入队标志位
    int dist[MAXN];                 //最短距离
    int prevv[MAXN];                //最短路中的父结点
    int preve[MAXN];                //最短路中的父边编号
    int addflow[MAXN];              //可增加量
    queue<int> q;                   //队列

	//图成员
	vector<Edge> G[MAXN];			//网络结点邻接表
	//vector< vector<Edge> > G(1000);  //
    vector<Consumer> consumers;		//消费节点数组
    map<int,int> netToConsumer;		//网络节点消费节点映射

	//成员初始化
	void init(int n){
        //节点邻接表初始化
		for(int i=0;i<n;i++){
            G[i].clear();
        }
		//消费节点数组初始化
        consumers.clear();
    }

	//根据输入创建图
	void createGraph(char * topo[MAX_EDGE_NUM]){
        //获取网络参数
		string line = topo[0];
        stringstream ss(line);
        ss>>nodeNum>>edgeNum>>consumerNum;
		serverCost = strtol(topo[2],NULL,10);

		//初始化
        //init(nodeNum);
		////初始化边
        for(int i=4;i<edgeNum+4;i++){
            Edge e;
            line=topo[i];
			ss.clear();ss.str("");ss.str(line);
            ss>>e.from>>e.to>>e.cap>>e.cost;
            G[e.from].push_back(e);
        }
		////初始化消费节点
        for(int i=5+edgeNum;i<consumerNum+5+edgeNum;i++){
            Consumer consumer;
            line=topo[i];
            ss.clear();ss.str("");ss.str(line);
            ss>>consumer.no>>consumer.netNode>>consumer.flowNeed;
            netToConsumer[consumer.netNode]=consumer.no;
            consumers.push_back(consumer);
        }
    }

    //spfa
    void spfa(int start, int& index1, int& index2, int& index3){
        fill(dist, dist+nodeNum, INF);//距离初始化为INF
        fill(inq, inq+nodeNum, false);//入队标志初始化
        dist[start] = 0;
        addflow[start] = INF;
        q.push(start);
        inq[start] = true;
        while (!q.empty())
        {
            int v = q.front();
            q.pop();
            inq[v] = false;
            for (int i = 0; i<G[v].size(); i++)
            {
                Edge &e = G[v][i];
                if (e.cap>0 && dist[e.to]>dist[v]+e.cost)//松弛操作
                {
                    dist[e.to] = dist[v] + e.cost;
                    addflow[e.to] = min(addflow[v], e.cap);
                    if(!inq[e.to]){
                        q.push(e.to);
                        inq[e.to] = true;
                    }

                }
            }
        }

        index1 = 0;
        index2 = 0;
        index3 = 0;
        int temp = 0;
        int maxValue = 0;

        //流量越大越好，费用越低越好
        for(int i=0;i<nodeNum;i++){
            if(i == start)
                continue;

            temp = addflow[i]*64/dist[i];

            if(temp > maxValue){
                maxValue = temp;
                index3 = index2;
                index2 = index1;
                index1 = i;
            }
        }
    }

    //spfa2
    int spfa2(int start, vector<int>& singleServers, int f){
        singleServers.clear();

        fill(dist, dist+nodeNum, INF);//距离初始化为INF
        fill(inq, inq+nodeNum, false);//入队标志初始化
        dist[start] = 0;
        addflow[start] = INF;
        q.push(start);
        inq[start] = true;
        while (!q.empty())
        {
            int v = q.front();
            q.pop();
            inq[v] = false;
            for (int i = 0; i<G[v].size(); i++)
            {
                Edge &e = G[v][i];
                if (e.cap>0 && dist[e.to]>dist[v]+e.cost)//松弛操作
                {
                    dist[e.to] = dist[v] + e.cost;
                    addflow[e.to] = min(addflow[v], e.cap);
                    if(!inq[e.to]){
                        q.push(e.to);
                        inq[e.to] = true;
                    }

                }
            }
        }

        vector<pair<int,int> > tempdist(nodeNum, {0,0});
        for(int i=0;i<nodeNum;i++){
            tempdist[i].first = dist[i];
            tempdist[i].second = i;
        }
        sort(tempdist.begin(),tempdist.end());


        //计算满足消费节点流量需求的最小cost服务器部署方案
        int cnt = 1;
        int cost = 0;
        while(f>0){
            int index = tempdist[cnt].second;
            cost += tempdist[cnt].first * addflow[index] + serverCost;   //花费乘以流量加服务器
            f -=  addflow[index];
            singleServers.push_back(index);
            cnt++;
        }
        return cost;
    }
};

#endif
