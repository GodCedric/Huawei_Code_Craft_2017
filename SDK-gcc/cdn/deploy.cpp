#include "deploy.h"
//#include <stdio.h>
//#include <iostream>
//#include <sstream>

#include "gettime.h"
#include "graph.h"
#include "mincostflow.h"
#include "geneticalgorithm.h"
#include "analyzegraph.h"

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
    string result;

    // 创建图
    Graph graph;
    graph.createGraph(topo);

    //图分析，得到网络节点优先概率
    vector<double> probability(graph.nodeNum,0);
    analyzegraph(graph, probability);

    //创建最小费用最大流模型
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow;
    mincostflow.createMCF(graph);

    //////////GA搜索最优解
    //GA参数
    int generation = 10000;                     //设置迭代次数
    int geneBit = graph.nodeNum;                //基因编码位数
    int maxServers = graph.consumerNum;         //服务器最大配置数目
    int chormNum = 100;                         //种群内染色体数量,先定个100条吧
    const double crossoverRate = 0.8;                 //交叉概率
    const double mulationRate = 0.1;                  //突变概率

    //GA成员
    vector<Chorm> population(100);              //种群
    srand((unsigned)time(NULL));                //随机数种子
    vector<pair<int,int> > fitAll(100,{0,0});          //适应度,first为适应度，second为对应坐标

    //初始最差解：每个消费节点相连的网络节点放个服务器
    ostringstream stream;
    stream<<graph.consumerNum<<"\n"<<"\n";
    for(int i=0;i<graph.consumerNum;i++){
        stream<<graph.consumers[i].netNode<<" "<<i<<" "<<graph.consumers[i].flowNeed<<"\n";
        population[0].gene[graph.consumers[i].netNode] = true;  //把最差解先放进去
    }
    minCost = graph.consumerNum*graph.serverCost;       //以最差解费用初始化最小费用
    result = stream.str();
    //cout<<minCost<<endl;
    //cout<<endl;
    int tempMinCost = minCost;
    int cntMinCost = 0;
    //cout<<minCost<<endl;

    //初始化种群
    for(int i=1;i<chormNum;i++){
        generateChorm(population[i], probability, chormNum, geneBit, maxServers);
    }

    //GA迭代
    while(generation--){

        //接近90s时停止迭代
        timelimit = gettime();
        if(timelimit > 85)
            break;

        //initChorm(probability, population, chormNum, geneBit, maxServers);

        //以最小费用流算法为适度函数，求各染色体适应度
        fitness(population, mincostflow, geneBit, graph.serverCost, fitAll);
        /*for(int i=0;i<chormNum;i++){
            for(int j=0;j<geneBit;j++){
                cout<<population[i].gene[j];
            }
            cout<<" "<<population[i].fit;
            cout<<endl;
        }
        return;*/


        //染色体选择
        int cntValidChorm = 0;
        vector<Chorm> new_population(100);          //更新的种群
        chormSelection(population, new_population, fitAll, cntValidChorm);

        //更新当前最优解
        if(fitAll[0].first < minCost){
            //cout<<"Hello!"<<"  "<<minCost<<endl;
            decode(population[fitAll[0].second], geneBit, servers);
            res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
            cost = res+graph.serverCost*servers.size();
            if(res!=INF && cost<minCost){
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
            minCost = cost;
        }

        //交叉
        crossover(crossoverRate, population, new_population, cntValidChorm, probability, chormNum, geneBit, maxServers);

        //变异
        mutation(mulationRate, population, chormNum, geneBit);

        //收敛后退出
        //cout<<cntMinCost<<endl;
        if(cntValidChorm>70 && tempMinCost==minCost){
            cntMinCost++;
        }else{
            cntMinCost = 0;
        }
        if(cntValidChorm>70 && cntMinCost>1000){//如果1000次迭代最小代价仍然没有改变，则认为收敛，跳出迭代
            break;
        }
        tempMinCost = minCost;
        //break;

    }


    //string result = stream.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
