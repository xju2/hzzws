// a set of tools used for calculating p0-values or setting limits
//
#ifndef _HZZWS_ROOSTATSHELPER_H_
#define _HZZWS_ROOSTATSHELPER_H_


#include <utility>
#include <map>

#include "RooStats/ModelConfig.h"
#include "RooNLLVar.h"
#include "RooWorkspace.h"
#include "RooArgSet.h"
#include "RooAbsData.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "TFile.h"

using namespace std;

namespace RooStatsHelper{
    void setDefaultMinimize();
    void setVarfixed(RooWorkspace* ws, const char* varName, double imass);
    void setVarFree(RooWorkspace* combined, const char* varName);
    pair<double,double> getVarVal(const RooWorkspace& w, const char* var);
    RooFitResult* minimize(RooNLLVar* nll, RooWorkspace* combWS=nullptr, bool save = true, const RooArgSet* minosSet = NULL);
    RooNLLVar* createNLL(RooAbsData* data, RooStats::ModelConfig* mc);
    // Make asimov data
    void unfoldConstraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter);
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
            const char* condName,
            const char* muName,
            bool isRatioLogLikelihood = false);
    // sqrt(2* ((s+b)ln(1+s/b) - b ))
    double getRoughSig(double s, double b);

    // generate toys
    void generateToy(RooWorkspace* w, 
            const char* poi_name,
            double poi_value,
            int seed, map<string, double>& res);
    RooAbsData* generatePseudoData(RooWorkspace* w, 
            const char* poi_name, int seed);

    void randomizeSet(RooAbsPdf* pdf, RooArgSet* globs, int seed);
    void SetRooArgSetConst(RooArgSet& argset, bool flag = true);

    // Scan POI
    bool ScanPOI(RooWorkspace* ws, 
            const string& data_name,
            const string& poi_name, 
            int total, double low, double hi,
            TTree* tree); 
    void PrintExpEvts(RooSimultaneous* simPdf,
            RooRealVar* mu, const RooArgSet* observables);
    double GetObsNevtsOfSignal(RooSimultaneous* simPdf,
            RooRealVar* mu, const RooArgSet* observables, bool subrange);
    bool fixTermsWithPattern(RooStats::ModelConfig* mc, const char* pat ) ;
}
#endif
