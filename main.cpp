#include <ctime>
#include <iostream>

#include "header/graph.hpp"
#include "header/Routing.hpp"



void show_demand(Graph&graph);//for test

int main(int argc, char** argv)
{
    if(argc!=2){
        std::cerr<<"Wrong parameters!"<<std::endl;
        return -1;
    }
    std::string path = "./benchmark/";
    std::string fileName = argv[1];
    //argv[1]
    Graph* graph = new Graph(path+fileName);
	
	//graph->showEffectedNetSize();
    show_demand(*graph);
    graph->placementInit();	
    show_demand(*graph);
	
/*	
	int counter = 0;
	while(graph->cellMoving()) counter++;
	std::cout << counter << std::endl;
	
    show_demand(*graph);*/
	return 0;
}

void show_demand(Graph&graph)
{
    int LayerNum = graph.LayerNum();
    std::pair<int,int>Row = graph.RowBound();
    std::pair<int,int>Col = graph.ColBound();

    int total_demand = 0;
    for(int lay = 1;lay <= LayerNum;lay++)
    {
        
        std::cout<<"Layer : "<< lay <<"\n";
        for(int row = Row.second;row >=Row.first ; row--)
        {
            for(int col = Col.first; col <= Col.second; col++)
            {
                std::cout<<graph(row,col,lay).demand<<" ";
                total_demand+=graph(row,col,lay).demand;
            }
            std::cout<<"\n";
        }
    }
    //std::cout<<"Total :"<<total_demand<<"\n";
}
