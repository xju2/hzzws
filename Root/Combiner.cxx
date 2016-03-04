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
#include "Hzzws/SampleHist.h"
#include "Hzzws/CBGauss.h"
#include "Hzzws/ExpLaundau.h"
#include "Hzzws/SampleKeys.h"

Combiner::Combiner(const char* _name, const char* _configName):
    ws_name_(_name),
    simpdf_name("simPdf"),
    mainSectionName("main"),
    config_name_(_configName)
{
    cout<<"workspace name: "<< ws_name_ << endl;
    file_path_ = "./";
    data_chain = nullptr;
    workspace = nullptr;
    lumi_factor_ = 1.0;
}

Combiner::~Combiner(){
    if(data_chain) delete data_chain;
    if(sysMan) delete sysMan;
    for(auto& sample : allSamples) delete sample.second;
    if(workspace) delete workspace;
}

void Combiner::readConfig(const char* configName)
{
    Helper::readConfig(configName, '=', all_dic);
    Helper::printDic<string>(all_dic);
    workspace = new RooWorkspace("combined");

    auto& job_dic = all_dic.at(mainSectionName);

    try {
        file_path_ = job_dic.at("fileDir");
    } catch (const out_of_range& oor) {
        cout << "'fileDir' not specific, look in current directory" << endl;
    }
    std::cout<<"looking for files in "<<file_path_<<std::endl;
    Helper::getInputPath(file_path_);

    ///////////////////////////////////
    // check if normalization table exists
    ///////////////////////////////////
    try {
        string table_name = job_dic.at("normalization");
        table_name = file_path_ + "/" + table_name; 
        Helper::readNormTable(table_name.c_str(), all_norm_dic_, 
                lumi_factor_);
        Helper::printDic<double>(all_norm_dic_);
    } catch (const out_of_range& oor) {
        cout << "'normalization' not specific, get expected from histogram." << endl;
    }
    
    ////////////////////////////////////
    //load samples
    ////////////////////////////////////
    Helper::tokenizeString(job_dic.at("categories"), ',',all_categories_);
    string obs_str_tmp = job_dic["observable"];
    vector<string> obsPara_tmp, obsPara_name, obsPara_name_tokenized;
    vector<int> obsPara_bins;
    vector<double> var_low_vec, var_hi_vec;
    Helper::tokenizeString(obs_str_tmp, ',', obsPara_tmp);
  
    if(obsPara_tmp.size()%4==0) {         // var, bins, var_low, var_high
        var_low_ = (double) atof(obsPara_tmp.at(2).c_str());
        var_hi_ = (double) atof(obsPara_tmp.at(3).c_str());

        for (int i = 0; i < (int) obsPara_tmp.size(); i=i+4) {
            obsPara_name.push_back(obsPara_tmp.at(i).c_str());
            obsPara_bins.push_back((int) atof(obsPara_tmp.at(i + 1).c_str()));
            var_low_vec.push_back((double) atof(obsPara_tmp.at(i + 2).c_str()));
            var_hi_vec.push_back((double) atof(obsPara_tmp.at(i + 3).c_str()));
        }

    } else if(obsPara_tmp.size()%3==0) {  // var, var_low, var_high
        var_low_ = (double) atof(obsPara_tmp.at(1).c_str());
        var_hi_ = (double) atof(obsPara_tmp.at(2).c_str());

        for (int i = 0; i < (int) obsPara_tmp.size(); i=i+3) {
            obsPara_name.push_back(obsPara_tmp.at(i).c_str());
            var_low_vec.push_back((double) atof(obsPara_tmp.at(i + 1).c_str()));
            var_hi_vec.push_back((double) atof(obsPara_tmp.at(i + 2).c_str()));
        }   

    } else {
        cout << "Check observables limits!" <<endl;
        return;
    }

    // check NPlist to see if systematics should be done
    bool doSys=true;
    try {
        string NP_list = job_dic.at("NPlist") ;
    } catch (const out_of_range& oor) {
        doSys=false;
    }

    for (auto& sample : all_dic.at("samples")) {
        vector<string> tokens;
        Helper::tokenizeString(sample.second, ',', tokens);
        if(tokens.size() > 3){
            // 0: input_path, 1: shape_sys_path, 2: norm_sys_path, 3: name, 4: samples config (eg ggH_config.ini)
            if (tokens.at(0).find("analyticalParam")!=string::npos && (sample.first=="ggH" || sample.first=="VBFH")) {
                auto* newsample = new CBGauss(tokens.at(3).c_str(), sample.first.c_str(), 
                        tokens.at(0).c_str(), 
                        tokens.at(1).c_str(), 
                        tokens.at(2).c_str(), 
                        file_path_.c_str(), doSys);
                AddKeysSample(*newsample, file_path_+"/"+tokens.at(4));//doesn't really add keys: just needed for norm systematics

                allSamples[sample.first] = newsample;
            } else if(tokens.at(0).find("ExpLaundau") != string::npos) {
                auto* newsample = new ExpLaundau(tokens.at(3).c_str(), sample.first.c_str(), 
                        tokens.at(0).c_str(),
                        tokens.at(1).c_str(),
                        tokens.at(2).c_str(),
                        file_path_.c_str(), 
                        doSys);
                allSamples[sample.first] = newsample;
            } else {
                auto* newsample = new SampleHist(tokens.at(3).c_str(), sample.first.c_str(), 
                        tokens.at(0).c_str(), 
                        tokens.at(1).c_str(), 
                        tokens.at(2).c_str(), 
                        file_path_.c_str());
                if (tokens.size() > 4){
                    newsample->setMCCThreshold(atof(tokens.at(4).c_str()));
                }
                allSamples[sample.first] = newsample;
            }
        } 
        else {
            /* SampleKeys
             * 0: config.ini, 1: name
             * */
            auto* newsample = new ParametrizedSample(tokens.at(1).c_str(), sample.first.c_str()); 
            AddKeysSample(*newsample, tokens.at(0));
            allSamples[sample.first] = newsample;
            continue;
        }

        if (all_norm_dic_.size() > 0) {
            try {
                allSamples[sample.first]->setNormalizationMap(all_norm_dic_.at(sample.first));
            } catch (const out_of_range&) {
                cout << sample.first << " does not included in normalization table." << endl;
            }
        }
    }
    cout << "Added " << all_dic.at("samples").size() << " Samples" << endl;

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
        cout << "I am blind!!" << endl;
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

    vector<RooRealVar *> observables_tmp;

    for (int i = 0; i < (int)obsPara_name.size(); ++i) {
        cout << "The range of observable "+obsPara_name[i]+": " << var_low_vec[i] << " " << var_hi_vec[i] << endl;
        RooRealVar *var = nullptr;
        observables_tmp.push_back(var); 
        observables_tmp[i] = new RooRealVar(obsPara_name[i].c_str(), obsPara_name[i].c_str(), var_low_vec[i], var_hi_vec[i]);

        vector<string> var_name_tmp;
        Helper::tokenizeString(obsPara_name[i], '_', var_name_tmp);
        obsPara_name_tokenized.push_back(var_name_tmp.at(0));
    }

    /*
     * hard coded the branch name! also in SampleKeys
     */
    RooRealVar event_type("event_type", "event_type", -10, 10);
    RooRealVar prod_type("prod_type", "prod_type", -10, 10);
    RooRealVar met_et("met_et", "MET", 0, 500);


    while( getline( iss_cat, category_name, delim )) 
    {
        boost::algorithm::trim(category_name);
        cout << "===========================" << endl;
        cout <<"On category: "<< category_name << endl;

        string mcsets = findCategoryConfig(category_name, "mcsets");
        if(mcsets == "") continue;

        vector<string> mcsets_names;
        Helper::tokenizeString( mcsets, ',', mcsets_names) ;
        Category* category = new Category(category_name);
        ///////////////////////////////////
        // add observables
        ///////////////////////////////////
        obs.removeAll();
        // 0: obs_name, 1: nbins_str, 2: low_str, 3: hi_str;
        bool use_adaptive_binning = false;
        vector<RooRealVar*> observables;

        for(int i=0; i<(int)observables_tmp.size(); ++i){
            RooRealVar *var = nullptr;
            observables.push_back(var); 
            observables[i] = new RooRealVar(obsPara_name_tokenized[i].c_str(), obsPara_name_tokenized[i].c_str(), var_low_vec[i], var_hi_vec[i]);
            if (obsPara_bins.size() > 0){
                observables[i]->setBins(obsPara_bins[i]);	
            }
            else{
                // adaptive binning, only set range
                // The actual binning will be choosen for different input histograms
                use_adaptive_binning = true;
            }
            obs.add(*observables[i]); 
        } 

        category->setObservables(obs);
        /*
         * Add Samples in listed in mcsets
         * Note that sysMan(systematic manager) decides if add systematics 
         * and if yes, add which systematics
         */
        for(auto& mcset_name : mcsets_names){
            SampleBase* sample = getSample(mcset_name);
            if(sample) {
                category->addSample(sample, sysMan);
                if(use_adaptive_binning){ 
                    sample->useAdaptiveBinning();
                }
            }
        }
        cout <<"End add Samples: "<< category_name << endl;

        string catName(Form("%sCat",category_name.c_str()));
        channelCat.defineType(catName.c_str(), catIndex++);
        //////////////////////////////////////////////////////////////
        // Category will sum individual sample's pdf and add constraint terms
        //////////////////////////////////////////////////////////////
        RooAbsPdf* final_pdf = category->getPDF();
        final_pdf->Print();
        string final_pdf_name(final_pdf->GetName()); 
        // final_pdf->getVal(); // To increast the fitting speed??


        for(int i=0; i<(int)observables.size(); ++i){
            workspace ->import(*observables[i], RooFit::RecycleConflictNodes());
        }
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
            nuisanceSet.add(*nuisance);
            globalobsSet.add(*global);
        }

        // delete category;
        pdfMap[catName] = workspace->pdf(final_pdf_name.c_str());


        //  Add Data //

        if(data_chain){
            string cut = "";
            try{
                cut = all_dic.at(category_name).at("cut");
            } catch(const out_of_range& oor){
                log_err("cannot find |cut| in %s, make up one!", category_name.c_str());
                cut = GetTCut(category_name);
            }
            cout << "Cut on data: " << cut << endl;
            string obs_data_ch_name(Form("data_%s",category_name.c_str()));
            RooArgSet var_set;
            for(int i=0; i<(int)observables_tmp.size(); ++i){
                var_set.add(*observables_tmp[i]);
            }
            var_set.add(event_type);
            var_set.add(prod_type); 
            var_set.add(met_et);
            RooDataSet* data_ch = new RooDataSet(obs_data_ch_name.c_str(),
                    "data set",
                    var_set,
                    RooFit::Import(*data_chain),
                    RooFit::Cut(cut.c_str())
                    );

            // workspace->import(*data_ch, arg);
            // dataMap[catName] = (RooDataSet*) workspace->data(obs_data_ch_name.c_str());
            cout << "data: in "<< category_name<< " " << data_ch->sumEntries() << endl;
            dataMap[catName] = data_ch;

        }

        cout <<"End category: "<< category_name << endl;
    }

    if(data_chain){
        RooArgSet var_set;
        RooCmdArg arg1, arg2; // for 2D
        for(int i=0; i<(int)observables_tmp.size(); ++i){
            var_set.add(*observables_tmp[i]);
            if(i==0)
                arg1 = RooFit::RenameVariable(obsPara_name[i].c_str(), obsPara_name_tokenized[i].c_str());
            else
                arg2 = RooFit::RenameVariable(obsPara_name[i].c_str(), obsPara_name_tokenized[i].c_str());
        }
        var_set.add(channelCat);
        RooDataSet* obsData = new RooDataSet(
                "obsData",
                "observed data",
                var_set,
                RooFit::Index(channelCat),
                RooFit::Import(dataMap)
                );
        // workspace->import(*obsData, RooFit::RecycleConflictNodes());
        if((int)observables_tmp.size()>1)
            workspace->import(*obsData, arg1, arg2);
        else
            workspace->import(*obsData, arg1);

    }



    auto* simPdf = new RooSimultaneous(simpdf_name.c_str(), simpdf_name.c_str(), pdfMap, channelCat);
    workspace ->import(*simPdf, RooFit::RecycleConflictNodes());



    this->configWorkspace(workspace);
    RooRealVar* mH = workspace->var("mH");
    if(mH) {
        mH->setRange(var_low_, var_hi_);
    }
    //workspace->Print();
    std::cout<<"end of Combiner::readConfig"<<std::endl;
    workspace ->writeToFile(ws_name_);

}


