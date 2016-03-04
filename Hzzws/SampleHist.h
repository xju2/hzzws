/* Shapes are obtained from histograms, implemented as RooHistPdf
 * and shape variations implemented as RooStarMomentMorph
 * */
#ifndef __HZZWS_SAMPLEHIST_H__
#define __HZZWS_SAMPLEHIST_H__
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

#include "Hzzws/SampleBase.h"

using namespace std;
class SampleHist : public SampleBase {

    public:
        typedef map<TString, vector<TH1*> > ShapeDic;

        SampleHist(const char* name, // used to construct PDF
                const char* nickname,     // used to name the signal strength, if it's signal
                const char* input,        // root file contains smoothed histograms
                const char* shape_sys,    // root file contains shape variations 
                const char* norm_sys,     // text files contains normalization uncertainties
                const char* _path         // path of previous files
                );
        virtual ~SampleHist();

        virtual bool setChannel(const RooArgSet&, const char* channelName, bool with_sys);
        void setMCCThreshold(float thresh); // if thresh < 0, not use mc constraint

        virtual bool addShapeSys(const TString& npName);
        
        
        virtual RooAbsPdf* getPDF();

    private:
        RooAbsPdf* makeHistPdf(TH1*, const char* base_name, bool is_norm = false);
        void getShapeSys();
        RooStarMomentMorph* createRooStarMomentMorph(const string& outputName);
        virtual void getExpectedValue();
        float cut_sys(float var);

    protected: 
        bool use_mcc_ ; // use MC constraint if true
        float thresh_ ; // threshold value for MC constraint
        TFile* hist_files_;
        TFile* shape_files_;

        //////////////////////////////////////// 
        // Following variables are dependent on category
        //////////////////////////////////////// 
        // RooArgList obs_list_ ;            
        string obsname;
        
        TH1* norm_hist; // norminal histogram
        TH1* raw_hist;
        //////////////////////////////////////// 
        // PDF systematics
        //////////////////////////////////////// 
        ShapeDic shapes_dic;
        RooAbsPdf* norm_pdf;
        vector<pair<RooAbsPdf*, RooAbsPdf*> > sysPdfs;
        vector<string> paramNames;

};
#endif
