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
class Helper{
  public:
    Helper(){}
    virtual ~Helper(){}
    // help to read text file
    static void readConfig(const char* filename, // input file name
            char delim, 
            map<string, map<string, string> >& all_dic // reference to a dictionary
            );
    static void tokenizeString(const string& str, char delim, vector<string>& tokens);
    static void printDic( const map<string, map<string, string> >& all_dic );

    // to have a uniformed name convention for nuisance parameters and global name
    static RooRealVar* createNuisanceVar(const char* npName);
    static RooRealVar* createGlobalVar(const char* npName);
    static RooAbsPdf* createConstraint(const char* npName);

};
#endif
