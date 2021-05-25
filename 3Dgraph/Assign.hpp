
#ifndef ASSIGN_HPP
#define ASSIGN_HPP

#include "Graph.hpp"


//功能描述:
//跟Demand分配有關的查詢跟紀錄都由此標頭檔案定義
//包含is_congestion,add_segment

inline std::pair<bool,bool> is_congestion(int x,int y,int z,Graph&graph,Net_routing_Info&net_info)
{
    if( net_info.not_in(x,y,z) )//need one more demand
        return {graph(x,y,z).capacity == graph(x,y,z).demand,true};
    else 
        return {false,false};//do not need one more demand
}

//P1,P2 only differ by one dir.
void add_segment_3D(Point&P1,Point&P2,Graph&graph,Net_routing_Info&net_info);
void add_net(Graph&graph,NET_3D&net,float weight,int min_layer);

#endif
