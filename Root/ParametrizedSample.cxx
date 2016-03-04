/*
 */
using namespace std;
#include <stdlib.h>

#include <RooStats/HistFactory/RooBSpline.h>
#include <RooArgList.h>
#include <RooArgSet.h>
#include <RooRealVar.h>
#include <RooProduct.h>
#include <RooRealSumPdf.h>

#include "Hzzws/ParametrizedSample.h"

ParametrizedSample::ParametrizedSample(const char* name,
        const char* nickname, 
        const char* para_name, float low, float hi):
    SampleBase(name, nickname),
    para_name_(para_name)
{
    signal_samples_ = new vector<SampleBase*>();
    masses_ = new vector<double>();
    mH_ = new RooRealVar(para_name_.c_str(),para_name_.c_str(), (low+hi)/2.0, low, hi);
    // mH_ = new RooRealVar(para_name_.c_str(),para_name_.c_str(), 250, 200., 600.);
    bases_ = NULL;
    order_ = 3;
}

ParametrizedSample::~ParametrizedSample(){
    for(auto sample : *signal_samples_) 
        if(sample) delete sample;
   
    if(signal_samples_) delete signal_samples_;
    if(masses_) delete masses_;
    if(mH_) delete mH_;
}

bool ParametrizedSample::AddSample(SampleBase* signal)
{
    if(!signal) return false;
    signal_samples_->push_back(signal);
    cout << "mass: " << signal->get_mass() << endl;
    masses_->push_back(signal->get_mass());
    return true;
}

bool ParametrizedSample::setChannel(const RooArgSet& observable, 
        const char* channelName, bool with_sys)
{
    if(signal_samples_->size() < 1) return false;
    bool result = true;
    for(const auto& sample : *signal_samples_){
        if(!sample->setChannel(observable, channelName, with_sys)){
            result = false;
        }
    }
    category_name_ = string(channelName);
    base_name_ = TString(Form("%s_%s", pdf_name_.c_str(), channelName));
    return result;
}

bool ParametrizedSample::addShapeSys(const TString& npName)
{
    if(signal_samples_->size() < 1) return false;
    bool result = true;
    for(const auto& sample : *signal_samples_){
        if(! sample->addShapeSys(npName)) result = false;
    }
    return result;
}

bool ParametrizedSample:: addNormSys(const TString& npName)
{
    if(signal_samples_->size() < 1) return false;
    bool result = true;
    for(const auto& sample : *signal_samples_){
        if(! sample->addNormSys(npName)) result = false;
    }
    return result;
}

RooAbsPdf* ParametrizedSample::getPDF()
{
    if(!bases_) BuildBases();
    RooArgList* controlPoints = new RooArgList();
    for(const auto& sample : *signal_samples_){
        controlPoints->add(*(sample->getPDF()) );
    }
    const char* bs_pdf_name = Form("bs_%s", base_name_.Data());
    auto* bs_pdf = new RooStats::HistFactory::RooBSpline(bs_pdf_name, bs_pdf_name, *controlPoints, *bases_, RooArgSet()); 
    const char* category_pdf_name = Form("%s_Para", base_name_.Data());
    auto* one = new RooRealVar("one", "one", 1.0); 
    return new RooRealSumPdf(category_pdf_name, category_pdf_name, RooArgList(*bs_pdf), RooArgList(*one));
}

RooAbsReal* ParametrizedSample::getCoeff()
{
    if(!bases_) BuildBases();
    // parametrize normalization and systematics
    RooArgList* cp_norm = new RooArgList();
    RooArgList* cp_fiv = new RooArgList();
    for(const auto& sample : *signal_samples_){
        RooAbsReal* coefficient = sample->getCoeff();
        if(coefficient->InheritsFrom("RooProduct")){
            RooProduct* norm_prod = dynamic_cast<RooProduct*>(coefficient);
            norm_prod->Print();
            const RooArgList& comp = norm_prod->components();
            for(int i = 0; i < comp.getSize(); i++){
                RooAbsArg* arg = comp.at(i);
                // cout <<"arg: " <<  arg->GetName() << endl;
                if(arg->InheritsFrom("RooStats::HistFactory::FlexibleInterpVar")){
                    cp_fiv->add(*arg);
                } else if (TString(arg->GetName()).Contains("nATLAS")){
                    cp_norm->add(*arg);
                } 
            }
        } else {
            cp_norm->add(*coefficient);
        }
    }
    // cout << "size of control point [normalize]: " << cp_norm->getSize() << endl;
    // cout << "size of control point [norm_sys]: " << cp_fiv->getSize() << endl;
    RooArgList prodSet;
    const char* bs_norm_name = Form("bs_n%s", base_name_.Data());
    auto* bs_norm = new RooStats::HistFactory::RooBSpline(bs_norm_name, bs_norm_name, *cp_norm, *bases_, RooArgSet()); 
    prodSet.add(*bs_norm);
    if(is_signal_) addMu(prodSet);

    if(cp_fiv->getSize() > 0){
        const char* bs_fiv_name = Form("bs_fiv_%s", base_name_.Data());
        auto* bs_fiv = new RooStats::HistFactory::RooBSpline(bs_fiv_name, bs_fiv_name, *cp_fiv, *bases_, RooArgSet()); 
        prodSet.add(*bs_fiv);
    } 
    const char* prod_name = Form("nTot%s_Para", base_name_.Data());
    auto* normProd = new RooProduct(prod_name, prod_name, prodSet);
    return normProd;
}

void ParametrizedSample::SetParaRange(const string& range_name, double low_value, double hi_value){
    mH_->setRange(range_name.c_str(), low_value, hi_value);
}

void ParametrizedSample::BuildBases()
{
    if(masses_->size() < 1) { throw std::runtime_error("Call AddSample first please!"); }
    bases_ = new RooStats::HistFactory::RooBSplineBases(Form("bases_%s", nickname_.c_str()),
            Form("bases_%s", nickname_.c_str()), order_, *masses_, *mH_);
}
