#include "Graph.hpp"
#include <iostream>
#include "Assign.hpp"
#include "Routing.hpp"
#include "random"
#include "time.h"

std::ostream& operator<<(std::ostream&os,const Point&p)
{
    os<<"("<<p.x<<","<<p.y<<","<<p.z<<")";
    return os;
}


void show_demand(Graph&graph,Point*Pin_now = nullptr)
{
    int Layer_Num = graph.Layer_Num();
    std::pair<int,int>Grids_size = graph.Grid_size();
    

    int high_d = 0;
    int low_d = 0;
    for(int z = 1;z <= Layer_Num;z++)
    {
        
        std::cout<<"Layer : "<<z<<"\n";
        for(int y = Grids_size.second;y >=1 ; y--)
        {
            for(int x = 1; x <= Grids_size.first; x++)
            {
                if(Pin_now&&(Pin_now->x==x)&&(Pin_now->y==y)&&(Pin_now->z==z))
                    std::cout<<"*"<<" ";
                else 
                    std::cout<<graph(x,y,z).demand<<" ";

                if(z>=5)
                    high_d+=graph(x,y,z).demand;
                else 
                    low_d+=graph(x,y,z).demand;
            }
            std::cout<<"\n";
        }
    }
    std::cout<<"total demand in L1 L2 L3 L4 :"<<low_d<<"\n";
    std::cout<<"total demand in L5 L6 L7 :"<<high_d<<"\n";
}


void move_to_final(Graph&graph,Net_routing_Info&net_info,std::vector<Point>&points)
{
    for(int i = 1;i < points.size();i++)
    {
        add_segment_3D(points.at(i-1),points.at(i),graph,net_info);
    }

}

//繞線COST超參數
float VIA_W = 1;
float ESCAPE_W = 1;
int LAYER_SEARCH_RANGE = 10;

