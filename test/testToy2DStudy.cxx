/*
 * =====================================================================================
 *
 *       Filename:  testToyStudies.cxx
 *
 *    Description:  toy stduies for low mass 
 *
 *        Version:  1.0
 *        Created:  11/03/2015 11:41:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <string>
#include <map>

#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TSystem.h>
#include <RooWorkspace.h>
#include <RooRealVar.h>
#include "Hzzws/RooStatsHelper.h"

using namespace std;
int main(int argc, char** argv)
{
    string input_name("2015_Graviton_histfactory_v1_fb.root");
    string out_name("toys.root");
    string wsName = "combWS";
    string mcName = "ModelConfig";
    string dataName = "combData";
    string muName = "xs";
    string mhName = "mG";
    string kName = "GkM";
    int seed_init = 7;
    double kappa_input = 0.0;
    
    // Load additional class
    gSystem->Load("src/HggTwoSidedCBPdf_cc.so");
    gSystem->Load("src/HggScalarLineShapePdf_cc.so");
    gSystem->Load("src/HggGravitonLineShapePdf_cc.so");
    gSystem->Load("src/FlexibleInterpVarMkII_cc.so");

    if (argc > 1 && string(argv[1]) == "help"){
        cout << argv[0] << " ws.root mhName kName kappa seed out_name" << endl;
        return 1;
    }
    if(argc > 1) input_name = string(argv[1]);
    if(argc > 2) mhName = string(argv[2]);
    if(argc > 3) kName = string(argv[3]);
    if(argc > 4) kappa_input = (double) atof(argv[4]);
    if(argc > 5) seed_init = atoi(argv[5]);
    if(argc > 6) out_name = string(argv[6]);

    /*print out input information*/
    cout << "Input: " << input_name  << endl;
    cout << "kappa: " << kappa_input << endl;
    cout << "seed: " << seed_init << endl;
    cout << "out_name: " << out_name << endl;

    auto* file_in = TFile::Open(input_name.c_str(), "read");
    if(!file_in || file_in->IsZombie()){ return 1; }
    auto* workspace = (RooWorkspace*) file_in->Get(wsName.c_str());
    auto* obs_data = (RooDataSet*) workspace->data(dataName.c_str()); 
    auto* mc = (RooStats::ModelConfig*) workspace->obj(mcName.c_str());
    
    RooRealVar* mH = (RooRealVar*) workspace->var(mhName.c_str());
    if(mH){ 
        mH->setRange(500, 2000); 
        mH->Print();
    }

    auto* mu = (RooRealVar*) workspace->var(muName.c_str());
    if(mu) {
        mu->setRange(0, 5000);
        mu->Print();
    }

    auto* kappa = (RooRealVar*) workspace->var(kName.c_str());
    if(kappa) {
        kappa->setRange(0.01, 0.3);
        kappa->Print();
    }
    auto* nbkg = (RooRealVar*) workspace->var("nbkg");
    if(nbkg) {
        nbkg->Print();
    }

    workspace->saveSnapshot("nominalGO",*mc->GetGlobalObservables());
    workspace->saveSnapshot("nominalNP",*mc->GetNuisanceParameters());
    
    bool only_mass = true;

    auto* fout = TFile::Open(out_name.c_str(),"recreate");
    TTree* physics = new TTree("physics","physics");
    double test_statistic = -1;
    double kappa_val = -1;
    if(only_mass) kappa_val = kappa_input;
    double mG_val = -1;
    double nbkg_val = -1;
    double xs_val = -1;
    double xs_cond_val = -1;
    int itoy_val = -1;
    physics->Branch("ts", &test_statistic, "ts/D");
    physics->Branch("kappa", &kappa_val, "kappa/D");
    physics->Branch("mG", &mG_val, "mG/D");
    physics->Branch("nbkg", &nbkg_val, "nbkg/D");
    physics->Branch("xs", &xs_val, "xs/D");
    physics->Branch("xs_cond", &xs_cond_val, "xs_cond/D");
    physics->Branch("toy_id", &itoy_val, "toy_id/I");

    /* Profile NP to conditional MLE */
    double mu_value = 0.0;
    mu->setVal(mu_value);
    mu->setConstant(1);
    mH->setConstant(1);
    kappa->setConstant(1);
    auto* nll = RooStatsHelper::createNLL(obs_data, mc);
    RooStatsHelper::minimize(nll, workspace);
    workspace->saveSnapshot("condGO",*mc->GetGlobalObservables());
    workspace->saveSnapshot("condNP",*mc->GetNuisanceParameters());
    mu->setConstant(0);
    mH->setConstant(0);
    kappa->setConstant(0);

    
    RooStatsHelper::setDefaultMinimize();
    RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

    int n_xbins = 50, n_ybins = 29;
    TH2F* h2_template = new TH2F("h2_template", "2D template", n_xbins, 500, 2000, n_ybins, 0.01, 0.3);
    
    int ntoys = 1;
    for(int itoy = 0; itoy < ntoys; itoy++)
    {
        mu->setVal(mu_value); // generate toy data with mu=mu_value
        itoy_val = itoy + seed_init;
        RooDataSet* toy_data = (RooDataSet*) RooStatsHelper::generatePseudoData(workspace,  mu->GetName(), itoy_val);
        
        /***
        // get the toy from Andrew's input
        auto* f1 = TFile::Open("/afs/cern.ch/user/a/ahard/public/ForXiangyang/workspaceWithToys_Graviton.root");
        auto* toyws = (RooWorkspace*) f1->Get("combWS");
        auto* toy_data = (RooDataSet*) toyws->data(Form("toyData%d", seed_init));
        if (!toy_data) {
            cout << itoy << " does not exist"<< endl;
        }
        **/
        auto* toy_nll = RooStatsHelper::createNLL(toy_data, mc);

        // conditional fit
        cout << "doing conditional fit" << endl;
        mu->setVal(mu_value);
        mu->setConstant(1);
        RooStatsHelper::minimize(toy_nll, workspace);
        double cond_nll = toy_nll->getVal();
        xs_cond_val = mu->getVal();

        // scan mH and kappa and profile xs and theta.
        string hist_name(Form("toy_%d", itoy));
        TH2F* h2 = (TH2F*) h2_template->Clone(hist_name.c_str());
        
        mH->setConstant(1);
        kappa->setConstant(1);
        mu->setConstant(0);  // xs is profiled
        for(int ix = 1; ix < n_xbins+1; ix++) {
            double fix_mass_val = h2->GetXaxis()->GetBinCenter(ix);
            cout<<"kappa: " << kappa_input << ", mG: "<< fix_mass_val << endl;
            mH->setVal(fix_mass_val);
            if (only_mass){
                cout << "doing unconditional fit" << endl;
                int iy = h2->GetYaxis()->FindBin(kappa_input);
                kappa->setVal(kappa_input);

                RooStatsHelper::minimize(toy_nll, workspace);
                double best_nll = toy_nll->getVal();
                test_statistic = 2*(cond_nll - best_nll);
                h2->SetBinContent(ix, iy, test_statistic);
                mG_val = mH->getVal();
                nbkg_val = nbkg->getVal();
                xs_val = mu->getVal();
                physics->Fill();
            } else {
                for(int iy= 1; iy < n_ybins+1; iy++) {
                    double fix_kappa_val = h2->GetYaxis()->GetBinCenter(iy);
                    kappa->setVal(fix_kappa_val);

                    RooStatsHelper::minimize(toy_nll, workspace);
                    double best_nll = toy_nll->getVal();
                    double test_statistic = 2*(cond_nll - best_nll);
                    h2->SetBinContent(ix, iy, test_statistic);
                }
            }
        }
        mH->setConstant(0);
        kappa->setConstant(0);
        delete toy_data;
        delete toy_nll;
        fout->cd();
        h2->Write();
    }
    fout->cd();
    physics->Write();
    fout->Close();
    file_in->Close();
}
