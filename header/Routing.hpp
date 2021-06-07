#ifndef ROUTING_HPP
#define ROUTING_HPP

#include "data_structure.hpp"
#include "graph.hpp"
#include <utility>

std::pair<bool,bool> CanRout(int row,int col,int lay,Graph&graph,int NetId);
inline std::pair<bool,bool> CanRout(const Point&P,Graph&graph,int NetId){return CanRout(P.row,P.col,P.lay,graph,NetId);}
void add_segment_3D(const Point&P1,const Point&P2,Graph&graph,int NetID);


#endif
