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
        map<string, map<string, string> > all_dic;

        //TFile* data_file; //TODO
        map<string, Sample*> allSamples;
        SystematicsManager* sysMan;
        RooArgSet obs;
        vector<Category*> allCategories;

        string findCategoryConfig(string& cat_name, const char* name);
        Sample* getSample(string& name);
        ////////////////////////
        //tokenize the string with specific delimeter. 
        //!!Don't forget delete the returned vector<string>*!!
        ////////////////////////
        vector<string>* tokenizeString(string& str, char delim);
        void printDic();

        // read the overall configurations
        void readConfig(const char* _name);
    
    public:
        explicit Combiner(const char* _name, const char* _configName);
        virtual ~Combiner();

};
#endif
