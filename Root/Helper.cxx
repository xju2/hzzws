#include "Hzzws/Helper.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace Helper{

void readConfig(const char* filename, char delim,
        map<string, map<string, string> >& all_dic)
{
    ifstream file(filename, ifstream::in);
    string line;
    int lineCount = 0;
    map<string, string> section_dic;
    string section_name;
    while ( getline(file, line) ){ 
        if( line[0] == '[' ){
            if( lineCount < 1 ){
                section_name = string(line.begin()+1, line.end()-1);
            }else{
                all_dic[section_name] = section_dic;
                section_dic.clear();
                section_name = string(line.begin()+1, line.end()-1);
            }
        }else if( line[0] == '#' ){
            continue;
        }else{
            size_t delim_pos = line.find(delim);
            if (delim_pos != string::npos) {
                string tagName = line.substr(0, delim_pos-1);
                string token = line.substr(delim_pos+1, line.size());
                boost::algorithm::trim(tagName);
                boost::algorithm::trim(token);
                section_dic[tagName] = token;
            } 
            else {
                cerr << line << " does not have delimeter '" << delim << "', ignored" << endl;
            }
        }
        lineCount ++ ;
    }
    all_dic[section_name] = section_dic;  //pick up the last section
}

void tokenizeString(const string& str, char delim, vector<string>& tokens)
{
    istringstream iss(str);
    string token;
    while ( getline(iss, token, delim) ){
        boost::algorithm::trim(token);
        tokens.push_back(token);
    }
}

void tokenizeString(const char* str, char delim, vector<string>& tokens)
{
    string tmp_str(str);
    tokenizeString(tmp_str, delim, tokens);
}

void printDic( const map<string, map<string, string> >& all_dic )
{
    for(auto& kv : all_dic){
        cout << "section: |" << kv.first << "|" << endl;
        for(auto& sec : kv.second){
            cout<< "\t |" << sec.first <<"| = |" << sec.second << "|" << endl;
        }
    }
}

RooRealVar* createNuisanceVar(const char* npName)
{
    string npVarName(Form("alpha_%s",npName));
    RooRealVar* npVar = new RooRealVar(npVarName.c_str(), npVarName.c_str(), 0.0, -5., 5.);
    return npVar;
}

RooRealVar* createGlobalVar(const char* npName)
{
    string npVarName(Form("nom_%s",npName));
    RooRealVar* npVar = new RooRealVar(npVarName.c_str(), npVarName.c_str(), 0.0, -10., 10.);
    npVar->setConstant();
    return npVar;
}

RooAbsPdf* createConstraint(const char* npName)
{
    // TODO: may add other constraint functions here
    auto* var = createNuisanceVar(npName);
    auto* mean = createGlobalVar(npName);
    auto* sigma = new RooRealVar("sigma", "sigma", 1.0);
    string _pdfname(Form("alpha_%sConstraint", npName ));
    RooGaussian* gauss = new RooGaussian(_pdfname.c_str(), _pdfname.c_str(), *var, *mean, *sigma);
    return gauss;
}

}
