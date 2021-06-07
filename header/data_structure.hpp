#ifndef _DATA_STRUCTURE_HPP_
#define _DATA_STRUCTURE_HPP_
#define PARSER_TEST

#include <utility>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>

std::vector<std::string> split(const std::string&str,char symbol,int l,int r,int num);


struct Layer{
    Layer(const std::string& RoutDir = "H",int defaultSupply = 0,float pf = 0)
        : powerFactor{pf},supply{defaultSupply},horizontal{(RoutDir=="H")}{}
    bool horizontal;//<RoutingDir>
    int supply;//<defaultSupplyOfOneGGrid>
    float powerFactor;//<powerFactor>
};


struct MasterCell
{
    MasterCell(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCell);
    using Blkg = std::pair<int,int>;//(Layer index,demand)
    using Pin = int;//Layer
    std::unordered_map<std::string,Pin>pins;//PinName : Pin
    std::unordered_map<std::string,Blkg>blkgs;//BlkgName : Blkg
};

struct CellInst{
    CellInst(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCells,std::unordered_map<std::string,CellInst*>&CellInsts);
    MasterCell* mCell;
    int row;//<gGridRowIdx>
    int col;//<gGridColIdx>
    bool Movable;//<movableCstr>
    //VoltageArea* vArea;//null if no VoltageArea
    int vArea;
};



struct Net{
    std::string netName;//<netName>
    Net(std::ifstream&is,std::unordered_map<std::string,CellInst*>&CellInsts,std::unordered_map<std::string,Net*>&Nets);
    int minLayer;//<minRoutingLayConstraint>
    float weight;//<weight>
    using PIN = std::pair<CellInst*,std::string>;
    //記下CellInst*是為了以後移動可以得到更新的座標 x,y
    //std::string代表Pin的name,用來CellInst內查找Pin
    std::vector<PIN> net_pins;
};

struct Ggrid{
    Ggrid(): demand(0) {}
    Ggrid(int c) : capacity(c), demand(0) {}
    int get_remaining(void)const {return capacity - demand;}
    float congestion_rate()const{
        if(capacity==0)return 1; //以免 / 0 error.
        return (float)demand/capacity;
    }
    void add_demand(){demand+=1;}
    void PassingByNet(int NetId)
    {
        if(NotIn(NetId)){
            PassingNets.insert({NetId,true});
            add_demand();
        }
    }
    bool AlreadyIn(int NetId){return PassingNets.find(NetId) != PassingNets.end();}
    bool NotIn(int NetId){return !AlreadyIn(NetId);}

//----------------------------------Data Member----------------------------------------------
    int capacity;
    int demand;
    std::unordered_map<int, bool> PassingNets;
    //key : netId,val : true always.


    //maybeFutre
    //key: netId, value: sum of the number of pins and the number of segments in this Gcell for this net
};



//Others

struct Point{
    Point(int row_val,int col_val,int lay_val = 1)
        :row{row_val},col{col_val},lay{lay_val}{}
    int row,col,lay;
};

#endif
