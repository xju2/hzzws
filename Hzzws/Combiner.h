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

#include <vector>
#include <map>
using namespace std;
class Combiner{
    private:
        TString name;
        vector<Category*> allCategories;
        TFile* data_file;
        //RooArgSet& obs;
        //SystematicsManager* sysMan;
        map<TString, Sample*> allSamples;

        void printDic(map<string, map<string, string> >& all_dic);
        // read the overall configurations
        void readConfig(const char* _name);
    
    public:
        Combiner(const char* _name, const char* _configName);
        virtual ~Combiner();

};
#endif
