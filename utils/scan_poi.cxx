#include <stdlib.h>
#include <iostream>
#include <string>
#include <utility>

#include <RooWorkspace.h>
#include <RooRealVar.h>
#include <TFile.h>
#include <TTree.h>


#include "Hzzws/RooStatsHelper.h"
#include "Hzzws/Helper.h"

using namespace std;
int main(int argc, char** argv)
{

    RooStatsHelper::setDefaultMinimize();
    string out_name("scan_mu.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";
    int err_id = 0;
    if((argc > 1 && string(argv[1]).find("help") != string::npos) ||
            argc < 3)
    {
        cout << argv[0] << " combined.root out.root ws_name mu_name data_name 100:0:3 mH:110:140,mu:0:20" << endl;
        cout << "ws_name: name of RooWorkspace" << endl;
        cout << "mu_name: POI name, that needs to be scanned" << endl;
        cout << "100:0:3, scan mu in range of (0, 3) with 100 bins " << endl;
        cout << "data_name: observed data or asimovData_1_0. AsimovData follow convention: asimovData_1_0, 1 is mu value, 0 is mu value profiled to (-1 means no profile)" << endl;
        exit(++err_id);
    }

    string input_name(argv[1]);
    int opt_id = 2;
    if(argc > opt_id) out_name = string(argv[opt_id]); opt_id ++;
    if(argc > opt_id) wsName = string(argv[opt_id]); opt_id ++;
    if(argc > opt_id) muName = string(argv[opt_id]); opt_id ++;
    if(argc > opt_id) dataName = string(argv[opt_id]); opt_id ++;
    
    // set binning and range for POI
    int nbins = 100;
    double low = 0, hi = 3;
    if (argc > opt_id) {
        string options(argv[opt_id]);
        vector<string> tokens;
        Helper::tokenizeString(options, ':', tokens);
        if (tokens.size() != 3) {
            cout << "range setting for POI is wrong: " << options << endl;
            exit(++err_id);
        } else {
            nbins = atoi(tokens.at(0).c_str());
            low = (double) atof(tokens.at(1).c_str());
            hi = (double) atof(tokens.at(2).c_str());
        }
    }

    // set range for other parameters
    map<string, pair<double, double> > map_var_range;
    if(argc > opt_id) {
        string options(argv[opt_id]);
        cout << "range: " << options << endl;
        vector<string> tokens;
        Helper::tokenizeString(options, ',', tokens);
        for(auto iter = tokens.begin(); iter != tokens.end(); iter++){
            string token(*iter);
            vector<string> values;
            Helper::tokenizeString(token, ':', values);
            if (values.size() != 3) {
                cout << "range setting is wrong: " << token << endl;
                exit(++err_id);
            }
            double low_va = (double) atof(values.at(1).c_str());
            double hi_va = (double) atof(values.at(2).c_str());
            map_var_range[values.at(0)] = make_pair(low_va, hi_va);
        }
    }

    auto* file_in = TFile::Open(input_name.c_str(), "read");
    auto* workspace = (RooWorkspace*) file_in->Get(wsName.c_str());

    if (map_var_range.size() > 0) {
        for(auto it = map_var_range.begin(); it != map_var_range.end(); it++){
            string var_name(it->first);
            double low = (it->second).first;
            double hi = (it->second).second;
            auto par = (RooRealVar*) workspace->var(var_name.c_str());
            if(!par){
                log_warn("%s does not exist!", var_name.c_str());
            } else {
                log_info("%s set range to [%.2f, %.2f]", var_name.c_str(), low, hi);
                par->setRange(low, hi);
            }
        }
    }

    if (dataName.find("asimov") != string::npos){
        log_info("processing asimov data: %s", dataName.c_str());
        auto data = (RooDataSet*) workspace->data(dataName.c_str());
        if (!data) {
            log_info("%s does not exist in current workspace, make one", dataName.c_str());
            vector<string> tokens;
            Helper::tokenizeString(dataName, '_', tokens);
            double mu_val = 1.0, profileMu = 0.0;
            bool do_profile = true;
            size_t n_opts = tokens.size();
            if(n_opts > 1) mu_val = (double) atof(tokens.at(1).c_str());
            if(n_opts > 2) profileMu = (double) atof(tokens.at(2).c_str());
            if(profileMu < 0) do_profile = false;
            data = RooStatsHelper::makeAsimovData(workspace, mu_val, profileMu, muName.c_str(),
                    mcName.c_str(), dataName.c_str(), do_profile);
            dataName = data->GetName();
        }
    }

    TTree* physics = new TTree("physics", "physics");
    RooStatsHelper::ScanPOI(workspace, dataName, 
            muName.c_str(), nbins, low, hi, physics);

    auto* file_out = TFile::Open(out_name.c_str(), "RECREATE");
    file_out ->cd();
    physics->Write();
    file_out->Close();

    delete physics;
    file_in->Close();
}
