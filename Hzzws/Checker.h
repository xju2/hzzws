#ifndef _HZZWS_CHECKER_H_
#define _HZZWS_CHECKER_H_

#include "RooStats/ModelConfig.h"
#include "RooNLLVar.h"
#include "RooWorkspace.h"
#include "RooArgSet.h"
#include "RooAbsData.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "TFile.h"

class Checker
{
public:
    explicit Checker(const char* input_name, const char* ws_name, 
            const char* mc_name, const char* data_name, const char* mu_name);
    virtual ~Checker();
    double getExpectedPvalue();
    double getObservedPvalue();
    double getExpectedLimit();
    double getObservedLimit();
    bool CheckNuisPdfConstraint();
protected:
    TFile* input_file_;
    RooWorkspace* ws_;
    RooStats::ModelConfig* mc_;
    RooDataSet* obs_data_;
    RooDataSet* asimov_data_;
    RooRealVar* poi_;

private:
    bool CheckNuisPdfConstraint(const RooArgSet* nuis, const RooArgSet* pdfConstraint);
};
#endif
