// =========================================================================
// 
//    Description:  
// 
// ==========================================================================
#include "Hzzws/SampleHist.h"
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

#include "Hzzws/Helper.h"
#include "Hzzws/BinningUtil.h"

SampleHist::SampleHist(const char* _name,
        const char* _input,
        const char* _shape_sys) : SampleBase(_name)
{
    cout <<"creating SampleHist"<<endl;
    cout <<"name: " << _name << endl;
    cout <<"input: " << _input << endl;
    cout <<"Shape: " << _shape_sys << endl;
    TString hist_file_name(Form("%s/%s", Helper::getInputPath().c_str(), _input));
    hist_files_ = TFile::Open(hist_file_name.Data(), "read");
    if(!hist_files_ || hist_files_->IsZombie()) {
        log_err("%s does not exist", hist_file_name.Data());
    }
    string shape_file_name(Form("%s/%s", Helper::getInputPath().c_str(), _shape_sys));
    shape_files_ = TFile::Open(shape_file_name.c_str(), "read");
    nom_hist = nullptr;
    use_mcc_ = false;
}

SampleHist::~SampleHist(){
    if(hist_files_) hist_files_->Close();
    if(shape_files_) shape_files_->Close();
    if(nom_hist) delete nom_hist;
}

bool SampleHist::setChannel(const RooArgSet& _obs, const char* _ch_name, bool with_sys)
{
    SampleBase::setChannel(_obs, _ch_name, with_sys);

    if (obs_list_.getSize() == 1) obsname = string(obs_list_.at(0)->GetName());
    else if (obs_list_.getSize() == 2) 
        obsname = string(Form("%s_%s", obs_list_.at(0)->GetName(), obs_list_.at(1)->GetName()));
    else {
        cout << "3D is not supported.. " << endl;
    }

    // get norminal histogram (normalized to 1)
    TString histName(Form("%s_%s", obsname.c_str(), category_name_.c_str()));
    nom_hist =(TH1*) hist_files_->Get(histName.Data());
   
    // TString histName(Form("%s-Nominal-%s", obsname.c_str(), category_name_.c_str()));
    // nom_hist =(TH1*) shape_files_->Get(histName.Data());
    if (!Helper::IsGoodTH1(nom_hist)) {
        log_err("bad histogram %s", histName.Data());
        exit(2);
    }
    nom_hist->SetDirectory(0);
    if(shape_files_){
        string raw_hist_name(Form("%s-Nominal-%s",obsname.c_str(), category_name_.c_str()));
        raw_hist = (TH1*) shape_files_->Get(raw_hist_name.c_str());
        if(!Helper::IsGoodTH1(raw_hist)){
            log_err("raw hist: %s is not good", raw_hist_name.c_str());
            raw_hist = nom_hist;
        }
    } else { raw_hist = nom_hist; }
    
    if (with_sys) {
        // get shape sys dictionary
        this ->getShapeSys();
    }
    return true;
}

RooAbsPdf* SampleHist::makeHistPdf(TH1* hist, const char* base_name, bool is_norm)
{
    RooDataHist *datahist = new RooDataHist(Form("%s_RooDataHist", base_name), "datahist",
            this->obs_list_, hist);
    string pdfname(Form("%s_%s", base_name, obsname.c_str()));
    RooHistPdf *histpdf = new RooHistPdf(pdfname.c_str(), pdfname.c_str(),
            this->obs_list_, *datahist, 3);
    RooDataHist* newdatahist = nullptr;
    if (use_adpt_bin_) {
        log_info("use adaptive binning");
        // create a dataHist with statistic error from un-smoothed histogram
        // to determine the binning
        // to calculate the statistic error
        // rho: use 0.3, determined by half-half fitting?
        RooBinning* binning = new RooBinning();
        RooRealVar rho("rho", "rho", 0.3);
        RooRealVar* obs = (RooRealVar*) obs_list_.at(0);
        auto* keyspdf = new RooNDKeysPdf("keyspdf", "keyspdf", *obs, *raw_hist, rho,
                "3am", 3, false, false);
        TMatrixD kref = keyspdf->getWeights(0);
        Roo1DMomentMorphFunction f("f", "f", *obs, kref);
        BinningUtil::getBinning(*binning, *obs,f);
        obs->setBinning(*binning, "adaptive");
        delete keyspdf;
        cout << "total bins: " << binning->numBoundaries() << endl;
        newdatahist = BinningUtil::makeAsimov1D( *histpdf, *obs, 
                *binning,
                "adaptive", 
                nom_hist // TODO: use un-smoothed histogram
                );
        delete datahist; // release old dataHist
    } else {
        newdatahist = datahist;
    }

    if (!use_mcc_ || !is_norm) {
        delete histpdf;
        RooHistPdf *newhistpdf = new RooHistPdf(pdfname.c_str(), pdfname.c_str(), 
                obs_list_, *newdatahist, 3);
        return newhistpdf;
    } else {
        delete histpdf; // delete old histpdf 
        auto* expandDataHist = new RooExpandedDataHist(*newdatahist, Form("%s_EDH",base_name));
        auto* newhistpdf = new RooExpandedHistPdf(pdfname.c_str(), pdfname.c_str(), 
                obs_list_, *expandDataHist, 3);
        return newhistpdf;
    }
}

