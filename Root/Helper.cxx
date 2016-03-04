#include "Hzzws/Helper.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include <TString.h>
#include <TMath.h>
#include "RooStats/HistFactory/RooBSplineBases.h"
#include "RooStats/HistFactory/RooBSpline.h"

namespace Helper{


bool readConfig(const char* filename, char delim,
        map<string, map<string, string> >& all_dic)
{
    cout << "reading: " << filename << endl;
    ifstream file(filename, ifstream::in);
    if(file.fail() || file.bad()){
        cout <<"file: "<< filename << " is bad." << endl; 
        return false;
    }
    string line;
    int lineCount = 0;
    map<string, string> section_dic;
    string section_name;
    while ( getline(file, line) ){ 
        boost::algorithm::trim(line);
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
    return true;
}

void tokenizeString(const string& str, char delim, vector<string>& tokens)
{
    tokens.clear();
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

void readAcceptancePoly(std::vector<double>& params, const char* prod, const char* chan, const char* sys) { 

  if (!params.empty()){
    cerr <<"ERROR: "<< __func__ <<" given params vector must be empty!"<<endl;
    return;
  }

  const char* inPolyFile = Form("%s/polyNorm.txt", getInputPath().c_str());
  string line;
  string paramString;
  ifstream myfile(inPolyFile);
  if (myfile.is_open()){
    //loop until we find prod and chan
    while (getline(myfile, line) ){
      if (line.find(prod)!=string::npos && line.find(chan)!=string::npos){
        //loop until we find right sys
        while (getline(myfile, line) ){
          if (line.find(sys)!=string::npos){
            paramString = line; 
            break;
          }
        }
        break;
      }
    }
    myfile.close();
  }
  else {
    cerr<<"ERROR: file "<<inPolyFile<<" is not open!"<<endl;
    return;
  }

  if (paramString.empty()){
    cerr<<"ERROR: could not find in "<<inPolyFile<<": ["<<prod<<" "<<chan<<"] "<<sys<<endl;
    return;
  }

  vector<string> tokens;
  tokenizeString(paramString.c_str(),' ',tokens);
  for (int i(1);i<(int)tokens.size();++i)
    params.push_back(stod(tokens[i]));
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
        map<string, map<string, double> >& all_norm_dic,
        double lumi)
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
        getline(file, line);
        if (lineCount++ == 0){
           tokenizeString(line, '&', category_names);
        } else if( line[0] == '#' ){
            cout << line <<" ignored" << endl;
            continue;
        }else{
            istringstream iss(line);
            string cat_name;
            iss >> cat_name;
            if (cat_name == "") continue;
            int total = (int) category_names.size();
            double yield;
            int index = 0;
            while ( index < total ){
                char ch;
                iss >> ch >> yield;
                sample_dic[category_names.at(index)] = yield * lumi;
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

TChain* loader(const string& inFile_name, const string& chain_name)
{
    TChain* chain = new TChain(chain_name.c_str());
    TString in_name(inFile_name);
    if(in_name.Contains("root")) {
        chain->Add(inFile_name.c_str());
        cout << "total events: " << chain->GetEntries() << " in " << inFile_name.c_str() << endl;
        return chain;
    }
    fstream input(inFile_name.c_str(), fstream::in);
    string file_name;
    int ncounter = 0;
    while (input >> file_name){
        // cout << "adding: " << file_name << endl;
        chain->Add(file_name.c_str());
        ncounter ++;
    }
    cout << "total events: " << chain->GetEntries() << " in " << ncounter << " files." << endl;
    input.close();
    return chain;
}

bool IsGoodTH1(TH1* h1){
    return (h1 != NULL) && (!TMath::IsNaN(h1->Integral())) && (h1->Integral() != 0);
}

void printStopwatch(TStopwatch& timer)
{
  double kestRealTime = timer.RealTime();
  double kestCpuTime  = timer.CpuTime();
  int real_h = 0, real_m = 0, real_s =0;
  int cpu_h = 0, cpu_m = 0, cpu_s =0;
  real_h = (int) floor(kestRealTime/3600.) ;
  real_m = (int) floor((kestRealTime - real_h * 3600)/60.);
  real_s = kestRealTime - real_h*3600 - real_m*60;

  cpu_h = (int) floor(kestCpuTime/3600.) ;
  cpu_m = (int) floor((kestCpuTime - real_h * 3600)/60.);
  cpu_s = kestCpuTime - real_h*3600 - real_m*60;
  
  printf("RealTime=%dH%dM%ds, CPU=%dH%dM%ds\n",real_h,real_m,real_s,cpu_h,cpu_m,cpu_s);
}

const std::string& getInputPath(std::string i){
  static const std::string path(i);
  return path;
}


}
