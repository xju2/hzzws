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
    ws_name_(_name),
    simpdf_name("simPdf"),
    mainSectionName("main")
{
    cout<<"workspace name: "<< ws_name_ << endl;
    readConfig(_configName);
}

Combiner::~Combiner(){
    delete sysMan;
    for(auto& sample : allSamples) delete sample.second;
}

void Combiner::readConfig(const char* configName){
    Helper::readConfig(configName, '=', all_dic);
    Helper::printDic<string>(all_dic);

    ///////////////////////////////////
    // load the data
    ///////////////////////////////////
    //string input_data = all_dic["data"]["file_path"];  //TODO
    //data_file = TFile::Open(input_data.c_str(), "READ"); //TODO

    string file_path = "./";
    try {
        file_path = all_dic.at(mainSectionName).at("fileDir");
    } catch (const out_of_range& oor) {
        cout << "'fileDir' not specific, look in current directory" << endl;
    }

    ///////////////////////////////////
    // check if normalization table exists
    ///////////////////////////////////
    try {
        string table_name = all_dic.at(mainSectionName)["normalization"];
        table_name = file_path + "/" + table_name; 
        Helper::readNormTable(table_name.c_str(), all_norm_dic_);
        Helper::printDic<double>(all_norm_dic_);
    } catch (const out_of_range& oor) {
        cout << "'normalization' not specific, get expected from histogram." << endl;
    }

    ///////////////////////////////////
    //load samples
    ///////////////////////////////////
    for (auto& sample : all_dic.at("samples")) {
        cout << sample.second << endl;
        vector<string> tokens;
        Helper::tokenizeString(sample.second, ',', tokens);
        // 0: input_path, 1: shape_sys_path, 2: norm_sys_path, 3: name, 4: useMCConstraint;
        auto* newsample = new Sample(tokens.at(3).c_str(), sample.first.c_str(), 
                tokens.at(0).c_str(), tokens.at(1).c_str(), tokens.at(2).c_str(), file_path.c_str());
        if (tokens.size() > 4){
            newsample->setMCCThreshold(atof(tokens.at(4).c_str()));
        }
        if (all_norm_dic_.size() > 0) {
            try {
                newsample->setNormalizationMap(all_norm_dic_.at(sample.first));
            } catch (const out_of_range&) {
                cout << sample.first << " does not included in normalization table." << endl;
            }
        }
        allSamples[sample.first] = newsample;
    }

    ///////////////////////////////////
    //load systematics
    ///////////////////////////////////
    auto& job_dic = all_dic.at(mainSectionName);
    try {
        string NP_list = job_dic.at("NPlist") ;
        sysMan = new SystematicsManager(Form("%s/%s", file_path.c_str(), NP_list.c_str()));
    } catch (const out_of_range& oor) {
        sysMan = new SystematicsManager();
        cerr << "NPlist is not defined, meaning no systematics used" << endl;
    }

    auto* workspace = new RooWorkspace(ws_name_.Data());
    ///////////////////////////////////
    //add categories
    ///////////////////////////////////
    istringstream iss_cat( all_dic.at(mainSectionName)["categories"] );
    string category_name;
    RooCategory channelCat("channelCat", "channelCat");
    map<string, RooAbsPdf*> pdfMap;
    int catIndex = 0;
    char delim = ',';
    while( getline( iss_cat, category_name, delim )) 
    {
        boost::algorithm::trim(category_name);
        cout <<"On category: "<< category_name <<endl;

        string mcsets = findCategoryConfig(category_name, "mcsets");
        if(mcsets == "") continue;

        vector<string> mcsets_names;
        Helper::tokenizeString( mcsets, ',', mcsets_names) ;
        Category* category = new Category(category_name);
        ///////////////////////////////////
        // add observable  (not for 2D yet)
        ///////////////////////////////////
        obs.removeAll();
        string obs_str = findCategoryConfig(category_name, "observable");
        vector<string> obsPara;
        Helper::tokenizeString(obs_str, delim, obsPara);
        // 0: obs_name, 1: nbins_str, 2: low_str, 3: hi_str;
        RooRealVar *var = nullptr;
        bool use_adaptive_binning = false;
        if (obsPara.size() > 3){
            var = new RooRealVar(obsPara.at(0).c_str(), obsPara.at(0).c_str(),
                    atof(obsPara.at(2).c_str()), atof(obsPara.at(3).c_str())); 
            var->setBins(atoi(obsPara.at(1).c_str()));
        } 
        else { 
            // adaptive binning, only set range
            // The actual binning will be choosen for different input histograms
            var = new RooRealVar(obsPara.at(0).c_str(), obsPara.at(0).c_str(),
                    atof(obsPara.at(1).c_str()), atof(obsPara.at(2).c_str()));
            use_adaptive_binning = true;
        }
        obs.add(*var);
        category->setObservables(obs);

        for(auto& mcset_name : mcsets_names){
            Sample* sample = getSample(mcset_name);
            if(use_adaptive_binning) sample ->useAdaptiveBinning();
            if(sample) category->addSample(sample, sysMan);
        }

        string catName(Form("%sCat",category_name.c_str()));
        channelCat.defineType(catName.c_str(), catIndex++);
        //////////////////////////////////////////////////////////////
        // Category will sum individual sample's pdf and add constraint terms
        //////////////////////////////////////////////////////////////
        RooAbsPdf* final_pdf = category ->getPDF();
        string final_pdf_name(final_pdf->GetName()); 
        // final_pdf->getVal(); // To increast the fitting speed??
        workspace ->import(*var, RooFit::RecycleConflictNodes());
        workspace ->import(*final_pdf, RooFit::RecycleConflictNodes(), RooFit::Silence());

        //////////////////////////////////////////////////////////////
        // Add nuisance parameters and global observables for MC statistic
        //////////////////////////////////////////////////////////////
        TIter next_global(category->getGlobal().createIterator());
        TIter next_nuis(category->getNuisance().createIterator());
        RooAbsReal* global; 
        RooAbsReal* nuisance;
        while ((
                    global = (RooAbsReal*) next_global(),
                    nuisance = (RooAbsReal*) next_nuis()
                    )) 
        {
            nuisanceSet.add(*nuisance);
            globalobsSet.add(*global);
        }
        // delete var;
        delete category;
        pdfMap[catName] = workspace->pdf(final_pdf_name.c_str());
    }
    auto* simPdf = new RooSimultaneous(simpdf_name.c_str(), simpdf_name.c_str(), pdfMap, channelCat);
    workspace ->import(*simPdf, RooFit::Silence());
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

    //////////////////////////////
    // define nusiance parameters and global observables
    //////////////////////////////
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
    ws->import(modelConfig);
}