void SampleHist::getShapeSys()
{
    // call this function every time dealing with new category
    // return a dictionary for all shape nusiance parameter in specific category
    // to avoid visiting the files many times
    // name convention of histograms are: 
    // 1. npName-CategoryName-sym, for symmetry error
    // 2. npName-CategoryName-up, for upward error
    // 3. npName-CategoryName-down, for downward error
    
    //prepare for the a new category
    shapes_dic.clear();
    paramNames.clear();
    sysPdfs.clear();
    if(!shape_files_ || shape_files_->IsZombie()) return;
        
    TIter next(shape_files_->GetListOfKeys());
    TKey* key;
    while((key = (TKey*) next())) {
        TString keyname(key->GetName());
        if(! keyname.Contains(this->category_name_)) continue;

        // use '-' to separate np names and category names
        vector<string> splitNames;
        Helper::tokenizeString(key->GetName(), '-', splitNames);
        if (splitNames.size() < 4) {
            log_err("%s is not valid", key->GetName());
            continue;
        }
        TString npName(splitNames.at(1));
        TString varyName(splitNames.at(3));
        if (varyName.EqualTo("down")) continue;

        // only load the systematics of current category
        vector<TH1*> shape_vary;
        TH1* h_up = dynamic_cast<TH1*>(key->ReadObj());
        if(!Helper::IsGoodTH1(h_up)){
            log_err("%s is NaN, skipped", h_up->GetName());
            continue;
        }
        if (varyName.EqualTo("sym")) {
            shape_vary.push_back(h_up);
        } else if (varyName.EqualTo("up")) {
            // make sure first push_back "up" then "down" !
            TString& downname = keyname.ReplaceAll("up","down");
            TH1* h_down = dynamic_cast<TH1*>(shape_files_->Get(downname.Data()));
            if (Helper::IsGoodTH1(h_down)) { 
                shape_vary.push_back(h_up);
                shape_vary.push_back(h_down);
            } else{ 
                cout<<" Cannot find down shape: "<< downname.Data()<<endl;
                continue;
            }
        } else {
            cout << "Don't undertand the histname: " << keyname << endl;
        }
        shapes_dic[npName] = shape_vary;
    }
    cout << shapes_dic.size() << " shape systematics added!" << endl;
}


