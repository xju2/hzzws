#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <TFile.h>
#include <TSystem.h>
#include <RooWorkspace.h>
#include "RooDataSet.h"
#include "RooStats/ModelConfig.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooSimultaneous.h"
#include "RooStats/AsymptoticCalculator.h"

#include "Hzzws/RooStatsHelper.h"
#include "Hzzws/Helper.h"
#include "Hzzws/runAsymptoticsCLsCorrectBands.h"

using namespace std;

int main(int argc, char** argv)
{
    if ((argc > 1 && string(argv[1]) == "help") ||
            argc < 5)
    {
        cout << argv[0] << " combined.root ws_name mu_name data_name var:value,var:value option obs,exp" << endl;
        cout << "option: pvalue,limit" << endl;
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
    string options = "";
    if(argc > opt_id) { options = argv[opt_id]; }  opt_id ++;
    string data_opt = "";
    if(argc > opt_id) { data_opt = argv[opt_id]; }  opt_id ++;

    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggTwoSidedCBPdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggScalarLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggGravitonLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/FlexibleInterpVarMkII_cc.so");

    // read the workspace.
    auto input_file = TFile::Open(input_name.c_str(), "read");
    auto ws = (RooWorkspace*) input_file->Get(wsName.c_str());
    auto mc = (RooStats::ModelConfig*) ws->obj(mcName.c_str());
    auto obs_data = (RooDataSet*) ws->data(dataName.c_str());
    auto poi = (RooRealVar*) ws->var(muName.c_str());
    const RooArgSet* observables = mc->GetObservables();
    auto nuisances = mc->GetNuisanceParameters();
    const char* asimov_data_name = "asimovData_0_paz";

    RooStatsHelper::fixVariables(ws, fix_variables);
    
    auto simPdf =dynamic_cast<RooSimultaneous*>(mc->GetPdf());
    // Check Nuisance parameters
    RooStatsHelper::CheckNuisPdfConstraint(nuisances, simPdf->getAllConstraints(*observables, *const_cast<RooArgSet*>(nuisances), false));

    stringstream out_ss;
    
    if (options == "" || options.find("pvalue") != string::npos) {
        // get expected limit
        double obs_p0 = -1;
        double exp_p0 = -1;
        if (data_opt == "" || data_opt.find("obs") != string::npos) {
            obs_p0 = RooStatsHelper::getPvalue(ws, mc, obs_data, poi->GetName());
        }
        if (data_opt == "" || data_opt.find("exp") != string::npos) {
            bool do_profile = true;
            auto asimov_data = (RooDataSet*) ws->data(asimov_data_name); 
            if(!asimov_data) {
                asimov_data = RooStatsHelper::makeAsimovData(ws, 0.0, 0.0, poi->GetName(), mcName.c_str(), dataName.c_str(), do_profile);
            }
            exp_p0 = RooStatsHelper::getPvalue(ws, mc, asimov_data, poi->GetName()); 
        }
        cout << "expected p0: " << exp_p0 << endl;
        cout << "obs p0: " << obs_p0 << endl;
        out_ss << fix_variables << " " << obs_p0 << " " << exp_p0 << endl;
    } 
    if (options.find("limit") != string::npos){
        Limit::run_limit(ws, mc, obs_data, poi, asimov_data_name, &out_ss);
    }

    fstream file_out("stats_results.txt",  fstream::out);
    file_out << out_ss.str();
    file_out.close();
    return 0;
}
