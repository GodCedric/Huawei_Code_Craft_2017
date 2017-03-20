#include "deploy.h"
//#include <stdio.h>
//#include <iostream>
//#include <sstream>

#include "gettime.h"
#include "graph.h"
#include "mincostflow.h"
#include "xjbs.h"

using namespace std;

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    //程序计时
    double timelimit;
    timelimit = gettime();

    //全局结果
    vector<int> servers;                //服务器位置
    int minCost;                        //当前最小费用
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）

    // 创建图
    Graph graph;
    graph.createGraph(topo);

    //初始最差解：每个消费节点相连的网络节点放个服务器
    ostringstream stream;
    stream<<graph.consumerNum<<"\n"<<"\n";
    for(int i=0;i<graph.consumerNum;i++){
        stream<<graph.consumers[i].netNode<<" "<<i<<" "<<graph.consumers[i].flowNeed<<"\n";
    }
    minCost = graph.consumerNum*graph.serverCost;       //以最差解费用初始化最小费用
    cout<<minCost<<endl;

    //测试用例
    vector<int> servers2;
    servers2.push_back(0);
    servers2.push_back(1);
    servers2.push_back(24);
    servers2.push_back(3);
    //servers2.push_back(13);
    //servers2.push_back(15);
    //servers2.push_back(38);


    //创建最小费用最大流模型
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow;
    mincostflow.createMCF(graph);

    //搜索最优解
    bool flag = true;   //是否退出迭代标志
    int xx = 2;
    while(xx--){
        //迭代计时
        timelimit = gettime();
        cout<<"timelimit: "<<timelimit<<endl;

        if(xx == 0){
            servers2.pop_back();
        }
        res = mincostflow.multiMinCostFlow(servers2,minCostPath,m);
        cost = res+graph.serverCost*servers2.size();
        cout<<cost<<endl;
        //判断该解费用是否小于当前最小费用，如果是则写入当前解
        if(res>0 && cost<minCost){
            minCost = cost;
            stream.clear();
            stream.str("");
            stream<<m<<"\n"<<"\n";
            for(int i=0;i<m;i++){
                for(int j=0;j<minCostPath[i].size();j++){
                    stream<<minCostPath[i][j];
                    if(j == minCostPath[i].size()-1)
                        stream<<"\n";
                    else
                        stream<<" ";
                }
            }
            stream<<"\n"<<cost<<"\n";
        }
    }

    string result = stream.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
