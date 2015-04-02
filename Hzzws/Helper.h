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
            char delim, 
            map<string, map<string, string> >& all_dic // reference to a dictionary
            );
    void tokenizeString(const string& str, char delim, vector<string>& tokens);
    void printDic( const map<string, map<string, string> >& all_dic );

    // to have a uniformed name convention for nuisance parameters and global name
    RooRealVar* createNuisanceVar(const char* npName);
    RooRealVar* createGlobalVar(const char* npName);
    RooAbsPdf* createConstraint(const char* npName);

}
#endif
