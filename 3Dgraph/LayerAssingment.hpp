#ifndef LA_HPP
#define LA_HPP

#include <vector>
#include <queue>
#include <utility>
#include <iostream>
//Net_3D : Including segments.
//segment : Including two terminals from grid i to grid j
//Ggrid j always need one demand because it is "IN" direction.
//Ggrid i need one demand iff i is the starting point of the net_3D.


struct Point{
    int x,y,z = 0;
};

struct segment{
    Point p1,p2;
};
using NET_3D = std::vector<segment>;


struct Ggrid{
    Ggrid(int supply,int dmnd = 0):capacity{supply},demand{dmnd}{}
    int capacity;
    int demand;
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

class Graph{
public:
    Graph(std::vector<Layer>L)
        :Layers{L}
        {

        }
    void Non_default_grid(int x,int y,int z,int offset){(*this)(x,y,z).adjust_cap(offset);}
    Ggrid& operator()(int x,int y,int z){
        if(z<1)
        {
            std::cerr<<"Error : Layer index z must larger than 1!\n";
            exit(1);
        }
        return (*this)[z](x,y);
    }
    int Layer_Num()const{return Layers.size();}
    std::pair<int,int>Grid_size()const{return Layers.at(0).Grid_size();}
    Layer& operator[](int i){return Layers.at(i-1);}
private:
    std::vector<Layer>Layers;
};
//P1,P2 only differ by one dir.
void add_segment(Graph&graph,Point&P1,Point&P2);//call by add_net_3D
void add_net_3D(Graph&graph,NET_3D&net_3D);

#endif
