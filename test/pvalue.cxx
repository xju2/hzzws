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
using namespace std;
int main(int argc, char** argv){

    RooStatsHelper::setDefaultMinimize();
    string input_name("combined.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";
    if (argc > 1){
        input_name = string(argv[1]);
    }
    if (argc > 2){
        muName = string(argv[2]);
    }
    RooStatsHelper* stats_helper = new RooStatsHelper(input_name.c_str(), wsName.c_str(),
            mcName.c_str(), dataName.c_str(), muName.c_str());
    double exp_pvalue = stats_helper->getExpectedPvalue();
    cout << "expected p0: " << exp_pvalue << endl;
}
