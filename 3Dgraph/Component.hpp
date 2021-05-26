#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <map>



//-----------------------------------------------------Point-----------------------------------------------------------
struct Point{
    Point(int x_val,int y_val,int z_val = 1)
        :x{x_val},y{y_val},z{z_val}{}
    int x,y,z;
};

inline bool operator<(const Point&p1,const Point&p2)
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
inline int Manhattan_distance_2D(const Point&P1,const Point&P2){return std::abs(P1.x-P2.x) + std::abs(P1.y-P2.y);}




struct segment{
    Point p1,p2;
};
using NET_3D = std::vector<segment>;


//---------------------------------------Net_routing_Info--------------------------------------------

//功能:
//1. 紀錄min_layer以及weight等Net性質
//2. 用來記錄這條NET通過的Ggrid座標,用以判斷是否需要demand.
//到時候rip-up reroute也要從這裡拆掉
struct Net_routing_Info
{
    Net_routing_Info(int min_l,float w)
        :min_Layer{min_l},weight{w}{}
    int min_Layer;//This Net's min_Layer constraint.
    std::map<Point,bool>Pass_grids;//The Grids that this Net already pass through.
    float weight;

    bool already_in(int x,int y,int z){return Pass_grids.find({x,y,z})!=Pass_grids.end();}
    bool not_in(int x,int y,int z){return !already_in(x,y,z);}
};




#endif 
