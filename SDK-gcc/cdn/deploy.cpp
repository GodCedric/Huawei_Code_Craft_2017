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
    bool excellentGene[MAXN];       //优秀基因
    bool goodGene[MAXN];            //良好基因
    bool mediumGene[MAXN];          //中等基因
    vector<double> probability(graph.nodeNum,0);
    analyzegraph(graph, probability, excellentGene, goodGene, mediumGene);

    //创建最小费用最大流模型
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow(graph);
    //mincostflow.createMCF(graph);

    //////////GA搜索最优解
    //GA参数
    int generation = 10000;                     //设置迭代次数
    int geneBit = nodeNum;                //基因编码位数
    int maxServers = consumerNum;         //服务器最大配置数目
    int chormNum = 100;                         //种群内染色体数量,先定个100条吧
    const double crossoverRate = 0.7;                 //交叉概率
    const double mulationRate = 0.15;                  //突变概率

    //GA成员
    Chorm defaultChorm(0);
    vector<Chorm> population(100,defaultChorm);   //种群
    vector<Chorm> new_population(100,defaultChorm);          //更新的种群
    srand((unsigned)time(NULL));                //随机数种子
    vector<pair<int,int> > fitAll(100,{0,0});          //适应度,first为适应度，second为对应坐标

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
    cout<<worstCost<<endl;
    cout<<endl;
    result = stream.str();
    //cout<<minCost<<endl;
    //cout<<endl;
    int tempMinCost = minCost;
    int cntMinCost = 0;
    //cout<<minCost<<endl;

    //初始化种群
    //将优良中基因与初始最差基因交配产生一批基因
    int lengthGene = geneBit / 10;      //1/10的基因片段
    int initChormNum = 16;
    for(int i=0;i<initChormNum;i++){
        int start = rand() % (geneBit - lengthGene);
        population[i*3] = population[0];
        population[i*3+1] = population[0];
        population[i*3+2] = population[0];
        for(int j=0;j<lengthGene;j++){
            //优等基因交换
            swap(population[i*3].gene[j],excellentGene[j]);
            //良好基因交换
            swap(population[i*3+1].gene[j],goodGene[j]);
            //中等基因交换
            swap(population[i*3+2].gene[j],mediumGene[j]);
        }
    }
    /*for(int i=0;i<graph.consumerNum;i++){
        int netNum = graph.consumers[i].netNode;
        int nn = graph.G[netNum].size();
        for(int j=0;j<nn;j++){
            population[initChormNum] = population[0];
            population[initChormNum].gene[netNum] = false;
            population[initChormNum].gene[graph.G[netNum][j].to] = true;
            initChormNum++;
        }
        if(initChormNum >= chormNum*2/3)
            break;
    }*/
    //cout<<initChormNum<<endl;
    //剩下的随机产生
    for(int i=initChormNum*3+1;i<chormNum;i++){
        generateChorm(population[i], probability, chormNum, geneBit, maxServers);
    }

    //GA迭代
    int cntChanged = 0;         //最小代价改变次数
    int nProtect = 0;           //染色体保护数目
    bool breakflag = false;
    int cntValidChorm = 0;
    //int cntMCF = 0;
    while(generation--){
        //cout<<generation<<endl;

        //以最小费用流算法为适度函数，求各染色体适应度
        fitness(population, mincostflow, servers, geneBit, graph.serverCost, fitAll, nProtect, breakflag);
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
        if(cntValidChorm>50 && tempMinCost == minCost){
            cntMinCost++;
        }else{
            cntMinCost = 0;
        }
        if((cntChanged>15||cntValidChorm>50) && cntMinCost>25){//如果50次迭代最小代价仍然没有改变，则认为收敛，跳出迭代
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
    //cout<<cntMCF<<endl;

    //string result = stream.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
