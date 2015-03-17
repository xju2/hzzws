// =====================================================================================
// 
//       Filename:  Sample.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  03/13/2015 11:38:50
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#include "Hzzws/Sample.h"
#include <iostream>

#include <RooDataHist.h>
#include <TKey.h>
#include <TString.h>
#include <RooArgList.h>
#include <RooRealVar.h>
#include "RooStats/HistFactory/FlexibleInterpVar.h"

Sample::Sample(const char* _name, 
        const char* _input_path,  
        const char* _shape_sys_path, 
        const char* _norm_sys_path):
    name(_name)
{
    hist_files = TFile::Open(_input_path, "read");
    if(!hist_files) std::cerr<<"Input file doesnot exist: "<< _input_path << std::endl;
    shape_files = TFile::Open(_shape_sys_path, "read");
    if(!shape_files) std::cerr <<" No shape systematic for Sample: "<< _name <<
        " at: "<< _shape_sys_path<< std::endl;
    norm_sys_file.open(_norm_sys_path, std::ifstream::in);

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
    // todo: need to be tested for 2-d
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
        std::cerr <<"3D is not supported.. "<<std::endl;
    }
    std::cout<<"Sample "<< name <<" expects "<< expected_values <<" in "<< category_name <<std::endl;
    return expected_values;
}


void Sample::setChannel(RooArgSet& _obs, const char* _ch_name){
    obsList.Clear();

    // set observables
    obsList = RooArgList(_obs);
    if(obsList.getSize() == 1) obsname = std::string(obsList.at(0)->GetName());
    else if(obsList.getSize() == 2) obsname = std::string(Form("%s_%s", obsList.at(0)->GetName(), obsList.at(1)->GetName()));
    else {
        std::cerr <<"3D is not supported.. "<<std::endl;
    }
    std::cout<<" observables name: "<< obsname << std::endl;

    // set category name
    category_name = std::string(_ch_name);
    std::cout<<" Sample: "<< name<< " set to category: " << category_name<<std::endl;

    baseName = TString(Form("%s_%s", name.c_str(), category_name.c_str()));
    // get norminal histogram

    TString histName(Form("%s_%s", obsname.c_str(), category_name.c_str()));
    norm_hist =(TH1*) hist_files->Get(histName.Data());
    if(!norm_hist){
        std::cerr<< histName.Data() << " does not exist!!" << std::endl;
    }

    // get shape sys dictionary
    this ->getShapeSys();
    // get normalization sys dictionary
    this ->getNormSys();

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
            std::vector<TH1*> shape_vary;
            if(varyName.EqualTo("sym")){
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
            }else if(varyName.EqualTo("up")){
                // make sure first push_back "up" then "down" !
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
                TString& downname = keyname.ReplaceAll("up","down");
                TH1* h1 = dynamic_cast<TH1*>(shape_files->Get(downname.Data()));
                if(h1) shape_vary.push_back(h1);
                else std::cerr<<" Cannot find down shape: "<< downname.Data()<<std::endl;
            }else if(varyName.EqualTo("down")){
                continue;
            }else{
                std::cerr <<"Don't undertand the histname: "<< keyname <<std::endl;
            }
            shapes_dic[npName] = shape_vary;
        }
        delete splitNames;
    }
}

void Sample::getNormSys(){
    norms_dic.clear();
    // re-fill norm dic
    std::vector<float>  norm_sys;
    norm_sys.push_back(0.0);
    TString np("none");
    norms_dic[np] = norm_sys;
}

void Sample::addShapeSys(TString& npName){
    vector<TH1*> shape_varies ;
    try{
        shape_varies = this->shapes_dic.at(npName);
    }catch(const out_of_range& oor){
        return ;
    }

    TH1* histUp   = dynamic_cast<TH1*>(norm_hist->Clone(Form("%s_up",  norm_hist->GetName())));
    TH1* histDown = dynamic_cast<TH1*>(norm_hist->Clone(Form("%s_down",norm_hist->GetName())));
    if(shape_varies.size() == 1){
        // add symmetric error
        TH1* h1 = shape_varies.at(0);
        for(unsigned int i = 0; i < norm_hist->GetNbinsX(); i++){
            histUp->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*(1+h1->GetBinContent(i+1)));
            histDown->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*(1-h1->GetBinContent(i+1)));
        }
    }else if(shape_varies.size() == 2){
        // add asymmetric error
        TH1* h1 = shape_varies.at(0);
        TH1* h2 = shape_varies.at(1);
        for(unsigned int i = 0; i < norm_hist->GetNbinsX(); i++){
            histUp->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*h1->GetBinContent(i+1));
            histDown->SetBinContent(i+1, norm_hist->GetBinContent(i+1)*h2->GetBinContent(i+1));
        }
    }else{
        cerr <<" Check the size of shape varies: "<< shape_varies.size() <<endl;
    }
    paramNames.push_back(string(npName.Data()));
    RooHistPdf* histUpPDF   = this->makeHistPdf(histUp);
    RooHistPdf* histDownPDF = this->makeHistPdf(histDown);
    sysPdfs.push_back(make_pair(histUpPDF, histDownPDF));
}

void Sample::addNormSys(TString& npName){
    vector<float> norm_varies;
    try{
        norm_varies = this->norms_dic.at(npName);
    }catch(const out_of_range& oor){
        return;
    }
    TString npVarName(Form("alpha_%s",npName.Data()));
    RooRealVar npVar(npVarName.Data(), npVarName.Data(), 0.0, -5., 5.);
    np_vars.add(npVar);
    lowValues.push_back(norm_varies.at(0));
    highValues.push_back(norm_varies.at(1));
}

RooAbsPdf* Sample::getPDF(){
    // todo
    RooHistPdf* norm_pdf = this->makeHistPdf(this->norm_hist);
    if(paramNames.size() < 1) // no shape systematics
    {
        return  norm_pdf;
    }else{
        string pdfname(Form("%s_withSys", norm_pdf->GetName()));
        return norm_pdf;
        //RooStarMomentMorph* morph = RooStarMomentMorph(pdfname.c_str(), norm_pdf, sysPdfs, paramNames, 
    }
}

RooAbsReal* Sample::getCoeff(){
   TString varName(Form("n%s", baseName.Data()));
   RooRealVar* norm = new RooRealVar(varName.Data(), varName.Data(), this->getExpectedValue());
   if(lowValues.size() < 1){
        return  norm;
   }else{
       TString fixName(Form("fiv_%s", baseName.Data()));
       // add FlexibleInterpVar
       RooStats::HistFactory::FlexibleInterpVar* fiv = new RooStats::HistFactory::FlexibleInterpVar(fixName.Data(), fixName.Data(), np_vars, 1., lowValues, highValues);
       // 4: piece-wise log outside boundaries, polynomial interpolation inside
       fiv ->setAllInterpCodes(4); 
       TString prodName(Form("nTot%s", baseName.Data()));
       RooProduct* normProd = new RooProduct(prodName.Data(), prodName.Data(), RooArgList(*norm, *fiv));
       return normProd;
   }
}