int main()
{
    std::cout<<"enter escape_w\n";
    std::cin>>ESCAPE_W;

  
    //--------------------------------------------Graph Init
    Point P1{1,1};
    Point P2{10,10};
    Graph graph = std::vector<Layer>{//M1,M2,M3
        Layer(P1,P2,true,10,100),   // M1 : H
        Layer(P1,P2,false,10,100),  // M2 : V 
        Layer(P1,P2,true,1,100),   //  M3 : H
        Layer(P1,P2,false,1,100),   //  M4 : v
        Layer(P1,P2,true,0.1,100),   //  M5 : H
        Layer(P1,P2,false,0.1,100),   //  M6 : v
        Layer(P1,P2,true,0.1,100),   //  M7 : H
    };

    
    //--------------------------------------------Net setting-----------------------------------
    Net_routing_Info Net1{1,1};//minLayer,weight


    Point PIN1{1,1};

    int test_time = 1000000;
    srand(0);

    int operation = 0;
    while(test_time--)
    {

        char MOV[] = {'w','s','a','d','n'};

        int operation = MOV[rand()%5];

        if(operation=='x')
        {
            std::cout<<"exit \n";
            exit(1);
        }


        else if(operation=='d')
        {
            //std::cout<<"move +H !!\n";
            auto result = Heuristic_routing(PIN1,1,0,graph,Net1);

            auto &PATH = std::get<0>(result);
            float cost = std::get<1>(result);
            auto &Final = std::get<2>(result);
            if(PATH.empty())
            {
                //std::cout<<PIN1<<" can't move +H !! \n";
            }
            else{
                //std::cout<<PIN1<<"move to "<<Final<<" cost = " <<cost<<"\n";
                PIN1 = Final;
                move_to_final(graph,Net1,PATH);
            }
            //show_demand(graph,&PIN1);
        }
        else if(operation=='a')
        {
            //std::cout<<"move -H !!\n";
            auto result = Heuristic_routing(PIN1,-1,0,graph,Net1);

            auto &PATH = std::get<0>(result);
            float cost = std::get<1>(result);
            auto &Final = std::get<2>(result);
            if(PATH.empty())
            {
               // std::cout<<PIN1<<" can't move -H !! \n";
            }
            else{
               // std::cout<<PIN1<<"move to "<<Final<<" cost = " <<cost<<"\n";
                PIN1 = Final;
                move_to_final(graph,Net1,PATH);
            }
            //show_demand(graph,&PIN1);
        }

        else if(operation=='w')
        {

           // std::cout<<"move +V !!\n";
            auto result = Heuristic_routing(PIN1,0,1,graph,Net1);

            auto &PATH = std::get<0>(result);
            float cost = std::get<1>(result);
            auto &Final = std::get<2>(result);
            if(PATH.empty())
            {
             //   std::cout<<PIN1<<" can't move +V !! \n";
            }
            else{
               // std::cout<<PIN1<<"move to "<<Final<<" cost = " <<cost<<"\n";
                PIN1 = Final;
                move_to_final(graph,Net1,PATH);
            }
            //show_demand(graph,&PIN1);
        }

        else if(operation=='s')
        {

            //std::cout<<"move -V !!\n";
            auto result = Heuristic_routing(PIN1,0,-1,graph,Net1);

            auto &PATH = std::get<0>(result);
            float cost = std::get<1>(result);
            auto &Final = std::get<2>(result);
            if(PATH.empty())
            {
               // std::cout<<PIN1<<" can't move -V !! \n";
            }
            else{
              //  std::cout<<PIN1<<"move to "<<Final<<" cost = " <<cost<<"\n";
                PIN1 = Final;
                move_to_final(graph,Net1,PATH);
            }
            //show_demand(graph,&PIN1);
        }

        else if(operation=='n')
        {
            //std::cout<<"add new net!!\n";
            int x = rand()%10+1;
            int y = rand()%10+1;
            int z = rand()%7+1;
            PIN1 = Point{x,y,z};
            // int L = rand()%7+1;
            Net1 = Net_routing_Info{1,1};
            //show_demand(graph,&PIN1);
        }

        else if(operation=='f')
        {
            std::cout<<"adjsut capacity!\n";
            std::cout<<"enter the x,y,z and offset\n";
            int x,y,z,off;
            std::cin>>x>>y>>z>>off;
            graph.Non_default_grid(x,y,z,off);
            std::cout<<Point{x,y,z}<<"'s cap = "<<graph(x,y,z).capacity<<"\n";
        }


    }
    show_demand(graph,&PIN1);

    // //over flow test
    // graph.Non_default_grid(2,1,1,-10);
    // auto result1 = Heuristic_routing(PIN1,1,0,graph,Net1);

    // std::cout<<"cost = " <<std::get<1>(result1)<<"\n";
    // std::cout<< "point = "<<std::get<2>(result1)<<"\n";


    // //escape rate test : Let 
    // graph.Non_default_grid(2,1,2,-10);
    // graph.Non_default_grid(3,1,1,-10);
 
    // auto result2 = Heuristic_routing(PIN1,1,0,graph,Net1);
   
    // std::cout<<"cost = " <<std::get<1>(result2)<<"\n";
    // std::cout<< "point = "<<std::get<2>(result2)<<"\n";

  
    


    

    // show_demand(graph);










    // NET_3D net1 = {
    //     {{1,4,1},{1,4,2}},
    //     {{1,4,2},{1,4,3}}, 
    //     {{4,4,1},{4,4,2}}, 
    //     {{4,4,2},{4,4,3}},
    //     {{2,2,1},{2,2,2}},
    //     {{2,2,2},{2,1,2}},
    //     {{2,1,2},{2,1,3}},
    //     {{3,1,2},{3,4,2}},
    //     {{3,4,2},{3,4,3}},
    //     {{3,1,2},{3,1,3}},

    //     {{2,1,3},{3,1,3}},
    //     {{1,4,3},{4,4,3}}
    // };
    // NET_3D net3 = {
    //     {{2,2,1},{2,2,3}},
    //     {{4,2,1},{4,2,3}},
    //     {{2,2,2},{2,5,2}},
    //     {{2,2,3},{4,2,3}}
    // };

    // add_net(graph,net1,0.5,1);
    // add_net(graph,net3,0.5,1);




    //L_shape({1,4,1},{2,2,3},graph,Net1); //L shape not done now, to Pin net not done
    
    return 0;
}
