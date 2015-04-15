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

class RooStatsHelper{
public:
    explicit RooStatsHelper(const char* input_name, const char* ws_name, 
            const char* mc_name, const char* data_name, const char* mu_name);
    virtual ~RooStatsHelper();
    ////////////////////////////////////////////////////////////
    // static functions
    ////////////////////////////////////////////////////////////
    static void setDefaultMinimize();
    static void setVarfixed(RooWorkspace* ws, const char* varName, double imass);
    static void setVarFree(RooWorkspace* combined, const char* varName);
    static pair<double,double> getVarVal(const RooWorkspace& w, const char* var);
    static int minimize(RooNLLVar* nll, RooWorkspace* combWS=nullptr);
    static RooNLLVar* createNLL(RooAbsData* data, RooStats::ModelConfig* mc);
    // Make asimov data
    static RooDataSet* makeAsimovData(RooWorkspace* combined, 
            double muval, 
            double profileMu,  // used when fit data
            const char* muName, // name of POI
            const char* mcname, // name of ModelConfig
            const char* dataname, // name of observed Data
            bool doprofile    // profile to data?
            );
    // get p0-value
    static double getPvalue(RooWorkspace* combined, 
            RooAbsPdf* combPdf, 
            RooStats::ModelConfig* mc, 
            RooAbsData* data, 
            RooArgSet* nuis_tmp, 
            const char* condName,
            const char* muName,
            bool isRatioLogLikelihood = false);
    // sqrt(2* ((s+b)ln(1+s/b) - b ))
    static double getRoughSig(double s, double b);
    ////////////////////////////////////////////////////////////
    // public functions
    ////////////////////////////////////////////////////////////
    double getExpectedPvalue();
    double getObservedPvalue();
    double getExpectedLimit();
    double getObservedLimit();
private:
    TFile* input_file_;
    RooWorkspace* ws_;
    RooStats::ModelConfig* mc_;
    RooDataSet* obs_data_;
    RooDataSet* asimov_data_;
    RooRealVar* poi_;
};
#endif
