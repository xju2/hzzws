// =========================================================================
// 
//    Description:  
// 
// ==========================================================================
#include "Hzzws/Sample.h"
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
#include "RooBinning.h"
#include "Roo1DMomentMorphFunction.h"

#include "Hzzws/Helper.h"
#include "Hzzws/BinningUtil.h"

Sample::Sample(const char* _name, 
        const char* _nickname,
        const char* _input,  
        const char* _shape_sys, 
        const char* _norm_sys,
        const char* _path
        ):
    name(_name),
    nickname(_nickname)
{
    hist_files_ = TFile::Open(Form("%s/%s", _path, _input), "read");
    shape_files_ = TFile::Open(Form("%s/%s", _path, _shape_sys), "read");
    Helper::readConfig(Form("%s/%s", _path, _norm_sys), '=', norm_sys_dic_);
    if(name.find("Signal") != string::npos) is_signal_ = true;
    else is_signal_ = false;

    norm_hist = nullptr;
    use_mcc_ = false;
    use_adpt_bin_ = false;
    mc_constraint = nullptr;
    expected_events = -1;
}

Sample::~Sample(){

}

RooAbsPdf* Sample::makeHistPdf(TH1* hist, const char* base_name, bool is_norm)
{
    RooRealVar* obs =(RooRealVar*) obs_list_.at(0);
    RooBinning* binning = new RooBinning();
    if (use_adpt_bin_){ // find right binning
        // TODO: use un-smoothed histogram 
        // to determine the binning
        // to calculate the statistic error
        // rho: use 0.3, determined by half-half fitting?
        RooRealVar rho("rho", "rho", 0.3);
        auto* keyspdf = new RooNDKeysPdf("keyspdf", "keyspdf", *obs, *norm_hist, rho,
                "3am", 3, false, false);
        TMatrixD kref = keyspdf->getWeights(0);
        Roo1DMomentMorphFunction f("f", "f", *obs, kref);
        BinningUtil::getBinning(*binning, *obs,f);
        obs->setBinning(*binning, "adaptive");
        delete keyspdf;
        cout << "total bins: " << binning->numBoundaries() << endl;
    } 

    RooDataHist *datahist = new RooDataHist(Form("%s_RooDataHist", base_name), "datahist",  
            this->obs_list_, hist);
    string pdfname(Form("%s_%s", base_name, obsname.c_str()));
    RooHistPdf *histpdf = new RooHistPdf(pdfname.c_str(), pdfname.c_str(), 
            this->obs_list_, *datahist, 3);
    RooDataHist* newdatahist = nullptr;
    if (use_adpt_bin_) { 
        // create a dataHist with statistic error from un-smoothed histogram
        newdatahist = BinningUtil::makeAsimov1D( *histpdf, *obs, 
                //obs->getBinning(), 
                *binning,
                "adaptive", 
                norm_hist // TODO: use un-smoothed histogram
                );
        delete datahist; // release old dataHist
    } 
    else {
        newdatahist = datahist;
    }

    if (!use_mcc_ || !is_norm) {
        delete histpdf;
        RooHistPdf *newhistpdf = new RooHistPdf(pdfname.c_str(), pdfname.c_str(), 
                obs_list_, *newdatahist, 3);
        delete binning;
        return newhistpdf;
    }
    else {
        delete histpdf; // delete old histpdf 
        auto* expandDataHist = new RooExpandedDataHist(*newdatahist, Form("%s_EDH",base_name));
        auto* newhistpdf = new RooExpandedHistPdf(pdfname.c_str(), pdfname.c_str(), 
                obs_list_, *expandDataHist, 3);
        delete binning;
        return newhistpdf;
    }
}

void Sample::getExpectedValue(){
    // TODO: need to be tested for 2-d
    expected_events = 0;  
    try {
        expected_events = normalization_dic_[category_name];
        return;
    } catch (const out_of_range& oor) {
        cout << "map for normalization does not exist, use histogram" << endl;
    }
    if (!norm_hist) {
        return ;
    }
    RooRealVar* x_var = dynamic_cast<RooRealVar*>(obs_list_.at(0));
    double xmax = x_var->getMax(), xmin = x_var->getMin();
    int binl = norm_hist->FindFixBin(xmin);
    int binh = norm_hist->FindFixBin(xmax);
    if (obs_list_.getSize() == 1) {
        expected_events = norm_hist->Integral(binl, binh);
    } 
    else if (obs_list_.getSize() == 2) {
        RooRealVar* y_var = dynamic_cast<RooRealVar*>(obs_list_.at(1));
        double ymax = y_var->getMax(), ymin = y_var->getMin();
        int binyl = norm_hist->FindFixBin(xmin, ymin);
        int binyh = norm_hist->FindFixBin(xmax, ymax);
        expected_events = norm_hist->Integral(binyl, binyh);
    } else {
        cerr << "3D is not supported.. " << endl;
    }
    cout << "Sample " << name << " expects " << expected_events << " in " << category_name << endl;
}


