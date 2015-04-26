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
    file.close();
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
void readNormTable(const char* file_name, 
        map<string, map<string, double> >& all_norm_dic)
{
    cout << "reading normalization table" << endl;
    cout << "input: " << file_name << endl;
    ifstream file(file_name, ifstream::in);
    if (!file.is_open()) {
        cerr << "ERROR: cannot open " << file_name << endl;
        exit(1);
    }
    string line;
    int lineCount = 0;
    map<string, double> sample_dic;
    vector<string> category_names;
    string section_name;
    while ( !file.eof() && file.good() )
    {
        if (lineCount++ == 0){
            getline(file, line);
           tokenizeString(line, '&', category_names);
        } else {
            string cat_name;
            file >> cat_name;
            if (cat_name == "") continue;
            int total = (int) category_names.size();
            double yield;
            int index = 0;
            while ( index < total ){
                char ch;
                file >> ch >> yield;
                sample_dic[category_names.at(index)] = yield;
                index ++;
            }
            all_norm_dic[cat_name] = sample_dic;
            sample_dic.clear();
        }
    } 
    file.close();
}

void readScaleFile(const char* file_name, map<string, double>& all_dic)
{
   ifstream file(file_name, ifstream::in);
   string sample_name;
   double scale_value;
   while (file >> sample_name >> scale_value){
        all_dic[sample_name] = scale_value;
   }
  file.close(); 
}

}
