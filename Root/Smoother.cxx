#include "Hzzws/Smoother.h"

#include <RooDataSet.h>
#include <RooNDKeysPdf.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <RooCmdArg.h>
#include <RooArgList.h>
#include <RooRealVar.h>

#include <algorithm>

Smoother::Smoother(const string& outname, float rho) {
    m_rho = rho;
    outfile = TFile::Open((outname+ ".root").c_str(), "recreate");
}

Smoother::~Smoother() {
    if (outfile->IsOpen()) outfile->Close();
}

void Smoother::setInFileSingle(const string& fname) {
    m_files.clear();
    m_files.push_back(fname);
}

void Smoother::setInFileMulti(const vector<string>& files) {
    m_files.clear();
    for (const auto& file_name : files){
        m_files.push_back(file_name);
    }
}

void Smoother::smooth(const string& oname, const string& treename, 
        const RooArgSet &treeobs, const string& cut) const 
{
    if (m_files.size() < 1) {
        cout << "Error! No input file(s) specified!" << endl;
        return;
    }

    TChain* tcut = new TChain(treename.c_str()); 
    if (m_files.size() > 0) {
        for (auto &f : m_files) {
            tcut->AddFile(f.c_str());
        }
    }
    // hard-code to add Cut
    RooArgSet obsAndCut(treeobs);
    RooRealVar* cut_var = new RooRealVar("event_type", "event_type", -10, 10);
    RooRealVar* cut_var2 = new RooRealVar("prod_type", "prod_type", -10, 10);
    RooRealVar* weight = new RooRealVar("weight", "weight", 0, 10000);
    // obsAndCut.add(*weight);
    obsAndCut.add(*cut_var);
    obsAndCut.add(*cut_var2);
    RooDataSet *ds = new RooDataSet(Form("%s_RooDataSet", oname.c_str()), "dataset", 
            obsAndCut, RooFit::Import(*tcut),  RooFit::Cut(cut.c_str())/*, RooFit::WeightVar("weight")*/);
    // RooDataSet *ds = new RooDataSet(Form("%s_RooDataSet", oname.c_str()), "dataset", 
    //         obsAndCut, RooFit::Import(*tcut), RooFit::WeightVar("weight"));
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
   
    delete cut_var;
    delete h1;
    delete keyspdf;
    delete ds;
    delete tcut;
}
