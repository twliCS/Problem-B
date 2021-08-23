#include "../header/Routing.hpp"


bool node::IsSingle()//Is leaf and no parent
{
    return routing_tree->leaf.find(this)!=routing_tree->leaf.end()&&!parent;
}
void node::connect(node *host)
{
    if(this->parent!=host&&this!=host)
    {   
        child.insert(host);
        host->parent = this;
        this->routing_tree->leaf.erase(this);
    }

    //future:adding host to routing tree (Broadcast)
}

//-------------------------------------------------node Member function-----------------------------------------------------------





bool passing(Ggrid*grid,NetGrids*net)
{
    if(net->AlreadyPass(grid))
        return true;
    else{
        if(grid->get_remaining()){
            net->PassGrid(grid);
            return true;
        }
    }
    return false;
}





//--------------------------------------------------two-pin-sets function---------------------------------------------------
int TwoPinNetsInit(Graph*graph,NetGrids*net,TwoPinNets&pinset)
{
    int totalInit = 0;
    bool canInit = false;
    for(auto &twopin:pinset)
    {
        pos pin1 = twopin.first->p;
        pos pin2 = twopin.second->p;
        //std::cout<<"pin1!\n";
        if(pin1.lay!=-1){  (canInit = passing(&(*graph)(pin1.row,pin1.col,pin1.lay),net)==true) ? totalInit+=1:totalInit+=0;} //need
        //std::cout<<"canInit ="<<canInit<< "!\n";
        if(pin2.lay!=-1&&canInit){(canInit = passing(&(*graph)(pin2.row,pin2.col,pin2.lay),net)==true) ? totalInit+=1:totalInit+=0;}//need
        //std::cout<<"canInit ="<<canInit<< "!\n";
        if(!canInit){
            // std::cout<<"return -1!\n";
            return -1;
        }
    }
    return totalInit;
}

void Sgmt_Init(node*v,node*u,pos &PosS,pos&PosT,pos &PosDelta)
{
    int sRow = v->p.row;
    int sCol = v->p.col;
    int sLay = v->p.lay;

    int tRow = u->p.row;
    int tCol = u->p.col;
    int tLay = u->p.lay;
            
    int d_r = (sRow==tRow)? 0 : ( (sRow>tRow)? -1:1);
    int d_c = (sCol==tCol)? 0 : ( (sCol>tCol)? -1:1);
    int d_l = (sLay==tLay)? 0 : ( (sLay>tLay)? -1:1);  
    using std::abs;
    int check = abs(d_r) + abs(d_c) + abs(d_l);
    if(check>1){std::cerr<<"error in Sgmt_Init!!! Input: "<<u->p<<" " << v->p <<" is not a segment\n";exit(1);}
    
    PosS = pos{sRow,sCol,sLay};
    PosT = pos{tRow,tCol,tLay};
    PosDelta = pos{d_r,d_c,d_l};
}

void Sgmt_Grid(Graph*graph,NetGrids*net,node*v,node*u,bool(*f)(Ggrid*,NetGrids*))
{
    pos PosS,PosT,PosDelta;
    Sgmt_Init(v,u,PosS,PosT,PosDelta);
    do{
        auto &grid = (*graph)(PosS.row,PosS.col,PosS.lay);
        f(&grid,net);
        PosS.row+=PosDelta.row;
        PosS.col+=PosDelta.col;
        PosS.lay+=PosDelta.lay;
    }while(PosS!=PosT);
    auto &grid = (*graph)(PosS.row,PosS.col,PosS.lay);
    f(&grid,net);//last
}


