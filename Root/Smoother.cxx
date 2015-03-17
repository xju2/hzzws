// =====================================================================================
// 
//       Filename:  Smoother.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  03/13/2015 12:15:32
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#include "Smoother.h"
#include <RooDataSet.h>
#include <RooNDKeysPdf.h>
#include <TString.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <RooCmdArg.h>
#include <RooArgList.h>
#include <RooRealVar.h>

Smoother::Smoother(const char* _tree_path, const char* _tree_name, const char* _outfilename):
    weight_name("weight")
{
    TFile* file = TFile::Open(_tree_path, "read");
    tree = (TTree*) file->Get(_tree_name);
    tree ->SetDirectory(0);
    file->Close();
    outfile = TFile::Open(_outfilename, "recreate");
}

Smoother::~Smoother(){
    if(outfile->IsOpen()) outfile->Close();
}

void Smoother::smooth(RooArgSet& obsAndweight, const char* branch_name, const char* cuts, 
        const char* outname, double rho, TString& options)
{
    std::string dsname(Form("ds_%s",outname));
    RooDataSet* ds = new RooDataSet(dsname.c_str(), dsname.c_str(), 
            obsAndweight, RooFit::Import(*tree), RooFit::WeightVar(weight_name.c_str()), RooFit::Cut(cuts));
    RooNDKeysPdf *keyspdf = new RooNDKeysPdf(Form("%s_keyspdf",outname), "keyspdf", 
            obsAndweight, *ds, options, rho);
    RooArgList obsList(obsAndweight);
    RooCmdArg arg2 = RooCmdArg::none();
    RooRealVar* x = (RooRealVar*) obsList.at(0); 
    if(obsList.getSize() > 2){
        RooRealVar* y = (RooRealVar*) obsList.at(1); 
        arg2 = RooFit::YVar(*y, RooFit::Binning(y->getBinning()));
    }
    TH1* h1 = keyspdf->createHistogram(Form("%s", outname), *x, RooFit::Binning(x->getBinning()), arg2); 
    outfile ->cd();
    h1 ->Write();
}
