#include <stdlib.h>
#include <string>
#include <iostream>

#include <TFile.h>
#include <TSystem.h>
#include <RooWorkspace.h>
#include "RooDataSet.h"
#include "RooStats/ModelConfig.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooStats/AsymptoticCalculator.h"

#include "Hzzws/RooStatsHelper.h"
#include "Hzzws/Checker.h"
#include "Hzzws/Helper.h"
#include "Hzzws/runAsymptoticsCLsCorrectBands.h"
#include "RooMinimizer.h"

using namespace std;

// bool cmd_line(int argc, char** argv, map<string, string>& options);

int main(int argc, char** argv)
{
    if (argc > 1 && string(argv[1]) == "help"){
        cout << argv[0] << " combined.root ws_name mu_name data_name" << endl;
        return 0;
    }
    // Load additional class
    gSystem->Load("src/HggTwoSidedCBPdf_cc.so");
    gSystem->Load("src/HggScalarLineShapePdf_cc.so");
    gSystem->Load("src/HggGravitonLineShapePdf_cc.so");
    gSystem->Load("src/FlexibleInterpVarMkII_cc.so");

    RooStatsHelper::setDefaultMinimize();
    string input_name("combined.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";
    if (argc > 1){
        input_name = string(argv[1]);
    }
    if (argc > 2) {
        wsName = string(argv[2]);
    }
    if (argc > 3){
        muName = string(argv[3]);
    }
    if (argc > 4){
        dataName = string(argv[4]);
    }
    Checker* stats_helper = new Checker(input_name.c_str(), wsName.c_str(),
            mcName.c_str(), dataName.c_str(), muName.c_str());
    stats_helper->CheckNuisPdfConstraint();
    // double exp_pvalue = stats_helper->getExpectedPvalue();
    double obs_pvalue = stats_helper->getObservedPvalue();
    // cout << "expected p0: " << exp_pvalue << endl;
    cout << "obs p0: " << obs_pvalue << endl;
    delete stats_helper;

    return 0;
}
/*
bool cmd_line(int argc, char** argv, map<string, string>& options)
{
    opt_dic["-p"] = "0"; // obtain pvalue    
    opt_dic["-n"] = "0"; // check nusiance parameter

}
*/
