#include "../header/graph.hpp"
#include "../header/Routing.hpp"



//------------------------------------------------------Destructor-------------------------------------------------------------
Graph::~Graph(){}


//------------------------------------------------------Constructor--------------------------------------------------------------
Graph::Graph(std::string fileName){
    parser(fileName);
}

void Graph::parser(std::string fileName){
    std::ifstream is{fileName};
    if(!is){
        std::cerr<<"error:file "<<fileName<<" cann't open!\n";
        exit(1);
    }

    std::string type;
    int value;

    //cell max move
    is >> type >> MAX_Cell_MOVE;

    //GGridBoundaryIdx
    is >> type >> RowBegin >> ColBegin >> RowEnd >> ColEnd;

    //---------------------------------------------------Layer&Ggrid setting-------------------------------------------------------
    is >> type >> value;
    Layers.resize(value);//value is #Layers
    Ggrids.resize(value);
    for(int i = 0; i < value; ++i)
    {
        int index,supply;
        float powerfactor;
        is >> type >> type >> index >> type >> supply >> powerfactor;
        Layers.at(index-1) = Layer{type,supply,powerfactor};//Layer Info
        Ggrids.at(index-1) = Ggrid2D(RowEnd-RowBegin+1,Ggrid1D(ColEnd-ColBegin+1,Ggrid{supply}));//Layer Ggrids
    }

    //NumNonDefaultSupplyGGrid
    is >> type >> value;
    for(int i=0;i<value;++i){
        int row,col,lay,offset;
	    is >> row >> col >> lay >> offset;
	    (*this)(row,col,lay).capacity += offset;
    }
    //-------------------------------------------------------MasterCell------------------------------------------------------------
    is >> type >> value;
    std::getline(is,type);
    for(int i = 0;i<value;i++){
        MasterCell *TMP = new MasterCell(is,mCell);
    }

    //-------------------------------------------------------CellInst------------------------------------------------------------
    std::string each_line;
    std::getline(is,each_line);
    int InstNum = std::stoi(split(each_line,' ',0,each_line.size(),2).at(1));
    for(int i = 0;i<InstNum;i++){
        std::getline(is,each_line);
        CellInst *TMP = new CellInst(each_line,mCell,CellInsts);
    }
    //----------------------------------------------------------Net------------------------------------------------------------
    std::getline(is,each_line);
    int NumNets = std::stoi(split(each_line,' ',0,each_line.size(),2).at(1));
    for(int i = 0;i<NumNets;i++){
        Net *TMP = new Net(is,CellInsts,Nets);
    }

    //------------------------------------------------------Initial Routing------------------------------------------------------------
    is >> type >> value;
    for(int i=0;i<value;++i){
        int r1,c1,l1;
        int r2,c2,l2;
        is >> r1 >> c1 >> l1 >> r2 >> c2 >> l2 >>type;
        int NetId = std::stoi(std::string(type.begin()+1,type.end()));
        add_segment_3D(Point{r1,c1,l1},Point{r2,c2,l2},*this,NetId);
    }
    //------------------------------------------------------NumVoltageAreas------------------------------------------------------------
    is >> type >> value;
    voltageAreas.resize(value);
    for(int i=0;i<value;++i){
        int counter=0;
        is >> type >> type >> type >> counter;
        for(int j=0;j<counter;++j){
            int g_x,g_y;
            is >> g_x >> g_y;
            voltageAreas.at(i).push_back({g_x,g_y});
        }
        is >> type >> counter;
        for(int j=0;j<counter;++j){
            is >> type;
            CellInsts[type]->vArea = i;
        }
    }

    /*test
    for(auto it=CellInsts.begin();it!=CellInsts.end();++it){
        std::cout<<it->first<<std::endl;
        std::cout<<it->second->mCell<<std::endl;
        std::cout<<"row:"<<it->second->row<<std::endl;
        std::cout<<"col:"<<it->second->col<<std::endl;
        std::cout<<"Move:"<<it->second->Movable<<std::endl;
        std::cout<<"vArea"<<it->second->vArea<<std::endl<<std::endl;
    }*/


    is.close();
}







