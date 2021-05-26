#include "Graph.hpp"
#include <iostream>
#include "Assign.hpp"
#include "Routing.hpp"


std::ostream& operator<<(std::ostream&os,const Point&p)
{
    os<<"("<<p.x<<","<<p.y<<","<<p.z<<")";
    return os;
}


void show_demand(Graph&graph)
{
    int Layer_Num = graph.Layer_Num();
    std::pair<int,int>Grids_size = graph.Grid_size();
    
    for(int z = 1;z <= Layer_Num;z++)
    {
        std::cout<<"Layer : "<<z<<"\n";
        for(int y = Grids_size.second;y >=1 ; y--)
        {
            for(int x = 1; x <= Grids_size.first; x++)
            {
                std::cout<<graph(x,y,z).demand<<" ";
            }
            std::cout<<"\n";
        }
    }
}

//繞線COST超參數
float VIA_W = 1;
float ESCAPE_W = 10;
int LAYER_SEARCH_RANGE = 3;

int main()
{
    Point P1{1,1,1};
    Point P2{5,5,1};
    Graph graph = std::vector<Layer>{//M1,M2,M3
        Layer(P1,P2,true,0.1,10),   // M1 : H
        Layer(P1,P2,false,0.5,10),  // M2 : V 
        Layer(P1,P2,true,1.5,1),   //  M3 : H
    };

    
    Net_routing_Info Net1{1,1};


    //over flow test
    graph.Non_default_grid(2,1,1,-10);
    auto result1 = Heuristic_routing(P1,1,0,graph,Net1);

    std::cout<<"cost = " <<std::get<1>(result1)<<"\n";
    std::cout<< "point = "<<std::get<2>(result1)<<"\n";


    //escape rate test
    graph.Non_default_grid(2,1,2,-10);
    graph.Non_default_grid(3,1,1,-10);
 
    auto result2 = Heuristic_routing(P1,1,0,graph,Net1);
   
    std::cout<<"cost = " <<std::get<1>(result2)<<"\n";
    std::cout<< "point = "<<std::get<2>(result2)<<"\n";


    //L_shape({1,4,1},{2,2,3},graph,Net1); L shape not done now, to Pin net not done

    show_demand(graph);
    // NET_3D net1 = {
    //     {{1,4,1},{1,4,2}},
    //     {{1,4,2},{1,4,3}}, 
    //     {{4,4,1},{4,4,2}}, 
    //     {{4,4,2},{4,4,3}},
    //     {{2,2,1},{2,2,2}},
    //     {{2,2,2},{2,1,2}},
    //     {{2,1,2},{2,1,3}},
    //     {{3,1,2},{3,4,2}},
    //     {{3,4,2},{3,4,3}},
    //     {{3,1,2},{3,1,3}},

    //     {{2,1,3},{3,1,3}},
    //     {{1,4,3},{4,4,3}}
    // };
    // NET_3D net3 = {
    //     {{2,2,1},{2,2,3}},
    //     {{4,2,1},{4,2,3}},
    //     {{2,2,2},{2,5,2}},
    //     {{2,2,3},{4,2,3}}
    // };

    // add_net(graph,net1,0.5,1);
    // add_net(graph,net3,0.5,1);
    
    return 0;
}
