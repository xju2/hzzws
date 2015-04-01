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
#include "RooStats/ModelConfig.h"

#include "Hzzws/Helper.h"

Combiner::Combiner(const char* _name, const char* _configName):
    m_name(_name),
    simpdf_name("simPdf"),
    mainSectionName("main")
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
    Helper::readConfig(configName, '=', all_dic);
    Helper::printDic(all_dic);

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
        file_path = all_dic.at(mainSectionName).at("fileDir");
    }catch(const out_of_range& oor){
        cout << "'fileDir' not specific, look in current directory" << endl;
    }
    for(auto& sample : all_dic.at("samples")){
        cout << sample.second << endl;
        vector<string> tokens;
        Helper::tokenizeString(sample.second, ',', tokens);
        // 0: input_path, 1: shape_sys_path, 2: norm_sys_path, 3: name;
        allSamples[sample.first] = new Sample(tokens.at(3).c_str(), sample.first.c_str(), 
                tokens.at(0).c_str(), tokens.at(1).c_str(), tokens.at(2).c_str(), file_path.c_str());
    }

    ///////////////////////////////////
    //load systematics
    ///////////////////////////////////
    auto& job_dic = all_dic.at(mainSectionName);
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
    string obs_str = all_dic.at(mainSectionName)["observable"];
    
    char delim = ',';
    vector<string> obsPara;
    Helper::tokenizeString(obs_str, delim, obsPara);
    // 0: obs_name, 1: nbins_str, 2: low_str, 3: hi_str;
    RooRealVar var(obsPara.at(0).c_str(), obsPara.at(0).c_str(),
            atoi(obsPara.at(2).c_str()), atoi(obsPara.at(3).c_str())); 
    var.setBins(atoi(obsPara.at(1).c_str()));
    cout << var.GetName() << endl;
    obs.add(var);

    auto* workspace = new RooWorkspace(m_name.Data());
    ///////////////////////////////////
    //add categories
    ///////////////////////////////////
    istringstream iss_cat( all_dic.at(mainSectionName)["categories"]);
    string category_name;
    RooCategory channelCat("channelCat", "channelCat");
    map<string, RooAbsPdf*> pdfMap;
    int catIndex = 0;
    while( getline( iss_cat, category_name, delim )){
        boost::algorithm::trim(category_name);
        cout <<"On category: "<< category_name <<endl;

        string mcsets = findCategoryConfig(category_name, "mcsets");
        if(mcsets == "") continue;

        vector<string> mcsets_names;
        Helper::tokenizeString( mcsets, ',', mcsets_names) ;
        Category* category = new Category(category_name);
        category->setObservables(obs);

        for(auto& mcset_name : mcsets_names){
            Sample* sample = getSample(mcset_name);
            if(sample) category->addSample(sample, sysMan);
        }

        string catName(Form("%sCat",category_name.c_str()));
        channelCat.defineType(catName.c_str(), catIndex++);
        // Category will sum individual sample's pdf and add constraint term
        RooAbsPdf* final_pdf = category ->getPDF();
        string final_pdf_name(final_pdf->GetName()); 
        workspace ->import(*final_pdf, RooFit::RecycleConflictNodes());
        delete category;
        pdfMap[catName] = workspace->pdf(final_pdf_name.c_str());
    }
    auto* simPdf = new RooSimultaneous(simpdf_name.c_str(), simpdf_name.c_str(), pdfMap, channelCat);
    workspace ->import(*simPdf);
    this->configWorkspace(workspace);
    workspace ->writeToFile("combined.root");
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
        try{
            token = all_dic.at(mainSectionName).at(name);
        }catch(const out_of_range& orr){
            cerr << "ERROR (Combiner::findCategoryConfig) : no |" << name << "| found in |" << cat_name << "|" << endl;
            return "";
        }
    }
    return token;
}

void Combiner::configWorkspace(RooWorkspace* ws)
{
    //////////////////// 
    // define set
    //////////////////// 
    RooArgSet poiSet;
    poiSet.add(* ws->var("mu") );

    RooArgSet nuisanceSet;
    RooArgSet globalobsSet;
    for(auto& np : *(sysMan->getNP()))
    {
        string nuisanceName(Form("alpha_%s", np.Data()));
        string globalName  (Form("nom_%s", np.Data()));
        nuisanceSet.add( *ws->var(nuisanceName.c_str()) );
        globalobsSet.add( *ws->var(globalName.c_str()) );
    }
    ws->defineSet("obs", obs);
    ws->defineSet("nuisance", nuisanceSet);
    ws->defineSet("globalobs", globalobsSet);
    ws->defineSet("poi", poiSet);

    //////////////////// 
    // make model config 
    //////////////////// 

    RooStats::ModelConfig modelConfig("ModelConfig","H->4l example");
    modelConfig.SetWorkspace           ( *ws );

    modelConfig.SetPdf(*ws->pdf(simpdf_name.c_str()));
    modelConfig.SetParametersOfInterest( *ws->set("poi") );
    modelConfig.SetObservables         ( *ws->set("obs") );
    modelConfig.SetNuisanceParameters  ( *ws->set("nuisance") );
    modelConfig.SetGlobalObservables   ( *ws->set("globalobs") );
    ws ->import(modelConfig);
}
