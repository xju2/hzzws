#include "Hzzws/Checker.h"

#include "RooStats/AsymptoticCalculator.h"
#include "RooRealVar.h"
#include "RooSimultaneous.h"

#include "Hzzws/RooStatsHelper.h"
#include "Hzzws/Helper.h"

#include <iostream>
using namespace std;

Checker::Checker(const char* input_name, const char* ws_name,
        const char* mc_name, const char* data_name, const char* mu_name)
{
   input_file_ = TFile::Open(input_name, "read");
   cout << "Input: " << input_name << endl;
   ws_ = (RooWorkspace*) input_file_->Get(ws_name);
   mc_ = (RooStats::ModelConfig*) ws_->obj(mc_name);
   if(mc_) {
        mc_->GetParametersOfInterest()->Print("v");
        mc_->GetNuisanceParameters()->Print("v");
   }
   obs_data_ = (RooDataSet*) ws_->data(data_name);
   asimov_data_ = nullptr;  // create it when needed
   poi_ = (RooRealVar*) ws_->var(mu_name);
   if(poi_){
       poi_->Print();
   }
   auto mG = ws_->var("mG");
   if (mG) mG->setConstant(0);
   auto kappa = ws_->var("GkM");
   if (kappa) kappa->setConstant(0);
   // fixGammaTerms
   RooStatsHelper::fixTermsWithPattern(mc_, "gamma_stat");
   ws_->saveSnapshot("nominalGlobs", *mc_->GetGlobalObservables());
   ws_->saveSnapshot("nominalNuis", *mc_->GetNuisanceParameters());
}

Checker::~Checker()
{
    if (!input_file_->IsZombie()) input_file_->Close();
}

double Checker::getExpectedPvalue()
{
    if (asimov_data_ == nullptr){
        /***
    Checker::makeAsimovData(combined, 
            1.0, // mu in pdf
            0.0, // the value when profile to data
            muName.c_str(), 
            mcName.c_str(),
            dataName.c_str(),
            false); // donot profile!
    RooDataSet* asimovData = (RooDataSet*) combined ->data("asimovData1_paz");
    ***/
        asimov_data_ = dynamic_cast<RooDataSet*>(RooStats::AsymptoticCalculator::GenerateAsimovData(*mc_->GetPdf(), *mc_->GetObservables()));
    }
    return RooStatsHelper::getPvalue(ws_, mc_->GetPdf(), mc_, asimov_data_, "nominalGlobs", poi_->GetName());
}

double Checker::getObservedPvalue()
{
    if (!obs_data_) {
        cerr <<" obsData does not exist() " << endl;
        return -100;
    }
   auto* mH = (RooRealVar*) ws_->var("mH");
   if(mH){
        mH->setConstant(false);
   }
    return RooStatsHelper::getPvalue(ws_, mc_->GetPdf(), mc_, obs_data_, "", poi_->GetName());
}

double Checker::getExpectedLimit()
{
    // TODO
    return -1;
}

double Checker::getObservedLimit()
{
    // TODO
    return -1;
}

bool Checker::CheckNuisPdfConstraint(const RooArgSet* nuis, const RooArgSet* pdfConstraint)
{
    if(!nuis || nuis->getSize() < 1) {
        cout << "no nusiance parameter is found" << endl;
        return true;
    }
    if(nuis->getSize() != pdfConstraint->getSize())
    {
        log_err("number of nuisance %d; pdfConstraint size: %d", nuis->getSize(), pdfConstraint->getSize());

        TIterator* iterNuis = nuis->createIterator();
        TIterator* iterPdfCons = pdfConstraint->createIterator();
        RooRealVar* nuisVal;
        RooAbsPdf* pdfCon;
        while((nuisVal = (RooRealVar*)iterNuis->Next())){
            TString constrainName(Form("%sConstraint",nuisVal->GetName())); 
            if(pdfConstraint->find(constrainName.Data()) == NULL){
                cout <<nuisVal->GetName()<<" not Constraint"<<endl;
            }
        }
        while((pdfCon = (RooAbsPdf*)iterPdfCons->Next())){
            TString nuisName(pdfCon ->GetName());
            nuisName.ReplaceAll("Constraint","");
            if(nuis->find(nuisName.Data()) == NULL){
                cout <<nuisName<<" is omitted"<<endl;
            }
        }
        return false;
    } else {
        cout <<"total number of nuisance parameters: "<<nuis->getSize()<<endl;
    }
    return true;
}

bool Checker::CheckNuisPdfConstraint(){
    RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mc_->GetPdf());
    auto* nuis = const_cast<RooArgSet*>(mc_->GetNuisanceParameters());
    const RooArgSet* observables = mc_->GetObservables();
    const RooArgSet* pdfConstraint = simPdf->getAllConstraints(*observables, *nuis, false);
    return CheckNuisPdfConstraint(nuis, pdfConstraint);
}
