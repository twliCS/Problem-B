#include "Routing.hpp"
#include <map>
#include <forward_list>
#include <iostream>
#include <utility>
#include <tuple>



//繞線COST超參數
float VIA_W = 1;
float ESCAPE_W = 1;
int LAYER_SEARCH_RANGE = 1;

bool Point_is_Valid(const Point &P,const Graph&graph,int min_Layer);//用來檢查給定的point是否在合法範圍
float routing_cost(int x,int y,int z,Graph&graph,Net_routing_Info&net_info);// 用來計算由到目標點x,y,z的cost 會包含net_weight*powerfactor,以及逃脫率
float VIA_cost(int x,int y,int z,Graph&graph,Net_routing_Info&net_info){return net_info.weight* graph[z].get_pf() + VIA_W;}
float VIA_cost(Point P,Graph&graph,Net_routing_Info&net_info){return VIA_cost(P.x,P.y,P.z,graph,net_info);}


//Routing
std::pair<std::vector<Point>,float> Layer_routing(Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info);



bool Point_is_Valid(const Point &P,const Graph&graph,int min_Layer)
{
    int bound_x = graph.Grid_size().first;
    int bound_y = graph.Grid_size().second;
    int max_L = graph.Layer_Num();
    if(P.x<1||P.y<1||P.z<min_Layer)return false;
    if(P.x>bound_x||P.y>bound_y||P.z>max_L)return false;
    return true;
}
float routing_cost(int x,int y,int z,Graph&graph,Net_routing_Info&net_info)
{
    float nc = 0;//neighbor_congestion , the higher congestion it is,the lower escape_rate it is.

    int max_x = graph.Grid_size().first;
    int max_y = graph.Grid_size().second;
    int min_L = net_info.min_Layer;
    int max_L = graph.Layer_Num();

    //neighborhood congestion caculate function.
    //如果neighbor已經屬於net一部分,則不用考慮他的congestion,因為一定能過去
    auto nc_cac = [&graph,&net_info,max_x,max_y,max_L,min_L](float& nc,int x,int y,int z)
    {
        if(Point_is_Valid({x,y,z},graph,net_info.min_Layer) && net_info.not_in(x,y,z) ) 
            nc += graph(x,y,z).congestion_rate(); 
    };
    if(z % 2 == 1){//this Layer is H direction
        nc_cac(nc,x+1,y,z);
        nc_cac(nc,x-1,y,z);
    }
    else{//this Layer is V direction
        nc_cac(nc,x,y+1,z);
        nc_cac(nc,x,y-1,z);
    }
    //VIA direction
    nc_cac(nc,x,y,z+1);
    nc_cac(nc,x,y,z-1);
    return net_info.weight * graph[z].get_pf() + ESCAPE_W * nc;
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
        auto cong = is_congestion(x,y,z,*gr,*info); //if x,y,z is congestion : return nullptr. 
        if(cong.first==true)return nullptr;
        path_node * u = new path_node{{x,y,z+_delta},next_point->cost,next_point};
        if(cong.second==true)
                u->cost += VIA_cost(u->p,*gr,*info);
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


std::pair<std::vector<Point>,float> Layer_routing(Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info)
{

    //lambda : 用來簡化參數
    auto IS_CON = [&graph,&net_info](int x,int y,int z){return is_congestion(x,y,z,graph,net_info);};
    auto ROUT_COST = [&graph,&net_info](int x,int y,int z){return routing_cost(x,y,z,graph,net_info);};

    auto Try_ROUT = [&graph,&net_info,&IS_CON,&ROUT_COST](int x,int y,int z,path_node*parent,float &min_c,path_node*&min_path){
        auto cong = IS_CON(x,y,z);
        if(!cong.first){//目標點沒有congestion
            bool need_dmd = cong.second;
            float cost = parent->cost + ( (need_dmd) ? ROUT_COST(x,y,z) : 0);
            if(cost < min_c)
            {
                delete min_path;
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


    float min_cost = INT_MAX;
    path_node *min_cost_path = nullptr;//if no path,then return false
    
    node_gen GEN(graph,net_info,P,1);//初始化,delta設為1:往上
    std::forward_list<path_node*>recycle;//記憶體回收器
    path_node*v;

    //--------------------------------------------------------------Layer_ROUT---------------------------------------------------
    while(( v = GEN()) && (v->p.z<=upper_layer) )//move upper
    {
        Layer_ROUT(v,delta_x,delta_y,min_cost,min_cost_path);
        recycle.push_front(v);
    }
    GEN.init(graph,net_info,P,-1);
    while((v = GEN() )&& (v->p.z>=lower_layer) )//move lower
    {
        Layer_ROUT(v,delta_x,delta_y,min_cost,min_cost_path);
        recycle.push_front(v);
    }
    
    //-------------------------------------------------------------儲存最佳解------------------------------------------------------
    if(min_cost_path==nullptr){return {{},INT_MAX};}


    //紀錄該路徑的三個點 一條z方向,一條V或H 即可用來還原路線
    std::vector<Point>PATH;
    PATH.push_back(min_cost_path->p);//目標點
    PATH.push_back(min_cost_path->parent->p);//目標Layer
    PATH.push_back(P);//起始點

    P = min_cost_path->p;//更新座標(到時候改掉)
    recycle.clear();//回收

    return {PATH,min_cost};
}



std::pair<std::vector<Point>,float> Heuristic_routing(Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info,int target_layer)
{
    // if(target_layer!=-1) // target is pin
    //     return Monotonic_one_mov();


    return Layer_routing(P,delta_x,delta_y,graph,net_info);
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

    //L_shape 特有的
    auto one_move = [&graph,&net_info](Point&cur,const Point&target,int delta_x,int delta_y)
    {
        if(Manhattan_distance_2D(cur,target) != 1){//target is not P2.
           return Heuristic_routing(cur,delta_x,delta_y,graph,net_info,-1);
        }
        else{
            return Heuristic_routing(cur,delta_x,delta_y,graph,net_info,target.z);//assign to Layer : target.z
        }
    };

    auto straight_move = [&one_move](Point&cur_p,int &cur,const Point& target_p,int target,int dx,int dy,std::vector<std::vector<Point>>&PATH)
    {
        bool can_reach = true;
        float cost = 0;
        while( (cur!=target) && (can_reach==true) )
        {
            auto move_result = one_move(cur_p,target_p,dx,dy);

            if(!move_result.first.empty())
            {
                PATH.push_back(move_result.first);
                cost += move_result.second;
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






int main()
{
    


    return 0;
}
