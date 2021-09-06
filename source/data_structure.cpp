#include "../header/data_structure.hpp"


//---------------------MasterCell---------------------
MasterCell::MasterCell(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCell)
{
    std::string name;
    is >> name >> name;
    #ifdef PARSER_TEST
        std::cout<<name<<"\n";
    #endif
    int pinNum;
    int blkNum;
    is >> pinNum >> blkNum;
    mCell.insert({name,this});
    for(int i = 0;i<pinNum;i++){
        std::string pinName ;
        std::string metal ;
        is >> pinName >> pinName >> metal;
        int layer = std::stoi(metal.substr(1));
        pins.insert({pinName,layer});
        #ifdef PARSER_TEST
            std::cout<<pinName<<" M"<<layer<<" \n";
        #endif
    }
    for(int i = 0;i<blkNum;i++){
        std::string blkName;
        std::string metal;
        int demand;
        is >> blkName >> blkName >> metal >> demand;
        int layer = std::stoi(metal.substr(1));
        blkgs.insert({blkName,{layer,demand}});
        #ifdef PARSER_TEST
            std::cout<<blkName<<" M"<<layer<<" "<<demand<<" \n";
        #endif
    }

}

//---------------------CellInst---------------------
CellInst::CellInst(std::ifstream&is,std::unordered_map<std::string,MasterCell*>&mCells,std::unordered_map<std::string,CellInst*>&CellInsts)
{
	std::string tmp;
	std::string cell_name;
    std::string m_cell_name;
    std::string type;
    is >> tmp >> cell_name >> m_cell_name;
    
	name = cell_name;
	mCell = mCells.find(m_cell_name)->second;
    is >> row >> col >> type;
    Movable = (type=="Movable");
    #ifdef PARSER_TEST
        std::cout<<cell_name<<" "<<m_cell_name<<" "<<row<<" "<<col<<" "<<Movable<<"\n";
    #endif
    CellInsts.insert({cell_name,this});
    vArea=-1;//defalut no voltageArea constraint
}

//---------------------Net---------------------
Net::Net(std::ifstream&is,std::unordered_map<std::string,CellInst*>&CellInsts,std::unordered_map<std::string,Net*>&Nets)    
{

    int pinNum;
    std::string LayerCstr;
    is >> netName >> netName >> pinNum >> LayerCstr >> weight;
    if(LayerCstr=="NoCstr")
        minLayer = 1;
    else
        minLayer = std::stoi(LayerCstr.substr(1));

    Nets.insert({netName,this});
    #ifdef PARSER_TEST
        std::cout<<netName<<" "<<pinNum<<" "<<LayerCstr<<" "<<weight<<"\n";
    #endif

    for(int i = 0;i<pinNum;i++){
        std::string info;
        std::string cellName;
        std::string pinName;
        is >> info >>info;
        int index=info.find('/');
        cellName = info.substr(0,index);
        pinName = info.substr(index+1);
        CellInst* Cell = CellInsts.find(cellName)->second;
		Cell->nets.push_back(this);//may duplicate!!!!

        net_pins.push_back({Cell,pinName});
        #ifdef PARSER_TEST
            std::cout<<cellName<<" "<<pinName<<"\n";
        #endif
        //std::cout<<cellName<<" "<<pinName<<"\n";
    }
}


void CellInst::fixCell(){
	if(vArea == -1) Movable = false;
}

void Net::updateFixedBoundingBox(){
	fixedBoundingBox = std::vector<int>{INT32_MAX, INT32_MIN, INT32_MAX, INT32_MIN};
	bool existFixed = false;
	for(auto& pin : net_pins){
		if(pin.first->Movable) continue;
		existFixed = true;
		fixedBoundingBox[0] = min(fixedBoundingBox[0], pin.first->row);
		fixedBoundingBox[1] = max(fixedBoundingBox[1], pin.first->row);
		fixedBoundingBox[2] = min(fixedBoundingBox[2], pin.first->col);
		fixedBoundingBox[3] = max(fixedBoundingBox[3], pin.first->col);
	}
	
	if(!existFixed) fixedBoundingBox.resize(0);
}

