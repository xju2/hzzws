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

class Combiner{
    private:
        TString name;
        std::vector<Category*>* categories;
        TFile* data_file;
        RooArgSet& obs;
        SystematicsManager* sysMan;
    
    public:
        Combiner(const char* _name);
        virtual ~Combiner();
        // read the overall configurations
        bool readConfig(const char* _name);
};
#endif
