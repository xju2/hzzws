//    Description:  Smooth the distributions from the mini-tree
// 
#ifndef __Smoother_H__
#define __Smoother_H__
#include <string>

#include <RooArgSet.h>
#include <TH1.h>
#include <TTree.h>
#include <TChain.h>
#include "TVectorD.h"

using namespace std;

class Smoother{
   public:
        Smoother(const string& outname, TVectorD rho);
        Smoother(TVectorD rho);
        Smoother();
        ~Smoother();

        void smooth(const string& input_name, 
                const string& oname, 
                const string& treename, 
                const RooArgSet& treeobs, 
                const string& cuts) const ;

   private:
        TVectorD m_rho_;
        TFile* outfile;
};

#endif
