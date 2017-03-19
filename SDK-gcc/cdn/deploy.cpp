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
    //全局结果
    vector<int> servers;                //服务器位置
    int minCost;                        //当前最小费用
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）

    // 创建图
    Graph graph;
    graph.createGraph(topo);

    //初始最差解：每个消费节点相连的网络节点放个服务器


    //测试用例
    vector<int> servers2;
    servers2.push_back(0);
    servers2.push_back(1);
    servers2.push_back(24);
    /*servers2.push_back(37);
    servers2.push_back(13);
    servers2.push_back(15);
    servers2.push_back(38);*/


    // 最小费用最大流求解
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow;
    mincostflow.createMCF(graph);
    res = mincostflow.multiMinCostFlow(servers2,minCostPath,m);
    cost = res+graph.serverCost*servers2.size();
    cout<<cost<<endl;

    ostringstream ss;
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
        ss<<"\n"<<cost<<"\n";
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
