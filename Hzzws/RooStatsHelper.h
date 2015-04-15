// a set of tools used for calculating p0-values or setting limits
//
#ifndef _HZZWS_ROOSTATSHELPER_H_
#define _HZZWS_ROOSTATSHELPER_H_


#include <utility>
#include "RooStats/ModelConfig.h"
#include "RooNLLVar.h"
#include "RooWorkspace.h"
#include "RooArgSet.h"
#include "RooAbsData.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "TFile.h"

using namespace std;

namespace RooStatsHelper{
    void setDefaultMinimize();
    void setVarfixed(RooWorkspace* ws, const char* varName, double imass);
    void setVarFree(RooWorkspace* combined, const char* varName);
    pair<double,double> getVarVal(const RooWorkspace& w, const char* var);
    int minimize(RooNLLVar* nll, RooWorkspace* combWS=nullptr);
    RooNLLVar* createNLL(RooAbsData* data, RooStats::ModelConfig* mc);
    // Make asimov data
    RooDataSet* makeAsimovData(RooWorkspace* combined, 
            double muval, 
            double profileMu,  // used when fit data
            const char* muName, // name of POI
            const char* mcname, // name of ModelConfig
            const char* dataname, // name of observed Data
            bool doprofile    // profile to data?
            );
    // get p0-value
    double getPvalue(RooWorkspace* combined, 
            RooAbsPdf* combPdf, 
            RooStats::ModelConfig* mc, 
            RooAbsData* data, 
            RooArgSet* nuis_tmp, 
            const char* condName,
            const char* muName,
            bool isRatioLogLikelihood = false);
    // sqrt(2* ((s+b)ln(1+s/b) - b ))
    double getRoughSig(double s, double b);
}
#endif
