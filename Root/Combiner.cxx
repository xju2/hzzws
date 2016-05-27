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
#include "Hzzws/SampleFactory.h"
#include "Hzzws/CoefficientFactory.h"
#include "Hzzws/CBGauss.h"
#include "Hzzws/SampleKeys.h"
#include "Hzzws/RooStatsHelper.h"

Combiner::Combiner(const char* _name, const char* _configName):
    ws_name_(_name),
    config_name_(_configName)
{
    cout<<"workspace name: "<< ws_name_ << endl;
    cout<<"configuration name: " << config_name_ << endl;
    file_path_ = "./";
    data_chain = nullptr;
    workspace = nullptr;
}

Combiner::~Combiner(){
    if(workspace) delete workspace;
    if(data_chain) delete data_chain;
    if(sysMan) delete sysMan;
    for(auto& coef : allCoefficients) delete coef.second;
}

void Combiner::readConfig(const char* configName)
{
  Helper::readConfig(configName, '=', all_dic);
  Helper::printDic<string>(all_dic);
  workspace = new RooWorkspace("combined");

  auto& job_dic = all_dic.at("main");

  try {
    file_path_ = job_dic.at("fileDir")+"/";
  } catch (const out_of_range& oor) {
    cout << "'fileDir' not specific, look in current directory" << endl;
  }
  std::cout<<"looking for files in "<<file_path_<<std::endl;
  Helper::getInputPath(file_path_);

  ///////////////////////////////////
  //load coefficients
  ///////////////////////////////////

  std::cout<<"reading in coefficients"<<std::endl;
  if (all_dic.find("coefficients")==all_dic.end()){
    log_err("No coefficients provided! aborting");
     exit(-1);
  }
  for (auto& sample : all_dic.at("coefficients")) {
    std::cout<<"on coef of sample "<<sample.first<<std::endl;

    //format
    // {sampleName} = {coefficient formula} : {{args}}

    auto newcoef = CoefficientFactory::CreateCoefficient(sample.second);

    if (!newcoef){
      log_err("Failed to add coefficient for sample %s! Critical failure: aborting Combiner!", sample.first.c_str()); exit(-1);
    }

    allCoefficients[sample.first] = newcoef;
  }

  cout << "Added " << allCoefficients.size() << " Coefficients" << endl;

  ///////////////////////////////////
  //load systematics
  ///////////////////////////////////
  try {
    string NP_list = job_dic.at("NPlist") ;
    sysMan = new SystematicsManager(Form("%s/%s", file_path_.c_str(), NP_list.c_str()));
  } catch (const out_of_range& oor) {
    sysMan = new SystematicsManager();
    cerr << "NPlist is not defined, meaning no systematics used" << endl;
  }

  ///////////////////////////////////
  // load the data
  ///////////////////////////////////
  try {
    string input_data = job_dic.at("data");
    data_chain = Helper::loader(input_data.c_str(), "tree_incl_all");
    cout << "you are adding DATA!!" << endl;
  } catch (const out_of_range& oor) {
    cout << "no datai: I am blind!!" << endl;
  }

  ///////////////////////////////////
  //add categories
  ///////////////////////////////////
  istringstream iss_cat( job_dic["categories"] );
  string category_name;
  RooCategory channelCat("channelCat", "channelCat");
  map<string, RooAbsPdf*> pdfMap;
  map<string, RooDataSet*> dataMap;
  int catIndex = 0;
  char delim = ',';

  while( getline( iss_cat, category_name, delim ))
  {
    boost::algorithm::trim(category_name);
    cout << "====================================" << endl;
    cout <<"On category: "<< category_name << endl;

    string mcsets = findCategoryConfig(category_name, "mcsets");
    if(mcsets == "") {
      log_err("No corresponding MC set! %s will be dropped!", category_name.c_str());
      continue;
    }

    vector<string> mcsets_names;
    Helper::tokenizeString( mcsets, ',', mcsets_names) ;
    Category* category = new Category(category_name);

    ///////////////////////////////////
    // add observables
    ///////////////////////////////////
    RooArgSet ch_obs_minitree; // observables in mini-tree
    RooArgSet ch_obs_ws;       // observables in workspace
    string obs_str = findCategoryConfig("observables", category_name);
    bool use_adaptive_binning = false;
    getObservables(obs_str, ch_obs_ws, ch_obs_minitree, use_adaptive_binning);
    category->setObservables(ch_obs_ws);
    cout << ch_obs_ws.getSize() << " observable in " << category_name << endl;
    obs_.add(ch_obs_minitree,true); // add observable in this channel to overall obs set.
    obs_ws_.add(ch_obs_ws,true); // add observable in this channel to overall obs set.
    //
    // Add Samples in listed in mcsets
    // Samples are created on-the-fly
    //
    for(auto& mcset_name : mcsets_names){
        string sampleargs = findCategoryConfig(category_name, mcset_name);
        SampleBase* sample = createSample(mcset_name, sampleargs);
        if(sample) {
            category->addSample(sample, sysMan);
            if(use_adaptive_binning){
                sample->useAdaptiveBinning();
            }
        } else {
            log_err("Failed to add mcset %s! IGNORE THIS SAMPLE!",mcset_name.c_str());
            continue;
        }
    }

    cout <<"End add Samples: "<< category_name << endl;

    string catName(Form("%sCat",category_name.c_str()));
    channelCat.defineType(catName.c_str(), catIndex++);
    //////////////////////////////////////////////////////////////
    // Category will sum individual sample's pdf and add constraint terms
    //////////////////////////////////////////////////////////////
    RooAbsPdf* final_pdf = category->getPDF();
    string final_pdf_name(final_pdf->GetName());
    // final_pdf->getVal(); // To increast the fitting speed??
    std::cout<<"final_pdf @: "<<final_pdf<<std::endl;
    final_pdf->Print();

    std::cout<<"now to import it.."<<std::endl;

    std::cout<<"here is the workspace before import:"<<std::endl;
    workspace->Print();

    std::cout<<"\n ok now I'm going to import my new pdf"<<std::endl;
    workspace ->import(*final_pdf, RooFit::RecycleConflictNodes());

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
        nuisanceSet_.add(*nuisance);
        globalobsSet_.add(*global);
    }

    pdfMap[catName] = workspace->pdf(final_pdf_name.c_str());

    ////////////////////////////////////////////
    //  Add Data (per channel)
    ////////////////////////////////////////////
    if(data_chain){
      std::cout<<"adding data in category "<<category_name<<std::endl;
        string cut = findCategoryConfig("cut", category_name);
        cout << "Cut on data: |" << cut << "|" << endl;
        cout << "Observables in minitree: " << ch_obs_minitree.getSize() << endl;
        string obs_data_ch_name(Form("data_%s",category_name.c_str()));
        addCutVariables(ch_obs_minitree, cut);
        RooCmdArg cut_arg = cut==""?RooCmdArg::none():RooFit::Cut(cut.c_str());
        RooDataSet* data_ch = new RooDataSet(obs_data_ch_name.c_str(),
                "data set",
                ch_obs_minitree,
                RooFit::Import(*data_chain),
                cut_arg
                );
        cout << "data: in "<< category_name<< " " << data_ch->sumEntries() << endl;
        dataMap[catName] = data_ch;
    }

    cout <<"End category: "<< category_name << endl;
    cout <<"Summary: "<< endl;
    final_pdf->Print();
    cout << "====================================" << endl;
    delete category;
    delete final_pdf;
  }

  ////////////////////////////////////////////
  //  Add Data //
  ////////////////////////////////////////////
  if(data_chain){
    std::cout<<"adding data in ALL categories"<<std::endl;

    obs_.add(channelCat);
    RooDataSet* obsData = new RooDataSet(
        "obsData",
        "observed data",
        obs_,
        RooFit::Index(channelCat),
        RooFit::Import(dataMap)
        );
    TString orignames(""), newnames("");
    for(auto items : rename_map_) {
      if (orignames!="") orignames+=",";
      if (newnames!="") newnames+=",";
      orignames+=items.first;
      newnames+=items.second;
    }
    workspace->import(*obsData, RooFit::RenameVariable(orignames.Data(),newnames.Data()));
  }

  auto* simPdf = new RooSimultaneous("simPdf", "simPdf", pdfMap, channelCat);
  workspace ->import(*simPdf, RooFit::RecycleConflictNodes(), RooFit::Silence());

  this->configWorkspace(workspace);


  //////////////////////////////////////////////
  // Add Asimov
  // //////////////////////////////////////////

  workspace->saveSnapshot("ParamsBeforeAsimovGeneration",workspace->allVars());

  auto mc = (RooStats::ModelConfig*)workspace->obj("ModelConfig");

  for (auto & opt : all_dic){
    if (opt.first.find("asimov")==std::string::npos) continue;

    TString asimovName=opt.first.c_str();
    asimovName.ReplaceAll(" ","");
    asimovName.ReplaceAll("asimov:","");

    std::cout<<"going to make asimov dataset called: "<<asimovName<<std::endl;

    for (auto & par : opt.second){
      RooRealVar* var = workspace->var(par.first.c_str());
      if (var) 
        var->setVal( atof(par.second.c_str()) );
      else {
        log_err("Trying to build asimov but received an invalid parameter: %s",par.first.c_str());
        exit (-1);
      }
    }

    auto adata = RooStatsHelper::makeUnconditionalAsimov(workspace, mc , asimovName.Data());
    if (!workspace->data(asimovName.Data())) workspace->import(*adata); // currently done inside makeUnconditionalAsimov() so we don't really have to do this
  }

  workspace->loadSnapshot("ParamsBeforeAsimovGeneration");


  //////////////////////////////////////////
  // Done
  /////////////////////////////////////////


  workspace ->writeToFile(ws_name_);
}

