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
    #ifdef PARSER_TEST
        std::cout<<type<<' '<<MAX_Cell_MOVE<<std::endl;
    #endif



    //GGridBoundaryIdx
    is >> type >> RowBegin >> ColBegin >> RowEnd >> ColEnd;
    #ifdef PARSER_TEST
        std::cout<<"GGridBoundaryIdx "<< RowBegin <<ColBegin << RowEnd << ColEnd<<std::endl;
    #endif
    //---------------------------------------------------Layer&Ggrid setting-------------------------------------------------------
    is >> type >> value;
    #ifdef PARSER_TEST
        std::cout<<type<<' '<< value << std::endl;
    #endif
    Layers.resize(value);//value is #Layers
    Ggrids.resize(value);
    for(int i = 0; i < value; ++i)
    {
        int index,supply;
        float powerfactor;
        //     Lay     M1      1        H       10       1.2
        is >> type >> type >> index >> type >> supply >> powerfactor;
        Layers.at(index-1) = Layer{type,supply,powerfactor};//Layer Info
        Ggrids.at(index-1) = Ggrid2D(RowEnd-RowBegin+1,Ggrid1D(ColEnd-ColBegin+1,Ggrid{supply}));//Layer Ggrids
    }
    #ifdef PARSER_TEST
        for(size_t i=0;i<Layers.size();++i){
            std::cout<<"M"<<i+1<<' ';
            if(Layers.at(i).horizontal)
                std::cout<<"H ";
            else
                std::cout<<"V ";
            std::cout << Layers.at(i).supply << ' ' << Layers.at(i).powerFactor<<std::endl;
        }
    #endif

    //----------------------------------------------------row/col/lay setting------------------------------------------------------
    for(int lay = 1;lay <= LayerNum();++lay)
    {
        for(int row = RowBegin; row <=RowEnd; ++row){
            for(int col = ColBegin; col<=ColEnd;++col)
            {
                (*this)(row,col,lay).lay = lay;
                (*this)(row,col,lay).row = row;
                (*this)(row,col,lay).col = col;
            }
        }
    }

    //NumNonDefaultSupplyGGrid

    is >> type >> value;
    #ifdef PARSER_TEST
        std::cout<<type<<' '<<value<<std::endl;
    #endif
    for(int i=0;i<value;++i){
        int row,col,lay,offset;
	    is >> row >> col >> lay >> offset;
	    (*this)(row,col,lay).capacity += offset;
        #ifdef PARSER_TEST
            std::cout<<row<<' '<<col<<' '<<lay<<' '<<offset<<std::endl;
        #endif
    }

    //-------------------------------------------------------MasterCell------------------------------------------------------------
    is >> type >> value;
    #ifdef PARSER_TEST
        std::cout<<type<<' '<<value<<std::endl;
    #endif
    for(int i = 0;i<value;i++){
        MasterCell *TMP = new MasterCell(is,mCell);
    }

    //-------------------------------------------------------CellInst------------------------------------------------------------
    is >> type >> value;
    for(int i = 0;i<value;i++){
        CellInst *TMP = new CellInst(is,mCell,CellInsts);
    }
    //----------------------------------------------------------Net------------------------------------------------------------
    is >> type >> value;
    for(int i = 0;i<value;i++){
        Net *TMP = new Net(is,CellInsts,Nets);
    }

    //------------------------------------------------------Initial Routing------------------------------------------------------------
    is >> type >> value;
    for(int i=0;i<value;++i){
        int r1,c1,l1;
        int r2,c2,l2;
        is >> r1 >> c1 >> l1 >> r2 >> c2 >> l2 >>type;
        int NetId = std::stoi(std::string(type.begin()+1,type.end()));
        Ggrid& g1 = (*this)(r1,c1,l1);
        Ggrid& g2 = (*this)(r2,c2,l2);
        add_segment_3D(g1, g2 ,*this,NetId);
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
            CellInsts[type]->vArea = i;//" from 0 to graph.cpp:voltageAreas.size()-1 "
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
