// =====================================================================================
// 
//       Filename:  SystematicsManager.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  03/13/2015 17:21:43
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
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

void SystematicsManager::add_sys(Sample* sample){
    if (all_nps->size() < 1) return ;
    for(unsigned int i=0; i < all_nps->size(); i++){
        TString& np = all_nps->at(i);
        std::cout<<"Name of NP: "<< np <<std::endl;
        sample -> addShapeSys( np );
        sample -> addNormSys(  np );
    }
}

