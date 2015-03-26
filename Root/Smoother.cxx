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
#include "Hzzws/Smoother.h"
#include <RooDataSet.h>
#include <RooNDKeysPdf.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <RooCmdArg.h>
#include <RooArgList.h>
#include <RooRealVar.h>

using namespace std;

Smoother::Smoother(string outname, float rho) {
    m_rho = rho;
    string outputname = string(getenv("WSDIR")) + "/intermediates/" + outname;
    infile = new TFile();
    outfile = TFile::Open((outputname + ".root").c_str(), "recreate");
}

Smoother::~Smoother() {
    if (infile->IsOpen()) infile->Close();
    if (outfile->IsOpen()) outfile->Close();
}

void Smoother::setInFileSingle(string fname) {
    if (infile->IsOpen()) infile->Close();
    infile = TFile::Open(fname.c_str(), "read");
}

void Smoother::setInFileMulti(vector<string> files) {
    if (infile->IsOpen()) infile->Close();
    m_files = files;
}

void Smoother::smooth(string oname, string treename, RooArgSet &treeobs, string cut, float m) {
    if (!infile->IsOpen() && m != 0) {
        cout << "Invalid file!" << endl;
        return;
    }

    TTree *tcut = new TTree();
    if (m == 0) {
        TChain *chain = new TChain(treename.c_str());
        for (auto &f : m_files) {
            chain->AddFile(f.c_str());
        }
        tcut = chain->CopyTree(cut.c_str());
    }
    else {
        TTree *tree = (TTree*) infile->Get(treename.c_str());
        tcut = tree->CopyTree(cut.c_str());
    }

    //RooRealVar weight("weight", "weight", 0, 100000);
    //treeobs.add(weight);

    RooDataSet *ds = new RooDataSet(Form("%s_RooDataSet", oname.c_str()), "dataset", treeobs, RooFit::Import(*tcut)/*, RooFit::WeightVar("weight")*/);
    RooNDKeysPdf *keyspdf = new RooNDKeysPdf(Form("%s_RooNDKeysPdf", oname.c_str()), "keyspdf", treeobs, *ds, "m", m_rho);

    RooArgList obsList(treeobs);
    RooCmdArg arg2 = RooCmdArg::none();
    RooRealVar *x = (RooRealVar*) obsList.at(0); 
    if (obsList.getSize() > /*2*/ 1) {
        RooRealVar *y = (RooRealVar*) obsList.at(1); 
        arg2 = RooFit::YVar(*y, RooFit::Binning(y->getBinning()));
    }
    TH1 *h1 = keyspdf->createHistogram(Form("%s", oname.c_str()), *x, RooFit::Binning(x->getBinning()), arg2);
    h1->SetName(oname.c_str());
    outfile->cd();
    h1->Write();

    delete tcut;
}
