
#ifndef PARS_DS
#define PARS_DS

#include <utility>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>



//parser
std::vector<std::string> split(const std::string&str,char symbol,int l,int r,int num);


struct VoltageArea{
    std::string voltageAreaName;//<voltageArea Name>
    std::vector<std::pair<int,int>>gGridRowIdx;//<gGridColIdx>
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
    VoltageArea* vArea;//null if no VoltageArea
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


MasterCell::MasterCell(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCell)
{
    std::string info;
    std::getline(is,info);
    auto parser = split(info,' ',0,info.size(),4);
    std::string name = parser.at(1);
    std::cout<<name<<"\n";
    int pinNum = std::stoi(parser.at(2));
    int blkNum = std::stoi(parser.at(3));
    mCell.insert({name,this});
    for(int i = 0;i<pinNum;i++)
    {
        std::getline(is,info);
        parser = split(info,' ',0,info.size(),3);
        std::string pinName = parser.at(1);
        std::string Metal = parser.at(2);
        int layer = std::stoi(std::string(Metal.begin()+1,Metal.end()));
        pins.insert({pinName,layer});
        std::cout<<pinName<<" M"<<layer<<" \n";
    }
    for(int i = 0;i<blkNum;i++)
    {
        std::getline(is,info);
        parser = split(info,' ',0,info.size(),4);
        std::string blkName = parser.at(1);
        std::string Metal = parser.at(2);
        int layer = std::stoi(std::string(Metal.begin()+1,Metal.end()));
        int demand = std::stoi(parser.at(3));
        blkgs.insert({blkName,{layer,demand}});
        std::cout<<blkName<<" M"<<layer<<" "<<demand<<" \n";
    }   
}

CellInst::CellInst(std::string&info,std::unordered_map<std::string,MasterCell*>&mCells,std::unordered_map<std::string,CellInst*>&CellInsts)
{
    auto parser = split(info,' ',0,info.size(),6);
    std::string cell_name = parser.at(1);
    mCell = mCells.find(parser.at(2))->second;
    row = std::stoi(parser.at(3));
    col = std::stoi(parser.at(4));
    Movable = (parser.at(5)=="Movable");
    std::cout<<cell_name<<" "<<row<<" "<<col<<" "<<Movable<<"\n";
    CellInsts.insert({cell_name,this});
}

Net::Net(std::ifstream&is,std::unordered_map<std::string,CellInst*>&CellInsts,std::unordered_map<std::string,Net*>&Nets)
{
    std::string info;
    std::getline(is,info);
    auto parser = split(info,' ',0,info.size(),5);
    std::string name = parser.at(1);
    int pinNum = std::stoi(parser.at(2));
    std::string LayerCstr = parser.at(3);
    if(LayerCstr=="NoCstr")
        minLayer = 1;
    else 
        minLayer = std::stoi(std::string(LayerCstr.begin()+1,LayerCstr.end()));
    weight = std::stof(parser.at(4));

    Nets.insert({name,this});

    std::cout<<name<<" "<<pinNum<<" "<<LayerCstr<<" "<<weight<<"\n";
    
    for(int i = 0;i<pinNum;i++)
    {
        std::getline(is,info);
        parser = split(info,' ',0,info.size(),2);
        parser = split(parser.at(1),'/',0,parser.at(1).size(),2);
        std::string cellName = parser.at(0);
        std::string pinName = parser.at(1);
        CellInst* Cell = CellInsts.find(cellName)->second;
        net_pins.push_back({Cell,pinName});
        std::cout<<cellName<<" "<<pinName<<"\n";
    }
}

#endif 
