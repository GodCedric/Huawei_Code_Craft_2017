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
//#define MAXEDGE 20          //每个节点链路数量上限
#define INF 100000	    	//数值上限
#define MAXPATH 50000		//网络路径上限
#define MAXPATHNODE 1000	//单条路径节点数上限
//#define MAXSERVER 100

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
        init(nodeNum);
		////初始化边
        for(int i=4;i<edgeNum+4;i++){
            Edge e;
            line=topo[i];
			ss.clear();
			ss.str("");
            ss.str(line);
            ss>>e.from>>e.to>>e.cap>>e.cost;
            G[e.from].push_back(e);
        }
		////初始化消费节点
        for(int i=5+edgeNum;i<consumerNum+5+edgeNum;i++){
            Consumer consumer;
            line=topo[i];
            ss.clear();
			ss.str("");
            ss.str(line);
            ss>>consumer.no>>consumer.netNode>>consumer.flowNeed;
            netToConsumer[consumer.netNode]=consumer.no;
            consumers.push_back(consumer);
        }
    }
};

#endif
