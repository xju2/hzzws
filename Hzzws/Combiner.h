// =====================================================================================
// 
//       Filename:  Combiner.h
// 
//    Description:  Combining each category
// 
//        Version:  1.0
//        Created:  03/16/2015 21:09:25
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#ifndef __HZZWS_COMBINER_H__
#define __HZZWS_COMBINER_H__
#include "Hzzws/Sample.h"
#include "Hzzws/Category.h"
#include "Hzzws/SystematicsManager.h"

#include <TString.h>
#include <TFile.h>
#include <RooArgSet.h>

#include <vector>
using namespace std;
class Combiner{
    private:
        TString name;
        vector<Category*> allCategories;
        //TFile* data_file;
        //RooArgSet& obs;
        SystematicsManager* sysMan;
        vector<Sample*> allSamples;

        void printDic(map<string, map<string, string> >& all_dic);
        // read the overall configurations
        void readConfig(const char* _name);
    
    public:
        Combiner(const char* _name, const char* _configName);
        virtual ~Combiner();

};
#endif
