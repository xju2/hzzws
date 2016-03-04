// 
//    Description:  Combining each category
// 
#ifndef __HZZWS_COMBINER_H__
#define __HZZWS_COMBINER_H__
#include "Hzzws/SampleBase.h"
#include "Hzzws/Category.h"
#include "Hzzws/SystematicsManager.h"
#include "Hzzws/ParametrizedSample.h"

#include <TString.h>
#include <TFile.h>
#include <TChain.h>
#include <RooArgSet.h>
#include <RooSimultaneous.h>
#include <RooWorkspace.h>

#include <vector>
#include <map>
using namespace std;
class Combiner{
    
    public:
        explicit Combiner(const char* _name, const char* _configName);

        void SetLumiFactor(double lumi_factor);

        void combine();
        virtual ~Combiner();

    private:
        TString ws_name_; // name of workspace
        string simpdf_name; // simutaneous pdf's name
        string mainSectionName; // main section name
        double var_low_;
        double var_hi_;
        string file_path_;
        vector<string> all_categories_;
        RooWorkspace* workspace;
        double lumi_factor_;
        string config_name_;

        map<string, map<string, string> > all_dic;
        map<string, map<string, double> > all_norm_dic_;
        RooArgSet nuisanceSet;
        RooArgSet globalobsSet;

        TChain* data_chain;
        map<string, SampleBase*> allSamples;
        SystematicsManager* sysMan;
        RooArgSet obs;

        string findCategoryConfig(string& cat_name, const char* name);
        SampleBase* getSample(string& name);

        // read the overall configurations
        void readConfig(const char* _name);
        void configWorkspace(RooWorkspace* ws);
        void AddKeysSample(SampleBase& param, const string& config_file);
        string GetTCut(const string& ch_name);
};
#endif
