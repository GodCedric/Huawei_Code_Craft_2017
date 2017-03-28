#ifndef __GENETICALGORITHM__H__
#define __GENETICALGORITHM__H__

#include <set>
#include <algorithm>

// 染色体
struct Chorm{
    bool gene[1000];                       //基因序列
    int fit = 0;                                //适应值

    Chorm(int f):fit(f){
        for(int i=0;i<MAXN;i++){
            gene[i] = false;
        }
    }
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
    //int numservers = rand() % (maxServers - 1) + 1;

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

// 随机生成一个有较大概率有解的染色体
void generateChorm2(Chorm& chorm, Chorm chorm1, int geneBit){

    //以当前最优解执行变异操作，产生新解
    chorm = chorm1;
    int n = rand() % 5; //变异个数
    while(n--){
        int index = rand() % geneBit;
        chorm.gene[index] = ! chorm.gene[index];
    }
}


// 初始化种群  待修改
/*void initChorm(vector<double>& probability, vector<Chorm>& population, int chormNum, int geneBit, int maxServers){
    //先把最差解放进去


    for(int i=1;i<chormNum;i++){
        generateChorm(population[i], probability, chormNum, geneBit, maxServers);
    }
}*/

// 求各染色体的适应度
void fitness(vector<Chorm>& population, MCF& mincostflow, vector<int>& servers,int nodeNum, int serverCost, vector<pair<int,int> >& fitAll, int nProtect,bool& breakflag){
    //vector<int> servers;                //服务器位置
    int fit;                            //适应度
    //double timelimit;                   //程序计时
    for(int i=nProtect;i<population.size();i++){
        decode(population[i], nodeNum, servers);//获取服务器部署
        int fit = mincostflow.multiMinCostFlow2(servers);
        //cntMCF++;
        if(gettime() > 88.5){
            breakflag = true;
            break;
        }
        if(fit != INFMAX)
            fit += serverCost*servers.size();
        //cout<<fit<<"  ";
        fitAll[i].first = fit;                  //存储适应度
        fitAll[i].second = i;                   //存储对应染色体坐标
        population[i].fit = fit;                //得到适应度
        //cout<<fit<<endl;
    }
    //cout<<endl;
}

// 染色体选择
void chormSelection(vector<Chorm>& population, vector<Chorm>& new_population, vector<pair<int,int> >& fitAll, int& cntValidChorm){
    int chormNum = population.size();
    //根据各染色体的适应度排序，适应度高的从从前到后放到new_population里面，适应度为-1的舍弃，并补充新的染色体到种群
    sort(fitAll.begin(),fitAll.end());
    //将有解的染色体放到new_population里面
    for(int i=0;i<chormNum-1;i++){
        if(fitAll[i].first!=INFMAX && fitAll[i].first!=fitAll[i+1].first){
            new_population[cntValidChorm++] = population[fitAll[i].second];
        }
    }
}

// 交叉
void crossover(double crossoverRate,
               vector<Chorm>& population, vector<Chorm>& new_population,
               int cntValidChorm, vector<double>& probability,
               int chormNum, int geneBit, int maxServers, int nProtect){

    int m = 5;  //保护几条染色体
    int n = 15;  //有几条交叉感染的染色体
    int x = 20;  //留出10条染色体的位置随机生成，保证种群多样性

    //交叉位数，有10%的染色体基因发生交叉
    const int bitcnt = geneBit / 10;

    //不满足10个解的继续寻找，满足的解保留，不满足的更新
    if(cntValidChorm < m){
        for(int i=cntValidChorm;i<chormNum;i++){
            generateChorm2(population[i], population[0], geneBit);
            //generateChorm(new_population[i], probability, chormNum, geneBit, maxServers);
            //randomChorm(new_population[i], chormNum, geneBit, maxServers);
        }
        nProtect = cntValidChorm;
    }else{//已经存在5个有效解了
        //前10个最小有效解两两交叉获得15个新的有效解,放在尾部
        nProtect = m;
        int cnt = 0;
        int a = 0;
        int b = 0;
        for(int i=0;i<m;i++){
            for(int j=i+1;j<m;j++){
                cnt++;
                a = chormNum-x-cnt;
                for(int k1=0;k1<geneBit;k1++){
                    new_population[a].gene[k1] = population[i].gene[k1];
                }
                cnt++;
                b = chormNum-x-cnt;
                for(int k1=0;k1<geneBit;k1++){
                    new_population[b].gene[k1] = population[j].gene[k1];
                }
                //交叉，随机生成交叉起始点
                int start = rand() / (chormNum-bitcnt);
                //开始交叉
                for(int mm=start;mm<bitcnt;mm++){
                    swap(new_population[a].gene[mm],new_population[b].gene[mm]);
                }
            }
        }
        //第6个开始的有效染色体两两按交叉概率交叉
        int bian = min(cntValidChorm, chormNum-n-1);
        for(int i=m;i<bian;){

            //double rate = rand() % 1000 / 1000.0;
            //if(rate < crossoverRate){
                //交叉，随机生成交叉起始点
                int start = rand() % (geneBit-bitcnt);
                //开始交叉
                for(int m=start;m<bitcnt;m++){
                    swap(new_population[i].gene[m],new_population[i+1].gene[m]);
                }
            //}

            i += 2;
        }
        //剩下的无解的染色体仍然按照随机生成，并且至少保证留出来20个染色体为随机生成，补充种群多样性
        for(int i=((cntValidChorm<(chormNum-x))?cntValidChorm:(chormNum-x));i<chormNum;i++){
            generateChorm2(population[i], population[0], geneBit);
            //generateChorm(new_population[i], probability, chormNum, geneBit, maxServers);
            //randomChorm(new_population[i], chormNum, geneBit, maxServers);
        }
    }

    // 更新种群
    population = new_population;

}

//变异
void mutation(const double mulationRate, vector<Chorm>& population,
              int chormNum, int geneBit){
    double rate = rand() % 1000 / 1000.0;
    if(rate < mulationRate){
        //单数染色体不变异，双数染色体变异
        for(int i=1;i<chormNum-20;){
            int index = rand() % geneBit;
            population[i].gene[index] = !population[i].gene[index];
            i += 2;
        }
    }

}


#endif
