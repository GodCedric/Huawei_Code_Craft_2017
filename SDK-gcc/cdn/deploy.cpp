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
    vector<int> result_servers;
    int minCost;                        //当前最小费用
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）
    string result;                      //文件输出结果

    // 创建图
    Graph graph;
    graph.createGraph(topo);
    //int a=0,b=0,c=0;
    //graph.spfa(0,a,b,c);
    //return;

    //cout<<graph.G.size()<<endl;
    int nodeNum = graph.nodeNum;
    int consumerNum = graph.consumerNum;
    int serverCost = graph.serverCost;

    //int Judge = 0;
    //if(nodeNum>200 && nodeNum<600){
        //Judge = 1;
    //}


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


    //测试
    /*ostringstream stream;
    int haha[41] = {4,8,10,11,15,17,25,28,29,34,35,42,44,57,62,63,67,70,75,76,80,85,90,102,103,109,118,119,120,122,126,130,135,139,141,142,146,147,149,153,154};
    vector<int> servers(haha,haha+41);

    res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
    cost = res+graph.serverCost*servers.size();
    stream.clear();stream.str("");
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
    result = stream.str();
    cout<<"***********"<<cost<<endl;
    write_result(result.c_str(), filename);
    return;*/

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
    stream<<graph.consumerNum<<"\n"<<"\n";
    for(int i=0;i<graph.consumerNum;i++){
        stream<<graph.consumers[i].netNode<<" "<<i<<" "<<graph.consumers[i].flowNeed<<"\n";
        //初始种群设置
        population[0].gene[graph.consumers[i].netNode] = true;  //把最差解先放进去
        firstLevel.insert(graph.consumers[i].netNode);
    }
    minCost = graph.consumerNum*graph.serverCost;       //以最差解费用初始化最小费用
    int worstCost = minCost;
    cout<<worstCost<<endl;
    cout<<endl;
    result = stream.str();

    //初步寻优，第一层服务器有相连的可能降低代价
    bool breakflag = false;
    Chorm localOpt = population[0];
    Chorm localIni = localOpt;
    Chorm localIni1 = localOpt;
    Chorm localIni2 = localOpt;
    int localCost = worstCost;
    for(int i=0;i<graph.consumerNum;i++){
        int netNum = graph.consumers[i].netNode;
        if(firstLevel.find(netNum) == firstLevel.end())
            continue;
        int nn = graph.G[netNum].size();
        //Chorm chorm1 = population[0];
        //Chorm chorm2 = population[0];
        int fit1;
        int fit2;
        for(int j=0;j<nn;j++){

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
                fit1 = mincostflow.multiMinCostFlow2(servers);
                fit1 += serverCost*servers.size();
                localIni1 = localOpt;

                localOpt = localIni;
                localOpt.gene[second] = false;
                decode(localOpt, nodeNum, servers);//获取服务器部署
                fit2 = mincostflow.multiMinCostFlow2(servers);
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


            /*chorm1.gene[netNum] = false;
            decode(chorm1, nodeNum, servers);//获取服务器部署
            int fit1 = mincostflow.multiMinCostFlow2(servers);
            fit1 += serverCost*servers.size();

            int second = graph.G[netNum][j].to;
            if(firstLevel.find(second) != firstLevel.end()){//如果在第一层找到，证明有链路相连，可能降低代价

                chorm2 = population[0];
                chorm2.gene[second] = false;

                decode(chorm2, nodeNum, servers);//获取服务器部署
                int fit2 = mincostflow.multiMinCostFlow2(servers);
                fit2 += serverCost*servers.size();

                if(fit1<fit2 && fit1<INFMAX){//如果fit1是较小解
                    localIni = localOpt;

                    localOpt.gene[netNum] = false;
                    decode(localOpt, nodeNum, servers);//获取服务器部署
                    int fit = mincostflow.multiMinCostFlow2(servers);
                    fit += serverCost*servers.size();
                    if(fit<INFMAX && fit<localCost){
                        //满足减小代价，保留更改
                        localCost = fit;
                        cout<<1<<" "<<fit<<endl;
                        break;
                    }else{//没有减小代价或无解，返回初始状态
                        localOpt = localIni;
                    }
                }else if(fit2<fit1 && fit2<INFMAX){
                    localIni = localOpt;

                    localOpt.gene[second] = false;
                    decode(localOpt, nodeNum, servers);//获取服务器部署
                    int fit = mincostflow.multiMinCostFlow2(servers);
                    fit += serverCost*servers.size();
                    if(fit<INFMAX && fit<localCost){
                        //满足减小代价，保留更改
                        localCost = fit;
                        firstLevel.erase(second);
                        cout<<2<<" "<<fit<<endl;
                    }else{//没有减小代价或无解，返回初始状态
                        localOpt = localIni;
                    }
                }
            }else{//否则的话，此节点在树中的深度为2,存下来用于二级深度寻优
                secondLevel[second].push_back(netNum);
            }*/
        }
        if(breakflag)
            break;
    }

    decode(localOpt, geneBit, servers);
    res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
    cost = res+graph.serverCost*servers.size();
    result_servers = servers;
    cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<cost<<endl;
    stream.clear();stream.str("");
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
    result = stream.str();
    if(breakflag){
        write_result(result.c_str(), filename);
        return;
    }
    //write_result(result.c_str(), filename);
    //return;

    minCost = cost;
    population[1] = localOpt;

    cout<<"**********************"<<endl;

    //第二种寻优方案，节点向内逐步纵深
    int nn = servers.size();
    for(int i=0;i<nn;i++){
    	int location = servers[i];//取出服务器位置
    	localIni = localOpt;
    	localOpt.gene[location] = false;
    	localIni1 = localOpt;

    	int cn = graph.G[location].size();
    	bool changeflag = false;
    	for(int j=0;j<cn;j++){
            //时间控制
            if(gettime() > ENDTIME){
                breakflag = true;
                break;
            }

            localOpt = localIni1;

    		int netNode = graph.G[location][j].to;
    		localOpt.gene[netNode] = true;
    		decode(localOpt, nodeNum, servers);//获取服务器部署
    		int fit = mincostflow.multiMinCostFlow2(servers);
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
    res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
    cost = res+graph.serverCost*servers.size();
    result_servers = servers;
    cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<cost<<endl;


    stream.clear();stream.str("");
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
    result = stream.str();
    if(breakflag){
        write_result(result.c_str(), filename);
        return;
    }
    //if(Judge == 1){
       // write_result(result.c_str(), filename);
        //return;
    //}
    //write_result(result.c_str(), filename);
    //return;

    minCost = cost;
    population[2] = localOpt;

    cout<<"**********************"<<endl;

    //二级深度寻优
    //localOpt = population[0];
    //localCost = worstCost;
    /*map<int,vector<int> >::iterator itr = secondLevel.begin();
    Chorm temp = localOpt;
    for(;itr!=secondLevel.end();itr++){
        //时间控制
        if(gettime() > 88){
            breakflag = true;
            break;
        }

        int ind = itr->first;
        vector<int> deep2 = itr->second;
        if(deep2.size() > 1){//有降低代价的可能
            localIni = localOpt;

            localOpt.gene[ind] = true;
            for(int i=0;i<deep2.size()-1;i++){
                for(int j=i+1;j<deep2.size();j++){
                    localOpt.gene[deep2[i]] = false;
                    localOpt.gene[deep2[j]] = false;

                    decode(localOpt, nodeNum, servers);//获取服务器部署
                    int fit = mincostflow.multiMinCostFlow2(servers);
                    fit += serverCost*servers.size();

                    if(fit<INFMAX && fit<localCost){
                        //满足减小代价，保留更改
                        temp = localOpt;
                        localCost = fit;
                        cout<<fit<<endl;
                    }else{//没有减小代价或无解，返回初始状态
                        localOpt = localIni;
                    }
                }
            }
        }
    }

    localOpt = temp;
    decode(localOpt, geneBit, servers);
    res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
    cost = res+graph.serverCost*servers.size();
    stream.clear();stream.str("");
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
    result = stream.str();

    if(breakflag){
        write_result(result.c_str(), filename);
        return;
    }

    minCost = cost;
    population[2] = localOpt;
    cout<<"**********************"<<endl;*/
    //return;

    //初步寻优方案，以每个消费节点为起点，寻找各自满足单个消费节点的服务器部署位置
    /*int singleCost;     //单消费节点最优
    int localOpt = serverCost;  //局部最优
    int cntStay = 0;        //保留优秀解的个数
    vector<int> serversForCnos[nodeNum];       //保留对每个消费节点的最优服务器部署方案
    for(int i=0;i<graph.consumers.size();i++){
        singleCost = graph.spfa2(graph.consumers[i].netNode, servers, graph.consumers[i].flowNeed);
        serversForCnos[i] = servers;
        //if(singleCost < serverCost){    //说明该服务器有改进空间


        //}
        cout<<singleCost<<"  "<<serversForCnos[i].size()<<endl;
    }
    return;*/


    //////////以上两级寻优主要针对中高级测试用例
    //////////初级测试用例仍主要采用遗传
    //////////


    for(int i=3;i<chormNum;i++){
            generateChorm3(population[i], localOpt, geneBit, probability);
    }
    /*if(Judge == 1){
        //剩下的随机产生
        for(int i=3;i<chormNum;i++){
            generateChorm3(population[i], localOpt, geneBit, probability);
            //randomChorm(population[i],chormNum,geneBit,maxServers);
            //generateChorm(population[i], probability, chormNum, geneBit, maxServers);
        }
    }else{
        for(int i=3;i<chormNum;i++){
            generateChorm3(population[i], localOpt, geneBit, probability);
            //randomChorm(population[i],chormNum,geneBit,maxServers);
            //generateChorm(population[i], probability, chormNum, geneBit, maxServers);
        }
        //将与消费节点最近的网络节点生成一个染色体
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
            if(initChormNum >= 80)
                break;
        }

        //0和1号染色体随机交配产生100条染色体
        int lengthGene = geneBit / 20;
        for(int i=2;i<chormNum-1;){
            population[i] = population[0];
            population[i+1] = population[1];
            int start = rand() % (geneBit - lengthGene);
            for(int j=0;j<lengthGene;j++){
                swap(population[i].gene[j],population[i+1].gene[j]);
            }
            i += 2;
        }

        //将优良中基因与初始最差基因交配产生一批基因
        //先将3个基因放进去
        for(int i=0;i<geneBit;i++){
            population[3].gene[i] = excellentGene[i];
            population[4].gene[i] = goodGene[i];
            population[5].gene[i] = mediumGene[i];
        }
        int lengthGene = geneBit / 10;      //1/10的基因片段
        int initChormNum = 16;
        for(int i=0;i<initChormNum;i++){
            int start = rand() % (geneBit - lengthGene);
            population[6+i*3] = population[0];
            population[6+i*3+1] = population[1];
            population[6+i*3+2] = population[2];
            for(int j=0;j<lengthGene;j++){
                //优等基因交换
                swap(population[i*3].gene[j],excellentGene[j]);
                //良好基因交换
                swap(population[i*3+1].gene[j],goodGene[j]);
                //中等基因交换
                swap(population[i*3+2].gene[j],mediumGene[j]);
            }
        }
        initChormNum = 18*3 + 6;

        //剩下的随机产生
        for(int i=initChormNum;i<chormNum;i++){
            //randomChorm(population[i],chormNum,geneBit,maxServers);
            generateChorm(population[i], probability, chormNum, geneBit, maxServers);
        }
    }*/

    //GA迭代
    int cntChanged = 0;         //最小代价改变次数
    int nProtect = 0;           //染色体保护数目
    int cntValidChorm = 0;
    int tempMinCost = minCost;
    int cntMinCost = 0;
    //minCost = worstCost;
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
            decode(new_population[0], geneBit, servers);
            res = mincostflow.multiMinCostFlow(servers,minCostPath,m);
            cost = res+graph.serverCost*servers.size();
            result_servers = servers;
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
            cout<<minCost<<endl;
        }
        //cout<<cntValidChorm<<endl;

        //交叉
        crossover(crossoverRate, population, new_population, cntValidChorm, probability, chormNum, geneBit, maxServers, nProtect);

        //变异
        //mutation(mulationRate, population, chormNum, geneBit);

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
    //cout<<generation<<endl;
    cout<<10000 - generation<<endl;
    //cout<<cntMCF<<endl;

    for(int i=0;i<result_servers.size();i++){
        cout<<result_servers[i]<<"  ";
    }
    cout<<endl;

    result = stream.str();
    //cout<<result<<endl;
    const char* topo_file = result.c_str();

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
