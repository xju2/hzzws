//////////////////////////////////////////////////////////// 
// Sample contains smoothed histograms and systematic inputs
// It knows how to implement the systematics and acquire PDFs 
//////////////////////////////////////////////////////////// 
#ifndef __HZZWS_SAMPLE_H__
#define __HZZWS_SAMPLE_H__
#include <string>
#include <fstream>
#include <map>

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
#include <RooMCHistConstraint.h>
#include "RooBinning.h"

using namespace std;
class Sample{

    public:
        typedef map<TString, vector<TH1*> > ShapeDic;
        typedef map<TString, vector<float> >  NormDic;

        explicit Sample(const char* name, // used to construct PDF
                const char* nickname,     // used to name the signal strength, if it's signal
                const char* input,        // root file contains smoothed histograms
                const char* shape_sys,    // root file contains shape variations 
                const char* norm_sys,     // text files contains normalization uncertainties
                const char* _path         // path of previous files
                );
        virtual ~Sample();
        string getName();

        //////////////////////////////////////////////////////////// 
        // Set the status of the sample
        //////////////////////////////////////////////////////////// 
        void setChannel(RooArgSet&, const char* channelName, bool with_sys);
        void setMCCThreshold(float thresh); // if thresh < 0, not use mc constraint
        void useAdaptiveBinning(); // use adaptive binning
        void setNormalizationMap(const map<string, double>& norm_map);

        //////////////////////////////////////////////////////////// 
        // Add systematics for the nusiance parameter 'npName'
        //////////////////////////////////////////////////////////// 
        bool addShapeSys(TString& npName);
        bool addNormSys(TString& npName);
        
        // derivate class may want their implemations of the pdfs and coefficiency
        virtual RooAbsPdf* getPDF();
        virtual RooAbsReal*  getCoeff();
        virtual RooAbsPdf* get_mc_constraint();
        
    protected: 
        
        string name; 
        string nickname; 
        bool is_signal_ ;
        bool use_mcc_ ; // use MC constraint if true
        bool use_adpt_bin_ ; // use adaptive binning if true
        float thresh_ ; // threshold value for MC constraint
        TString base_name_ ; // name_categoryName
        TFile* hist_files_;
        TFile* shape_files_;
        map<string, map<string, string> > norm_sys_dic_;
        map<string, double> normalization_dic_;


        //////////////////////////////////////// 
        // Following variables are dependent on category
        //////////////////////////////////////// 
        string category_name;
        RooArgList obs_list_ ;            
        string obsname;
        // RooBinning binning;
        TH1* norm_hist; // norminal histogram
        double expected_events; // normalization
        //////////////////////////////////////// 
        // PDF systematics
        //////////////////////////////////////// 
        ShapeDic shapes_dic;
        RooAbsPdf* norm_pdf;
        vector<pair<RooAbsPdf*, RooAbsPdf*> > sysPdfs;
        vector<string> paramNames;
        //////////////////////////////////////// 
        // Normalization systematics 
        //////////////////////////////////////// 
        NormDic norms_dic;
        vector<double> lowValues;
        vector<double> highValues;
        RooArgList np_vars;
        void addNormSys(TString& npName, double low, double up);
        //////////////////////////////////////// 
        // Constraint terms for each binning systematics
        //////////////////////////////////////// 
        RooMCHistConstraint* mc_constraint; 

        //////////////////////////////////////// 
        //  Functions...
        //////////////////////////////////////// 
        RooAbsPdf* makeHistPdf(TH1*, const char* base_name, bool is_norm = false);
        void getExpectedValue();
        void addMu(RooArgList& prodSet);
        void getShapeSys();
        void getNormSys();

        RooStarMomentMorph* createRooStarMomentMorph(const string& outputName);
};
#endif
