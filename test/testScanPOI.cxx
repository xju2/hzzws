#include <stdlib.h>
#include <iostream>

#include <RooWorkspace.h>
#include <RooRealVar.h>
#include <TFile.h>
#include <TTree.h>

#include <string>

#include "Hzzws/RooStatsHelper.h"
using namespace std;
int main(int argc, char** argv)
{
    RooStatsHelper::setDefaultMinimize();
    string input_name("combined.root");
    string out_name("scan_mu.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";
    string mhName = "mH";
    if(argc > 1 && string(argv[1]).compare("help") == 0){
        cout << argv[0] << " combined.root mu" << endl;
    }
    if(argc > 1) input_name = string(argv[1]);
    if(argc > 2) muName = string(argv[2]);

    auto* file_in = TFile::Open(input_name.c_str(), "read");
    auto* workspace = (RooWorkspace*) file_in->Get(wsName.c_str());
    // RooSimultaneous* simPdf = (RooSimultaneous*) workspace->obj("simPdf");
    RooRealVar* mH = (RooRealVar*) workspace->var(mhName.c_str());
    // RooRealVar* m4l = (RooRealVar*) workspace->var("m4l");
    if(mH){
        mH->setRange(110., 140.);
        mH->setVal(125.09);
        mH->setConstant(true);
    }
    auto* mu_BSM = (RooRealVar*) workspace->var("mu_BSM");
    if(mu_BSM){
        mu_BSM->setConstant(false);
    }
    auto* mu = (RooRealVar*) workspace->var("mu");
    if(mu){
        mu->setRange(0,20);
        mu->setConstant(false);
    }
/*
    string data_name = "asimovData_1_paz";
    RooStatsHelper::makeAsimovData(workspace, 1.0, 0.0, muName.c_str(), 
            "ModelConfig",
            "obsData", true);
*/   
    TTree* physics = new TTree("physics", "physics");
    string data_name = "obsData";
    RooStatsHelper::ScanPOI(workspace, data_name, 
            muName.c_str(), 100, 0., 3., physics);

    auto* file_out = TFile::Open(out_name.c_str(), "RECREATE");
    file_out ->cd();
    physics->Write();
    file_out->Close();

    delete physics;
    file_in->Close();
}
