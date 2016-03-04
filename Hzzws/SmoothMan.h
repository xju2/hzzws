// 
//    Description:  do the smooth job
// 
#ifndef _HZZWS_SMOOTHMAN_H_
#define _HZZWS_SMOOTHMAN_H_

#include "Hzzws/Smoother.h"
#include <string>
#include <map>
#include <vector>

#include "RooArgSet.h"


using namespace std;

class SmoothMan{
    public:
        SmoothMan(const char *configFile);
        virtual ~SmoothMan();

        void readConfig(const char *configFile);

        void process();
        void processSmoother(Smoother *sm, const string& );
        void getObs(string cat, string &oname, string &treename, RooArgSet &treeobs); 

    private:
        map<string, map<string, string> > m_dic;
};
#endif
