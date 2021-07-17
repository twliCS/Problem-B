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
        std::cout<<cell_name<<" "<<m_cell_name<<" "<<row<<" "<<col<<" "<<Movable<<"\n";
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


int Net::get_two_pins(std::vector<std::pair<point,point>>& two_pin_nets){
    //std::cout<<"netName: "<<netName<<std::endl;
    size_t len = net_pins.size();
    size_t real_len = 0;//some pins are at same row,col,lay.
    int row[len];
    int col[len];
    int lay[len];

    std::map<std::string,std::set<int>>PointMap;//record the pins at same row,col but with different lay.

    for(size_t i=0;i<len;++i){
        int r = net_pins.at(i).first->row;
        int c = net_pins.at(i).first->col;
        int l = net_pins.at(i).first->mCell->pins[net_pins.at(i).second];
        
        std::string position_2d = std::to_string(r)+","+std::to_string(c);
        if(PointMap.find(position_2d) != PointMap.end())
        {
            auto & layRecord = PointMap[position_2d];
            if(layRecord.find(l) == layRecord.end())layRecord.insert(l);
        }
        else
        {
            std::set<int>layRecord;
            layRecord.insert(l);
            PointMap[position_2d] = layRecord; 
        }
        row[real_len] = r;
        col[real_len] = c;
        lay[real_len] = l;
        //std::cout<<"("<<r<<","<<c<<","<<l<<")"<<std::endl;
        real_len++;
    }


    auto is_pseudo = [&PointMap](int row,int col)
    {
        std::string position_2d = std::to_string(row)+","+std::to_string(col);
        return PointMap.find(position_2d)==PointMap.end();
    };

    auto assign_lay = [&PointMap](int row1,int col1,int row2,int col2)
    {
        std::string position1_2d = std::to_string(row1)+","+std::to_string(col1);
        std::string position2_2d = std::to_string(row2)+","+std::to_string(col2);
        
        auto &candidateP1 = PointMap[position1_2d];
        auto &candidateP2 = PointMap[position2_2d];

        float min_dis = FLT_MAX;
        int lay1,lay2;
        for (auto l1 : candidateP1)
            for(auto l2 : candidateP2)
            {
                if((l1-l2)*(l1-l2)< min_dis)
                {
                    lay1 = l1;
                    lay2 = l2;
                    min_dis = (l1-l2)*(l1-l2);
                }
            }
        return std::pair<int,int>{lay1,lay2};
    };
    Tree t = flute(real_len, row, col, ACCURACY);
    //還原z值
    for (int i=0; i<2*t.deg-2; i++) {
        int x1=t.branch[i].x;
        int x2=t.branch[t.branch[i].n].x;
        int y1=t.branch[i].y;
        int y2=t.branch[t.branch[i].n].y;
        //std::cout<<"p1 = "<<x1<<","<<y1<<"\n";
        //std::cout<<"p2 = "<<x2<<","<<y2<<"\n";
        if(x1!=x2 || y1!=y2){//the output of flute sometimes PointMap 
            int l1 = is_pseudo(x1,y1) ? -1:0;
            int l2 = is_pseudo(x2,y2) ? -1:0;
            if(l1==-1&&l2==-1)//all pseudo 
                ;
            else if (l1==-1)
                l2 = *PointMap[std::to_string(x2)+","+std::to_string(y2)].begin();
            else if(l2==-1)
                l1 = *PointMap[std::to_string(x1)+","+std::to_string(y1)].begin();
            else{
                std::pair<int,int> lays = assign_lay(x1,y1,x2,y2);
                l1 = lays.first;
                l2 = lays.second;
            }
            two_pin_nets.push_back({{x1,y1,l1},{x2,y2,l2}});
        }
    }

    for(auto &p:PointMap)
    {
        int cuti = p.first.find(',');
        int r = std::stoi(p.first.substr(0,cuti));
        int c = std::stoi(p.first.substr(cuti+1));
        int lastlay = *p.second.begin();
        auto it = p.second.begin();it++;
        for(;it!=p.second.end();it++)
        {
            two_pin_nets.push_back({{r,c,lastlay},{r,c,*it}});
            lastlay = *it;
        }
    }
    if(t.branch)
        free(t.branch);
    return t.length;
}
