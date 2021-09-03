#include "header/graph.hpp"
#include "header/Routing.hpp"
#include "header/TwoPinNet.hpp"
#include "header/analysis.hpp"
#include "header/RoutingSchedule.hpp"
#include "header/flute.h"

#include <time.h>
#include <thread>
#include <algorithm>
Graph* graph = nullptr;


void Init( std::string path,std::string fileName){graph = new Graph(path+fileName);}
void OutPut(Graph*graph,std::string fileName);
void OnlyRouting(Graph*graph,int batchSize = 1,bool overflow = false,float topPercent = 0);
void RoutingWithCellMoving(Graph*graph);
void countdmd(Graph*graph);
#include <chrono>
std::chrono::duration<double, std::milli> IN;
std::chrono::duration<double, std::milli> RoutingTime;
std::chrono::duration<double, std::milli> OverFlowProcessTime;
std::chrono::duration<double, std::milli> pinsTime;

std::chrono::duration<double,std::milli>movtime;
std::chrono::duration<double,std::milli>collectTime;

table strtable;
float origin;

int seed;//random seed

int totalNetsTime = 0;
bool t2t = false;

int main(int argc, char** argv)
{
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    readLUT();
    if(argc!=2){
        std::cerr<<"Wrong parameters!"<<std::endl;
        return -1;
    }
    std::string path = "./benchmark/";
    std::string fileName = argv[1];
    
    Init(path,fileName);    
    strtable.init(graph);
    origin = graph->score;
    


    //找多個,然後關聯性最大的一起移動一起繞






    t2t = true;
    OnlyRouting(graph,1);
    std::cout<<"only rout score:"<<origin-graph->score<<"\n";

    // countdmd(graph);
    int num = 10000;
    while(num--){
        t2t = false;
        RoutingWithCellMoving(graph);
        std::cout<<"move : score:"<<origin-graph->score<<"\n";
        t2t = true;
        OnlyRouting(graph,1);
        std::cout<<"only rout score:"<<origin-graph->score<<"\n";
    }
    // countdmd(graph);
    

    // std::cout<<"seed:"<<seed<<"\n";
    std::cout<<"final score:"<<origin-graph->score<<"\n";
    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> t3(t2-t1);
    std::cout<<"spend :"<<t3.count()/1000<<"s\n";
    std::cout<<"collect time:"<<collectTime.count()/1000<<"s\n";
    std::cout<<"overflow Proecess time:"<<OverFlowProcessTime.count()/1000<<"s\n";
    std::cout<<"routing time:"<<RoutingTime.count()/1000<<"s\n";
    std::cout<<"total nets time:"<<totalNetsTime<<"\n";
    OutPut(graph,fileName);

    utilization(graph,fileName);
    delete graph;
    

	return 0;
}

std::map<Net*,int> RelatedNets(CellInst*c)
{
    std::map<Net*,int>Nets;//有一些net重複了
    int Idx = 0;
    for(auto net:c->nets){
        if(Nets.find(net)==Nets.end())
            Nets.insert({net,Idx++});
    }
    return Nets;
}
void OutPut(Graph*graph,std::string fileName)
{

    std::vector<std::string>segments;
    std::cout<<"Routing complete !\n";
    PrintAll(graph,&segments);
    //寫成輸出檔案
    int NumRoutes = segments.size();
    fileName = fileName.substr(0,fileName.size()-4);
    fileName = fileName+"Out.txt";
    std::ofstream os{fileName};
    if(!os){
        std::cerr<<"error:file "<<fileName<<" cann't open!\n";
        exit(1);
    } 

    os<<"NumMovedCellInst "<< graph->moved_cells.size() <<"\n";
    for(auto cell:graph->moved_cells)
        os<<"CellInst "<< cell->name << " " << cell->row << " " << cell->col <<"\n";
    os<<"NumRoutes "<<NumRoutes<<"\n";

    for(auto s:segments)
    {
        os<<s<<"\n";
    }
    std::cout<<"saving done!\n";

    os.close();
}


//先做一個topPercent的版本
void OnlyRouting(Graph*graph,int batchSize,bool overflow,float topPercent)
{
    float sc = graph->score;
    std::vector<netinfo> netlist = getNetlist(graph);//get netList
    //-----------------overflow allowed----------------------------
    if(overflow){
        routing(graph,netlist,0,netlist.size()*topPercent,overFlowRouting,batchSize);
    }
    //------------------------------------------------------------
    routing(graph,netlist,0,netlist.size(),RoutingSchedule,batchSize);
}