Coefficient* Combiner::getCoefficient(string& name)
{
    Coefficient* coef = NULL;
    try{
        coef = allCoefficients.at(name);
    }catch(const out_of_range& oor){
        cerr << "ERROR (Combiner::getCoefficient): Coefficient " << name << " not defined! " << endl;
    }
    return coef;
}

string Combiner::findCategoryConfig(const string& sec_name, const string& key_name)
{
    string token = "";
    try {
        token = all_dic.at(sec_name).at(key_name);
    } catch (const out_of_range& orr) {
        try {
            token = all_dic.at("main").at(key_name);
        } catch (const out_of_range& orr) {
            // Loop over the section names and try to match sec_name
            int ntimes = 0;
            for (auto it : all_dic) {
                if(it.first.find(sec_name) != string::npos){
                    if(it.second.find(key_name) != it.second.end()) {
                        token = it.second.at(key_name);
                    }
                    ntimes ++;
                }
            }
            if(ntimes > 1){
                log_err("%s are found more than twice!", sec_name.c_str());
            }
        }
    }
    return token;
}

void Combiner::configWorkspace(RooWorkspace* ws)
{
    cout << "Configuring the workspace" << endl;

    ws->Print();
    ////////////////////
    // define set
    ////////////////////
    RooArgSet* poiSet = (RooArgSet*)ws->allVars().selectByName("mu*,xs*,XS*,BR*");

    if (poiSet->getSize()==0)
        log_err("I cannot find POI!");

    //////////////////////////////
    // define nusiance parameters and global observables
    //////////////////////////////
    for(auto& np : *(sysMan->getNP()))
    {
        string nuisanceName(Form("alpha_%s", np.Data()));
        string globalName  (Form("nom_%s", np.Data()));
        if (! ws->var(nuisanceName.c_str()) ) {
            log_err("no nusiance parameter: %s in workspace", nuisanceName.c_str());
            continue;
        }
        if (! ws->var(globalName.c_str())) {
            log_err("no global observable: %s in workspace", globalName.c_str());
            continue;
        }
        nuisanceSet_.add( *ws->var(nuisanceName.c_str()) );
        globalobsSet_.add( *ws->var(globalName.c_str()) );
    }
    ws->defineSet("obs", obs_ws_);
    ws->defineSet("nuisance", nuisanceSet_);
    ws->defineSet("globalobs", globalobsSet_);
    ws->defineSet("poi", *poiSet);

    ////////////////////
    // make model config
    ////////////////////
    RooStats::ModelConfig modelConfig("ModelConfig","H->4l example");
    modelConfig.SetWorkspace           ( *ws );
    modelConfig.SetPdf(*ws->pdf("simPdf"));
    modelConfig.SetParametersOfInterest( *ws->set("poi") );
    modelConfig.SetObservables         ( *ws->set("obs") );
    modelConfig.SetNuisanceParameters  ( *ws->set("nuisance") );
    modelConfig.SetGlobalObservables   ( *ws->set("globalobs") );
    ws->import(modelConfig);
    ws->saveSnapshot("nominalGlobs", *ws->set("globalobs"));
    ws->saveSnapshot("nominalNuis", *ws->set("nuisance"));

    // TODO add asimovData and performan S+B and B-only Fit
    cout << "end of configuring workspace" << endl;
    return;
}


