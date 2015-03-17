// =====================================================================================
// 
//       Filename:  Sample.h
// 
//    Description:  @name, name of the sample
//                  @hist_files,  root file contains smoothed histograms
//                  @shape_files, root file contains shape variations
//                  @norm_sys_file,  text file contains normalize sys
// 
//        Version:  1.0
//        Created:  03/13/2015 11:20:07
//       Revision:  none
//       Compiler:  g++
// 
// 
// =====================================================================================
#ifndef __HZZWS_SAMPLE_H__
#define __HZZWS_SAMPLE_H__
#include <string>

#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <RooAbsPdf.h>
#include <RooHistPdf.h>
#include <RooArgSet.h>
#include <RooArgList.h>
#include <RooProduct.h>
#include <RooAbsReal.h>

#include <fstream>
#include <map>
using namespace std;
class Sample{

    public:
        typedef std::map<TString, std::vector<TH1*> > ShapeDic;
        typedef std::map<TString, std::vector<float> > NormDic;

        Sample(const char* name, const char* input_path, 
                const char* shape_sys_path, const char* norm_sys_path);
        virtual ~Sample();
        inline string getName(){ return this->name;}

        void setChannel(RooArgSet&, const char* channelName);

        void addShapeSys(TString& npName);
        void addNormSys(TString& npName);
        
        //derivate class may want their implemations
        virtual RooAbsPdf* getPDF();
        virtual RooAbsReal*  getCoeff();

    private: 
        //Data
        std::string name;
        TString baseName; // name_categoryName
        TFile* hist_files;
        TFile* shape_files;
        std::ifstream norm_sys_file;
        //following variables are Category dependent
        std::string category_name;
        RooArgList obsList;            
        std::string obsname;

        //double expected_values;  
        TH1* norm_hist;
        // systematics dictionary for the channel
        ShapeDic shapes_dic;
        NormDic norms_dic;
        // PDF sys
        vector<pair<RooAbsPdf*, RooAbsPdf*> > sysPdfs;
        vector<string> paramNames;
        // constraint
        RooArgSet* np_constraint;
        // yields sys
        vector<double> lowValues;
        vector<double> highValues;
        RooArgList np_vars;


        //fuctions
        RooHistPdf* makeHistPdf(TH1*);
        double getExpectedValue();
        void getShapeSys();
        void getNormSys();
};
#endif
