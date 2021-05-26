#include "Routing.hpp"
#include <map>
#include <forward_list>
#include <iostream>
#include <utility>
#include <tuple>





//some error必須修正
//escape rate 要考慮來的人是誰
std::pair<bool,float> routing_cost(int x,int y,int z,Graph&graph,Net_routing_Info&net_info)
{
    if(!Point_is_Valid(x,y,z,graph,net_info.min_Layer))
        return {false,FLT_MAX};
    auto check = is_congestion(x,y,z,graph,net_info);
    if(check.first==true){return {true,INT_MAX};}//若無法到目標點X,Y,Z直接回傳congestion

    float net_cost = (check.second==true) ? net_info.weight * graph[z].get_pf() : 0;//是否需要額外demand才能到x,y,z

    
    //計算escape_rate

    float nc = 0;//neighbor_congestion , the higher congestion it is,the lower escape_rate it is.


    //neighborhood congestion caculate function.
    //如果neighbor已經屬於net一部分,則不用考慮他的congestion,因為一定能過去
    //這部分ESCAPE_W必須調整的好,否則就算無法到達neighbor也無法計算出來 : congestion_rate : 0~1
    auto nc_cac = [&graph,&net_info](float& nc,int x,int y,int z)
    {
        
        if(Point_is_Valid({x,y,z},graph,net_info.min_Layer) && net_info.not_in(x,y,z)) {
            nc += graph(x,y,z).congestion_rate(); //congestion_rate : 0~1
        }
    };
    if(z % 2 == 1){//this Layer is H direction
        nc_cac(nc,x+1,y,z);
        nc_cac(nc,x-1,y,z);
    }
    else{//this Layer is V direction
        nc_cac(nc,x,y+1,z);
        nc_cac(nc,x,y-1,z);
    }
    nc_cac(nc,x,y,z-1);
    nc_cac(nc,x,y,z+1);
    return {false,net_cost + ESCAPE_W * nc};//回傳congestion=false, cost = net_cost + ESCAPE_W * nc
}





struct path_node //用來記錄經過的gGrid座標
{
    Point p;
    float cost;
    path_node * parent;
};

//Node generator 用於在Heuristic_routing查找其他Layer的點
class node_gen
{
public:
    node_gen(Graph&graph,Net_routing_Info& net_info,Point init_Point,int delta)
        :gr{&graph},info{&net_info},_delta{delta}
    {
        next_point = new path_node{init_Point,0,nullptr};
    }
    void init(Graph&graph,Net_routing_Info& net_info,Point init_Point,int delta)
    {
        gr = &graph;
        info = &net_info;
        _delta = delta;
        next_point = new path_node{init_Point,0,nullptr};
    }
    path_node* operator()()
    {
        if(!next_point)
        {
            std::cerr<<"must init node generator again!!! \n";
            return nullptr;
        }
        int x = next_point->p.x;
        int y = next_point->p.y;
        int z = next_point->p.z;

        auto via_cost = VIA_cost(x,y,z,*gr,*info);
        if(via_cost.first==true)//congestion
            return nullptr;
        
        path_node* u = new path_node{{x,y,z+_delta},next_point->cost+via_cost.second,next_point};
        path_node*ret = next_point;
        next_point = u;
        return ret;
    }
private:
    int _delta;
    path_node *next_point;
    Graph* gr;
    Net_routing_Info * info;    
};

std::tuple<std::vector<Point>,float,Point> Layer_routing(const Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info)
{

    //試著由壓縮圖上的(P.x,P.y)移動到(P.x+delta_x,P.y+delta_y)
    auto Try_ROUT = [&graph,&net_info](int x,int y,int z,path_node*parent,float &min_c,path_node*&min_path){
        auto result = routing_cost(x,y,z,graph,net_info);
        if(!result.first){//目標點沒有congestion
            float cost = result.second + parent->cost;
            if(cost < min_c)
            {
                if(min_path!=nullptr){delete min_path;}
                min_path = new path_node{{x,y,z},cost,parent};//新的最佳解
                min_c = cost;
            }
        }
    };


    //移動Layer時試著進行Rouring到target : (x+delta_x,y+delta_y,z)
    auto Layer_ROUT = [&Try_ROUT](path_node*v,int delta_x,int delta_y,float&min_c,path_node*&min_path)
    {
        int l = v->p.z;
        if(l%2==1&&delta_x!=0)//This Layer now is H , and the target dir is H too.
            Try_ROUT(v->p.x+delta_x,v->p.y,l,v,min_c,min_path);
        if(l%2==0&&delta_y!=0)//This Layer now is V , and the target dir is V too.
            Try_ROUT(v->p.x,v->p.y+delta_y,l,v,min_c,min_path);
    };

   
    //---------------------------------------------------------------參數設定-----------------------------------------------------

    //Scan Layer range up / down
    int upper_layer = std::min(P.z + LAYER_SEARCH_RANGE, graph.Layer_Num()); 
    int lower_layer = std::max(P.z - LAYER_SEARCH_RANGE, net_info.min_Layer);

    

    float min_cost = FLT_MAX;
    path_node *min_cost_path = nullptr;//if no path,then return false
    
    node_gen GEN(graph,net_info,P,1);//初始化,delta設為1:往上
    std::forward_list<path_node*>recycle;//記憶體回收器
    path_node*v;
     
    //--------------------------------------------------------------Layer_ROUT---------------------------------------------------
    while(( v = GEN()) && (v->p.z <= upper_layer) )//move upper
    {
        Layer_ROUT(v,delta_x,delta_y,min_cost,min_cost_path);
        recycle.push_front(v);
        
    }
    
    GEN.init(graph,net_info,P,-1);//設定向下
    while((v = GEN() )&& (v->p.z >= lower_layer))//move lower
    {
        Layer_ROUT(v,delta_x,delta_y,min_cost,min_cost_path);
        recycle.push_front(v);
    }
    
    
    //-------------------------------------------------------------儲存最佳解------------------------------------------------------
    if(min_cost_path==nullptr){return {{},FLT_MAX,P};}


    //紀錄該路徑的三個點 一條z方向,一條V或H 即可用來還原路線
    std::vector<Point>PATH;
    PATH.push_back(min_cost_path->p);//目標點  PATH.at(0)  ,若外部要使用,則需要更新
    PATH.push_back(min_cost_path->parent->p);//目標Layer
    PATH.push_back(P);//起始點

    recycle.clear();//回收

    return {PATH,min_cost,PATH.at(0)};
}






