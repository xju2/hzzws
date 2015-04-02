// 
//    Description:  
// 
#include "Hzzws/SmoothMan.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <algorithm>

#include "RooRealVar.h"

#include "Hzzws/Helper.h"

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

    vector<string> signals;
    Helper::tokenizeString(m_dic["main"]["signals"], ',', signals);
    vector<string> backgrounds;
    Helper::tokenizeString(m_dic["main"]["backgrounds"], ',', backgrounds);

    vector<string> allsamples;
    for (auto &s : signals) allsamples.push_back(s);
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

        string outname = outputname + "_" + sample;

        vector<string> props;
        Helper::tokenizeString(m_dic["main"][sample], ',', props);
        float rho = atof(props[1].c_str());

        string inf = string(getenv("WSDIR")) + "/share/" + props[0];
        ifstream f(inf.c_str(), ifstream::in);
        string l;

        if (find(signals.begin(), signals.end(), sample) != signals.end()) {
            while (getline(f, l)) {
                vector<string> p;
                Helper::tokenizeString(l, ' ', p);
                cout << "File: " << p[0] << endl;

                outname = outname + "_" + p[1];
                Smoother *sm = new Smoother(outname, rho);
                sm->setInFileSingle(p[0]);

                processSmoother(sm);

                delete sm;
            }
            f.close();
        }
        else {
            vector<string> files;
            while (getline(f, l)) {
                files.push_back(l);
            }
            f.close();
            
            Smoother *sm = new Smoother(outname, rho);
            sm->setInFileMulti(files);

            processSmoother(sm);

            delete sm;
        }
    }
}

void SmoothMan::processSmoother(Smoother *sm) {
    string oname, treename;
    RooArgSet treeobs;
    if (m_dic["main"].count("treename") && m_dic["main"].count("branch") && m_dic["main"].count("observables")) {
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
        
        if (m_dic[c].count("treename") && m_dic[c].count("branch") && m_dic[c].count("observables")) {
            string onametemp, treenametemp;
            RooArgSet treeobstemp;
            getObs(c, onametemp, treenametemp, treeobstemp);
            sm->smooth(onametemp + c, treenametemp, treeobstemp, cut);
        }
        else {
            sm->smooth(oname + c, treename, treeobs, cut);
        }
    }
}

void SmoothMan::getObs(string cat, string &oname, string &treename, RooArgSet &treeobs) {
    treename = m_dic[cat]["treename"];

    vector<string> branches;  
    Helper::tokenizeString(m_dic[cat]["branch"], ',', branches);
    vector<string> obs;
    Helper::tokenizeString(m_dic[cat]["observables"], ',', obs);
    if (branches.size() % 4 != 0) {
        cout << "Wrong number of parameters for observable, skipping." << endl;
        return;
    }
    if (branches.size() / 4 != obs.size()) {
        cout << "Mismatch between branches and observables, skipping." << endl;
        return;
    }
    
    for (int i = 0; i < (int) branches.size() / 4; i++) {
        string obsname = branches[i];
        int obsbins = atoi(branches[i + 1].c_str());
        float obslow = atof(branches[i + 2].c_str());
        float obshigh = atof(branches[i + 3].c_str());
        RooRealVar *v = new RooRealVar(obsname.c_str(), obsname.c_str(), obslow, obshigh);
        v->setBins(obsbins);
        treeobs.add(*v);
        oname += obs[i] + "_";
    }
}