void Backtack_Sgmt_Grid(Graph*graph,NetGrids*net,node*v,bool(*f)(Ggrid*,NetGrids*))
{
    while(v->parent)
    {
        Sgmt_Grid(graph,net,v,v->parent,f);
        v = v->parent;
    }
}
//------------------------------------------------Demand Interface------------------------------------------------------------------------
float RipUpNet(Graph*graph,NetGrids*net)
{
    float sc = 0;
    for(auto &g:net->grids)
    {
        Ggrid* grid = g.first;
        bool alreadyAdd = g.second;
        if(alreadyAdd)
        {
            grid->delete_demand();
            g.second = false;
            float pf = graph->getLay(grid->lay).powerFactor;
            float weight = graph->getNet(net->NetId).weight;
            sc+=pf*weight;
        }
    }
    graph->score-=sc;
    return sc;
}
float AddingNet(Graph*graph,NetGrids*net)
{
    float sc = 0;
    for(auto &g:net->grids)
    {
        Ggrid* grid = g.first;
        bool alreadyAdd = g.second;
        if(!alreadyAdd)
        {
            if(grid->get_remaining())
            {
                grid->add_demand();//need
                g.second = true;
                float pf = graph->getLay(grid->lay).powerFactor;
                float weight = graph->getNet(net->NetId).weight;
                sc+=pf*weight;
            }
        }
    }
    graph->score+=sc;
    return sc;
}

void RipUpAll(Graph*graph)
{
    for(int i = 1;i<=graph->Nets.size();i++){
        NetGrids* net = graph->getNetGrids(i); 
        RipUpNet(graph,net);
    }
}
void AddingAll(Graph*graph)
{
    for(int i = 1;i<=graph->Nets.size();i++){
        NetGrids* net = graph->getNetGrids(i);   
        AddingNet(graph,net);
    }
}
class minCost{
public:
    minCost() = default;
    bool operator()(const node* n1,const node*n2){return n1->cost > n2->cost;}
};
void TreeInterface(Graph*graph,NetGrids*net,bool(*callback)(Ggrid* ,NetGrids*),tree* nettree)
{
    tree *t = nullptr;
    if(nettree){
        t = nettree;
    }
    else
        t = graph->getTree(net->NetId);

    for(auto leaf:t->leaf){
        if(leaf->IsSingle()&&leaf->p.lay!=-1)
            callback(&(*graph)(leaf->p.row,leaf->p.col,leaf->p.lay),net);
        else if(leaf->p.lay!=-1)
            Backtack_Sgmt_Grid(graph,net,leaf,callback);
    }
}



//Output interface---------------------
void backTrackPrint(Graph*graph,Net*net,node*v,std::vector<std::string>*segment)
{
    while (v->parent)
    {
        auto &g = (*graph)(v->p.row,v->p.col,v->p.lay);

        std::string posv = pos2str(v->p);
        std::string posu = pos2str(v->parent->p);
        if(segment){segment->push_back(posu+" "+posv+" "+net->netName);}
        else{std::cout<<(posu+" "+posv+" "+net->netName)<<"\n";}
        v = v->parent;
    }
    
}

void printTree(Graph*graph,Net*net,tree*t,std::vector<std::string>*segment)
{
    for(auto leaf:t->leaf){
        if(!leaf->IsSingle()&&leaf->p.lay!=-1)
            backTrackPrint(graph,net,leaf,segment);
    }
}
void PrintAll(Graph*graph,std::vector<std::string>*segment)
{
    for(int i = 1;i<=graph->Nets.size();i++){
        auto t = graph->getTree(i);
        auto &net = graph->getNet(i);
        printTree(graph,&net,t,segment);
    }
}





tree* getPath(Graph*graph,NetGrids*net,node*v,std::unordered_map<node*,bool>&t1Point)
{

    net->PassGrid(graph,v);
    tree* path = new tree;
    node* last = new node(v->p);
    path->addNode(last);
    v = v->parent;
    while(v)
    {
        node* n = new node(v->p);
        path->addNode(n);
        last->connect(n);
        last = n;
        net->PassGrid(graph,n);
        if(t1Point.find(v)!=t1Point.end())break;
        
        v = v->parent;
    }
    return path;
}


