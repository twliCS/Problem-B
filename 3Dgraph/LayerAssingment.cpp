#include "LayerAssingment.hpp"
#include <iostream>
#include <map>
#include <string>

std::ostream& operator<<(std::ostream&os,const Point&p)
{
    os<<"("<<p.x<<","<<p.y<<","<<p.z<<")";
    return os;
}
//---------------------------------------------------------------------------Layer Assignment Function--------------------------------------------------------
bool operator<(const Point&p1,const Point&p2)
{
    if(p1.x<p2.x)return true;
    else if(p1.x==p2.x)
    {
        if(p1.y<p2.y)return true;
        else if(p1.y==p2.y)
        {
            if(p1.z<p2.z)return true;
        }
    }
    return false;
}

//P1,P2 only differ by one dir.
void add_segment_3D(Graph&graph,Point&P1,Point&P2,std::map<Point,bool>&record)//don not need consider starting point.
{
    int x = P1.x,t_x = P2.x;
    int y = P1.y,t_y = P2.y;
    int z = P1.z,t_z = P2.z;
    int delta_x = (x!=t_x)? ((x < t_x)? 1:-1):0;
    int delta_y = (y!=t_y)? ((y < t_y)? 1:-1):0;
    int delta_z = (z!=t_z)? ((z < t_z)? 1:-1):0;

    auto record_demand = [&](int x,int y,int z){
        if(record.find({x,y,z})==record.end()){
            record.insert({{x,y,z},true});
            graph(x,y,z).add_demand();
        }
    };
    while(x!=t_x||y!=t_y||z!=t_z)
    {
        record_demand(x,y,z);
        x+=delta_x;
        y+=delta_y;
        z+=delta_z;
    }
    record_demand(x,y,z);//(x,y,z) = (t_x,t_y,t_z).
}

void add_net(Graph&graph,NET_3D&net)//need consider starting point.
{
    Point start_Ggrid = net.at(0).p1;
    std::map<Point,bool>record;//紀錄以免重算demand
    for(auto s:net)//for each segment_3D
    {
        add_segment_3D(graph,s.p1,s.p2,record);
    }
    std::cout<<"this net's WL = "<<record.size()<<"\n";
}



struct Queue_node{
    Point p;
    float cost;         //cost = Sigma (PowerFactor(i)*GgridLength(i)) 
    int wl;
    std::string path;
};
class min_cost{
public:
    bool operator()(const Queue_node&n1,const Queue_node&n2){return n1.cost > n2.cost;}//cost越大越下面
};
using Priority_Q = std::priority_queue<Queue_node,std::vector<Queue_node>,min_cost>;

//lay : z
//dir : x or y

//功能:
//1.更新COST_TABLE
//2.放入Priority_Q



// NET_3D LayerAssignment(NET_3D&sol2D,Graph&graph)//Assin one Net from 2D_sol to 3D Graph.
// {


//     //Find Feasible path 
//     std::map<Point,bool>record;//one net one Record
//     for(auto &seg:sol2D)//segment (p1.x,p1.y) and (p2.x,p2.y) only diff by one dir.
//     {
//         if(seg.p1.x!=seg.p2.x || seg.p1.y!=seg.p2.y)
//         {
//             //int min_layer = minlayer[];
//             // if(seg.p1.x!=seg.p2.x)
//             //     Dijkstra_x();
//             // else 
//             //     Dijkstra_y();
//         }
//     }
//     //add_net_3D();
// }


//---------------------------------------------------------------------------Layer Assignment Function--------------------------------------------------------

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

int main()
{
    Point P1{1,1,0};
    Point P2{5,5,0};
    Graph graph = std::vector<Layer>{//M1,M2,M3
        Layer(P1,P2,true,0.1,10),   //H
        Layer(P1,P2,false,0.5,10),  //V 
        Layer(P1,P2,true,1.5,1),   //H
    };

    NET_3D net1 = {
        {{1,4,1},{1,4,2}},
        {{1,4,2},{1,4,3}}, 
        {{4,4,1},{4,4,2}}, 
        {{4,4,2},{4,4,3}},
        {{2,2,1},{2,2,2}},
        {{2,2,2},{2,1,2}},
        {{2,1,2},{2,1,3}},
        {{3,1,2},{3,4,2}},
        {{3,4,2},{3,4,3}},
        {{3,1,2},{3,1,3}},

        {{2,1,3},{3,1,3}},
        {{1,4,3},{4,4,3}}
    };
    NET_3D net3 = {
        {{2,2,1},{2,2,3}},
        {{4,2,1},{4,2,3}},
        {{2,2,2},{2,5,2}},
        {{2,2,3},{4,2,3}}
    };
    add_net(graph,net1);
    show_demand(graph);
    add_net(graph,net3);
    show_demand(graph);



    
    return 0;
}