void Combiner::combine(){
    readConfig(config_name_.c_str());
    std::cout<<"end of Combiner::combine"<<std::endl;
}

void Combiner::getObservables(
        const string& obs_str,
        RooArgSet& obs_ws, RooArgSet& obs_minitree,
        bool& adaptive)
{
    // @format
    // obs1_name_in_minitree:obs1_name_in_ws nbins low hi; obs2_name_in_minitree:obs2_name_in_ws nbins low hi

    if (obs_str=="COUNT"){//keyword - the only exception to the above
       std::cout<<"detected COUNT: only doing counting in this category, no observables"<<std::endl;
       return;
    }
    vector<string> obs_list;
    Helper::tokenizeString(obs_str, ';', obs_list);
    if (obs_list.size() < 1) {
        log_err("observable input cannot be recognized: %s", obs_str.c_str());
        return ;
    }
    for (auto obs_input : obs_list){
        vector<string> obs_parameters;
        Helper::tokenizeString(obs_input, ',', obs_parameters);
        const string& obs_names = obs_parameters.at(0);
        size_t pos_semi_comma = obs_names.find(':');
        string minitree_name = "";
        string ws_name = "";
        if (pos_semi_comma == string::npos) {
            minitree_name = ws_name = obs_names;
        } else {
            minitree_name = obs_names.substr(0, pos_semi_comma);
            ws_name = obs_names.substr(pos_semi_comma+1, obs_names.size());
        }
        cout <<" MINITREE name: " << minitree_name << endl;
        cout <<" WSNAME: " << ws_name << endl;
        if(rename_map_.find(minitree_name) == rename_map_.end()) {
            rename_map_[minitree_name] = ws_name;
        }
        int nbins = 100;
        double low_val=9999, hi_val=9999;
        if(obs_parameters.size() == 3){
            low_val = (double) atof(obs_parameters.at(1).c_str());
            hi_val = (double) atof(obs_parameters.at(2).c_str());
            adaptive = true;
        } else if(obs_parameters.size() == 4) {
            nbins = atoi(obs_parameters.at(1).c_str());
            low_val = (double) atof(obs_parameters.at(2).c_str());
            hi_val = (double) atof(obs_parameters.at(3).c_str());
        } else {
            log_err("I don't understand: %s", obs_input.c_str());
            continue;
        }
        cout<<"Low: "<< low_val << ", High: " << hi_val << " bins:" << nbins << endl;
        float value = (low_val+hi_val)/2.;
        auto var_ws = new RooRealVar( ws_name.c_str(), ws_name.c_str(),
                value, low_val, hi_val
                );
        var_ws->Print();
        if(!adaptive) var_ws->setBins(nbins);
        obs_ws.add(*var_ws);
        auto var_minitree = new RooRealVar( minitree_name.c_str(), minitree_name.c_str(),
                    value, low_val, hi_val
                );
        if(!adaptive) var_minitree->setBins(nbins);
        obs_minitree.add(*var_minitree);
        var_minitree->Print();
    }
}

