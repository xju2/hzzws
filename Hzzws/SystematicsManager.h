// 
//    Description:  Have all the systematics and know how to add systematics
// 
#ifndef __HZZWS_SYSTEMATICSMANAGER_H__
#define __HZZWS_SYSTEMATICSMANAGER_H__

#include <vector>
#include "Sample.h"
#include <RooWorkspace.h>
#include <TString.h>

using namespace std;

class SystematicsManager{
    private:
        std::vector<TString>* all_nps;

    public:
        SystematicsManager();
        SystematicsManager(const char* fileName);
        virtual ~SystematicsManager();
        void readNPs(const char* fileName);
        vector<TString>* add_sys(Sample*);
        inline int totalNP(){return all_nps->size();}
};
#endif
