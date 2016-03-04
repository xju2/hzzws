// =========================================================================
// 
//    Description:  
// 
// ==========================================================================
#include "Hzzws/SampleBase.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <RooDataHist.h>
#include <TKey.h>
#include <TString.h>
#include <RooArgList.h>
#include <RooRealVar.h>
#include "RooStats/HistFactory/FlexibleInterpVar.h"
#include "RooExpandedDataHist.h"
#include "RooExpandedHistPdf.h"
#include "RooNDKeysPdf.h"
#include "Roo1DMomentMorphFunction.h"
#include "RooFormulaVar.h"
#include "TMath.h"

#include "Hzzws/Helper.h"
#include "Hzzws/BinningUtil.h"

SampleBase::SampleBase(const char* _name, const char* _nickname):
    pdf_name_(_name),
    nickname_(_nickname)
{
    if(pdf_name_.find("Signal") != string::npos) is_signal_ = true;
    else is_signal_ = false;
    if(pdf_name_.find("BSM") != string::npos) is_bsm_ = true;
    else is_bsm_ = false;
    char delim = ':';
    if(pdf_name_.find(":") != string::npos){
        cout<<"My PDF: " << pdf_name_ << endl;
       vector<string> tokens;
       Helper::tokenizeString(pdf_name_, delim, tokens);
       pdf_name_ = tokens[0];
       do_coupling_ = (bool) atoi(tokens.at(1).c_str());
    } else { do_coupling_ = false; }

    mc_constraint = nullptr;
    expected_events_ = -1;
    use_adpt_bin_ = false;
    norm_sys_handler_ = new SysText();
    norm_pdf = nullptr;
}

SampleBase::~SampleBase(){
    if(norm_pdf != nullptr)  delete norm_pdf;
    if(mc_constraint != nullptr) delete mc_constraint;
    if(norm_sys_handler_ != nullptr) delete norm_sys_handler_;
}

bool SampleBase::setChannel(const RooArgSet& _obs, const char* _ch_name, bool with_sys)
{
    // if (mc_constraint != nullptr) delete mc_constraint; // if do so, program crash....
    mc_constraint = nullptr;
    norm_pdf = nullptr;

    // set category name
    category_name_ = string(_ch_name);
    cout<<" SampleBase: "<< pdf_name_ << " set to category: " << category_name_<<endl;

    // set observables
    obs_list_.removeAll();
    auto* iter =  _obs.createIterator();
    TIter next(iter);
    RooRealVar* var;
    while ( (var = (RooRealVar*) next()) ){
        obs_list_.add(*var);
    }

    base_name_ = TString(Form("%s_%s", pdf_name_.c_str(), category_name_.c_str()));
    if(with_sys && norm_sys_handler_) norm_sys_handler_->SetChannel(_ch_name);

    try {
        expected_events_ = normalization_dic_.at(category_name_);
        if(expected_events_ < 0){
            log_err("Negative expected events %.2f in %s!", expected_events_, _ch_name);
            expected_events_ = 1.0;
        }
    } catch (const out_of_range& oor) {
        log_err("map for normalization does not exist %s", _ch_name);
        expected_events_ = -1;
    }

    return true;
}

bool SampleBase::addNormSys(const TString& npName){
    bool result = false;
    if(norm_sys_handler_) result = norm_sys_handler_->AddSys(npName);
    /*
    if(!result){
        log_err("%s is not implemented in %s", npName.Data(), base_name_.Data());
    }
    */
    return result;
}

RooAbsReal* SampleBase::getCoeff()
{
   TString varName(Form("n%s", base_name_.Data()));
   RooRealVar* norm = new RooRealVar(varName.Data(), varName.Data(), expected_events_);
   RooArgList prodSet;
   prodSet.add(*norm);
   this->addMu(prodSet);

   if (norm_sys_handler_){
       TString fixName(Form("fiv_%s", base_name_.Data()));
       // add FlexibleInterpVar
       auto* fiv = (RooStats::HistFactory::FlexibleInterpVar*) norm_sys_handler_->GetSys(fixName.Data());
       if(fiv) prodSet.add(*fiv);
   }
   TString prodName(Form("nTot%s", base_name_.Data()));
   RooProduct* normProd = new RooProduct(prodName.Data(), prodName.Data(), prodSet);
   return normProd;
}

void SampleBase::addMu(RooArgList& prodSet)
{
    string mu_name(Form("mu_%s", this->nickname_.c_str()));
    RooRealVar* mu_sample = new RooRealVar(mu_name.c_str(), mu_name.c_str(), 1.0, -30, 30);
    mu_sample->setConstant(true);

    if(do_coupling_){
        string sqrt_mu_name(Form("sqrt_mu_%s", this->nickname_.c_str()));
        RooFormulaVar* sqrt_mu_sample = new RooFormulaVar(sqrt_mu_name.c_str(), sqrt_mu_name.c_str(), "TMath::Sqrt(@0)", RooArgList(*mu_sample));
        prodSet.add(*sqrt_mu_sample);
    } else {
        prodSet.add(*mu_sample);
    }
    
    if(is_signal_) {
        RooRealVar* mu = new RooRealVar("mu", "mu", 1.0, -30., 30);
        mu->setConstant(true);
        if(do_coupling_) {
            RooFormulaVar* sqrt_mu= new RooFormulaVar("sqrt_mu", "sqrt_mu", "TMath::Sqrt(@0)", RooArgList(*mu));
            prodSet.add(*sqrt_mu);
        } else {
            prodSet.add(*mu);
        }
    }
    if(is_bsm_) {
        RooRealVar* mu_bsm = new RooRealVar("mu_BSM", "mu BSM", 1.0, -30., 30);
        mu_bsm->setConstant(true);
        if(do_coupling_){
            string sqrt_mu_name("sqrt_mu_BSM");
            RooFormulaVar* sqrt_mu_bsm = new RooFormulaVar(sqrt_mu_name.c_str(), sqrt_mu_name.c_str(), "TMath::Sqrt(@0)", RooArgList(*mu_bsm));
            prodSet.add(*sqrt_mu_bsm);
        }else{
            prodSet.add(*mu_bsm);
        }
    }
}


RooAbsPdf* SampleBase::get_mc_constraint(){
    return mc_constraint;
}

bool SampleBase::setNormalizationMap(const map<string, double>& norm_map)
{
    normalization_dic_.clear();
    for (auto& norm_iter : norm_map){
        normalization_dic_[norm_iter.first] = norm_iter.second;
    }
    return true;
}

double SampleBase::ExpectedEvents() const {
    return expected_events_;
}

void SampleBase::useAdaptiveBinning(){
    use_adpt_bin_ = true;
}

bool SampleBase::setNormSysDicFromText(const string& file_name) 
{
    norm_sys_handler_->ReadConfig(file_name.c_str());
    cout << "SysText: "<< file_name <<" for normalization created!" << endl;
    return true;
}
