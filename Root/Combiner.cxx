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
#include <boost/algorithm/string.hpp>

#include <RooRealVar.h>
#include <RooAddPdf.h>
#include <RooWorkspace.h>
#include <RooCategory.h>

Combiner::Combiner(const char* _name, const char* _configName):
    m_name(_name)
{
    cout<<" name: "<< m_name << endl;
    readConfig(_configName);
}

Combiner::~Combiner(){
    delete sysMan;
    for(auto* cat : allCategories)  delete cat;
    for(auto& sample : allSamples) delete sample.second;
}

void Combiner::readConfig(const char* configName){
    ifstream file(configName, ifstream::in);
    string line;
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
        }else if( line[0] == '#' ){
            continue;
        }else{
            char delim = '=';
            auto* tokens = tokenizeString(line, delim);
            section_dic[tokens->at(0)] = tokens->at(1);
            delete tokens;
        }
        lineCount ++ ;
    }
    all_dic[section_name] = section_dic;  //pick up the last section
    //printDic();

    ///////////////////////////////////
    //load the data
    ///////////////////////////////////
    //string input_data = all_dic["data"]["file_path"];  //TODO
    //data_file = TFile::Open(input_data.c_str(), "READ"); //TODO

    ///////////////////////////////////
    //load samples
    ///////////////////////////////////
    string file_path = "./";
    try{
        file_path = all_dic.at("jobs").at("fileDir");
    }catch(const out_of_range& oor){
        //do nothing
    }
    for(auto& sample : all_dic.at("samples")){
        char delim = ',';
        cout << sample.second << endl;
        auto* tokens = tokenizeString(sample.second, delim);
        // 0: input_path, 1: shape_sys_path, 2: norm_sys_path, 3: name;
        allSamples[sample.first] = new Sample(tokens->at(3).c_str(), tokens->at(0).c_str(),
                tokens->at(1).c_str(), tokens->at(2).c_str(), file_path.c_str());
        delete tokens;
    }

    ///////////////////////////////////
    //load systematics
    ///////////////////////////////////
    auto& job_dic = all_dic.at("jobs");
    try{
        string NP_list = job_dic.at("NPlist") ;
        sysMan = new SystematicsManager(NP_list.c_str());
    }catch(const out_of_range& oor){
        sysMan = new SystematicsManager();
        cerr << "NPlist is not defined" << endl;
    }
    ///////////////////////////////////
    //add observable  (not for 2D yet)
    ///////////////////////////////////
    string obs_str = all_dic.at("jobs")["observable"];
    
    char delim = ',';
    auto* obsPara = tokenizeString(obs_str, delim);
    // 0: obs_name, 1: nbins_str, 2: low_str, 3: hi_str;
    RooRealVar var(obsPara->at(0).c_str(), obsPara->at(0).c_str(),
            atoi(obsPara->at(2).c_str()), atoi(obsPara->at(3).c_str())); 
    var.setBins(atoi(obsPara->at(1).c_str()));
    cout << var.GetName() << endl;
    obs.add(var);
    delete obsPara;

    ///////////////////////////////////
    //add categories
    ///////////////////////////////////
    istringstream iss_cat( all_dic.at("jobs")["categories"]);
    string category_name;
    RooCategory channelCat("channelCat", "channelCat");
    map<string, RooAbsPdf*> pdfMap;
    int catIndex = 0;
    while( getline( iss_cat, category_name, delim )){
        boost::algorithm::trim(category_name);
        cout <<"On category: "<< category_name <<endl;

        string signals = findCategoryConfig(category_name, "signal");
        if(signals == "") continue;

        string backgrounds = findCategoryConfig(category_name, "background");
        if(backgrounds == "") continue;

        auto* signal_names = tokenizeString( signals, ',' ) ;
        auto* bkg_names = tokenizeString( backgrounds, ',' );
        Category* category = new Category(category_name);
        category->setObservables(obs);

        bool is_signal_sample = true;
        for(auto& signal_name : *signal_names){
            Sample* sample = getSample(signal_name);
            if(sample) category->addSample(sample, is_signal_sample, sysMan);
        }
        is_signal_sample = false;
        for(auto& bkg_name : *bkg_names){
            Sample* sample = getSample(bkg_name);
            if(sample) category->addSample(sample, is_signal_sample, sysMan);
        }
        string catName(Form("%sCat",category_name.c_str()));
        channelCat.defineType(catName.c_str(), catIndex++);
        pdfMap[catName] = category ->getPDF();
    }
    auto* simPdf = new RooSimultaneous("simPdf","simPdf", pdfMap, channelCat);
    auto* workspace = new RooWorkspace(m_name.Data());
    workspace ->import(*simPdf);
    workspace ->writeToFile("combined.root");
}

// void Combiner::toWorkspace(){
// }

void Combiner::printDic()
{
    for(auto& kv : all_dic){
        cout << "section: |" << kv.first << "|" << endl;
        for(auto& sec : kv.second){
            cout<< "\t |" << sec.first <<"| = |" << sec.second << "|" << endl;
        }
    }
}


vector<string>* Combiner::tokenizeString(string& str, char delim)
{
    istringstream iss(str);
    vector<string>* tokens = new vector<string>();
    string token;
    while ( getline(iss, token, delim) ){
        boost::algorithm::trim(token);
        tokens ->push_back(token);
    }
    return tokens;
}

Sample* Combiner::getSample(string& name)
{
    Sample* sample = NULL;
    try{
        sample = allSamples.at(name);
    }catch(const out_of_range& oor){
        cerr << "ERROR (Combiner::getSample): Sample " << name << " not defined! " << endl;
    }
    return sample;
}

string Combiner::findCategoryConfig(string& cat_name, const char* name)
{
    string token;
    try{
        token = all_dic.at(cat_name).at(name);
    }catch(const out_of_range& orr){ 
        //not defined in category, search in jobs
        try{
            token = all_dic.at("jobs").at(name);
        }catch(const out_of_range& orr){
            cerr << "ERROR (Combiner::findCategoryConfig) : no |" << name << "| found in |" << cat_name << "|" << endl;
            return "";
        }
    }
    return token;
}
