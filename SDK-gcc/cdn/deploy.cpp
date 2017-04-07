#include "deploy.h"
//#include <stdio.h>
//#include <iostream>
//#include <sstream>

#include "gettime.h"
#include "graph.h"
#include "mincostflow.h"
#include "geneticalgorithm.h"
#include "analyzegraph.h"
#include "getresult.h"

using namespace std;

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{

    //程序计时
    double timelimit;
    timelimit = gettime();

    //全局结果
    vector<int> servers;                //服务器位置
    vector<int> result_servers;
    vector<int> optimal_servers;
    int minCost;                        //当前最小费用
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）
    string result;                      //文件输出结果

    // 创建图
    Graph graph;
    graph.createGraph(topo);
    int nodeNum = graph.nodeNum;
    int consumerNum = graph.consumerNum;
    int serverCost = graph.serverCost;

    //根据图规模选择策略
    int caseflag;
    if(nodeNum > 600){
        caseflag = 2;//高级图耗掉时间
    }else if(nodeNum < 200){
        caseflag = 0;//初中级图判断收敛，提前退出
    }else{
        caseflag = 1;
    }

    //图分析，得到网络节点优先概率
    bool excellentGene[MAXN];       //优秀基因
    bool goodGene[MAXN];            //良好基因
    bool mediumGene[MAXN];          //中等基因
    vector<double> probability(nodeNum,0);
    analyzegraph(graph, probability, excellentGene, goodGene, mediumGene);

    //创建最小费用最大流模型
    int res;            //最小费用结果
    int cost;           //最小费用
    MCF mincostflow(graph);

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
    set<int> firstLevel;
    map<int,vector<int> > secondLevel;
    ostringstream stream;
    stream<<consumerNum<<"\n"<<"\n";
    for(int i=0;i<consumerNum;i++){
        stream<<graph.consumers[i].netNode<<" "<<i<<" "<<graph.consumers[i].flowNeed<<"\n";
        //初始种群设置
        population[0].gene[graph.consumers[i].netNode] = true;  //把最差解先放进去
        firstLevel.insert(graph.consumers[i].netNode);
    }
    minCost = consumerNum*serverCost;       //以最差解费用初始化最小费用
    int worstCost = minCost;
    cout<<worstCost<<endl;
    cout<<endl;
    result = stream.str();


    bool breakflag = false;
    Chorm localOpt = population[0];
    Chorm localIni = localOpt;
    Chorm localIni1 = localOpt;
    Chorm localIni2 = localOpt;
    int localCost = worstCost;

    int nn;

    //第一次缩减代价，第一层服务器有相连的可能降低代价
    if(caseflag != 0){
        for(int i=0;i<consumerNum;++i){
            int netNum = graph.consumers[i].netNode;
            if(firstLevel.find(netNum) == firstLevel.end())
                continue;
            int nn = graph.G[netNum].size();
            int fit1;
            int fit2;
            for(int j=0;j<nn;++j){

                //时间控制
                if(gettime() > ENDTIME){
                    breakflag = true;
                    break;
                }

                int second = graph.G[netNum][j].to;

                if(firstLevel.find(second) != firstLevel.end()){
                    localIni = localOpt;
                    localOpt.gene[netNum] = false;
                    decode(localOpt, nodeNum, servers);//获取服务器部署
                    fit1 = mincostflow.multiMinCostFlow3(servers);
                    fit1 += serverCost*servers.size();
                    localIni1 = localOpt;

                    localOpt = localIni;
                    localOpt.gene[second] = false;
                    decode(localOpt, nodeNum, servers);//获取服务器部署
                    fit2 = mincostflow.multiMinCostFlow3(servers);
                    fit2 += serverCost*servers.size();
                    localIni2 = localOpt;

                    if(fit1<fit2 && fit1<minCost){
                        localOpt = localIni1;
                        minCost = fit1;
                        cout<<minCost<<endl;
                        break;
                    }else if(fit2<fit1 && fit2<minCost){
                        localOpt = localIni2;
                        minCost = fit2;
                        cout<<minCost<<endl;
                        firstLevel.erase(second);
                    }else{
                        localOpt = localIni;
                    }
                }
            }
            if(breakflag)
                break;
        }

        decode(localOpt, geneBit, servers);
        optimal_servers = servers;
        if(breakflag){
            getresult(mincostflow, optimal_servers, serverCost, minCostPath, m, stream);
            result = stream.str();
            write_result(result.c_str(), filename);
            return;
        }

    //minCost = cost;
    population[1] = localOpt;

    cout<<"**********************"<<endl;

    }

    //第三次缩减代价，服务器规模缩减，以路径代价代替服务器代价
    if(caseflag != 0){
        nn = servers.size();
        for(int i=0;i<nn;++i){
            //时间控制
            if(gettime() > ENDTIME){
                breakflag = true;
                break;
        }

    	int location = servers[i];//取出服务器位置
    	localIni = localOpt;
    	localOpt.gene[location] = false;

    	decode(localOpt, nodeNum, servers);//获取服务器部署
        int fit = mincostflow.multiMinCostFlow3(servers);
        fit += serverCost*servers.size();
        if(fit < minCost){//代价降低保留更改
            localIni = localOpt;
            minCost = fit;
            cout<<fit<<endl;
        }else{//没有降低返回初始状态
            localOpt = localIni;
        }

    	if(breakflag)
            break;
        }

        decode(localOpt, geneBit, servers);
        optimal_servers = servers;
        if(breakflag){
            getresult(mincostflow, optimal_servers, serverCost, minCostPath, m, stream);
            result = stream.str();
            write_result(result.c_str(), filename);
            return;
        }

        //population[3] = localOpt;

        cout<<"************************"<<endl;


        //第三次缩减代价，服务器规模缩减，以路径代价代替服务器代价
        nn = servers.size();
        for(int i=0;i<nn;++i){
            //时间控制
            if(gettime() > ENDTIME){
                breakflag = true;
                break;
            }

        int location = servers[i];//取出服务器位置
    	localIni = localOpt;
    	localOpt.gene[location] = false;

    	decode(localOpt, nodeNum, servers);//获取服务器部署
        int fit = mincostflow.multiMinCostFlow3(servers);
        fit += serverCost*servers.size();
        if(fit < minCost){//代价降低保留更改
            localIni = localOpt;
            minCost = fit;
            cout<<fit<<endl;
        }else{//没有降低返回初始状态
            localOpt = localIni;
        }

    	if(breakflag)
            break;
        }

        decode(localOpt, geneBit, servers);
        optimal_servers = servers;
        if(breakflag){
            getresult(mincostflow, optimal_servers, serverCost, minCostPath, m, stream);
            result = stream.str();
            write_result(result.c_str(), filename);
            return;
        }

        population[3] = localOpt;

        cout<<"************************"<<endl;
    }

    //第二次缩减代价，节点向内逐步纵深
    if(caseflag != 0){
        nn = servers.size();
        for(int i=0;i<nn;++i){
            int location = servers[i];//取出服务器位置
            localIni = localOpt;
            localOpt.gene[location] = false;
            localIni1 = localOpt;

            int cn = graph.G[location].size();
            bool changeflag = false;
            for(int j=0;j<cn;++j){
                //时间控制
                if(gettime() > ENDTIME){
                    breakflag = true;
                    break;
                }

                localOpt = localIni1;

                int netNode = graph.G[location][j].to;
                localOpt.gene[netNode] = true;
                decode(localOpt, nodeNum, servers);//获取服务器部署
                int fit = mincostflow.multiMinCostFlow3(servers);
                fit += serverCost*servers.size();
                if(fit < minCost){//代价降低保留更改
                    changeflag = true;
                    localIni1 = localOpt;
                    minCost = fit;
                    cout<<fit<<endl;
                }else{//没有降低返回初始状态
                    localOpt = localIni1;
                }
            }
            if(!changeflag){
                localOpt = localIni;
            }
            if(breakflag)
                break;
        }

        decode(localOpt, geneBit, servers);
        optimal_servers = servers;
        if(breakflag){
            getresult(mincostflow, optimal_servers, serverCost, minCostPath, m, stream);
            result = stream.str();
            write_result(result.c_str(), filename);
            return;
        }

        //minCost = cost;
        population[2] = localOpt;

    }

    return;


    cout<<"**********************"<<endl;


    //剩余染色体按随机变异生成
    for(int i=3;i<chormNum;++i){
            generateChorm3(population[i], localOpt, geneBit, probability);
    }


    //GA迭代
    int cntChanged = 0;         //最小代价改变次数
    int nProtect = 0;           //染色体保护数目
    int cntValidChorm = 0;
    while(generation--){

        //以最小费用流算法为适度函数，求各染色体适应度
        fitness(population, mincostflow, servers, geneBit, serverCost, fitAll, nProtect, breakflag);
        if(breakflag)
            break;

        //染色体选择
        cntValidChorm = 0;
        chormSelection(population, new_population, fitAll, cntValidChorm);


        //更新当前最优解
        if(fitAll[0].first < minCost){
            decode(new_population[0], geneBit, servers);
            optimal_servers = servers;
            minCost = fitAll[0].first;
            cout<<minCost<<endl;
        }

        //交叉
        crossover(crossoverRate, population, new_population, cntValidChorm, probability, chormNum, geneBit, maxServers, nProtect);

        //变异
        mutation(mulationRate, population, chormNum, geneBit);

        //收敛后退出
        //cout<<cntMinCost<<endl;
        /*if(cntValidChorm>50 && tempMinCost == minCost){
            cntMinCost++;
        }else{
            cntMinCost = 0;
        }
        if((cntChanged>15||cntValidChorm>50) && cntMinCost>25){//如果50次迭代最小代价仍然没有改变，则认为收敛，跳出迭代
            break;
        }
        if(cntMinCost > 600)
            break;
        tempMinCost = minCost;*/
        //cout<<minCost<<endl;
        //break;

    }

    cout<<10000 - generation<<endl;

    getresult(mincostflow, optimal_servers, serverCost, minCostPath, m, stream);
    result = stream.str();
    write_result(result.c_str(), filename);

    return;

}
