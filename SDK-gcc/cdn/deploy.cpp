#include "deploy.h"
//#include <stdio.h>
//#include <iostream>
//#include <sstream>

#include "gettime.h"
#include "graph.h"
#include "mincostflow.h"
#include "geneticalgorithm.h"

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
    //cout<<minCost<<endl;

    //测试用例
    /*vector<int> servers2;
    servers2.push_back(0);
    servers2.push_back(1);
    servers2.push_back(24);
    servers2.push_back(3);*/
    //servers2.push_back(13);
    //servers2.push_back(15);
    //servers2.push_back(38);


    //创建最小费用最大流模型
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow;
    mincostflow.createMCF(graph);

    //////////GA搜索最优解
    //GA参数
    int generation = 10000;                     //设置迭代次数
    int geneBit = graph.nodeNum;                //基因编码位数
    int maxServers = graph.consumerNum;           //服务器最大配置数目


    //初始化种群
    int chormNum = 100;                         //种群内染色体数量,先定个100条吧
    vector<Chorm> population(100);                   //种群
    srand((unsigned)time(NULL));                //随机数种子
    vector<int> fitAll(100,-1);                 //适应度


    //GA迭代
    string result;
    while(generation--){

        //接近90s时停止迭代
        timelimit = gettime();
        if(timelimit > 85)
            break;

        // 随机部署服务器
        for(int i=0;i<chormNum;i++){
            //服务器清空置零
            for(int m=0;m<geneBit;m++){
                population[i].gene[m] = false;
            }

            int numservers = rand() % maxServers;   //随机生成服务器数量，最多maxServers个
            //随机分布服务器
            for(int j=0;j<numservers;j++){
                int index = rand() % geneBit;
                population[i].gene[index] = true;
            }
            /*for(int m=0;m<geneBit;m++){
                cout<<population[i].gene[m];
            }
            cout<<endl;*/
        }

        //以最小费用流算法为适度函数，求各染色体适应度
        fitness(population, mincostflow, geneBit, fitAll);

        //先来个xjbs
        int Minindex = -1;
        int temp = INF;
        for(int i=0;i<100;i++){
            if(fitAll[i]>0 && fitAll[i]<temp){
                Minindex = i;
                temp = fitAll[i];
            }
        }
        decode(population[Minindex], geneBit, servers);
        res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
        //cout<<m<<endl;
        //for(int i=0;i<minCostPath[0].size();i++)
            //cout<<minCostPath[0][i]<<" ";
        //cout<<endl;
        cost = res+graph.serverCost*servers.size();
        //cout<<temp<<"  "<<res<<endl;
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
            //stream<<"\n"<<cost<<"\n";
            result = stream.str();
            //cout<<result<<endl;
        }
        //轮盘赌选择

    }


    //string result = stream.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