node* Search(Graph*graph,NetGrids*net,node *v,const pos&delta,std::unordered_map<std::string,float>&gridCost,tree*tmp,BoundingBox Bx)
{
    //Dir checking
    if(delta.row!=0&&v->p.lay%2==1){return nullptr;}//error routing dir
    if(delta.col!=0&&v->p.lay%2==0){return nullptr;}//error routing dir

    //Boundary checking
    pos P = {v->p.row+delta.row,v->p.col+delta.col,v->p.lay+delta.lay};
    if(P.row<Bx.minRow||P.row>Bx.maxRow){return nullptr;} //RowBound checking
    if(P.col<Bx.minCol||P.col>Bx.maxCol){return nullptr;} //ColBound checking

    int minLayer = graph->getNet(net->NetId).minLayer;
    if(P.lay<minLayer ||P.lay>graph->LayerNum()){return nullptr;}//minLayer checking
    
    //Capacity checking
    Ggrid& g = (*graph)(P.row,P.col,P.lay);
    bool enough = (net->AlreadyPass(&g))? true : g.get_remaining(); ////need change  first come,first serve
    if(!enough)return nullptr;


    //Caculate Cost    
    std::string str = pos2str(P);
    float lastCost = (gridCost.find(str)==gridCost.end())? FLT_MAX:gridCost[str];
    float pf = graph->getLay(P.lay).powerFactor;
    float weight = graph->getNet(net->NetId).weight;
    
    node * n = new node(P);
    n->cost = (net->AlreadyPass(&g))? 0:weight*pf+v->cost;////need

    if(n->cost<lastCost)
    {
        tmp->addNode(n);
        v->connect(n);
        gridCost[str] = n->cost;
    }
    else{
        delete n;
        n = nullptr;
    }
    return n;
}



void combine(tree*t1,tree*t2)
{
    for(auto l:t2->leaf)
    {
        t1->leaf.insert(l);
        l->routing_tree = t1;
    }
    for(auto n:t2->all)
    {
        t1->all.push_back(n);
        n->routing_tree = t1;
    }
    t2->all.clear();
    t2->leaf.clear();
    delete t2;
}

bool AssingPesudo(Graph*graph,NetGrids*net,node*n)
{
    float Bestcost = FLT_MAX;
    int minLay = graph->getNet(net->NetId).minLayer;
    int bestLay = minLay;
    for(int i = minLay;i<=graph->LayerNum();i++)
    {
        auto &grid = (*graph)(n->p.row,n->p.col,i);
        int netcost = 0;
        if(net->AlreadyPass(&grid))
        {
            n->p.lay = i;
            return true;
        }
        else 
            netcost = 1/(1+exp2(grid.get_remaining()));
        if(netcost<Bestcost)
        {
            Bestcost = netcost;
            n->p.lay = i;
        }
    }
    if(n->p.lay==-1){return false;}
    if((*graph)(n->p.row,n->p.col,n->p.lay).get_remaining()==0){return false;}
    net->PassGrid(&(*graph)(n->p.row,n->p.col,n->p.lay));
    return true;
}


bool TargetTree(Graph*graph,NetGrids*net,tree*t,std::unordered_map<std::string,node*>&target)
{   
    bool isPseudo = false;
    for(auto n:t->all)
    {
        if(n->p.lay==-1){ 
            std::string twoDpos = std::to_string(n->p.row)+" "+std::to_string(n->p.col);
            target[twoDpos] = n;
            isPseudo  = true;
            for(int i = graph->getNet(net->NetId).minLayer;i<=graph->LayerNum();i++)
            {
                std::string pos3d = twoDpos+" "+std::to_string(i);
                target.insert({pos3d,nullptr});//nullptr represents pesudo
            }
        }
        else{
            target[pos2str(n->p)] = n;
        } 
    }
    return isPseudo;
}


bool SourceTree(Graph*graph,NetGrids*net, tree*t1,tree*tmp,\
std::priority_queue<node*,std::vector<node*>,minCost>&Q,std::unordered_map<std::string,float>&gridCost,std::unordered_map<node*,bool>&t1Point)
{
    bool sourceInit = true;
    for(auto n:t1->all){
        node * v = new node(n->p);tmp->addNode(v);
        if(v->p.lay==-1)
        {
            sourceInit = AssingPesudo(graph,net,v);
            if(!sourceInit)break;
        }
        v->cost = 0;
        Q.push(v);
        gridCost[pos2str(v->p)] = 0;
        t1Point[v] = true;
    }
    return sourceInit;
}

