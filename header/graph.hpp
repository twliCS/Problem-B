#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_



#include <string>
#include <fstream>
#include <utility>
//---------------------------------------------dataStructures---------------------------------------------------
#include <unordered_map>
#include "data_structure.hpp"

class Graph{
public:
    Graph(std::string fileName);
    void parser(std::string fileName);
    ~Graph();


//---------------------------------------------General using--------------------------------------------------------
    Ggrid& operator()(int row,int col,int lay){
        if(row > RowEnd)
        {
            std::cerr<<"Ggrid accessing Error!!!,Input : row must <= RowEnd :\n"<<" row = "<<row<<" RowEnd = "<<RowEnd<<"\n";
            exit(1);
        }
        if(col > ColEnd)
        {
            std::cerr<<"Ggrid accessing Error!!!,Input : col must <= ColEnd :\n"<<" col = "<<col<<" ColEnd = "<<ColEnd<<"\n";
            exit(1);
        }
        if(lay > Layers.size())
        {
            std::cerr<<"Ggrid accessing Error!!!,Input : lay must <= LayNum :\n"<<" lay = "<<lay<<" LayNum = "<<Layers.size()<<"\n";
            exit(1);
        }
        return Ggrids.at(lay-1).at(row-RowBegin).at(col-ColBegin);
    }

    int LayerNum()const{return Layers.size();}
    std::pair<int,int>RowBound()const{return {RowBegin,RowEnd};}
    std::pair<int,int>ColBound()const{return {ColBegin,ColEnd};}

//--------------------------------------------Data Mmeber------------------------------------------------------------
    std::unordered_map<std::string,MasterCell*>mCell;
    std::unordered_map<std::string,CellInst*>CellInsts;
    std::unordered_map<std::string,Net*>Nets;
    std::vector<std::vector<std::pair<int,int>>>voltageAreas;
private:
    using Ggrid1D = std::vector<Ggrid>;
    using Ggrid2D = std::vector<Ggrid1D>;
    using Ggrid3D = std::vector<Ggrid2D>;
    std::vector<Layer>Layers;
    Ggrid3D Ggrids;
    int MAX_Cell_MOVE;
    int RowBegin,ColBegin;
    int RowEnd,ColEnd;
};



#endif
