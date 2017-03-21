#ifndef __GENETICALGORITHM__H__
#define __GENETICALGORITHM__H__

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

// 随机生成一个有较大概率有解的染色体
void generateChorm(Chorm& chorm){


}

// 初始化种群
void initChorm(int chormNum, int geneBit, vector<Chorm>& population, int maxServers){
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

}

// 求各染色体的适应度
void fitness(vector<Chorm>& population, MCF& mincostflow, int nodeNum, vector<pair<int,int> >& fitAll){
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）
    vector<int> servers;                //服务器位置
    int fit;                            //适应度
    for(int i=0;i<population.size();i++){
        decode(population[i], nodeNum, servers);//获取服务器部署
        fit = mincostflow.multiMinCostFlow(servers,minCostPath,m);
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
        if(fitAll[i].first!=INF){
            new_population[cntValidChorm++] = population[fitAll[i].second];
        }
    }
}

// 交叉
void crossover(const double crossoverRate, vector<Chorm>& population, vector<Chorm>& new_population, int& cntValidChorm){
    //前10优解保护，这10个解交叉生成45个子代
    //不满足10个解的继续寻找


}


#endif
