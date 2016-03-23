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

using namespace std;

int main(int argc, char** argv)
{
    if ((argc > 1 && string(argv[1]) == "help") ||
            argc < 5)
    {
        cout << argv[0] << " combined.root ws_name mu_name data_name var:value,var:value" << endl;
        return 0;
    }

    RooStatsHelper::setDefaultMinimize();
    string input_name(argv[1]);
    string wsName(argv[2]);
    string mcName = "ModelConfig";
    string muName(argv[3]);
    string dataName(argv[4]);
    int opt_id = 5;
    string fix_variables = "";
    if(argc > opt_id) { fix_variables = argv[opt_id]; }  opt_id ++;

    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggTwoSidedCBPdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggScalarLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggGravitonLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/FlexibleInterpVarMkII_cc.so");

    // Checker* stats_helper = new Checker(input_name.c_str(), wsName.c_str(),
    //         mcName.c_str(), dataName.c_str(), muName.c_str());
    // stats_helper->CheckNuisPdfConstraint();
    // double exp_pvalue = stats_helper->getExpectedPvalue();
    // double obs_pvalue = stats_helper->getObservedPvalue();
    // cout << "expected p0: " << exp_pvalue << endl;
    // cout << "obs p0: " << obs_pvalue << endl;
    // delete stats_helper;

    Limit::runAsymptoticsCLs(
            input_name.c_str(), 
            wsName.c_str(), 
            mcName.c_str(),
            dataName.c_str(), 
            "asimovData_0", 
            "test", 0.95, 
            muName.c_str(), fix_variables 
            );
    return 0;
}