void Sample::setChannel(RooArgSet& _obs, const char* _ch_name, bool with_sys)
{
    obs_list_.removeAll();
    // if (mc_constraint != nullptr) delete mc_constraint; // if do so, program crash....
    mc_constraint = nullptr;

    // set observables
    auto* iter =  _obs.createIterator();
    TIter next(iter);
    RooRealVar* var;
    while ( (var = (RooRealVar*) next()) ){
        obs_list_.add(*var);
    }
    if (obs_list_.getSize() == 1) obsname = string(obs_list_.at(0)->GetName());
    else if (obs_list_.getSize() == 2) 
        obsname = string(Form("%s_%s", obs_list_.at(0)->GetName(), obs_list_.at(1)->GetName()));
    else {
        cerr << "3D is not supported.. " << endl;
    }

    // set category name
    category_name = string(_ch_name);
    cout<<" Sample: "<< name<< " set to category: " << category_name<<endl;

    base_name_ = TString(Form("%s_%s", name.c_str(), category_name.c_str()));

    // get norminal histogram
    TString histName(Form("%s_%s", obsname.c_str(), category_name.c_str()));
    norm_hist =(TH1*) hist_files_->Get(histName.Data());
    if (!norm_hist) {
        throw Form("ERROR (Sample::setChannel): histogram %s does not exist", histName.Data());
    }

    // get expected events
    getExpectedValue();
    cout << "expected events: " << expected_events << endl;
    
    if (with_sys) {
        // get shape sys dictionary
        this ->getShapeSys();
        // get normalization sys dictionary
        this ->getNormSys();
    }
}

void Sample::getShapeSys(){
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
    if(!shape_files_) return;
        
    TIter next(shape_files_->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())){
        // use '-' to separate np names and category names
        vector<string> splitNames;
        Helper::tokenizeString(key->GetName(), '-', splitNames);
        TString npName(splitNames.at(0));
        TString varyName(splitNames.at(2));

        TString keyname(key->GetName());
        if( keyname.Contains(this->category_name) )
        { // only load the systematics of current category
            vector<TH1*> shape_vary;
            if (varyName.EqualTo("sym")) {
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
            } else if (varyName.EqualTo("up")) {
                // make sure first push_back "up" then "down" !
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
                TString& downname = keyname.ReplaceAll("up","down");
                TH1* h1 = dynamic_cast<TH1*>(shape_files_->Get(downname.Data()));
                if (h1) shape_vary.push_back(h1);
                else cerr<<" Cannot find down shape: "<< downname.Data()<<endl;
            } else if (varyName.EqualTo("down")) {
                continue;
            } else {
                cerr << "Don't undertand the histname: " << keyname << endl;
            }
            shapes_dic[npName] = shape_vary;
        }
    }
    cout << shapes_dic.size() << " shape systematics added!" << endl;
}

void Sample::getNormSys(){
    norms_dic.clear();
    lowValues.clear();
    highValues.clear();
    np_vars.removeAll();
    for (auto& sec : norm_sys_dic_) {
        if (sec.first.compare(category_name) == 0) {
            for (auto& npName : sec.second) {
                istringstream iss(npName.second);
                float low_value, high_value;
                iss >> low_value >> high_value ;
                vector<float>  norm_sys;
                norm_sys.push_back(low_value);
                norm_sys.push_back(high_value);
                TString _name(npName.first);
                norms_dic[_name] = norm_sys;
            }
        }
    }
    cout << norms_dic.size() <<" normalization systematics added!" << endl;
}

bool Sample::addShapeSys(TString& npName){
    vector<TH1*> shape_varies ;
    try{
        shape_varies = this->shapes_dic.at(npName);
    } catch (const out_of_range& oor) {
        return false;
    }
    if (!norm_hist) return false;

    TH1* histUp   = dynamic_cast<TH1*>(norm_hist->Clone(Form("%s_up",  norm_hist->GetName())));
    TH1* histDown = dynamic_cast<TH1*>(norm_hist->Clone(Form("%s_down",norm_hist->GetName())));
    if (shape_varies.size() == 1) {
        // add symmetric error
        TH1* h1 = shape_varies.at(0);
        for (int i = 0; i < norm_hist->GetNbinsX(); i++) {
            histUp->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*(1+h1->GetBinContent(i+1)));
            histDown->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*(1-h1->GetBinContent(i+1)));
        }
    } else if(shape_varies.size() == 2) {
        // add asymmetric error
        TH1* h1 = shape_varies.at(0);
        TH1* h2 = shape_varies.at(1);
        for (int i = 0; i < norm_hist->GetNbinsX(); i++) {
            // histUp->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*h1->GetBinContent(i+1));
            // histDown->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*h2->GetBinContent(i+1));
            histDown->SetBinContent(i+1, h2->GetBinContent(i+1));
            histUp->SetBinContent(i+1, h1->GetBinContent(i+1));
        }
    } else {
        cerr <<"WARNNING: (Sample::addShapeSys) Check the size of shape varies: " 
            << shape_varies.size() <<endl;
    }
    paramNames.push_back(string(npName.Data()));
    RooAbsPdf* histUpPDF   = this->makeHistPdf(histUp,   Form("%s_%s_up",base_name_.Data(), npName.Data()), false);
    RooAbsPdf* histDownPDF = this->makeHistPdf(histDown, Form("%s_%s_down",base_name_.Data(), npName.Data()), false);
    sysPdfs.push_back(make_pair(histDownPDF, histUpPDF));
    // check if need to add norm sys
    if (histUp->Integral() != 1 || histDown->Integral() != 1){
     if (np_vars.index(Form("alpha_%s", npName.Data())) == -1){
        addNormSys(npName, expected_events * histDown->Integral() ,
                           expected_events * histUp->Integral());
                           }
    }
    return true;
}

