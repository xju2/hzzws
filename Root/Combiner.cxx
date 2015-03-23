// 
//    Description:  Combine each categories
// 
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

    //load the data
    string input_data = all_dic["data"]["file_path"];
    data_file = TFile::Open(input_data.c_str(), "READ");

    //load samples
    for(auto& sample : all_dic.at("samples")){
        string parameters = sample.second;
        istringstream iss(parameters);
        string input_path, shape_sys_path, norm_sys_path, name;
        getline( iss, input_path, ',');
        getline( iss, shape_sys_path, ',');
        getline( iss, norm_sys_path, ',');
        getline( iss, name, ',');
        cout<<input_path<< shape_sys_path<< norm_sys_path<<name<<endl;
    }
      
}

void Combiner::printDic(map<string, map<string, string> >& all_dic){
    for(auto& kv : all_dic){
        cout << "section: " << kv.first << endl;
        for(auto& sec : kv.second){
            cout<< "\t " << sec.first <<" = " << sec.second <<endl;
        }
    }
}

