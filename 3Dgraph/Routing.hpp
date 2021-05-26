#ifndef ROUTING_HPP
#define ROUTING_HPP

#include <float.h> //FLT_MAX

#include "Graph.hpp"
#include "Assign.hpp"

//超參數
//在早期,congestion不嚴重的時候,其實不用每個Layer都check LAYER_SEARCH_RANGE可以比較小,但之後要隨狀況調整
extern float VIA_W;
extern float ESCAPE_W;
extern int LAYER_SEARCH_RANGE;






std::tuple<std::vector<Point>,float,Point> Layer_routing(const Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info);
//------------------------------------------------------------------------Heuristic_routing------------------------------------------------------------------------------
//參數說明:
//P代表的是現在所處的位置點,欲連接到的點則隱含使用(P.x+delta_x,P.y+delta_y)表示
//delta_x,delta_y 一次只能有一者不為0
//graph,net_info用以得到繞線資訊,包含congestion以及min_layer還有net_weight.

//target_layer:
//則是指定目標位置的z座標,default=-1 代表不指定
//當目標點為pin時需要指定target_layer

//Output:
//First : std::vector<Point> 連接到目標點所需經過的至多三個點
//Second : float : 欲繞線至目標點的Cost
//Third : Point : 移動的目標位置(包含z座標)
//Note : 並不包含將demand加入到congestion map上 ,僅回傳路線以及cost.
inline std::tuple<std::vector<Point>,float,Point>  Heuristic_routing(Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info,int target_layer = -1)
{
    if(delta_x!=0 && delta_y!=0)//diagonal
    {
        std::cerr<<"error input in Heuristic_routing! , delta_x!= 0&&delta_y!=0 is an invalid input!\n";
        exit(1);
    }
    if(delta_x==0 && delta_y==0)//no move
    {
        std::cerr<<"error input in Heuristic_routing! , delta_x==0 &&delta_y==0 is an invalid input!\n";
        exit(1);
    }

    // if(target_layer!=-1) // target is pin
    // {
    //     //
    //     ;
    // }
    // else
    return Layer_routing(P,delta_x,delta_y,graph,net_info);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------




std::pair<bool,Point> L_shape(const Point &P1,const Point &P2,Graph&graph,Net_routing_Info& net_info);





/*
---------------------------------------------------------------------------Helper function---------------------------------------------------------------------------
*/


//--------------------------------------------------------------------------Point_is_Valid------------------------------------------------------------------------------
//用來檢查給定的point是否在合法範圍
inline bool Point_is_Valid(int x,int y,int z,const Graph&graph,int min_Layer)
{
    int bound_x = graph.Grid_size().first;
    int bound_y = graph.Grid_size().second;
    int max_L = graph.Layer_Num();
    if(x<1||y<1||z<min_Layer)return false;
    if(x>bound_x||y>bound_y||z>max_L)return false;
    return true;
}
inline bool Point_is_Valid(const Point &P,const Graph&graph,int min_Layer)
{
    return Point_is_Valid(P.x,P.y,P.z,graph,min_Layer);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------



/*
cost function使用前必須先透過is_congestion判斷是否能移動至該位置
*/
//--------------------------------------------------------------------------routing_cost------------------------------------------------------------------------------
// 用來計算(水平或垂直)移動至目標點x,y,z的cost，會包含net_weight*powerfactor,以及Escape rate
//first : congestion or not
//second :cost
std::pair<bool,float> routing_cost(int x,int y,int z,Graph&graph,Net_routing_Info&net_info);
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------VIA_cost----------------------------------------------------------------------------------
//計算透過VIA移動至目標點x,y,z的cost , 包含net_weight*powerfactor以及VIA_weight
//first : congestion or not
//second : cost
inline std::pair<bool,float> VIA_cost(int x,int y,int z,Graph&graph,Net_routing_Info&net_info){
        if(!Point_is_Valid(x,y,z,graph,net_info.min_Layer))
        {
            return {true,FLT_MAX};
        }
        auto cong = is_congestion(x,y,z,graph,net_info);
        if(cong.first==true) return {true,FLT_MAX};//congestion
        if(cong.second==true) return {false,net_info.weight* graph[z].get_pf() + VIA_W};//need one demand
        return {false,0};//do not need demand
}
inline std::pair<bool,float> VIA_cost(Point P,Graph&graph,Net_routing_Info&net_info){return VIA_cost(P.x,P.y,P.z,graph,net_info);}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------


#endif

