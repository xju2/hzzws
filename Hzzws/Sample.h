// 
//    Description:  @name, name of the sample
//                  @hist_files,  root file contains smoothed histograms
//                  @shape_files, root file contains shape variations
//                  @all_norm_dic,  dictionary contains normalization uncertainties
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
#include <RooStarMomentMorph.h>

#include <fstream>
#include <map>
using namespace std;
class Sample{

    public:
        typedef map<TString, vector<TH1*> > ShapeDic;
        typedef map<TString, vector<float> >  NormDic;

        explicit Sample(const char* name, const char* nickname, const char* input, 
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
        
        string name; // used to construct PDF
        string nickname; // used to name mu, i.e. POI
        bool is_signal;
        TString baseName; // name_categoryName
        TFile* hist_files;
        TFile* shape_files;
        map<string, map<string, string> > all_norm_dic;

        //////////////////////////////////////// 
        //following variables are Category dependent
        //////////////////////////////////////// 
        string category_name;
        RooArgList obsList;            
        string obsname;

        TH1* norm_hist; // norminal histogram
        // PDF sys
        RooAbsPdf* norm_pdf;
        ShapeDic shapes_dic;
        vector<pair<RooAbsPdf*, RooAbsPdf*> > sysPdfs;
        vector<string> paramNames;
        // yields sys
        NormDic norms_dic;
        vector<double> lowValues;
        vector<double> highValues;
        RooArgList np_vars;
        // constraint
        RooArgSet* np_constraint;


        //fuctions
        RooHistPdf* makeHistPdf(TH1*);
        double getExpectedValue();
        void addMu(RooArgList& prodSet);
        void getShapeSys();
        void getNormSys();

        RooRealVar* createNuisanceVar(const char* npName);
        RooStarMomentMorph* getRooStarMomentMorph(const string& outputName);
};
#endif