bool SampleHist::addShapeSys(const TString& npName)
{
    vector<TH1*>* shape_varies = nullptr;
    try{
        shape_varies = &(this->shapes_dic.at(npName));
    } catch (const out_of_range& oor) {
        // log_info("%s not implemented for %s", npName.Data(), base_name_.Data());
        return false;
    }
    if (!nom_hist) return false;

    TH1* histUp   = dynamic_cast<TH1*>(nom_hist->Clone(Form("%s_up",  nom_hist->GetName())));
    TH1* histDown = dynamic_cast<TH1*>(nom_hist->Clone(Form("%s_down",nom_hist->GetName())));
    if (shape_varies->size() == 1) {
        // add symmetric error
        TH1* h1 = shape_varies->at(0);
        for (int i = 0; i < nom_hist->GetNbinsX(); i++) {
            histUp->SetBinContent(i+1, nom_hist->GetBinContent(i+1)*(1+h1->GetBinContent(i+1)));
            histDown->SetBinContent(i+1, nom_hist->GetBinContent(i+1)*(1-h1->GetBinContent(i+1)));
        }
    } else if(shape_varies->size() == 2) {
        // add asymmetric error
        TH1* h1 = shape_varies->at(0);
        TH1* h2 = shape_varies->at(1);
        // h1->Scale(1./h1->Integral());
        // h2->Scale(1./h2->Integral());
        for (int i = 0; i < nom_hist->GetNbinsX(); i++) {
            float x_val = nom_hist->GetBinCenter(i+1);
            int ibin_1 = h1->FindBin(x_val);
            int ibin_2 = h2->FindBin(x_val);
            float up_var = cut_sys(h1->GetBinContent(ibin_1));
            float down_var = cut_sys(h2->GetBinContent(ibin_2));

            histUp->SetBinContent(i+1, nom_hist->GetBinContent(i+1)*up_var);
            histDown->SetBinContent(i+1, nom_hist->GetBinContent(i+1)*down_var);
        }
    } else {
        cout <<"WARNNING: (SampleHist::addShapeSys) Check the size of shape varies: " 
            << shape_varies->size() <<endl;
    }
    paramNames.push_back(string(npName.Data()));
    RooAbsPdf* histUpPDF   = this->makeHistPdf(histUp,   Form("%s_%s_up",base_name_.Data(), npName.Data()), false);
    RooAbsPdf* histDownPDF = this->makeHistPdf(histDown, Form("%s_%s_down",base_name_.Data(), npName.Data()), false);
    sysPdfs.push_back(make_pair(histDownPDF, histUpPDF));
    return true;
}


RooAbsPdf* SampleHist::getPDF(){
    nom_pdf = this->makeHistPdf(this->nom_hist, base_name_.Data(), true);
    if (use_mcc_) 
    {
        string pdfname(Form("%s_normConstraint", nom_pdf->GetName()));
        mc_constraint = new RooMCHistConstraint(pdfname.c_str(), "constraint", 
                *nom_pdf, RooMCHistConstraint::Poisson, thresh_);
    }
    if (paramNames.size() < 1) // no shape systematics
    {
        return  nom_pdf;
    } else {
        string pdfname(Form("%s_withSys", nom_pdf->GetName()));
        return this->createRooStarMomentMorph(pdfname);
    }
}


RooStarMomentMorph* SampleHist::createRooStarMomentMorph(const string& outputName)
{
    if (this->sysPdfs.size() != this->paramNames.size()) {
        cout << "problem with inputs!  sysPdfs.size()=" <<
            sysPdfs.size() << ", paramNames.size()=" << paramNames.size() << endl;
    }

    RooArgList pdfList, parList;
    vector<int> nnuispoints;
    vector<double> nrefpoints;

    for (int isys = 0; isys < (int)sysPdfs.size(); isys++)
    {
        if (sysPdfs[isys].first==0 || sysPdfs[isys].second==0) {
            cout << "pdf for " << paramNames[isys] << " missing!" << endl;
        }

        pdfList.add(*sysPdfs[isys].first);
        nrefpoints.push_back(-1.);

        pdfList.add(*sysPdfs[isys].second);
        nrefpoints.push_back(1.);

        // number variations
        nnuispoints.push_back(2);

        RooRealVar* var = Helper::createNuisanceVar(paramNames.at(isys).c_str());
        parList.add(*var);
    }
    pdfList.add(*(this->nom_pdf));

    // make RooStarMomentMorph
    auto* tmpmorph = new RooStarMomentMorph (outputName.c_str(), outputName.c_str(),
            parList, obs_list_, pdfList, nnuispoints, nrefpoints,
            RooStarMomentMorph::Linear);
    tmpmorph->useHorizontalMorphing(false);
   
    // add bin by bin scale factor
    if (use_mcc_) {
        // copied from WorkspaceToolBase, line 911
        tmpmorph->setUseBinByBinScaleFactors(true);
        auto* tmppdf = dynamic_cast<RooExpandedHistPdf*>(this->nom_pdf);
        if (tmppdf) {
            ((RooExpandedDataHist&)(tmppdf->dataHist())).applyScaleFactor(false);
        } 
    }
    RooArgSet observableSet(obs_list_);
    tmpmorph->setBinIntegrator(observableSet);
    return tmpmorph;
}

void SampleHist::setMCCThreshold(float thresh){
    if (thresh > 0) {
        use_mcc_ = true;
        thresh_ = thresh;
    } else {
        use_mcc_ = false;
    }
}

float SampleHist::cut_sys(float var)
{
    if(var > 2) return 2.0;
    if(var < 1e-3) return 1.0;
    return var;
}
