#ifndef _DATA_STRUCTURE_HPP_
#define _DATA_STRUCTURE_HPP_

#include <utility>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>

std::vector<std::string> split(const std::string&str,char symbol,int l,int r,int num);

/*
struct VoltageArea{
    std::string voltageAreaName;//<voltageArea Name>
    std::vector<std::pair<int,int>>gGridRowIdx;//<gGridColIdx>
};
*/

struct Layer{
    Layer(std::ifstream&is,std::vector<Layer*>&layers);
    //std::string layerName;//<layerName>
    //int index;//<Idx>
    bool vertical;//<RoutingDir>
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
    CellInst(std::string&info,std::unordered_map<std::string,MasterCell*>&mCells,std::unordered_map<std::string,CellInst*>&CellInsts);
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
    int get_remaining(void) {return capacity - demand;}
    int capacity;
    int demand;
    std::unordered_map<int, int> passingNets; //key: netId, value: sum of the number of pins and the number of segments in this Gcell for this net
};

#endif
