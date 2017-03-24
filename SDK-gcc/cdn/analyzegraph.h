#ifndef __ANALYZEGRAPH__H__
#define __ANALYZEGRAPH__H__

void analyzegraph(Graph& graph, vector<double>& probability, bool eGene[], bool gGene[], bool mGene[]){

    //获取优良中基因
    int index1 = 0;
    int index2 = 0;
    int index3 = 0;
    for(int i=0;i<graph.consumers.size();i++){
        graph.spfa(graph.consumers[i].netNode,index1,index2,index3);
        eGene[index1] = true;
        gGene[index2] = true;
        mGene[index3] = true;
    }

    //分析图，对每一个网络节点设置一个优先概率，以在产生基因的时候按照每个节点的优先概率选出
    vector<double> priporityNode(graph.nodeNum,1);
    //与消费节点直接相连的加10分，间接相连的加5分
    for(int i=0;i<graph.consumerNum;i++){
        int netNode = graph.consumers[i].netNode;
        priporityNode[netNode] += 50.0;
        for(int j=0;j<graph.G[netNode].size();j++){
            priporityNode[graph.G[netNode][j].to] += 30.0;
        }
    }

    //按照每个点输出边的比值f/cost加分
    for(int i=0;i<graph.nodeNum;i++){
        for(int j=0;j<graph.G[i].size();j++){
            priporityNode[i] += (double)graph.G[i][j].cap / (double)graph.G[i][j].cost;
        }
    }

    //获得优先概率
    double totalsum = 0;
    for(int i=0;i<graph.nodeNum;i++){
        totalsum += priporityNode[i];
    }
    for(int i=0;i<graph.nodeNum;i++){
        probability[i] = priporityNode[i] / totalsum;
    }
}


#endif // __ANALYZEGRAPH__H__
