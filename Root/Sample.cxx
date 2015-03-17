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
    nominal_pdf = NULL;
    np_constraint = NULL;
}

Sample::~Sample(){

}

RooHistPdf* Sample::makeHistPdf()
{
    TString histName(Form("%s_%s", obsname.c_str(), category_name.c_str()));
    TH1* hist =(TH1*) hist_files->Get(histName.Data());
    if(hist){
        RooDataHist *datahist = new RooDataHist(Form("%s_RooDataHist",histName.Data()), "datahist",  this->obsList, hist);
        RooHistPdf *histpdf = new RooHistPdf(Form("%s_%s_%s",obsname.c_str(),name.c_str(),category_name.c_str()), "histpdf", this->obsList, *datahist, 3);
        return histpdf;
    }else{
        std::cerr<< histName.Data() << " does not exist!!" << std::endl;
        return NULL;
    }
}


void Sample::setObs(RooArgSet& _obs){
    obsList = RooArgList(_obs);
    if(obsList.getSize() == 1) obsname = std::string(obsList.at(0)->GetName());
    else if(obsList.getSize() == 2) obsname = std::string(Form("%s_%s", obsList.at(0)->GetName(), obsList.at(1)->GetName()));
    else {
        std::cerr <<"3D is not supported.. "<<std::endl;
    }
    std::cout<<" observables name: "<< obsname << std::endl;
}

void Sample::getExpectedValue(){
    // todo: need to be tested for 2-d
    TH1* hist = dynamic_cast<TH1*>( hist_files->Get(Form("%s_%s", obsname.c_str(), category_name.c_str())) );
    RooRealVar* x_var = dynamic_cast<RooRealVar*>(obsList.at(0));
    double xmax = x_var ->getMax(), xmin = x_var ->getMin();
    int binl = hist->FindFixBin(xmin);
    int binh = hist->FindFixBin(xmax);
    if(obsList.getSize() == 1) {
        expected_values = hist->Integral(binl, binh);
    }else if(obsList.getSize() == 2) {
        RooRealVar* y_var = dynamic_cast<RooRealVar*>(obsList.at(1));
        double ymax = y_var ->getMax(), ymin = y_var ->getMin();
        int binyl = hist->FindFixBin(xmin, ymin);
        int binyh = hist->FindFixBin(xmax, ymax);
        expected_values = hist ->Integral(binyl, binyh);
    }else{
        std::cerr <<"3D is not supported.. "<<std::endl;
    }
    std::cout<<"Sample "<< name <<" expects "<< expected_values <<" in "<< category_name <<std::endl;
}

void Sample::makeNominalPdfs()
{
    nominal_pdf = this->makeHistPdf();
}

void Sample::setChannel(const char* _ch_name){
    category_name = std::string(_ch_name);
    std::cout<<" Sample: "<< name<< " set to category: " << category_name<<std::endl;
}

Sample::ShapeDic Sample::getShapeSys(){
    // return a dictionary for all shape nusiance parameter in specific category
    // to avoid visiting the files many times
    // name convention of histograms are: 
    // 1. npName_CategoryName_sym, for symmetry error
    // 2. npName_CategoryName_up, for upward error
    // 3. npName_CategoryName_down, for downward error
    TIter next(shape_files->GetListOfKeys());
    TKey* key;
    Sample::ShapeDic shapes_dic;
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
                shape_vary.push_back(dynamic_cast<TH1*>(key->ReadObj()));
                //find "down"
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
    return shapes_dic;
}


Sample::NormDic Sample::getNormSys(){
    Sample::NormDic norms_dic;
    std::vector<float>  norm_sys;
    norm_sys.push_back(0.0);
    TString np("none");
    norms_dic[np] = norm_sys;
    return  norms_dic;
}

