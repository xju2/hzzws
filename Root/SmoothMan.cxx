// =====================================================================================
// 
//       Filename:  SmoothMan.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  03/13/2015 18:21:53
//       Revision:  none
//       Compiler:  g++
// 
// =====================================================================================
#include "Hzzws/SmoothMan.h"
#include <fstream>

SmoothMan::SmoothMan(const char* configFile){
    std::ifstream ifile(configFile, std::ifstream::in);     
    while(!ifile.eof() && ifile.good()){

    }
}

SmoothMan::~SmoothMan(){

}
