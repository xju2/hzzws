// =====================================================================================
// 
//       Filename:  SmoothMan.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  03/13/2015 18:21:53
//       Revision:  none
//       Compiler:  g++
// 
// =====================================================================================
#include "Hzzws/SmoothMan.h"
#include "Hzzws/Smoother.h"

using namespace std;

SmoothMan::SmoothMan(const char *configFile) {
    readConfig(configFile);
}

SmoothMan::~SmoothMan() {

}

void SmoothMan::readConfig(const char *configFile) {
    ifstream file(configFile, ifstream::in);
    string line;
    map<string, map<string, string> > all_dic;
    int lineCount = 0;
    map<string, string> section_dic;
    string section_name;
    while (getline(file, line)) {
        if (line[0] == '[') {
            if (lineCount < 1) {
                section_name = string(line.begin() + 1, line.end() - 1);
            }
            else {
                all_dic[section_name] = section_dic;
                section_dic.clear();
                section_name = string(line.begin() + 1, line.end() - 1);
            }
        }
        else {
            if (line.find("(") != string::npos) {
                int pose = line.find("=");
                int posf = line.find("(");
                int posl = line.rfind(")");
                string tagName = line.substr(0, pose);
                string token = line.substr(posf + 1, posl - posf - 1);
                boost::algorithm::trim(tagName);
                boost::algorithm::trim(token);
                section_dic[tagName] = token;
            }
            else {
                istringstream iss(line);
                string tagName;
                if(getline(iss, tagName, '=')){
                    string token;
                    getline(iss, token,'=');
                    boost::algorithm::trim(tagName);
                    boost::algorithm::trim(token);
                    section_dic[tagName] = token;
                }
            }
        }
        lineCount++;
    }
    all_dic[section_name] = section_dic;
    m_dic = all_dic;
    printDic(m_dic);
    file.close();
}

void SmoothMan::printDic(map<string, map<string, string> > &dic) {
    for (auto &kv : dic) {
        cout << "section: " << kv.first << endl;
        for (auto &sec : kv.second) {
            cout<< "\t " << sec.first <<" = " << sec.second <<endl;
        }
    }
}

void SmoothMan::process() {
    string filedir = m_dic["main"]["filedir"];
    string outputname = m_dic["main"]["outputname"];

    vector<string> signals = parser(m_dic["main"]["signals"], ',');
    vector<string> backgrounds = parser(m_dic["main"]["backgrounds"], ',');
    vector<string> categories = parser(m_dic["main"]["categories"], ',');

    // Signals
    cout << "Smoothing signals." << endl;
    for (auto &s : signals) {
        if (!(m_dic["main"].count(s))) {
            cout << "No signal " << s << " defined, skipping." << endl;
            continue;
        }
        cout << "Signal: " << s << endl;

        string outname = outputname + "_" + s;

        vector<string> props = parser(m_dic["main"][s], ',');
        float rho = atof(props[1].c_str());

        string inf = string(getenv("WSDIR")) + "/share/" + props[0];
        ifstream f(inf.c_str(), ifstream::in);
        string l;
        while (getline(f, l)) {
            vector<string> p = parser(l, ' ');
            cout << "File: " << p[0] << endl;

            float m = atof(p[1].c_str());

            outname = Form("%s_%d", outname.c_str(), (int) m);
            Smoother *sm = new Smoother(outname, rho);
            sm->setInFileSingle(p[0]);

            for (auto &c : categories) {
                if (!(m_dic.count(c))) {
                    cout << "No category " << c << " defined, skipping." << endl;
                    continue;
                }
                cout << "Category: " << c << endl;

                string oname;

                string cut = m_dic[c]["cut"];
                string treename = m_dic[c]["treename"];

                vector<string> branches = parser(m_dic[c]["branch"], ',');
                vector<string> obs = parser(m_dic[c]["observables"], ',');
                if (branches.size() % 4 != 0) {
                    cout << "Wrong number of parameters for observable, skipping." << endl;
                    continue;
                }
                if (branches.size() / 4 != obs.size()) {
                    cout << "Mismatch between branches and observables, skipping." << endl;
                    continue;
                }
                RooArgSet treeobs;
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
                
                oname += c;

                sm->smooth(oname, treename, treeobs, cut, m);
            }
            delete sm;
        }
        f.close();
    }
    
    // Backgrounds
    cout << "Smoothing backgrounds." << endl;
    for (auto &b : backgrounds) {
        if (!(m_dic["main"].count(b))) {
            cout << "No background" << b << " defined, skipping." << endl;
            continue;
        }
        cout << "Background: " << b << endl;

        string outname = outputname + "_" + b;

        vector<string> props = parser(m_dic["main"][b], ',');
        float rho = atof(props[1].c_str());
        Smoother *sm = new Smoother(outname, rho);

        string inf = string(getenv("WSDIR")) + "/share/" + props[0];
        ifstream f(inf.c_str(), ifstream::in);
        string l;
        vector<string> files;
        while (getline(f, l)) {
            files.push_back(l);
        }
        sm->setInFileMulti(files);

        for (auto &c : categories) {
            if (!(m_dic.count(c))) {
                cout << "No category " << c << " defined, skipping." << endl;
                continue;
            }
            cout << "Category: " << c << endl;

            string oname;

            string cut = m_dic[c]["cut"];
            string treename = m_dic[c]["treename"];

            vector<string> branches = parser(m_dic[c]["branch"], ',');
            vector<string> obs = parser(m_dic[c]["observables"], ',');
            if (branches.size() % 4 != 0) {
                cout << "Wrong number of parameters for observable, skipping." << endl;
                continue;
            }
            if (branches.size() / 4 != obs.size()) {
                cout << "Mismatch between branches and observables, skipping." << endl;
                continue;
            }
            RooArgSet treeobs;
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

            oname += c;

            sm->smooth(oname, treename, treeobs, cut);
        }
        f.close();
        delete sm;
    }
}

vector<string> SmoothMan::parser(string s, char d) {
    vector<string> vals;

    istringstream stream(s);
    string c;
    while (getline(stream, c, d)) {
        boost::algorithm::trim(c);
        vals.push_back(c);
    }

    return vals;
}
