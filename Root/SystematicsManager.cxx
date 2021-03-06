#include "Hzzws/SystematicsManager.h"
#include <fstream>
using namespace std;

SystematicsManager::SystematicsManager(){
    all_nps = new std::vector<TString>();
}

SystematicsManager::~SystematicsManager(){
    delete all_nps;
}

SystematicsManager::SystematicsManager(const char* fileName){
    all_nps = new std::vector<TString>();
    cout << "Systematic list: " << fileName << endl;
    this->readNPs(fileName);
}

void SystematicsManager::readNPs(const char* fileName){
    if(all_nps && all_nps->size() > 1){
        std::cout<<"SystematicsManager HAVE a set of NPs!"<< std::endl;
        return;
    }
    ifstream ifile(fileName, ifstream::in); 
    while (!ifile.eof()){
        TString np_name;
        ifile >> np_name;
        if (ifile.good() && np_name[0] != '#') {
            all_nps ->push_back(np_name);
        }
    }
    cout << "SystematicsManager reads in "<< all_nps->size() <<" NPs" << endl;

}

vector<TString>* SystematicsManager::add_sys(SampleBase* sample){
    if (all_nps->size() < 1) return NULL;
    auto* nps_vec = new vector<TString>();
    for(unsigned int i=0; i < all_nps->size(); i++){
        TString& np = all_nps->at(i);
        bool has_norm = sample->addNormSys(  np );
        bool has_shape = sample->addShapeSys( np );
        if(has_shape || has_norm){
            nps_vec ->push_back( np );
        }
    }
    return nps_vec;
}


