#include "graph.hpp"
Graph::Graph(std::string fileName){
    parser(fileName);
}

Graph::~Graph(){}

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
    is >> type >> value >> value >> row >> col;

    //Layers
    is >> type >> value;
    layers.resize(value);
    for(size_t i=0;i<layers.size();++i){
        Layer* TMP = new Layer(is,layers);
    }

    //Initial each layer's routing supply
    for(int i = 0; i < row; i++) {
        ggrids.push_back(std::vector<std::vector<Ggrid*>>());
        //graph.cellCount.push_back(vector<unordered_map<int,int>>());
        //graph.placement.push_back(vector<unordered_set<int>>());
        for(int j = 0; j < col; j++) {
            ggrids.at(i).push_back(std::vector<Ggrid*>());
            //graph.cellCount[i].push_back(unordered_map<int,int>());
            //graph.placement[i].push_back(unordered_set<int>());
            for(int k = 0; k < layers.size(); k++) {
                ggrids.at(i).at(j).push_back(new Ggrid(layers.at(k)->supply));
	        }
        }
    }
    //NumNonDefaultSupplyGGrid
    is >> type >> value;
    for(int i=0;i<value;++i){
        int x,y,z,offset;
	    is >> x >> y >> z >> offset;
	    ggrids.at(x-1).at(y-1).at(z-1)->capacity += offset;
    }
    //------------------------------------------------------MasterCell------------------------------------------------------------
    is >> type >> value;
    std::getline(is,type);
    for(int i = 0;i<value;i++){
        MasterCell *TMP = new MasterCell(is,mCell);
    }

    //------------------------------------------------------CellInst------------------------------------------------------------
    std::string each_line;
    std::getline(is,each_line);
    int InstNum = std::stoi(split(each_line,' ',0,each_line.size(),2).at(1));
    for(int i = 0;i<InstNum;i++){
        std::getline(is,each_line);
        CellInst *TMP = new CellInst(each_line,mCell,CellInsts);
    }
    //------------------------------------------------------Net------------------------------------------------------------
    std::getline(is,each_line);
    int NumNets = std::stoi(split(each_line,' ',0,each_line.size(),2).at(1));
    for(int i = 0;i<NumNets;i++){
        Net *TMP = new Net(is,CellInsts,Nets);
    }

    //------------------------------------------------------Route------------------------------------------------------------
    is >> type >> value;
    for(int i=0;i<value;++i){
        int x1,y1,z1;
        int x2,y2,z2;
        is >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >>type;
        ++ggrids.at(x1-1).at(y1-1).at(z1-1)->demand;
        ++ggrids.at(x2-1).at(y2-1).at(z2-1)->demand;
        //not done yet...
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
