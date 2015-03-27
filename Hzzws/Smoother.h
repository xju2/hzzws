// =====================================================================================
// 
//       Filename:  Smoother.h
// 
//    Description:  Smooth the distributions from the mini-tree
// 
//        Version:  1.0
//        Created:  03/13/2015 11:45:32
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
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
        Smoother(string outname, float rho);
        ~Smoother();

        void setInFileSingle(string fname);
        void setInFileMulti(vector<string> files);
        void smooth(string oname, string treename, RooArgSet &treeobs, string cuts);
    
   private:
        float m_rho = 1.0;
        vector<string> m_files;
        TFile *infile, *outfile;
};

#endif