SampleBase* Combiner::createSample(const string& name, const string& sample_input)
{
    // {sampletype} : {{sampleargs}}
    strvec sampleargs;
    Helper::tokenizeString(sample_input, ':', sampleargs);
    if (sampleargs.size()!=2){
      log_err("Trying to create sample but requires exactly two colon-seperated fields \"sampletype : arg0, arg1, arg2 ...\"");
      log_err("I received \"%s\"", sample_input.c_str());
      return NULL;
    }
    std::string sampletype = sampleargs[0];
    std::string copyargs = sampleargs[1];
    Helper::tokenizeString(copyargs, ',', sampleargs);

    auto newsample = SampleFactory::CreateSample(sampletype,sampleargs);

    if (!newsample){
        log_err("Failed to add sample %s!,aborting Combiner!", name.c_str());
        return NULL;
    }

    if(allCoefficients.find(name) != allCoefficients.end()) {
        newsample->addCoefficient(allCoefficients[name]);
    } else {
        log_err("Tried to add coefficient for sample %s but I couldn't find it!",name.c_str());
        return NULL;
    }
    return newsample;
}
void Combiner::addCutVariables(RooArgSet& obs, const string& cut)
{
    if (cut == "") return;
    strvec name_list;
    Helper::getListOfNames(cut, name_list);
    for(auto name : name_list){
        obs.add(*(new RooRealVar(name.c_str(), name.c_str(), -1E6, 1E6)));
    }
}
