// 
//    Description:  @name, name of the sample
//                  @hist_files,  root file contains smoothed histograms
//                  @shape_files, root file contains shape variations
//                  @norm_sys_file,  text file contains normalize sys
// 
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
        typedef map<TString, vector<TH1*> > ShapeDic;
        typedef map<TString, vector<float> > NormDic;

        explicit Sample(const char* name, const char* input, 
                const char* shape_sys, const char* norm_sys, const char* _path);
        virtual ~Sample();
        inline string getName(){ return this->name;}

        void setChannel(RooArgSet&, const char* channelName, bool with_sys);

        bool addShapeSys(TString& npName);
        bool addNormSys(TString& npName);
        
        //derivate class may want their implemations
        virtual RooAbsPdf* getPDF();
        virtual RooAbsReal*  getCoeff();

    private: 
        //Data
        string name;
        TString baseName; // name_categoryName
        TFile* hist_files;
        TFile* shape_files;
        ifstream norm_sys_file;
        //following variables are Category dependent
        string category_name;
        RooArgList obsList;            
        string obsname;

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
