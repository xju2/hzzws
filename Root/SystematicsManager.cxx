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
}

SystematicsManager::~SystematicsManager(){
    delete all_nps;
}

SystematicsManager::SystematicsManager(const char* fileName){
    this->readNPs(fileName);
}

void SystematicsManager::readNPs(const char* fileName){
    if(all_nps && all_nps->size() > 1){
        std::cout<<"SystematicsManager HAVE a set of NPs!"<< std::endl;
        return;
    }
    all_nps = new std::vector<TString>();
    std::ifstream ifile(fileName, std::ifstream::in); 
    while (!ifile.eof() && ifile.good()){
        TString np_name;
        ifile >> np_name;
        if (np_name[0] != '#') {
            all_nps ->push_back(np_name);
        }
    }
    std::cout<<"SystematicsManager reads in "<< all_nps->size() <<" NPs"<<endl;

}

void SystematicsManager::add_sys(Sample* sample){

    for(auto np : *all_nps){
        std::cout<<"Name of NP: "<< np <<std::endl;
        sample -> addShapeSys( np );
        sample -> addNormSys(  np );
    }
}

