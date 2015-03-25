// 
//    Description:  Have all the systematics and know how to add systematics
// 
#ifndef __HZZWS_SYSTEMATICSMANAGER_H__
#define __HZZWS_SYSTEMATICSMANAGER_H__

#include <vector>
#include "Sample.h"
#include <RooWorkspace.h>
#include <TString.h>


class SystematicsManager{
    private:
        std::vector<TString>* all_nps;

    public:
        SystematicsManager();
        SystematicsManager(const char* fileName);
        virtual ~SystematicsManager();
        void readNPs(const char* fileName);
        void add_sys(Sample*);
        void add_np(RooWorkspace*);
};
#endif
