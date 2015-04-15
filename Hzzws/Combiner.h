// 
//    Description:  Combining each category
// 
#ifndef __HZZWS_COMBINER_H__
#define __HZZWS_COMBINER_H__
#include "Hzzws/Sample.h"
#include "Hzzws/Category.h"
#include "Hzzws/SystematicsManager.h"

#include <TString.h>
#include <TFile.h>
#include <RooArgSet.h>
#include <RooSimultaneous.h>

#include <vector>
#include <map>
using namespace std;
class Combiner{
    
    public:
        explicit Combiner(const char* _name, const char* _configName);
        void combine();
        virtual ~Combiner();

    private:
        TString ws_name_; // name of workspace
        string simpdf_name; // simutaneous pdf's name
        string mainSectionName; // main section name
        map<string, map<string, string> > all_dic;
        map<string, map<string, double> > all_norm_dic_;
        RooArgSet nuisanceSet;
        RooArgSet globalobsSet;

        //TFile* data_file; //TODO
        map<string, Sample*> allSamples;
        SystematicsManager* sysMan;
        RooArgSet obs;

        string findCategoryConfig(string& cat_name, const char* name);
        Sample* getSample(string& name);

        // read the overall configurations
        void readConfig(const char* _name);
        void configWorkspace(RooWorkspace* ws);
};
#endif