bool isTarget(node *v,std::unordered_map<std::string,node*>&target)
{
    std::string str = pos2str(v->p);
    if(target.find(str)!=target.end())//find
    {
        node* n = target.at(str);
        if(!n)//it is pesudo 
        {
            std::string twoDpos = std::to_string(v->p.row)+" "+std::to_string(v->p.col);
            target.at(twoDpos)->p = v->p; //updating
        }
        return true;
    }
    return false;
}

tree* Tree2Tree(Graph*graph,NetGrids*net,tree*t1,tree*t2)
{
    if(t1==t2)return t1;//precheck

    //---------------------------------------------------------------Source(tree1) Init------------------------------------------------------
    std::priority_queue<node*,std::vector<node*>,minCost>Q;
    std::unordered_map<std::string,float>gridCost;
    std::unordered_map<node*,bool>t1Point;
    tree* tmp = new tree;//routing nodes are saved in tmp.
    bool sourceInit = SourceTree(graph,net,t1,tmp,Q,gridCost,t1Point);//Multi Source
    //---------------------------------------------------------------Target(tree2) Init------------------------------------------------------
    std::unordered_map<std::string,node*>target;
    bool t2isPesudo = TargetTree(graph,net,t2,target);//Multi Target
    

    BoundingBox Bx;
    if(sourceInit){
        t1->updateEndPoint(graph);
        t2->updateEndPoint(graph);    
        if(t2isPesudo)//t2 is pesudo
        {
            Bx = BoundingBox (graph,&graph->getNet(net->NetId));
        }
        else{
            Bx = BoundingBox (graph,&graph->getNet(net->NetId),t1,t2);
        }
    }
    node *targetPoint = nullptr;
    while(!Q.empty()&&!targetPoint&&sourceInit)
    {
        node * v = Q.top();Q.pop();
       
        if(isTarget(v,target))//find
        {
            targetPoint = v;
            break;
        }
        node * u = nullptr;
        if(u = Search(graph,net,v,{0,0,1},gridCost,tmp,Bx))Q.push(u); //up
        if(u = Search(graph,net,v,{0,0,-1},gridCost,tmp,Bx))Q.push(u);//down
        if(u = Search(graph,net,v,{0,1,0},gridCost,tmp,Bx))Q.push(u);//-col
        if(u = Search(graph,net,v,{0,-1,0},gridCost,tmp,Bx))Q.push(u);//+col
        if(u = Search(graph,net,v,{1,0,0},gridCost,tmp,Bx))Q.push(u);//+row
        if(u = Search(graph,net,v,{-1,0,0},gridCost,tmp,Bx))Q.push(u);//-row
    }
   
    //std::cout<<"Step4\n";//Step 4
    if(targetPoint){
        tree*path = getPath(graph,net,targetPoint,t1Point);
        combine(t1,path);
        combine(t1,t2);
        delete tmp;
        return t1;
    }
    else {
        delete tmp;
        return nullptr;
    }
}





std::pair<ReroutInfo,bool> Reroute(Graph*graph,int NetId,TwoPinNets&twopins)
{
    //std::cout<<"init"<<"\n";
    NetGrids * netgrids = new NetGrids(NetId);
    int initdemand = TwoPinNetsInit(graph,netgrids,twopins);//Init
    //std::cout<<"ImitDmd =  "<<initdemand<<"\n";

    tree*T= nullptr;
    //std::cout<<"TwopinNet!\n";
    for(auto pins:twopins)
    {   
        if(initdemand!=-1)
            T = Tree2Tree(graph,netgrids,pins.first->routing_tree,pins.second->routing_tree);
        if(!T) //把整個two-pin nets 繞線產生出來的tree全部collect成一棵回傳
        {
            std::set<tree*>collect;//set(避免duplicate delete)
            for(auto pins:twopins)
            {
                collect.insert(pins.first->routing_tree);
                collect.insert(pins.second->routing_tree);
            }
            tree* CollectTree = new tree;
            for(tree* t:collect)
            {
                for(node * pin : t->leaf){CollectTree->leaf.insert(pin);}//for rip-up
                for(node * pin : t->all){CollectTree->all.push_front(pin);}//for delete
            }
        
            return {ReroutInfo{CollectTree,netgrids},false};
        }
    }
    return {ReroutInfo{T,netgrids},true};
}
