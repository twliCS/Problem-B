#ifndef ROUTING_HPP
#define ROUTING_HPP

#include "data_structure.hpp"
#include "graph.hpp"
#include <utility>
#include <float.h>

std::pair<bool,bool> CanRout(int row,int col,int lay,Graph&graph,int NetId);
void add_segment_3D(Ggrid&P1,Ggrid&P2,Graph&graph,int NetId);


extern int LAYER_SEARCH_RANGE;
extern float ESCAPE_W;

// 用來計算(僅水平或僅垂直)移動至目標點P的cost，會包含net_weight*powerfactor,以及Escape rate
// src : 當前的點,方向由src.lay決定:H or V
// d : 與當前的點之delta值,僅-1或1兩種可能
//first : CanRout or not
//second :cost
std::pair<bool,float> Cost1D(Ggrid&src,int d,Graph&graph,int NetId);


//First  : path from P to (P.row+d_row,P.col+d_col) .
//1. First[0] : (P.row+d_row,P.col+d_col)
//2. If it is empty : No path from P to (P.row+d_row,P.col+d_col) .
//Second : cost of this path
std::pair<std::vector<Ggrid*>,float> Layer_routing(Ggrid &P,int d_row,int d_col,Graph&graph,int NetId);
#endif
