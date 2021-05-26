#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <utility>
#include <iostream>

#include "Layer.hpp"
class Graph{
public:
    Graph(std::vector<Layer>L)
        :Layers{L}
        {

        }
    void Non_default_grid(int x,int y,int z,int offset){(*this)(x,y,z).adjust_cap(offset);}
    Ggrid& operator()(int x,int y,int z){
        if(z<1)
        {
            std::cerr<<"class Graph: operator()(int x,int y,int z) : Error : Layer index z must larger than 0!\n";
            exit(1);
        }
        if(z>Layers.size())
        {
            std::cerr<<"class Graph: operator()(int x,int y,int z) : Error : Layer index z must less than max_layer!\n";
            exit(1);
        }
        return (*this)[z](x,y);
    }
    int Layer_Num()const{return Layers.size();}
    std::pair<int,int>Grid_size()const{return Layers.at(0).Grid_size();}
    Layer& operator[](int i){
        if(i<1)
        {
            std::cerr<<"class Graph: operator[](int i) : Error : i must larger than 0!\n";
            exit(1);
        }
        if(i>Layers.size())
        {
            std::cerr<<"class Graph: operator()(int x,int y,int z) : Error : Layer index z must less than max_layer!\n";
            exit(1);
        }
        return Layers.at(i-1);
    }
private:
    std::vector<Layer>Layers;
};


#endif
