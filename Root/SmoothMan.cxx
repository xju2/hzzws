// 
//    Description:  
// 
#include "Hzzws/SmoothMan.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <exception>

#include "RooRealVar.h"
#include "TVectorD.h"

#include "Hzzws/Helper.h"

using namespace std;
SmoothMan::SmoothMan(const char *configFile) {
    readConfig(configFile);
}

SmoothMan::~SmoothMan() {

}

void SmoothMan::readConfig(const char *configFile) {
    Helper::readConfig(configFile, '=', m_dic);
    Helper::printDic(m_dic);
}

void SmoothMan::process() {
    string filedir = m_dic["main"]["filedir"];
    string outputname = m_dic["main"]["outputname"];
    string outdir = "./";
    try{
        outdir = m_dic["main"]["outdir"];
    }catch (out_of_range& oor){

    }

    vector<string> signals;
    Helper::tokenizeString(m_dic["main"]["signals"], ',', signals);
    vector<string> backgrounds;
    Helper::tokenizeString(m_dic["main"]["backgrounds"], ',', backgrounds);

    vector<string> allsamples;
    for (auto &s : signals) {allsamples.push_back(s);}
    for (auto &b : backgrounds) allsamples.push_back(b);

    cout << "Smoothing samples." << endl;
    for (auto &sample : allsamples) {
        if (!(m_dic["main"].count(sample))) {
            cout << "No sample " << sample << " defined, skipping." << endl;
            continue;
        }
        cout << "Sample: " << sample << endl;

        if (find(signals.begin(), signals.end(), sample) != signals.end() && find(backgrounds.begin(), backgrounds.end(), sample) != backgrounds.end()) {
            cout << "Sample listed in both signals and backgrounds, skipping." << endl;
            continue;
        }

        string outname = filedir + "/" + outputname + "_" + sample;

        vector<string> props;
        Helper::tokenizeString(m_dic["main"][sample], ',', props);
        if(props.size() < 2){
            cout <<"provide rho please!"<< endl;
            continue;
        }

       TVectorD rho(props.size()-1);
       for(int i=1; i<(int)props.size();i++) {rho(i-1) = atof(props[i].c_str());} 

       string inf = filedir + "/" + props[0];

        if (find(signals.begin(), signals.end(), sample) != signals.end()) {
            ifstream f(inf.c_str(), ifstream::in);
            string l;
            while (getline(f, l)) {
                vector<string> p;
                Helper::tokenizeString(l, ' ', p);
                cout << "File: " << p[0] << endl;

                string outname_mass = outdir+"/"+outname+".root";
                Smoother *sm = new Smoother(outname_mass, rho);

                processSmoother(sm, p[0]);
                delete sm;
            }
            f.close();
        }
        else {
            string outname_new = outdir+"/"+ outname;
            Smoother *sm = new Smoother(Form("%s.root",outname_new.c_str()), rho);

            processSmoother(sm, inf);

            delete sm;
        }
    }
}

void SmoothMan::processSmoother(Smoother *sm, const string& infile_name) 
{
    string oname, treename;
    RooArgSet treeobs;
    if (m_dic["main"].count("treename") && m_dic["main"].count("observables")) 
    {
        getObs("main", oname, treename, treeobs);
    }

    vector<string> categories;
    Helper::tokenizeString(m_dic["main"]["categories"], ',', categories);
    for (auto &c : categories) {
        if (!(m_dic.count(c))) {
            cout << "No category " << c << " defined, skipping." << endl;
            continue;
        }
        cout << "Category: " << c << endl;

        string cut = m_dic[c]["cut"];
        
        if ((m_dic[c].count("treename") && m_dic[c].count("observables")) || (m_dic["main"].count("treename") && m_dic[c].count("observables"))) {
            string onametemp, treenametemp;
            RooArgSet treeobstemp;
            getObs(c, onametemp, treenametemp, treeobstemp);
            sm->smooth(infile_name, onametemp + c, treenametemp, treeobstemp, cut);
        }
        else {
            sm->smooth(infile_name, oname + c, treename, treeobs, cut);
        }
    }
}

void SmoothMan::readObservable(const string& str, vector<string>& obs_str, string& branch_name)
{
    string tmp_str(str);
    Helper::tokenizeString(tmp_str, ',', obs_str);
    if(obs_str.at(0).find(":") != string::npos){
      vector<string> tmp_vec;
      Helper::tokenizeString(obs_str.at(0), ':', tmp_vec);
      branch_name = tmp_vec.at(0); 
      obs_str.at(0) = tmp_vec.at(1);
      }
    else{ // uses leftmost part of branch name: varName_aaaa_bbbb
      branch_name = obs_str.at(0);
      vector<string> tmp;
      Helper::tokenizeString(obs_str.at(0), '_', tmp);
      obs_str.at(0) = tmp.at(0);
      } 
} 

void SmoothMan::getObs(string cat, string &oname, string &treename, RooArgSet &treeobs) 
{
    if(m_dic[cat].count("treename"))  treename = m_dic[cat]["treename"];
    else  treename = m_dic["main"]["treename"];

    vector<string> branch;  
    Helper::tokenizeString(m_dic[cat]["observables"], ';', branch);

    for (int i=0; i<(int)branch.size(); ++i){
        vector<string> tmp_obs;
        string tmp_branch;
        readObservable(branch[i], tmp_obs, tmp_branch);

        if (tmp_obs.size() != 4) {
            cout << "Wrong number of parameters for observable, skipping." << endl;
            return;
        }

        RooRealVar *v = new RooRealVar(tmp_branch.c_str(), tmp_branch.c_str(), atof(tmp_obs.at(2).c_str()), atof(tmp_obs.at(3).c_str()));
        v->setBins( atoi(tmp_obs.at(1).c_str()) ); 
        treeobs.add(*v);
        oname += tmp_obs.at(0) + "_";
    }
}

