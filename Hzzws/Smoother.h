//    Description:  Smooth the distributions from the mini-tree
// 
#ifndef __Smoother_H__
#define __Smoother_H__
#include <string>

#include <RooArgSet.h>
#include <TH1.h>
#include <TTree.h>
#include <TChain.h>

using namespace std;

class Smoother{
   public:
        Smoother(const string& outname, float rho);
        ~Smoother();

        void setInFileSingle(const string& fname);
        void setInFileMulti(const vector<string>& files);
        void smooth(const string& oname, const string& treename, const RooArgSet &treeobs, const string& cuts) const ;
    
   private:
        float m_rho = 1.0;
        vector<string> m_files;
        TFile* outfile;
};

#endif
