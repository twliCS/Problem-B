#include "../header/data_structure.hpp"


//---------------------MasterCell---------------------
MasterCell::MasterCell(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCell)
{
    std::string name;
    is >> name >> name;
    #ifdef PARSER_TEST
        std::cout<<name<<"\n";
    #endif
    int pinNum;
    int blkNum;
    is >> pinNum >> blkNum;
    mCell.insert({name,this});
    for(int i = 0;i<pinNum;i++){
        std::string pinName ;
        std::string metal ;
        is >> pinName >> pinName >> metal;
        int layer = std::stoi(metal.substr(1));
        pins.insert({pinName,layer});
        #ifdef PARSER_TEST
            std::cout<<pinName<<" M"<<layer<<" \n";
        #endif
    }
    for(int i = 0;i<blkNum;i++){
        std::string blkName;
        std::string metal;
        int demand;
        is >> blkName >> blkName >> metal >> demand;
        int layer = std::stoi(metal.substr(1));
        blkgs.insert({blkName,{layer,demand}});
        #ifdef PARSER_TEST
            std::cout<<blkName<<" M"<<layer<<" "<<demand<<" \n";
        #endif
    }

}

//---------------------CellInst---------------------
CellInst::CellInst(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCells,std::unordered_map<std::string,CellInst*>&CellInsts)
{
    std::string cell_name;
    std::string m_cell_name;
    std::string type;
    is >> cell_name >> cell_name >> m_cell_name;
    mCell = mCells.find(m_cell_name)->second;
    is >> row >> col >> type;
    Movable = (type=="Movable");
    #ifdef PARSER_TEST
        std::cout<<cell_name<<" "<<row<<" "<<col<<" "<<Movable<<"\n";
    #endif
    CellInsts.insert({cell_name,this});
    vArea=-1;//defalut no voltageArea constraint
}

//---------------------Net---------------------
Net::Net(std::ifstream&is,std::unordered_map<std::string,CellInst*>&CellInsts,std::unordered_map<std::string,Net*>&Nets)
{
    int pinNum;
    std::string LayerCstr;
    is >> netName >> netName >> pinNum >> LayerCstr >> weight;
    if(LayerCstr=="NoCstr")
        minLayer = 1;
    else
        minLayer = std::stoi(LayerCstr.substr(1));

    Nets.insert({netName,this});
    #ifdef PARSER_TEST
        std::cout<<netName<<" "<<pinNum<<" "<<LayerCstr<<" "<<weight<<"\n";
    #endif

    for(int i = 0;i<pinNum;i++){
        std::string info;
        std::string cellName;
        std::string pinName;
        is >> info >>info;
        int index=info.find('/');
        cellName = info.substr(0,index);
        pinName = info.substr(index+1);
        CellInst* Cell = CellInsts.find(cellName)->second;
        net_pins.push_back({Cell,pinName});
        #ifdef PARSER_TEST
            std::cout<<cellName<<" "<<pinName<<"\n";
        #endif
    }
}