SampleBase* Combiner::getSample(string& name)
{
    SampleBase* sample = NULL;
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
    cout << "Configuring the workspace" << endl;

    ws->Print();
    //////////////////// 
    // define set
    //////////////////// 
    RooArgSet poiSet;
    if(ws->var("mu_BSM")) poiSet.add(* ws->var("mu_BSM") );
    else if(ws->var("mu")) poiSet.add(* ws->var("mu"));
    else {
        log_err("I cannot find POI!");
    }

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
    ws->saveSnapshot("nominalGlobs", *ws->set("globalobs"));
    ws->saveSnapshot("nominalNuis", *ws->set("nuisance"));
    cout << "end of configuring workspace" << endl;
    return;
}

void Combiner::AddKeysSample(SampleBase& samplebase, const string& config_file) 
{
    auto parametrizedSample = dynamic_cast<ParametrizedSample*>(&samplebase);
    auto cbGauss = dynamic_cast<CBGauss*>(&samplebase);

    map<string, map<string, string> > all_keys_info;
    Helper::readConfig(config_file.c_str(), '=', all_keys_info);
    Helper::printDic<string>(all_keys_info);

    string& all_samples =  all_keys_info.at("Init").at("mcsets");
    string& minitree_dir = all_keys_info.at("Init").at("minitree_path");
    vector<string>* sample_list = new vector<string>();
    Helper::tokenizeString(all_samples, ',', *sample_list);
    if(sample_list->size() < 2) {
        cout << "Only one sample is provided in " << config_file << endl;
        cout << "I don' know parametrization" << endl;
        delete sample_list;
        exit(1);
    }
    for (const auto& sample : *sample_list) {
        const auto& keys_dict = all_keys_info.at(sample);

        //What to do if CBGauss:
        if (cbGauss){
            cbGauss->AddMassPoint(atof(keys_dict.at("mH").c_str()), file_path_+"/"+keys_dict.at("norm"), file_path_+"/"+keys_dict.at("shape_mean"), file_path_+"/"+keys_dict.at("shape_sigma"));
        }

        //What to do if ParametrizedSample:
        if (parametrizedSample){
            double mH = (double)atof(keys_dict.at("mH").c_str());
            string minitree(Form("%s/%s",minitree_dir.c_str(), keys_dict.at("minitree").c_str()));
            vector<string> yields_str;
            Helper::tokenizeString(keys_dict.at("yield"), ',', yields_str);
            auto* keys_pdf = new SampleKeys(Form("%s_%.f",samplebase.get_pdf_name().c_str(), mH), 
                    Form("%s_%.f", samplebase.get_nick_name().c_str(), mH), 
                    mH, var_low_, var_hi_,
                    minitree.c_str(), 
                    keys_dict.at("shape").c_str(),
                    keys_dict.at("norm").c_str(),
                    file_path_.c_str()
                    );
            // set expected yields
            if(all_categories_.size() > yields_str.size()) {
                log_err("not providing enough yields! %s, cat(%d), config(%d)", config_file.c_str(),
                        (int)all_categories_.size(), (int)yields_str.size());
            } else {
                int ntotal =  (int)all_categories_.size();
                for(int i = 0; i < ntotal; i ++) {
                    keys_pdf->SetExpectedEvents(all_categories_.at(i), (double)atof(yields_str.at(i).c_str()));
                }
            }
            parametrizedSample->AddSample(keys_pdf);
        }
    }
    delete sample_list;
}

string Combiner::GetTCut(const string& ch_name)
{
    if(ch_name.find("4mu") != string::npos){
        return "event_type == 0";
    } else if (ch_name.find("4e") != string::npos) {
        return "event_type == 1";
    } else if (ch_name.find("2mu2e") != string::npos) {
        if(var_low_ < 140) return "event_type == 2";
        else return "event_type == 2 || event_type == 3";
    } else if (ch_name.find("2e2mu") != string::npos) {
        if(var_low_ < 140) return "event_type == 3";
        else return "event_type == 2 || event_type == 3";
    } else if (ch_name.find("hi_met") != string::npos) {
        return "met_et > 100";
    } else if (ch_name.find("low_met") != string::npos) {
        return "met_et <= 100";
    }else{
        log_err("I don't know your category %s", ch_name.c_str());
    }
    return "1==1";
}

void Combiner::SetLumiFactor(double lumi_factor_) {
    cout << "Set Luminosity factor: " << lumi_factor_ << endl;
    this->lumi_factor_ = lumi_factor_;
}

void Combiner::combine(){
    readConfig(config_name_.c_str());
    std::cout<<"end of Combiner::combine"<<std::endl;
}
