#include "LayerAssingment.hpp"
#include <iostream>
#include <map>

std::ostream& operator<<(std::ostream&os,const Point_3D&p)
{
    os<<"("<<p.x<<","<<p.y<<","<<p.z<<")";
    return os;
}

bool operator<(const Point_3D&p1,const Point_3D&p2)
{
    if(p1.x<p2.x)return true;
    else if(p1.x==p2.x)
    {
        if(p1.y<p2.y)return true;
        else if(p1.y==p2.y)
        {
            if(p1.z<p2.z)return true;
        }
    }
    
    return false;
}

//P1,P2 only differ by one dir.
void add_segment_3D(Graph_3D&graph,Point_3D&P1,Point_3D&P2,std::map<Point_3D,bool>&record)//don not need consider starting point.
{
    int x = P1.x-1,t_x = P2.x-1;
    int y = P1.y-1,t_y = P2.y-1;
    int z = P1.z-1,t_z = P2.z-1;
    int delta_x = (x!=t_x)? ((x < t_x)? 1:-1):0;
    int delta_y = (y!=t_y)? ((y < t_y)? 1:-1):0;
    int delta_z = (z!=t_z)? ((z < t_z)? 1:-1):0;
    while(x!=t_x||y!=t_y||z!=t_z)
    {
        if(record.find(Point_3D{x,y,z})==record.end())
        {
            record.insert({Point_3D{x,y,z},true});
            graph[z].add_demand(x,y);
        }    
        x+=delta_x;
        y+=delta_y;
        z+=delta_z;
    }
    if(record.find(Point_3D{x,y,z})==record.end())
    {
        record.insert({Point_3D{x,y,z},true});
        graph[z].add_demand(x,y);
    }    
}

void add_net(Graph_3D&graph,NET_3D&net)//need consider starting point.
{
    Point_3D start_Ggrid = net.at(0).p1;
    std::map<Point_3D,bool>record;//紀錄以免重算
    for(auto s:net)//for each segment_3D
    {
        add_segment_3D(graph,s.p1,s.p2,record);
    }
    std::cout<<"this net's WL = "<<record.size()<<"\n";
}

void Layer::show_congestion()const
{
    for(auto row = Ggrids.rbegin();row!=Ggrids.rend();++row)
    {
        for(auto &col : *row)
        {
            std::cout<<col.capacity-col.demand<<" ";
        }
        std::cout<<"\n";
    }
    std::cout<<"\n";
}
void Layer::show_demand()const
{
    for(auto row = Ggrids.rbegin();row!=Ggrids.rend();++row)
    {
        for(auto &col : *row)
        {
            std::cout<<col.demand<<" ";
        }
        std::cout<<"\n";
    }
    std::cout<<"\n";
}
int main()
{


    Point_2D P1{1,1};
    Point_2D P2{5,5};
    Graph_3D graph ={//M1,M2,M3
        Layer(P1,P2,true,0.8,10),   //H
        Layer(P1,P2,false,0.8,5),  //V 
        Layer(P1,P2,true,0.8,5),   //H
    };


    NET_3D net1 = {
        {{1,4,1},{1,4,2}},
        {{1,4,2},{1,4,3}}, 
        {{4,4,1},{4,4,2}}, 
        {{4,4,2},{4,4,3}},
        {{2,2,1},{2,2,2}},
        {{2,2,2},{2,1,2}},
        {{2,1,2},{2,1,3}},
        {{3,1,2},{3,4,2}},
        {{3,4,2},{3,4,3}},
        {{3,1,2},{3,1,3}},

        {{2,1,3},{3,1,3}},
        {{1,4,3},{4,4,3}}
    };
    NET_3D net3 = {
        {{2,2,1},{2,2,3}},
        {{4,2,1},{4,2,3}},
        {{2,2,2},{2,5,2}},
        {{2,2,3},{4,2,3}}
    };
    add_net(graph,net1);
    for(auto &l:graph)
    {
        l.show_demand();
    }
    add_net(graph,net3);

    for(auto &l:graph)
    {
        l.show_demand();
    }
    return 0;
}
