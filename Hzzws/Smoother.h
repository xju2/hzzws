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
#include <TString.h>
#include <TTree.h>
class Smoother{
    private:
        std::string weight_name;
        TTree* tree;
        TFile* outfile;
   public:
        explicit Smoother(const char* tree_path, const char* tree_name, const char* outfilename);
        ~Smoother();
        void smooth(RooArgSet& obsAndweight, const char* branch_name, const char* cuts, 
                const char* outname, double rho, TString& options);
};
#endif
