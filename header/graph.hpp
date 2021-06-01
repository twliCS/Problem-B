#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_

#include <unordered_map>
#include <string>
#include <fstream>

#include "data_structure.hpp"

class Graph{
public:
    Graph(std::string fileName);
    void parser(std::string fileName);
    ~Graph();

    std::unordered_map<std::string,MasterCell*>mCell;
    std::unordered_map<std::string,CellInst*>CellInsts;
    std::unordered_map<std::string,Net*>Nets;
    std::vector<Layer*>layers;
    std::vector<std::vector<std::vector<Ggrid*>>>ggrids;
    std::vector<std::vector<std::pair<int,int>>>voltageAreas;

private:
    int MAX_Cell_MOVE;
    int row;
    int col;
};

#endif
