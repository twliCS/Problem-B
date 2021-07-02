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
        //std::cout<<cellName<<" "<<pinName<<"\n";
    }
    EndPoint = std::vector<Ggrid*>(4,nullptr);
}


void Net::PassingGrid(Ggrid&grid)
{
    if(NotPass(grid)){
        if(grid.capacity==grid.demand)
        {
            std::cerr<<"Error input in PassingGgrid, already overflow!!\n";
            std::cout<<"happen : NetName: "<<netName<<" grid : "<<grid.row<<" "<<grid.col<<" "<<grid.lay<<"\n";
            //exit(1);
        }
        else{
            grid.add_demand();
            PassingGrids[&grid] = true;

            //Updating Endpoint
            if(!EndPoint.at(0)){EndPoint.at(0) = &grid;}
            if(!EndPoint.at(1)){EndPoint.at(1) = &grid;}
            if(!EndPoint.at(2)){EndPoint.at(2) = &grid;}
            if(!EndPoint.at(3)){EndPoint.at(3) = &grid;}

            int leftmost  = EndPoint.at(0)->col;
            int rightmost = EndPoint.at(1)->col;
            int bottom    = EndPoint.at(2)->row;
            int top       = EndPoint.at(3)->row;

            if(grid.col < leftmost)  EndPoint.at(0) = &grid;
            if(grid.col > rightmost) EndPoint.at(1) = &grid;
            if(grid.row < bottom)    EndPoint.at(2) = &grid;
            if(grid.row > top)       EndPoint.at(3) = &grid;
        }
    }
}


int Net::get_two_pins(std::vector<std::vector<int>>& two_pin_nets){
    //std::cout<<"netName: "<<netName<<std::endl;
    size_t len = net_pins.size();
    int row[len];
    int col[len];
    for(size_t i=0;i<len;++i){
        row[i]=net_pins.at(i).first->row;
        col[i]=net_pins.at(i).first->col;
        //std::cout<<"("<<row[i]<<","<<col[i]<<")"<<std::endl;
    }

    readLUT();
    Tree t = flute(len, row, col, ACCURACY);
    for (int i=0; i<2*t.deg-2; i++) {
        int x1=t.branch[i].x;
        int x2=t.branch[t.branch[i].n].x;
        int y1=t.branch[i].y;
        int y2=t.branch[t.branch[i].n].y;
        if(x1!=x2||y1!=y2){
            two_pin_nets.push_back({x1,y1,x2,y2});
        }
    }
    return t.length;
}
