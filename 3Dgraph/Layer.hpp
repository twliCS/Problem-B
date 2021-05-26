#ifndef LAYER_HPP
#define LAYER_HPP

#include "Component.hpp"
#include <iostream>


struct Ggrid{
    Ggrid(int supply,int dmnd = 0):capacity{supply},demand{dmnd}{}
    int capacity;
    int demand;
    float congestion_rate()const{
        if(capacity==0)return 1; //以免 / 0 error.
        return demand/capacity;
    }
    void add_demand(int d = 1){demand+=d;}
    void adjust_cap(int offset){capacity+=offset;}
};

class Layer{
public:
    Layer(const Point &p1,const Point &p2,bool H,float pf,int defaultSupply)
        : is_H{H},power_factor{pf},defaultSupplyOfOneGGrid{defaultSupply}
    {
        //assume p1 is left bottom,p2 is right top.
        X_grids = p2.x - p1.x + 1;
        Y_grids = p2.y - p1.y + 1;
        Ggrids = Ggrid_2D(Y_grids,Ggrid_1D(X_grids,defaultSupplyOfOneGGrid));
    }
    Ggrid& operator()(int x,int y){
        if(x<1)
        {
            std::cerr<<"Error : Layer index x must larger than 1!\n";
            exit(1);
        }    
        if(y<1)
        {
            std::cerr<<"Error : Layer index y must larger than 1!\n";
            exit(1);
        }
        return (*this)[y][x-1];
    }
    float get_pf()const{return power_factor;}
    std::pair<int,int>Grid_size()const{return {X_grids,Y_grids};}
    bool dir_is_H()const{return is_H;}
private:
    using Ggrid_1D = std::vector<Ggrid>;
    using Ggrid_2D = std::vector<Ggrid_1D>;
    Ggrid_1D& operator[](int i){return Ggrids.at(i-1);}
    bool is_H;//Horizontal routing direction.
    float power_factor;
    int defaultSupplyOfOneGGrid;
    Ggrid_2D Ggrids;//[y][x]
    int X_grids;
    int Y_grids;
};
#endif
