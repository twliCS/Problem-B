#ifndef LA_HPP
#define LA_HPP

#include <vector>
#include <queue>


//Net_3D : Including segment_3Ds.
//segment_3D : Including two terminals from grid i to grid j
//Ggrid j always need one demand because it is "IN" direction.
//Ggrid i need one demand iff i is the starting point of the net_3D.


struct Point_3D{
    int x,y,z;
};
struct segment_3D{
    Point_3D p1,p2;
};
using NET_3D = std::vector<segment_3D>;
struct Point_2D{
        int x,y;
};
struct Ggrid{
    Ggrid(int supply,int dmnd = 0):capacity{supply},demand{dmnd}{}
    int capacity;
    int demand;
};
class Layer{
public:
    Layer(const Point_2D &p1,const Point_2D &p2,bool H,int pf,int defaultSupply)
        : is_H{H},power_factor{pf},defaultSupplyOfOneGGrid{defaultSupply}
    {
        //assume p1 is left bottom,p2 is right top.
        int X_grid = p2.x - p1.x + 1;
        int Y_grid = p2.y - p1.y + 1;
        Ggrids = Ggrid_2D(X_grid,Ggrid_1D(Y_grid,defaultSupplyOfOneGGrid));
    }
    void Non_default_grid(int x,int y,int offset){Ggrids[y-1][x-1].capacity+=offset;}
    void add_demand(int x,int y){Ggrids[y][x].demand+=1;}
    void show_congestion()const;
    void show_demand()const;
private:
    using Ggrid_1D = std::vector<Ggrid>;
    using Ggrid_2D = std::vector<Ggrid_1D>;
    bool is_H;//Horizontal routing direction.
    int power_factor;
    int defaultSupplyOfOneGGrid;
    Ggrid_2D Ggrids;//[y][x]
};

using Graph_3D = std::vector<Layer>;


//P1,P2 only differ by one dir.
void add_segment_3D(Graph_3D&graph,Point_3D&P1,Point_3D&P2);//call by add_net_3D
void add_net_3D(Graph_3D&graph,NET_3D&net_3D);


#endif
