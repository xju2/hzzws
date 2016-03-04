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
#include <RooWorkspace.h>
#include <RooRealVar.h>
#include "Hzzws/RooStatsHelper.h"

using namespace std;
int main(int argc, char** argv)
{
    string input_name("combined.root");
    string out_name("toys.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";
    string mhName = "mH";
    int seed_init = 7;
    double poi_value = 0.0;

    if (argc > 1 && string(argv[1]) == "help"){
        cout << argv[0] << " mu_val seed out_name" << endl;
        return 1;
    }
    if(argc > 1) poi_value = (double) atof(argv[1]);
    if(argc > 2) seed_init = atoi(argv[2]);
    if(argc > 3) out_name = string(argv[3]);

    auto* file_in = TFile::Open(input_name.c_str(), "read");
    auto* workspace = (RooWorkspace*) file_in->Get(wsName.c_str());
    
    RooRealVar* mH = (RooRealVar*) workspace->var(mhName.c_str());
    if(mH){
        mH->Print();
        mH->setVal(125.09); mH->setConstant(true);
    }

    auto* mu = (RooRealVar*) workspace->var(muName.c_str());
    mu->setRange(-40, 40);
    

    auto* fout = TFile::Open(out_name.c_str(),"recreate");
    TTree* physics = new TTree("physics","physics");
    map<string, double> res;
    res["nll_hat"] = -1;
    res["poi_hat"] = -1;
    res["status_hat"] = 0;
    res["nll_cond"] = -1;
    res["poi_cond"] = -1;
    res["status_cond"] = 0;
    for( auto& dic : res){
        physics->Branch(dic.first.c_str(), 
                &(dic.second), Form("%s/D",dic.first.c_str()));
    }
    
    int ntoys = 20;
    RooStatsHelper::setDefaultMinimize();
    RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    for(int itoy = 0; itoy < ntoys; itoy++){
        RooStatsHelper::generateToy(workspace, muName.c_str(), poi_value, itoy+seed_init, res);
        physics->Fill();
    }
    fout->cd();
    physics->Write();
    fout->Close();
    file_in->Close();
}
