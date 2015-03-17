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
#include <iostream>
#include <map>
#include <stdexcept>

Combiner::Combiner(const char* _name, const char* _configName):
    name(_name)
{
    cout<<" name: "<< _name << endl;
  readConfig(_configName);
}

Combiner::~Combiner(){

}

void Combiner::readConfig(const char* configName){
    ifstream file(configName, ifstream::in);
    string line;
    map<string, map<string, string> > all_dic;
    int lineCount = 0;
    map<string, string> section_dic;
    string section_name;
    while ( getline(file, line) ){ 
        if( line[0] == '[' ){
            if( lineCount < 1 ){
                section_name = string(line.begin()+1, line.end()-1);
            }else{
                all_dic[section_name] = section_dic;
                section_dic.clear();
                section_name = string(line.begin()+1, line.end()-1);
            }
        }else{
            istringstream iss(line);
            string tagName;
            if( getline( iss, tagName, '=') ){
                string token;
                getline( iss, token,'=');
                section_dic[tagName] = token;
            }
        }
        lineCount ++ ;
    }
    all_dic[section_name] = section_dic;  //pick up the last section
    printDic(all_dic);
}

void Combiner::printDic(map<string, map<string, string> >& all_dic){
    for(auto& kv : all_dic){
        cout << "section: " << kv.first << endl;
        for(auto& sec : kv.second){
            cout<< "\t " << sec.first <<" = " << sec.second <<endl;
        }
    }
}
