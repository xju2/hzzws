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

#include "Hzzws/Helper.h"

Sample::Sample(const char* _name, 
        const char* _nickname,
        const char* _input,  
        const char* _shape_sys, 
        const char* _norm_sys,
        const char* _path):
    name(_name),
    nickname(_nickname)
{
    hist_files = TFile::Open(Form("%s/%s", _path, _input), "read");
    shape_files = TFile::Open(Form("%s/%s", _path, _shape_sys), "read");
    Helper::readConfig(Form("%s/%s", _path, _norm_sys), '=', all_norm_dic);
    if(name.find("Signal") != string::npos) is_signal = true;
    else is_signal = false;

    np_constraint = NULL;
    norm_hist = NULL;
}

Sample::~Sample(){

}

RooHistPdf* Sample::makeHistPdf(TH1* hist)
{
    if(hist){
        RooDataHist *datahist = new RooDataHist(Form("%s_RooDataHist",hist->GetName()), "datahist",  this->obsList, hist);
        string pdfname(Form("%s_%s", baseName.Data(), obsname.c_str()));
        RooHistPdf *histpdf = new RooHistPdf(pdfname.c_str(), "histpdf", this->obsList, *datahist, 3);
        return histpdf;
    }else{
        return NULL;
    }
}

double Sample::getExpectedValue(){
    // TODO: need to be tested for 2-d
    if(!norm_hist){
        return 0.0;
    }
    double expected_values = 0;  
    RooRealVar* x_var = dynamic_cast<RooRealVar*>(obsList.at(0));
    double xmax = x_var ->getMax(), xmin = x_var ->getMin();
    int binl = norm_hist ->FindFixBin(xmin);
    int binh = norm_hist ->FindFixBin(xmax);
    if(obsList.getSize() == 1) {
        expected_values = norm_hist ->Integral(binl, binh);
    }else if(obsList.getSize() == 2) {
        RooRealVar* y_var = dynamic_cast<RooRealVar*>(obsList.at(1));
        double ymax = y_var ->getMax(), ymin = y_var ->getMin();
        int binyl = norm_hist ->FindFixBin(xmin, ymin);
        int binyh = norm_hist ->FindFixBin(xmax, ymax);
        expected_values = norm_hist ->Integral(binyl, binyh);
    }else{
        cerr <<"3D is not supported.. "<<endl;
    }
    cout<<"Sample "<< name <<" expects "<< expected_values <<" in "<< category_name <<endl;
    return expected_values;
}


void Sample::setChannel(RooArgSet& _obs, const char* _ch_name, bool with_sys)
{
    obsList.removeAll();

    // set observables
    TIter next(_obs.createIterator());
    RooRealVar* var;
    while ( (var = (RooRealVar*) next()) ){
        obsList.add(*var);
    }
    if(obsList.getSize() == 1) obsname = string(obsList.at(0)->GetName());
    else if(obsList.getSize() == 2) obsname = string(Form("%s_%s", obsList.at(0)->GetName(), obsList.at(1)->GetName()));
    else {
        cerr << "3D is not supported.. " << endl;
    }
    cout<<" observables name: "<< obsname << endl;

    // set category name
    category_name = string(_ch_name);
    cout<<" Sample: "<< name<< " set to category: " << category_name<<endl;

    baseName = TString(Form("%s_%s", name.c_str(), category_name.c_str()));

    // get norminal histogram
    TString histName(Form("%s_%s", obsname.c_str(), category_name.c_str()));
    norm_hist =(TH1*) hist_files->Get(histName.Data());
    if(!norm_hist){
        cerr << "ERROR (Sample::setChannel): " << histName.Data() << " does not exist" << endl;
    }
    
    if (with_sys){
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
    // 1. npName_CategoryName_sym, for symmetry error
    // 2. npName_CategoryName_up, for upward error
    // 3. npName_CategoryName_down, for downward error
    
    //prepare for the a new category
    shapes_dic.clear();
    paramNames.clear();
    sysPdfs.clear();
    if(!shape_files) return;
        


    TIter next(shape_files->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*) next())){
        TString keyname(key->GetName());
        TObjArray* splitNames = keyname.Tokenize("_");
        TObjString* tmpStr = dynamic_cast<TObjString*>(splitNames->At(0));
        TString npName   = tmpStr ->GetString();
        int ntokens = splitNames->GetEntriesFast(); 
        tmpStr = dynamic_cast<TObjString*>(splitNames->At(ntokens - 1));
        TString varyName = tmpStr ->GetString();

        if( keyname.Contains(this->category_name) ){
            vector<TH1*> shape_vary;
            if(varyName.EqualTo("sym")){
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
            }else if(varyName.EqualTo("up")){
                // make sure first push_back "up" then "down" !
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
                TString& downname = keyname.ReplaceAll("up","down");
                TH1* h1 = dynamic_cast<TH1*>(shape_files->Get(downname.Data()));
                if(h1) shape_vary.push_back(h1);
                else cerr<<" Cannot find down shape: "<< downname.Data()<<endl;
            }else if(varyName.EqualTo("down")){
                continue;
            }else{
                cerr << "Don't undertand the histname: " << keyname << endl;
            }
            shapes_dic[npName] = shape_vary;
        }
        delete splitNames;
    }
    cout << shapes_dic.size() <<" shape systematics added!" << endl;
}

