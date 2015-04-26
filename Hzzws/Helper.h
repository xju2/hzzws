//    Description:  A set of helper functions
//
#ifndef _HZZWS_HELPER_H
#define _HZZWS_HELPER_H

#include <map>
#include <string>
#include <vector>
#include <RooRealVar.h>
#include <RooGaussian.h>

using namespace std;
namespace Helper{
    // help to read text file
    void readConfig(const char* filename, // input file name
            char delim,  // delimeter
            map<string, map<string, string> >& all_dic // reference to a dictionary
            );
    void readNormTable(const char* file_name, 
            map<string, map<string, double> >& all_norm_dic);
    void readScaleFile(const char* file_name, map<string, double>& all_dic);
    void tokenizeString(const string& str, char delim, vector<string>& tokens);
    void tokenizeString(const char* str, char delim, vector<string>& tokens);
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

    // to have a uniformed name convention for nuisance parameters and global name
    RooRealVar* createNuisanceVar(const char* npName);
    RooRealVar* createGlobalVar(const char* npName);
    RooAbsPdf* createConstraint(const char* npName);

}
#endif