bool Sample::addNormSys(TString& npName){
    vector<float>* norm_varies;
    try{
        norm_varies =&( this->norms_dic.at(npName));
    } catch (const out_of_range& oor) {
        return false;
    }
    addNormSys(npName, norm_varies ->at(0), norm_varies->at(1));
    return true;
}

void Sample::addNormSys(TString& npName, double low, double up)
{
    RooRealVar* npVar = Helper::createNuisanceVar(npName.Data());
    np_vars.add(*npVar);
    lowValues.push_back(low);
    highValues.push_back(up);
}

RooAbsPdf* Sample::getPDF(){
    norm_pdf = this->makeHistPdf(this->norm_hist, base_name_.Data(), true);
    if (use_mcc_) 
    {
        string pdfname(Form("%s_normConstraint", norm_pdf->GetName()));
        mc_constraint = new RooMCHistConstraint(pdfname.c_str(), "constraint", 
                *norm_pdf, RooMCHistConstraint::Poisson, thresh_);
    }
    if (paramNames.size() < 1) // no shape systematics
    {
        return  norm_pdf;
    } else {
        string pdfname(Form("%s_withSys", norm_pdf->GetName()));
        return this->createRooStarMomentMorph(pdfname);
    }
}

RooAbsReal* Sample::getCoeff(){
   TString varName(Form("n%s", base_name_.Data()));
   RooRealVar* norm = new RooRealVar(varName.Data(), varName.Data(), expected_events);
   RooArgList prodSet;
   prodSet.add(*norm);
   if (is_signal_) this->addMu(prodSet);
   
   if (lowValues.size() > 0) {
       TString fixName(Form("fiv_%s", base_name_.Data()));
       // add FlexibleInterpVar
       auto* fiv = new RooStats::HistFactory::FlexibleInterpVar(fixName.Data(), fixName.Data(), np_vars, 1., lowValues, highValues);
       // 4: piece-wise log outside boundaries, polynomial interpolation inside
       fiv->setAllInterpCodes(4); 
       prodSet.add(*fiv);
   }
   TString prodName(Form("nTot%s", base_name_.Data()));
   RooProduct* normProd = new RooProduct(prodName.Data(), prodName.Data(), prodSet);
   return normProd;
}

void Sample::addMu(RooArgList& prodSet)
{
    RooRealVar* mu = new RooRealVar("mu", "mu", 1.0, -30., 30);
    string mu_name(Form("mu_%s", this->nickname.c_str()));
    RooRealVar* mu_sample = new RooRealVar(mu_name.c_str(), mu_name.c_str(), 1.0, -30, 30);
    mu->setConstant(true);
    mu_sample->setConstant(true);
    prodSet.add(*mu);
    prodSet.add(*mu_sample);
}

RooStarMomentMorph* Sample::createRooStarMomentMorph(const string& outputName)
{
    if (this->sysPdfs.size() != this->paramNames.size()) {
        cerr << "problem with inputs!  sysPdfs.size()=" <<
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
    pdfList.add(*(this->norm_pdf));

    // make RooStarMomentMorph
    auto* tmpmorph = new RooStarMomentMorph (outputName.c_str(), outputName.c_str(),
            parList, obs_list_, pdfList, nnuispoints, nrefpoints,
            RooStarMomentMorph::Linear);
    tmpmorph->useHorizontalMorphing(false);
   
    // add bin by bin scale factor
    if (use_mcc_) {
        // copied from WorkspaceToolBase, line 911
        tmpmorph->setUseBinByBinScaleFactors(true);
        auto* tmppdf = dynamic_cast<RooExpandedHistPdf*>(this->norm_pdf);
        if (tmppdf) {
            ((RooExpandedDataHist&)(tmppdf->dataHist())).applyScaleFactor(false);
        } 
    }
    RooArgSet observableSet(obs_list_);
    tmpmorph->setBinIntegrator(observableSet);
    return tmpmorph;
}

string Sample::getName(){
    return this->name;
}

void Sample::setMCCThreshold(float thresh){
    if (thresh > 0) {
        use_mcc_ = true;
        thresh_ = thresh;
    } else {
        use_mcc_ = false;
    }
}
void Sample::useAdaptiveBinning(){
    use_adpt_bin_ = true;
}

RooAbsPdf* Sample::get_mc_constraint(){
    return mc_constraint;
}

void Sample::setNormalizationMap(const map<string, double>& norm_map)
{
    normalization_dic_.clear();
    for (auto& norm_iter : norm_map){
        normalization_dic_[norm_iter.first] = norm_iter.second;
    }
}
