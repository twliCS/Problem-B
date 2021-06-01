#include "data_structure.hpp"

//---------------------Layer---------------------
Layer::Layer(std::ifstream&is,std::vector<Layer*>&layers){
    std::string type;
    int index;

    //ex: Lay M1 1 H 10 1.2
    is >> type >> type >> index >> type >> supply >> powerFactor;
    if(type=="V")
        vertical=true;
    else
        vertical=false;
    layers.at(index-1) = this;
    std::cout<<this->vertical<<' '<< this->supply <<' '<<this->powerFactor<<std::endl;
};

//---------------------MasterCell---------------------
MasterCell::MasterCell(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCell)
{
    std::string info;

    std::getline(is,info);
    std::cout<<info<<std::endl;
    auto parser = split(info,' ',0,info.size(),4);
    std::string name = parser.at(1);
    std::cout<<name<<"\n";
    int pinNum = std::stoi(parser.at(2));
    int blkNum = std::stoi(parser.at(3));
    mCell.insert({name,this});
    std::cout<<pinNum<<std::endl;
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
        std::cout<<info<<std::endl;
        parser = split(info,' ',0,info.size(),4);
        std::string blkName = parser.at(1);
        std::string Metal = parser.at(2);
        int layer = std::stoi(std::string(Metal.begin()+1,Metal.end()));
        int demand = std::stoi(parser.at(3));
        blkgs.insert({blkName,{layer,demand}});
        std::cout<<blkName<<" M"<<layer<<" "<<demand<<" \n";
    }

}

//---------------------CellInst---------------------
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
    vArea=-1;//defalut no voltageArea constraint
}

//---------------------Net---------------------
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


std::vector<std::string> split(const std::string&str,char symbol,int l,int r,int num)//[l,r)
{
    std::string tmp_str;
    std::vector<std::string> V;V.reserve(num+2);
    auto ptr = str.begin()+l;
    auto end = str.begin()+r;
    for(;ptr!=end;++ptr)
    {
        if(*ptr==symbol)
        {
            if(!tmp_str.empty()){//symbol also be omit
                V.push_back(tmp_str);
                tmp_str.clear();
                num--;
            }
        }
        else
            tmp_str+=*ptr;
    }
    V.push_back(tmp_str);
    return V;
}
