// =====================================================================================
// 
//       Filename:  Combiner.cxx
// 
//    Description:  Combine each categories
// 
//        Version:  1.0
//        Created:  03/16/2015 21:13:34
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#include "Hzzws/Combiner.h"
#include <fstream>
#include <string>
#include <sstream>

Combiner::Combiner(const char* _name, const char* _configName):
    name(_name)
{
  readConfig(_configName);
}

Combiner::~Combiner(){

}

void Combiner::readConfig(const char* configName){
    ifstream file(configName, ifstream::in);
    string line;
    while ( getline(file, line) ){ 
        istringstream iss(line);
        string tagName;
        if( getline( iss, tagName, '=') ){
            cout<< tagName <<endl;
            if( tagName == "data" ){
                string token;
                getline( iss, token,'=');
            }
        }
    }
}
