#include <ctime>
#include <iostream>

#include "graph.hpp"

int main(int argc, char** argv)
{
    if(argc!=2){
        std::cerr<<"Wrong parameters!"<<std::endl;
        return -1;
    }
    std::string path = "./benchmark/";
    std::string fileName = argv[1];
    //argv[1]
    Graph* graph = new Graph(path+fileName);

    return 0;
}
