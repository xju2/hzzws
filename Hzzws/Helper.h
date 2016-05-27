//    Description:  A set of helper functions
//
#ifndef _HZZWS_HELPER_H
#define _HZZWS_HELPER_H

#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <RooRealVar.h>
#include <RooGaussian.h>

#include <TChain.h>
#include <TH1.h>
#include "TStopwatch.h"

#ifndef log_err
#define log_err(M, ...) fprintf(stdout, "[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_warn(M, ...) fprintf(stdout, "[Warning] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stdout, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

typedef std::vector<std::string> strvec;
typedef std::vector<TString> tstrvec;
typedef std::map<std::string, std::string> strmap;

using namespace std;
namespace Helper{


    /////////////////////////
    // help to read text file
    // //////////////////////
    bool readConfig(const char* filename, // input file name
            char delim,  // delimeter
            map<string, map<string, string> >& all_dic // reference to a dictionary
            );
    void readNormTable(const char* file_name,
            map<string, map<string, double> >& all_norm_dic,
            double lumi = 1.0);
    void readScaleFile(const char* file_name, map<string, double>& all_dic);
    void tokenizeString(const string& str, char delim, vector<string>& tokens);
    void tokenizeString(const char* str, char delim, vector<string>& tokens);
    void tokenizeString(const char* str, char delim, vector<TString>& tokens);
    tstrvec fileList(const char* pattern, std::map<float, TString>* m=NULL);
    template<typename T>
    void printDic( const map<string, map<string, T> >& all_dic )
    {
        for(auto& kv : all_dic){
            cout << "section: |" << kv.first << "|" << endl;
            for(auto& sec : kv.second){
                cout<< "\t |" << sec.first <<"| = |" << sec.second << "|" << endl;
            }
        }
    }
    void readAcceptancePoly(std::vector<double>& params, const char* prod, const char* chan, const char* sys="Nominal");


    // to have a uniformed name convention for nuisance parameters and global name
    RooRealVar* createNuisanceVar(const char* npName);
    RooRealVar* createGlobalVar(const char* npName);
    RooAbsPdf* createConstraint(const char* npName);

    TChain* loader(const string& inFile_name, const string& chain_name);
    bool IsGoodTH1(TH1* h1);
    void printStopwatch(TStopwatch& timer);
    const std::string& getInputPath(std::string i=std::string("."));

    void getListOfNames(const string& cut, strvec& name_list);
}
#endif
