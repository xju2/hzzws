#include <stdlib.h>
#include <string>
#include <iostream>

#include <TFile.h>
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
    if (argc > 1 && string(argv[1]) == "help"){
        cout << argv[0] << " combined.root mass fixother mu_name " << endl;
        return 0;
    }

    RooStatsHelper::setDefaultMinimize();
    string input_name("combined.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu_BSM";
    double mass = 10.;
    int fixother = 0;
    if (argc > 1) input_name = string(argv[1]);
    if (argc > 2) mass = (double) atof(argv[2]);
    if (argc > 3) fixother = atoi(argv[3]);
    if (argc > 4) muName = string(argv[4]);

    Checker* stats_helper = new Checker(input_name.c_str(), wsName.c_str(),
            mcName.c_str(), dataName.c_str(), muName.c_str());
    stats_helper->CheckNuisPdfConstraint();
    // double exp_pvalue = stats_helper->getExpectedPvalue();
    // double obs_pvalue = stats_helper->getObservedPvalue();
    // cout << "expected p0: " << exp_pvalue << endl;
    // cout << "obs p0: " << obs_pvalue << endl;
    delete stats_helper;

    Limit::runAsymptoticsCLs(
            input_name.c_str(), wsName.c_str(), mcName.c_str(),
            dataName.c_str(), "asimovData_0", 
            "conditionalGlobs_0", "nominalGlobs", 
            "test", mass, 0.95, muName.c_str(), fixother
            );
    return 0;
}