void CellInst::updateOptimalRegion(){
	originalRow = row;
	originalCol = col;
	initRow = row;
	initCol = col;
	
	std::vector<int> regionCol, regionRow;
	for(auto& net : nets){
		if(!net->fixedBoundingBox.size()) continue;
		regionRow.push_back(net->fixedBoundingBox[0]);
		regionRow.push_back(net->fixedBoundingBox[1]);
		regionCol.push_back(net->fixedBoundingBox[2]);
		regionCol.push_back(net->fixedBoundingBox[3]);
	}

	sort(regionRow.begin(), regionRow.end());
	sort(regionCol.begin(), regionCol.end());

	if(!regionCol.size() || !regionRow.size()){
		//std::cout << "*";
		//optimalRegion.resize(0);
		optimalRegion.resize(4);
		optimalRegion[0] = row;
		optimalRegion[1] = row;
		optimalRegion[2] = col;
		optimalRegion[3] = col;
	}else{
		optimalRegion.resize(4);
		optimalRegion[0] = regionRow[regionRow.size() / 2 - 1];
		optimalRegion[1] = regionRow[regionRow.size() / 2];
		optimalRegion[2] = regionCol[regionCol.size() / 2 - 1];
		optimalRegion[3] = regionCol[regionCol.size() / 2];
	}
}

std::vector<int> CellInst::generatefineOptimalRegion(){
	std::vector<int> regionCol, regionRow;
	for(auto& net : nets){
		net->cells_row.erase(net->cells_row.find(row));
		if(net->cells_row.size()){
			regionRow.push_back(*net->cells_row.begin());
			regionRow.push_back(*net->cells_row.rbegin());
		}
		net->cells_row.insert(row);



		net->cells_col.erase(net->cells_col.find(col));
		if(net->cells_col.size()){
			regionCol.push_back(*net->cells_col.begin());
			regionCol.push_back(*net->cells_col.rbegin());
		}
		net->cells_col.insert(col);
	}


	sort(regionRow.begin(), regionRow.end());
	sort(regionCol.begin(), regionCol.end());

	std::vector<int> fineoptimalRegion(4);
	if(!regionCol.size() || !regionRow.size()){
		fineoptimalRegion[0] = row;
		fineoptimalRegion[1] = row;
		fineoptimalRegion[2] = col;
		fineoptimalRegion[3] = col;
	}else{
		fineoptimalRegion[0] = regionRow[regionRow.size() / 2 - 1];
		fineoptimalRegion[1] = regionRow[regionRow.size() / 2];
		fineoptimalRegion[2] = regionCol[regionCol.size() / 2 - 1];
		fineoptimalRegion[3] = regionCol[regionCol.size() / 2];
	}


	return fineoptimalRegion;
}

void Net::updateCellsCoor(){
	for(auto& pin : net_pins){
		cells_row.insert(pin.first->row);
		cells_col.insert(pin.first->col);
	}
};


void CellInst::expandOptimalReion(int x, int rowBegin, int rowEnd, int colBegin, int colEnd){
	if(optimalRegion.empty()) return ;
	optimalRegion[0] = max(optimalRegion[0] - x, rowBegin);
	optimalRegion[1] = min(optimalRegion[1] + x, rowEnd);
	optimalRegion[2] = max(optimalRegion[2] - x, colBegin);
	optimalRegion[3] = min(optimalRegion[3] + x, colEnd);
}


int Net::costToBox(int row, int col){
	if(fixedBoundingBox.empty()) return 0;	
	int dist = 0;

	if(!(row > fixedBoundingBox[0] && row < fixedBoundingBox[1]))
		dist += min(abs(row - fixedBoundingBox[0]), abs(row - fixedBoundingBox[1]));
	if(!(col > fixedBoundingBox[2] && col < fixedBoundingBox[3]))
		dist += min(abs(col - fixedBoundingBox[2]), abs(col - fixedBoundingBox[3]));

	return dist * (int)(weight * 100);
}

bool CellInst::inOptimalRegion(int row, int col){
	//if(!optimalRegion.size()) return true;
	if(!optimalRegion.size() ||
		!(row >= optimalRegion[0] && row <= optimalRegion[1]) || 
		!(col >= optimalRegion[2] && col <= optimalRegion[3]))
	return false;
	return true;
}
