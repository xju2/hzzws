#include "Hzzws/Checker.h"

#include "RooStats/AsymptoticCalculator.h"
#include "RooRealVar.h"

#include "Hzzws/RooStatsHelper.h"

Checker::Checker(const char* input_name, const char* ws_name,
        const char* mc_name, const char* data_name, const char* mu_name)
{
   input_file_ = TFile::Open(input_name, "read");
   ws_ = (RooWorkspace*) input_file_->Get(ws_name);
   mc_ = (RooStats::ModelConfig*) ws_->obj(mc_name);
   obs_data_ = (RooDataSet*) ws_->data(data_name);
   asimov_data_ = nullptr;  // create it when needed
   poi_ = (RooRealVar*) ws_->var(mu_name);
}

Checker::~Checker(){
    if (ws_) delete ws_;
    if (mc_) delete mc_;
    if (obs_data_) delete obs_data_;
    if (asimov_data_) delete asimov_data_;
    if (input_file_) input_file_->Close();
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
    RooArgSet* nuis =(RooArgSet*) mc_->GetNuisanceParameters();
    return RooStatsHelper::getPvalue(ws_, mc_->GetPdf(), mc_, asimov_data_, nuis, "nominalGlobs", poi_->GetName());
}

double Checker::getObservedPvalue()
{
    if (!obs_data_) {
        cerr <<" obsData does not exist() " << endl;
        return -100;
    }
    RooArgSet* nuis =(RooArgSet*) mc_->GetNuisanceParameters();
    return RooStatsHelper::getPvalue(ws_, mc_->GetPdf(), mc_, obs_data_, nuis, "", poi_->GetName());
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