#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
bool randomaccept(float sc1,float sc2)
{
    // seed = time(NULL);
    // srand(seed);
    // int r = rand()%10;
    if(sc1<sc2)return true;

    // return r>8;

    return false;

}

void movRouting(Graph*graph,std::vector<netinfo>&netlist,CellInst*movCell)
{
    bool allSucces = true;
    float sc = graph->score;
    std::vector<ReroutInfo>infos;std::vector<int>RipId;infos.reserve(netlist.size());RipId.reserve(netlist.size());
    
   

    for(int j = 0;j<netlist.size();j++){
        int nid = netlist.at(j).netId;
        allSucces = RoutingSchedule(graph,nid,infos,RipId);
        if(!allSucces)
        allSucces = overFlowRouting(graph,nid,infos,RipId,0,nullptr);
        if(!allSucces)break;

    }
  
    if(allSucces&&randomaccept(graph->score,sc)){
        Accept(graph,infos);
        movCell->originalRow = movCell->row;
        movCell->originalCol = movCell->col;
        // std::cout<<"score:"<<origin-graph->score<<"\n";
    }else{
        graph->removeCellsBlkg(movCell);
        Reject(graph,infos,RipId);
        movCell->row = movCell->originalRow;
        movCell->col = movCell->originalCol;

        if(!graph->insertCellsBlkg(movCell))
        {
            std::cerr<<"error in insert\n";
        }
    }
}

void RoutingWithCellMoving(Graph*graph)
{


    

    graph->placementInit();
    std::pair<std::string,CellInst*>movcellPair;

    int mov = 0;

    std::string last;
    int samecount = 0;
    while((movcellPair=graph->cellMoving()).second)
    {
        CellInst* movCell = movcellPair.second;


        
        mov++;
        graph->moved_cells.insert(movCell);
        if(graph->moved_cells.size()>graph->MAX_Cell_MOVE)
        {
            graph->removeCellsBlkg(movCell);
            movCell->row = movCell->originalRow;
            movCell->col = movCell->originalCol;
            graph->insertCellsBlkg(movCell);
			if(movCell->row == movCell->initRow && movCell->col == movCell->initCol){
				graph->moved_cells.erase(movCell);
			}
            continue;
        }

        std::map<Net*,int>Nets = RelatedNets(movCell);
        std::vector<netinfo>Netsid;Netsid.reserve(Nets.size());

        auto t1 = std::chrono::high_resolution_clock::now();
    
        for(auto n:Nets)
        {
            int nid = std::stoi(n.first->netName.substr(1,-1));
            int hpwl = HPWL(n.first);
            int wl = graph->getNetGrids(nid)->wl();
            Netsid.push_back({nid,hpwl,wl});
        }

        totalNetsTime+=Nets.size();
        auto t2 = std::chrono::high_resolution_clock::now();
        collectTime+=t2-t1;

        t1 = std::chrono::high_resolution_clock::now();
        movRouting(graph,Netsid,movCell);
        // routing(graph,Netsid,0,Netsid.size(),RoutingSchedule);
        t2 = std::chrono::high_resolution_clock::now();
        RoutingTime+=t2-t1;

        if(movCell->row == movCell->initRow && movCell->col == movCell->initCol){
			graph->moved_cells.erase(movCell);
		}
        
    }


}
void countdmd(Graph*graph){
    int LayerNum = graph->LayerNum();
    std::pair<int,int>Row = graph->RowBound();
    std::pair<int,int>Col = graph->ColBound();

    int total_demand = 0;
    for(int lay = 1;lay <= LayerNum;lay++)
    {
        
        // std::cout<<"Layer : "<< lay <<"\n";
        for(int row = Row.second;row >=Row.first ; row--)
        {
            for(int col = Col.first; col <= Col.second; col++)
            {
                // std::cout<<graph(row,col,lay).demand<<" ";
                total_demand+=(*graph)(row,col,lay).demand;
            }
            // std::cout<<"\n";
        }
    }
    std::cout<<"Total :"<<total_demand<<"\n";
}
