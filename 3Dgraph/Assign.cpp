#include "Assign.hpp"

//P1,P2 only differ by one dir.
//但距離可以大於1
void add_segment_3D(Point&P1,Point&P2,Graph&graph,Net_routing_Info&net_info)//don not need consider starting point.
{
    int x = P1.x,t_x = P2.x;
    int y = P1.y,t_y = P2.y;
    int z = P1.z,t_z = P2.z;
    int delta_x = (x!=t_x)? ((x < t_x)? 1:-1):0;
    int delta_y = (y!=t_y)? ((y < t_y)? 1:-1):0;
    int delta_z = (z!=t_z)? ((z < t_z)? 1:-1):0;

    auto record_demand = [&](int x,int y,int z){
        if(net_info.not_in(x,y,z)){
            net_info.Pass_grids.insert({{x,y,z},true});
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

void add_net(Graph&graph,NET_3D&net,float weight,int min_layer)//need consider starting point.
{
    Point start_Ggrid = net.at(0).p1;
    Net_routing_Info net_info;//紀錄以免重算demand
    net_info.weight = weight;
    net_info.min_Layer = min_layer;
    for(auto s:net)//for each segment_3D
    {
        add_segment_3D(s.p1,s.p2,graph,net_info);
    }
    std::cout<<"this net's WL = "<<net_info.Pass_grids.size()<<"\n";
}
