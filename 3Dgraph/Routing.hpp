#ifndef ROUTING_HPP
#define ROUTING_HPP

#include "Graph.hpp"
#include "Assign.hpp"

//超參數
//在早期,congestion不嚴重的時候,其實不用每個Layer都check LAYER_SEARCH_RANGE可以比較小,但之後要隨狀況調整


std::pair<std::vector<Point>,float> Heuristic_routing(Point &P,int delta_x,int delta_y,Graph&graph,Net_routing_Info& net_info,int target_layer = -1);
std::pair<bool,Point> L_shape(const Point &P1,const Point &P2,Graph&graph,Net_routing_Info& net_info);







#endif

