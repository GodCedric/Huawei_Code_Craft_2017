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
    string result;                      //文件输出结果

    // 创建图
    Graph graph;
    graph.createGraph(topo);
    //cout<<graph.G.size()<<endl;
    int nodeNum = graph.nodeNum;
    int consumerNum = graph.consumerNum;
    int serverCost = graph.serverCost;

    //图分析，得到网络节点优先概率
    vector<double> probability(graph.nodeNum,0);
    analyzegraph(graph, probability);

    //创建最小费用最大流模型
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow(nodeNum,graph);
    //mincostflow.createMCF(graph);

    //////////GA搜索最优解
    //GA参数
    int generation = 10000;                     //设置迭代次数
    int geneBit = nodeNum;                //基因编码位数
    int maxServers = consumerNum;         //服务器最大配置数目
    int chormNum = 50;                         //种群内染色体数量,先定个100条吧
    const double crossoverRate = 0.7;                 //交叉概率
    const double mulationRate = 0.15;                  //突变概率

    //GA成员
    Chorm defaultChorm(0);
    vector<Chorm> population(50,defaultChorm);   //种群
    vector<Chorm> new_population(50,defaultChorm);          //更新的种群
    srand((unsigned)time(NULL));                //随机数种子
    vector<pair<int,int> > fitAll(50,{0,0});          //适应度,first为适应度，second为对应坐标

    //初始最差解：每个消费节点相连的网络节点放个服务器
    ostringstream stream;
    stream<<graph.consumerNum<<"\n"<<"\n";
    for(int i=0;i<graph.consumerNum;i++){
        stream<<graph.consumers[i].netNode<<" "<<i<<" "<<graph.consumers[i].flowNeed<<"\n";
        //初始种群设置
        population[0].gene[graph.consumers[i].netNode] = true;  //把最差解先放进去
    }
    minCost = graph.consumerNum*graph.serverCost;       //以最差解费用初始化最小费用
    int worstCost = minCost;
    result = stream.str();
    //cout<<minCost<<endl;
    //cout<<endl;
    int tempMinCost = minCost;
    int cntMinCost = 0;
    //cout<<minCost<<endl;

    //初始化种群
    //从消费节点向内推一个节点
    int initChormNum = 1;
    for(int i=0;i<graph.consumerNum;i++){
        int netNum = graph.consumers[i].netNode;
        int nn = graph.G[netNum].size();
        for(int j=0;j<nn;j++){
            population[initChormNum] = population[0];
            population[initChormNum].gene[netNum] = false;
            population[initChormNum].gene[graph.G[netNum][j].to] = true;
            initChormNum++;
        }
        if(initChormNum >= chormNum/1.5)
            break;
    }
    //cout<<initChormNum<<endl;
    for(int i=initChormNum;i<chormNum;i++){
        generateChorm(population[i], probability, chormNum, geneBit, maxServers);
    }

    //GA迭代
    int cntChanged = 0;         //最小代价改变次数
    int nProtect = 0;           //染色体保护数目
    bool breakflag = false;
    int cntValidChorm = 0;
    while(generation--){
        //cout<<generation<<endl;

        //接近90s时停止迭代
        //timelimit = gettime();
        //if(timelimit > 88)
            //break;


        //以最小费用流算法为适度函数，求各染色体适应度
        for(int i=nProtect;i<population.size();i++){
            decode(population[i], nodeNum, servers);//获取服务器部署
            int fit = mincostflow.multiMinCostFlow2(servers);

            if(gettime() > 87.5){
                breakflag = true;
                break;
            }

            fit += serverCost*servers.size();
            fitAll[i].first = fit;                  //存储适应度
            fitAll[i].second = i;                   //存储对应染色体坐标
            population[i].fit = fit;                //得到适应度
            //cout<<fit<<endl;
        }




        //fitness(population, mincostflow, servers, geneBit, graph.serverCost, fitAll, nProtect, breakflag);
        if(breakflag)
            break;

        //染色体选择
        cntValidChorm = 0;
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
				//const char* topo_file = result.c_str();
				//write_result(topo_file, filename);
                //cout<<result<<endl;
            }
            minCost = cost;
            cntChanged++;
            //cout<<minCost<<endl;
        }
        //cout<<cntValidChorm<<endl;

        //交叉
        crossover(crossoverRate, population, new_population, cntValidChorm, probability, chormNum, geneBit, maxServers, nProtect);

        //变异
        mutation(mulationRate, population, chormNum, geneBit);

        //收敛后退出
        //cout<<cntMinCost<<endl;
        if(cntValidChorm>80 && tempMinCost == minCost){
            cntMinCost++;
        }else{
            cntMinCost = 0;
        }
        if((cntChanged>15||cntValidChorm>80) && cntMinCost>50){//如果50次迭代最小代价仍然没有改变，则认为收敛，跳出迭代
            break;
        }
        if(cntMinCost > 600)
            break;
        tempMinCost = minCost;
        //cout<<minCost<<endl;
        //break;

    }
    //cout<<generation<<endl;
    //cout<<10000 - generation<<endl;

    //string result = stream.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
