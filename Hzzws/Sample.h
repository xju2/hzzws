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

#include <fstream>
#include <map>
class Sample{
    private: 
        //Data
        std::string name;
        TFile* hist_files;
        TFile* shape_files;
        std::ifstream norm_sys_file;
        //following variables are Category dependent
        std::string category_name;
        RooArgList obsList;            
        std::string obsname;

        double expected_values;  
        RooAbsPdf* nominal_pdf;  
        RooArgSet* np_constraint;

        //fuctions
        RooHistPdf* makeHistPdf();
        void getExpectedValue();

    public:
        Sample(const char* name, const char* input_path, 
                const char* shape_sys_path, const char* norm_sys_path);
        virtual ~Sample();

        void setChannel(const char* channelName);
        void setObs(RooArgSet& _obs);
        typedef std::map<TString, std::vector<TH1*> > ShapeDic;
        typedef std::map<TString, std::vector<float> > NormDic;
        ShapeDic getShapeSys();
        NormDic getNormSys();
        
        //derivate class may want their implemations
        virtual void makeNominalPdfs();
};
#endif
