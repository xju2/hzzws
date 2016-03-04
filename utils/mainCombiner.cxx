#include "Hzzws/Combiner.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]){
    string configname = "test.ini";
    if (argc > 1 && string(argv[1]) == "help"){
        cout << argv[0] << " config_file combined.root lumi_factor" << endl;
        return 0;
    }
    string out_name = "combined.root";
    if (argc > 1){
        configname = string(argv[1]);
    }
    if(argc > 2) out_name = string(argv[2]);
    double lumi_fact = 1.0;
    if(argc > 3)  lumi_fact = (double)atof(argv[3]);
    Combiner* combiner = new Combiner(out_name.c_str(), configname.c_str());
    combiner->SetLumiFactor(lumi_fact);
    combiner->combine();
    delete combiner;
    return 0;
}
