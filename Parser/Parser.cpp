#include "Parser_DS.hpp"



int main()
{

    std::ifstream is{"case1.txt"};
    if(!is)
    {
        std::cerr<<"error cann't opern!\n";
        exit(1);
    }


    //----------------------------------------------------這邊是直接找到MasterCell,之後會改掉----------------------------------------
    std::string each_line;

    int NumMasterCell = 0;
    while(std::getline(is,each_line))
    {
        if(each_line.find("NumMasterCell")==0)
        {
            auto search = std::find(each_line.begin(),each_line.end(),' ');
            search++;
            NumMasterCell = std::stoi(std::string(search,each_line.end()));
            break;
        }
    }

    //------------------------------------------------------Master Cell ------------------------------------------------------
    std::unordered_map<std::string,MasterCell*>mCell;//Master Cell record
    for(int i = 0;i<NumMasterCell;i++){
        MasterCell *TMP = new MasterCell(is,mCell);
    }
    //------------------------------------------------------CellInst------------------------------------------------------
    std::unordered_map<std::string,CellInst*>CellInsts;//CellInst record
    std::getline(is,each_line);
    int InstNum = std::stoi(split(each_line,' ',0,each_line.size(),2).at(1));
    for(int i = 0;i<InstNum;i++)
    {
        std::getline(is,each_line);
        CellInst *TMP = new CellInst(each_line,mCell,CellInsts);
    }
    //------------------------------------------------------Net------------------------------------------------------------
    std::unordered_map<std::string,Net*>Nets;//Net record
    std::getline(is,each_line);
    int NumNets = std::stoi(split(each_line,' ',0,each_line.size(),2).at(1));
    for(int i = 0;i<NumNets;i++)
    {
        Net *TMP = new Net(is,CellInsts,Nets);
    }
    

    
    
    is.close();
    return 0;
}


std::vector<std::string> split(const std::string&str,char symbol,int l,int r,int num)//[l,r)
{
    std::string tmp_str;
    std::vector<std::string> V;V.reserve(num+2);
    auto ptr = str.begin()+l;
    auto end = str.begin()+r;
    for(;ptr!=end;++ptr)
    {
        if(*ptr==symbol)
        {
            if(!tmp_str.empty()){//symbol also be omit
                V.push_back(tmp_str);
                tmp_str.clear();
                num--;
            }   
        }
        else
            tmp_str+=*ptr;
    }
    V.push_back(tmp_str);
    return V;
}
