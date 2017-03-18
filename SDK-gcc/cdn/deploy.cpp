#include "deploy.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "graph.h"
#include "mincostflow.h"

using namespace std;

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    // 创建图
    Graph graph;
    graph.createGraph(topo);

    // 初始最差解：每个消费节点相连的网络节点放置服务器
    vector<int> servers;  //服务器
    vector<int> consumerNetNodes;  //消费节点所连网络节点
    int f = 0;   //流量需求
    for(int i=0;i<graph.consumerNum;i++){
        servers.push_back(graph.consumers[i].netNode);
        consumerNetNodes.push_back(graph.consumers[i].netNode);
        f += graph.consumers[i].flowNeed;
    }

    // 最小费用最大流求解
    MCF mincostflow;
    mincostflow.createMCF(graph);
    vector<int> minCostPath[1000];   //路径
    int m = 1000;
    int flowAll = 0;
    int minCost = 0;
    ostringstream ss;
    int res = mincostflow.multiMinCostFlow(servers,consumerNetNodes,f,minCostPath,m,flowAll,minCost);
    if(res >= 0){
        // 文件写入
        ss<<m<<"\n"<<"\n";
        for(int i=0;i<m;i++){
            for(int j=0;j<minCostPath[i].size();j++){
                ss<<minCostPath[i][j];
                if(j == minCostPath[i].size()-1)
                    ss<<"\n";
                else
                    ss<<" ";
            }
        }
    }else{
        ss<<"NA"<<"\n";     //无解输出NA
    }
    string result = ss.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();



	// 需要输出的内容
	//char * topo_file = (char *)"17\n\n0 8 0 20\n21 8 0 20\n9 11 1 13\n21 22 2 20\n23 22 2 8\n1 3 3 11\n24 3 3 17\n27 3 3 26\n24 3 3 10\n18 17 4 11\n1 19 5 26\n1 16 6 15\n15 13 7 13\n4 5 8 18\n2 25 9 15\n0 7 10 10\n23 24 11 23";

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