//P1 P2 are " pins "
//search the L_shape path from p1 to p2.
//first : can reach P2
//second : P2 final coordinate
std::pair<bool,Point> L_shape(const Point &P1,const Point &P2,Graph&graph,Net_routing_Info& net_info)
{

    if(!Point_is_Valid(P1,graph,net_info.min_Layer) || !Point_is_Valid(P2,graph,net_info.min_Layer))
    {
        std::cerr<<"error Point !!!\n";
        exit(1);
    }


    int delta_x = (P1.x < P2.x) ? 1 : -1;
    int delta_y = (P1.y < P2.y) ? 1 : -1;
   
    Point Path1{P1}; 
    float path1_cost = 0; //path's total cost
    std::vector<std::vector<Point>>PATH1;//save segment of each move.
    Point Path2{P1};
    float path2_cost = 0;
    std::vector<std::vector<Point>>PATH2;


    auto estimate_one_move = [&graph,&net_info](Point&cur,const Point&target,int delta_x,int delta_y)
    {
        
        if(Manhattan_distance_2D(cur,target) != 1){//target is not P2.
           return Heuristic_routing(cur,delta_x,delta_y,graph,net_info,-1);
        }
        else{
            return Heuristic_routing(cur,delta_x,delta_y,graph,net_info,target.z);//assign to Layer : target.z
        }
    };
    //straight_move是一個使用estimate_one_move的範例
    auto straight_move = [&estimate_one_move](Point&cur_p,int &cur,const Point& target_p,int target,int dx,int dy,std::vector<std::vector<Point>>&PATH)
    {
        bool can_reach = true;
        float cost = 0;
        while( (cur!=target) && (can_reach==true) )
        {

            auto move_result = estimate_one_move(cur_p,target_p,dx,dy);
            
            auto one_move_path = std::get<0>(move_result);
            if(!one_move_path.empty())
            {
                PATH.push_back(one_move_path);
                cost += std::get<1>(move_result);//取得cost
                cur_p = std::get<2>(move_result);//移動至目標點
            }
            else 
                can_reach = false;
        }
        return std::make_tuple(can_reach,cost);
    };


    //------------------------------------Path1 First X then Y----------------------------------------------
    bool path1_can_reach = false;
    float cost;
    std::tie(path1_can_reach,cost) = straight_move(Path1,Path1.x,P2,P2.x,delta_x,0,PATH1);//Move x direction
    if(path1_can_reach)
    {
        path1_cost+=cost;
        std::tie(path1_can_reach,cost) = straight_move(Path1,Path1.y,P2,P2.y,0,delta_y,PATH1);//Move y direction
    }
    if(path1_can_reach){
        path1_cost+=cost;
    }
    //------------------------------------Path2 First Y then X----------------------------------------------
    bool path2_can_reach = false;
    std::tie(path2_can_reach,cost) = straight_move(Path2,Path2.y,P2,P2.y,0,delta_y,PATH2);//Move y direction
    if(path2_can_reach){
        path2_cost+=cost;
        straight_move(Path2,Path2.x,P2,P2.x,delta_x,0,PATH2);//Move x direction
    }
    if(path2_can_reach){
        path2_cost+=cost;
    }
    //------------------------------------Selection Least Cost path----------------------------------------


    if(!path2_can_reach&&!path1_can_reach)
        return {false,P2};

    std::vector<std::vector<Point>>* BEST_PATH = 
        (path1_can_reach && path2_can_reach) ? ( (path1_cost < path2_cost) ? &PATH1 : &PATH2  ) 
            : ( (path1_can_reach) ? &PATH1 : &PATH2 );
    
    
    //--------------------------------------真實加入Demand-------------------------------------------------
    for(auto &mov:*BEST_PATH)
    {
        for(int i = 1;i<mov.size();i++)
        {
            add_segment_3D(mov.at(i-1),mov.at(i),graph,net_info);
        }
    }

    Point P2_final = BEST_PATH->at(BEST_PATH->size()-1).at(0);
    return {true,P2_final};
}
