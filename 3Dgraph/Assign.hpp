
#ifndef ASSIGN_HPP
#define ASSIGN_HPP

#include "Graph.hpp"


//功能描述:
//跟Demand分配有關的查詢跟紀錄都由此標頭檔案定義
//包含is_congestion,add_segment



//------------------------------------------------------------------------is_congestion------------------------------------------------------------------------------
//功能:
//1. 繞線至目標點(x,y,z)是否需要額外demand (若已經屬於net的一部分則不需要)
//2. 若需要額外demand,則判斷是否congestion
//first : is congestion or not 
//second : need demand or not 
inline std::pair<bool,bool> is_congestion(int x,int y,int z,Graph&graph,Net_routing_Info&net_info)
{
    if( net_info.not_in(x,y,z) )//need one more demand
        return {graph(x,y,z).capacity == graph(x,y,z).demand,true};
    else 
        return {false,false};//do not need one more demand
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------


//P1,P2 only differ by one dir.
void add_segment_3D(Point&P1,Point&P2,Graph&graph,Net_routing_Info&net_info);
// void add_net(Graph&graph,NET_3D&net,float weight,int min_layer);

#endif