void Sample::getNormSys(){
    norms_dic.clear();
    for (auto& sec : all_norm_dic){
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
    }catch(const out_of_range& oor){
        return false;
    }

    TH1* histUp   = dynamic_cast<TH1*>(norm_hist->Clone(Form("%s_up",  norm_hist->GetName())));
    TH1* histDown = dynamic_cast<TH1*>(norm_hist->Clone(Form("%s_down",norm_hist->GetName())));
    if(shape_varies.size() == 1){
        // add symmetric error
        TH1* h1 = shape_varies.at(0);
        for(int i = 0; i < norm_hist->GetNbinsX(); i++){
            histUp->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*(1+h1->GetBinContent(i+1)));
            histDown->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*(1-h1->GetBinContent(i+1)));
        }
    }else if(shape_varies.size() == 2){
        // add asymmetric error
        TH1* h1 = shape_varies.at(0);
        TH1* h2 = shape_varies.at(1);
        for(int i = 0; i < norm_hist->GetNbinsX(); i++){
            histUp->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*h1->GetBinContent(i+1));
            histDown->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*h2->GetBinContent(i+1));
        }
    }else{
        cerr <<"WARNNING: (Sample::addShapeSys) Check the size of shape varies: "<< shape_varies.size() <<endl;
    }
    paramNames.push_back(string(npName.Data()));
    RooHistPdf* histUpPDF   = this->makeHistPdf(histUp);
    RooHistPdf* histDownPDF = this->makeHistPdf(histDown);
    sysPdfs.push_back(make_pair(histUpPDF, histDownPDF));
    return true;
}

bool Sample::addNormSys(TString& npName){
    vector<float>* norm_varies;
    try{
        norm_varies =&( this->norms_dic.at(npName));
    }catch(const out_of_range& oor){
        return false;
    }
    RooRealVar* npVar = this->createNuisanceVar(npName.Data());
    np_vars.add(*npVar);
    lowValues.push_back(norm_varies ->at(0));
    highValues.push_back(norm_varies->at(1));
    return true;
}

RooAbsPdf* Sample::getPDF(){
    norm_pdf = this->makeHistPdf(this->norm_hist);
    if(paramNames.size() < 1) // no shape systematics
    {
        return  norm_pdf;
    }else{
        string pdfname(Form("%s_withSys", norm_pdf->GetName()));
        RooStarMomentMorph* morph = this->getRooStarMomentMorph(pdfname);
        return morph;
    }
}

RooAbsReal* Sample::getCoeff(){
   TString varName(Form("n%s", baseName.Data()));
   RooRealVar* norm = new RooRealVar(varName.Data(), varName.Data(), this->getExpectedValue());
   RooArgList prodSet;
   prodSet.add(*norm);
   if(is_signal) this->addMu(prodSet);
   
   if(lowValues.size() > 0){
       TString fixName(Form("fiv_%s", baseName.Data()));
       // add FlexibleInterpVar
       auto* fiv = new RooStats::HistFactory::FlexibleInterpVar(fixName.Data(), fixName.Data(), np_vars, 1., lowValues, highValues);
       // 4: piece-wise log outside boundaries, polynomial interpolation inside
       fiv->setAllInterpCodes(4); 
       prodSet.add(*fiv);
   }
   TString prodName(Form("nTot%s", baseName.Data()));
   RooProduct* normProd = new RooProduct(prodName.Data(), prodName.Data(), prodSet);
   return normProd;
}

void Sample::addMu(RooArgList& prodSet)
{
    RooRealVar* mu = new RooRealVar("mu", "mu", 1.0, -30., 30);
    string mu_name(Form("mu_%s", nickname.c_str()));
    RooRealVar* mu_sample = new RooRealVar(mu_name.c_str(), mu_name.c_str(), 1.0, -30, 30);
    prodSet.add(*mu);
    prodSet.add(*mu_sample);
}
RooStarMomentMorph* Sample::getRooStarMomentMorph(const string& outputName)
{
    if (sysPdfs.size() != paramNames.size()) {
        cerr << "problem with inputs!  sysPdfs.size()=" <<
            sysPdfs.size() << ", paramNames.size()=" << paramNames.size() << endl;
    }

    RooArgList pdfList, parList;
    vector<int> nnuispoints;
    vector<double> nrefpoints;

    for (int isys=0; isys < (int)sysPdfs.size(); isys++) 
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

        RooRealVar* var = createNuisanceVar(paramNames.at(isys).c_str());
        parList.add(*var);
    }
    pdfList.add(*nomPdf);

    // make RooStarMomentMorph
    RooStarMomentMorph tmpmorph(outputName.c_str(), outputName.c_str(),
            parList, obsList, pdfList, nnuispoints, nrefpoints,
            RooStarMomentMorph::Linear);
    tmpmorph.useHorizontalMorphing(false);
    // TODO: Add bin by bin scale factor?
    return tmporph;
}

RooRealVar* Sample::createNuisanceVar(const char* npName)
{
    TString npVarName(Form("alpha_%s",npName));
    RooRealVar* npVar = new RooRealVar(npVarName.Data(), npVarName.Data(), 0.0, -5., 5.);
    return npVar;
}
