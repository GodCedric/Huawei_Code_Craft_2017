#ifndef __GETRESULT__H__
#define __GETRESULT__H__

void getresult(MCF& mincostflow, vector<int> servers, int serverCost,
               vector<int> minCostPath[], int &m,
               ostringstream& stream){

    int res = mincostflow.multiMinCostFlow(servers,minCostPath,m);

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

}



#endif
