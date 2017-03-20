#ifndef __GENETICALGORITHM__H__
#define __GENETICALGORITHM__H__

// 染色体
struct Chorm{
    bool gene[MAXN] = {false};                       //基因序列
    int fit;                                //适应值
    //double rfit;                          //相对fit值
    //double cfit;                          //积累概率
};

// 解码：根据基因序列产生服务器部署位置
void decode(Chorm& chorm, int nodeNum, vector<int>& servers){
    servers.clear();
    for(int i=0;i<nodeNum;i++){
        if(chorm.gene[i]){
            servers.push_back(i);
        }
    }
}

// 求各染色体的适应度
void fitness(vector<Chorm>& population, MCF& mincostflow, int nodeNum, vector<int>& fitAll){
    vector<int> minCostPath[MAXPATH];   //路径
    int m = MAXPATH;                    //路径数目（初始设为最大值）
    vector<int> servers;                //服务器位置
    int fit;                            //适应度
    for(int i=0;i<population.size();i++){
        decode(population[i], nodeNum, servers);//获取服务器部署
        fit = mincostflow.multiMinCostFlow(servers,minCostPath,m);
        fitAll[i] = fit;                        //存储适应度
        population[i].fit = fit;                //得到适应度
        //cout<<fit<<endl;
    }
}





#endif
