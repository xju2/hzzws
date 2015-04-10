#include <stdlib.h>
#include <string>
#include <iostream>

#include <TFile.h>
#include <RooWorkspace.h>
#include "RooDataSet.h"
#include "RooStats/ModelConfig.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"

#include "Hzzws/RooStatsHelper.h"
using namespace std;
int main(int argc, char** argv){
    RooStatsHelper::setDefaultMinimize();
    string input_name("combined.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";

    TFile* f1 = TFile::Open(input_name.c_str());
    RooWorkspace* combined = (RooWorkspace*) f1->Get(wsName.c_str()); 
    // RooDataSet* data = (RooDataSet*) combined ->data(dataName.c_str());
    RooStats::ModelConfig* mc = (RooStats::ModelConfig*) combined ->obj(mcName.c_str());
    RooArgSet* nuis_tmp =(RooArgSet*) mc->GetNuisanceParameters();
    RooAbsPdf* combPdf = mc ->GetPdf();
    combined->saveSnapshot("nominalGlobs",*mc->GetGlobalObservables());
    combined->saveSnapshot("nominalNuis",*mc->GetNuisanceParameters());

    RooStatsHelper::makeAsimovData(combined, 1.0, 0.0, 
            muName.c_str(), 
            mcName.c_str(),
            dataName.c_str(),
            false);
    RooDataSet* asimovData = (RooDataSet*) combined ->data("asimovData1_paz");
    double exp_pvalue = RooStatsHelper::getPvalue(combined, 
            combPdf, mc, asimovData, nuis_tmp, "nominalGlobs", muName.c_str());
    cout << "expected p0: " << exp_pvalue << endl;
}
