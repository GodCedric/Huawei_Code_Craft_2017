#ifndef __GENETICALGORITHM__H__
#define __GENETICALGORITHM__H__

#include <set>
#include <algorithm>

// 染色体
struct Chorm{
    bool gene[MAXN] = {false};                       //基因序列
    int fit;                                //适应值
    //double rfit;                          //相对fit值
    //double cfit;                          //积累概率
};

// 编码：把一个服务器部署方案转换为染色体
void encode(vector<int>& servers, Chorm& chorm){
    for(int i=0;i<servers.size();i++){
        chorm.gene[servers[i]] = true;
    }
}

// 解码：根据基因序列产生服务器部署位置
void decode(Chorm& chorm, int nodeNum, vector<int>& servers){
    servers.clear();
    for(int i=0;i<nodeNum;i++){
        if(chorm.gene[i]){
            servers.push_back(i);
        }
    }
}

// 完全随机产生染色体
void randomChorm(Chorm& chorm, int chormNum, int geneBit, int maxServers){
    //服务器清空置零
    for(int m=0;m<geneBit;m++){
        chorm.gene[m] = false;
    }

    int numservers = rand() % (maxServers-1) +1;   //随机生成服务器数量，最多maxServers个
    //随机分布服务器
    for(int j=0;j<numservers;j++){
        int index = rand() % geneBit;
        chorm.gene[index] = true;
    }
}

// 随机生成一个有较大概率有解的染色体
void generateChorm(Chorm& chorm, vector<double>& probability, int chormNum, int geneBit, int maxServers){
    //服务器清空置零
    for(int m=0;m<geneBit;m++){
        chorm.gene[m] = false;
    }

    int numservers = rand() % (maxServers-maxServers/2) + maxServers/2;   //随机生成服务器数量，最多maxServers个

    set<int> serversIndex;

    //轮盘赌获取serverIndex
    while(serversIndex.size() < numservers){
        double randomrate = rand() % 100000 / 100000.0;
        double tempsum = 0;
        for(int i=0;i<probability.size();i++){
            tempsum += probability[i];
            if(tempsum >= randomrate){
                serversIndex.insert(i);
                break;
            }
        }
    }

    //获取染色体
    set<int>::iterator itr;
    for(itr=serversIndex.begin();itr!=serversIndex.end();itr++){
        chorm.gene[*itr] = true;
    }

}

// 初始化种群  待修改
void initChorm(vector<double>& probability, vector<Chorm>& population, int chormNum, int geneBit, int maxServers){
    //先把最差解放进去

    for(int i=1;i<chormNum;i++){
        generateChorm(population[i], probability, chormNum, geneBit, maxServers);
    }
}

// 求各染色体的适应度
void fitness(vector<Chorm>& population, MCF& mincostflow, int nodeNum, int serverCost, vector<pair<int,int> >& fitAll){
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）
    vector<int> servers;                //服务器位置
    int fit;                            //适应度
    for(int i=0;i<population.size();i++){
        decode(population[i], nodeNum, servers);//获取服务器部署
        fit = mincostflow.multiMinCostFlow(servers,minCostPath,m);
        fit += serverCost*servers.size();
        fitAll[i].first = fit;                  //存储适应度
        fitAll[i].second = i;                   //存储对应染色体坐标
        population[i].fit = fit;                //得到适应度
        //cout<<fit<<endl;
    }
}

// 染色体选择
void chormSelection(vector<Chorm>& population, vector<Chorm>& new_population, vector<pair<int,int> >& fitAll, int& cntValidChorm){
    int chormNum = population.size();
    //根据各染色体的适应度排序，适应度高的从从前到后放到new_population里面，适应度为-1的舍弃，并补充新的染色体到种群
    sort(fitAll.begin(),fitAll.end());
    //将有解的染色体放到new_population里面
    for(int i=0;i<chormNum;i++){
        if(fitAll[i].first<INF){
            new_population[cntValidChorm++] = population[fitAll[i].second];
        }
    }
}

// 交叉
void crossover(const double crossoverRate,
               vector<Chorm>& population, vector<Chorm>& new_population,
               int& cntValidChorm, vector<double>& probability,
               int chormNum, int geneBit, int maxServers){

    //交叉位数，有20%的染色体基因发生交叉
    const int bitcnt = geneBit / 5;

    //不满足10个解的继续寻找，满足的解保留，不满足的更新
    if(cntValidChorm < 10){
        for(int i=cntValidChorm;i<chormNum;i++){
            generateChorm(new_population[i], probability, chormNum, geneBit, maxServers);
        }
    }else{//已经存在10个有效解了
        //前3个最小有效解保留，并两两交叉获得3个新的有效解,放在尾部
        /*int cnt = 0;
        for(int i=0;i<3;i++){
            for(int j=i+1;j<3;j++){
                cnt++;
                int a = chormNum-cnt;
                new_population[a] = population[i];
                cnt++;
                int b = chormNum-cnt;
                new_population[b] = population[j];
                //交叉，随机生成交叉起始点
                int start = rand() / (chormNum-bitcnt - 1) + 1;
                //开始交叉
                for(int m=start;m<bitcnt;m++){
                    swap(new_population[a].gene[m],new_population[b].gene[m]);
                }
            }
        }*/
        //第4个开始的有效染色体两两按交叉概率交叉
        for(int i=0;i<cntValidChorm-1;){

            double rate = rand() % 1000 / 1000.0;
            if(rate < crossoverRate){
                //交叉，随机生成交叉起始点
                int start = rand() % (geneBit-bitcnt - 1) + 1;
                //开始交叉
                for(int m=start;m<bitcnt;m++){
                    swap(new_population[i].gene[m],new_population[i+1].gene[m]);
                }
            }

            i += 2;
        }
        //剩下的无解的染色体仍然按照随机生成
        for(int i=((cntValidChorm<80)?cntValidChorm:80);i<chormNum;i++){
            generateChorm(new_population[i], probability, chormNum, geneBit, maxServers);
        }
    }

    // 更新种群
    population = new_population;

}


#endif
